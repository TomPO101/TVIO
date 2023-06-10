#include "Session.h"

#define CHART_MORE_DATA_EXTRA_LARGE_AMOUNT (300000)

TradingView::Session::Session(std::string stockMarket, std::string stockTicker, std::string timeInterval, std::string timeFrame, unsigned int ID, bool replay, bool recordMessages) :
	m_Log(),
	m_TVIO(m_Log, ID),
	m_DataPointManager(m_Log, stockMarket, stockTicker, timeInterval, timeFrame),
	m_ComIDManager(stockTicker, timeInterval, timeFrame),
	m_Outbox(m_TVIO),
	m_Inbox(m_TVIO, m_DataPointManager),
	m_StockMarket(stockMarket),
	m_StockTicker(stockTicker),
	m_TimeInterval(timeInterval),
	m_FormatedTimeInterval(),
	m_TimeFrame(timeFrame),
	m_IsThreaded(false),
	m_ID(ID),
	m_HasReplay(replay),
	m_Aborted(false),
	m_ExecutionTime(0)
{
	m_FormatedTicker = m_StockMarket + ":" + m_StockTicker;
	m_FormatedTimeInterval = m_TimeInterval;
	if (m_TimeFrame == TIME_FRAME_SECOND)
		m_FormatedTimeInterval = m_TimeInterval + 'S';
}

TradingView::Session::~Session(void)
{
}

void TradingView::Session::Start(void)
{
	std::chrono::steady_clock::time_point startTime = std::chrono::high_resolution_clock::now();

	 m_Log.Add(LOG_SESSION_HEADER, "Session ID", std::to_string(m_ID));
	 m_Log.Add(LOG_SESSION_HEADER, "Session Start Time", boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time()));
	 m_Log.Add(LOG_SESSION_HEADER, "Stock Market", m_StockMarket);
	 m_Log.Add(LOG_SESSION_HEADER, "Stock Ticker", m_StockTicker);
	 m_Log.Add(LOG_SESSION_HEADER, "Time Interval", m_TimeInterval);
	 m_Log.Add(LOG_SESSION_HEADER, "Time Frame", m_TimeFrame);
	 m_Log.Add(LOG_SESSION_HEADER, "Has Replay", std::to_string(m_HasReplay));

	if (!m_ComIDManager.ProcessStockIDData()) {
		 m_Log.Add(LOG_SESSION_ERROR, "Failed To Find Stock Ticker Resolve Symbol ID! Aborting...", "");
		Abort(); return;
	}

	if (!m_TVIO.Connect()) {
		 m_Log.Add(LOG_SESSION_ERROR, "Failed To Connect To TradingView! Aborting...", "");
		Abort(); return;
	}

	if (!CreateSession()) {
		 m_Log.Add(LOG_SESSION_ERROR, "Failed To Create Session! Aborting...", "");
		Abort(); return;
	}

	if (!RequestMoreData(CHART_MORE_DATA_EXTRA_LARGE_ID, CHART_MORE_DATA_EXTRA_LARGE_AMOUNT)) {
		 m_Log.Add(LOG_SESSION_ERROR, "Failed To Request More Data! Aborting...", "");
		Abort(); return;
	}

	if (m_HasReplay && m_TimeFrame != TIME_FRAME_RANGE) {
		if (!ExtendDataWithReplay()) {
			 m_Log.Add(LOG_SESSION_ERROR, "Failed To Extend With Replay! Aborting...", "");
			Abort(); return;
		}
		if (!RequestMoreData(CHART_MORE_DATA_EXTRA_LARGE_ID, CHART_MORE_DATA_EXTRA_LARGE_AMOUNT)) {
			 m_Log.Add(LOG_SESSION_ERROR, "Failed To Request More Replay Data! Aborting...", "");
			Abort(); return;
		}
	}

	 m_Log.Add(LOG_SESSION_HEADER, "Session End Time", boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time()));
	 m_ExecutionTime = static_cast<unsigned int>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - startTime).count());
	 m_Log.Add(LOG_SESSION_HEADER, "Session Execution Time", std::to_string(m_ExecutionTime));

	LogResults();

	m_TVIO.Disconnect();
}

bool TradingView::Session::CreateSession(void)
{
	if (!m_Inbox.ReadMessage()) {
		 m_Log.Add(LOG_SESSION_ERROR, "Error Reading Message", "Create Session");
		return false;
	}

	TradingView::Message setAuth;
	setAuth.SetID(AUTH_ID);
	setAuth.SetMethod(AUTH_METHOD);
	setAuth.AddParameter<std::string>(AUTH_KEY);

	m_Outbox.AddMessage(setAuth);

	TradingView::Message startSession;
	startSession.SetID(m_ComIDManager.GetCreateChartSessionID());
	startSession.SetMethod(CHART_SESSION_METHOD);
	startSession.AddParameter<std::string>(m_ComIDManager.GetChartSessionCode());
	startSession.AddParameter<std::string>("");

	m_Outbox.AddMessage(startSession);

	TradingView::Message switchTimeZone;
	switchTimeZone.SetID(m_ComIDManager.GetSwitchTimeZoneID());
	switchTimeZone.SetMethod(CHART_SWITCH_TIMEZONE_METHOD);
	switchTimeZone.AddParameter<std::string>(m_ComIDManager.GetChartSessionCode());
	switchTimeZone.AddParameter<std::string>(NEW_YORK_TIMEZONE);

	m_Outbox.AddMessage(switchTimeZone);

	TradingView::Message resolveSymbol;
	resolveSymbol.SetID(m_ComIDManager.GetResolveChartSymbolID());
	resolveSymbol.SetMethod(CHART_RESOLVE_SYMBOL_METHOD);
	resolveSymbol.AddParameter<std::string>(m_ComIDManager.GetChartSessionCode());
	resolveSymbol.AddParameter<std::string>(SYMBOL_ONE);

	nlohmann::json paramJs;
	std::string paramStr;
	paramJs[SYMBOL] = m_FormatedTicker;
	paramJs[ADJUSTMENT] = SPLITS;
	paramJs[SESSION] = EXTENDED;

	if (m_TimeFrame != TIME_FRAME_RANGE) {
		paramStr = paramJs.dump();
	}
	else {
		nlohmann::json objJS;
		nlohmann::json inputObjJs;
		objJS[SYMBOL] = paramJs;
		objJS[TYPE] = RANGE_BASIC_STUDY;
		inputObjJs[RANGE] = 1;
		inputObjJs[PHANTOM_BARS] = false;
		objJS[INPUTS] = inputObjJs;
		paramStr = objJS.dump();
	}

	paramStr.insert(0, "=");
	resolveSymbol.AddParameter<std::string>(paramStr);

	m_Outbox.AddMessage(resolveSymbol);

	TradingView::Message createSeries;
	createSeries.SetID(m_ComIDManager.GetCreateChartSeriesID());
	createSeries.SetMethod(CHART_SERIES_METHOD);
	createSeries.AddParameter<std::string>(m_ComIDManager.GetChartSessionCode());
	createSeries.AddParameter<std::string>(SERIES_ONE);
	createSeries.AddParameter<std::string>(SERIES_ONE);
	createSeries.AddParameter<std::string>(SYMBOL_ONE);
	createSeries.AddParameter<std::string>(m_FormatedTimeInterval);
	createSeries.AddParameter<unsigned int>(300);

	m_Outbox.AddMessage(createSeries);
	
	if (!m_Outbox.SendMessages()) {
		 m_Log.Add(LOG_SESSION_ERROR, "Error Sending Messges", "Create Session");
		return false;	
	}
	// TRY NOT WAITING FOR THE 300 DP MESSAGE HERE AND IN REPLAY

	if (!m_Inbox.WaitForMessage(MessageType::DATA_POINT_ARRAY, false)) {
		 m_Log.Add(LOG_SESSION_ERROR, "Error Waiting For Message", "Create Session");
		return false;
	}

	return true;
}

bool TradingView::Session::RequestMoreData(unsigned int type, unsigned int amount)
{
	TradingView::Message requestData;
	requestData.SetID(type);
	requestData.SetMethod(CHART_MORE_DATA_METHOD);
	requestData.AddParameter<std::string>(m_ComIDManager.GetChartSessionCode());
	requestData.AddParameter<std::string>(SERIES_ONE);
	requestData.AddParameter<unsigned int>(amount);

	m_Outbox.AddMessage(requestData);

	if (!m_Outbox.SendMessages()) {
		 m_Log.Add(LOG_SESSION_ERROR, "Error Sending Messges", "Request More Data");
		return false;
	}
	
	if (!m_Inbox.WaitForMessage(MessageType::DATA_POINT_ARRAY, true)) {
		m_Log.Add(LOG_SESSION_ERROR, "Error Waiting For Message", "Request More Data");
		return false;
	}

	if (m_DataPointManager.GetDataPoints().empty())
		return false;

	return true;
}

bool TradingView::Session::ExtendDataWithReplay(void)
{
	TradingView::Message replaySession;
	replaySession.SetID(m_ComIDManager.GetReplayChartCreateSessionID());
	replaySession.SetMethod(CHART_REPLAY_SESSION_METHOD);
	replaySession.AddParameter<std::string>(m_ComIDManager.GetReplayChartSessionCode());

	m_Outbox.AddMessage(replaySession);

	if (!m_Outbox.SendMessages()) {
		 m_Log.Add(LOG_SESSION_ERROR, "Error Sending Messges", "Extend With Replay Creating Session");
		return false;
	}

	if (!m_Inbox.WaitForMessage(MessageType::REPLAY_INSTANCE_ID, false)) {
		 m_Log.Add(LOG_SESSION_ERROR, "Error Waiting For Message", "Extend With Replay Creating Session");
		return false;
	}

	TradingView::Message replayAddSeries;
	replayAddSeries.SetID(m_ComIDManager.GetReplayChartAddSeriesID());
	replayAddSeries.SetMethod(CHART_REPLAY_ADD_SERIES_METHOD);
	replayAddSeries.AddParameter<std::string>(m_ComIDManager.GetReplayChartSessionCode());
	replayAddSeries.AddParameter<std::string>(m_ComIDManager.GetReplayChartSeriesCode());

	nlohmann::json symbolParamJs;
	symbolParamJs[SYMBOL] = m_FormatedTicker;
	symbolParamJs[ADJUSTMENT] = SPLITS;
	symbolParamJs[SESSION] = EXTENDED;
	std::string param = symbolParamJs.dump();
	param.insert(0, "=");

	replayAddSeries.AddParameter<std::string>(param);
	replayAddSeries.AddParameter<std::string>(m_FormatedTimeInterval);

	m_Outbox.AddMessage(replayAddSeries);

	TradingView::Message resolveSymbol;
	resolveSymbol.SetID(m_ComIDManager.GetReplayResolveChartSymbolID());
	resolveSymbol.SetMethod(CHART_REPLAY_RESOLVE_SYMBOL_METHOD);
	resolveSymbol.AddParameter<std::string>(m_ComIDManager.GetChartSessionCode());
	resolveSymbol.AddParameter<std::string>(SYMBOL_TWO);

	nlohmann::json paramJs;
	paramJs[REPLAY] = m_ComIDManager.GetReplayChartSessionCode();
	paramJs[SYMBOL] = symbolParamJs;
	param = paramJs.dump();
	param.insert(0, "=");

	resolveSymbol.AddParameter<std::string>(param);

	m_Outbox.AddMessage(resolveSymbol);

	TradingView::Message modifySeries;
	modifySeries.SetID(m_ComIDManager.GetReplayModifyChartSeriesID());
	modifySeries.SetMethod(CHART_MODIFY_SERIES_METHOD);
	modifySeries.AddParameter<std::string>(m_ComIDManager.GetChartSessionCode());
	modifySeries.AddParameter<std::string>(SERIES_ONE);
	modifySeries.AddParameter<std::string>(SERIES_TWO);
	modifySeries.AddParameter<std::string>(SYMBOL_TWO);
	modifySeries.AddParameter<std::string>(m_FormatedTimeInterval);

	m_Outbox.AddMessage(modifySeries);

	TradingView::Message replayReset;
	replayReset.SetID(m_ComIDManager.GetReplayChartResetID());
	replayReset.SetMethod(CHART_REPLAY_RESET_METHOD);
	replayReset.AddParameter<std::string>(m_ComIDManager.GetReplayChartSessionCode());
	replayReset.AddParameter<std::string>(m_ComIDManager.GetReplayChartSeriesCode());

	m_DataPointManager.OrderByDate();
	std::time_t lastTime = m_DataPointManager.GetDataPoints().front().GetTimeStamp() - 1;
	lastTime = boost::posix_time::to_time_t((boost::posix_time::from_time_t(lastTime) + boost::posix_time::hours(4)));
	std::cout << "Last Time: " << boost::posix_time::from_time_t(lastTime) << std::endl;
	replayReset.AddParameter<std::time_t>(lastTime);

	m_Outbox.AddMessage(replayReset);

	if (!m_Outbox.SendMessages()) {
		m_Log.Add(LOG_SESSION_ERROR, "Error Sending Messges", "Extend With Replay");
		return false;
	}

	if (!m_Inbox.WaitForMessage(MessageType::DATA_POINT_ARRAY, false)) {
		m_Log.Add(LOG_SESSION_ERROR, "Error Waiting For Message", "Extend With Replay");
		return false;
	}
	return true;
}

void TradingView::Session::Abort(void)
{
	m_Aborted = true;
	m_TVIO.Disconnect();
}

void TradingView::Session::LogResults(void)
{
	 m_Log.Add(LOG_SESSION_RESULTS, "Inbox Messages", std::to_string(m_Inbox.GetMessages().size()));
	 m_Log.Add(LOG_SESSION_RESULTS, "Outbox Messages", std::to_string(m_Outbox.GetSentMessages().size()));

	if (!m_DataPointManager.GetDataPoints().empty()) {
		 m_DataPointManager.OrderByDate();
		 m_Log.Add(LOG_SESSION_RESULTS, "Data Points", std::to_string(m_DataPointManager.GetDataPoints().size()));
		 m_Log.Add(LOG_SESSION_RESULTS, "Data Point Start Date", boost::posix_time::to_simple_string(m_DataPointManager.GetDataPoints().front().GetPTime()));
		 m_Log.Add(LOG_SESSION_RESULTS, "Data Points End Date", boost::posix_time::to_simple_string(m_DataPointManager.GetDataPoints().back().GetPTime()));
	}
	else {
		 m_Log.Add(LOG_SESSION_ERROR, "No Data Points were collected", "");
	}
}