#include "Log.h"
#include <fstream>
#include <boost/date_time/posix_time/posix_time.hpp>

TradingView::Log::Log(void) :
	m_Logs()
{
	Add(LOG_DATE, "Date", boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time()));
}

void TradingView::Log::Add(const std::string& group, const std::string& key, const std::string &value)
{
	if (!m_Logs.empty()) {
		bool found = false;
		for (auto& items : m_Logs)
			if (items.first == group) {
				items.second.push_back(LogData(group, key, value));
				found = true;
				break;
			}
		if (!found) goto AddNew;
	} else goto AddNew;
	
	AddNew:
	std::vector<LogData> logs;
	logs.push_back(LogData(group, key, value));
	m_Logs.insert({ group, logs });
	
}

void TradingView::Log::Display(void)
{
	for (const auto& items : m_Logs) {
		std::cout << "Log Group: " << items.first << std::endl;
		for (const auto& log : items.second)
			std::cout << log.key << ": " << log.value << std::endl;
		std::cout << std::endl << std::endl;
	}
}

void TradingView::Log::Write(const std::string& path, const std::string& fileName)
{
	std::ofstream file;
	file.open(path + fileName + ".txt");
	for (const auto& items : m_Logs) {
		file << "Log Group: " << items.first << std::endl;
		for (const auto& log : items.second)
			file << log.key << ": " << log.value << std::endl;
		file << std::endl << std::endl;
	}
	file.close();
}
