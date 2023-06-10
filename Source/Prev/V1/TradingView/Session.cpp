#include "Session.h"

TradingView::Session::Session(TradingView::TVIO* TVIO, std::string timeFrame, std::string timeGroup) :
	m_TVIO(TVIO),
	m_Outbox(new TradingView::Outbox(m_TVIO)),
	m_Inbox(new TradingView::Inbox(m_TVIO)),
	m_ChartSessionID("cs_lEGq9nskjz0v"),
	m_ChartReplaySessionID("rs_ijFvZWzw6wKh"),
	m_ChartReplaySeriesID("551EIYmNx43D0"),
	m_TimeFrame(timeFrame),
	m_TimeGroup(timeGroup)
{
	if (m_TimeGroup == TIMEGROUP_SECOND) {
		m_CreateSeriesID = CHART_SERIES_SECOND_ID;
		m_ReplayAddSeriesID = CHART_REPLAY_ADD_SERIES_SECOND_ID;
		m_ModifySeriesID = CHART_MODIFY_SERIES_SECOND_ID;
	}
}

TradingView::Session::~Session(void)
{
}

void TradingView::Session::Start(void)
{
	if (!ConnectToTradingView())
		return;

	CreateSession();

	ShowOverview();

	RequestMoreData(CHART_MORE_DATA_EXTRA_LARGE_ID, 300000);

	ExtendDataWithReplay();

	ShowOverview();

	DisconnectFromTradingView();
}

bool TradingView::Session::ConnectToTradingView(void)
{
	return m_TVIO->Connect();
}

bool TradingView::Session::DisconnectFromTradingView(void)
{
	return m_TVIO->Disconnect();
}

void TradingView::Session::CreateSession(void)
{
	m_Inbox->ReadMessage();
	std::cout << "CREATING STUDY:" << std::endl;

	TradingView::Message setAuth;
	setAuth.SetID(AUTH_ID);
	setAuth.SetMethod(AUTH_METHOD);
	setAuth.AddParameter<std::string>("eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzUxMiJ9.eyJ1c2VyX2lkIjo1NjM1MDExLCJleHAiOjE1OTU1OTExNDIsImlhdCI6MTU5NTUwNDc0MiwicGxhbiI6InByb19wcmVtaXVtIiwiZXh0X2hvdXJzIjoxLCJwZXJtIjoiYW1leCxueXNlLG5hc2RhcSIsInN0dWR5X3Blcm0iOiJ0di1wcm9zdHVkaWVzLHR2LXZvbHVtZWJ5cHJpY2UiLCJtYXhfc3R1ZGllcyI6MjUsIm1heF9mdW5kYW1lbnRhbHMiOjB9.cLWxq-MGOQMQfEu51hTiKZ5HwmL6g0PRp65Yt-VAui7hA12TeazAId1T9xMIdgP46pXh5gIclpY0S_Ir21pP57_Jufw-Rz1LPfSa0L7Dbho73x6oNJxztRz4X2iYkNoccAos3oCzHmzb90J9Elz4rAstiv3wkj9Fh7_EcmyeQLc");

	m_Outbox->AddMessage(setAuth);

	TradingView::Message startSession;
	startSession.SetID(CHART_SESSION_ID);
	startSession.SetMethod(CHART_SESSION_METHOD);
	startSession.AddParameter<std::string>(m_ChartSessionID);
	startSession.AddParameter<std::string>("");

	m_Outbox->AddMessage(startSession);

	TradingView::Message switchTimeZone;
	switchTimeZone.SetID(CHART_SWITCH_TIMEZONE_ID);
	switchTimeZone.SetMethod(CHART_SWITCH_TIMEZONE_METHOD);
	switchTimeZone.AddParameter<std::string>(m_ChartSessionID);
	switchTimeZone.AddParameter<std::string>(NEW_YORK_TIMEZONE);

	m_Outbox->AddMessage(switchTimeZone);

	TradingView::Message resolveSymbol;
	resolveSymbol.SetID(CHART_RESOLVE_SYMBOL_ID);
	resolveSymbol.SetMethod(CHART_RESOLVE_SYMBOL_METHOD);
	resolveSymbol.AddParameter<std::string>(m_ChartSessionID);
	resolveSymbol.AddParameter<std::string>(SYMBOL_ONE);

	nlohmann::json paramJs;
	paramJs[SYMBOL] = STOCK_TICKER_SPY;
	paramJs[ADJUSTMENT] = SPLITS;
	paramJs[SESSION] = EXTENDED;
	std::string param = paramJs.dump();
	param.insert(0, "=");

	resolveSymbol.AddParameter<std::string>(param);

	m_Outbox->AddMessage(resolveSymbol);

	TradingView::Message createSeries;
	createSeries.SetID(m_CreateSeriesID);
	createSeries.SetMethod(CHART_SERIES_METHOD);
	createSeries.AddParameter<std::string>(m_ChartSessionID);
	createSeries.AddParameter<std::string>(SERIES_ONE);
	createSeries.AddParameter<std::string>(SERIES_ONE);
	createSeries.AddParameter<std::string>(SYMBOL_ONE);
	createSeries.AddParameter<std::string>(m_TimeFrame);
	createSeries.AddParameter<unsigned int>(300);

	m_Outbox->AddMessage(createSeries);
	
	m_Outbox->SendMessages();

	m_Inbox->WaitForMessage(MessageType::DATA_POINT_ARRAY);
}

void TradingView::Session::RequestMoreData(unsigned int type, unsigned int amount)
{
	std::cout << "REQUESTING MORE DATA:" << std::endl;

	TradingView::Message requestData;
	requestData.SetID(type);
	requestData.SetMethod(CHART_MORE_DATA_METHOD);
	requestData.AddParameter<std::string>(m_ChartSessionID);
	requestData.AddParameter<std::string>(SERIES_ONE);
	requestData.AddParameter<unsigned int>(amount);

	m_Outbox->AddMessage(requestData);

	m_Outbox->SendMessages();

	m_Inbox->WaitForMessage(MessageType::DATA_POINT_ARRAY);

	ShowOverview();
}

void TradingView::Session::ExtendDataWithReplay(void)
{
	std::cout << "EXTENDING DATA WITH REPLAY:" << std::endl;

	TradingView::Message replaySession;
	replaySession.SetID(CHART_REPLAY_SESSION_ID);
	replaySession.SetMethod(CHART_REPLAY_SESSION_METHOD);
	replaySession.AddParameter<std::string>(m_ChartReplaySessionID);

	m_Outbox->AddMessage(replaySession);

	m_Outbox->SendMessages();

	m_Inbox->WaitForMessage(MessageType::REPLAY_INSTANCE_ID);

	TradingView::Message replayAddSeries;
	replayAddSeries.SetID(m_ReplayAddSeriesID);
	replayAddSeries.SetMethod(CHART_REPLAY_ADD_SERIES_METHOD);
	replayAddSeries.AddParameter<std::string>(m_ChartReplaySessionID);
	replayAddSeries.AddParameter<std::string>(m_ChartReplaySeriesID);

	nlohmann::json symbolParamJs;
	symbolParamJs[SYMBOL] = STOCK_TICKER_SPY;
	symbolParamJs[ADJUSTMENT] = SPLITS;
	symbolParamJs[SESSION] = EXTENDED;
	std::string param = symbolParamJs.dump();
	param.insert(0, "=");

	replayAddSeries.AddParameter<std::string>(param);
	replayAddSeries.AddParameter<std::string>(m_TimeFrame);

	m_Outbox->AddMessage(replayAddSeries);

	TradingView::Message resolveSymbol;
	resolveSymbol.SetID(CHART_REPLAY_RESOLVE_SYMBOL_ID);
	resolveSymbol.SetMethod(CHART_REPLAY_RESOLVE_SYMBOL_METHOD);
	resolveSymbol.AddParameter<std::string>(m_ChartSessionID);
	resolveSymbol.AddParameter<std::string>(SYMBOL_TWO);

	nlohmann::json paramJs;
	paramJs[REPLAY] = m_ChartReplaySessionID;
	paramJs[SYMBOL] = symbolParamJs;
	param = paramJs.dump();
	param.insert(0, "=");

	resolveSymbol.AddParameter<std::string>(param);

	m_Outbox->AddMessage(resolveSymbol);

	TradingView::Message modifySeries;
	modifySeries.SetID(m_ModifySeriesID);
	modifySeries.SetMethod(CHART_MODIFY_SERIES_METHOD);
	modifySeries.AddParameter<std::string>(m_ChartSessionID);
	modifySeries.AddParameter<std::string>(SERIES_ONE);
	modifySeries.AddParameter<std::string>(SERIES_TWO);
	modifySeries.AddParameter<std::string>(SYMBOL_TWO);
	modifySeries.AddParameter<std::string>(m_TimeFrame);

	m_Outbox->AddMessage(modifySeries);

	TradingView::Message replayReset;
	replayReset.SetID(CHART_REPLAY_RESET_ID);
	replayReset.SetMethod(CHART_REPLAY_RESET_METHOD);
	replayReset.AddParameter<std::string>(m_ChartReplaySessionID);
	replayReset.AddParameter<std::string>(m_ChartReplaySeriesID);

	std::vector<DataPoint> dps = m_Inbox->m_MessageProcessor->m_DPProcessor->m_DataPoints;
	std::sort(dps.begin(), dps.end(), [](DataPoint& x, DataPoint& y) {
		return x.GetTime() < y.GetTime();
	});
	std::time_t lastTime = dps.front().GetTimeStamp() - 1;
	replayReset.AddParameter<std::time_t>(lastTime);

	m_Outbox->AddMessage(replayReset);

	m_Outbox->SendMessages();

	m_Inbox->WaitForMessage(MessageType::DATA_POINT_ARRAY);

	RequestMoreData(CHART_MORE_DATA_EXTRA_LARGE_ID, 298001);

	ShowOverview();
}

void TradingView::Session::ShowOverview(void)
{
	std::vector<DataPoint> dps = m_Inbox->m_MessageProcessor->m_DPProcessor->m_DataPoints;
	std::sort(dps.begin(), dps.end(), [](DataPoint& x, DataPoint& y) {
		return x.GetTime() < y.GetTime();
	});
	if (dps.size() > 0) {
		std::cout << std::endl;
		std::cout << "DataPoints: " << dps.size() << std::endl;
		std::cout << "FirstDate: " << dps.front().GetTime() << std::endl;
		std::cout << "LastDate: " << dps.back().GetTime() << std::endl;

		//std::cout << std::endl;
		//for (auto& dp : dps) {
		//	std::cout << dp.GetTime() << std::endl;
		//}
	}
}
