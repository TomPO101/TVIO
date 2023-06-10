#include "DataPoint.h"

TradingView::DataPoint::DataPoint(void) :
    m_StockTicker(""),
    m_StockMarket(""),
    m_TimeInterval(""),
    m_TimeFrame(""),
    m_PTime(),
    m_Open(0.0f),
    m_Close(0.0f),
    m_High(0.0f),
    m_Low(0.0f),
    m_Volume(0)
{
}

void TradingView::DataPoint::Display(void)
{
    std::cout
        << "Time: " << m_PTime << std::endl
        << "Volume: " << m_Volume << std::endl
        << "Open: " << m_Open << std::endl
        << "Close: " << m_Close << std::endl
        << "High: " << m_High << std::endl
        << "Low: " << m_Low << std::endl << std::endl;
}
