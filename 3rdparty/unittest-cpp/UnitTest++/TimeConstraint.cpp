#include "TimeConstraint.h"
#include "TestResults.h"
#include "MemoryOutStream.h"
#include "CurrentTest.h"

namespace UnitTest {


TimeConstraint::TimeConstraint(int ms, TestDetails const& details)
	: m_details(details)
    , m_maxMs(ms)
{
    m_timer.Start();
}

TimeConstraint::~TimeConstraint()
{
    double const totalTimeInMs = m_timer.GetTimeInMs();
    if (totalTimeInMs > m_maxMs)
    {
        MemoryOutStream stream;
        stream << "Time constraint failed. Expected to run test under " << m_maxMs <<
                  "ms but took " << totalTimeInMs << "ms.";

		CurrentTest::Results()->OnTestFailure(m_details, stream.GetText());
    }
}

}
