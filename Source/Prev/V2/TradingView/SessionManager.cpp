#include "SessionManager.h"

TradingView::TVIO* TradingView::SessionManager::m_TVIO;
std::vector<TradingView::Session*> TradingView::SessionManager::m_Sessions;
std::vector<TradingView::SessionChartID> TradingView::SessionManager::m_SessionChartIDs;
boost::thread_group TradingView::SessionManager::m_Threadpool;

void TradingView::SessionManager::Init(void)
{
	m_TVIO = new TradingView::TVIO();
	m_TVIO->Connect();

	SessionChartID ID1;
	ID1.ID = 1;
	ID1.chartSessionID = CHART_SESSION_ID_1;
	ID1.chartReplaySessionID = CHART_REPLAY_SESSION_ID_1;
	ID1.chartReplaySeriesID = CHART_REPLAY_SERIES_ID_1;
	m_SessionChartIDs.push_back(ID1);

	SessionChartID ID2;
	ID2.ID = 2;
	ID2.chartSessionID = CHART_SESSION_ID_2;
	ID2.chartReplaySessionID = CHART_REPLAY_SESSION_ID_2;
	ID2.chartReplaySeriesID = CHART_REPLAY_SERIES_ID_2;
	m_SessionChartIDs.push_back(ID2);
}

void TradingView::SessionManager::CreateSession(std::string timeFrame, std::string timeGroup)
{
	SessionChartID sessionChartID;
	if (!m_Sessions.empty()) {
		for (auto& chartID : m_SessionChartIDs) {
			bool taken = false;
			for (auto& s : m_Sessions) {
				if (s->GetSessionChartID().ID == chartID.ID) {
					taken = true;
				}
			}
			if (!taken) {
				sessionChartID = chartID;
				goto create;
			}
		}
	} 
	else {
		sessionChartID = m_SessionChartIDs.at(0);
		goto create;
	}

create:
	m_Threadpool.create_thread(boost::bind(&boost::asio::io_service::run, &m_TVIO->GetIOC()));
	Session* session = new Session(m_TVIO, timeFrame, timeGroup, sessionChartID);
	m_Sessions.push_back(session);
}

void TradingView::SessionManager::Run(void)
{
	m_Sessions.at(0)->SetAuth();
	for (auto& session : m_Sessions) {
		m_TVIO->GetIOC().post(boost::bind(&TradingView::Session::Start, session));
	}
}

void TradingView::SessionManager::Finish(void)
{
	//m_TVIO->GetIOC().stop();
	//m_TVIO->GetIOC().reset();
	m_Threadpool.join_all();
	m_TVIO->Disconnect();
}
