#include "Session.h"
#include "HashGenerator.h"
#include "TradingCommon/StockInfo.h"

#define CHART_MORE_DATA_EXTRA_LARGE_AMOUNT (300000)

namespace TradingView {
namespace IO {

Session::Session(const std::string& stockTicker, const std::string &timeInterval,
			     const std::string& timeFrame, const int ID, const bool replay, const bool recordMessages) :
	m_StockTicker(stockTicker),
	m_TimeInterval(timeInterval),
	m_TimeFrame(timeFrame),
	m_Log("Session"),
	m_DataPointManager(m_Log, m_StockTicker, m_TimeInterval, m_TimeFrame),
	m_Outbox(recordMessages),
	m_Inbox(m_DataPointManager, recordMessages),
	m_FormatedTicker(StockInfo::GetStockMarket(m_StockTicker) + ":" + m_StockTicker),
	m_FormatedTimeInterval(m_TimeInterval),
	m_ChartSessionHash(HashGenerator::GenerateChartSessionHash()),
	m_ChartReplaySessionHash(HashGenerator::GenerateChartReplaySessionHash()),
	m_ChartReplaySeriesHash(HashGenerator::GenerateChartReplaySeriesHash()),
	m_ID(ID),
	m_HasReplay(replay),
	m_Threaded(false),
	m_Running(false),
	m_Aborted(false),
	m_ExecutionTime(0),
	m_NetworkID(0)
{
	if (m_TimeFrame == TIME_FRAME_SECOND)
		m_FormatedTimeInterval = m_TimeInterval + 'S';
	if (m_TimeFrame == TIME_FRAME_DAY)
		m_FormatedTimeInterval = m_TimeInterval + 'D';
}

void Session::Start(void)
{
	m_Running = true;

	const std::chrono::steady_clock::time_point startTime = std::chrono::high_resolution_clock::now();

	//std::cout << "Processing: " << m_StockTicker << std::endl;

	m_Log.Add(LOG_SESSION_SUMMARY, "Session ID", std::to_string(m_ID));
	m_Log.Add(LOG_SESSION_SUMMARY, "Session Start Time", boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time()));
	m_Log.Add(LOG_SESSION_SUMMARY, "Stock Ticker", m_StockTicker);
	m_Log.Add(LOG_SESSION_SUMMARY, "Time Interval", m_TimeInterval);
	m_Log.Add(LOG_SESSION_SUMMARY, "Time Frame", m_TimeFrame);
	m_Log.Add(LOG_SESSION_SUMMARY, "Has Replay", std::to_string(m_HasReplay));

	if (!Network::Register(m_NetworkID)) {
		m_Log.Add(LOG_SESSION_ERROR, "Failed To Register With TV Network! Aborting...", "");
		Abort(); return;
	}

	/*if (!Network::Init()) {
		m_Log.Add(LOG_SESSION_ERROR, "Failed To Init TV Network! Aborting...", "");
		Abort(); return;
	}*/

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

	if (!Network::UnRegister(m_NetworkID)) {
		m_Log.Add(LOG_SESSION_ERROR, "Failed To UnRegister From TV Network! Aborting...", "");
		Abort(); return;
	}

	m_DataPointManager.OrderByDate();

	m_DataPointManager.RemoveDuplicates();

	m_Log.Add(LOG_SESSION_SUMMARY, "Session End Time", boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time()));
	m_ExecutionTime = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - startTime).count());
	m_Log.Add(LOG_SESSION_SUMMARY, "Session Execution Time", std::to_string(m_ExecutionTime));

	LogResults();
}

bool Session::RequestInitialData(void)
{
	std::string inboxResult = m_Inbox.WaitForMessage(MessageType::UNDEFINED, false, m_NetworkID);
	if (inboxResult != INBOX_OK) {
		 m_Log.Add(LOG_SESSION_ERROR, "Error Reading Message", "Get Initial Data | " + inboxResult);
		return false;
	}
	
	std::shared_ptr<Message> setAuth(std::make_shared<Message>());
	setAuth->SetMethod(AUTH_METHOD);
	setAuth->AddParameter<std::string>(AUTH_KEY);

	m_Outbox.AddMessage(setAuth);

	std::shared_ptr<Message> createSession(std::make_shared<Message>());
	createSession->SetMethod(CHART_SESSION_METHOD);
	createSession->AddParameter<std::string>(m_ChartSessionHash);
	createSession->AddParameter<std::string>("");

	m_Outbox.AddMessage(createSession);

	std::shared_ptr<Message> switchTimeZone(std::make_shared<Message>());
	switchTimeZone->SetMethod(CHART_SWITCH_TIMEZONE_METHOD);
	switchTimeZone->AddParameter<std::string>(m_ChartSessionHash);
	switchTimeZone->AddParameter<std::string>(NEW_YORK_TIMEZONE);

	m_Outbox.AddMessage(switchTimeZone);

	std::shared_ptr<Message> resolveSymbol(std::make_shared<Message>());
	resolveSymbol->SetMethod(CHART_RESOLVE_SYMBOL_METHOD);
	resolveSymbol->AddParameter<std::string>(m_ChartSessionHash);
	resolveSymbol->AddParameter<std::string>(SYMBOL_ONE);

	Value symbolObj(kObjectType);
	symbolObj.AddMember(SYMBOL, m_FormatedTicker, resolveSymbol->GetAllocator());
	symbolObj.AddMember(ADJUSTMENT, SPLITS, resolveSymbol->GetAllocator());
	symbolObj.AddMember(SESSION, EXTENDED, resolveSymbol->GetAllocator());

	resolveSymbol->AddParameter<Value&>(symbolObj);

	m_Outbox.AddMessage(resolveSymbol);

	std::shared_ptr<Message> createSeries(std::make_shared<Message>());
	createSeries->SetMethod(CHART_SERIES_METHOD);
	createSeries->AddParameter<std::string>(m_ChartSessionHash);
	createSeries->AddParameter<std::string>(SERIES_ONE);
	createSeries->AddParameter<std::string>(SERIES_ONE);
	createSeries->AddParameter<std::string>(SYMBOL_ONE);
	createSeries->AddParameter<std::string>(m_FormatedTimeInterval);
	createSeries->AddParameter<int>(300);

	m_Outbox.AddMessage(createSeries);

	if (!m_Outbox.SendMessages(m_NetworkID)) {
		m_Log.Add(LOG_SESSION_ERROR, "Error Sending Messges", "Get Initial Data");
		return false;
	}

	inboxResult = m_Inbox.WaitForMessage(MessageType::DATA_POINT_ARRAY, true, m_NetworkID);
	if (inboxResult != INBOX_OK) {
		m_Log.Add(LOG_SESSION_ERROR, "Error Waiting For Initial Data Points Message", "Get Initial Data | " + inboxResult);
		return false;
	}

	std::shared_ptr<Message> requestData(std::make_shared<Message>());
	requestData->SetMethod(CHART_MORE_DATA_METHOD);
	requestData->AddParameter<std::string>(m_ChartSessionHash);
	requestData->AddParameter<std::string>(SERIES_ONE);
	requestData->AddParameter<int>(CHART_MORE_DATA_EXTRA_LARGE_AMOUNT);

	m_Outbox.AddMessage(requestData);

	if (!m_Outbox.SendMessages(m_NetworkID)) {
		m_Log.Add(LOG_SESSION_ERROR, "Error Sending Messges", "Get Initial Data");
		return false;
	}

	inboxResult = m_Inbox.WaitForMessage(MessageType::DATA_POINT_ARRAY, true, m_NetworkID);
	if (inboxResult != INBOX_OK) {
		m_Log.Add(LOG_SESSION_ERROR, "Error Waiting For Data Points Message", "Get Initial Data | " + inboxResult);
		return false;
	}

	return true;
}

bool Session::ExtendDataWithReplay(void)
{
	std::shared_ptr<Message> replaySession(std::make_shared<Message>());
	replaySession->SetMethod(CHART_REPLAY_SESSION_METHOD);
	replaySession->AddParameter<std::string>(m_ChartReplaySessionHash);

	m_Outbox.AddMessage(replaySession);

	if (!m_Outbox.SendMessages(m_NetworkID)) {
		 m_Log.Add(LOG_SESSION_ERROR, "Error Sending Messages", "Extend With Replay");
		return false;
	}

	std::string inboxResult = m_Inbox.WaitForMessage(MessageType::REPLAY_INSTANCE_ID, false, m_NetworkID);
	if (inboxResult != INBOX_OK) {
		m_Log.Add(LOG_SESSION_ERROR, "Error Waiting For Replay Instance ID Message", "Extend With Replay | " + inboxResult);
		return false;
	}

	std::shared_ptr<Message> replayAddSeries(std::make_shared<Message>());
	replayAddSeries->SetMethod(CHART_REPLAY_ADD_SERIES_METHOD);
	replayAddSeries->AddParameter<std::string>(m_ChartReplaySessionHash);
	replayAddSeries->AddParameter<std::string>(m_ChartReplaySeriesHash);

	Value symbolParam(kObjectType);
	symbolParam.AddMember(SYMBOL, m_FormatedTicker, replayAddSeries->GetAllocator());
	symbolParam.AddMember(ADJUSTMENT, SPLITS, replayAddSeries->GetAllocator());
	symbolParam.AddMember(SESSION, EXTENDED, replayAddSeries->GetAllocator());

	replayAddSeries->AddParameter<Value&>(symbolParam);
	replayAddSeries->AddParameter<std::string>(m_FormatedTimeInterval);

	m_Outbox.AddMessage(replayAddSeries);

	std::shared_ptr<Message> resolveSymbol(std::make_shared<Message>());
	resolveSymbol->SetMethod(CHART_REPLAY_RESOLVE_SYMBOL_METHOD);
	resolveSymbol->AddParameter<std::string>(m_ChartSessionHash);
	resolveSymbol->AddParameter<std::string>(SYMBOL_TWO);

	Value param(kObjectType);
	param.AddMember(REPLAY, m_ChartReplaySessionHash, resolveSymbol->GetAllocator());
	param.AddMember(SYMBOL, symbolParam, resolveSymbol->GetAllocator());

	resolveSymbol->AddParameter<Value&>(param);

	m_Outbox.AddMessage(resolveSymbol);

	std::shared_ptr<Message> modifySeries(std::make_shared<Message>());
	modifySeries->SetMethod(CHART_MODIFY_SERIES_METHOD);
	modifySeries->AddParameter<std::string>(m_ChartSessionHash);
	modifySeries->AddParameter<std::string>(SERIES_ONE);
	modifySeries->AddParameter<std::string>(SERIES_TWO);
	modifySeries->AddParameter<std::string>(SYMBOL_TWO);
	modifySeries->AddParameter<std::string>(m_FormatedTimeInterval);

	m_Outbox.AddMessage(modifySeries);

	int i;
	for (i = 0; i < 20; i++) {
		std::shared_ptr<Message> replayReset(std::make_shared<Message>());
		replayReset->SetMethod(CHART_REPLAY_RESET_METHOD);
		replayReset->AddParameter<std::string>(m_ChartReplaySessionHash);
		replayReset->AddParameter<std::string>(m_ChartReplaySeriesHash);

		std::time_t lastTime = m_DataPointManager.GetLastEarliestDPTime() - 1;

		replayReset->AddParameter<std::time_t>(lastTime);

		m_Outbox.AddMessage(replayReset);

		// it will either stop the connection when it cant do any more resets or just keep going returning no data which the wait for message timeout takes care of
		// also point too deep error can occure on the first reset try

		if (!m_Outbox.SendMessages(m_NetworkID)) {
			m_Log.Add(LOG_SESSION_ERROR, "Error Sending Reset Messge", "Extend With Replay");
			return false;
		}

		inboxResult = m_Inbox.WaitForMessage(MessageType::DATA_POINT_ARRAY, true, m_NetworkID);
		if (inboxResult != INBOX_OK) {
			if (inboxResult == INBOX_TIMEOUT_ERROR || inboxResult == INBOX_POINT_TOO_DEEP_ERROR)
				break;
			m_Log.Add(LOG_SESSION_ERROR, "Error Waiting For Reset Data Points", "Extend With Replay | " + inboxResult);
			m_Log.Add(LOG_SESSION_SUMMARY, "Successful Replay Resets", std::to_string(i));
			return false;
		}

		std::shared_ptr<Message> requestData(std::make_shared<Message>());
		requestData->SetMethod(CHART_MORE_DATA_METHOD);
		requestData->AddParameter<std::string>(m_ChartSessionHash);
		requestData->AddParameter<std::string>(SERIES_ONE);
		requestData->AddParameter<int>(CHART_MORE_DATA_EXTRA_LARGE_AMOUNT);

		m_Outbox.AddMessage(requestData);

		if (!m_Outbox.SendMessages(m_NetworkID)) {
			m_Log.Add(LOG_SESSION_ERROR, "Error Sending Reset Request More Data Points", "Extend With Replay");
			return false;
		}

		inboxResult = m_Inbox.WaitForMessage(MessageType::DATA_POINT_ARRAY, true, m_NetworkID);
		if (inboxResult != INBOX_OK) {
			m_Log.Add(LOG_SESSION_ERROR, "Error Waiting For Request more Data Points", "Extend With Replay | " + inboxResult);
			m_Log.Add(LOG_SESSION_SUMMARY, "Successful Replay Resets", std::to_string(i));
			return false;
		}
	}

	m_Log.Add(LOG_SESSION_SUMMARY, "Successful Replay Resets", std::to_string(i));

	return true;
}

void Session::Abort(void)
{
	m_Aborted = true;

	if (!Network::UnRegister(m_NetworkID)) {
		m_Log.Add(LOG_SESSION_ERROR, "Failed To UnRegister From TV Network In Abort! Aborting...", "");
		Abort(); return;
	}
}

void Session::LogResults(void)
{
	 m_Log.Add(LOG_SESSION_RESULTS, "Inbox Messages", std::to_string(m_Inbox.GetMessages().size()));
	 m_Log.Add(LOG_SESSION_RESULTS, "Outbox Messages", std::to_string(m_Outbox.GetSentMessages().size()));

	if (!m_DataPointManager.GetDataPoints().empty()) {
		 m_Log.Add(LOG_SESSION_RESULTS, "Data Points", std::to_string(m_DataPointManager.GetDataPoints().size()));
		 m_Log.Add(LOG_SESSION_RESULTS, "Data Point Start Date", to_simple_string(m_DataPointManager.GetDataPoints().front()->GetPTime()));
		 m_Log.Add(LOG_SESSION_RESULTS, "Data Points End Date", to_simple_string(m_DataPointManager.GetDataPoints().back()->GetPTime()));
	}
	else 
		 m_Log.Add(LOG_SESSION_ERROR, "No Data Points were collected", "");
}

}
}