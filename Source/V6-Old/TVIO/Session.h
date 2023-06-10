#pragma once
#include "HashGenerator.h"
#include "Inbox.h"
#include "Outbox.h"
#include "Log/Log.h"

namespace TradingView {
namespace IO {

#define MAX_CONSECUTIVE_SESSIONS (6)

using namespace TDPGeneral;
using namespace TradingCommon;

class Session
{
public:
	Session(const std::string& stockTicker, const std::string& timeInterval, const std::string& timeFrame,
		    const int ID=1, const bool replay=false, const bool recordMessages=false);
		    
	void Start(void);
	void LogResults(void);

	void SetThreaded(const bool threaded) { m_Threaded = threaded; };
	void SetRunning(const bool running) { m_Running = running; };

	const bool Aborted(void) const { return m_Aborted; };
	const bool Threaded(void) const { return m_Threaded; };
	const bool Running(void) const { return m_Running; };

	const int GetID(void) const { return m_ID; };
	Inbox& GetInbox(void) { return m_Inbox; };
	DataPointManager& GetDataPointManager(void) { return m_DataPointManager; };
	Log& GetLog(void) { return m_Log; };

	const std::string& GetStockTicker(void) const { return m_StockTicker; };
	const bool GetHasReplay(void) const { return m_HasReplay; };
	const int GetExecutionTime(void) const { return m_ExecutionTime; };

	bool RequestInitialData(void);
	bool ExtendDataWithReplay(void);
	void Abort(void);

private:
	const std::string m_StockTicker;
	const std::string m_TimeInterval;
	const std::string m_TimeFrame;
	Log m_Log;
	Network m_Network;
	DataPointManager m_DataPointManager;
	Outbox m_Outbox;
	Inbox m_Inbox;
	std::string m_FormatedTicker;
	std::string m_FormatedTimeInterval;
	const std::string m_ChartSessionHash;
	const std::string m_ChartReplaySessionHash;
	const std::string m_ChartReplaySeriesHash;
	const int m_ID;
	const bool m_HasReplay;
	bool m_Threaded;
	bool m_Running;
	bool m_Aborted;
	int m_ExecutionTime;
};

}
}