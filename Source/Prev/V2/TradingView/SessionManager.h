#pragma once
#include "Session.h"

namespace TradingView {

#define CHART_SESSION_ID_1 ("cs_lEGq9nskjz0v")
#define CHART_REPLAY_SESSION_ID_1 ("rs_ijFvZWzw6wKh")
#define CHART_REPLAY_SERIES_ID_1 ("551EIYmNx43D0")

#define CHART_SESSION_ID_2 ("cs_MlQfq2CAdvl2")
#define CHART_REPLAY_SESSION_ID_2 ("rs_e8AVPMMZhpGy")
#define CHART_REPLAY_SERIES_ID_2 ("TXYEk7ThIrPU0")

class SessionManager
{
public:
	static void Init(void);
	static void CreateSession(std::string timeFrame, std::string timeGroup);
	static void Run(void);
	static void Finish(void);

private:
	static boost::thread_group m_Threadpool;
	static TradingView::TVIO* m_TVIO;
	static std::vector<Session*> m_Sessions;
	static std::vector<SessionChartID> m_SessionChartIDs;
};

}