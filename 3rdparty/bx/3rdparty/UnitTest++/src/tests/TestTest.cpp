#include "../UnitTest++.h"
#include "../TestReporter.h"
#include "../TimeHelpers.h"
#include "ScopedCurrentTest.h"

using namespace UnitTest;

namespace {

TEST(PassingTestHasNoFailures)
{
    class PassingTest : public Test
    {
    public:
        PassingTest() : Test("passing") {}
        virtual void RunImpl() const
        {
            CHECK(true);
        }
    };

    TestResults results;
	{
		ScopedCurrentTest scopedResults(results);
		PassingTest().Run();
	}

    CHECK_EQUAL(0, results.GetFailureCount());
}


TEST(FailingTestHasFailures)
{
    class FailingTest : public Test
    {
    public:
        FailingTest() : Test("failing") {}
        virtual void RunImpl() const
        {
            CHECK(false);
        }
    };

    TestResults results;
	{
		ScopedCurrentTest scopedResults(results);
		FailingTest().Run();
	}

    CHECK_EQUAL(1, results.GetFailureCount());
}


TEST(ThrowingTestsAreReportedAsFailures)
{
    class CrashingTest : public Test
    {
    public:
        CrashingTest() : Test("throwing") {}
        virtual void RunImpl() const
        {
            throw "Blah";
        }
    };
 
    TestResults results;
	{
		ScopedCurrentTest scopedResult(results);
		CrashingTest().Run();
	}

	CHECK_EQUAL(1, results.GetFailureCount());
}


#ifndef UNITTEST_MINGW
TEST(CrashingTestsAreReportedAsFailures)
{
    class CrashingTest : public Test
    {
    public:
        CrashingTest() : Test("crashing") {}
        virtual void RunImpl() const
        {
            reinterpret_cast< void (*)() >(0)();
        }
    };

    TestResults results;
	{
		ScopedCurrentTest scopedResult(results);
		CrashingTest().Run();
	}

	CHECK_EQUAL(1, results.GetFailureCount());
}
#endif

TEST(TestWithUnspecifiedSuiteGetsDefaultSuite)
{
    Test test("test");
    CHECK(test.m_details.suiteName != NULL);
    CHECK_EQUAL("DefaultSuite", test.m_details.suiteName);
}

TEST(TestReflectsSpecifiedSuiteName)
{
    Test test("test", "testSuite");
    CHECK(test.m_details.suiteName != NULL);
    CHECK_EQUAL("testSuite", test.m_details.suiteName);
}

void Fail()
{
	CHECK(false);
}

TEST(OutOfCoreCHECKMacrosCanFailTests)
{
	TestResults results;
	{
		ScopedCurrentTest scopedResult(results);
		Fail();
	}

	CHECK_EQUAL(1, results.GetFailureCount());
}

}
