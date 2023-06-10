#pragma once
#include "Message.h"
#include "DataPoint.h"

namespace TradingView {

class Log;

class DataPointManager
{
public:
	DataPointManager(TradingView::Log& log, std::string stockMarket, std::string stockTicker, std::string timeInterval, std::string timeFrame);
	~DataPointManager(void);

	bool RecieveCandidate(TradingView::Message& data, bool processDPs);
	void OrderByDate(void);
	void TruncateToPrevDay(void);

	std::vector<TradingView::DataPoint>& GetDataPoints(void) { return m_DataPoints; };

private:
	void ProcessDataPoints(nlohmann::json& dp);

private:
	TradingView::Log& m_Log;
	std::vector<TradingView::DataPoint> m_DataPoints;
	std::string m_StockTicker;
	std::string m_StockMarket;
	std::string m_TimeInterval;
	std::string m_TimeFrame;
};

}