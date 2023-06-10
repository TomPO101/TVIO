#pragma once
#include "DataPointProcessor.h"

namespace TradingView {

class MessageProcessor
{
	friend class Session;
public:
	MessageProcessor();
	~MessageProcessor();

	std::vector<TradingView::Message> ProcessRawMessage(std::string msg);

private:
	std::vector<TradingView::Message> SplitMessages(std::string message);
	void ProcessMethods(TradingView::Message& msg);

private:
	DataPointProcessor* m_DPProcessor;
};

}