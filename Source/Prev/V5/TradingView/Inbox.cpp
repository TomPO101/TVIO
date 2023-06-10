#include "Inbox.h"

TradingView::Inbox::Inbox(TradingView::TVIO& TVIO, TradingView::DataPointManager& dataPointManager, const bool recordMessages) :
	m_TVIO(TVIO),
	m_MessageProcessor(dataPointManager),
    m_RecordMessages(recordMessages)
{
}

std::string TradingView::Inbox::WaitForMessage(const MessageType type, const bool processDPs)
{
    const std::chrono::steady_clock::time_point startTime = std::chrono::high_resolution_clock::now();

    std::string message;
    std::vector<std::shared_ptr<TradingView::Message>> messages;
    
    while (true) {
        if (!m_TVIO.Read(message))
            return INBOX_IO_ERROR;

        messages.clear();
        m_MessageProcessor.ProcessRawMessage(messages, message, processDPs);
        message.clear();

        std::string error = "";
        bool found = false;

        for (const auto& msg : messages) {
            if (m_RecordMessages)
                m_Messages.push_back(msg);

            //if (msg->GetType() != MessageType::DATA_POINT_ARRAY)
                //msg->Display();

            if (msg->GetType() == MessageType::PROTOCOL_ERROR)
                error = INBOX_PROROCOL_ERROR;
            if (msg->GetType() == MessageType::POINT_TOO_DEEP_ERROR)
                error = INBOX_POINT_TOO_DEEP_ERROR;

            if (msg->GetType() == type)
                found = true;
        }

        if (error != "")
            return error;
        else if (found) 
            return INBOX_OK;

        if ((std::chrono::high_resolution_clock::now() - startTime) > std::chrono::seconds(WAIT_FOR_MESSAGE_TIMEOUT)) {
            //std::cout << "Wait For Message Timed Out" << std::endl;
            return INBOX_TIMEOUT_ERROR;
        }
    }
}