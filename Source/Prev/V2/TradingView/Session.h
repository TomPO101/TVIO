#pragma once
#include "Inbox.h"
#include "Outbox.h"

namespace TradingView {

struct SessionChartID
{
	unsigned int ID;
	std::string chartSessionID;
	std::string chartReplaySessionID;
	std::string chartReplaySeriesID;
};

class Session
{
public:
	Session(TradingView::TVIO* TVIO, std::string timeFrame, std::string timeGroup, SessionChartID sessionChartID);
	~Session(void);

	void Start(void);
	void RequestMoreData(unsigned int type, unsigned int amount);
	void ExtendDataWithReplay(void);
	void ShowOverview(void);
	void SetAuth(void);

	SessionChartID& GetSessionChartID(void) { return m_SessionChartID; };

private:
	void CreateSession(void);

private:
	TradingView::TVIO* m_TVIO;
	TradingView::Outbox* m_Outbox;
	TradingView::Inbox* m_Inbox;
	SessionChartID m_SessionChartID;
	std::string m_TimeFrame;
	std::string m_TimeGroup;
	unsigned int m_CreateSeriesID;
	unsigned int m_ReplayAddSeriesID;
	unsigned int m_ModifySeriesID;
};

}