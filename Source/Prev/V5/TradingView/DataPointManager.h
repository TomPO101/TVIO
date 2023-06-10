#pragma once
#include "Message.h"
#include "DataPoint.h"

namespace TradingView {

class Log;

class DataPointManager
{
public:
	DataPointManager(TradingView::Log& log, const std::string& stockMarket, const std::string& stockTicker, const std::string& timeInterval, const std::string& timeFrame);

	bool RecieveCandidate(const std::shared_ptr<TradingView::Message>& data, const bool processDPs);
	void OrderByDate(void);
	void TruncateToPrevDay(void);
	void Truncate(const boost::posix_time::ptime& startDate, const boost::posix_time::time_duration& startTime, const boost::posix_time::ptime& endDate, const boost::posix_time::time_duration& endTime);
	const unsigned int CheckForDuplicates(void);
	void RemoveDuplicates(void);

	std::vector<TradingView::DataPoint>& GetDataPoints(void) { return m_DataPoints; };
	std::time_t& GetLastEarliestDPTime(void) { return m_LastEarliestDPTime; };

private:
	void ProcessDataPoints(const rapidjson::Value& data);

private:
	TradingView::Log& m_Log;
	std::vector<TradingView::DataPoint> m_DataPoints;
	const std::string m_StockTicker;
	const std::string m_StockMarket;
	const std::string m_TimeInterval;
	const std::string m_TimeFrame;
	std::time_t m_LastEarliestDPTime;
};

}