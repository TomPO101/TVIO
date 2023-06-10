#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <map>

namespace TradingView {

#define LOG_DATE ("Log Date")

#define LOG_SESSION_SUMMARY ("Log Session Summary")
#define LOG_SESSION_RESULTS ("Log Session Results")
#define LOG_TRUNCATED_RESULTS ("Log Truncated Results")

#define LOG_SESSION_ERROR ("Log Session Error")
#define LOG_TVIO_ERROR ("Log TVIO Error")

#define LOG_SESSION_MANAGER_SUMMARY ("Log Session Manager Summary")
#define LOG_SESSION_MANAGER_ERROR ("Log Session Manager Error")

struct LogData
{
	std::string group, key, value;
	LogData(const std::string& g, const std::string& k, const std::string& v) : group(g), key(k), value(v) {}
};

class Log
{
public:
	Log(void);

	void Add(const std::string& group, const std::string& key, const std::string& value);
	void Display(void);
	void Write(const std::string& path, const std::string& fileName);

private:
	std::map<std::string, std::vector<LogData>> m_Logs;
};

}