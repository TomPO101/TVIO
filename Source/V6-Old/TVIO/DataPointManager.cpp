#include "DataPointManager.h"

namespace TradingView {
namespace IO {

DataPointManager::DataPointManager(Log& log, const std::string& stockTicker, const std::string& timeInterval, const std::string& timeFrame) :
    m_Log(log),
    m_DataPoints(),
    m_StockTicker(stockTicker),
    m_TimeInterval(timeInterval),
    m_TimeFrame(timeFrame),
    m_LastEarliestDPTime(0)
{
    m_DataPoints.reserve(50000);
}

bool DataPointManager::RecieveCandidate(std::shared_ptr<Message>& data, const bool processDPs)
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

void DataPointManager::TruncateToPrevDay(void)
{
    if (!m_DataPoints.empty()) {
        std::vector<std::shared_ptr<DataPoint>> dps;
        const ptime prevDay = second_clock::universal_time() - hours(24);
        for (const auto& dp : m_DataPoints) {
            if (dp->GetPTime().date() == prevDay.date() &&
                dp->GetPTime().time_of_day() >= time_duration(4, 0, 0) &&
                dp->GetPTime().time_of_day() <= time_duration(19, 59, 59)) {

                dps.push_back(dp);
            }
        }

        m_DataPoints = dps;

        if (!dps.empty()) {
            m_Log.Add(LOG_TRUNCATED_RESULTS, "Data Points", std::to_string(dps.size()));
            m_Log.Add(LOG_TRUNCATED_RESULTS, "Data Point Start Date", to_simple_string(dps.front()->GetPTime()));
            m_Log.Add(LOG_TRUNCATED_RESULTS, "Data Points End Date", to_simple_string(dps.back()->GetPTime()));
        }
        else m_Log.Add(LOG_SESSION_ERROR, "Truncated Data Points was empty", "");
    }
    else m_Log.Add(LOG_SESSION_ERROR, "No Data Points to Truncate", "");
}

void DataPointManager::Truncate(const ptime& startDate, const ptime& endDate)
{
    if (!m_DataPoints.empty()) {

        m_DataPoints.erase(std::remove_if(m_DataPoints.begin(), m_DataPoints.end(), [&](const std::shared_ptr<DataPoint>& x) {
            return x->GetPTime() < startDate || x->GetPTime() > endDate;
        }), m_DataPoints.end());

        if (!m_DataPoints.empty()) {
            m_Log.Add(LOG_TRUNCATED_RESULTS, "Data Points", std::to_string(m_DataPoints.size()));
            m_Log.Add(LOG_TRUNCATED_RESULTS, "Data Point Start Date", to_simple_string(m_DataPoints.front()->GetPTime()));
            m_Log.Add(LOG_TRUNCATED_RESULTS, "Data Points End Date", to_simple_string(m_DataPoints.back()->GetPTime()));
        }
        else m_Log.Add(LOG_SESSION_ERROR, "Truncated Data Points was empty", "");
    }
    else m_Log.Add(LOG_SESSION_ERROR, "No Data Points to Truncate", "");
}

void DataPointManager::OrderByDate(void)
{
    std::sort(m_DataPoints.begin(), m_DataPoints.end(), [](std::shared_ptr<DataPoint>& x, std::shared_ptr<DataPoint>& y) {
    	return x->GetPTime() < y->GetPTime();
    });
}

void DataPointManager::ProcessDataPoints(const rapidjson::Value& data)
{
    m_LastEarliestDPTime = (std::time_t)(data[0][VALUE_KEY][0].GetDouble());

    for (const auto& value : data.GetArray()) {
        if (value.HasMember(VALUE_KEY)) {

            m_DataPoints.push_back(std::make_shared<DataPoint>(
                m_StockTicker,
                m_TimeInterval,
                m_TimeFrame,
                from_time_t((std::time_t)value[VALUE_KEY][0].GetDouble()),
                value[VALUE_KEY][1].GetFloat(),
                value[VALUE_KEY][4].GetFloat(),
                value[VALUE_KEY][2].GetFloat(),
                value[VALUE_KEY][3].GetFloat(),
                static_cast<int>(value[VALUE_KEY][5].GetDouble())
            ));
        }
    }
}

const size_t DataPointManager::CheckForDuplicates(void)
{
    size_t count = 0;
    auto result = std::adjacent_find(m_DataPoints.begin(), m_DataPoints.end(), [&](const std::shared_ptr<DataPoint>& x, const std::shared_ptr<DataPoint>& y) {
        if (x->GetPTime() == y->GetPTime() &&
            x->GetVolume() == y->GetVolume())
        {
            count++;
            return true;
        }
        return false;
    });

    return count;
}

void DataPointManager::RemoveDuplicates(void)
{
    m_DataPoints.erase(std::unique(m_DataPoints.begin(), m_DataPoints.end(), [](const std::shared_ptr<DataPoint>& x, const std::shared_ptr<DataPoint>& y) {
        if (x->GetPTime() == y->GetPTime() &&
            x->GetVolume() == y->GetVolume())
        {
            return true;
        }
        return false;
            
    }), m_DataPoints.end());
}

void DataPointManager::DisplayInfo(void)
{
    std::cout << "**************************************************" << std::endl;
    std::cout << "DataPoint Manager Display Info" << std::endl;
    std::cout << "**************************************************" << std::endl;
    std::cout << "Start Time: " << to_simple_string(m_DataPoints.front()->GetPTime()) << std::endl;
    std::cout << "End Time: " << to_simple_string(m_DataPoints.back()->GetPTime()) << std::endl;
    std::cout << "Data Points: " << m_DataPoints.size() << std::endl;
}

}
}