#include "Log.h"
#include <fstream>

TradingView::Log::Log(void) :
	m_Logs()
{
}

TradingView::Log::~Log(void)
{
}

void TradingView::Log::Add(std::string group, std::string key, std::string value)
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
	for (auto& items : m_Logs) {
		std::cout << "Log Group: " << items.first << std::endl;
		for (auto& log : items.second) 
			std::cout << log.key << ": " << log.value << std::endl;
		std::cout << std::endl << std::endl;
	}
}

void TradingView::Log::Write(std::string path, std::string fileName)
{
	std::ofstream file;
	file.open(path + fileName + ".txt");
	for (auto& items : m_Logs) {
		file << "Log Group: " << items.first << std::endl;
		for (auto& log : items.second)
			file << log.key << ": " << log.value << std::endl;
		file << std::endl << std::endl;
	}
	file.close();
}
