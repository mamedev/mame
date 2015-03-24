#include "../UnitTest++.h"
#include "../DeferredTestReporter.h"
#include "../Config.h"
#include <cstring>

namespace UnitTest
{

namespace 
{

#ifdef UNITTEST_USE_CUSTOM_STREAMS
	MemoryOutStream& operator <<(MemoryOutStream& lhs, const std::string& rhs)
	{
		lhs << rhs.c_str();
		return lhs;
	}
#endif

struct MockDeferredTestReporter : public DeferredTestReporter
{
    virtual void ReportSummary(int, int, int, float) 
    {
    }
};

struct DeferredTestReporterFixture
{
    DeferredTestReporterFixture()
        : testName("UniqueTestName")
        , testSuite("UniqueTestSuite")
        , fileName("filename.h")
        , lineNumber(12)
        , details(testName.c_str(), testSuite.c_str(), fileName.c_str(), lineNumber)
    {
    }

    MockDeferredTestReporter reporter;
    std::string const testName;
    std::string const testSuite;
    std::string const fileName;
    int const lineNumber;
    TestDetails const details;
};

TEST_FIXTURE(DeferredTestReporterFixture, ReportTestStartCreatesANewDeferredTest)
{
    reporter.ReportTestStart(details);
    CHECK_EQUAL(1, (int)reporter.GetResults().size());
}

TEST_FIXTURE(DeferredTestReporterFixture, ReportTestStartCapturesTestNameAndSuite)
{
    reporter.ReportTestStart(details);

    DeferredTestResult const& result = reporter.GetResults().at(0);
    CHECK_EQUAL(testName.c_str(), result.testName);
    CHECK_EQUAL(testSuite.c_str(), result.suiteName);
}

TEST_FIXTURE(DeferredTestReporterFixture, ReportTestEndCapturesTestTime)
{
    float const elapsed = 123.45f;
    reporter.ReportTestStart(details);
    reporter.ReportTestFinish(details, elapsed);

    DeferredTestResult const& result = reporter.GetResults().at(0);
    CHECK_CLOSE(elapsed, result.timeElapsed, 0.0001f);
}

TEST_FIXTURE(DeferredTestReporterFixture, ReportFailureSavesFailureDetails)
{
    char const* failure = "failure";

    reporter.ReportTestStart(details);
    reporter.ReportFailure(details, failure);

    DeferredTestResult const& result = reporter.GetResults().at(0);
    CHECK(result.failed == true);
    CHECK_EQUAL(fileName.c_str(), result.failureFile);
}

TEST_FIXTURE(DeferredTestReporterFixture, ReportFailureSavesFailureDetailsForMultipleFailures)
{
    char const* failure1 = "failure 1";
    char const* failure2 = "failure 2";

    reporter.ReportTestStart(details);
    reporter.ReportFailure(details, failure1);
    reporter.ReportFailure(details, failure2);

    DeferredTestResult const& result = reporter.GetResults().at(0);
    CHECK_EQUAL(2, (int)result.failures.size());
    CHECK_EQUAL(failure1, result.failures[0].second);
    CHECK_EQUAL(failure2, result.failures[1].second);
}

TEST_FIXTURE(DeferredTestReporterFixture, DeferredTestReporterTakesCopyOfFailureMessage)
{
    reporter.ReportTestStart(details);

    char failureMessage[128];
    char const* goodStr = "Real failure message";
    char const* badStr = "Bogus failure message";
    
	using namespace std;

    strcpy(failureMessage, goodStr);
    reporter.ReportFailure(details, failureMessage);
    strcpy(failureMessage, badStr);

    DeferredTestResult const& result = reporter.GetResults().at(0);
    DeferredTestResult::Failure const& failure = result.failures.at(0);
    CHECK_EQUAL(goodStr, failure.second);
}

}}
