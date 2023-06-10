#pragma once
#include "DataPoint.h"

namespace TradingView {

class DataPointProcessor
{
	friend class Session;
public:
	DataPointProcessor();
	~DataPointProcessor();

	bool RecieveCandidate(TradingView::Message data);
	std::vector<DataPoint>& GetDataPoints(void) { return m_DataPoints; };

private:
	void ProcessDataPoints(nlohmann::json dp);

private:
	std::vector<DataPoint> m_DataPoints;
};

}