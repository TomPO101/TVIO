#include "ComIDManager.h"
#include "Message.h"
#include "csv.h"
#include <fstream>

TradingView::ComIDManager::ComIDManager(std::string stockTicker, std::string timeInterval, std::string timeFrame) :
	m_StockTicker(stockTicker),
	m_TimeInterval(timeInterval),
	m_TimeFrame(timeFrame),
	m_ChartSessionCode(CHART_SESSION_CODE),
	m_ReplayChartSessionCode(CHART_REPLAY_SESSION_CODE),
	m_ReplayChartSeriesCode(CHART_REPLAY_SERIES_CODE),
	m_CreateChartSessionID(0),
	m_SwitchTimeZoneID(0),
	m_CreateChartSeriesID(0),
	m_ReplayChartCreateSessionID(0),
	m_ReplayChartAddSeriesID(0),
	m_ReplayResolveChartSymbolID(0),
	m_ReplayModifyChartSeriesID(0),
	m_ReplayChartResetID(0),
	m_ResolveChartSymbolID(0)
{
}

TradingView::ComIDManager::~ComIDManager(void)
{
}

bool TradingView::ComIDManager::ProcessStockIDData(void)
{
	try {
		io::CSVReader<10> in(R"(C:\Code\StockTrading\Projects\TradingView\C++\TradingView\TradingView\Assets\TradingViewComIDs.csv)");

		in.read_header(
			io::ignore_extra_column, // ignores columns that exist but are not in the argument list
			"Ticker",
			"CreateChartSession",
			"SwitchTimeZone",
			"ResolveChartSymbolSecond",
			"CreateChartSeries",
			"ReplayChartCreateSession",
			"ReplayChartAddSeries",
			"ReplayResolveChartSymbol",
			"ReplayModifyChartSeries",
			"ReplayChartReset"
		);

		std::string ticker;
		unsigned int createChartSession;
		unsigned int switchTimeZone;
		unsigned int resolveChartSymbolSecond;
		unsigned int createChartSeries;
		unsigned int replayChartCreateSession;
		unsigned int replayChartAddSeries;
		unsigned int replayResolveChartSymbol;
		unsigned int replayModifyChartSeries;
		unsigned int replayChartReset;

		while (in.read_row(
				ticker,
				createChartSession,
				switchTimeZone,
				resolveChartSymbolSecond,
				createChartSeries,
				replayChartCreateSession,
				replayChartAddSeries,
				replayResolveChartSymbol,
				replayModifyChartSeries,
				replayChartReset
			))
		{
			if (ticker == m_StockTicker) {
				m_CreateChartSessionID = createChartSession;
				m_SwitchTimeZoneID = switchTimeZone;
				m_CreateChartSeriesID = createChartSeries;
				m_ReplayChartCreateSessionID = replayChartCreateSession;
				m_ReplayChartAddSeriesID = replayChartAddSeries;
				m_ReplayResolveChartSymbolID = replayResolveChartSymbol;
				m_ReplayModifyChartSeriesID = replayModifyChartSeries;
				m_ReplayChartResetID = replayChartReset;

				if (m_TimeFrame == TIME_FRAME_SECOND) {
					m_ResolveChartSymbolID = resolveChartSymbolSecond;
				}
				break;
			}
			//std::cout << ticker << ID << std::endl;
		}
		return true;
	}
	catch (std::exception& e) {
		std::cout << e.what() << "\n";
		return false;
	}
}