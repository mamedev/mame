#include "TestRunner.h"
#include "TestResults.h"
#include "TestReporter.h"
#include "TestReporterStdout.h"
#include "TimeHelpers.h"
#include "MemoryOutStream.h"

#include <cstring>


namespace UnitTest {

int RunAllTests()
{
	TestReporterStdout reporter;
	TestRunner runner(reporter);
	return runner.RunTestsIf(Test::GetTestList(), NULL, True(), 0);
}


TestRunner::TestRunner(TestReporter& reporter)
	: m_reporter(&reporter)
	, m_result(new TestResults(&reporter))
	, m_timer(new Timer)
{
	m_timer->Start();
}

TestRunner::~TestRunner()
{
	delete m_result;
	delete m_timer;
}

TestResults* TestRunner::GetTestResults()
{
	return m_result;
}

int TestRunner::Finish() const
{
    float const secondsElapsed = static_cast<float>(m_timer->GetTimeInMs() / 1000.0);
    m_reporter->ReportSummary(m_result->GetTotalTestCount(), 
							  m_result->GetFailedTestCount(), 
							  m_result->GetFailureCount(), 
							  secondsElapsed);
    
	return m_result->GetFailureCount();
}

bool TestRunner::IsTestInSuite(const Test* const curTest, char const* suiteName) const
{
	using namespace std;
	return (suiteName == NULL) || !strcmp(curTest->m_details.suiteName, suiteName);
}

void TestRunner::RunTest(TestResults* const result, Test* const curTest, int const maxTestTimeInMs) const
{
	if (curTest->m_isMockTest == false)
		CurrentTest::Results() = result;

	Timer testTimer;
	testTimer.Start();

	result->OnTestStart(curTest->m_details);

	curTest->Run();

	double const testTimeInMs = testTimer.GetTimeInMs();
	if (maxTestTimeInMs > 0 && testTimeInMs > maxTestTimeInMs && !curTest->m_details.timeConstraintExempt)
	{
	    MemoryOutStream stream;
	    stream << "Global time constraint failed. Expected under " << maxTestTimeInMs <<
	            "ms but took " << testTimeInMs << "ms.";

	    result->OnTestFailure(curTest->m_details, stream.GetText());
	}

	result->OnTestFinish(curTest->m_details, static_cast< float >(testTimeInMs / 1000.0));
}

}
