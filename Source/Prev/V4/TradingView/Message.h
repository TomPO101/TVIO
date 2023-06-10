#pragma once
#include "TVIO.h"
#include "Profiler.h"

#define RAPIDJSON_HAS_STDSTRING 1
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <string>
#include <vector>
#include <chrono>

namespace TradingView {

#define METHOD_KEY ("m")
#define METHOD_SYMBOL ("~m~")
#define PARAM_KEY ("p")
#define SERIES_KEY ("s")
#define VALUE_KEY ("v")

#define AUTH_METHOD ("set_auth_token")
#define CHART_SESSION_METHOD ("chart_create_session")
#define CHART_SERIES_METHOD ("create_series")
#define CHART_MODIFY_SERIES_METHOD ("modify_series")
#define CHART_MORE_DATA_METHOD ("request_more_data")
#define CHART_SWITCH_TIMEZONE_METHOD ("switch_timezone")
#define CHART_RESOLVE_SYMBOL_METHOD ("resolve_symbol")
#define CHART_REPLAY_SESSION_METHOD ("replay_create_session")
#define CHART_REPLAY_ADD_SERIES_METHOD ("replay_add_series")
#define CHART_REPLAY_RESOLVE_SYMBOL_METHOD ("resolve_symbol")
#define CHART_REPLAY_RESET_METHOD ("replay_reset")

#define RESPONSE_METHOD_TIMESCALE ("timescale_update")
#define RESPONSE_METHOD_REPLAY ("replay_instance_id")
#define RESPONSE_METHOD_REPLAY_OK ("replay_ok")
#define RESPONSE_METHOD_REPLAY_ERROR ("replay_error")
#define RESPONSE_METHOD_PROTOCOL_ERROR ("protocol_error")
#define RESPONSE_METHOD_SERIES_LOADING ("series_loading")

#define NEW_YORK_TIMEZONE ("America/New_York")

#define RANGE_BASIC_STUDY ("BarSetRange@tv-basicstudies-72!")
#define PHANTOM_BARS ("phantomBars")

#define SESSION ("session")
#define REPLAY ("replay")
#define SPLITS ("splits")
#define EXTENDED ("extended")
#define TYPE ("type")
#define INPUTS ("inputs")

#define SYMBOL ("symbol")
#define SYMBOL_ONE ("symbol_1")
#define SYMBOL_TWO ("symbol_2")
#define SYMBOL_THREE ("symbol_3")

#define SERIES_ONE ("s1")
#define SERIES_TWO ("s2")
#define SERIES_THREE ("s3")

#define SESSION_ONE ("sessions_1")

#define STUDY_ONE ("st1")

#define EMPTY_OBJ ("{}")

#define ADJUSTMENT ("adjustment")

#define MARKET_NASDAQ ("NASDAQ")
#define MARKET_AMEX ("AMEX")
#define MARKET_NYSE ("NYSE")	

#define TIME_INTERVAL_ONE ("1")
#define TIME_INTERVAL_FIFTEEN ("15")

#define TIME_FRAME_SECOND ("SECOND")

enum MessageType
{
	UNDEFINED,
	TIMESCALE_UPADATE,
	DATA_POINT_ARRAY,
	REPLAY_INSTANCE_ID,
	REPLAY_OK,
	REPLAY_ERROR,
	PROTOCOL_ERROR,
	SERIES_LOADING
};

class Message
{
public:
	Message(void);
	~Message(void);

	void SetID(unsigned int ID);
	void SetMethod(std::string method);
	void SetMessage(std::string& jsonStr);
	void SetType(TradingView::MessageType type) { m_Type = type; };

	void Display(void);
	
	std::string GetMethod(void);
	rapidjson::Document& GetMessage(void) { return m_Message; };
	std::string GetPreparedMessage(void);
	MessageType GetType(void) { return m_Type; };
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& GetAllocator(void) { return m_Message.GetAllocator(); };

	template<typename ParamType>
	void AddParameter(ParamType param);
	
private:
	TradingView::MessageType m_Type;
	unsigned int m_ID;
	rapidjson::Document m_Message;
};

template<typename ParamType>
inline void Message::AddParameter(ParamType value)
{
	#pragma warning(suppress:4984)
	if constexpr (std::is_integral_v<ParamType>)  // check if int
		m_Message[PARAM_KEY].PushBack(rapidjson::Value(value), m_Message.GetAllocator());
	else
		m_Message[PARAM_KEY].PushBack(rapidjson::Value(value, m_Message.GetAllocator()), m_Message.GetAllocator());
}

}