#pragma once
#include "DataPointManager.h"

namespace TradingView {
namespace IO {

class MessageProcessor
{
	friend class Inbox;

private:
	MessageProcessor(DataPointManager& dataPointManager);
	void ProcessRawMessage(std::vector<std::shared_ptr<Message>>& messages, const std::string& message, const bool processDPs);
	void SplitMessages(std::vector<std::shared_ptr<Message>>& messages, const std::string& message);
	void ProcessMethods(std::vector<std::shared_ptr<Message>>& messages, const bool processDPs);

private:
	DataPointManager& m_DataPointManager;
};

}
}