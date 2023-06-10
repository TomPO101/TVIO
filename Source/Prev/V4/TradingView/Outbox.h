#pragma once
#include "Message.h"

namespace TradingView {

class Outbox
{
public:
	Outbox(TradingView::TVIO& TVIO, bool recordMessages);
	~Outbox(void);

	void AddMessage(std::shared_ptr<TradingView::Message> message);
	bool SendMessages(void);

	std::vector<std::shared_ptr<TradingView::Message>>& GetPendingMessages(void) { return m_PendingMessages; }
	std::vector<std::shared_ptr<TradingView::Message>>& GetSentMessages(void) { return m_SentMessages; };

private:
	TradingView::TVIO& m_TVIO;
	std::vector<std::shared_ptr<TradingView::Message>> m_PendingMessages;
	std::vector<std::shared_ptr<TradingView::Message>> m_SentMessages;
	bool m_RecordMessages;
};

}