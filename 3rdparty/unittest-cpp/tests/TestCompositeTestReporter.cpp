#include "UnitTest++/UnitTestPP.h"
#include "UnitTest++/CompositeTestReporter.h"

using namespace UnitTest;

namespace {

TEST(ZeroReportersByDefault)
{
	CHECK_EQUAL(0, CompositeTestReporter().GetReporterCount());
}

struct MockReporter : TestReporter
{
	MockReporter()
		: testStartCalled(false)
		, testStartDetails(NULL)
		, failureCalled(false)
		, failureDetails(NULL)
		, failureStr(NULL)
		, testFinishCalled(false)
		, testFinishDetails(NULL)
		, testFinishSecondsElapsed(-1.0f)
		, summaryCalled(false)
		, summaryTotalTestCount(-1)
		, summaryFailureCount(-1)
		, summarySecondsElapsed(-1.0f)
	{
	}

	virtual void ReportTestStart(TestDetails const& test)
	{
		testStartCalled = true;
		testStartDetails = &test;
	}

	virtual void ReportFailure(TestDetails const& test, char const* failure)
	{
		failureCalled = true;
		failureDetails = &test;
		failureStr = failure;
	}

	virtual void ReportTestFinish(TestDetails const& test, float secondsElapsed)
	{
		testFinishCalled = true;
		testFinishDetails = &test;
		testFinishSecondsElapsed = secondsElapsed;
	}

	virtual void ReportSummary(int totalTestCount, 
							   int failedTestCount,
							   int failureCount,
							   float secondsElapsed)
	{
		summaryCalled = true;
		summaryTotalTestCount = totalTestCount;
		summaryFailedTestCount = failedTestCount;
		summaryFailureCount = failureCount;
		summarySecondsElapsed = secondsElapsed;
	}

	bool testStartCalled;
	TestDetails const* testStartDetails;

	bool failureCalled;
	TestDetails const* failureDetails;
	const char* failureStr;

	bool testFinishCalled;
	TestDetails const* testFinishDetails;
	float testFinishSecondsElapsed;

	bool summaryCalled;
	int summaryTotalTestCount;
	int summaryFailedTestCount;
	int summaryFailureCount;
	float summarySecondsElapsed;
};

TEST(AddReporter)
{
	MockReporter r;
	CompositeTestReporter c;

	CHECK(c.AddReporter(&r));
	CHECK_EQUAL(1, c.GetReporterCount());
}

TEST(RemoveReporter)
{
	MockReporter r;
	CompositeTestReporter c;

	c.AddReporter(&r);
	CHECK(c.RemoveReporter(&r));
	CHECK_EQUAL(0, c.GetReporterCount());
}

struct Fixture
{
	Fixture()
	{
		c.AddReporter(&r0);
		c.AddReporter(&r1);
	}

	MockReporter r0, r1;
	CompositeTestReporter c;
};

TEST_FIXTURE(Fixture, ReportTestStartCallsReportTestStartOnAllAggregates)
{
	TestDetails t("", "", "", 0);
	c.ReportTestStart(t);

	CHECK(r0.testStartCalled);
	CHECK_EQUAL(&t, r0.testStartDetails);
	CHECK(r1.testStartCalled);
	CHECK_EQUAL(&t, r1.testStartDetails);
}

TEST_FIXTURE(Fixture, ReportFailureCallsReportFailureOnAllAggregates)
{
	TestDetails t("", "", "", 0);
	const char* failStr = "fail";
	c.ReportFailure(t, failStr);

	CHECK(r0.failureCalled);
	CHECK_EQUAL(&t, r0.failureDetails);
	CHECK_EQUAL(failStr, r0.failureStr);

	CHECK(r1.failureCalled);
	CHECK_EQUAL(&t, r1.failureDetails);
	CHECK_EQUAL(failStr, r1.failureStr);
}

TEST_FIXTURE(Fixture, ReportTestFinishCallsReportTestFinishOnAllAggregates)
{
	TestDetails t("", "", "", 0);
	const float s = 1.2345f;
	c.ReportTestFinish(t, s);

	CHECK(r0.testFinishCalled);
	CHECK_EQUAL(&t, r0.testFinishDetails);
	CHECK_CLOSE(s, r0.testFinishSecondsElapsed, 0.00001f);

	CHECK(r1.testFinishCalled);
	CHECK_EQUAL(&t, r1.testFinishDetails);
	CHECK_CLOSE(s, r1.testFinishSecondsElapsed, 0.00001f);
}

TEST_FIXTURE(Fixture, ReportSummaryCallsReportSummaryOnAllAggregates)
{
	TestDetails t("", "", "", 0);
	const int testCount = 3;
	const int failedTestCount = 4;
	const int failureCount = 5;
	const float secondsElapsed = 3.14159f;

	c.ReportSummary(testCount, failedTestCount, failureCount, secondsElapsed);

	CHECK(r0.summaryCalled);
	CHECK_EQUAL(testCount, r0.summaryTotalTestCount);
	CHECK_EQUAL(failedTestCount, r0.summaryFailedTestCount);
	CHECK_EQUAL(failureCount, r0.summaryFailureCount);
	CHECK_CLOSE(secondsElapsed, r0.summarySecondsElapsed, 0.00001f);

	CHECK(r1.summaryCalled);
	CHECK_EQUAL(testCount, r1.summaryTotalTestCount);
	CHECK_EQUAL(failedTestCount, r1.summaryFailedTestCount);
	CHECK_EQUAL(failureCount, r1.summaryFailureCount);
	CHECK_CLOSE(secondsElapsed, r1.summarySecondsElapsed, 0.00001f);
}

}
