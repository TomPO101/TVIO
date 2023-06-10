#include "ComIDManager.h"
#include "Message.h"
#include "csv.h"
#include <fstream>

TradingView::ComIDManager::ComIDManager(std::string stockTicker, std::string timeInterval, std::string timeFrame) :
	m_StockTicker(stockTicker),
	m_TimeInterval(timeInterval),
	m_FormatedTimeInterval(""),
	m_TimeFrame(timeFrame),
	m_ChartSessionCode(""),
	m_ChartReplaySessionCode(""),
	m_ChartReplaySeriesCode(""),
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
		io::CSVReader<16> in(R"(C:\Code\StockTrading\Projects\TradingView\C++\TradingView\TradingView\Assets\TradingViewComIDs.csv)");

		// MIGHT BEBETTER TO SPLIT UP EACH TIME FRAME IN SEPERATE CSV FILES

		in.read_header(
			io::ignore_extra_column, // ignores columns that exist but are not in the argument list
			"Ticker",
			"ChartSessionCode",
			"ChartReplaySessionCode",
			"ChartReplaySeriesCode",
			"CreateChartSession",
			"SwitchTimeZone",
			"ResolveChartSymbolSecond",
			"CreateChartSeries",
			"CreateChartSeries15S",
			"ReplayChartCreateSession",
			"ReplayChartAddSeries",
			"ReplayChartAddSeries15S",
			"ReplayResolveChartSymbol",
			"ReplayModifyChartSeries",
			"ReplayModifyChartSeries15S",
			"ReplayChartReset"
		);

		std::string ticker;
		std::string chartSessionCode;
		std::string chartReplaySessionCode;
		std::string chartReplaySeriesCode;
		unsigned int createChartSession;
		unsigned int switchTimeZone;
		unsigned int resolveChartSymbolSecond;
		unsigned int createChartSeries;
		unsigned int createChartSeries15S;
		unsigned int replayChartCreateSession;
		unsigned int replayChartAddSeries;
		unsigned int replayChartAddSeries15S;
		unsigned int replayResolveChartSymbol;
		unsigned int replayModifyChartSeries;
		unsigned int replayModifyChartSeries15S;
		unsigned int replayChartReset;

		while (in.read_row(
				ticker,
				chartSessionCode,
				chartReplaySessionCode,
				chartReplaySeriesCode,
				createChartSession,
				switchTimeZone,
				resolveChartSymbolSecond,
				createChartSeries,
				createChartSeries15S,
				replayChartCreateSession,
				replayChartAddSeries,
				replayChartAddSeries15S,
				replayResolveChartSymbol,
				replayModifyChartSeries,
				replayModifyChartSeries15S,
				replayChartReset
			))
		{
			if (ticker == m_StockTicker) {
				m_ChartSessionCode = chartSessionCode;
				m_ChartReplaySessionCode = chartReplaySessionCode;
				m_ChartReplaySeriesCode = chartReplaySeriesCode;
				m_CreateChartSessionID = createChartSession;
				m_SwitchTimeZoneID = switchTimeZone;
				m_ReplayChartCreateSessionID = replayChartCreateSession;
				m_ReplayResolveChartSymbolID = replayResolveChartSymbol;
				m_ReplayChartResetID = replayChartReset;

				if (m_TimeFrame == TIME_FRAME_SECOND) {
					m_ResolveChartSymbolID = resolveChartSymbolSecond;
					if (m_TimeInterval == TIME_INTERVAL_ONE) {
						m_CreateChartSeriesID = createChartSeries;
						m_ReplayChartAddSeriesID = replayChartAddSeries;
						m_ReplayModifyChartSeriesID = replayModifyChartSeries;
					}
					//if (m_TimeInterval == TIME_INTERVAL_FIFTEEN) {
					//	m_CreateChartSeriesID = createChartSeries15S;
					//	m_ReplayChartAddSeriesID = replayChartAddSeries15S;
					//	m_ReplayModifyChartSeriesID = replayModifyChartSeries15S;
					//}
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