#include "TimeHelpers.h"
#include <unistd.h>

namespace UnitTest {

Timer::Timer()
{
    m_startTime.tv_sec = 0;
    m_startTime.tv_usec = 0;
}

void Timer::Start()
{
    gettimeofday(&m_startTime, 0);
}

double Timer::GetTimeInMs() const
{
    struct timeval currentTime;
    gettimeofday(&currentTime, 0);

	double const dsecs = currentTime.tv_sec - m_startTime.tv_sec;
    double const dus = currentTime.tv_usec - m_startTime.tv_usec;

	return (dsecs * 1000.0) + (dus / 1000.0);
}

void TimeHelpers::SleepMs(int ms)
{
    usleep(ms * 1000);
}

}
