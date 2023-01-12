#pragma once

namespace XTB
{
	void ParseJSON(rapidjson::Document& response, const char* message)
	{
		if (response.Parse(message).HasParseError())
		{
			delete message;
			throw std::exception(("Error while parsing response: " + std::string(rapidjson::GetParseError_En(response.GetParseError()))).c_str());
		}

		delete message;
	}

	class BaseResponse
	{
	protected:
		rapidjson::Document response;

	public:
		BaseResponse(const char* message, const char* value)
		{
			ParseJSON(response, message);

			if (response["status"].IsNull())
				throw std::exception("Error: The server response does not containt the status.");
			
			if (!response["status"].GetBool())
				throw std::exception(("Error: "  + std::string(response["errorDescr"].GetString())).c_str());
				
			if (response[value].IsNull())
				throw std::exception(("Error: Could not find " + std::string(value) + " in the server response.").c_str());
		}
	};

	class LoginResponse : public BaseResponse
	{
	private:
		std::string _streamSessionId;

	public:
		LoginResponse(const char* message) : BaseResponse(message, "streamSessionId")
		{
			_streamSessionId = response["streamSessionId"].GetString();
		}

		const std::string& StreamSessionId() const
		{
			return _streamSessionId;
		}
	};

	class MarginLevelResponse : public BaseResponse
	{
	private:
		std::shared_ptr<BalanceRecord> _balance;

	public:
		MarginLevelResponse(const char* message, bool streaming = false) : BaseResponse(message, "returnData")
		{
			_balance.reset(new BalanceRecord(response["returnData"], streaming));
		}

		const std::shared_ptr<BalanceRecord>& Balance() const
		{
			return _balance;
		}
	};

	class MarginTradeResponse : public BaseResponse
	{
	private:
		double _margin;

	public:
		MarginTradeResponse(const char* message) : BaseResponse(message, "returnData")
		{
			const rapidjson::Value& value = response["returnData"];
			_margin = value["margin"].GetDouble();
		}

		double Margin() const
		{
			return _margin;
		}
	};

	class ServerTimeResponse : public BaseResponse
	{
	private:
		int64_t _time;
		std::string _timeString;

	public:
		ServerTimeResponse(const char* message) : BaseResponse(message, "returnData")
		{
			const rapidjson::Value& value = response["returnData"];
			_time = value["time"].GetInt64();
			_timeString = value["timeString"].GetString();
		}

		int64_t Time() const
		{
			return _time;
		}

		const std::string& TimeString() const
		{
			return _timeString;
		}
	};

	class SymbolResponse : public BaseResponse
	{
	private:
		std::shared_ptr<SymbolRecord> _symbol;

	public:
		SymbolResponse(const char* message) : BaseResponse(message, "returnData")
		{
			_symbol.reset(new SymbolRecord(response["returnData"]));
		}

		const std::shared_ptr<SymbolRecord>& Symbol() const
		{
			return _symbol;
		}
	};

	class TradingHoursResponse : public BaseResponse
	{
	private:
		std::map<std::string, std::shared_ptr<TradingHoursRecord>> _tradingHours;

	public:
		TradingHoursResponse(const char* message) : BaseResponse(message, "returnData")
		{
			const rapidjson::Value& data = response["returnData"].GetArray();

			for (auto it = data.Begin(); it != data.End(); ++it)
			{
				TradingHoursRecord* record = new TradingHoursRecord(*it);
				_tradingHours[record->symbol].reset(record);
			}
		}

		const std::map<std::string, std::shared_ptr<TradingHoursRecord>>& TradingHours() const
		{
			return _tradingHours;
		}
	};

	class TradesResponse : public BaseResponse
	{
	private:
		std::map<int, TradeRecord*> _trades;

	public:
		TradesResponse(const char* message, bool streaming = false) : BaseResponse(message, "returnData")
		{
			const rapidjson::Value& data = response["returnData"].GetArray();

			for (auto it = data.Begin(); it != data.End(); ++it)
			{
				TradeRecord* record = new TradeRecord(*it, streaming);
				_trades[record->position] = record;
			}	
		}

		const std::map<int, TradeRecord*>& Trades() const
		{
			return _trades;
		}
	};

	class TradeTransactionResponse : public BaseResponse
	{
	private:
		int _order;

	public:
		TradeTransactionResponse(const char* message) : BaseResponse(message, "returnData")
		{
			const rapidjson::Value& data = response["returnData"];
			_order = data["order"].GetInt();
		}

		int Order() const
		{
			return _order;
		}
	};

	class TradeTransactionStatusResponse : public BaseResponse
	{
	private:
		std::shared_ptr<TradeStatusRecord> _tradeTransactionStatus;

	public:
		TradeTransactionStatusResponse(const char* message, bool streaming = false) : BaseResponse(message, "returnData")
		{
			_tradeTransactionStatus.reset(new TradeStatusRecord(response["returnData"], streaming));
		}

		const std::shared_ptr<TradeStatusRecord>& TradeTransactionStatus() const
		{
			return _tradeTransactionStatus;
		}
	};
}
