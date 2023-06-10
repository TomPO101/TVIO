#include "Outbox.h"

namespace TradingView {
namespace IO {

Outbox::Outbox(Network& IO, const bool recordMessages) :
	m_Network(IO),
	m_PendingMessages(),
	m_SentMessages(),
	m_RecordMessages(recordMessages)
{
}

void Outbox::AddMessage(std::shared_ptr<Message>& message)
{
	m_PendingMessages.push_back(std::move(message));
}

bool Outbox::SendMessages(void)
{
	std::string preparedMessage;
	for (auto& msg : m_PendingMessages) {
		//msg.Display();
		if (!m_Network.Write(msg->GetPreparedMessage())) {
			return false;
		}
		if (m_RecordMessages)
			m_SentMessages.push_back(std::move(msg));
	}
	m_PendingMessages.clear();
	return true;
}

}
}