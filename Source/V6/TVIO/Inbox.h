#pragma once
#include "MessageProcessor.h"

namespace TradingView {
namespace IO {

#define WAIT_FOR_MESSAGE_TIMEOUT (8)
#define INBOX_OK ("Inbox Ok")
#define INBOX_IO_ERROR ("Inbox Ok IO Error")
#define INBOX_TIMEOUT_ERROR ("Inbox Ok Timeout Error")
#define INBOX_PROTOCOL_ERROR ("Inbox Protocol Error")
#define INBOX_POINT_TOO_DEEP_ERROR ("Inbox Point Too Deep Error")

class Inbox
{
	friend class Session;

private:
	Inbox(DataPointManager& dataPointManager, const bool recordMessages);

	std::string WaitForMessage(const MessageType type, const bool processDPs, const int& id);

	MessageProcessor& GetMessageProcessor(void) { return m_MessageProcessor; };
	std::vector<std::shared_ptr<Message>>& GetMessages(void) { return m_Messages; };
	
private:
	Log m_Log;
	MessageProcessor m_MessageProcessor;
	std::vector<std::shared_ptr<Message>> m_Messages;
	const bool m_RecordMessages;
};

}
}