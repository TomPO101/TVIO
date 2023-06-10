#pragma once
#include "MessageProcessor.h"

namespace TradingView {

class Inbox
{
	friend class Session;
public:
	Inbox(TradingView::TVIO* TVIO);
	~Inbox(void);

	bool ReadMessage(void);
	void ReadMessages(std::string rawMessage);
	bool WaitForMessage(MessageType type);
	
private:
	TradingView::TVIO* m_TVIO;
	TradingView::MessageProcessor* m_MessageProcessor;
	std::vector<TradingView::Message> m_Messages;
};

}