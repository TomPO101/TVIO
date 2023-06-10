#include "Inbox.h"

namespace TradingView {
namespace IO {

Inbox::Inbox(Network& IO, DataPointManager& dataPointManager, const bool recordMessages) :
	m_Network(IO),
	m_MessageProcessor(dataPointManager),
    m_Messages(),
    m_RecordMessages(recordMessages)
{
}

std::string Inbox::WaitForMessage(const MessageType type, const bool processDPs)
{
    const std::chrono::steady_clock::time_point startTime = std::chrono::high_resolution_clock::now();

    std::string message;
    std::vector<std::shared_ptr<Message>> messages;
    
    while (true) {
        // needs returning rather than passing by ref and moving in tvio
        if (!m_Network.Read(message))
            return INBOX_IO_ERROR;

        messages.clear();
        m_MessageProcessor.ProcessRawMessage(messages, message, processDPs);
        message.clear();

        std::string error;
        bool found = false;

        for (auto& msg : messages) {
            if (m_RecordMessages)
                m_Messages.push_back(std::move(msg));

            //if (msg->GetType() != MessageType::DATA_POINT_ARRAY)
                //msg->Display();

            if (msg->GetType() == MessageType::PROTOCOL_ERROR)
                error = INBOX_PROTOCOL_ERROR;
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

}
}