#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>

#include "Records.h"

namespace XTB
{
	class Trade
	{
	private:
		TradeRecord* opened;  // for opened trades or limit/stop orders to open a new one
		TradeRecord* pending; // for limit/stop orders to close fully or partially an opened trade
		TradeRecord* closed;  // for closed trades 

	public:

		~Trade()
		{
			delete opened;
			delete pending;
			delete closed;
		}

		void SetOpened(TradeRecord* record)
		{
			delete opened;
			opened = record;
		}

		void SetPending(TradeRecord* record)
		{
			delete pending;
			pending = record;
		}

		void SetClosed(TradeRecord* record)
		{
			delete closed;
			closed = record;
		}

		TradeRecord* GetOpened()
		{
			return opened;
		}

		TradeRecord* GetPending()
		{
			return pending;
		}

		TradeRecord* GetClosed()
		{
			return closed;
		}

		void RemoveOpened()
		{
			delete opened;
			opened = nullptr;
		}

		void RemovePending()
		{
			delete pending;
			pending = nullptr;
		}

		void RemoveClosed()
		{
			delete closed;
			closed = nullptr;
		}
	};
}
