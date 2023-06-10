#include "Inbox.h"

TradingView::Inbox::Inbox(TradingView::TVIO& TVIO, TradingView::DataPointManager& dataPointManager) :
	m_TVIO(TVIO),
	m_MessageProcessor(dataPointManager)
{
}

TradingView::Inbox::~Inbox(void)
{
}

bool TradingView::Inbox::ReadMessage(void)
{
    std::string message;
    if (!m_TVIO.Read(message)) {
        return false;
    }
	std::vector<TradingView::Message> processedMsgs = m_MessageProcessor.ProcessRawMessage(message, false);
    for (auto& msg : processedMsgs) {
        m_Messages.push_back(msg);

        if (msg.GetType() == PROTOCOL_ERROR) {
            return false;
        }
    }
	return true;
}

bool TradingView::Inbox::WaitForMessage(MessageType type, bool processDPs)
{
    while (true) {
        std::string message;

        if (!m_TVIO.Read(message))
            return false;

        std::vector<TradingView::Message> processedMsgs = m_MessageProcessor.ProcessRawMessage(message, processDPs);
        for (auto& msg : processedMsgs) {
            m_Messages.push_back(msg);
            //msg.Display();
            if (msg.GetType() == PROTOCOL_ERROR) {
                return false;
            }
            if (msg.GetType() == type)
                goto found;
        }
    }
found:
    return true;
}