#pragma once
#include <string>

namespace TradingView {

#define CHART_SESSION_CODE ("cs_lEGq9nskjz0v")
#define CHART_REPLAY_SESSION_CODE ("rs_ijFvZWzw6wKh")
#define CHART_REPLAY_SERIES_CODE ("551EIYmNx43D0")

#define AUTH_ID (504)
#define CHART_MORE_DATA_MED_ID (59)
#define CHART_MORE_DATA_LARGE_ID (60)
#define CHART_MORE_DATA_EXTRA_LARGE_ID (61)

class ComIDManager
{
public:
	ComIDManager(std::string stockTicker, std::string timeInterval, std::string timeFrame);
	~ComIDManager(void);

	std::string GetChartSessionCode(void) { return m_ChartSessionCode; };
	std::string GetReplayChartSessionCode(void) { return m_ReplayChartSessionCode; };
	std::string GetReplayChartSeriesCode(void) { return m_ReplayChartSeriesCode; };
	unsigned int GetCreateChartSessionID(void) { return m_CreateChartSessionID; };
	unsigned int GetSwitchTimeZoneID(void) { return m_SwitchTimeZoneID; };
	unsigned int GetCreateChartSeriesID(void) { return m_CreateChartSeriesID; };
	unsigned int GetReplayChartCreateSessionID(void) { return m_ReplayChartCreateSessionID; };
	unsigned int GetReplayChartAddSeriesID(void) { return m_ReplayChartAddSeriesID; };
	unsigned int GetReplayResolveChartSymbolID(void) { return m_ReplayResolveChartSymbolID; };
	unsigned int GetReplayModifyChartSeriesID(void) { return m_ReplayModifyChartSeriesID; };
	unsigned int GetReplayChartResetID(void) { return m_ReplayChartResetID; };
	unsigned int GetResolveChartSymbolID(void) { return m_ResolveChartSymbolID; };

	bool ProcessStockIDData(void);

private:
	std::string m_StockTicker;
	std::string m_TimeInterval;
	std::string m_TimeFrame;
	std::string m_ChartSessionCode;
	std::string m_ReplayChartSessionCode;
	std::string m_ReplayChartSeriesCode;
	unsigned int m_CreateChartSessionID;
	unsigned int m_SwitchTimeZoneID;
	unsigned int m_CreateChartSeriesID;
	unsigned int m_ReplayChartCreateSessionID;
	unsigned int m_ReplayChartAddSeriesID;
	unsigned int m_ReplayResolveChartSymbolID;
	unsigned int m_ReplayModifyChartSeriesID;
	unsigned int m_ReplayChartResetID;
	unsigned int m_ResolveChartSymbolID;
};

}