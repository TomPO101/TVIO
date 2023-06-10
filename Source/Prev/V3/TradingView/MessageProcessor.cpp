#include "MessageProcessor.h"
#include <fstream>

TradingView::MessageProcessor::MessageProcessor(TradingView::DataPointManager& dataPointManager) :
    m_DataPointManager(dataPointManager)
{
}

TradingView::MessageProcessor::~MessageProcessor()
{
}

std::vector<TradingView::Message> TradingView::MessageProcessor::ProcessRawMessage(std::string& msg, bool processDPs)
{
    std::vector<TradingView::Message> splitMsgs = SplitMessages(msg);

    for (auto& m : splitMsgs) {
        ProcessMethods(m, processDPs);
        //if (m.GetType() == DATA_POINT_ARRAY)
        //    m.Display();
    }

	return splitMsgs;
}

std::vector<TradingView::Message> TradingView::MessageProcessor::SplitMessages(std::string message)
{
    std::ofstream myfile;
    myfile.open(R"(C:\Code\StockTrading\Projects\TradingView\C++\TradingView\TradingView\Assets\Messages.txt)", std::ios_base::app);
    myfile << message << "\n\n\n\n\n\n\n\n\n";
    myfile.close();

    std::vector<TradingView::Message> jsonMsgs;
    TradingView::Message msg;
    std::string delim = "~m~";
    std::string rawMsg;
    nlohmann::json js;
    int start = 0;
    int end = 0;
    while (end != std::string::npos) {
        end = message.find(delim, start);
        rawMsg = message.substr(start, end - start);
        if (!rawMsg.empty()) {
            if (rawMsg.at(0) == '{') {

                std::chrono::steady_clock::time_point startTime = std::chrono::high_resolution_clock::now();
                js = nlohmann::json::parse(rawMsg);
                std::cout << "Json Parse Time: " << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - startTime).count() << std::endl;

                msg.SetParameters(js);
                jsonMsgs.push_back(msg);
            }
            else if (std::find_if(rawMsg.begin(), rawMsg.end(), [](unsigned char c) { return !std::isdigit(c); }) == rawMsg.end()) {
                msg.SetID(std::stoi(rawMsg));
            }
        }
        start = end + delim.length();
    }
    return jsonMsgs;
}

void TradingView::MessageProcessor::ProcessMethods(TradingView::Message& msg, bool processDPs)
{
    std::string method = msg.GetMethod();
    if (method == RESPONSE_METHOD_TIMESCALE) {
        msg.SetType(MessageType::TIMESCALE_UPADATE);
        if (m_DataPointManager.RecieveCandidate(msg, processDPs))
            msg.SetType(MessageType::DATA_POINT_ARRAY);
    }
    if (method == RESPONSE_METHOD_REPLAY)
        msg.SetType(MessageType::REPLAY_INSTANCE_ID); 
    if (method == RESPONSE_METHOD_REPLAY_OK)
        msg.SetType(MessageType::REPLAY_OK);
    if (method == RESPONSE_METHOD_REPLAY_ERROR)
        msg.SetType(MessageType::REPLAY_ERROR); 
    if (method == RESPONSE_METHOD_PROTOCOL_ERROR)
        msg.SetType(MessageType::PROTOCOL_ERROR);
    if (method == RESPONSE_METHOD_SERIES_LOADING)
        msg.SetType(MessageType::SERIES_LOADING); 
    
}
