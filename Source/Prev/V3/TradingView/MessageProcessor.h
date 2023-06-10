#pragma once
#include "DataPointManager.h"

namespace TradingView {

class MessageProcessor
{
public:
	MessageProcessor(TradingView::DataPointManager& dataPointManager);
	~MessageProcessor(void);

	std::vector<TradingView::Message> ProcessRawMessage(std::string& msg, bool processDPs);

private:
	std::vector<TradingView::Message> SplitMessages(std::string message);
	void ProcessMethods(TradingView::Message& msg, bool processDPs);

private:
	TradingView::DataPointManager& m_DataPointManager;
};

}