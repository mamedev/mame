#include "../UnitTest++.h"
#include "../TestResults.h"
#include "RecordingReporter.h"

using namespace UnitTest;

namespace {

TestDetails const details("testname", "suitename", "filename", 123);


TEST(StartsWithNoTestsRun)
{
    TestResults results;
    CHECK_EQUAL (0, results.GetTotalTestCount());
}

TEST(RecordsNumbersOfTests)
{
    TestResults results;
    results.OnTestStart(details);
    results.OnTestStart(details);
    results.OnTestStart(details);
    CHECK_EQUAL(3, results.GetTotalTestCount());
}

TEST(StartsWithNoTestsFailing)
{
    TestResults results;
    CHECK_EQUAL (0, results.GetFailureCount());
}

TEST(RecordsNumberOfFailures)
{
    TestResults results;
    results.OnTestFailure(details, "");
    results.OnTestFailure(details, "");
    CHECK_EQUAL(2, results.GetFailureCount());
}

TEST(RecordsNumberOfFailedTests)
{
    TestResults results;

    results.OnTestStart(details);
    results.OnTestFailure(details, "");
    results.OnTestFinish(details, 0);

    results.OnTestStart(details);
    results.OnTestFailure(details, "");
    results.OnTestFailure(details, "");
    results.OnTestFailure(details, "");
    results.OnTestFinish(details, 0);

    CHECK_EQUAL (2, results.GetFailedTestCount());
}

TEST(NotifiesReporterOfTestStartWithCorrectInfo)
{
    RecordingReporter reporter;
    TestResults results(&reporter);
    results.OnTestStart(details);

    CHECK_EQUAL (1, reporter.testRunCount);
    CHECK_EQUAL ("suitename", reporter.lastStartedSuite);
    CHECK_EQUAL ("testname", reporter.lastStartedTest);
}

TEST(NotifiesReporterOfTestFailureWithCorrectInfo)
{
    RecordingReporter reporter;
    TestResults results(&reporter);

    results.OnTestFailure(details, "failurestring");
    CHECK_EQUAL (1, reporter.testFailedCount);
    CHECK_EQUAL ("filename", reporter.lastFailedFile);
    CHECK_EQUAL (123, reporter.lastFailedLine);
    CHECK_EQUAL ("suitename", reporter.lastFailedSuite);
    CHECK_EQUAL ("testname", reporter.lastFailedTest);
    CHECK_EQUAL ("failurestring", reporter.lastFailedMessage);
}

TEST(NotifiesReporterOfCheckFailureWithCorrectInfo)
{
    RecordingReporter reporter;
    TestResults results(&reporter);

    results.OnTestFailure(details, "failurestring");
    CHECK_EQUAL (1, reporter.testFailedCount);

    CHECK_EQUAL ("filename", reporter.lastFailedFile);
    CHECK_EQUAL (123, reporter.lastFailedLine);
    CHECK_EQUAL ("testname", reporter.lastFailedTest);
    CHECK_EQUAL ("suitename", reporter.lastFailedSuite);
    CHECK_EQUAL ("failurestring", reporter.lastFailedMessage);
}

TEST(NotifiesReporterOfTestEnd)
{
    RecordingReporter reporter;
    TestResults results(&reporter);

    results.OnTestFinish(details, 0.1234f);
    CHECK_EQUAL (1, reporter.testFinishedCount);
    CHECK_EQUAL ("testname", reporter.lastFinishedTest);
    CHECK_EQUAL ("suitename", reporter.lastFinishedSuite);
    CHECK_CLOSE (0.1234f, reporter.lastFinishedTestTime, 0.0001f);
}


}
