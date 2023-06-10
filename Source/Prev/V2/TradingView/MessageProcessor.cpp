#include "MessageProcessor.h"

TradingView::MessageProcessor::MessageProcessor() :
    m_DPProcessor(new TradingView::DataPointProcessor())
{
}

TradingView::MessageProcessor::~MessageProcessor()
{
}

std::vector<TradingView::Message> TradingView::MessageProcessor::ProcessRawMessage(std::string msg)
{
	//std::cout << std::endl << msg << std::endl;

    std::vector<TradingView::Message> splitMsgs = SplitMessages(msg);

    for (auto& m : splitMsgs) {
        ProcessMethods(m);
        if (m.GetType() != DATA_POINT_ARRAY)
            m.Display();
    }

	return splitMsgs;
}

std::vector<TradingView::Message> TradingView::MessageProcessor::SplitMessages(std::string message)
{
    std::vector<TradingView::Message> jsonMsgs;
    std::string delim = "~m~";
    int start = 0;
    int end = 0;
    while (end != std::string::npos) {
        end = message.find(delim, start);
        std::string rawMsg = message.substr(start, end - start);

        if (nlohmann::json::accept(rawMsg)) {
            TradingView::Message msg;

            if (rawMsg.at(0) == '{') {
                nlohmann::json js = nlohmann::json::parse(rawMsg);
                std::string params = js.dump();
                
                msg.SetParameters(js);

                jsonMsgs.push_back(msg);
            } 
            else {
                msg.SetID(std::stoi(rawMsg));
            }
        }
        start = end + delim.length();
    }
    return jsonMsgs;
}

void TradingView::MessageProcessor::ProcessMethods(TradingView::Message& msg)
{
    std::string method = msg.GetMethod();
    if (method == RESPONSE_METHOD_TIMESCALE) {
        msg.SetType(MessageType::TIMESCALE_UPADATE);
        if (m_DPProcessor->RecieveCandidate(msg))
            msg.SetType(MessageType::DATA_POINT_ARRAY);
    }
    if (method == RESPONSE_METHOD_REPLAY)
        msg.SetType(MessageType::REPLAY_INSTANCE_ID); 
    if (method == RESPONSE_METHOD_REPLAY_OK)
        msg.SetType(MessageType::REPLAY_OK);
    if (method == RESPONSE_METHOD_REPLAY_ERROR)
        msg.SetType(MessageType::REPLAY_ERROR);
    if (method == RESPONSE_METHOD_SERIES_LOADING)
        msg.SetType(MessageType::SERIES_LOADING); 
    
}
