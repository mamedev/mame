#include "CompositeTestReporter.h"
#include <cstddef>

namespace UnitTest {

CompositeTestReporter::CompositeTestReporter()
	: m_reporterCount(0)
{
}

int CompositeTestReporter::GetReporterCount() const
{
	return m_reporterCount;
}

bool CompositeTestReporter::AddReporter(TestReporter* reporter)
{
	if (m_reporterCount == kMaxReporters)
		return false;

	m_reporters[m_reporterCount++] = reporter;
	return true;
}

bool CompositeTestReporter::RemoveReporter(TestReporter* reporter)
{
    for (int index = 0; index < m_reporterCount; ++index)
    {
        if (m_reporters[index] == reporter)
        {
            m_reporters[index] = m_reporters[m_reporterCount - 1];
			--m_reporterCount;
			return true;
        }
    }

    return false;
}

void CompositeTestReporter::ReportFailure(TestDetails const& details, char const* failure)
{
    for (int index = 0; index < m_reporterCount; ++index)
		m_reporters[index]->ReportFailure(details, failure);
}

void CompositeTestReporter::ReportTestStart(TestDetails const& test)
{
	for (int index = 0; index < m_reporterCount; ++index)
		m_reporters[index]->ReportTestStart(test);
}

void CompositeTestReporter::ReportTestFinish(TestDetails const& test, float secondsElapsed)
{
	for (int index = 0; index < m_reporterCount; ++index)
		m_reporters[index]->ReportTestFinish(test, secondsElapsed);
}

void CompositeTestReporter::ReportSummary(int totalTestCount,
										  int failedTestCount,
										  int failureCount,
										  float secondsElapsed)
{
	for (int index = 0; index < m_reporterCount; ++index)
		m_reporters[index]->ReportSummary(totalTestCount, failedTestCount, failureCount, secondsElapsed);
}

}
