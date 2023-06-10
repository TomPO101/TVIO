#include "Message.h"

TradingView::Message::Message(unsigned int ID, nlohmann::json message, TradingView::MessageType type) :
	m_ID(ID),
	m_Message(message),
	m_Type(type)
{
}

TradingView::Message::Message(void)
{
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
	m_Message[METHOD_KEY] = method;
}

void TradingView::Message::SetParameters(nlohmann::json param)
{
	m_Message = param;
}

void TradingView::Message::Display(void)
{
	std::cout << "ID: " << m_ID << std::endl;
	std::cout << "Params: " << m_Message.dump(4) << std::endl;
}

bool TradingView::Message::FindKey(nlohmann::json js, std::string key)
{
	return js.find(key) != js.end();
}

std::string TradingView::Message::GetMethod(void)
{
	if (FindKey(m_Message, "m"))
		return m_Message.at("m").get<std::string>();
	return "";
}

std::string TradingView::Message::GetPreparedMessage(void)
{	
	// add writer and check if the message type is resilve symbol or any message that needs the = char or \ slashes
	// try without backslshes first
	std::string msg = "~m~" + std::to_string(m_ID) + "~m~" + m_Message.dump();
	return msg;
}