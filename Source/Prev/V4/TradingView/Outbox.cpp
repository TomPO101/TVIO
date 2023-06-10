#include "Outbox.h"

TradingView::Outbox::Outbox(TradingView::TVIO& TVIO, bool recordMessages) :
	m_TVIO(TVIO),
	m_PendingMessages(),
	m_SentMessages(),
	m_RecordMessages(recordMessages)
{
}

TradingView::Outbox::~Outbox(void)
{
}

void TradingView::Outbox::AddMessage(std::shared_ptr<TradingView::Message> message)
{
	m_PendingMessages.push_back(message);
}

bool TradingView::Outbox::SendMessages(void)
{
	std::string preparedMessage;
	for (auto& msg : m_PendingMessages) {
		preparedMessage = msg->GetPreparedMessage();
		//msg->Display();
		if (!m_TVIO.Write(preparedMessage)) {
			return false;
		}
		if (m_RecordMessages)
			m_SentMessages.push_back(msg);
	}
	m_PendingMessages = {};
	return true;
}
