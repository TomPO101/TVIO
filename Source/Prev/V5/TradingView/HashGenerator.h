#pragma once
#include <string>

namespace TradingView {

#define CHART_SESSION_PREFIX ("cs_")
#define CHART_REPLAY_SESSION_PREFIX ("rs_")
#define CHART_SESSION_HASH_LENGTH (12)
#define CHART_REPLAY_SESSION_HASH_LENGTH (12)
#define CHART_REPLAY_SERIES_HASH_LENGTH (13)

class HashGenerator
{
public:
	static const std::string GenerateChartSessionHash(void);
	static const std::string GenerateChartReplaySessionHash(void);
	static const std::string GenerateChartReplaySeriesHash(void);

private:
	static const std::string GenerateHash(const unsigned int length);
private:
	static bool m_SRandSet;
};

}
