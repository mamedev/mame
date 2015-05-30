#include "UnitTest++/Config.h"
#include "UnitTest++/UnitTestPP.h"

#include "UnitTest++/ReportAssert.h"
#include "UnitTest++/ReportAssertImpl.h"
#include "UnitTest++/AssertException.h"

#include "RecordingReporter.h"
#include <csetjmp>

using namespace UnitTest;

namespace {

TEST(CanSetAssertExpected)
{
	Detail::ExpectAssert(true);
	CHECK(Detail::AssertExpected());

	Detail::ExpectAssert(false);
	CHECK(!Detail::AssertExpected());
}

#ifndef UNITTEST_NO_EXCEPTIONS

TEST(ReportAssertThrowsAssertException)
{
    bool caught = false;

    try
    {
		TestResults testResults;
		TestDetails testDetails("", "", "", 0);
        Detail::ReportAssertEx(&testResults, &testDetails, "", "", 0);
    }
    catch(AssertException const&)
    {
        caught = true;
    }

    CHECK(true == caught);
}

TEST(ReportAssertClearsExpectAssertFlag)
{
	RecordingReporter reporter;
	TestResults testResults(&reporter);
	TestDetails testDetails("", "", "", 0);

	try
	{
		Detail::ExpectAssert(true);
		Detail::ReportAssertEx(&testResults, &testDetails, "", "", 0);
	}
	catch(AssertException const&)
	{
	}

	CHECK(Detail::AssertExpected() == false);
	CHECK_EQUAL(0, reporter.testFailedCount);
}

TEST(ReportAssertWritesFailureToResultsAndDetailsWhenAssertIsNotExpected)
{
    const int lineNumber = 12345;
    const char* description = "description";
    const char* filename = "filename";

	RecordingReporter reporter;
	TestResults testResults(&reporter);
	TestDetails testDetails("", "", "", 0);

    try
    {
        Detail::ReportAssertEx(&testResults, &testDetails, description, filename, lineNumber);
    }
    catch(AssertException const&)
    {
    }

	CHECK_EQUAL(description, reporter.lastFailedMessage);
	CHECK_EQUAL(filename, reporter.lastFailedFile);
	CHECK_EQUAL(lineNumber, reporter.lastFailedLine);
}

TEST(ReportAssertReportsNoErrorsWhenAssertIsExpected)
{
	Detail::ExpectAssert(true);

	RecordingReporter reporter;
	TestResults testResults(&reporter);
	TestDetails testDetails("", "", "", 0);

	try
	{
		Detail::ReportAssertEx(&testResults, &testDetails, "", "", 0);
	}
	catch(AssertException const&)
	{
	}

	CHECK_EQUAL(0, reporter.testFailedCount);
}

TEST(CheckAssertMacroSetsAssertExpectationToFalseAfterRunning)
{
	Detail::ExpectAssert(true);
	CHECK_ASSERT(ReportAssert("", "", 0));
	CHECK(!Detail::AssertExpected());
	Detail::ExpectAssert(false);
}

#else

TEST(SetAssertJumpTargetReturnsFalseWhenSettingJumpTarget)
{
	CHECK(UNITTEST_SET_ASSERT_JUMP_TARGET() == false);
}

TEST(JumpToAssertJumpTarget_JumpsToSetPoint_ReturnsTrue)
{
	const volatile bool taken = !!UNITTEST_SET_ASSERT_JUMP_TARGET();

	volatile bool set = false;
	if (taken == false)
	{
		UNITTEST_JUMP_TO_ASSERT_JUMP_TARGET();
		set = true;
	}

	CHECK(set == false);
}

#endif

}
