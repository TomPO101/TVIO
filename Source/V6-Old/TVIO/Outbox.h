#pragma once
#include "Message.h"

namespace TradingView {
namespace IO {

class Outbox
{
	friend class Session;

private:
	void AddMessage(std::shared_ptr<Message>& message);
	bool SendMessages(void);

	const std::vector<std::shared_ptr<Message>>& GetPendingMessages(void) const { return m_PendingMessages; }
	const std::vector<std::shared_ptr<Message>>& GetSentMessages(void) const { return m_SentMessages; };

private:
	Outbox(Network& IO, const bool recordMessages);
	Network& m_Network;
	std::vector<std::shared_ptr<Message>> m_PendingMessages;
	std::vector<std::shared_ptr<Message>> m_SentMessages;
	const bool m_RecordMessages;
};

}
}