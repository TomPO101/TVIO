#pragma once
#include "HashGenerator.h"
#include "Inbox.h"
#include "Outbox.h"
#include "Log.h"

namespace TradingView {

class Session
{
public:
	Session(const std::string& stockMarket, const std::string& stockTicker, const std::string& timeInterval, const std::string& timeFrame, const unsigned int ID, const bool replay, const bool recordMessages);
	~Session(void);

	void Start(void);
	void LogResults(void);
	const bool& IsThreaded(void) const { return m_IsThreaded; };
	const bool& Aborted(void) const { return m_Aborted; };

	void SetIsThreaded(bool isThreaded) { m_IsThreaded = isThreaded; };

	TradingView::Inbox& GetInbox(void) { return m_Inbox; };
	TradingView::DataPointManager& GetDataPointManager(void) { return m_DataPointManager; };
	TradingView::Log& GetLog(void) { return m_Log; };

	const std::string& GetStockTicker(void) const { return m_StockTicker; };
	const bool& GetHasReplay(void) const { return m_HasReplay; };
	const unsigned int& GetExecutionTime(void) const { return m_ExecutionTime; };

private:
	bool RequestInitialData(void);
	bool ExtendDataWithReplay(void);
	void Abort(void);

private:
	TradingView::Log m_Log;
	TradingView::TVIO m_TVIO;
	TradingView::DataPointManager m_DataPointManager;
	TradingView::Outbox m_Outbox;
	TradingView::Inbox m_Inbox;
	const std::string m_StockTicker;
	const std::string m_StockMarket;
	const std::string m_TimeInterval;
	const std::string m_TimeFrame;
	std::string m_FormatedTicker;
	std::string m_FormatedTimeInterval;
	const std::string m_ChartSessionHash;
	const std::string m_ChartReplaySessionHash;
	const std::string m_ChartReplaySeriesHash;
	const unsigned int m_ID;
	const bool m_HasReplay;
	bool m_IsThreaded;
	bool m_Aborted;
	unsigned int m_ExecutionTime;
};

}