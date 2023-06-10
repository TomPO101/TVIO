#pragma once
#include "Session.h"

namespace TradingView {
namespace IO {

using namespace TDPGeneral;

class SessionManager
{
public:
	static void Init(void);
	static void CreateSession(const std::string& stockTicker, const std::string& timeInterval, const std::string& timeFrame, bool replay=false, bool recordMessages=false);
							  
	static void ProcessSessions(void);
	static void ClearSessions(void);
	static void LogSummery(void);
	static void Finish(void);

	static Log& GetLog(void) { return m_Log; };
	static std::vector<std::shared_ptr<Session>>& GetSessions(void) { return m_Sessions; };

private:
	static void Run(void);
	static void ProcessStats(void);

private:
	static Log m_Log;
	static std::vector<std::thread> m_Threads;
	static std::mutex m_Mutex;
	static std::vector<std::shared_ptr<Session>> m_Sessions;

	static volatile bool m_CanProcess;
	static volatile bool m_Finished;
	static volatile int m_SessionsFinished;
	static size_t m_ProcessedSessions;
	static size_t m_TotalDPsCollected;
	static size_t m_SessionExecutionTime;
	static std::vector<std::string> m_SessionsAborted;
	static std::vector<std::string> m_SessionDPsEmpty;
};

}
}