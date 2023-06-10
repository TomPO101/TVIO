#pragma once
#include "MessageProcessor.h"

namespace TradingView {

class Inbox
{
public:
	Inbox(TradingView::TVIO& TVIO, TradingView::DataPointManager& dataPointManager);
	~Inbox(void);

	bool ReadMessage(void);
	void ReadMessages(std::string rawMessage);
	bool WaitForMessage(MessageType type, bool processDPs);

	TradingView::MessageProcessor& GetMessageProcessor(void) { return m_MessageProcessor; };
	std::vector<TradingView::Message>& GetMessages(void) { return m_Messages; };
	
private:
	TradingView::TVIO& m_TVIO;
	TradingView::MessageProcessor m_MessageProcessor;
	std::vector<TradingView::Message> m_Messages;
};

}