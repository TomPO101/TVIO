#include "Outbox.h"

TradingView::Outbox::Outbox(TradingView::TVIO* TVIO) :
	m_TVIO(TVIO)
{
}

TradingView::Outbox::~Outbox(void)
{
}

void TradingView::Outbox::AddMessage(TradingView::Message message)
{
	m_Messages.push_back(message);
}

void TradingView::Outbox::SendMessages(void)
{
	for (auto& msg : m_Messages) {
		std::string preparedMessage = msg.GetPreparedMessage();
		std::cout << preparedMessage << std::endl;
		m_TVIO->Write(preparedMessage);
	}

	m_Messages = {};
}
