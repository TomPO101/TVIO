#include "HashGenerator.h"
#include <algorithm>
#include <time.h>

bool TradingView::HashGenerator::m_SRandSet;

const std::string TradingView::HashGenerator::GenerateChartSessionHash(void)
{
	return CHART_SESSION_PREFIX + GenerateHash(CHART_SESSION_HASH_LENGTH);
}

const std::string TradingView::HashGenerator::GenerateChartReplaySessionHash(void)
{
    return CHART_REPLAY_SESSION_PREFIX + GenerateHash(CHART_REPLAY_SESSION_HASH_LENGTH);
}

const std::string TradingView::HashGenerator::GenerateChartReplaySeriesHash(void)
{
    return GenerateHash(CHART_REPLAY_SERIES_HASH_LENGTH);
}

const std::string TradingView::HashGenerator::GenerateHash(const unsigned int length)
{
    if (m_SRandSet == NULL)
        m_SRandSet = false;
    if (!m_SRandSet) {
        srand((unsigned)time(NULL));
        m_SRandSet = true;
    }

    auto randchar = []() -> char {
        const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        const size_t maxIndex = (sizeof(charset) - 1);
        return charset[rand() % maxIndex];
    };
    std::string str(length, 0);
    std::generate_n(str.begin(), length, randchar);
    return str;
}

