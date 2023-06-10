#include "DataPoint.h"

DataPoint::DataPoint()
{
}

DataPoint::~DataPoint()
{
}

void DataPoint::Display(void)
{
    std::cout
        << "Time: " << m_Time << std::endl
        << "Volume: " << m_Volume << std::endl
        << "Open: " << m_Open << std::endl
        << "Close: " << m_Close << std::endl
        << "High: " << m_High << std::endl
        << "Low: " << m_Low << std::endl << std::endl;
}
