#pragma once
#include "Session.h"

namespace TradingView {

#define MAX_CONSECUTIVE_SESSIONS (7)

class SessionManager
{
public:
	static void CreateSession(const std::string& stockMarket, const std::string& stockTicker, const std::string& timeInterval, const std::string& timeFrame, bool replay, bool recordMessages);
	static void Run(void);
	static void Clear(void);
	static void LogSummery(void);

	static TradingView::Log& GetLog(void) { return m_Log; };
	static std::vector<std::shared_ptr<TradingView::Session>>& GetSessions(void) { return m_Sessions; };

private:
	static void TryAddSessionThread(void);
	static void ProcessStats(void);

private:
	static TradingView::Log m_Log;
	static std::vector<std::thread> m_Threads;
	static std::vector<std::shared_ptr<TradingView::Session>> m_Sessions;

	static unsigned int m_ProcessedSessions;
	static unsigned int m_TotalDPsCollected;
	static unsigned int m_SessionExecutionTime;
	static std::vector<std::string> m_SessionsAborted;
	static std::vector<std::string> m_SessionDPsEmpty;
};

}