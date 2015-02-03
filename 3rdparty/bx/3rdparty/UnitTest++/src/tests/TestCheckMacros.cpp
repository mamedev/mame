#include "../UnitTest++.h"
#include "../CurrentTest.h"
#include "RecordingReporter.h"
#include "ScopedCurrentTest.h"

using namespace std;

namespace {

TEST(CheckSucceedsOnTrue)
{
    bool failure = true;
    {
        RecordingReporter reporter;
        UnitTest::TestResults testResults(&reporter);

		ScopedCurrentTest scopedResults(testResults);
		CHECK(true);

		failure = (testResults.GetFailureCount() > 0);
    }

    CHECK(!failure);
}

TEST(CheckFailsOnFalse)
{
    bool failure = false;
    {
        RecordingReporter reporter;
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);
        CHECK(false);
        failure = (testResults.GetFailureCount() > 0);
    }

    CHECK(failure);
}

TEST(FailureReportsCorrectTestName)
{
    RecordingReporter reporter;
    {
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);
        CHECK(false);
    }

    CHECK_EQUAL(m_details.testName, reporter.lastFailedTest);
}

TEST(CheckFailureIncludesCheckContents)
{
    RecordingReporter reporter;
    {
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);
        const bool yaddayadda = false;
        CHECK(yaddayadda);
    }

    CHECK(strstr(reporter.lastFailedMessage, "yaddayadda"));
}

int ThrowingFunction()
{
    throw "Doh";
}

TEST(CheckFailsOnException)
{
    bool failure = false;
    {
        RecordingReporter reporter;
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);
        CHECK(ThrowingFunction() == 1);
        failure = (testResults.GetFailureCount() > 0);
    }

    CHECK(failure);
}

TEST(CheckFailureBecauseOfExceptionIncludesCheckContents)
{
    RecordingReporter reporter;
    {
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);
        CHECK(ThrowingFunction() == 1);
    }

    CHECK(strstr(reporter.lastFailedMessage, "ThrowingFunction() == 1"));
}

TEST(CheckEqualSucceedsOnEqual)
{
    bool failure = true;
    {
        RecordingReporter reporter;
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);
        CHECK_EQUAL(1, 1);
        failure = (testResults.GetFailureCount() > 0);
    }

    CHECK(!failure);
}

TEST(CheckEqualFailsOnNotEqual)
{
    bool failure = false;
    {
        RecordingReporter reporter;
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);
        CHECK_EQUAL(1, 2);
        failure = (testResults.GetFailureCount() > 0);
    }

    CHECK(failure);
}

TEST(CheckEqualFailsOnException)
{
    bool failure = false;
    {
        RecordingReporter reporter;
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);
        CHECK_EQUAL(ThrowingFunction(), 1);
        failure = (testResults.GetFailureCount() > 0);
    }

    CHECK(failure);
}

TEST(CheckEqualFailureContainsCorrectDetails)
{
    int line = 0;
    RecordingReporter reporter;
    {
        UnitTest::TestResults testResults(&reporter);
		UnitTest::TestDetails const testDetails("testName", "suiteName", "filename", -1);
		ScopedCurrentTest scopedResults(testResults, &testDetails);

		CHECK_EQUAL(1, 123);    line = __LINE__;
    }

    CHECK_EQUAL("testName", reporter.lastFailedTest);
    CHECK_EQUAL("suiteName", reporter.lastFailedSuite);
    CHECK_EQUAL("filename", reporter.lastFailedFile);
    CHECK_EQUAL(line, reporter.lastFailedLine);
}

TEST(CheckEqualFailureBecauseOfExceptionContainsCorrectDetails)
{
    int line = 0;
    RecordingReporter reporter;
    {
        UnitTest::TestResults testResults(&reporter);
		UnitTest::TestDetails const testDetails("testName", "suiteName", "filename", -1);
		ScopedCurrentTest scopedResults(testResults, &testDetails);

		CHECK_EQUAL(ThrowingFunction(), 123);    line = __LINE__;
    }

    CHECK_EQUAL("testName", reporter.lastFailedTest);
    CHECK_EQUAL("suiteName", reporter.lastFailedSuite);
    CHECK_EQUAL("filename", reporter.lastFailedFile);
    CHECK_EQUAL(line, reporter.lastFailedLine);
}

TEST(CheckEqualFailureBecauseOfExceptionIncludesCheckContents)
{
    RecordingReporter reporter;
    {
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);
        CHECK_EQUAL(ThrowingFunction(), 123);
    }

    CHECK(strstr(reporter.lastFailedMessage, "ThrowingFunction()"));
    CHECK(strstr(reporter.lastFailedMessage, "123"));
}

int g_sideEffect = 0;
int FunctionWithSideEffects()
{
    ++g_sideEffect;
    return 1;
}

TEST(CheckEqualDoesNotHaveSideEffectsWhenPassing)
{
    g_sideEffect = 0;
    {
        UnitTest::TestResults testResults;
		ScopedCurrentTest scopedResults(testResults);
        CHECK_EQUAL(1, FunctionWithSideEffects());
    }
    CHECK_EQUAL(1, g_sideEffect);
}

TEST(CheckEqualDoesNotHaveSideEffectsWhenFailing)
{
    g_sideEffect = 0;
    {
        UnitTest::TestResults testResults;
		ScopedCurrentTest scopedResults(testResults);
        CHECK_EQUAL(2, FunctionWithSideEffects());
    }
    CHECK_EQUAL(1, g_sideEffect);
}


TEST(CheckCloseSucceedsOnEqual)
{
    bool failure = true;
    {
        RecordingReporter reporter;
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);
        CHECK_CLOSE (1.0f, 1.001f, 0.01f);
        failure = (testResults.GetFailureCount() > 0);
    }

    CHECK(!failure);
}

TEST(CheckCloseFailsOnNotEqual)
{
    bool failure = false;
    {
        RecordingReporter reporter;
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);
        CHECK_CLOSE (1.0f, 1.1f, 0.01f);
        failure = (testResults.GetFailureCount() > 0);
    }

    CHECK(failure);
}

TEST(CheckCloseFailsOnException)
{
    bool failure = false;
    {
        RecordingReporter reporter;
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);
        CHECK_CLOSE ((float)ThrowingFunction(), 1.0001f, 0.1f);
        failure = (testResults.GetFailureCount() > 0);
    }

    CHECK(failure);
}

TEST(CheckCloseFailureContainsCorrectDetails)
{
    int line = 0;
    RecordingReporter reporter;
    {
        UnitTest::TestResults testResults(&reporter);
		UnitTest::TestDetails testDetails("test", "suite", "filename", -1);
		ScopedCurrentTest scopedResults(testResults, &testDetails);

		CHECK_CLOSE (1.0f, 1.1f, 0.01f);    line = __LINE__;
    }

    CHECK_EQUAL("test", reporter.lastFailedTest);
    CHECK_EQUAL("suite", reporter.lastFailedSuite);
    CHECK_EQUAL("filename", reporter.lastFailedFile);
    CHECK_EQUAL(line, reporter.lastFailedLine);
}

TEST(CheckCloseFailureBecauseOfExceptionContainsCorrectDetails)
{
    int line = 0;
    RecordingReporter reporter;
    {
        UnitTest::TestResults testResults(&reporter);
		UnitTest::TestDetails testDetails("closeTest", "closeSuite", "filename", -1);
		ScopedCurrentTest scopedResults(testResults, &testDetails);
        CHECK_CLOSE ((float)ThrowingFunction(), 1.0001f, 0.1f);    line = __LINE__;
    }

    CHECK_EQUAL("closeTest", reporter.lastFailedTest);
    CHECK_EQUAL("closeSuite", reporter.lastFailedSuite);
    CHECK_EQUAL("filename", reporter.lastFailedFile);
    CHECK_EQUAL(line, reporter.lastFailedLine);
}

TEST(CheckCloseFailureBecauseOfExceptionIncludesCheckContents)
{
    RecordingReporter reporter;
    {
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);
        CHECK_CLOSE ((float)ThrowingFunction(), 1.0001f, 0.1f);
    }

    CHECK(strstr(reporter.lastFailedMessage, "(float)ThrowingFunction()"));
    CHECK(strstr(reporter.lastFailedMessage, "1.0001f"));
}

TEST(CheckCloseDoesNotHaveSideEffectsWhenPassing)
{
    g_sideEffect = 0;
    {
        UnitTest::TestResults testResults;
		ScopedCurrentTest scopedResults(testResults);
        CHECK_CLOSE (1, FunctionWithSideEffects(), 0.1f);
    }
    CHECK_EQUAL(1, g_sideEffect);
}

TEST(CheckCloseDoesNotHaveSideEffectsWhenFailing)
{
    g_sideEffect = 0;
    {
        UnitTest::TestResults testResults;
		ScopedCurrentTest scopedResults(testResults);
        CHECK_CLOSE (2, FunctionWithSideEffects(), 0.1f);
    }
    CHECK_EQUAL(1, g_sideEffect);
}


class ThrowingObject
{
public:
    float operator[](int) const
    {
        throw "Test throw";
    }
};


TEST(CheckArrayCloseSucceedsOnEqual)
{
    bool failure = true;
    {
        RecordingReporter reporter;
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);
        const float data[4] = { 0, 1, 2, 3 };
        CHECK_ARRAY_CLOSE (data, data, 4, 0.01f);
        failure = (testResults.GetFailureCount() > 0);
    }

    CHECK(!failure);
}

TEST(CheckArrayCloseFailsOnNotEqual)
{
    bool failure = false;
    {
        RecordingReporter reporter;
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);

		int const data1[4] = { 0, 1, 2, 3 };
        int const data2[4] = { 0, 1, 3, 3 };
		CHECK_ARRAY_CLOSE (data1, data2, 4, 0.01f);

		failure = (testResults.GetFailureCount() > 0);
    }

    CHECK(failure);
}

TEST(CheckArrayCloseFailureIncludesCheckExpectedAndActual)
{
    RecordingReporter reporter;
    {
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);

		int const data1[4] = { 0, 1, 2, 3 };
        int const data2[4] = { 0, 1, 3, 3 };
        CHECK_ARRAY_CLOSE (data1, data2, 4, 0.01f);
    }

    CHECK(strstr(reporter.lastFailedMessage, "xpected [ 0 1 2 3 ]"));
    CHECK(strstr(reporter.lastFailedMessage, "was [ 0 1 3 3 ]"));
}

TEST(CheckArrayCloseFailureContainsCorrectDetails)
{
    int line = 0;
    RecordingReporter reporter;
    {
        UnitTest::TestResults testResults(&reporter);
		UnitTest::TestDetails testDetails("arrayCloseTest", "arrayCloseSuite", "filename", -1);
		ScopedCurrentTest scopedResults(testResults, &testDetails);

		int const data1[4] = { 0, 1, 2, 3 };
        int const data2[4] = { 0, 1, 3, 3 };
        CHECK_ARRAY_CLOSE (data1, data2, 4, 0.01f);     line = __LINE__;
    }

    CHECK_EQUAL("arrayCloseTest", reporter.lastFailedTest);
    CHECK_EQUAL("arrayCloseSuite", reporter.lastFailedSuite);
    CHECK_EQUAL("filename", reporter.lastFailedFile);
    CHECK_EQUAL(line, reporter.lastFailedLine);
}

TEST(CheckArrayCloseFailureBecauseOfExceptionContainsCorrectDetails)
{
    int line = 0;
    RecordingReporter reporter;
    {
        UnitTest::TestResults testResults(&reporter);
		UnitTest::TestDetails testDetails("arrayCloseTest", "arrayCloseSuite", "filename", -1);
		ScopedCurrentTest scopedResults(testResults, &testDetails);

		int const data[4] = { 0, 1, 2, 3 };
        CHECK_ARRAY_CLOSE (data, ThrowingObject(), 4, 0.01f);     line = __LINE__;
    }

    CHECK_EQUAL("arrayCloseTest", reporter.lastFailedTest);
    CHECK_EQUAL("arrayCloseSuite", reporter.lastFailedSuite);
    CHECK_EQUAL("filename", reporter.lastFailedFile);
    CHECK_EQUAL(line, reporter.lastFailedLine);
}

TEST(CheckArrayCloseFailureIncludesTolerance)
{
    RecordingReporter reporter;
    {
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);

		float const data1[4] = { 0, 1, 2, 3 };
        float const data2[4] = { 0, 1, 3, 3 };
        CHECK_ARRAY_CLOSE (data1, data2, 4, 0.01f);
    }

    CHECK(strstr(reporter.lastFailedMessage, "0.01"));
}


TEST(CheckArrayCloseFailsOnException)
{
    bool failure = false;
    {
        RecordingReporter reporter;
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);

		const float data[4] = { 0, 1, 2, 3 };
        ThrowingObject obj;
        CHECK_ARRAY_CLOSE (data, obj, 3, 0.01f);

		failure = (testResults.GetFailureCount() > 0);
    }

    CHECK(failure);
}

TEST(CheckArrayCloseFailureOnExceptionIncludesCheckContents)
{
    RecordingReporter reporter;
    {
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);

		const float data[4] = { 0, 1, 2, 3 };
        ThrowingObject obj;
        CHECK_ARRAY_CLOSE (data, obj, 3, 0.01f);
    }

    CHECK(strstr(reporter.lastFailedMessage, "data"));
    CHECK(strstr(reporter.lastFailedMessage, "obj"));
}


TEST(CheckArrayEqualSuceedsOnEqual)
{
    bool failure = true;
    {
        RecordingReporter reporter;
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);

		const float data[4] = { 0, 1, 2, 3 };
        CHECK_ARRAY_EQUAL (data, data, 4);

		failure = (testResults.GetFailureCount() > 0);
    }

    CHECK(!failure);
}

TEST(CheckArrayEqualFailsOnNotEqual)
{
    bool failure = false;
    {
        RecordingReporter reporter;
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);

		int const data1[4] = { 0, 1, 2, 3 };
        int const data2[4] = { 0, 1, 3, 3 };
        CHECK_ARRAY_EQUAL (data1, data2, 4);

		failure = (testResults.GetFailureCount() > 0);
    }

    CHECK(failure);
}

TEST(CheckArrayEqualFailureIncludesCheckExpectedAndActual)
{
    RecordingReporter reporter;
    {
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);

		int const data1[4] = { 0, 1, 2, 3 };
        int const data2[4] = { 0, 1, 3, 3 };
        CHECK_ARRAY_EQUAL (data1, data2, 4);
    }

    CHECK(strstr(reporter.lastFailedMessage, "xpected [ 0 1 2 3 ]"));
    CHECK(strstr(reporter.lastFailedMessage, "was [ 0 1 3 3 ]"));
}

TEST(CheckArrayEqualFailureContainsCorrectInfo)
{
    int line = 0;
    RecordingReporter reporter;
    {
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);

		int const data1[4] = { 0, 1, 2, 3 };
        int const data2[4] = { 0, 1, 3, 3 };
        CHECK_ARRAY_EQUAL (data1, data2, 4);     line = __LINE__;
    }

    CHECK_EQUAL("CheckArrayEqualFailureContainsCorrectInfo", reporter.lastFailedTest);
    CHECK_EQUAL(__FILE__, reporter.lastFailedFile);
    CHECK_EQUAL(line, reporter.lastFailedLine);
}

TEST(CheckArrayEqualFailsOnException)
{
    bool failure = false;
    {
        RecordingReporter reporter;
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);

		const float data[4] = { 0, 1, 2, 3 };
        ThrowingObject obj;
        CHECK_ARRAY_EQUAL (data, obj, 3);

		failure = (testResults.GetFailureCount() > 0);
    }

    CHECK(failure);
}

TEST(CheckArrayEqualFailureOnExceptionIncludesCheckContents)
{
    RecordingReporter reporter;
    {
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);

		const float data[4] = { 0, 1, 2, 3 };
        ThrowingObject obj;
        CHECK_ARRAY_EQUAL (data, obj, 3);
    }

    CHECK(strstr(reporter.lastFailedMessage, "data"));
    CHECK(strstr(reporter.lastFailedMessage, "obj"));
}

float const* FunctionWithSideEffects2()
{
    ++g_sideEffect;
    static float const data[] = {1,2,3,4};
    return data;
}

TEST(CheckArrayCloseDoesNotHaveSideEffectsWhenPassing)
{
    g_sideEffect = 0;
    {
        UnitTest::TestResults testResults;
		ScopedCurrentTest scopedResults(testResults);

		const float data[] = { 0, 1, 2, 3 };
        CHECK_ARRAY_CLOSE (data, FunctionWithSideEffects2(), 4, 0.01f);
    }
    CHECK_EQUAL(1, g_sideEffect);
}

TEST(CheckArrayCloseDoesNotHaveSideEffectsWhenFailing)
{
    g_sideEffect = 0;
    {
        UnitTest::TestResults testResults;
		ScopedCurrentTest scopedResults(testResults);

		const float data[] = { 0, 1, 3, 3 };
        CHECK_ARRAY_CLOSE (data, FunctionWithSideEffects2(), 4, 0.01f);
    }

	CHECK_EQUAL(1, g_sideEffect);
}

class ThrowingObject2D
{
public:
    float* operator[](int) const
    {
        throw "Test throw";
    }
};


TEST(CheckArray2DCloseSucceedsOnEqual)
{
    bool failure = true;
    {
        RecordingReporter reporter;
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);

		const float data[2][2] = { {0, 1}, {2, 3} };
        CHECK_ARRAY2D_CLOSE (data, data, 2, 2, 0.01f);

		failure = (testResults.GetFailureCount() > 0);
    }

    CHECK(!failure);
}

TEST(CheckArray2DCloseFailsOnNotEqual)
{
    bool failure = false;
    {
        RecordingReporter reporter;
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);

		int const data1[2][2] = { {0, 1}, {2, 3} };
        int const data2[2][2] = { {0, 1}, {3, 3} };
        CHECK_ARRAY2D_CLOSE (data1, data2, 2, 2, 0.01f);

		failure = (testResults.GetFailureCount() > 0);
    }

    CHECK(failure);
}

TEST(CheckArray2DCloseFailureIncludesCheckExpectedAndActual)
{
    RecordingReporter reporter;
    {
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);

		int const data1[2][2] = { {0, 1}, {2, 3} };
        int const data2[2][2] = { {0, 1}, {3, 3} };

		CHECK_ARRAY2D_CLOSE (data1, data2, 2, 2, 0.01f);
    }

    CHECK(strstr(reporter.lastFailedMessage, "xpected [ [ 0 1 ] [ 2 3 ] ]"));
    CHECK(strstr(reporter.lastFailedMessage, "was [ [ 0 1 ] [ 3 3 ] ]"));
}

TEST(CheckArray2DCloseFailureContainsCorrectDetails)
{
    int line = 0;
    RecordingReporter reporter;
    {
        UnitTest::TestResults testResults(&reporter);
		UnitTest::TestDetails testDetails("array2DCloseTest", "array2DCloseSuite", "filename", -1);
		ScopedCurrentTest scopedResults(testResults, &testDetails);

		int const data1[2][2] = { {0, 1}, {2, 3} };
        int const data2[2][2] = { {0, 1}, {3, 3} };
		CHECK_ARRAY2D_CLOSE (data1, data2, 2, 2, 0.01f);     line = __LINE__;
    }

    CHECK_EQUAL("array2DCloseTest", reporter.lastFailedTest);
    CHECK_EQUAL("array2DCloseSuite", reporter.lastFailedSuite);
    CHECK_EQUAL("filename", reporter.lastFailedFile);
    CHECK_EQUAL(line, reporter.lastFailedLine);
}

TEST(CheckArray2DCloseFailureBecauseOfExceptionContainsCorrectDetails)
{
    int line = 0;
    RecordingReporter reporter;
    {
        UnitTest::TestResults testResults(&reporter);
		UnitTest::TestDetails testDetails("array2DCloseTest", "array2DCloseSuite", "filename", -1);
		ScopedCurrentTest scopedResults(testResults, &testDetails);

		const float data[2][2] = { {0, 1}, {2, 3} };
        CHECK_ARRAY2D_CLOSE (data, ThrowingObject2D(), 2, 2, 0.01f);   line = __LINE__;
    }

    CHECK_EQUAL("array2DCloseTest", reporter.lastFailedTest);
    CHECK_EQUAL("array2DCloseSuite", reporter.lastFailedSuite);
    CHECK_EQUAL("filename", reporter.lastFailedFile);
    CHECK_EQUAL(line, reporter.lastFailedLine);
}

TEST(CheckArray2DCloseFailureIncludesTolerance)
{
    RecordingReporter reporter;
    {
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);

		float const data1[2][2] = { {0, 1}, {2, 3} };
        float const data2[2][2] = { {0, 1}, {3, 3} };
        CHECK_ARRAY2D_CLOSE (data1, data2, 2, 2, 0.01f);
    }

    CHECK(strstr(reporter.lastFailedMessage, "0.01"));
}

TEST(CheckArray2DCloseFailsOnException)
{
    bool failure = false;
    {
        RecordingReporter reporter;
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);

		const float data[2][2] = { {0, 1}, {2, 3} };
        ThrowingObject2D obj;
        CHECK_ARRAY2D_CLOSE (data, obj, 2, 2, 0.01f);

		failure = (testResults.GetFailureCount() > 0);
    }

    CHECK(failure);
}

TEST(CheckArray2DCloseFailureOnExceptionIncludesCheckContents)
{
    RecordingReporter reporter;
    {
        UnitTest::TestResults testResults(&reporter);
		ScopedCurrentTest scopedResults(testResults);

		const float data[2][2] = { {0, 1}, {2, 3} };
        ThrowingObject2D obj;
        CHECK_ARRAY2D_CLOSE (data, obj, 2, 2, 0.01f);
    }

    CHECK(strstr(reporter.lastFailedMessage, "data"));
    CHECK(strstr(reporter.lastFailedMessage, "obj"));
}

float const* const* FunctionWithSideEffects3()
{
    ++g_sideEffect;
    static float const data1[] = {0,1};
    static float const data2[] = {2,3};
    static const float* const data[] = {data1, data2};
    return data;
}

TEST(CheckArray2DCloseDoesNotHaveSideEffectsWhenPassing)
{
    g_sideEffect = 0;
    {
        UnitTest::TestResults testResults;
		ScopedCurrentTest scopedResults(testResults);

		const float data[2][2] = { {0, 1}, {2, 3} };
        CHECK_ARRAY2D_CLOSE (data, FunctionWithSideEffects3(), 2, 2, 0.01f);
    }
    CHECK_EQUAL(1, g_sideEffect);
}

TEST(CheckArray2DCloseDoesNotHaveSideEffectsWhenFailing)
{
    g_sideEffect = 0;
    {
        UnitTest::TestResults testResults;
		ScopedCurrentTest scopedResults(testResults);

		const float data[2][2] = { {0, 1}, {3, 3} };
        CHECK_ARRAY2D_CLOSE (data, FunctionWithSideEffects3(), 2, 2, 0.01f);
    }
    CHECK_EQUAL(1, g_sideEffect);
}

}
