#include "DataPointManager.h"
#include "Log.h"

TradingView::DataPointManager::DataPointManager(TradingView::Log& log, std::string stockMarket, std::string stockTicker, std::string timeInterval, std::string timeFrame) :
    m_Log(log),
    m_DataPoints(),
    m_StockMarket(stockMarket),
    m_StockTicker(stockTicker),
    m_TimeInterval(timeInterval),
    m_TimeFrame(timeFrame),
    m_LastEarliestDPTime(0),
    m_LastCollectedDPs(0)
{
}

TradingView::DataPointManager::~DataPointManager(void)
{
}

bool TradingView::DataPointManager::RecieveCandidate(std::shared_ptr<TradingView::Message>& data, bool processDPs)
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
        std::vector<DataPoint> dps;
        boost::posix_time::ptime today = (boost::posix_time::second_clock::local_time() - boost::posix_time::hours(4)) - boost::posix_time::hours(24);
        boost::posix_time::ptime time;
        for (auto& dp : m_DataPoints) {
            time = dp.GetPTime();
            if (time.date() == today.date() &&
                time.time_of_day() >= boost::posix_time::time_duration(4, 0, 0) &&
                time.time_of_day() <= boost::posix_time::time_duration(19, 59, 59)) {

                dps.push_back(dp);
            }
        }

        m_DataPoints = dps;

        if (!dps.empty()) {
            m_Log.Add(LOG_TRUNCATED_RESULTS, "Data Points", std::to_string(dps.size()));
            m_Log.Add(LOG_TRUNCATED_RESULTS, "Data Point Start Date", boost::posix_time::to_simple_string(dps.front().GetPTime()));
            m_Log.Add(LOG_TRUNCATED_RESULTS, "Data Points End Date", boost::posix_time::to_simple_string(dps.back().GetPTime()));
            //OrderByDate();
        }
        else m_Log.Add(LOG_SESSION_ERROR, "Truncated Data Points was empty", "");
    }
    else m_Log.Add(LOG_SESSION_ERROR, "No Data Points to Truncate", "");
}

void TradingView::DataPointManager::OrderByDate(void)
{
    std::sort(m_DataPoints.begin(), m_DataPoints.end(), [](DataPoint& x, DataPoint& y) {
    	return x.GetPTime() < y.GetPTime();
    });
}

void TradingView::DataPointManager::ProcessDataPoints(rapidjson::Value& data)
{
    DataPoint dp;

    std::cout << std::endl << data.Size() << std::endl;
    std::cout << boost::posix_time::from_time_t((std::time_t)(data[0][VALUE_KEY][0].GetDouble())) - boost::posix_time::hours(4) << std::endl;
    std::cout << boost::posix_time::from_time_t((std::time_t)(data[data.Size()-1][VALUE_KEY][0].GetDouble())) - boost::posix_time::hours(4) << std::endl << std::endl;

    m_LastEarliestDPTime = (std::time_t)(data[0][VALUE_KEY][0].GetDouble());
    m_LastCollectedDPs = data.GetArray().Size();

    for (auto& value : data.GetArray()) {
        if (value.HasMember(VALUE_KEY)) {
            dp.SetStockMarket(m_StockMarket);
            dp.SetStockTicker(m_StockTicker);
            dp.SetTimeInterval(m_TimeInterval);
            dp.SetTimeFrame(m_TimeFrame);
            dp.SetPTime(boost::posix_time::from_time_t((std::time_t)(value[VALUE_KEY][0].GetDouble())) - boost::posix_time::hours(4));
            dp.SetOpen(value[VALUE_KEY][1].GetFloat());
            dp.SetClose(value[VALUE_KEY][4].GetFloat());
            dp.SetHigh(value[VALUE_KEY][2].GetFloat());
            dp.SetLow(value[VALUE_KEY][3].GetFloat());
            dp.SetVolume(static_cast<unsigned int>(value[VALUE_KEY][5].GetDouble()));

            m_DataPoints.push_back(dp);
        }
    }
}