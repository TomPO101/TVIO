#include "DataPointProcessor.h"

TradingView::DataPointProcessor::DataPointProcessor()
{
}

TradingView::DataPointProcessor::~DataPointProcessor()
{
}

bool TradingView::DataPointProcessor::RecieveCandidate(TradingView::Message data)
{
    nlohmann::json msg = data.GetMessage();
    if (data.FindKey(msg, PARAM_KEY))
        if (data.FindKey(msg[PARAM_KEY][1], SERIES_ONE))
            if (data.FindKey(msg[PARAM_KEY][1][SERIES_ONE], SERIES_KEY)) {
                if (msg[PARAM_KEY][1][SERIES_ONE][SERIES_KEY].size() > 50) { // to make sure its a proper data message
                    ProcessDataPoints(msg[PARAM_KEY][1][SERIES_ONE][SERIES_KEY]);
                    return true;
                } 
                return false;
            }
    return false;
}

void TradingView::DataPointProcessor::ProcessDataPoints(nlohmann::json data)
{
    for (auto it = data.begin(); it != data.end(); ++it) {
        nlohmann::json jsDP = it.value();
        if (jsDP.find(VALUE_KEY) != jsDP.end()) {
            DataPoint dp;

            dp.SetTimeStamp(jsDP.at(VALUE_KEY)[0].get<std::time_t>());
            dp.SetFormatedTime(boost::posix_time::from_time_t(dp.GetTimeStamp()) - boost::posix_time::hours(4));
            dp.SetOpen(jsDP.at(VALUE_KEY)[1].get<float>());
            dp.SetClose(jsDP.at(VALUE_KEY)[2].get<float>());
            dp.SetHigh(jsDP.at(VALUE_KEY)[3].get<float>());
            dp.SetLow(jsDP.at(VALUE_KEY)[4].get<float>());
            dp.SetVolume(jsDP.at(VALUE_KEY)[5].get<int>());

            m_DataPoints.push_back(dp);
        }
    }
}
