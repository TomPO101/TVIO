#include "Outbox.h"

namespace TradingView {
namespace IO {

Outbox::Outbox(const bool recordMessages) :
	m_Log("Outbox"),
	m_PendingMessages(),
	m_SentMessages(),
	m_RecordMessages(recordMessages)
{
}

void Outbox::AddMessage(std::shared_ptr<Message>& message)
{
	m_PendingMessages.push_back(std::move(message));
}

bool Outbox::SendMessages(const int& id)
{
	std::string preparedMessage;
	for (auto& msg : m_PendingMessages) {
		//msg.Display();
		if (!Network::Write(msg->GetPreparedMessage(), id)) {
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