#pragma once
#include "Message.h"
#include "TradingCommon/DataPoint.h"
#include "Log/Log.h"

namespace TradingView {
namespace IO {

using namespace TradingCommon;
using namespace TDPGeneral;
using namespace boost::posix_time;
using namespace rapidjson;

class DataPointManager
{
	friend class Session;
	friend class MessageProcessor;

public:
	void OrderByDate(void);
	void TruncateToPrevDay(void);
	void Truncate(const ptime& startDate, const ptime& endDate);
	const size_t CheckForDuplicates(void);
	void RemoveDuplicates(void);
	void DisplayInfo(void);

	std::vector<std::shared_ptr<DataPoint>>& GetDataPoints(void) { return m_DataPoints; };

private:
	DataPointManager(Log& log, const std::string& stockTicker, const std::string& timeInterval, const std::string& timeFrame);
	std::time_t& GetLastEarliestDPTime(void) { return m_LastEarliestDPTime; };
	void ProcessDataPoints(const rapidjson::Value& data);
	bool RecieveCandidate(std::shared_ptr<Message>& data, const bool processDPs);

private:
	Log& m_Log;
	std::vector<std::shared_ptr<DataPoint>> m_DataPoints;
	const std::string& m_StockTicker;
	const std::string& m_TimeInterval;
	const std::string& m_TimeFrame;
	std::time_t m_LastEarliestDPTime;
};

}
}