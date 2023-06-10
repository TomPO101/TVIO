#include "Inbox.h"

TradingView::Inbox::Inbox(TradingView::TVIO& TVIO, TradingView::DataPointManager& dataPointManager, bool recordMessages) :
	m_TVIO(TVIO),
	m_MessageProcessor(dataPointManager),
    m_RecordMessages(recordMessages)
{
}

TradingView::Inbox::~Inbox(void)
{
}

bool TradingView::Inbox::WaitForMessage(MessageType type, bool processDPs)
{
    std::string message;
    std::vector<std::shared_ptr<TradingView::Message>> messages;
    std::chrono::steady_clock::time_point startTime = std::chrono::high_resolution_clock::now();
    while (true) {
        if (!m_TVIO.Read(message))
            return false;

        messages.clear();
        m_MessageProcessor.ProcessRawMessage(messages, message, processDPs);
        message.clear();

        for (auto& msg : messages) {
            if (m_RecordMessages)
                m_Messages.push_back(msg);

            if (msg->GetType() != MessageType::DATA_POINT_ARRAY)
                msg->Display();
            if (msg->GetType() == MessageType::PROTOCOL_ERROR)
                return false;
            if (msg->GetType() == type)
                return true;
        }
        //std::cout << "IN WHILE";
        if ((std::chrono::high_resolution_clock::now() - startTime) > std::chrono::seconds(WAIT_FOR_MESSAGE_TIMEOUT)) {
            std::cout << "Wait For Message Timed Out" << std::endl;
            return false;
        }
    }
}