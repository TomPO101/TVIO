#pragma once
#include "DataPointManager.h"

namespace TradingView {

class MessageProcessor
{
public:
	MessageProcessor(TradingView::DataPointManager& dataPointManager);
	~MessageProcessor(void);

	void ProcessRawMessage(std::vector<std::shared_ptr<TradingView::Message>>& messages, std::string& msg, bool processDPs);

private:
	void SplitMessages(std::vector<std::shared_ptr<TradingView::Message>>& messages, std::string& message);
	void ProcessMethods(std::vector<std::shared_ptr<TradingView::Message>>& messages, bool processDPs);

private:
	TradingView::DataPointManager& m_DataPointManager;
};

}