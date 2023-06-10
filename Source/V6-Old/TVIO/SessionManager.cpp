#include "SessionManager.h"

namespace TradingView {
namespace IO {

Log SessionManager::m_Log; 
std::vector<std::shared_ptr<Session>> SessionManager::m_Sessions;
volatile bool SessionManager::m_CanProcess;
volatile bool SessionManager::m_Finished;
volatile int SessionManager::m_SessionsFinished;
std::vector<std::thread> SessionManager::m_Threads;
std::mutex SessionManager::m_Mutex;
size_t SessionManager::m_ProcessedSessions;
size_t SessionManager::m_TotalDPsCollected;
size_t SessionManager::m_SessionExecutionTime;
std::vector<std::string> SessionManager::m_SessionsAborted;
std::vector<std::string> SessionManager::m_SessionDPsEmpty;

void SessionManager::Init(void)
{
	m_Log.ChangeTitle("SessionManager");
	m_CanProcess = false;
	m_Finished = false;
	m_SessionsFinished = 0;
	m_Threads.reserve(MAX_CONSECUTIVE_SESSIONS);
	for (int i = 0; i < MAX_CONSECUTIVE_SESSIONS; i++)
		m_Threads.push_back(std::thread(&SessionManager::Run));
}

void SessionManager::Run(void)
{
	while (true) {
		if (m_Finished) return;
		if (m_CanProcess) {
			if (!m_Sessions.empty()) {
				size_t index = -1;
				{
					std::scoped_lock<std::mutex> lock(m_Mutex);
					for (size_t i = 0; i < m_Sessions.size(); i++)
						if (!m_Sessions[i]->Threaded()) {
							m_Sessions[i]->SetThreaded(true);
							index = i;
							break;
						}
				}
				if (index != -1)
					if (!m_Sessions[index]->Running()) {
						m_Sessions[index]->Start();
						m_SessionsFinished++;
					}
			}
		}
	}
}

void SessionManager::CreateSession(const std::string& stockTicker, const std::string& timeInterval, const std::string& timeFrame, bool replay, bool recordMessages)				   
{
	m_Sessions.push_back(std::make_shared<Session>(stockTicker, timeInterval, timeFrame, (int)(m_Sessions.size() + 1), replay, recordMessages));
}

void SessionManager::ProcessSessions(void)
{
	m_CanProcess = true;

	while (m_SessionsFinished != m_Sessions.size()) {}

	m_CanProcess = false;

	m_SessionsFinished = 0;

	ProcessStats();
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

void SessionManager::ClearSessions(void)
{
	m_Sessions.clear();
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

void SessionManager::Finish(void)
{
	m_Finished = true;
	//std::this_thread::sleep_for(std::chrono::seconds(1));
	for (auto& thread : m_Threads)
		thread.join();
}

}
}