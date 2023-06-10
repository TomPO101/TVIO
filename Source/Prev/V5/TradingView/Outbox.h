#pragma once
#include "Message.h"

namespace TradingView {

class Outbox
{
public:
	Outbox(TradingView::TVIO& TVIO, const bool recordMessages);

	void AddMessage(const std::shared_ptr<TradingView::Message>& message);
	bool SendMessages(void);

	const std::vector<std::shared_ptr<TradingView::Message>>& GetPendingMessages(void) const { return m_PendingMessages; }
	const std::vector<std::shared_ptr<TradingView::Message>>& GetSentMessages(void) const { return m_SentMessages; };

private:
	TradingView::TVIO& m_TVIO;
	std::vector<std::shared_ptr<TradingView::Message>> m_PendingMessages;
	std::vector<std::shared_ptr<TradingView::Message>> m_SentMessages;
	const bool m_RecordMessages;
};

}