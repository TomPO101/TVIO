#pragma once
#include "Inbox.h"
#include "Outbox.h"

namespace TradingView {

class Session
{
public:
	Session(TradingView::TVIO* TVIO, std::string timeFrame, std::string timeGroup);
	~Session(void);

	void Start(void);
	void RequestMoreData(unsigned int type, unsigned int amount);
	void ExtendDataWithReplay(void);
	void ShowOverview(void);

private:
	bool ConnectToTradingView(void);
	bool DisconnectFromTradingView(void);
	void CreateSession(void);

private:
	TradingView::TVIO* m_TVIO;
	TradingView::Outbox* m_Outbox;
	TradingView::Inbox* m_Inbox;
	std::string m_ChartSessionID;
	std::string m_ChartReplaySessionID;
	std::string m_ChartReplaySeriesID;
	std::string m_TimeFrame;
	std::string m_TimeGroup;
	unsigned int m_CreateSeriesID;
	unsigned int m_ReplayAddSeriesID;
	unsigned int m_ModifySeriesID;
};

}