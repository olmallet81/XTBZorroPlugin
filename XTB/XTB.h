#pragma once

#include <OleAuto.h>

// C++ headers
#include <algorithm>
#include <atomic>
#include <chrono>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

// OpenSSL headers
#include <openssl/ssl.h>
#include <openssl/err.h>

// RapidJSON headers
#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/error/en.h>

// TCPTools headers
#include "TCPTools/Errors.h"
#include "TCPTools/StreamReader.h"
#include "TCPTools/StreamWriter.h"
#include "TCPTools/Connector.h"

// XTB API headers
#include "XTB/Trade.h"
#include "XTB/Commands.h"
#include "XTB/Responses.h"
#include "XTB/Streaming.h"

typedef double DATE;		
#include "include\trading.h"	

#ifdef XTB_EXPORTS
#define DLLFUNC extern "C" __declspec(dllexport)
#else
#define DLLFUNC extern "C" __declspec(dllimport)
#endif

#define PLUGIN_VERSION	2

using namespace TCPTools;

namespace XTB
{
	int(__cdecl *BrokerError)(const char* txt) = NULL;

	struct Tick;
	struct Balance;

	static std::string LogPath = "C:\\Zorro\\Log\\XTB\\"; // enter your path to Zorro folder
	static std::string StreamSessionId;
	static std::string Username;
	static std::string Password;
	static std::unique_ptr<Connector> ConnectorAPI;
	static std::unique_ptr<Connector> StreamingAPI;
	static std::unique_ptr<Balance> LastBalance;
	static std::unordered_map<std::string, std::shared_ptr<TradingHoursRecord>> TradingHours;
	static std::unordered_map<std::string, std::unique_ptr<Tick>> LastTick;
	static std::unordered_map<int, std::shared_ptr<Trade>> Trades;
	static std::atomic<bool> StreamingIsActive;
	static std::atomic<bool> StopStream;
	static int64_t LastTickTime;
	static clock_t LastPingTime;
	static int WaitPendingOrderFor = 1000; // in milliseconds
	static int64_t LastConnection = 0;
	static std::mutex mtx;

	void Log(const std::string& message)
	{
		time_t currentTime;
		struct tm *localTime;
		time(&currentTime);
		localTime = localtime(&currentTime);

		char buf[20];
		strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", localTime);

		static std::string logName = "Log_XTB_" + std::string(buf) + ".txt";
		static std::string lastMessage;
		static bool init = true;
		
		if (init)
		{
			CreateDirectoryA(LogPath.c_str(), nullptr);
			std::remove((LogPath + logName).c_str());
			init = false;
		}

		if (message != lastMessage)
		{
			strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localTime);
			std::ofstream outfile(LogPath + logName, std::ios::app);
			outfile << buf << ' ' << message << '\n';
			outfile.close();
			lastMessage = message;
		}	
	}

	Connector* GetServer(char* Type, bool streaming = false) 
	{
		if (std::string(Type) == "Real")
		{
			if (streaming)
			{
				return new Connector("xapi.xtb.com", 5113, true);
			}
			else
			{
				return new Connector("xapi.xtb.com", 5112, true);
			}
		}
		else // Demo
		{
			if (streaming)
			{
				return new Connector("xapi.xtb.com", 5125, true);
			}
			else
			{
				return new Connector("xapi.xtb.com", 5124, true);
			}
		}
	}

	int ConvertLocalTimeToUTC(int ms)
	{
		int millisecs = ms % 1000;
		time_t secs = ms / 1000;
		tm* p = gmtime(&secs);
		return 1000 * (int)mktime(p) + millisecs;
	}

	void GetTradingHours(const std::string& symbol) // check the time zone is correct for these times
	{
		std::unique_ptr<TradingHoursResponse> response(new TradingHoursResponse(ExecuteCommand(ConnectorAPI.get(), TradingHoursCommand(symbol))));
		TradingHours[symbol] = response->TradingHours().at(symbol);

		_putenv_s("TZ", "CET-1CEST"); // XTB time zone
		_tzset();

		// Convert quotes and trading times to Zorro UTC time zone 

		auto& trading = TradingHours[symbol]->trading;
		for (auto it = trading.begin(); it != trading.end(); ++it)
		{
			auto& times = it->second;

			if (times.first != 0)
				times.first = ConvertLocalTimeToUTC(times.first);

			if (times.second != 86400000)
				times.second = ConvertLocalTimeToUTC(times.second);
		}

		auto& quotes = TradingHours[symbol]->quotes;
		for (auto it = quotes.begin(); it != quotes.end(); ++it)
		{
			auto& times = it->second;

			if (times.first != 0)
				times.first = ConvertLocalTimeToUTC(times.first);

			if (times.second != 86400000)
				times.second = ConvertLocalTimeToUTC(times.second);
		}
	}

	int64_t GetServerTime()
	{
		std::unique_ptr<ServerTimeResponse> response(new ServerTimeResponse(ExecuteCommand(ConnectorAPI.get(), ServerTimeCommand())));
		return response->Time();
	}

	class Watch
	{
	private:
		std::chrono::steady_clock::time_point _start;
		std::chrono::steady_clock::time_point _end;

	public:
		Watch()
		{
			_start = std::chrono::steady_clock::now();
		}

		template <typename T>
		long long Elapsed()
		{
			return std::chrono::duration_cast<T>(std::chrono::steady_clock::now() - _start).count();
		}
	};
	
	double GetOLEAutomationDate(int64_t time)
	{
		int millisecs = time % 1000;
		time_t secs = time / 1000;
		tm* p = gmtime(&secs);

		SYSTEMTIME SystemTime = {};

		SystemTime.wMilliseconds = millisecs;
		SystemTime.wSecond = p->tm_sec;
		SystemTime.wMinute = p->tm_min;
		SystemTime.wHour = p->tm_hour;
		SystemTime.wDay = p->tm_mday;
		SystemTime.wMonth = p->tm_mon + 1;
		SystemTime.wYear = p->tm_year + 1900;

		double ole_date;
		SystemTimeToVariantTime(&SystemTime, &ole_date);

		return ole_date;
	}

	int IsMarketOpen(int64_t time)
	{
		int millisecs = time % 1000;
		time_t secs = time_t(1e-3 * time);
		tm* p = localtime(&secs);
		int t = (p->tm_hour * 3600 + p->tm_min * 60 + p->tm_sec) * 1000 + millisecs;

		for (auto it = TradingHours.begin(); it != TradingHours.end(); ++it)
		{
			if (it->second->trading.find(p->tm_wday) != it->second->trading.end())
			{
				if (t >= it->second->trading[p->tm_wday].first && t < it->second->trading[p->tm_wday].second)
					return 2;
			}
		}

		return 1;
	}

	struct Tick // should be renamed Symbols or SymbolList
	{
		double bid;
		double ask;
		double spread;
		double pip;
		double pipCost;
		double lotAmount;
		double lotAdjustment;
		double lotMax;
		double lotMin;
		double marginCost;
		double rollLong;
		double rollShort;
		double digits;
		int64_t time;

		Tick(const std::string& symbol) 
		{
			std::unique_ptr<SymbolResponse> symbolResponse(new SymbolResponse(ExecuteCommand(ConnectorAPI.get(), SymbolCommand(symbol))));
			bid = symbolResponse->Symbol()->bid;
			ask = symbolResponse->Symbol()->ask;
			spread = symbolResponse->Symbol()->spreadRaw;
			pip = 1 / pow(10, symbolResponse->Symbol()->pipsPrecision);		
			lotAmount = symbolResponse->Symbol()->contractSize * symbolResponse->Symbol()->lotMin;
			lotAdjustment = lotAmount < 1 ? lotAmount : 1.0 / symbolResponse->Symbol()->contractSize; // for Zorro taking int Amount for orders
			lotMin = symbolResponse->Symbol()->lotMin;
			lotMax = symbolResponse->Symbol()->lotMax;
			rollLong = symbolResponse->Symbol()->swapLong;
			rollShort = symbolResponse->Symbol()->swapShort;
			// rollLong and rollShort in base currency and for 100 lots
			digits = symbolResponse->Symbol()->precision;
			time = symbolResponse->Symbol()->time;
			std::unique_ptr<MarginTradeResponse> marginTradeResponse(new MarginTradeResponse(ExecuteCommand(ConnectorAPI.get(), MarginTradeCommand(symbol, 1))));
			marginCost = symbolResponse->Symbol()->lotMin * marginTradeResponse->Margin(); // adjusting to lotAmount
			pipCost = (marginCost * pip * (100 / symbolResponse->Symbol()->leverage)) / (.5 * (bid + ask)); // taking the mid for approximating asset price

			GetTradingHours(symbol);
		}

		void Update(TickRecord* record)
		{
			bid = record->bid;
			ask = record->ask;
			spread = record->spreadRaw;
			time = record->timestamp;
		}
	};
	
	double GetLimitValue(double limit, const std::string& asset)
	{
		if (limit <= 0)
			return 0;

		auto factor = std::pow(10, LastTick.at(asset)->digits);
		return static_cast<int>(factor * limit) / factor;
	}

	double ZorroToXTBVolume(int amount, const std::string& asset)
	{
		double volume = fabs(amount) * LastTick.at(asset)->lotAdjustment;
		return std::floor(100 * volume + 0.5) / 100;
	}

	int XTBToZorroVolume(double volume, const std::string& asset)
	{
		volume = volume / LastTick.at(asset)->lotAdjustment;
		return (int)(std::floor(100 * volume + 0.5) / 100);
	}

	int GetPosition(const std::string& asset)
	{
		double position = 0;

		for (const auto& trade : Trades)
		{
			auto record = trade.second->GetOpened();

			if (record && record->symbol == asset)
			{
				if (record->cmd == OPERATION_CODE::_BUY)
					position += record->volume;
				else if (record->cmd == OPERATION_CODE::_SELL)
					position -= record->volume;
			}
		}

		return XTBToZorroVolume(position, asset);
	}

	double GetAverageEntry(const std::string& asset)
	{
		double averageEntry = 0;
		double totalVolume = 0;

		for (const auto& trade : Trades)
		{
			auto record = trade.second->GetOpened();

			if (record && record->symbol == asset)
			{
				if (record->type == TRANSACTION_TYPE::_OPEN)
				{
					averageEntry += record->open_price * record->volume;
					totalVolume += record->volume;
				}
			}
			
		}

		return totalVolume > 0 ? averageEntry / totalVolume : 0;
	}

	struct Balance
	{
		double balance;
		double tradeVal;
		double marginVal;

		Balance()
		{
			MarginLevelResponse* response = new MarginLevelResponse(ExecuteCommand(ConnectorAPI.get(), MarginLevelCommand()));
			balance = response->Balance()->balance;
			tradeVal = response->Balance()->equity - balance;
			marginVal = response->Balance()->margin;
			delete response;
		}

		void Update(BalanceRecord* record)
		{	    
			balance = record->balance;
			tradeVal = record->equity - balance;
			marginVal = record->margin;
		}
	};

	TradeRecord* GetOpenedTrade(int id)
	{
		std::lock_guard<std::mutex> lock(mtx);
		return Trades.find(id) != Trades.end() ? Trades[id]->GetOpened() : nullptr;
	}

	TradeRecord* GetPendingTrade(int id)
	{
		std::lock_guard<std::mutex> lock(mtx);
		return Trades.find(id) != Trades.end() ? Trades[id]->GetPending() : nullptr;
	}

	TradeRecord* GetClosedTrade(int id)
	{
		//std::lock_guard<std::mutex> lock(mtx);
		return Trades.find(id) != Trades.end() ? Trades[id]->GetClosed() : nullptr;
	}

	void SetOpenedTrade(int id, TradeRecord* record)
	{
		std::lock_guard<std::mutex> lock(mtx);
		if (Trades.find(id) != Trades.end())
			Trades[id]->SetOpened(record);;
	}

	void SetPendingTrade(int id, TradeRecord* record)
	{
		std::lock_guard<std::mutex> lock(mtx);
		if (Trades.find(id) != Trades.end())
			Trades[id]->SetPending(record);
	}

	void SetClosedTrade(int id, TradeRecord* record)
	{
		std::lock_guard<std::mutex> lock(mtx);
		if (Trades.find(id) != Trades.end())
			Trades[id]->SetClosed(record);
	}

	void RemoveOpenedTrade(int id)
	{
		std::lock_guard<std::mutex> lock(mtx);
		if (Trades.find(id) != Trades.end())
			Trades[id]->RemoveOpened();
	}

	void RemovePendingTrade(int id)
	{
		std::lock_guard<std::mutex> lock(mtx);
		if (Trades.find(id) != Trades.end())
			Trades[id]->RemovePending();
	}

	void RemoveClosedTrade(int id)
	{
		std::lock_guard<std::mutex> lock(mtx);
		if (Trades.find(id) != Trades.end())
			Trades[id]->RemoveClosed();
	}

	int GetTradePosition(int id)
	{
		auto record = GetOpenedTrade(id);

		return record != nullptr ? XTBToZorroVolume(record->volume, record->symbol) : 0;
	}

	bool SetTrade(int id)
	{
		if (Trades.find(id) == Trades.end())
		{
			Trades[id].reset(new Trade());
			return true;
		}

		return false;
	}

	void CheckTradeRecord(TradeRecord* record)
	{
		// this method filters orders coming from XTB and keeps the useful ones only

		// if a trade is closed manually, no TradeID will be affected to the opposite direction pending order coming from XTB	
		if (!record->customComment.empty())
		{
			auto id = std::stoi(record->customComment);

			SetTrade(id);

			// deleted order
			if (record->state == "Deleted")
			{
				// opened trade being fully closed 
				if (record->type == TRANSACTION_TYPE::_OPEN)
					Trades[id]->GetOpened()->closed = true;

				// deleted pending order to open a new trade from Zorro or from XTB
				if (record->cmd != OPERATION_CODE::_BUY && record->cmd != OPERATION_CODE::_SELL)
				{
					auto trade = Trades[id]->GetOpened();

					// closing pending order to open a new trade only and not to reduce or close a opened trade
					if (trade->cmd != OPERATION_CODE::_BUY && trade->cmd != OPERATION_CODE::_SELL)
						trade->closed = true;
				}

				// pending order generated by XTB before opening a new trade or closing fully or partially an existing one
				return;
			}

			// closed trade
			if (record->closed)
			{
				Trades[id]->SetClosed(record);
				return;
			}

			auto trade = Trades[id]->GetOpened();
			if (trade)
			{
				if (trade->cmd != record->cmd)
				{
					// limit/stop order to reduce or close the main trade
					if (record->cmd != OPERATION_CODE::_BUY && record->cmd != OPERATION_CODE::_SELL)
					{
						Trades[id]->SetPending(record);
					}

					// limit/stop order having been activated
					if (((trade->cmd == OPERATION_CODE::_BUY_STOP || trade->cmd == OPERATION_CODE::_BUY_LIMIT) && record->cmd == OPERATION_CODE::_BUY) ||
						((trade->cmd == OPERATION_CODE::_SELL_STOP || trade->cmd == OPERATION_CODE::_SELL_LIMIT) && record->cmd == OPERATION_CODE::_SELL))
					{
						Trades[id]->SetOpened(record);
					}

					// opposite direction order generated by XTB to reduce or close the main trade
					return;
				}
			}

			// new opened trade or limit/stop order or pending order to open a new trade
			Trades[id]->SetOpened(record);
		}
	}

	void GetClosedTradesFromXTB()
	{
		std::unique_ptr<TradesResponse> response(new TradesResponse(ExecuteCommand(ConnectorAPI.get(), TradesHistoryCommand(LastConnection))));
		for (auto element : response->Trades())
		{
			if (!element.second->customComment.empty())
			{
				auto id = std::stoi(element.second->customComment);
				SetTrade(id);
				Trades[id]->SetClosed(element.second);
			}
		}
	}

	int GenerateTradeID()
	{
		// collecting old closed trades to avoid re-using the same id for the new ones
		if (!Trades.size())
			GetClosedTradesFromXTB();

		static int tradeID = 1000;

		while (Trades.find(tradeID) != Trades.end())
		{
			tradeID++;
		}

		return tradeID;
	}

	void GetTradesFomXTB()
	{
		// collectign old closed trades
		GetClosedTradesFromXTB();

		// collecting all open trades
		std::unique_ptr<TradesResponse> response(new TradesResponse(ExecuteCommand(ConnectorAPI.get(), TradesCommand(true))));

		std::vector<TradeRecord*> newTrades;

		// starting by collecting opened trades
		for (auto element : response->Trades())
		{
			if (element.second->type != TRANSACTION_TYPE::_OPEN)
				continue;

			if (!element.second->customComment.empty())
			{
				auto id = std::stoi(element.second->customComment);
				SetTrade(id);
				Trades[id]->SetOpened(element.second);
			}
			else
				newTrades.push_back(element.second);
		}

		// then pending trades to open a new trade or reduce/close the main trade
		for (auto element : response->Trades())
		{
			if (element.second->type != TRANSACTION_TYPE::_PENDING)
				continue;

			if (!element.second->customComment.empty())
			{
				auto id = std::stoi(element.second->customComment);

				if (SetTrade(id))
					Trades[id]->SetOpened(element.second);
				else
					Trades[id]->SetPending(element.second);
			}
			else
				newTrades.push_back(element.second);
		}

		// manually opened trades in XTB => will be retrieved from Zorro with brokerTrades method
		// doing it at the end to wait for all trade ids to be retrieved before generating new ones
		for (auto trade : newTrades)
		{
			auto id = GenerateTradeID();
			trade->customComment = std::to_string(id);
			SetTrade(id);
			Trades[id]->SetOpened(trade);
		}
	}

	// called by Zorro method brokerTrades, allows to load all opened trades including the ones opened from the XTB
	// whereas loadStatus will load the ones opened and recorded from Zorro only
	int GetTrades(DWORD value)
	{
		if (!Trades.size())
			GetTradesFomXTB();

		char hex[10];
		sprintf(hex, "%x", value);
		TRADE* trades = (TRADE*)strtoul(hex, NULL, 16);

		int i = 0;

		for (const auto& trade : Trades)
		{
			auto record = trade.second->GetOpened();
			if (record)
			{
				// stream did not start yet, Zorro loading trades
				if (LastTick[record->symbol] == nullptr)
					LastTick[record->symbol].reset(new Tick(record->symbol));

				trades[i].nID = trade.first;
				trades[i].nLots = XTBToZorroVolume(record->volume, record->symbol);
				trades[i].flags = record->cmd == OPERATION_CODE::_BUY || record->cmd == OPERATION_CODE::_BUY_LIMIT  || record->cmd == OPERATION_CODE::_BUY_STOP 
					? (DWORD)TR_OPEN + (DWORD)TR_LONG : (DWORD)TR_OPEN + (DWORD)TR_SHORT;
				trades[i].fEntryPrice = (float)record->open_price;

				i++;
			}
		}

		return i;
	}

	double GetTradeProfit(TradeRecord* trade)
	{
		if (trade->type == TRANSACTION_TYPE::_PENDING)
			return 0;

		if (trade->type == TRANSACTION_TYPE::_CLOSE)
			return trade->profit;

		if (trade->cmd == OPERATION_CODE::_BUY)
			return std::floor(100 * (LastTick[trade->symbol]->bid - trade->open_price) * XTBToZorroVolume(trade->volume, trade->symbol) * LastTick[trade->symbol]->pipCost) / 100;
		else
			return std::floor(100 * (trade->open_price - LastTick[trade->symbol]->ask) * XTBToZorroVolume(trade->volume, trade->symbol) * LastTick[trade->symbol]->pipCost) / 100;
	}

	void CloseStream()
	{
		try
		{		
			if (StreamingIsActive)
			{
				StreamingIsActive = false;
				StreamingAPI->WriteMessage(KeepAliveStop()); // can throw too
				StreamingAPI->Disconnect();
				Log("Streaming is closed.");
			}
		}
		catch (std::exception& ex)
		{
			Log("Exception caught in CloseStream: " + std::string(ex.what()));
		}
	}

	void ReadStream()
	{
		StopStream = false;

		try
		{
			while (!StopStream)
			{
				const char* message = StreamingAPI->ReadMessage("\n\n");

				if (message)
				{
					try
					{
						rapidjson::Document response;
						ParseJSON(response, message);

						if (!response["command"].IsNull())
						{
							const char* command = response["command"].GetString();

							if (!strcmp(command, "tickPrices"))
							{
								TickRecord* record = new TickRecord(response["data"]);
								std::lock_guard<std::mutex> lock(mtx);
								LastTick[record->symbol]->Update(record);
								delete record;
							}
							else if (!strcmp(command, "balance"))
							{
								BalanceRecord* record = new BalanceRecord(response["data"]);
								std::lock_guard<std::mutex> lock(mtx);
								LastBalance->Update(record);
								delete record;
							}
							else if (!strcmp(command, "trade"))
							{
								TradeRecord* record = new TradeRecord(response["data"]);
								std::lock_guard<std::mutex> lock(mtx);
								CheckTradeRecord(record);
							}
						}
					}
					catch (std::exception& ex)
					{
						Log("Exception caught in ReadStream: " + std::string(ex.what()));
					}
				}
			}

			CloseStream();
		}
		catch (std::exception& ex)
		{
			StreamingIsActive = false;
			Log("Streaming is closed.");

			Log("Exception caught in ReadStream: " + std::string(ex.what()));
		}
	}

	void StartStream(void (*func)())
	{
		StreamingAPI->Connect();
		StreamingAPI->WriteMessage(KeepAliveSubscribe(StreamSessionId));
		StreamingIsActive = true;
		Log("Streaming is active.");

		std::thread t(func);
		t.detach();

		LastBalance.reset(new Balance());
		StreamingAPI->WriteMessage(BalanceSubscribe(StreamSessionId));
		StreamingAPI->WriteMessage(TradesSubscribe(StreamSessionId));
	}

	void LoginToServer()
	{
		ConnectorAPI->Connect();
		std::unique_ptr<LoginResponse> response(new LoginResponse(ExecuteCommand(ConnectorAPI.get(), LoginCommand(Username, Password))));
		StreamSessionId = response->StreamSessionId();

		Log("Log in to server completed!");
	}

	void LogoutFromServer()
	{
		StopStream = true;

		while (StreamingIsActive)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		if (ConnectorAPI->Connected())
		{
			ConnectorAPI->WriteMessage(LogoutCommand());
			ConnectorAPI->Disconnect();
		}

		TradingHours.clear();
		LastTick.clear();
		Trades.clear();

		Log("Log out from server completed!");
	}

	int SendOrderToXTB(TradeTransInfo* info)
	{
		try 
		{
			auto output = ExecuteCommand(ConnectorAPI.get(), TradeTransactionCommand(info));
			std::unique_ptr<TradeTransactionResponse> response(new TradeTransactionResponse(output));
			return response->Order();
		} 
		catch (std::exception& ex) 
		{
			// exception caught here to be able to handle such case in BrokerBuy2 and BrokerSell2
			Log("Exception caught in SendOrderToXTB: " + std::string(ex.what()));
			return 0;
		}
	}

	int GetOrderStatus(int order)
	{
		if (order <= 0)
			return 0;

		try
		{
			auto output = ExecuteCommand(ConnectorAPI.get(), TradeTransactionStatusCommand(order));
			std::unique_ptr<TradeTransactionStatusResponse> response(new TradeTransactionStatusResponse(output));
			return response->TradeTransactionStatus()->status;	
		}
		catch (std::exception& ex) 
		{
			// exception caught here to be able to handle such case in BrokerBuy2 and BrokerSell2
			Log("Exception caught in GetOrderStatus: " + std::string(ex.what()));
			return 0;
		}
	}

	int GetNumberOfOpenedTrades()
	{
		auto nb_opened = 0;

		for (const auto& trade : Trades)
		{
			auto record = trade.second->GetOpened();

			if (trade.second->GetOpened())
				nb_opened++;
		}

		return nb_opened;
	}
}
