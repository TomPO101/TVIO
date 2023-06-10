#pragma once
#include "Message.h"

class DataPoint
{
public:
	DataPoint();
	~DataPoint();

    void SetTimeStamp(std::time_t timeStamp) { m_TimeStamp = timeStamp; };
    void SetFormatedTime(boost::posix_time::ptime time) { m_Time = time; };
    void SetOpen(float open) { m_Open = open; };
    void SetClose(float close) { m_Close = close; };
    void SetHigh(float high) { m_High = high; };
    void SetLow(float low) { m_Low = low; };
    void SetVolume(unsigned int volume) { m_Volume = volume; };

    std::time_t GetTimeStamp(void) { return m_TimeStamp; };
    boost::posix_time::ptime GetTime(void) { return m_Time; };

    void Display(void);

private:
    std::time_t m_TimeStamp;
    boost::posix_time::ptime m_Time;
    float m_Open;
    float m_Close;
    float m_High;
    float m_Low;
    unsigned int m_Volume;
};