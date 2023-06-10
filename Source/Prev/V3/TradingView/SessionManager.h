#pragma once
#include "Session.h"

namespace TradingView {

class SessionManager
{
public:
	static void Init(void);
	static void CreateSession(std::string market, std::string stockTicker, std::string timeInterval, std::string timeFrame, bool replay=false);
	static void Run(void);

	static TradingView::Log& GetLog(void) { return m_Log; };
	static std::vector<std::shared_ptr<TradingView::Session>>& GetSessions(void) { return m_Sessions; };
	static unsigned int GetMaxThreads(void) { return m_MaxThreads; };

private:
	static void TryAddSessionThread(void);
	static void LogSummery(void);

private:
	static TradingView::Log m_Log;
	static unsigned int m_MaxThreads;
	static std::vector<std::thread> m_Threads;
	static std::vector<std::shared_ptr<TradingView::Session>> m_Sessions;
};

}