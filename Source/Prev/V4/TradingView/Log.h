#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <map>

namespace TradingView {

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
	LogData(std::string g, std::string k, std::string v) : group(g), key(k), value(v) {}
};

class Log
{
public:
	Log(void);
	~Log(void);

	void Add(std::string group, std::string key, std::string value);
	void Display(void);
	void Write(std::string path, std::string fileName);

private:
	std::map<std::string, std::vector<LogData>> m_Logs;
};

}