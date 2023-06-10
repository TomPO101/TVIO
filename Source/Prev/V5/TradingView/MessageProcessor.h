#pragma once
#include "DataPointManager.h"

namespace TradingView {

class MessageProcessor
{
public:
	MessageProcessor(TradingView::DataPointManager& dataPointManager);

	void ProcessRawMessage(std::vector<std::shared_ptr<TradingView::Message>>& messages, const std::string& message, const bool processDPs);

private:
	void SplitMessages(std::vector<std::shared_ptr<TradingView::Message>>& messages, const std::string& message);
	void ProcessMethods(std::vector<std::shared_ptr<TradingView::Message>>& messages, const bool processDPs);

private:
	TradingView::DataPointManager& m_DataPointManager;
};

}