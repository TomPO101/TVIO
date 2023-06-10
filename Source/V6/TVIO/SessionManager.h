#pragma once
#include "Session.h"

namespace TradingView {
namespace IO {

using namespace TDPGeneral;

class SessionManager
{
public:
	SessionManager(void);

	void Run(void);

	void CreateSession(const std::string& stockTicker, const std::string& timeInterval, const std::string& timeFrame, bool replay=false, bool recordMessages=false);
							  
	void LogSummery(void);

	Log& GetLog(void) { return m_Log; };
	std::vector<std::shared_ptr<Session>>& GetSessions(void) { return m_Sessions; };

private:
	void ProcessStats(void);

private:
	Log m_Log;
	std::vector<std::shared_ptr<Session>> m_Sessions;
	size_t m_ProcessedSessions;
	size_t m_TotalDPsCollected;
	size_t m_SessionExecutionTime;
	std::vector<std::string> m_SessionsAborted;
	std::vector<std::string> m_SessionDPsEmpty;
};

}
}