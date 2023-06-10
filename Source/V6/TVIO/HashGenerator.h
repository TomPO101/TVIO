#pragma once
#include <string>

namespace TradingView {
namespace IO {

#define CHART_SESSION_PREFIX ("cs_")
#define CHART_REPLAY_SESSION_PREFIX ("rs_")
#define CHART_SESSION_HASH_LENGTH (12)
#define CHART_REPLAY_SESSION_HASH_LENGTH (12)
#define CHART_REPLAY_SERIES_HASH_LENGTH (13)

class HashGenerator
{
	friend class Session;
	friend class Network;

private:
	static std::string GenerateChartSessionHash(void);
	static std::string GenerateChartReplaySessionHash(void);
	static std::string GenerateChartReplaySeriesHash(void);

private:
	static std::string GenerateHash(const int length);
private:
	static bool m_SRandSet;
};

}
}