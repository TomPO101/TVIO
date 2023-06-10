#include "Inbox.h"

TradingView::Inbox::Inbox(TradingView::TVIO* TVIO) :
	m_TVIO(TVIO),
	m_MessageProcessor(new TradingView::MessageProcessor())
{
}

TradingView::Inbox::~Inbox(void)
{
}

bool TradingView::Inbox::ReadMessage(void)
{
	std::string rawMessage = m_TVIO->Read();
	std::vector<TradingView::Message> processedMsgs = m_MessageProcessor->ProcessRawMessage(rawMessage);
	for (auto& msg : processedMsgs)
		m_Messages.push_back(msg);
	return true;
}

bool TradingView::Inbox::WaitForMessage(MessageType type)
{
    printf("Waiting for message.....\n");
    while (true) {
        std::string rawMessage = m_TVIO->Read();

        if (rawMessage == "The I/O operation has been aborted because of either a thread exit or an application request")
            return false;

        std::vector<TradingView::Message> processedMsgs = m_MessageProcessor->ProcessRawMessage(rawMessage);
        for (auto& msg : processedMsgs) {
            m_Messages.push_back(msg);
            if (msg.GetType() == type)
                goto found;
        }
    }
found:
    return true;
}

//GET MESSAGE BY METHOD
