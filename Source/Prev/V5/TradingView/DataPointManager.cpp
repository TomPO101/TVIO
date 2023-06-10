#include "DataPointManager.h"
#include "Log.h"

TradingView::DataPointManager::DataPointManager(TradingView::Log& log, const std::string& stockMarket, const std::string& stockTicker, const std::string& timeInterval, const std::string& timeFrame) :
    m_Log(log),
    m_DataPoints(),
    m_StockMarket(stockMarket),
    m_StockTicker(stockTicker),
    m_TimeInterval(timeInterval),
    m_TimeFrame(timeFrame),
    m_LastEarliestDPTime(0)
{
}

bool TradingView::DataPointManager::RecieveCandidate(const std::shared_ptr<TradingView::Message>& data, const bool processDPs)
{
    if (data->GetMessage().HasMember(PARAM_KEY))
        if (data->GetMessage()[PARAM_KEY][1].HasMember(SERIES_ONE))
            if (data->GetMessage()[PARAM_KEY][1][SERIES_ONE].HasMember(SERIES_KEY)) {
                if (data->GetMessage()[PARAM_KEY][1][SERIES_ONE][SERIES_KEY].Size() > 50) { // to make sure its a proper data message
                    if (processDPs)
                        ProcessDataPoints(data->GetMessage()[PARAM_KEY][1][SERIES_ONE][SERIES_KEY]);
                    return true;
                } 
                return false;
            }
    return false;
}

void TradingView::DataPointManager::TruncateToPrevDay(void)
{
    if (!m_DataPoints.empty()) {
        std::vector<TradingView::DataPoint> dps;
        const boost::posix_time::ptime prevDay = boost::posix_time::second_clock::universal_time() - boost::posix_time::hours(24);
        for (const auto& dp : m_DataPoints) {
            if (dp.GetPTime().date() == prevDay.date() &&
                dp.GetPTime().time_of_day() >= boost::posix_time::time_duration(4, 0, 0) &&
                dp.GetPTime().time_of_day() <= boost::posix_time::time_duration(19, 59, 59)) {

                dps.push_back(dp);
            }
        }

        m_DataPoints = dps;

        if (!dps.empty()) {
            m_Log.Add(LOG_TRUNCATED_RESULTS, "Data Points", std::to_string(dps.size()));
            m_Log.Add(LOG_TRUNCATED_RESULTS, "Data Point Start Date", boost::posix_time::to_simple_string(dps.front().GetPTime()));
            m_Log.Add(LOG_TRUNCATED_RESULTS, "Data Points End Date", boost::posix_time::to_simple_string(dps.back().GetPTime()));
        }
        else m_Log.Add(LOG_SESSION_ERROR, "Truncated Data Points was empty", "");
    }
    else m_Log.Add(LOG_SESSION_ERROR, "No Data Points to Truncate", "");
}

void TradingView::DataPointManager::Truncate(const boost::posix_time::ptime& startDate, const boost::posix_time::time_duration& startTime, const boost::posix_time::ptime& endDate, const boost::posix_time::time_duration& endTime)
{
    // needs completing
    if (!m_DataPoints.empty()) {
        std::vector<TradingView::DataPoint> dps;
        for (const auto& dp : m_DataPoints) {
            const auto& time = dp.GetPTime();
            if (time.date() >= startDate.date() &&
                time.date() <= endDate.date() &&
                time.time_of_day() >= startTime &&
                time.time_of_day() <= endTime) {

                dps.push_back(dp);
            }
        }

        m_DataPoints = dps;

        if (!dps.empty()) {
            m_Log.Add(LOG_TRUNCATED_RESULTS, "Data Points", std::to_string(dps.size()));
            m_Log.Add(LOG_TRUNCATED_RESULTS, "Data Point Start Date", boost::posix_time::to_simple_string(dps.front().GetPTime()));
            m_Log.Add(LOG_TRUNCATED_RESULTS, "Data Points End Date", boost::posix_time::to_simple_string(dps.back().GetPTime()));
        }
        else m_Log.Add(LOG_SESSION_ERROR, "Truncated Data Points was empty", "");
    }
    else m_Log.Add(LOG_SESSION_ERROR, "No Data Points to Truncate", "");
}

void TradingView::DataPointManager::OrderByDate(void)
{
    std::sort(m_DataPoints.begin(), m_DataPoints.end(), [](DataPoint& x, DataPoint& y) {
    	return x.GetTimeStamp() < y.GetTimeStamp();
    });
}

void TradingView::DataPointManager::ProcessDataPoints(const rapidjson::Value& data)
{
    m_LastEarliestDPTime = (std::time_t)(data[0][VALUE_KEY][0].GetDouble());

    DataPoint dp;
    for (const auto& value : data.GetArray()) {
        if (value.HasMember(VALUE_KEY)) {
            dp.SetStockMarket(m_StockMarket);
            dp.SetStockTicker(m_StockTicker);
            dp.SetTimeInterval(m_TimeInterval);
            dp.SetTimeFrame(m_TimeFrame);

            auto timet = (std::time_t)value[VALUE_KEY][0].GetDouble();
            dp.SetPTime(boost::posix_time::from_time_t(timet));
            dp.SetTimeStamp(timet);

            dp.SetOpen(value[VALUE_KEY][1].GetFloat());
            dp.SetClose(value[VALUE_KEY][4].GetFloat());
            dp.SetHigh(value[VALUE_KEY][2].GetFloat());
            dp.SetLow(value[VALUE_KEY][3].GetFloat());
            dp.SetVolume(static_cast<unsigned int>(value[VALUE_KEY][5].GetDouble()));

            m_DataPoints.push_back(dp);
        }
    }
}

const unsigned int TradingView::DataPointManager::CheckForDuplicates(void)
{
    unsigned int count = 0;
    auto result = std::adjacent_find(m_DataPoints.begin(), m_DataPoints.end(), [&](const TradingView::DataPoint& x, const TradingView::DataPoint& y) {
        if (x.GetTimeStamp() == y.GetTimeStamp()) {
            count++;
            return true;
        } 
        return false;
    });

    return count;
}

void TradingView::DataPointManager::RemoveDuplicates(void)
{
    m_DataPoints.erase(std::unique(m_DataPoints.begin(), m_DataPoints.end(), [](const TradingView::DataPoint& x, const TradingView::DataPoint& y) {
        return x.GetTimeStamp() == y.GetTimeStamp();
    }), m_DataPoints.end());
}