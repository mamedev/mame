#include "../UnitTest++.h"
#include "../TestMacros.h"
#include "../TestList.h"
#include "../TestResults.h"
#include "../TestReporter.h"
#include "../ReportAssert.h"
#include "RecordingReporter.h"
#include "ScopedCurrentTest.h"

using namespace UnitTest;

namespace {

TestList list1;
TEST_EX(DummyTest, list1)
{
}

TEST (TestsAreAddedToTheListThroughMacro)
{
    CHECK(list1.GetHead() != 0);
    CHECK(list1.GetHead()->next == 0);
}

struct ThrowingThingie
{
    ThrowingThingie() : dummy(false)
    {
        if (!dummy)
            throw "Oops";
    }

    bool dummy;
};

TestList list2;
TEST_FIXTURE_EX(ThrowingThingie, DummyTestName, list2)
{
}

TEST (ExceptionsInFixtureAreReportedAsHappeningInTheFixture)
{
    RecordingReporter reporter;
    TestResults result(&reporter);
	{
		ScopedCurrentTest scopedResults(result);
		list2.GetHead()->Run();
	}

    CHECK(strstr(reporter.lastFailedMessage, "xception"));
    CHECK(strstr(reporter.lastFailedMessage, "fixture"));
    CHECK(strstr(reporter.lastFailedMessage, "ThrowingThingie"));
}

struct DummyFixture
{
    int x;
};

// We're really testing the macros so we just want them to compile and link
SUITE(TestSuite1)
{
	TEST(SimilarlyNamedTestsInDifferentSuitesWork)
	{
	}

	TEST_FIXTURE(DummyFixture, SimilarlyNamedFixtureTestsInDifferentSuitesWork)
	{
	}
}

SUITE(TestSuite2)
{
	TEST(SimilarlyNamedTestsInDifferentSuitesWork)
	{
	}

	TEST_FIXTURE(DummyFixture,SimilarlyNamedFixtureTestsInDifferentSuitesWork)
	{
	}
}

TestList macroTestList1;
TEST_EX(MacroTestHelper1, macroTestList1)
{
}

TEST(TestAddedWithTEST_EXMacroGetsDefaultSuite)
{
    CHECK(macroTestList1.GetHead() != NULL);
    CHECK_EQUAL ("MacroTestHelper1", macroTestList1.GetHead()->m_details.testName);
    CHECK_EQUAL ("DefaultSuite", macroTestList1.GetHead()->m_details.suiteName);
}

TestList macroTestList2;
TEST_FIXTURE_EX(DummyFixture, MacroTestHelper2, macroTestList2)
{
}

TEST(TestAddedWithTEST_FIXTURE_EXMacroGetsDefaultSuite)
{
    CHECK(macroTestList2.GetHead() != NULL);
    CHECK_EQUAL ("MacroTestHelper2", macroTestList2.GetHead()->m_details.testName);
    CHECK_EQUAL ("DefaultSuite", macroTestList2.GetHead()->m_details.suiteName);
}

struct FixtureCtorThrows
{
	FixtureCtorThrows()	{ throw "exception"; }
};

TestList throwingFixtureTestList1;
TEST_FIXTURE_EX(FixtureCtorThrows, FixtureCtorThrowsTestName, throwingFixtureTestList1)
{
}

TEST(FixturesWithThrowingCtorsAreFailures)
{
	CHECK(throwingFixtureTestList1.GetHead() != NULL);
	RecordingReporter reporter;
	TestResults result(&reporter);
	{
		ScopedCurrentTest scopedResult(result);
		throwingFixtureTestList1.GetHead()->Run();
	}

	int const failureCount = result.GetFailedTestCount();
	CHECK_EQUAL(1, failureCount);
	CHECK(strstr(reporter.lastFailedMessage, "while constructing fixture"));
}

struct FixtureDtorThrows
{
	~FixtureDtorThrows() { throw "exception"; }
};

TestList throwingFixtureTestList2;
TEST_FIXTURE_EX(FixtureDtorThrows, FixtureDtorThrowsTestName, throwingFixtureTestList2)
{
}

TEST(FixturesWithThrowingDtorsAreFailures)
{
	CHECK(throwingFixtureTestList2.GetHead() != NULL);

	RecordingReporter reporter;
	TestResults result(&reporter);
	{
		ScopedCurrentTest scopedResult(result);
		throwingFixtureTestList2.GetHead()->Run();
	}

	int const failureCount = result.GetFailedTestCount();
	CHECK_EQUAL(1, failureCount);
	CHECK(strstr(reporter.lastFailedMessage, "while destroying fixture"));
}

const int FailingLine = 123;

struct FixtureCtorAsserts
{
	FixtureCtorAsserts()
	{
		UnitTest::ReportAssert("assert failure", "file", FailingLine);
	}
};

TestList ctorAssertFixtureTestList;
TEST_FIXTURE_EX(FixtureCtorAsserts, CorrectlyReportsAssertFailureInCtor, ctorAssertFixtureTestList)
{
}

TEST(CorrectlyReportsFixturesWithCtorsThatAssert)
{
	RecordingReporter reporter;
	TestResults result(&reporter);
	{
		ScopedCurrentTest scopedResults(result);
		ctorAssertFixtureTestList.GetHead()->Run();
	}

	const int failureCount = result.GetFailedTestCount();
	CHECK_EQUAL(1, failureCount);
	CHECK_EQUAL(FailingLine, reporter.lastFailedLine);
	CHECK(strstr(reporter.lastFailedMessage, "assert failure"));
}

}

// We're really testing if it's possible to use the same suite in two files
// to compile and link successfuly (TestTestSuite.cpp has suite with the same name)
// Note: we are outside of the anonymous namespace
SUITE(SameTestSuite)
{
	TEST(DummyTest1)
	{
	}
}

#define CUR_TEST_NAME CurrentTestDetailsContainCurrentTestInfo
#define INNER_STRINGIFY(X) #X
#define STRINGIFY(X) INNER_STRINGIFY(X)

TEST(CUR_TEST_NAME)
{
	const UnitTest::TestDetails* details = CurrentTest::Details();
	CHECK_EQUAL(STRINGIFY(CUR_TEST_NAME), details->testName);
}

#undef CUR_TEST_NAME
#undef INNER_STRINGIFY
#undef STRINGIFY
