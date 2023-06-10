#pragma once
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace TradingView {

class DataPoint
{
public:
    DataPoint(void);

    // POSIBLY PUT A VECTOR OF EMA AND OTHER INDICATORS BUT JUST DONT SAVE THEM TO DB

    void SetStockTicker(const std::string& stockTicker) { m_StockTicker = stockTicker; };
    void SetStockMarket(const std::string& stockMarket) { m_StockMarket = stockMarket; };
    void SetTimeInterval(const std::string& timeInterval) { m_TimeInterval = timeInterval; };
    void SetTimeFrame(const std::string& timeFrame) { m_TimeFrame = timeFrame; };
    void SetTimeStamp(const std::time_t& timeStamp) { m_TimeStamp = timeStamp; };
    void SetPTime(const boost::posix_time::ptime& pTime) { m_PTime = pTime; };
    void SetOpen(const float open) { m_Open = open; };
    void SetClose(const float close) { m_Close = close; };
    void SetHigh(const float high) { m_High = high; };
    void SetLow(const float low) { m_Low = low; };
    void SetVolume(const unsigned int volume) { m_Volume = volume; };

    const std::string& GetStockTicker(void) const { return m_StockTicker; };
    const std::string& GetStockMarket(void) const { return m_StockMarket; };
    const std::string& GetTimeInterval(void) const { return m_TimeInterval; };
    const std::string& GetTimeFrame(void) const { return m_TimeFrame; };
    const std::time_t& GetTimeStamp(void) const { return m_TimeStamp; };
    const boost::posix_time::ptime& GetPTime(void) const { return m_PTime; };
    const float& GetOpen(void) const { return m_Open; };
    const float& GetClose(void) const { return m_Close; };
    const float& GetHigh(void) const { return m_High; };
    const float& GetLow(void) const { return m_Low; };
    const unsigned int& GetVolume(void) const { return m_Volume; };

    void Display(void);

private:
    std::string m_StockTicker;
    std::string m_StockMarket;
    std::string m_TimeInterval;
    std::string m_TimeFrame;
    std::time_t m_TimeStamp;
    boost::posix_time::ptime m_PTime;
    float m_Open;
    float m_Close;
    float m_High;
    float m_Low;
    unsigned int m_Volume;
};

}