#include "UnitTest++/UnitTestPP.h"
#include "UnitTest++/TestResults.h"
#include "UnitTest++/TimeHelpers.h"
#include "RecordingReporter.h"
#include "ScopedCurrentTest.h"

using namespace UnitTest;

namespace
{

TEST(TimeConstraintSucceedsWithFastTest)
{
    TestResults result;
    {
		ScopedCurrentTest scopedResult(result);
        TimeConstraint t(200, TestDetails("", "", "", 0));
        TimeHelpers::SleepMs(5);
    }
    CHECK_EQUAL(0, result.GetFailureCount());
}

TEST(TimeConstraintFailsWithSlowTest)
{
    TestResults result;
    {
		ScopedCurrentTest scopedResult(result);
        TimeConstraint t(10, TestDetails("", "", "", 0));
        TimeHelpers::SleepMs(20);
    }
    CHECK_EQUAL(1, result.GetFailureCount());
}

TEST(TimeConstraintFailureIncludesCorrectData)
{
    RecordingReporter reporter;
    TestResults result(&reporter);
    {
		ScopedCurrentTest scopedResult(result);

		TestDetails const details("testname", "suitename", "filename", 10);
        TimeConstraint t(10, details);
        TimeHelpers::SleepMs(20);
    }

	using namespace std;

	CHECK(strstr(reporter.lastFailedFile, "filename"));
    CHECK_EQUAL(10, reporter.lastFailedLine);
    CHECK(strstr(reporter.lastFailedTest, "testname"));
}

TEST(TimeConstraintFailureIncludesTimeoutInformation)
{
    RecordingReporter reporter;
    TestResults result(&reporter);
    {
		ScopedCurrentTest scopedResult(result);
        TimeConstraint t(10, TestDetails("", "", "", 0));
        TimeHelpers::SleepMs(20);
    }

	using namespace std;

	CHECK(strstr(reporter.lastFailedMessage, "ime constraint"));
    CHECK(strstr(reporter.lastFailedMessage, "under 10ms"));
}

}
