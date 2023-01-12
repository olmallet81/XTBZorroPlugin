#pragma once

#include <chrono>
#include <cstdint>
#include <iostream>

namespace XTB
{
	static int CommandTimeSpace = 200; // in milliseconds, minimum recommended by XTB

	const char* ExecuteCommand(TCPTools::Connector* connector, const std::string& message)
	{
		static std::chrono::steady_clock::time_point lastCommandTimestamp;

		auto interval = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - lastCommandTimestamp).count();

		// if interval between now and last command is less than minimum command time space - wait
		if (interval < CommandTimeSpace)
			Sleep(CommandTimeSpace - (DWORD)interval);

		connector->WriteMessage(message);
		lastCommandTimestamp = std::chrono::steady_clock::now();

		const char* output = connector->ReadMessage("\n\n");

		if (!output)
			throw std::exception("Error while executing command: the server is currently not responding");

		return output;
	}

	std::string LoginCommand(const std::string& userId, const std::string& password)
	{
		rapidjson::Document args;
		args.SetObject();
		args.AddMember("userId", userId, args.GetAllocator());
		args.AddMember("password", password, args.GetAllocator());

		rapidjson::Document command;
		command.SetObject();
		command.AddMember("command", "login", command.GetAllocator());
		command.AddMember("arguments", args, command.GetAllocator());

		rapidjson::StringBuffer command_s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(command_s);
		command.Accept(writer);

		return command_s.GetString();
	}

	std::string LogoutCommand()
	{
		rapidjson::Document command;
		command.SetObject();
		command.AddMember("command", "logout", command.GetAllocator());

		rapidjson::StringBuffer command_s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(command_s);
		command.Accept(writer);

		return command_s.GetString();
	}

	std::string PingCommand()
	{
		rapidjson::Document command;
		command.SetObject();
		command.AddMember("command", "ping", command.GetAllocator());

		rapidjson::StringBuffer command_s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(command_s);
		command.Accept(writer);

		return command_s.GetString();
	}

	std::string ServerTimeCommand()
	{
		rapidjson::Document command;
		command.SetObject();
		command.AddMember("command", "getServerTime", command.GetAllocator());

		rapidjson::StringBuffer command_s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(command_s);
		command.Accept(writer);

		return command_s.GetString();
	}

	std::string SymbolCommand(const std::string& symbol)
	{
		rapidjson::Document args;
		args.SetObject();
		args.AddMember("symbol", symbol, args.GetAllocator());

		rapidjson::Document command;
		command.SetObject();
		command.AddMember("command", "getSymbol", command.GetAllocator());
		command.AddMember("arguments", args, command.GetAllocator());

		rapidjson::StringBuffer command_s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(command_s);
		command.Accept(writer);

		return command_s.GetString();
	}

	std::string TickPricesCommand(const std::string& symbol, int64_t timestamp)
	{
		rapidjson::Document args;
		args.SetObject();
		args.AddMember("level", 0, args.GetAllocator());
		args.AddMember("symbols", symbol, args.GetAllocator());
		args.AddMember("timestamp", timestamp, args.GetAllocator());

		rapidjson::Document command;
		command.SetObject();
		command.AddMember("command", "getTickPrices", command.GetAllocator());
		command.AddMember("arguments", args, command.GetAllocator());

		rapidjson::StringBuffer command_s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(command_s);
		command.Accept(writer);

		return command_s.GetString();
	}

	std::string TradingHoursCommand(const std::string& symbol)
	{
		rapidjson::Document args;
		args.SetObject();
		args.AddMember("symbols", symbol, args.GetAllocator());

		rapidjson::Document command;
		command.SetObject();
		command.AddMember("command", "getTradingHours", command.GetAllocator());
		command.AddMember("arguments", args, command.GetAllocator());

		rapidjson::StringBuffer command_s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(command_s);
		command.Accept(writer);

		return command_s.GetString();
	}

	std::string MarginLevelCommand()
	{
		rapidjson::Document command;
		command.SetObject();
		command.AddMember("command", "getMarginLevel", command.GetAllocator());

		rapidjson::StringBuffer command_s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(command_s);
		command.Accept(writer);

		return command_s.GetString();
	}

	std::string MarginTradeCommand(const std::string& symbol, double volume)
	{
		rapidjson::Document args;
		args.SetObject();
		args.AddMember("symbol", symbol, args.GetAllocator());
		args.AddMember("volume", volume, args.GetAllocator());

		rapidjson::Document command;
		command.SetObject();
		command.AddMember("command", "getMarginTrade", command.GetAllocator());
		command.AddMember("arguments", args, command.GetAllocator());

		rapidjson::StringBuffer command_s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(command_s);
		command.Accept(writer);

		return command_s.GetString();
	}

	struct TradeTransInfo
	{
		OPERATION_CODE::OPERATION_CODE cmd;
		std::string customComment;
		int64_t expiration = 0;
		int offset = 0;
		int order = 0;
		double price;
		double sl = 0.0;
		std::string symbol;
		double tp = 0.0;
		TRANSACTION_TYPE::TRANSACTION_TYPE type;
		double volume = 0.0;

		TradeTransInfo()
		{
		}

		TradeTransInfo(TradeRecord* record)
		{
			cmd = record->cmd;
			customComment = record->customComment;
			expiration = record->expiration;
			offset = record->offset;
			order = record->order;
			price = record->open_price;
			sl = record->sl;
			symbol = record->symbol;
			tp = record->tp;
			type = record->type;
			volume = record->volume;
		}
	};

	std::string TradesCommand(bool openedOnly)
	{
		rapidjson::Document args;
		args.SetObject();
		args.AddMember("openedOnly", openedOnly, args.GetAllocator());

		rapidjson::Document command;
		command.SetObject();
		command.AddMember("command", "getTrades", command.GetAllocator());
		command.AddMember("arguments", args, command.GetAllocator());

		rapidjson::StringBuffer command_s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(command_s);
		command.Accept(writer);

		return command_s.GetString();
	}

	std::string TradesHistoryCommand(int64_t start = 0, int64_t end = 0)
	{
		rapidjson::Document args;
		args.SetObject();
		args.AddMember("start", start, args.GetAllocator());
		args.AddMember("end", end, args.GetAllocator());

		rapidjson::Document command;
		command.SetObject();
		command.AddMember("command", "getTradesHistory", command.GetAllocator());
		command.AddMember("arguments", args, command.GetAllocator());

		rapidjson::StringBuffer command_s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(command_s);
		command.Accept(writer);

		return command_s.GetString();
	}

	std::string TradeRecordsCommand(int order)
	{
		rapidjson::Document args;
		args.SetObject();
		args.AddMember("orders", order, args.GetAllocator());

		rapidjson::Document command;
		command.SetObject();
		command.AddMember("command", "getTradeRecords", command.GetAllocator());
		command.AddMember("arguments", args, command.GetAllocator());

		rapidjson::StringBuffer command_s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(command_s);
		command.Accept(writer);

		return command_s.GetString();
	}

	std::string TradeTransactionCommand(TradeTransInfo* trade)
	{
		rapidjson::Document info;
		info.SetObject();
		info.AddMember("cmd", trade->cmd, info.GetAllocator());
		info.AddMember("customComment", trade->customComment, info.GetAllocator());
		info.AddMember("expiration", trade->expiration, info.GetAllocator());
		info.AddMember("offset", trade->offset, info.GetAllocator());
		info.AddMember("order", trade->order, info.GetAllocator());
		info.AddMember("price", trade->price, info.GetAllocator());
		info.AddMember("sl", trade->sl, info.GetAllocator());
		info.AddMember("symbol", trade->symbol, info.GetAllocator());
		info.AddMember("tp", trade->tp, info.GetAllocator());
		info.AddMember("type", trade->type, info.GetAllocator());
		info.AddMember("volume", trade->volume, info.GetAllocator());
		
		rapidjson::Document args;
		args.SetObject();
		args.AddMember("tradeTransInfo", info, args.GetAllocator());

		rapidjson::Document command;
		command.SetObject();
		command.AddMember("command", "tradeTransaction", command.GetAllocator());
		command.AddMember("arguments", args, command.GetAllocator());

		rapidjson::StringBuffer command_s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(command_s);
		command.Accept(writer);

		return command_s.GetString();
	}

	std::string TradeTransactionStatusCommand(int order)
	{
		rapidjson::Document args;
		args.SetObject();
		args.AddMember("order", order, args.GetAllocator());

		rapidjson::Document command;
		command.SetObject();
		command.AddMember("command", "tradeTransactionStatus", command.GetAllocator());
		command.AddMember("arguments", args, command.GetAllocator());

		rapidjson::StringBuffer command_s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(command_s);
		command.Accept(writer);

		return command_s.GetString();
	}
}
