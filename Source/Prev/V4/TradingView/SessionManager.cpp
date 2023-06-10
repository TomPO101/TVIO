#include "SessionManager.h"

TradingView::Log TradingView::SessionManager::m_Log;
unsigned int TradingView::SessionManager::m_MaxThreads;
std::vector<std::shared_ptr<TradingView::Session>> TradingView::SessionManager::m_Sessions;
std::vector<std::thread> TradingView::SessionManager::m_Threads;

void TradingView::SessionManager::Init(void)
{
	m_MaxThreads = 6;
}

void TradingView::SessionManager::CreateSession(std::string market, std::string stockTicker, std::string timeInterval, std::string timeFrame, bool replay) 
{
	m_Sessions.push_back(std::make_shared<TradingView::Session>(market, stockTicker, timeInterval, timeFrame, m_Sessions.size() + 1, replay));
}

void TradingView::SessionManager::TryAddSessionThread(void)
{
	for (auto& session : m_Sessions) {
		if (m_Threads.size() < m_MaxThreads && !session->IsThreaded()) {
			session->SetIsThreaded(true);
			m_Threads.push_back(std::thread(&TradingView::Session::Start, session));
		}
	}
}

void TradingView::SessionManager::LogSummery(void)
{
	unsigned int totalDPs = 0;

	m_Log.Add(LOG_SESSION_MANAGER_SUMMARY, "Processed Sessions", std::to_string(m_Sessions.size()));

	for (auto& session : m_Sessions) {
		std::vector<TradingView::DataPoint>& dps = session->GetDataPointManager().GetDataPoints();

		if (session->Aborted()) {
			m_Log.Add(LOG_SESSION_MANAGER_ERROR, "Session Was Aborted", session->GetStockTicker());
		}
		else {
			if (dps.empty()) {
				m_Log.Add(LOG_SESSION_MANAGER_ERROR, "Data Points Empty", session->GetStockTicker());
			}
			else {
				totalDPs += dps.size();
			}
		}
	}
	m_Log.Add(LOG_SESSION_MANAGER_SUMMARY, "Total Data Points Collected", std::to_string(totalDPs));
}

void TradingView::SessionManager::Run(void)
{
	if (!m_Sessions.empty()) {
		TryAddSessionThread();
		while (!m_Threads.empty()) {
			m_Threads.at(0).join();
			m_Threads.erase(m_Threads.begin());
			TryAddSessionThread();
		}
		LogSummery();
	}
	else {
		m_Log.Add(LOG_SESSION_MANAGER_ERROR, "No Sessions To Run", "");
	}
}
