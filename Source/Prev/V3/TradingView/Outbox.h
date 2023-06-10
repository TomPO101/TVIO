#pragma once
#include "Message.h"

namespace TradingView {

class Outbox
{
public:
	Outbox(TradingView::TVIO& TVIO);
	~Outbox(void);

	void AddMessage(TradingView::Message message);
	bool SendMessages(void);

	std::vector<TradingView::Message>& GetPendingMessages(void) { return m_PendingMessages; }
	std::vector<TradingView::Message>& GetSentMessages(void) { return m_SentMessages; };

private:
	TradingView::TVIO& m_TVIO;
	std::vector<TradingView::Message> m_PendingMessages;
	std::vector<TradingView::Message> m_SentMessages;
};

}