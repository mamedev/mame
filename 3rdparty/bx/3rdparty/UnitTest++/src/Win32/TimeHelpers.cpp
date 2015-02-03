#include "TimeHelpers.h"
#include <windows.h>

namespace UnitTest {

Timer::Timer()
	: m_threadHandle(::GetCurrentThread())
	, m_startTime(0)
{
#if defined(_MSC_VER) && (_MSC_VER == 1200) // VC6 doesn't have DWORD_PTR?
	typedef unsigned long DWORD_PTR;
#endif

	DWORD_PTR systemMask;
	::GetProcessAffinityMask(GetCurrentProcess(), &m_processAffinityMask, &systemMask);
	::SetThreadAffinityMask(m_threadHandle, 1);
	::QueryPerformanceFrequency(reinterpret_cast< LARGE_INTEGER* >(&m_frequency));
	::SetThreadAffinityMask(m_threadHandle, m_processAffinityMask);
}

void Timer::Start()
{
	m_startTime = GetTime();
}

double Timer::GetTimeInMs() const
{
	__int64 const elapsedTime = GetTime() - m_startTime;
	double const seconds = double(elapsedTime) / double(m_frequency);
	return seconds * 1000.0;
}

__int64 Timer::GetTime() const
{
	LARGE_INTEGER curTime;
	::SetThreadAffinityMask(m_threadHandle, 1);
	::QueryPerformanceCounter(&curTime);
	::SetThreadAffinityMask(m_threadHandle, m_processAffinityMask);
	return curTime.QuadPart;
}

void TimeHelpers::SleepMs(int const ms)
{
	::Sleep(ms);
}

}
