#include "SessionManager.h"

TradingView::Log TradingView::SessionManager::m_Log; 
std::vector<std::shared_ptr<TradingView::Session>> TradingView::SessionManager::m_Sessions;
std::vector<std::thread> TradingView::SessionManager::m_Threads;
unsigned int TradingView::SessionManager::m_ProcessedSessions;
unsigned int TradingView::SessionManager::m_TotalDPsCollected;
unsigned int TradingView::SessionManager::m_SessionExecutionTime;
std::vector<std::string> TradingView::SessionManager::m_SessionsAborted;
std::vector<std::string> TradingView::SessionManager::m_SessionDPsEmpty;

void TradingView::SessionManager::CreateSession(const std::string& stockMarket, const std::string& stockTicker, const std::string& timeInterval, const std::string& timeFrame, bool replay, bool recordMessages)
{
	m_Sessions.push_back(std::make_shared<TradingView::Session>(stockMarket, stockTicker, timeInterval, timeFrame, m_Sessions.size() + 1, replay, recordMessages));
}

void TradingView::SessionManager::TryAddSessionThread(void)
{
	for (auto& session : m_Sessions) {
		if (m_Threads.size() < MAX_CONSECUTIVE_SESSIONS && !session->IsThreaded()) {
			session->SetIsThreaded(true);
			m_Threads.push_back(std::thread(&TradingView::Session::Start, session));
		}
	}
}

void TradingView::SessionManager::ProcessStats(void)
{
	m_ProcessedSessions += m_Sessions.size();

	for (const auto& session : m_Sessions) {
		const unsigned int dpCount = session->GetDataPointManager().GetDataPoints().size();
		if (dpCount > 0) 
			m_TotalDPsCollected += dpCount;
		else 
			m_SessionDPsEmpty.push_back(session->GetStockTicker());

		if (session->Aborted())
			m_SessionsAborted.push_back(session->GetStockTicker());

		m_SessionExecutionTime += session->GetExecutionTime();
	}
}

void TradingView::SessionManager::LogSummery(void)
{
	for (const auto& stock : m_SessionDPsEmpty) 
		m_Log.Add(LOG_SESSION_MANAGER_ERROR, "Data Points Empty", stock);

	for (const auto& stock : m_SessionsAborted)
		m_Log.Add(LOG_SESSION_MANAGER_ERROR, "Session Was Aborted", stock);

	m_Log.Add(LOG_SESSION_MANAGER_SUMMARY, "Processed Sessions", std::to_string(m_ProcessedSessions));
	m_Log.Add(LOG_SESSION_MANAGER_SUMMARY, "Total Data Points Collected", std::to_string(m_TotalDPsCollected)); 
	m_Log.Add(LOG_SESSION_MANAGER_SUMMARY, "Total Session Execution Time", std::to_string(m_SessionExecutionTime));
}

void TradingView::SessionManager::Run(void)
{
	// threads finish and wait for the thread that is doing the join so wont be able to be added
	if (!m_Sessions.empty()) {
		TryAddSessionThread();
		while (!m_Threads.empty()) {
			m_Threads.at(0).join();
			m_Threads.erase(m_Threads.begin());
			TryAddSessionThread();
		}
		ProcessStats();
	}
	else
		m_Log.Add(LOG_SESSION_MANAGER_ERROR, "No Sessions To Run", "");
}

void TradingView::SessionManager::Clear(void)
{
	m_Sessions.clear();
	m_Threads.clear();
}
