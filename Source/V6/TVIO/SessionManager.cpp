#include "SessionManager.h"

namespace TradingView {
namespace IO {

SessionManager::SessionManager(void) :
	m_Log("SessionManager"),
	m_Sessions(),
	m_ProcessedSessions(0),
	m_TotalDPsCollected(0),
	m_SessionExecutionTime(),
	m_SessionsAborted(),
	m_SessionDPsEmpty()
{
}

void SessionManager::Run(void)
{
	std::vector<std::future<void>> futures;

	for (const auto& session : m_Sessions) {

		futures.push_back(std::async(std::launch::async, &Session::Start, session));
	}

	for (auto& future : futures)
		future.get();

	ProcessStats();
}

void SessionManager::CreateSession(const std::string& stockTicker, const std::string& timeInterval, const std::string& timeFrame, bool replay, bool recordMessages)
{
	m_Sessions.push_back(std::make_shared<Session>(stockTicker, timeInterval, timeFrame, (int)(m_Sessions.size() + 1), replay, recordMessages));
}

void SessionManager::ProcessStats(void)
{
	m_ProcessedSessions += m_Sessions.size();

	for (auto& session : m_Sessions) {
		const std::size_t dpCount = session->GetDataPointManager().GetDataPoints().size();
		if (dpCount > 0) 
			m_TotalDPsCollected += dpCount;
		else 
			m_SessionDPsEmpty.push_back(session->GetStockTicker());

		if (session->Aborted())
			m_SessionsAborted.push_back(session->GetStockTicker());

		m_SessionExecutionTime += session->GetExecutionTime();
	}
}

void SessionManager::LogSummery(void)
{
	for (const auto& stock : m_SessionDPsEmpty) 
		m_Log.Add(LOG_SESSION_MANAGER_ERROR, "Data Points Empty", stock);

	for (const auto& stock : m_SessionsAborted)
		m_Log.Add(LOG_SESSION_MANAGER_ERROR, "Session Was Aborted", stock);

	m_Log.Add(LOG_SESSION_MANAGER_SUMMARY, "Processed Sessions", std::to_string(m_ProcessedSessions));
	m_Log.Add(LOG_SESSION_MANAGER_SUMMARY, "Total Data Points Collected", std::to_string(m_TotalDPsCollected)); 
	m_Log.Add(LOG_SESSION_MANAGER_SUMMARY, "Total Session Execution Time", std::to_string(m_SessionExecutionTime));
}

}
}