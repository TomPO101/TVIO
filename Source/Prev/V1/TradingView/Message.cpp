#include "Message.h"

TradingView::Message::Message(unsigned int ID, nlohmann::json message, MessageType type=MessageType::UNDEFINED) :
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
	//if (!IsKey(m_Message, METHOD_SYMBOL))
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
	std::string msg = "~m~" + std::to_string(m_ID) + "~m~" + m_Message.dump();
	return msg;
}