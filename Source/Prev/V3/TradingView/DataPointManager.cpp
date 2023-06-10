#include "DataPointManager.h"
#include "Log.h"

TradingView::DataPointManager::DataPointManager(TradingView::Log& log, std::string stockMarket, std::string stockTicker, std::string timeInterval, std::string timeFrame) :
    m_Log(log),
    m_DataPoints(),
    m_StockMarket(stockMarket),
    m_StockTicker(stockTicker),
    m_TimeInterval(timeInterval),
    m_TimeFrame(timeFrame)
{
}

TradingView::DataPointManager::~DataPointManager(void)
{
}

bool TradingView::DataPointManager::RecieveCandidate(TradingView::Message& data, bool processDPs)
{
    if (data.FindKey(data.GetMessage(), PARAM_KEY))
        if (data.FindKey(data.GetMessage()[PARAM_KEY][1], SERIES_ONE))
            if (data.FindKey(data.GetMessage()[PARAM_KEY][1][SERIES_ONE], SERIES_KEY)) {
                if (data.GetMessage()[PARAM_KEY][1][SERIES_ONE][SERIES_KEY].size() > 50) { // to make sure its a proper data message
                    if (processDPs)
                        ProcessDataPoints(data.GetMessage()[PARAM_KEY][1][SERIES_ONE][SERIES_KEY]);
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
            OrderByDate();
        }
        else m_Log.Add(LOG_SESSION_ERROR, "Truncated Data Points was empty", "");
    }
    else m_Log.Add(LOG_SESSION_ERROR, "No Data Points to Truncate", "");
}

void TradingView::DataPointManager::OrderByDate(void)
{
    std::sort(m_DataPoints.begin(), m_DataPoints.end(), [](DataPoint x, DataPoint y) {
    	return x.GetPTime() < y.GetPTime();
    });
}

void TradingView::DataPointManager::ProcessDataPoints(nlohmann::json& data)
{
    nlohmann::json jsDP;
    DataPoint dp;

    std::cout << std::endl << data.size() << std::endl;
    std::cout << boost::posix_time::from_time_t(data.front().at(VALUE_KEY)[0].get<std::time_t>()) - boost::posix_time::hours(4) << std::endl;
    std::cout << boost::posix_time::from_time_t(data.back().at(VALUE_KEY)[0].get<std::time_t>()) - boost::posix_time::hours(4) << std::endl << std::endl;

    for (auto it = data.begin(); it != data.end(); ++it) {

        jsDP = it.value();

        if (jsDP.find(VALUE_KEY) != jsDP.end()) {
            dp.SetStockMarket(m_StockMarket);
            dp.SetStockTicker(m_StockTicker);
            dp.SetTimeInterval(m_TimeInterval);
            dp.SetTimeFrame(m_TimeFrame);
            dp.SetPTime(boost::posix_time::from_time_t(jsDP.at(VALUE_KEY)[0].get<std::time_t>()) - boost::posix_time::hours(4));
            dp.SetOpen(jsDP.at(VALUE_KEY)[1].get<float>());
            dp.SetClose(jsDP.at(VALUE_KEY)[4].get<float>());
            dp.SetHigh(jsDP.at(VALUE_KEY)[2].get<float>());
            dp.SetLow(jsDP.at(VALUE_KEY)[3].get<float>());
            dp.SetVolume(jsDP.at(VALUE_KEY)[5].get<int>());

            m_DataPoints.push_back(dp);
        }
    }
}