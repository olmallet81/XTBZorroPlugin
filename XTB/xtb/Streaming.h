#pragma once

#include <map>

namespace XTB
{
	std::string BalanceSubscribe(const std::string& streamSessionId)
	{
		rapidjson::Document command;
		command.SetObject();

		rapidjson::Document::AllocatorType& allocator = command.GetAllocator();

		command.AddMember("command", "getBalance", allocator);
		command.AddMember("streamSessionId", streamSessionId, allocator);

		rapidjson::StringBuffer command_s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(command_s);
		command.Accept(writer);

		return command_s.GetString();
	}

	std::string BalanceStop()
	{
		rapidjson::Document command;
		command.SetObject();

		rapidjson::Document::AllocatorType& allocator = command.GetAllocator();

		command.AddMember("command", "stopBalance", allocator);

		rapidjson::StringBuffer command_s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(command_s);
		command.Accept(writer);

		return command_s.GetString();
	}

	std::string TickPricesSubscribe(const std::string& symbol, const std::string& streamSessionId)
	{
		rapidjson::Document command;
		command.SetObject();

		rapidjson::Document::AllocatorType& allocator = command.GetAllocator();

		command.AddMember("command", "getTickPrices", allocator);
		command.AddMember("streamSessionId", streamSessionId, allocator);
		command.AddMember("symbol", symbol, allocator);
		command.AddMember("minArrivalTime", 1, allocator);
		command.AddMember("maxLevel", 0, allocator);

		rapidjson::StringBuffer command_s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(command_s);
		command.Accept(writer);

		return command_s.GetString();
	}

	std::string TickPricesStop(const std::string& symbol)
	{
		rapidjson::Document command;
		command.SetObject();

		rapidjson::Document::AllocatorType& allocator = command.GetAllocator();

		command.AddMember("command", "stopTickPrices", allocator);
		command.AddMember("symbol", symbol, allocator);

		rapidjson::StringBuffer command_s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(command_s);
		command.Accept(writer);

		return command_s.GetString();
	}

	std::string TradesSubscribe(const std::string& streamSessionId)
	{
		rapidjson::Document command;
		command.SetObject();

		rapidjson::Document::AllocatorType& allocator = command.GetAllocator();

		command.AddMember("command", "getTrades", allocator);
		command.AddMember("streamSessionId", streamSessionId, allocator);

		rapidjson::StringBuffer command_s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(command_s);
		command.Accept(writer);

		return command_s.GetString();
	}

	std::string TradesStop(const std::string& symbol)
	{
		rapidjson::Document command;
		command.SetObject();

		rapidjson::Document::AllocatorType& allocator = command.GetAllocator();

		command.AddMember("command", "stopTrades", allocator);

		rapidjson::StringBuffer command_s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(command_s);
		command.Accept(writer);

		return command_s.GetString();
	}

	std::string TradeStatusSubscribe(const std::string& streamSessionId)
	{
		rapidjson::Document command;
		command.SetObject();

		rapidjson::Document::AllocatorType& allocator = command.GetAllocator();

		command.AddMember("command", "getTradeStatus", allocator);
		command.AddMember("streamSessionId", streamSessionId, allocator);

		rapidjson::StringBuffer command_s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(command_s);
		command.Accept(writer);

		return command_s.GetString();
	}

	std::string TradeStatusStop(const std::string& symbol)
	{
		rapidjson::Document command;
		command.SetObject();

		rapidjson::Document::AllocatorType& allocator = command.GetAllocator();

		command.AddMember("command", "stopTradeStatus", allocator);

		rapidjson::StringBuffer command_s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(command_s);
		command.Accept(writer);

		return command_s.GetString();
	}

	std::string StreamingPing(const std::string& streamSessionId)
	{
		rapidjson::Document command;
		command.SetObject();

		rapidjson::Document::AllocatorType& allocator = command.GetAllocator();

		command.AddMember("command", "ping", allocator);
		command.AddMember("streamSessionId", streamSessionId, allocator);

		rapidjson::StringBuffer command_s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(command_s);
		command.Accept(writer);

		return command_s.GetString();
	}

	std::string KeepAliveSubscribe(const std::string& streamSessionId)
	{
		rapidjson::Document command;
		command.SetObject();

		rapidjson::Document::AllocatorType& allocator = command.GetAllocator();

		command.AddMember("command", "getKeepAlive", allocator);
		command.AddMember("streamSessionId", streamSessionId, allocator);

		rapidjson::StringBuffer command_s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(command_s);
		command.Accept(writer);

		return command_s.GetString();
	}

	std::string KeepAliveStop()
	{
		rapidjson::Document command;
		command.SetObject();

		rapidjson::Document::AllocatorType& allocator = command.GetAllocator();

		command.AddMember("command", "spotKeepAlive", allocator);

		rapidjson::StringBuffer command_s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(command_s);
		command.Accept(writer);

		return command_s.GetString();
	}
}