#pragma once
#include "MessageProcessor.h"

namespace TradingView {

#define WAIT_FOR_MESSAGE_TIMEOUT (8)
#define INBOX_OK ("Inbox Ok")
#define INBOX_IO_ERROR ("Inbox Ok IO Error")
#define INBOX_TIMEOUT_ERROR ("Inbox Ok Timeout Error")
#define INBOX_PROROCOL_ERROR ("Inbox Prorocol Error")
#define INBOX_POINT_TOO_DEEP_ERROR ("Inbox Point Too Deep Error")

class Inbox
{
public:
	Inbox(TradingView::TVIO& TVIO, TradingView::DataPointManager& dataPointManager, const bool recordMessages);

	std::string WaitForMessage(const MessageType type, const bool processDPs);

	TradingView::MessageProcessor& GetMessageProcessor(void) { return m_MessageProcessor; };
	std::vector<std::shared_ptr<TradingView::Message>>& GetMessages(void) { return m_Messages; };
	
private:
	TradingView::TVIO& m_TVIO;
	TradingView::MessageProcessor m_MessageProcessor;
	std::vector<std::shared_ptr<TradingView::Message>> m_Messages;
	const bool m_RecordMessages;
};

}