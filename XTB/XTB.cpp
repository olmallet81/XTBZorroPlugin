// XTB.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"

#include "XTB.h"

using namespace TCPTools;

namespace XTB
{
	DLLFUNC int BrokerOpen(char* Name, FARPROC fpError, FARPROC fpProgress)
	{
		strcpy_s(Name, 32, "XTB");
		(FARPROC&)BrokerError = fpError;
		(FARPROC&)BrokerProgress = fpProgress;

		return PLUGIN_VERSION;
	}

	DLLFUNC int BrokerLogin(char* User, char* Pwd, char* Type, char* Account)
	{
		try
		{
			if (!User)
			{
				LogoutFromServer();
				return 0;
			}

			Username = User;
			Password = Pwd;

			ConnectorAPI.reset(GetServer(Type));
			StreamingAPI.reset(GetServer(Type, true));

			return 1;
		}
		catch (std::exception& ex)
		{
			Log("Exception caught in BrokerLogin: " + std::string(ex.what()));
			if (Diagnostics) BrokerError(ex.what());
			return 0;
		}
	}

	DLLFUNC int BrokerTime(DATE *pTimeUTC)
	{
		try
		{
			if (!ConnectorAPI->Connected())
				LoginToServer();

			if (!StreamingIsActive)
				StartStream(&ReadStream);

			clock_t currentTime = clock();
			clock_t diff = (currentTime - LastPingTime) / CLOCKS_PER_SEC;
			if ((currentTime - LastPingTime) / CLOCKS_PER_SEC > 600) // ping sent with 10 minutes interval
			{
				ExecuteCommand(ConnectorAPI.get(), PingCommand());
				StreamingAPI->WriteMessage(StreamingPing(StreamSessionId));
				LastPingTime = currentTime;
				Log("Ping sent to server!");
			}

			int64_t serverTime = GetServerTime();
			*pTimeUTC = GetOLEAutomationDate(GetServerTime());

			return IsMarketOpen(serverTime);
		}
		catch (std::exception& ex)
		{
			ConnectorAPI->Disconnect();
			Log("Exception caught in BrokerTime: " + std::string(ex.what()));
			if (Diagnostics) BrokerError(ex.what());
			return 0;
		}
	}

	DLLFUNC int BrokerAccount(char* Account, double *pBalance, double *pTradeVal, double *pMarginVal)
	{
		try
		{
			std::lock_guard<std::mutex> lock(mtx);
			*pBalance = LastBalance->balance;
			*pTradeVal = LastBalance->tradeVal;
			*pMarginVal = LastBalance->marginVal;

			return 1;
		}
		catch (std::exception& ex)
		{
			Log("Exception caught in BrokerAccount: " + std::string(ex.what()));
			if (Diagnostics) BrokerError(ex.what());
			return 0;
		}
	}

	DLLFUNC int BrokerAsset(char* Asset, double *pPrice, double *pSpread, double *pVolume, double *pPip, double *pPipCost, double *pLotAmount, double *pMarginCost, double *pRollLong, double *pRollShort)
	{
		try
		{
			if (LastTick[Asset] == nullptr)
			{
				LastTick[Asset].reset(new Tick(Asset));
				StreamingAPI->WriteMessage(TickPricesSubscribe(Asset, StreamSessionId));
			}

			if (pPrice)		
			{
				std::lock_guard<std::mutex> lock(mtx);
				*pPrice = LastTick[Asset]->ask;
				*pSpread = LastTick[Asset]->spread;
				LastTickTime = LastTick[Asset]->time;

				if (pPip)
				{
					*pPip = LastTick[Asset]->pip;
					*pPipCost = LastTick[Asset]->pipCost;
					*pLotAmount = LastTick[Asset]->lotAmount;
					*pMarginCost = LastTick[Asset]->marginCost;
					*pRollLong = LastTick[Asset]->rollLong;
					*pRollShort = LastTick[Asset]->rollShort;
				}						
			}

			return 1;
		}
		catch (std::exception& ex)
		{
			Log("Exception caught in BrokerAsset: " + std::string(ex.what()));
			if (Diagnostics) BrokerError(ex.what());
			return 0;
		}	
	}

	DLLFUNC int BrokerBuy2(char* Asset, int Amount, double StopDist, double Limit, double *pPrice, int *pFill)
	{
		try
		{	
			std::unique_ptr<TradeTransInfo> info(new TradeTransInfo());
			info->symbol = Asset;
			info->volume = ZorroToXTBVolume(Amount, Asset);
			info->type = TRANSACTION_TYPE::_OPEN;
			auto tradeID = GenerateTradeID();
			info->customComment = std::to_string(tradeID);
			Limit = GetLimitValue(Limit, Asset);

			if (Amount > 0)
			{
				double ask = LastTick.at(Asset)->ask;

				if (Limit)
				{
					if (Limit > ask)
						info->cmd = OPERATION_CODE::_BUY_STOP;
					else if (Limit < ask)
						info->cmd = OPERATION_CODE::_BUY_LIMIT;
				}
				else
					info->cmd = OPERATION_CODE::_BUY;
				
				info->price = Limit ? Limit : ask;
				info->sl = StopDist > 0 ? info->price - StopDist : 0;
			}
			else
			{
				double bid = LastTick.at(Asset)->bid;

				if (Limit)
				{
					if (Limit > bid)
						info->cmd = OPERATION_CODE::_SELL_LIMIT;
					else if (Limit < bid)
						info->cmd = OPERATION_CODE::_SELL_STOP;
				}
				else
					info->cmd = OPERATION_CODE::_SELL;

				info->price = Limit ? Limit : bid;
				info->sl = StopDist > 0 ? info->price + StopDist : 0;
			}

			int order = SendOrderToXTB(info.get());
			int status = GetOrderStatus(order);

			if (status == REQUEST_STATUS::_ACCEPTED)
			{
				if (Limit)
				{
					*pPrice = Amount < 0 ? Limit - LastTick[Asset]->spread : Limit;
					*pFill = Amount;
					return tradeID;
				}

				auto trade = GetOpenedTrade(tradeID);
				if (trade)
				{
					*pPrice = trade->open_price;
					*pFill = XTBToZorroVolume(trade->volume, trade->symbol);
				}

				return tradeID;
			}

			if (status == REQUEST_STATUS::_PENDING)
			{
				auto watch = new Watch();
				while (watch->Elapsed<std::chrono::milliseconds>() <= WaitPendingOrderFor)
				{
					if (GetOrderStatus(order) == REQUEST_STATUS::_ACCEPTED)
						return tradeID;
				}

				if (Limit)
					throw std::exception(("The limit/stop order number " + std::to_string(tradeID) + " is still pending.").c_str());
				else
					throw std::exception(("The opening of the trade number " + std::to_string(tradeID) + " is still pending.").c_str());
			}

			if (status == REQUEST_STATUS::_REJECTED)
			{
				if (Limit)
					throw std::exception(("The limit/stop order number " + std::to_string(tradeID) + " has been rejected.").c_str());
				else
					throw std::exception(("The opening of the trade number " + std::to_string(tradeID) + " has been rejected.").c_str());
			}
			else
			{
				if (Limit)
					throw std::exception(("Error while trying to open the limit/stop order number " + std::to_string(tradeID) + ".").c_str());
				else
					throw std::exception(("Error while trying to open the trade number " + std::to_string(tradeID) + ".").c_str());
			}

		}
		catch (std::exception& ex)
		{
			Log("Exception caught in BrokerBuy2: " + std::string(ex.what()));
			if (Diagnostics) BrokerError(ex.what());
			return 0;
		}
	}

	DLLFUNC int BrokerTrade(int nTradeID, double *pOpen, double *pClose, double *pCost, double *pProfit)
	{
		try
		{   
			if (!Trades.size())
				GetTradesFomXTB();

			auto record = GetOpenedTrade(nTradeID);
			if (record == nullptr)
			{
				// when the trade has been fully closed from XTB or since the last connection
				if ((record = GetClosedTrade(nTradeID)) == nullptr)
					throw std::exception(("No trade with ID number " + std::to_string(nTradeID) + " could be found.").c_str());
			}

			// stream did not start yet, Zorro just checking status of loaded trades
			if (LastTick[record->symbol] == nullptr)
				LastTick[record->symbol].reset(new Tick(record->symbol));

			if (pOpen)
			{
				*pOpen = record->open_price;
				*pClose = record->type == TRANSACTION_TYPE::_PENDING ? *pOpen
					: record->closed ? record->close_price : record->cmd == OPERATION_CODE::_SELL
					? LastTick[record->symbol]->ask : LastTick[record->symbol]->bid;
				//*pCost to be computed with swap rates and commissions 
				*pProfit = record->type == TRANSACTION_TYPE::_PENDING ? 0 : GetTradeProfit(record);
			}
	
			int volume = XTBToZorroVolume(record->volume, record->symbol);

			return record->closed ? -volume : volume;
		}
		catch (std::exception& ex)
		{
			Log("Exception caught in BrokerTrade: " + std::string(ex.what()));
			if (Diagnostics) BrokerError(ex.what());
			return NAY;
		}
	}

	DLLFUNC int BrokerStop(int nTradeID, double dStop)
	{	
		try
		{
			auto record = GetOpenedTrade(nTradeID);
			if (!record)
				throw std::exception(("No trade with ID number " + std::to_string(nTradeID) + " could be found.").c_str());

			std::unique_ptr<TradeTransInfo> info(new TradeTransInfo(record));

			info->type = TRANSACTION_TYPE::_MODIFY;
			info->sl = dStop;

			auto order = SendOrderToXTB(info.get());
			auto status = GetOrderStatus(order);

			return status == REQUEST_STATUS::_ACCEPTED ? 1 : 0;
		}
		catch (std::exception& ex)
		{
			Log("Exception caught in BrokerStop: " + std::string(ex.what()));
			if (Diagnostics) BrokerError(ex.what());
			return 0;
		}
	}

	DLLFUNC int BrokerSell2(int nTradeID, int nAmount, double Limit, double *pClose, double *pCost, double *pProfit, int *pFill) 
	{
		try
		{	
			// checking if this trade has been closed fully or partially since the last call of this method
			auto record = GetClosedTrade(nTradeID);
			if (record)
			{
				*pClose = record->close_price;
				*pProfit = record->profit;
				*pFill = XTBToZorroVolume(record->volume, record->symbol);

				// keeping only full closing trades to avoid getting twice the same id when loading old closed trades
				// we must remove the limit trade if the position was closed by one
				if (GetOpenedTrade(nTradeID)->closed)
					RemoveOpenedTrade(nTradeID);
				else
					RemoveClosedTrade(nTradeID);

				RemovePendingTrade(nTradeID);
		
				return nTradeID;
			}

			// deleting a pending trade to reduce or close the main trade is allowed at most on 50% of the size by XTB
			// modifying the volume or the level does not seem to work all the time using TRANSACTION_TYPE::_MODIFY;
			if (Limit)
			{
				if (record = GetPendingTrade(nTradeID))
				{
					// if a pending trade already exists we delete the current one and enter a new one
					Limit = GetLimitValue(Limit, record->symbol);
					auto volume = ZorroToXTBVolume(nAmount, record->symbol);
					volume = min(.5 * GetOpenedTrade(nTradeID)->volume, volume);

					// checking if it is a new limit/stop order and the old one needs to be deleted
					if (Limit != record->open_price || volume != record->volume)
					{
						std::unique_ptr<TradeTransInfo> info(new TradeTransInfo(record));
						info->type = TRANSACTION_TYPE::_DELETE;

						auto order = SendOrderToXTB(info.get());
						auto status = GetOrderStatus(order);

						if (status == REQUEST_STATUS::_ACCEPTED)
							RemovePendingTrade(nTradeID);
					}
					else
						return 0; // Zorro just checking the status of this pending trade
				}
			}
			else
			{ 
				// to close partially or fully a trade we must first delete the pending order attached 
				if (record = GetPendingTrade(nTradeID))
				{
					std::unique_ptr<TradeTransInfo> info(new TradeTransInfo(record));
					info->type = TRANSACTION_TYPE::_DELETE;

					auto order = SendOrderToXTB(info.get());
					auto status = GetOrderStatus(order);

					if (status == REQUEST_STATUS::_ACCEPTED)
						RemovePendingTrade(nTradeID);
				}
			}

			record = GetOpenedTrade(nTradeID);
			if (!record)
				throw std::exception(("No trade with ID number " + std::to_string(nTradeID) + " could be found.").c_str());

			std::unique_ptr<TradeTransInfo> info(new TradeTransInfo(record)); 

			double volume = ZorroToXTBVolume(nAmount, info->symbol);
			// limiting limit/stop orders volume to reduce or close the main trade to 50% of the current position as XTB refuses to delete the ones with more
			if (record->type != TRANSACTION_TYPE::_PENDING && Limit && volume > .5 * record->volume)
				volume = .5 * record->volume;

			Limit = GetLimitValue(Limit, info->symbol);
			info->volume = min(volume, record->volume); // we cannot close more than the current opened position
			info->type = record->type == TRANSACTION_TYPE::_OPEN ? TRANSACTION_TYPE::_CLOSE : TRANSACTION_TYPE::_DELETE;
				
			if (info->type == TRANSACTION_TYPE::_CLOSE)
			{
				if (nAmount < 0)
				{
					double ask = LastTick.at(info->symbol)->ask;

					if (Limit)
					{
						if (Limit > ask)
							info->cmd = OPERATION_CODE::_BUY_STOP;
						else if (Limit < ask)
							info->cmd = OPERATION_CODE::_BUY_LIMIT;
					}
					else
						info->cmd = OPERATION_CODE::_BUY;

					info->price = Limit ? Limit : ask;
				}
				else
				{
					double bid = LastTick.at(info->symbol)->bid;

					if (Limit)
					{
						if (Limit > bid)
							info->cmd = OPERATION_CODE::_SELL_LIMIT;
						else if (Limit < bid)
							info->cmd = OPERATION_CODE::_SELL_STOP;
					}
					else
						info->cmd = OPERATION_CODE::_SELL;

					info->price = Limit ? Limit : bid;
				}
			}
				
			auto order = SendOrderToXTB(info.get());
			auto status = GetOrderStatus(order);
			
			if (status == REQUEST_STATUS::_ACCEPTED)
			{
				if (info->type == TRANSACTION_TYPE::_DELETE)
				{
					*pProfit = -1e-5; // if 0 or not defined it will be estimated by Zorro and will not be null
					return nTradeID;
				}

				if (Limit) return 0;

				if (auto trade = GetClosedTrade(nTradeID))
				{
					*pClose = trade->close_price;
					*pProfit = trade->profit;
					*pFill = XTBToZorroVolume(trade->volume, trade->symbol);

					// keeping only full closing trades to avoid getting twice the same id when loading old closed trades
					GetOpenedTrade(nTradeID)->closed ? RemoveOpenedTrade(nTradeID) : RemoveClosedTrade(nTradeID);
				}

				return nTradeID;
			}

			if (status == REQUEST_STATUS::_PENDING)
			{
				auto watch = new Watch();
				while (watch->Elapsed<std::chrono::milliseconds>() <= WaitPendingOrderFor)
				{
					if (GetOrderStatus(order) == REQUEST_STATUS::_ACCEPTED)
					{
						if (info->type == TRANSACTION_TYPE::_DELETE)
						{
							*pProfit = -1e-5; // if 0 or not defined it will be estimated by Zorro and will not be null
							return nTradeID;
						}

						if (Limit) return 0;

						if (auto trade = GetClosedTrade(nTradeID))
						{
							*pClose = trade->close_price;
							*pProfit = trade->profit;
							*pFill = XTBToZorroVolume(trade->volume, trade->symbol);

							// keeping only full closing trades to avoid getting twice the same id when loading old closed trades
							GetOpenedTrade(nTradeID)->closed ? RemoveOpenedTrade(nTradeID) : RemoveClosedTrade(nTradeID);
						}

						return nTradeID;
					}
				}

				if (Limit)
					throw std::exception(("The limit/stop order on trade number " + std::to_string(nTradeID) + " is still pending.").c_str());
				else if (info->type == TRANSACTION_TYPE::_DELETE)
					throw std::exception(("The cancellation of the limit/stop order on trade number " + std::to_string(nTradeID) + " is still pending.").c_str());
				else
					throw std::exception(("The (partial) closing of the trade number " + std::to_string(nTradeID) + " is still pending.").c_str());
			}

			if (status == REQUEST_STATUS::_REJECTED)
			{
				if (Limit)
					throw std::exception(("The limit/stop order on trade number " + std::to_string(nTradeID) + " has been rejected.").c_str());
				else if (info->type == TRANSACTION_TYPE::_DELETE)
					throw std::exception(("The cancellation of the limit/stop order on trade number " + std::to_string(nTradeID) + " has been rejected.").c_str());
				else
					throw std::exception(("The (partial) closing of the trade number " + std::to_string(nTradeID) + " has been rejected.").c_str());
			}
			else
			{
				if (Limit)
					throw std::exception(("Error while trying to open a limit/stop order on trade number " + std::to_string(nTradeID) + ".").c_str());
				else if (info->type == TRANSACTION_TYPE::_DELETE)
					throw std::exception(("Error while trying to cancel a limit/stop order on trade number " + std::to_string(nTradeID) + ".").c_str());
				else
					throw std::exception(("Error while trying to (partially) close the trade number " + std::to_string(nTradeID) + ".").c_str());
			}
		}
		catch (std::exception& ex)
		{
			Log("Exception caught in BrokerSell2: " + std::string(ex.what()));
			if (Diagnostics) BrokerError(ex.what());
			return 0;
		}
	}

	DLLFUNC double BrokerCommand(int nCommand, DWORD dwParameters)
	{
		try
		{
			switch (nCommand)
			{
				case GET_TIME:
					return GetOLEAutomationDate(LastTickTime);
				case GET_DIGITS:
				{
					std::string asset((char*)dwParameters);
					if (!LastTick.at(asset))
						throw std::exception(("Unknow asset " + asset + ", please subscribe to this asset first.").c_str());
					return LastTick.at(asset)->digits;
				}
				case GET_MINLOT:
				{
					std::string asset((char*)dwParameters);
					if (!LastTick.at(asset))
						throw std::exception(("Unknow asset " + asset + ", please subscribe to this asset first.").c_str());
					return LastTick.at(asset)->lotMin;
				}			
				case GET_LOTSTEP:
				{
					std::string asset((char*)dwParameters);
					if (!LastTick.at(asset))
						throw std::exception(("Unknow asset " + asset + ", please subscribe to this asset first.").c_str());
					return LastTick.at(asset)->lotMin;
				}
				case GET_MAXLOT:
				{
					std::string asset((char*)dwParameters);
					if (!LastTick.at(asset))
						throw std::exception(("Unknow asset " + asset + ", please subscribe to this asset first.").c_str());
					return LastTick.at(asset)->lotMax;
				}
				case GET_SERVERSTATE:
					return ConnectorAPI->Connected() ? 1 : 2; // check if server was disconnected since the last call, if yes return 3
				case GET_BROKERZONE:
					return CET;
				case GET_DELAY:
					return CommandTimeSpace;
				case GET_NTRADES:
					return GetNumberOfOpenedTrades();
				case GET_POSITION:
				{
					std::string asset((char*)dwParameters);
					if (!LastTick.at(asset))
						throw std::exception(("Unknow asset " + asset + ", please subscribe to this asset first.").c_str());
					return GetPosition(asset);
				}
				case GET_AVGENTRY:
				{
					std::string asset((char*)dwParameters);
					if (!LastTick.at(asset))
						throw std::exception(("Unknow asset " + asset + ", please subscribe to this asset first.").c_str());
					return GetAverageEntry(asset);
				}
				case GET_TRADES:
					return GetTrades(dwParameters);
				case GET_TRADEPOSITION:
					return GetTradePosition(dwParameters);
				case GET_WAIT:
					return WaitPendingOrderFor;
				case SET_DELAY:
					CommandTimeSpace = max((int)dwParameters, 200);
				case SET_DIAGNOSTICS:
				{
					Diagnostics = (int)dwParameters == 1 ? 1 : 0;
					return 1;
				}
				case SET_PATCH:
				{
					return 16; // rollover and commission
				}
				case SET_WAIT:
				{
					WaitPendingOrderFor = (int)dwParameters;
					return 1;
				}
				case SET_LASTCONNECTION: //custom command
				{
					double dt = (double)dwParameters;
					SYSTEMTIME SystemTime = {};
					VariantTimeToSystemTime((double)dwParameters, &SystemTime);

					tm d = {};
					d.tm_sec = SystemTime.wSecond;
					d.tm_min = SystemTime.wMinute;
					d.tm_hour = SystemTime.wHour;
					d.tm_mday = SystemTime.wDay;
					d.tm_mon = SystemTime.wMonth - 1;
					d.tm_year = SystemTime.wYear - 1900;

					LastConnection = 1000 * _mkgmtime(&d) + SystemTime.wMilliseconds;
					return 1;
				}
				default:
					return 0;
			}

			return 0;
		}
		catch (std::exception& ex)
		{
			Log("Exception caught in BrokerCommand: " + std::string(ex.what()));
			if (Diagnostics) BrokerError(ex.what());
			return 0;
		}
	}
}
