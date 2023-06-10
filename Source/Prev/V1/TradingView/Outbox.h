#pragma once
#include "Message.h"

namespace TradingView {

class Outbox
{
public:
	Outbox(TradingView::TVIO* TVIO);
	~Outbox(void);

	void AddMessage(TradingView::Message message);
	void SendMessages(void);
private:
	TradingView::TVIO* m_TVIO;
	std::vector<TradingView::Message> m_Messages;
};

}