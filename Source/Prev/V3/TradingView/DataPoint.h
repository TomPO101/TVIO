#pragma once
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace TradingView {

class DataPoint
{
public:
    DataPoint();
    ~DataPoint();

    // POSIBLY PUT A VECTOR OF EMA AND OTHER INDICATORS BUT JUST DONT SAVE THEM TO DB

    void SetStockTicker(std::string stockTicker) { m_StockTicker = stockTicker; };
    void SetStockMarket(std::string stockMarket) { m_StockMarket = stockMarket; };
    void SetTimeInterval(std::string timeInterval) { m_TimeInterval = timeInterval; };
    void SetTimeFrame(std::string timeFrame) { m_TimeFrame = timeFrame; };
    void SetPTime(boost::posix_time::ptime time) { m_PTime = time; };
    void SetOpen(float open) { m_Open = open; };
    void SetClose(float close) { m_Close = close; };
    void SetHigh(float high) { m_High = high; };
    void SetLow(float low) { m_Low = low; };
    void SetVolume(unsigned int volume) { m_Volume = volume; };

    std::string GetStockTicker(void) { return m_StockTicker; };
    std::string GetStockMarket(void) { return m_StockMarket; };
    std::string GetTimeInterval(void) { return m_TimeInterval; };
    std::string GetTimeFrame(void) { return m_TimeFrame; };
    std::time_t GetTimeStamp(void) { return to_time_t(m_PTime); };
    boost::posix_time::ptime& GetPTime(void) { return m_PTime; };
    float GetOpen(void) { return m_Open; };
    float GetClose(void) { return m_Close; };
    float GetHigh(void) { return m_High; };
    float GetLow(void) { return m_Low; };
    unsigned int GetVolume(void) { return m_Volume; };

    void Display(void);

private:
    std::string m_StockTicker;
    std::string m_StockMarket;
    std::string m_TimeInterval;
    std::string m_TimeFrame;
    boost::posix_time::ptime m_PTime;
    float m_Open;
    float m_Close;
    float m_High;
    float m_Low;
    unsigned int m_Volume;
};

}