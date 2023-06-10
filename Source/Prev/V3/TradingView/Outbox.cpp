#include "Outbox.h"

TradingView::Outbox::Outbox(TradingView::TVIO& TVIO) :
	m_TVIO(TVIO),
	m_PendingMessages(),
	m_SentMessages()
{
}

TradingView::Outbox::~Outbox(void)
{
}

void TradingView::Outbox::AddMessage(TradingView::Message message)
{
	m_PendingMessages.push_back(message);
}

bool TradingView::Outbox::SendMessages(void)
{
	std::string preparedMessage;
	for (auto& msg : m_PendingMessages) {
		preparedMessage = msg.GetPreparedMessage();
		//std::cout << preparedMessage << std::endl;
		msg.Display();
		if (!m_TVIO.Write(preparedMessage)) {
			return false;
		}
		m_SentMessages.push_back(msg);
	}
	m_PendingMessages = {};
	return true;
}
