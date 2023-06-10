#pragma once
#include "MessageProcessor.h"

namespace TradingView {

#define WAIT_FOR_MESSAGE_TIMEOUT (8)

class Inbox
{
public:
	Inbox(TradingView::TVIO& TVIO, TradingView::DataPointManager& dataPointManager, bool recordMessages);
	~Inbox(void);

	bool WaitForMessage(MessageType type, bool processDPs);

	TradingView::MessageProcessor& GetMessageProcessor(void) { return m_MessageProcessor; };
	std::vector<std::shared_ptr<TradingView::Message>>& GetMessages(void) { return m_Messages; };
	
private:
	TradingView::TVIO& m_TVIO;
	TradingView::MessageProcessor m_MessageProcessor;
	std::vector<std::shared_ptr<TradingView::Message>> m_Messages;
	bool m_RecordMessages;
};

}