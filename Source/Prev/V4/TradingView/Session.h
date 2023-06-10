#pragma once
#include "ComIDManager.h"
#include "Inbox.h"
#include "Outbox.h"
#include "Log.h"

namespace TradingView {

class Session
{
public:
	Session(std::string stockMarket, std::string stockTicker, std::string timeInterval, std::string timeFrame, unsigned int ID=-1, bool replay=false, bool recordMessages=false);
	~Session(void);

	void Start(void);
	void LogResults(void);
	bool IsThreaded(void) { return m_IsThreaded; };
	bool Aborted(void) { return m_Aborted; };

	void SetIsThreaded(bool isThreaded) { m_IsThreaded = isThreaded; };

	TradingView::Inbox& GetInbox(void) { return m_Inbox; };
	TradingView::DataPointManager& GetDataPointManager(void) { return m_DataPointManager; };
	TradingView::Log& GetLog(void) { return m_Log; };

	std::string GetStockTicker(void) { return m_StockTicker; };
	bool GetHasReplay(void) { return m_HasReplay; };
	unsigned int GetExecutionTime(void) { return m_ExecutionTime; };

private:
	bool RequestInitialData(void);
	bool ExtendDataWithReplay(void);
	void Abort(void);

private:
	TradingView::Log m_Log;
	TradingView::TVIO m_TVIO;
	TradingView::DataPointManager m_DataPointManager;
	TradingView::ComIDManager m_ComIDManager;
	TradingView::Outbox m_Outbox;
	TradingView::Inbox m_Inbox;
	std::string m_FormatedTicker;
	std::string m_StockTicker;
	std::string m_StockMarket;
	std::string m_TimeInterval;
	std::string m_FormatedTimeInterval;
	std::string m_TimeFrame;
	unsigned int m_ID;
	bool m_IsThreaded;
	bool m_HasReplay;
	bool m_Aborted;
	unsigned int m_ExecutionTime;
};

}