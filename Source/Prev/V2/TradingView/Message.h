#pragma once
#include "TVIO.h"
#include <nlohmann/json.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <string>
#include <vector>

namespace TradingView {

// NEED IF DEF AND VAR TO CONRROL IF SECOND OR MINUTE ANDCONTROL WHICH IDS ARE USED


#define METHOD_KEY ("m")
#define PARAM_KEY ("p")
#define SERIES_KEY ("s")
#define VALUE_KEY ("v")

#define AUTH_ID (504)
#define AUTH_METHOD ("set_auth_token")

#define CHART_SESSION_ID (55)
#define CHART_SESSION_METHOD ("chart_create_session")

#define CHART_SWITCH_TIMEZONE_ID (66)
#define CHART_SWITCH_TIMEZONE_METHOD ("switch_timezone")
#define NEW_YORK_TIMEZONE ("America/New_York")

#define CHART_RESOLVE_SYMBOL_ID (137)
#define CHART_RESOLVE_SYMBOL_METHOD ("resolve_symbol")

#define CHART_SERIES_MINUTE_ID (74)
#define CHART_SERIES_SECOND_ID (75)
#define CHART_SERIES_METHOD ("create_series")

#define CHART_MODIFY_SERIES_MINUTE_ID (70)
#define CHART_MODIFY_SERIES_SECOND_ID (71)
#define CHART_MODIFY_SERIES_METHOD ("modify_series")

#define CHART_MORE_DATA_MED_ID (59)
#define CHART_MORE_DATA_LARGE_ID (60)
#define CHART_MORE_DATA_EXTRA_LARGE_ID (61)
#define CHART_MORE_DATA_METHOD ("request_more_data")

#define CHART_REPLAY_SESSION_ID (53)
#define CHART_REPLAY_SESSION_METHOD ("replay_create_session")

#define CHART_REPLAY_ADD_SERIES_MINUTE_ID (149)
#define CHART_REPLAY_ADD_SERIES_SECOND_ID (150)
#define CHART_REPLAY_ADD_SERIES_METHOD ("replay_add_series")

#define CHART_REPLAY_RESOLVE_SYMBOL_ID (181)
#define CHART_REPLAY_RESOLVE_SYMBOL_METHOD ("resolve_symbol")

#define CHART_REPLAY_RESET_ID (71)
#define CHART_REPLAY_RESET_METHOD ("replay_reset")
	
#define RESPONSE_METHOD_TIMESCALE ("timescale_update")
#define RESPONSE_METHOD_REPLAY ("replay_instance_id")
#define RESPONSE_METHOD_REPLAY_OK ("replay_ok")
#define RESPONSE_METHOD_REPLAY_ERROR ("replay_error")
#define RESPONSE_METHOD_SERIES_LOADING ("series_loading")

#define SESSION ("session")
#define REPLAY ("replay")
#define SPLITS ("splits")
#define EXTENDED ("extended")

#define SYMBOL ("symbol")
#define SYMBOL_ONE ("symbol_1")
#define SYMBOL_TWO ("symbol_2")
#define SYMBOL_THREE ("symbol_3")

#define SERIES_ONE ("s1")
#define SERIES_TWO ("s2")
#define SERIES_THREE ("s3")

#define SESSION_ONE ("sessions_1")

#define STUDY_ONE ("st1")

#define ONE_SECOND ("1S")
#define ONE_MINUTE ("1")
#define TWO_MINUTE ("2")
#define THREE_MINUTE ("3")
#define TEN_MINUTE ("10")

#define TIMEGROUP_SECOND ("second")

#define EMPTY_OBJ ("{}")

#define ADJUSTMENT ("adjustment")

#define STOCK_TICKER_SPY ("AMEX:SPY")

enum MessageType
{
	UNDEFINED,
	TIMESCALE_UPADATE,
	DATA_POINT_ARRAY,
	REPLAY_INSTANCE_ID,
	REPLAY_OK,
	REPLAY_ERROR,
	SERIES_LOADING
};

class Message
{
public:
	Message(unsigned int ID, nlohmann::json message, MessageType type);
	Message(void);
	~Message(void);

	void SetID(unsigned int ID);
	void SetMethod(std::string method);
	void SetParameters(nlohmann::json param);
	void SetType(MessageType type) { m_Type = type; };

	void Display(void);

	bool FindKey(nlohmann::json js, std::string key);
	
	std::string GetMethod(void);
	nlohmann::json GetMessage(void) { return m_Message; };
	std::string GetPreparedMessage(void);
	MessageType GetType(void) { return m_Type; };

	// ADD PERMATER OBJECT FUNCTION AS WELL FOR KEY AND VALUES . PASS VECTOR AND ITER TO CONSTRUCT JSON OBJECT EG ={"symbol":"AMEX:SPY","adjustment":"splits","session":"extended"}
	template<typename ParamType>
	void AddParameter(ParamType param);
	//void AddParameterObject(nlohmann::json js);
	
private:
	MessageType m_Type;
	unsigned int m_ID;
	nlohmann::json m_Message;
};

template<typename ParamType>
inline void Message::AddParameter(ParamType param)
{
	m_Message[PARAM_KEY].push_back(param);
}

}