#include "Session.h"

#define CHART_MORE_DATA_EXTRA_LARGE_AMOUNT (300000)

TradingView::Session::Session(std::string stockMarket, std::string stockTicker, std::string timeInterval, std::string timeFrame, unsigned int ID, bool replay, bool recordMessages) :
	m_Log(),
	m_TVIO(m_Log, ID),
	m_DataPointManager(m_Log, stockMarket, stockTicker, timeInterval, timeFrame),
	m_ComIDManager(stockTicker, timeInterval, timeFrame),
	m_Outbox(m_TVIO, recordMessages),
	m_Inbox(m_TVIO, m_DataPointManager, recordMessages),
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

	 m_Log.Add(LOG_SESSION_SUMMARY, "Session ID", std::to_string(m_ID));
	 m_Log.Add(LOG_SESSION_SUMMARY, "Session Start Time", boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time()));
	 m_Log.Add(LOG_SESSION_SUMMARY, "Stock Market", m_StockMarket);
	 m_Log.Add(LOG_SESSION_SUMMARY, "Stock Ticker", m_StockTicker);
	 m_Log.Add(LOG_SESSION_SUMMARY, "Time Interval", m_TimeInterval);
	 m_Log.Add(LOG_SESSION_SUMMARY, "Time Frame", m_TimeFrame);
	 m_Log.Add(LOG_SESSION_SUMMARY, "Has Replay", std::to_string(m_HasReplay));

	if (!m_ComIDManager.ProcessStockIDData()) {
		 m_Log.Add(LOG_SESSION_ERROR, "Failed To Find Stock Ticker Resolve Symbol ID! Aborting...", "");
		Abort(); return;
	}

	if (!m_TVIO.Connect()) {
		m_Log.Add(LOG_SESSION_ERROR, "Failed To Connect To TradingView! Aborting...", "");
		Abort(); return;
	}

	if (!RequestInitialData()) {
		m_Log.Add(LOG_SESSION_ERROR, "Failed To Get Initial Data! Aborting...", "");
		Abort(); return;
	}

	if (m_HasReplay) {
		if (!ExtendDataWithReplay()) {
			m_Log.Add(LOG_SESSION_ERROR, "Failed To Extend With Replay! Aborting...", "");
			Abort(); return;
		}
	}

	 m_Log.Add(LOG_SESSION_SUMMARY, "Session End Time", boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time()));
	 m_ExecutionTime = static_cast<unsigned int>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - startTime).count());
	 m_Log.Add(LOG_SESSION_SUMMARY, "Session Execution Time", std::to_string(m_ExecutionTime));

	LogResults();

	m_TVIO.Disconnect();
}

bool TradingView::Session::RequestInitialData(void)
{
	if (!m_Inbox.WaitForMessage(MessageType::UNDEFINED, true)) {
		 m_Log.Add(LOG_SESSION_ERROR, "Error Reading Message", "Get Initial Data");
		return false;
	}
	
	std::shared_ptr<TradingView::Message> setAuth(std::make_shared<TradingView::Message>());
	setAuth->SetID(AUTH_ID);
	setAuth->SetMethod(AUTH_METHOD);
	setAuth->AddParameter<std::string>(AUTH_KEY);

	m_Outbox.AddMessage(setAuth);

	std::shared_ptr<TradingView::Message> createSession(std::make_shared<TradingView::Message>());
	createSession->SetID(m_ComIDManager.GetCreateChartSessionID());
	createSession->SetMethod(CHART_SESSION_METHOD);
	createSession->AddParameter<std::string>(m_ComIDManager.GetChartSessionCode());
	createSession->AddParameter<std::string>("");

	m_Outbox.AddMessage(createSession);

	std::shared_ptr<TradingView::Message> switchTimeZone(std::make_shared<TradingView::Message>());
	switchTimeZone->SetID(m_ComIDManager.GetSwitchTimeZoneID());
	switchTimeZone->SetMethod(CHART_SWITCH_TIMEZONE_METHOD);
	switchTimeZone->AddParameter<std::string>(m_ComIDManager.GetChartSessionCode());
	switchTimeZone->AddParameter<std::string>(NEW_YORK_TIMEZONE);

	m_Outbox.AddMessage(switchTimeZone);

	std::shared_ptr<TradingView::Message> resolveSymbol(std::make_shared<TradingView::Message>());
	resolveSymbol->SetID(m_ComIDManager.GetResolveChartSymbolID());
	resolveSymbol->SetMethod(CHART_RESOLVE_SYMBOL_METHOD);
	resolveSymbol->AddParameter<std::string>(m_ComIDManager.GetChartSessionCode());
	resolveSymbol->AddParameter<std::string>(SYMBOL_ONE);

	rapidjson::Value symbolObj(rapidjson::kObjectType);
	symbolObj.AddMember(SYMBOL, m_FormatedTicker, resolveSymbol->GetAllocator());
	symbolObj.AddMember(ADJUSTMENT, SPLITS, resolveSymbol->GetAllocator());
	symbolObj.AddMember(SESSION, EXTENDED, resolveSymbol->GetAllocator());

	resolveSymbol->AddParameter<rapidjson::Value&>(symbolObj);

	m_Outbox.AddMessage(resolveSymbol);

	std::shared_ptr<TradingView::Message> createSeries(std::make_shared<TradingView::Message>());
	createSeries->SetID(m_ComIDManager.GetCreateChartSeriesID());
	createSeries->SetMethod(CHART_SERIES_METHOD);
	createSeries->AddParameter<std::string>(m_ComIDManager.GetChartSessionCode());
	createSeries->AddParameter<std::string>(SERIES_ONE);
	createSeries->AddParameter<std::string>(SERIES_ONE);
	createSeries->AddParameter<std::string>(SYMBOL_ONE);
	createSeries->AddParameter<std::string>(m_FormatedTimeInterval);
	createSeries->AddParameter<unsigned int>(300);

	m_Outbox.AddMessage(createSeries);

	if (!m_Outbox.SendMessages()) {
		m_Log.Add(LOG_SESSION_ERROR, "Error Sending Messges", "Get Initial Data");
		return false;
	}

	if (!m_Inbox.WaitForMessage(MessageType::DATA_POINT_ARRAY, false)) {
		 m_Log.Add(LOG_SESSION_ERROR, "Error Waiting For Initial Data Points Message", "Get Initial Data");
		return false;
	}

	std::shared_ptr<TradingView::Message> requestData(std::make_shared<TradingView::Message>());
	requestData->SetID(CHART_MORE_DATA_EXTRA_LARGE_ID);
	requestData->SetMethod(CHART_MORE_DATA_METHOD);
	requestData->AddParameter<std::string>(m_ComIDManager.GetChartSessionCode());
	requestData->AddParameter<std::string>(SERIES_ONE);
	requestData->AddParameter<unsigned int>(CHART_MORE_DATA_EXTRA_LARGE_AMOUNT);

	m_Outbox.AddMessage(requestData);

	if (!m_Outbox.SendMessages()) {
		m_Log.Add(LOG_SESSION_ERROR, "Error Sending Messges", "Get Initial Data");
		return false;
	}

	if (!m_Inbox.WaitForMessage(MessageType::DATA_POINT_ARRAY, true)) {
		m_Log.Add(LOG_SESSION_ERROR, "Error Waiting For Data Points Message", "Get Initial Data");
		return false;
	}

	return true;
}

bool TradingView::Session::ExtendDataWithReplay(void)
{
	std::shared_ptr<TradingView::Message> replaySession(std::make_shared<TradingView::Message>());
	replaySession->SetID(m_ComIDManager.GetReplayChartCreateSessionID());
	replaySession->SetMethod(CHART_REPLAY_SESSION_METHOD);
	replaySession->AddParameter<std::string>(m_ComIDManager.GetChartReplaySessionCode());

	m_Outbox.AddMessage(replaySession);

	if (!m_Outbox.SendMessages()) {
		 m_Log.Add(LOG_SESSION_ERROR, "Error Sending Messges", "Extend With Replay");
		return false;
	}

	if (!m_Inbox.WaitForMessage(MessageType::REPLAY_INSTANCE_ID, false)) {
		 m_Log.Add(LOG_SESSION_ERROR, "Error Waiting For Replay Instance ID Message", "Extend With Replay");
		return false;
	}

	std::shared_ptr<TradingView::Message> replayAddSeries(std::make_shared<TradingView::Message>());
	replayAddSeries->SetID(m_ComIDManager.GetReplayChartAddSeriesID());
	replayAddSeries->SetMethod(CHART_REPLAY_ADD_SERIES_METHOD);
	replayAddSeries->AddParameter<std::string>(m_ComIDManager.GetChartReplaySessionCode());
	replayAddSeries->AddParameter<std::string>(m_ComIDManager.GetChartReplaySeriesCode());

	rapidjson::Value symbolParam(rapidjson::kObjectType);
	symbolParam.AddMember(SYMBOL, m_FormatedTicker, replayAddSeries->GetAllocator());
	symbolParam.AddMember(ADJUSTMENT, SPLITS, replayAddSeries->GetAllocator());
	symbolParam.AddMember(SESSION, EXTENDED, replayAddSeries->GetAllocator());

	replayAddSeries->AddParameter<rapidjson::Value&>(symbolParam);
	replayAddSeries->AddParameter<std::string>(m_FormatedTimeInterval);

	m_Outbox.AddMessage(replayAddSeries);

	std::shared_ptr<TradingView::Message> resolveSymbol(std::make_shared<TradingView::Message>());
	resolveSymbol->SetID(m_ComIDManager.GetReplayResolveChartSymbolID());
	resolveSymbol->SetMethod(CHART_REPLAY_RESOLVE_SYMBOL_METHOD);
	resolveSymbol->AddParameter<std::string>(m_ComIDManager.GetChartSessionCode());
	resolveSymbol->AddParameter<std::string>(SYMBOL_TWO);

	rapidjson::Value param(rapidjson::kObjectType);
	param.AddMember(REPLAY, m_ComIDManager.GetChartReplaySessionCode(), resolveSymbol->GetAllocator());
	param.AddMember(SYMBOL, symbolParam, resolveSymbol->GetAllocator());

	resolveSymbol->AddParameter<rapidjson::Value&>(param);

	m_Outbox.AddMessage(resolveSymbol);

	std::shared_ptr<TradingView::Message> modifySeries(std::make_shared<TradingView::Message>());
	modifySeries->SetID(m_ComIDManager.GetReplayModifyChartSeriesID());
	modifySeries->SetMethod(CHART_MODIFY_SERIES_METHOD);
	modifySeries->AddParameter<std::string>(m_ComIDManager.GetChartSessionCode());
	modifySeries->AddParameter<std::string>(SERIES_ONE);
	modifySeries->AddParameter<std::string>(SERIES_TWO);
	modifySeries->AddParameter<std::string>(SYMBOL_TWO);
	modifySeries->AddParameter<std::string>(m_FormatedTimeInterval);

	m_Outbox.AddMessage(modifySeries);

	for (unsigned int i = 0; i < 20; i++) {
		std::shared_ptr<TradingView::Message> replayReset(std::make_shared<TradingView::Message>());
		replayReset->SetID(m_ComIDManager.GetReplayChartResetID());
		replayReset->SetMethod(CHART_REPLAY_RESET_METHOD);
		replayReset->AddParameter<std::string>(m_ComIDManager.GetChartReplaySessionCode());
		replayReset->AddParameter<std::string>(m_ComIDManager.GetChartReplaySeriesCode());

		std::time_t lastTime = m_DataPointManager.GetLastEarliestDPTime() - 1;
		lastTime = boost::posix_time::to_time_t(boost::posix_time::from_time_t(lastTime));
		std::cout << "Last Time: " << boost::posix_time::from_time_t(lastTime) << std::endl;

		replayReset->AddParameter<std::time_t>(lastTime);

		m_Outbox.AddMessage(replayReset);

		// it will either stop the connection when it cant do any more resets or just keep going returning no data which the wait for message timeout takes care of
		std::string error;
		if (!m_Outbox.SendMessages()) {
			if (i == 0) {
				m_Log.Add(LOG_SESSION_ERROR, "Error Sending Reset Messge", "Extend With Replay");
				return false;
			} else {
				m_Log.Add(LOG_SESSION_SUMMARY, "Successful Replay Resets", std::to_string(i + 1));
				return true;
			}
		}
		if (!m_Inbox.WaitForMessage(MessageType::DATA_POINT_ARRAY, false)) {
			if (i == 0) {
				m_Log.Add(LOG_SESSION_ERROR, "Error Waiting For Reset Data Points", "Extend With Replay");
				return false;
			} else {
				m_Log.Add(LOG_SESSION_SUMMARY, "Successful Replay Resets", std::to_string(i + 1));
				return true;
			}
		}

		std::shared_ptr<TradingView::Message> requestData(std::make_shared<TradingView::Message>());
		requestData->SetID(CHART_MORE_DATA_EXTRA_LARGE_ID);
		requestData->SetMethod(CHART_MORE_DATA_METHOD);
		requestData->AddParameter<std::string>(m_ComIDManager.GetChartSessionCode());
		requestData->AddParameter<std::string>(SERIES_ONE);
		requestData->AddParameter<unsigned int>(CHART_MORE_DATA_EXTRA_LARGE_AMOUNT);

		m_Outbox.AddMessage(requestData);

		if (!m_Outbox.SendMessages()) {
			if (i == 0) {
				m_Log.Add(LOG_SESSION_ERROR, "Error Sending Reset Request More Data Points", "Extend With Replay");
				return false;
			} else {
				m_Log.Add(LOG_SESSION_SUMMARY, "Successful Replay Resets", std::to_string(i + 1));
				return true;
			}
		}
		if (!m_Inbox.WaitForMessage(MessageType::DATA_POINT_ARRAY, true)) {
			if (i == 0) {
				m_Log.Add(LOG_SESSION_ERROR, "Error Waiting For Request More Data Points", "Extend With Replay");
				return false;
			} else {
				m_Log.Add(LOG_SESSION_SUMMARY, "Successful Replay Resets", std::to_string(i + 1));
				return true;
			}
		}
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
       	 std::cout << "Sorting...." << std::endl;
		 m_DataPointManager.OrderByDate();
		 std::cout << "Finished Sorting...." << std::endl;
		 m_Log.Add(LOG_SESSION_RESULTS, "Data Points", std::to_string(m_DataPointManager.GetDataPoints().size()));
		 m_Log.Add(LOG_SESSION_RESULTS, "Data Point Start Date", boost::posix_time::to_simple_string(m_DataPointManager.GetDataPoints().front().GetPTime()));
		 m_Log.Add(LOG_SESSION_RESULTS, "Data Points End Date", boost::posix_time::to_simple_string(m_DataPointManager.GetDataPoints().back().GetPTime()));
	}
	else {
		 m_Log.Add(LOG_SESSION_ERROR, "No Data Points were collected", "");
	}
}