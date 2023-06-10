#include "Message.h"

TradingView::Message::Message(void) :
	m_Type(MessageType::UNDEFINED),
	m_ID(0),
	m_Message(rapidjson::kObjectType)
{
	rapidjson::Value method(rapidjson::kStringType);
	m_Message.AddMember(METHOD_KEY, "", m_Message.GetAllocator());
	rapidjson::Value param(rapidjson::kArrayType);
	m_Message.AddMember(PARAM_KEY, param, m_Message.GetAllocator());
}

TradingView::Message::~Message(void)
{
}

void TradingView::Message::SetID(unsigned int ID)
{
	m_ID = ID;
}

void TradingView::Message::SetMethod(std::string method)
{
	m_Message[METHOD_KEY].SetString(method.c_str(), m_Message.GetAllocator());
}

void TradingView::Message::SetMessage(std::string& jsonStr)
{
	m_Message.Parse(jsonStr.c_str());
}

void TradingView::Message::Display(void)
{
	rapidjson::StringBuffer ps;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> prettyWriter(ps);
	m_Message.Accept(prettyWriter);

	std::cout << "ID: " << m_ID << std::endl;
	std::cout << "Message: " << ps.GetString() << std::endl;
}

std::string TradingView::Message::GetMethod(void)
{
	if (m_Message.HasMember(METHOD_KEY))
		return m_Message[METHOD_KEY].GetString();
	return "";
}

std::string TradingView::Message::GetPreparedMessage(void)
{	
	rapidjson::StringBuffer sb;
	rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
	m_Message.Accept(writer);

	std::string msgStr = sb.GetString();

	//~m~137~m~{"m":"resolve_symbol", "p" : ["cs_Qv9A6Jp44xsW", "symbol_1", "={\"symbol\":\"AMEX:SPY\",\"adjustment\":\"splits\",\"session\":\"extended\"}"] }
	//~m~150~m~{"m":"replay_add_series","p":["rs_Bf6ADOS5uexD","COLVyZY9zgWP0","={\"symbol\":\"AMEX:SPY\",\"adjustment\":\"splits\",\"session\":\"extended\"}","1S"]}
	//~m~181~m~{"m":"resolve_symbol", "p" : ["cs_oIkttoqgCRVw", "symbol_2", "={\"replay\":\"rs_Bf6ADOS5uexD\",\"symbol\":{\"symbol\":\"AMEX:SPY\",\"adjustment\":\"splits\",\"session\":\"extended\"}}"] }

	//std::cout << msgStr << std::endl;
	std::size_t pos = msgStr.find('{', 2);
	if (pos != std::string::npos) {
		bool addEndQuote = true;
		bool addBackSlashes = true;
		msgStr.insert(pos, "\"=");
		for (std::size_t i = pos + 2; i < msgStr.size(); i++) {
			if (msgStr.at(i) == '\"' && addBackSlashes) {
				msgStr.insert(i, "\\");
				i++;
			}
			else if (msgStr.at(i) == '}' && addEndQuote) {
				addBackSlashes = false;
				if (msgStr.at(i + 1) == '}')
					msgStr.insert(i + 2, "\"");
				else
					msgStr.insert(i + 1, "\"");
				addEndQuote = false;
				i++;
			}
		}
	}

	std::string msg = METHOD_SYMBOL + std::to_string(m_ID) + METHOD_SYMBOL + msgStr;
	//std::cout << msg << std::endl;
	return msg;
}