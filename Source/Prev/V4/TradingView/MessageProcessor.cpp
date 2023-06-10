#include "MessageProcessor.h"
#include <fstream>

TradingView::MessageProcessor::MessageProcessor(TradingView::DataPointManager& dataPointManager) :
    m_DataPointManager(dataPointManager)
{
}

TradingView::MessageProcessor::~MessageProcessor()
{
}

void TradingView::MessageProcessor::ProcessRawMessage(std::vector<std::shared_ptr<TradingView::Message>>& messages, std::string& msg, bool processDPs)
{
    SplitMessages(messages, msg);
    ProcessMethods(messages, processDPs);
}

void TradingView::MessageProcessor::SplitMessages(std::vector<std::shared_ptr<TradingView::Message>>& messages, std::string& message)
{
    //std::ofstream myfile;
    //myfile.open(R"(C:\Code\StockTrading\Projects\TradingView\C++\TradingView\TradingView\Assets\Messages.txt)", std::ios_base::app);
    //myfile << message << "\n\n\n\n\n\n\n\n\n";
    //myfile.close();
    
    std::string delim = METHOD_SYMBOL;
    std::string rawMsg;
    int start = 0;
    int end = 0;
    unsigned int ID = 0;

    while (end != std::string::npos) {
        end = message.find(delim, start);
        rawMsg = message.substr(start, end - start);
        if (!rawMsg.empty()) {
            if (rawMsg.at(0) == '{') {
                std::shared_ptr<TradingView::Message> msg(std::make_shared<TradingView::Message>());
                msg->SetID(ID);
                msg->SetMessage(rawMsg);
                messages.push_back(msg);
            }
            else if (std::find_if(rawMsg.begin(), rawMsg.end(), [](unsigned char c) { return !std::isdigit(c); }) == rawMsg.end())
                ID = std::stoi(rawMsg);
        }
        start = end + delim.length();
    }
}

void TradingView::MessageProcessor::ProcessMethods(std::vector<std::shared_ptr<TradingView::Message>>& messages, bool processDPs)
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
            if (method == RESPONSE_METHOD_REPLAY_ERROR)
                msg->SetType(MessageType::REPLAY_ERROR);
            if (method == RESPONSE_METHOD_PROTOCOL_ERROR)
                msg->SetType(MessageType::PROTOCOL_ERROR);
            if (method == RESPONSE_METHOD_SERIES_LOADING)
                msg->SetType(MessageType::SERIES_LOADING);
        }
    }
    
}
