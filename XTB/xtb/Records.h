#pragma once

#include <map>

namespace XTB
{
	void GetString(const rapidjson::Value& value, const std::string& name, std::string& result)
	{
		if (!value.HasMember(name))
			throw std::exception(("Missing member '" + name + "' in json string").c_str()); // we should not throw but log a warning instead

		result = value[name].IsNull() ? "" : value[name].GetString();
	}

	template <typename T>
	T GetValue(const rapidjson::Value& value, const std::string& name, T default_value)
	{
		if (!value.HasMember(name))
			throw std::exception(("Missing member '" + name + "' in json string").c_str());

		if (!value[name].IsNull())
		{
			if (std::is_same<T, double>::value)
				return (T)value[name].GetDouble();
			else if (std::is_same<T, int64_t>::value)
				return (T)value[name].GetUint64();
			else if (std::is_same<T, int>::value)
				return (T)value[name].GetInt();
			else if (std::is_same<T, bool>::value)
				return (T)value[name].GetBool();
		}

		return default_value;
	}

	struct BalanceRecord
	{
		double balance;
		double credit;
		std::string currency; // not in streaming
		double equity;
		double margin;
		double margin_free;
		double margin_level;

		BalanceRecord(const rapidjson::Value& value, bool streaming = true)
		{
			balance = GetValue<double>(value, "balance", 0.);
			credit = GetValue<double>(value, "credit", 0.);
			if (!streaming) GetString(value, "currency", currency);
			equity = GetValue<double>(value, "equity", 0.);
			margin = GetValue<double>(value, "margin", 0.);
			margin_free = streaming ? GetValue<double>(value, "marginFree", 0.) : GetValue<double>(value, "margin_free", 0.);
			margin_level = streaming ? GetValue<double>(value, "marginLevel", 0.)  : GetValue<double>(value, "margin_level", 0.);
		}
	};

	struct QuotesRecord
	{
		int day;
		int fromT;
		int toT;

		QuotesRecord(const rapidjson::Value& value)
		{
			day = GetValue<int>(value, "day", 0);
			fromT = GetValue<int>(value, "fromT", 0);
			toT = GetValue<int>(value, "toT", 0);
		}
	};

	struct TradingRecord
	{
		int day;
		int fromT;
		int toT;

		TradingRecord(const rapidjson::Value& value)
		{
			day = GetValue<int>(value, "day", 0);
			fromT = GetValue<int>(value, "fromT", 0);
			toT = GetValue<int>(value, "toT", 0);
		}
	};

	struct SymbolRecord
	{
		double ask;
		double bid;
		std::string categoryName;
		int contractSize;
		std::string currency;
		bool currencyPair;
		std::string currencyProfit;
		std::string description;
		int64_t expiration;
		std::string groupName;
		double high;
		int initialMargin;
		int64_t instantMaxVolume;
		double leverage;
		bool longOnly;
		double lotMax;
		double lotMin;
		double lotStep;
		double low;
		int marginHedged;
		bool marginHedgedStrong;
		int marginMaintenance;
		int marginMode;
		double percentage;
		int pipsPrecision;
		int precision;
		int profitMode;
		int quoteId;
		bool shortSelling;
		double spreadRaw;
		double spreadTable;
		int64_t starting;
		int stepRuleId;
		int stopsLevel;
		int swap_rollover3days;
		bool swapEnable;
		double swapLong;
		double swapShort;
		int swapType;
		std::string symbol;
		double tickSize;
		double tickValue;
		int64_t time;
		std::string timeString;
		bool trailingEnable;
		int type;

		SymbolRecord(const rapidjson::Value& value)
		{
			ask = GetValue<double>(value, "ask", 0.);
			bid = GetValue<double>(value, "bid", 0.);
			GetString(value, "categoryName", categoryName);
			contractSize = GetValue<int>(value, "contractSize", 0);
			GetString(value, "currency", currency);
			currencyPair = GetValue<bool>(value, "currencyPair", false);
			bid = GetValue<double>(value, "bid", 0.);
			GetString(value, "currencyProfit", currencyProfit);
			GetString(value, "description", description);
			expiration = GetValue<int>(value, "expiration", 0);
			GetString(value, "groupName", groupName);
			high = GetValue<double>(value, "high", 0.);
			initialMargin = GetValue<int>(value, "initialMargin", 0);
			instantMaxVolume = GetValue<int64_t>(value, "instantMaxVolume", 0);
			leverage = GetValue<double>(value, "leverage", 0.);
			longOnly = GetValue<bool>(value, "longOnly", false);
			lotMax = GetValue<double>(value, "lotMax", 0.);
			lotMin = GetValue<double>(value, "lotMin", 0.);
			lotStep = GetValue<double>(value, "lotStep", 0.);
			low = GetValue<double>(value, "low", 0.);
			marginHedged = GetValue<int>(value, "marginHedged", 0);
			marginHedgedStrong = GetValue<bool>(value, "marginHedgedStrong", false);
			marginMaintenance = GetValue<int>(value, "marginMaintenance", 0);
			marginMode = GetValue<int>(value, "marginMode", 0);
			percentage = GetValue<double>(value, "percentage", 0.);
			pipsPrecision = GetValue<int>(value, "pipsPrecision", 0);
			precision = GetValue<int>(value, "precision", 0);
			profitMode = GetValue<int>(value, "profitMode", 0);
			quoteId = GetValue<int>(value, "quoteId", 0);
			shortSelling = GetValue<bool>(value, "shortSelling", false);
			spreadRaw = GetValue<double>(value, "spreadRaw", 0.);
			spreadTable = GetValue<double>(value, "spreadTable", 0.);
			starting = GetValue<int64_t>(value, "starting", 0);
			stepRuleId = GetValue<int>(value, "stepRuleId", 0);
			stopsLevel = GetValue<int>(value, "stopsLevel", 0);
			swap_rollover3days = GetValue<int>(value, "swap_rollover3days", 0);
			swapEnable = GetValue<bool>(value, "swapEnable", false);
			swapLong = GetValue<double>(value, "swapLong", 0.);
			swapShort = GetValue<double>(value, "swapShort", 0.);
			swapType = GetValue<int>(value, "swapType", 0);
			GetString(value, "symbol", symbol);
			tickSize = GetValue<double>(value, "tickSize", 0.);
			tickValue = GetValue<double>(value, "tickValue", 0.);
			time = GetValue<int64_t>(value, "time", 0);
			GetString(value, "timeString", timeString);
			type = GetValue<int>(value, "type", 0);
		}
	};

	struct TickRecord
	{
		double ask;
		int askVolume;
		double bid;
		int bidVolume;
		double high;
		long level;
		double low;
		int quoteId; // streaming only
		double spreadRaw;
		double spreadTable;
		std::string symbol;
		int64_t timestamp;

		TickRecord(const rapidjson::Value& value)
		{
			ask = GetValue<double>(value, "ask", 0.);
			askVolume = GetValue<int>(value, "askVolume", 0);
			bid = GetValue<double>(value, "bid", 0.);
			bidVolume = GetValue<int>(value, "bidVolume", 0);
			high = GetValue<double>(value, "high", 0.);
			level = GetValue<int>(value, "level", 0);
			low = GetValue<double>(value, "low", 0.);
			quoteId = GetValue<int>(value, "quoteId", 0);
			spreadRaw = GetValue<double>(value, "spreadRaw", 0.);
			spreadTable = GetValue<double>(value, "spreadTable", 0.);
			GetString(value, "symbol", symbol);
			timestamp = GetValue<int64_t>(value, "timestamp", (int64_t)0);
		}
	};

	namespace OPERATION_CODE
	{
		enum OPERATION_CODE
		{
			_BUY,
			_SELL,
			_BUY_LIMIT,
			_SELL_LIMIT,
			_BUY_STOP,
			_SELL_STOP,
			_BALANCE,
			_CREDIT,
			_UNKNOWN
		};
	}

	std::string OperationCode(OPERATION_CODE::OPERATION_CODE cmd)
	{
		switch (cmd)
		{
		case OPERATION_CODE::_BUY:
			return "BUY";
		case OPERATION_CODE::_SELL:
			return "SELL";
		case OPERATION_CODE::_BUY_LIMIT:
			return "BUY_LIMIT";
		case OPERATION_CODE::_SELL_LIMIT:
			return "SELL_LIMIT";
		case OPERATION_CODE::_BUY_STOP:
			return "BUY_STOP";
		case OPERATION_CODE::_SELL_STOP:
			return "SELL_STOP";
		case OPERATION_CODE::_BALANCE:
			return "BALANCE";
		case OPERATION_CODE::_CREDIT:
			return "CREDIT";
		default:
			return "UNKNOWN";
		}
	}

	namespace REQUEST_STATUS
	{
		enum REQUEST_STATUS
		{
			_ERROR = 0,
			_PENDING = 1,
			_ACCEPTED = 3,
			_REJECTED = 4,
			_UNKNOWN = 5
		};
	}

	std::string RequestStatus(REQUEST_STATUS::REQUEST_STATUS status)
	{
		switch (status)
		{
			case REQUEST_STATUS::_ERROR:
				return "ERROR";
			case REQUEST_STATUS::_PENDING:
				return "PENDING";
			case REQUEST_STATUS::_ACCEPTED:
				return "ACCEPTED";
			case REQUEST_STATUS::_REJECTED:
				return "REJECTED";
			default:
				return "UNKNOWN";
		}
	}

	namespace TRANSACTION_TYPE
	{
		enum TRANSACTION_TYPE
		{
			_OPEN,
			_PENDING,
			_CLOSE,
			_MODIFY,
			_DELETE,
			_UNKNOWN
		};
	}

	std::string TransactionType(TRANSACTION_TYPE::TRANSACTION_TYPE type)
	{
		switch (type)
		{
		case TRANSACTION_TYPE::_OPEN:
			return "OPEN";
		case TRANSACTION_TYPE::_PENDING:
			return "PENDING";
		case TRANSACTION_TYPE::_CLOSE:
			return "CLOSE";
		case TRANSACTION_TYPE::_MODIFY:
			return "MODIFY";
		case TRANSACTION_TYPE::_DELETE:
			return "DELETE";
		default:
			return "UNKNOWN";
		}
	}

	struct TradeRecord
	{
		double close_price;
		int64_t close_time;
		bool closed;
		OPERATION_CODE::OPERATION_CODE cmd;
		double commission;
		std::string customComment;
		int digits;
		int64_t expiration;
		double margin_rate;
		int offset;
		double open_price;
		int64_t open_time;
		int order;
		int order2; 
		int position; // position number (if type is OPEN and CLOSE) or transaction parameter (if type is PENDING)
		double profit;
		double sl;
		std::string state; // streaming only
		double storage;  
		std::string symbol;
		double tp;
		TRANSACTION_TYPE::TRANSACTION_TYPE type;  // streaming only
		double volume;

		TradeRecord(const rapidjson::Value& value, bool streaming = true)
		{
			close_price = GetValue<double>(value, "close_price", 0.);
			close_time = GetValue<int64_t>(value, "close_time", 0);
			closed = GetValue<bool>(value, "closed", false);
			cmd = (OPERATION_CODE::OPERATION_CODE)GetValue<int>(value, "cmd", (int)OPERATION_CODE::OPERATION_CODE::_UNKNOWN);
			commission = GetValue<double>(value, "commission", 0.);
			GetString(value, "customComment", customComment);
			digits = GetValue<int>(value, "digits", 0);
			expiration = GetValue<int64_t>(value, "expiration", 0);
			margin_rate = GetValue<double>(value, "margin_rate", 0.);
			offset = GetValue<int>(value, "offset", 0);
			open_price = GetValue<double>(value, "open_price", 0.);
			open_time = GetValue<int64_t>(value, "open_time", 0);
			order = GetValue<int>(value, "order", 0);
			order2 = GetValue<int>(value, "order2", 0);
			position = GetValue<int>(value, "position", 0);
			profit = GetValue<double>(value, "profit", 0.);
			sl = GetValue<double>(value, "sl", 0.);
			if (streaming) GetString(value, "state", state);
			storage = GetValue<double>(value, "storage", 0.);
			GetString(value, "symbol", symbol);
			tp = GetValue<double>(value, "tp", 0.);

			if (streaming)
				type = (TRANSACTION_TYPE::TRANSACTION_TYPE)GetValue<int>(value, "type", (int)TRANSACTION_TYPE::TRANSACTION_TYPE::_UNKNOWN);
			else
			{
				if (closed == true)
					type = TRANSACTION_TYPE::TRANSACTION_TYPE::_CLOSE;
				else if (cmd == OPERATION_CODE::OPERATION_CODE::_BUY_LIMIT || cmd == OPERATION_CODE::OPERATION_CODE::_SELL_LIMIT || cmd == OPERATION_CODE::OPERATION_CODE::_BUY_STOP || cmd == OPERATION_CODE::OPERATION_CODE::_SELL_STOP)
					type = TRANSACTION_TYPE::TRANSACTION_TYPE::_PENDING;
				else if (cmd == OPERATION_CODE::OPERATION_CODE::_BUY || cmd == OPERATION_CODE::OPERATION_CODE::_SELL)
					if (order != order2)
						type = TRANSACTION_TYPE::TRANSACTION_TYPE::_OPEN;
					else
						type = TRANSACTION_TYPE::TRANSACTION_TYPE::_PENDING;
				else
					type = TRANSACTION_TYPE::TRANSACTION_TYPE::_UNKNOWN;
			}

			volume = GetValue<double>(value, "volume", 0.);
		}

		std::string ToString()
		{
			std::stringstream ss;

			ss << "TRADE_RECORD" << '\n'
				<< "close_price: " << close_price << '\n'
				<< "closed: " << closed << '\n'
				<< "cmd: " << OperationCode(cmd) << '\n'
				<< "customComment: " << customComment << '\n'
				<< "open_price: " << open_price << '\n'
				<< "order: " << order << '\n'
				<< "order2: " << order2 << '\n'
				<< "position: " << position << '\n'
				<< "profit: " << profit << '\n'
				<< "state: " << state << '\n'
				<< "symbol: " << symbol << '\n'
				<< "type: " << TransactionType(type) << '\n'
				<< "volume: " << volume << '\n';

			return ss.str();
		}
	};

	struct TradeStatusRecord
	{
		using Request = REQUEST_STATUS::REQUEST_STATUS;

		double ask; // not in streaming
		double bid; // not in streaming
		std::string customComment;
		int order;
		double price; // streaming only
		Request status;

		TradeStatusRecord(const rapidjson::Value& value, bool streaming = true)
		{
			if (!streaming) ask = GetValue<double>(value, "ask", 0);
			if (!streaming) bid = GetValue<double>(value, "bid", 0.);
			GetString(value, "customComment", customComment);
			order = GetValue<int>(value, "order", 0);
			if (streaming) price = GetValue<double>(value, "price", 0.);
			status = (Request)GetValue<int>(value, "requestStatus", (int)Request::_UNKNOWN);
		}

		std::string ToString()
		{
			std::stringstream ss;

			ss << "TRADE_STATUS_RECORD" << '\n'
				<< "customComment: " << customComment << '\n'
				<< "order: " << order << '\n'
				<< "requestStatus: " << RequestStatus(status) << '\n';

			return ss.str();
		}
	};

	struct TradingHoursRecord
	{
		std::map<int, std::pair<int, int>> quotes;
		std::string symbol;
		std::map<int, std::pair<int, int>> trading;

		TradingHoursRecord(const rapidjson::Value& value)
		{
			const auto& quotesData = value["quotes"].GetArray();

			for (auto it = quotesData.Begin(); it != quotesData.End(); ++it)
			{
				QuotesRecord* record = new QuotesRecord(*it);
				record->day = record->day == 7 ? 0 : record->day;
				quotes[record->day] = std::make_pair(record->fromT, record->toT);
				delete record;
			}

			GetString(value, "symbol", symbol);

			const auto& tradingData = value["trading"].GetArray();

			for (auto it = tradingData.Begin(); it != tradingData.End(); ++it)
			{
				TradingRecord* record = new TradingRecord(*it);
				record->day = record->day == 7 ? 0 : record->day;
				trading[record->day] = std::make_pair(record->fromT, record->toT);
				delete record;
			}
		}
	};
}