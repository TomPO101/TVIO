#include "Message.h"

TradingView::Message::Message(void) :
	m_Type(MessageType::UNDEFINED),
	m_Message(rapidjson::kObjectType)
{
	m_Message.AddMember(METHOD_KEY, "", m_Message.GetAllocator());
	rapidjson::Value param(rapidjson::kArrayType);
	m_Message.AddMember(PARAM_KEY, param, m_Message.GetAllocator());
}

void TradingView::Message::SetMethod(const std::string& method)
{
	m_Message[METHOD_KEY].SetString(method.c_str(), m_Message.GetAllocator());
}

void TradingView::Message::SetMessage(const std::string& jsonStr)
{
	m_Message.Parse(jsonStr.c_str());
}

void TradingView::Message::Display(void)
{
	rapidjson::StringBuffer ps;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> prettyWriter(ps);
	m_Message.Accept(prettyWriter);
	std::cout << "Message: " << ps.GetString() << std::endl;
}

const std::string TradingView::Message::GetMethod(void) const
{
	if (m_Message.HasMember(METHOD_KEY))
		return m_Message[METHOD_KEY].GetString();
	return "";
}

const std::string TradingView::Message::GetPreparedMessage(void)
{	
	rapidjson::StringBuffer sb;
	rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
	m_Message.Accept(writer);

	std::string msgStr = sb.GetString();
	const std::size_t pos = msgStr.find('{', 2);

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

	return METHOD_SYMBOL + std::to_string(msgStr.size()) + METHOD_SYMBOL + msgStr;
}