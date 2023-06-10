#include "MessageProcessor.h"
#include <fstream>

namespace TradingView {
namespace IO {

MessageProcessor::MessageProcessor(DataPointManager& dataPointManager) :
    m_DataPointManager(dataPointManager)
{
}

void MessageProcessor::ProcessRawMessage(std::vector<std::shared_ptr<Message>>& messages, const std::string& message, const bool processDPs)
{
    SplitMessages(messages, message);
    ProcessMethods(messages, processDPs);
}

void MessageProcessor::SplitMessages(std::vector<std::shared_ptr<Message>>& messages, const std::string& message)
{
    std::string delim = METHOD_SYMBOL;
    std::string rawMsg;
    size_t start = 0;
    size_t end = 0;

    while (end != std::string::npos) {
        end = message.find(delim, start);
        rawMsg = message.substr(start, end - start);
        if (!rawMsg.empty()) {
            if (rawMsg.at(0) == '{') {
                messages.push_back(std::move(std::make_shared<Message>(rawMsg)));
            }
        }
        start = end + delim.length();
    }
}

void MessageProcessor::ProcessMethods(std::vector<std::shared_ptr<Message>>& messages, const bool processDPs)
{
    std::string method;
    for (auto& msg : messages) {
        method = msg->GetMethod();
        if (msg->GetMethod() != "") {
            if (method == RESPONSE_METHOD_TIMESCALE) {
                msg->SetType(MessageType::TIMESCALE_UPADATE);
                if (m_DataPointManager.RecieveCandidate(msg, processDPs))
                    msg->SetType(MessageType::DATA_POINT_ARRAY);
            }
            if (method == RESPONSE_METHOD_REPLAY)
                msg->SetType(MessageType::REPLAY_INSTANCE_ID);
            if (method == RESPONSE_METHOD_REPLAY_OK)
                msg->SetType(MessageType::REPLAY_OK);
            if (method == RESPONSE_METHOD_REPLAY_ERROR) {
                msg->SetType(MessageType::REPLAY_ERROR);
                std::string str = msg->GetMessage()[PARAM_KEY][2].GetString();
                if (str == POINT_TOO_DEEP)
                    msg->SetType(MessageType::POINT_TOO_DEEP_ERROR);
            }
            if (method == RESPONSE_METHOD_PROTOCOL_ERROR)
                msg->SetType(MessageType::PROTOCOL_ERROR);
            if (method == RESPONSE_METHOD_SERIES_LOADING)
                msg->SetType(MessageType::SERIES_LOADING);
        }
    }
}

}
}