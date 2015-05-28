#include "UnitTest++/Config.h"
#ifndef UNITTEST_NO_EXCEPTIONS

#include "UnitTest++/UnitTestPP.h"
#include "UnitTest++/CurrentTest.h"
#include "RecordingReporter.h"
#include "ScopedCurrentTest.h"

#include <stdexcept>

using namespace std;

namespace {

int ThrowingFunction()
{
    throw "Doh";
}

int ThrowingStdExceptionFunction()
{
    throw std::logic_error("Doh");
}

SUITE(CheckExceptionTests)
{
    struct CheckFixture
    {
        CheckFixture()
          : reporter()
          , testResults(&reporter)
        {
        }

        void PerformCheckWithNonStdThrow()
        {
            ScopedCurrentTest scopedResults(testResults);
            CHECK(ThrowingFunction() == 1);
        }

        void PerformCheckWithStdThrow()
        {
            ScopedCurrentTest scopedResults(testResults);
            CHECK(ThrowingStdExceptionFunction() == 1);
        }

        RecordingReporter reporter;
        UnitTest::TestResults testResults;
    };

    TEST_FIXTURE(CheckFixture, CheckFailsOnException)
    {
        PerformCheckWithNonStdThrow();
        CHECK(testResults.GetFailureCount() > 0);
    }

    TEST_FIXTURE(CheckFixture, CheckFailsOnStdException)
    {
        PerformCheckWithStdThrow();
        CHECK(testResults.GetFailureCount() > 0);
    }

    TEST_FIXTURE(CheckFixture, CheckFailureBecauseOfExceptionIncludesCheckContents)
    {
        PerformCheckWithNonStdThrow();
        CHECK(strstr(reporter.lastFailedMessage, "ThrowingFunction() == 1"));
    }

    TEST_FIXTURE(CheckFixture, CheckFailureBecauseOfStdExceptionIncludesCheckContents)
    {
        PerformCheckWithStdThrow();
        CHECK(strstr(reporter.lastFailedMessage, "ThrowingStdExceptionFunction() == 1"));
    }

    TEST_FIXTURE(CheckFixture, CheckFailureBecauseOfStandardExceptionIncludesWhat)
    {
        PerformCheckWithStdThrow();
        CHECK(strstr(reporter.lastFailedMessage, "exception (Doh)"));
    }
}

SUITE(CheckEqualExceptionTests)
{
    struct CheckEqualFixture
    {
        CheckEqualFixture()
          : reporter()
          , testResults(&reporter)
          , line(-1)
        {
        }

        void PerformCheckWithNonStdThrow()
        {
            UnitTest::TestDetails const testDetails("testName", "suiteName", "filename", -1);
            ScopedCurrentTest scopedResults(testResults, &testDetails);
            CHECK_EQUAL(ThrowingFunction(), 123); line = __LINE__;
        }

        void PerformCheckWithStdThrow()
        {
            UnitTest::TestDetails const testDetails("testName", "suiteName", "filename", -1);
            ScopedCurrentTest scopedResults(testResults, &testDetails);
            CHECK_EQUAL(ThrowingStdExceptionFunction(), 123); line = __LINE__;
        }

        RecordingReporter reporter;
        UnitTest::TestResults testResults;
        int line;
    };

    TEST_FIXTURE(CheckEqualFixture, CheckEqualFailsOnException)
    {
        PerformCheckWithNonStdThrow();
        CHECK(testResults.GetFailureCount() > 0);
    }

    TEST_FIXTURE(CheckEqualFixture, CheckEqualFailsOnStdException)
    {
        PerformCheckWithStdThrow();
        CHECK(testResults.GetFailureCount() > 0);   
    }

    TEST_FIXTURE(CheckEqualFixture, CheckEqualFailureBecauseOfExceptionContainsCorrectDetails)
    {
        PerformCheckWithNonStdThrow();

        CHECK_EQUAL("testName", reporter.lastFailedTest);
        CHECK_EQUAL("suiteName", reporter.lastFailedSuite);
        CHECK_EQUAL("filename", reporter.lastFailedFile);
        CHECK_EQUAL(line, reporter.lastFailedLine);
    }

    TEST_FIXTURE(CheckEqualFixture, CheckEqualFailureBecauseOfStdExceptionContainsCorrectDetails)
    {
        PerformCheckWithStdThrow();

        CHECK_EQUAL("testName", reporter.lastFailedTest);
        CHECK_EQUAL("suiteName", reporter.lastFailedSuite);
        CHECK_EQUAL("filename", reporter.lastFailedFile);
        CHECK_EQUAL(line, reporter.lastFailedLine);
    }

    TEST_FIXTURE(CheckEqualFixture, CheckEqualFailureBecauseOfExceptionIncludesCheckContents)
    {
        PerformCheckWithNonStdThrow();

        CHECK(strstr(reporter.lastFailedMessage, "ThrowingFunction()"));
        CHECK(strstr(reporter.lastFailedMessage, "123"));
    }

    TEST_FIXTURE(CheckEqualFixture, CheckEqualFailureBecauseOfStdExceptionIncludesCheckContents)
    {
        PerformCheckWithStdThrow();

        CHECK(strstr(reporter.lastFailedMessage, "ThrowingStdExceptionFunction()"));
        CHECK(strstr(reporter.lastFailedMessage, "123"));
    }

    TEST_FIXTURE(CheckEqualFixture, CheckEqualFailureBecauseOfStandardExceptionIncludesWhat)
    {
        PerformCheckWithStdThrow();

        CHECK(strstr(reporter.lastFailedMessage, "exception (Doh)"));
    }
}

SUITE(CheckCloseExceptionTests)
{
    struct CheckCloseFixture
    {
        CheckCloseFixture()
          : reporter()
          , testResults(&reporter)
          , line(-1)
        {
        }

        void PerformCheckWithNonStdThrow()
        {
            UnitTest::TestDetails const testDetails("closeTest", "closeSuite", "filename", -1);
            ScopedCurrentTest scopedResults(testResults, &testDetails);
            CHECK_CLOSE(static_cast<float>(ThrowingFunction()), 1.0001f, 0.1f); line = __LINE__;
        }

        void PerformCheckWithStdThrow()
        {
            UnitTest::TestDetails const testDetails("closeTest", "closeSuite", "filename", -1);
            ScopedCurrentTest scopedResults(testResults, &testDetails);
            CHECK_CLOSE(static_cast<float>(ThrowingStdExceptionFunction()), 1.0001f, 0.1f); line = __LINE__;
        }

        RecordingReporter reporter;
        UnitTest::TestResults testResults;
        int line;
    };

    TEST_FIXTURE(CheckCloseFixture, CheckCloseFailsOnException)
    {
        PerformCheckWithNonStdThrow();

        CHECK(testResults.GetFailureCount() > 0);
    }

    TEST_FIXTURE(CheckCloseFixture, CheckCloseFailsOnStdException)
    {
        PerformCheckWithStdThrow();

        CHECK(testResults.GetFailureCount() > 0);
    }

    TEST_FIXTURE(CheckCloseFixture, CheckCloseFailureBecauseOfExceptionContainsCorrectDetails)
    {
        PerformCheckWithNonStdThrow();

        CHECK_EQUAL("closeTest", reporter.lastFailedTest);
        CHECK_EQUAL("closeSuite", reporter.lastFailedSuite);
        CHECK_EQUAL("filename", reporter.lastFailedFile);
        CHECK_EQUAL(line, reporter.lastFailedLine);
    }

    TEST_FIXTURE(CheckCloseFixture, CheckCloseFailureBecauseOfStdExceptionContainsCorrectDetails)
    {
        PerformCheckWithStdThrow();

        CHECK_EQUAL("closeTest", reporter.lastFailedTest);
        CHECK_EQUAL("closeSuite", reporter.lastFailedSuite);
        CHECK_EQUAL("filename", reporter.lastFailedFile);
        CHECK_EQUAL(line, reporter.lastFailedLine);
    }

    TEST_FIXTURE(CheckCloseFixture, CheckCloseFailureBecauseOfExceptionIncludesCheckContents)
    {
        PerformCheckWithNonStdThrow();

        CHECK(strstr(reporter.lastFailedMessage, "static_cast<float>(ThrowingFunction())"));
        CHECK(strstr(reporter.lastFailedMessage, "1.0001f"));
    }

    TEST_FIXTURE(CheckCloseFixture, CheckCloseFailureBecauseOfStdExceptionIncludesCheckContents)
    {
        PerformCheckWithStdThrow();

        CHECK(strstr(reporter.lastFailedMessage, "static_cast<float>(ThrowingStdExceptionFunction())"));
        CHECK(strstr(reporter.lastFailedMessage, "1.0001f"));
    }

    TEST_FIXTURE(CheckCloseFixture, CheckCloseFailureBecauseOfStandardExceptionIncludesWhat)
    {
        PerformCheckWithStdThrow();

        CHECK(strstr(reporter.lastFailedMessage, "exception (Doh)"));
    }
}

class ThrowingObject
{
public:
    float operator[](int) const
    {
        throw "Test throw";
    }
};

class StdThrowingObject
{
public:
    float operator[](int) const
    {
        throw std::runtime_error("Test throw");
    }
};

SUITE(CheckArrayCloseExceptionTests)
{
    struct CheckArrayCloseFixture
    {
        CheckArrayCloseFixture()
          : reporter()
          , testResults(&reporter)
          , line(-1)
        {
        }

        void PerformCheckWithNonStdThrow()
        {
            UnitTest::TestDetails const testDetails("arrayCloseTest", "arrayCloseSuite", "filename", -1);
            ScopedCurrentTest scopedResults(testResults, &testDetails);
            int const data[4] = { 0, 1, 2, 3 };
            CHECK_ARRAY_CLOSE(data, ThrowingObject(), 4, 0.01f); line = __LINE__;
        }

        void PerformCheckWithStdThrow()
        {
            UnitTest::TestDetails const testDetails("arrayCloseTest", "arrayCloseSuite", "filename", -1);
            ScopedCurrentTest scopedResults(testResults, &testDetails);
            int const data[4] = { 0, 1, 2, 3 };
            CHECK_ARRAY_CLOSE(data, StdThrowingObject(), 4, 0.01f); line = __LINE__;
        }

        RecordingReporter reporter;
        UnitTest::TestResults testResults;
        int line;
    };

    TEST_FIXTURE(CheckArrayCloseFixture, CheckFailureBecauseOfExceptionContainsCorrectDetails)
    {
        PerformCheckWithNonStdThrow();

        CHECK_EQUAL("arrayCloseTest", reporter.lastFailedTest);
        CHECK_EQUAL("arrayCloseSuite", reporter.lastFailedSuite);
        CHECK_EQUAL("filename", reporter.lastFailedFile);
        CHECK_EQUAL(line, reporter.lastFailedLine);
    }

    TEST_FIXTURE(CheckArrayCloseFixture, CheckFailureBecauseOfStdExceptionContainsCorrectDetails)
    {
        PerformCheckWithStdThrow();

        CHECK_EQUAL("arrayCloseTest", reporter.lastFailedTest);
        CHECK_EQUAL("arrayCloseSuite", reporter.lastFailedSuite);
        CHECK_EQUAL("filename", reporter.lastFailedFile);
        CHECK_EQUAL(line, reporter.lastFailedLine);
    }

    TEST_FIXTURE(CheckArrayCloseFixture, CheckFailsOnException)
    {
        PerformCheckWithNonStdThrow();

        CHECK(testResults.GetFailureCount() > 0);
    }

    TEST_FIXTURE(CheckArrayCloseFixture, CheckFailsOnStdException)
    {
        PerformCheckWithStdThrow();

        CHECK(testResults.GetFailureCount() > 0);
    }

    TEST_FIXTURE(CheckArrayCloseFixture, CheckFailureOnExceptionIncludesCheckContents)
    {
        PerformCheckWithNonStdThrow();

        CHECK(strstr(reporter.lastFailedMessage, "data"));
        CHECK(strstr(reporter.lastFailedMessage, "ThrowingObject()"));
    }

    TEST_FIXTURE(CheckArrayCloseFixture, CheckFailureOnStdExceptionIncludesCheckContents)
    {
        PerformCheckWithStdThrow();

        CHECK(strstr(reporter.lastFailedMessage, "data"));
        CHECK(strstr(reporter.lastFailedMessage, "StdThrowingObject()"));
    }

    TEST_FIXTURE(CheckArrayCloseFixture, CheckFailureOnStdExceptionIncludesWhat)
    {
        PerformCheckWithStdThrow();

        CHECK(strstr(reporter.lastFailedMessage, "exception (Test throw)"));
    }
}

SUITE(CheckArrayEqualExceptionTests)
{
    struct CheckArrayEqualFixture
    {
        CheckArrayEqualFixture()
        : reporter()
        , testResults(&reporter)
        , line(-1)
        {
        }

        void PerformCheckWithNonStdThrow()
        {
            UnitTest::TestDetails const testDetails("arrayEqualTest", "arrayEqualSuite", "filename", -1);
            ScopedCurrentTest scopedResults(testResults, &testDetails);
            int const data[4] = { 0, 1, 2, 3 };
            CHECK_ARRAY_EQUAL(data, ThrowingObject(), 4); line = __LINE__;
        }

        void PerformCheckWithStdThrow()
        {
            UnitTest::TestDetails const testDetails("arrayEqualTest", "arrayEqualSuite", "filename", -1);
            ScopedCurrentTest scopedResults(testResults, &testDetails);
            int const data[4] = { 0, 1, 2, 3 };
            CHECK_ARRAY_EQUAL(data, StdThrowingObject(), 4); line = __LINE__;
        }

        RecordingReporter reporter;
        UnitTest::TestResults testResults;
        int line;
    };

    TEST_FIXTURE(CheckArrayEqualFixture, CheckFailureBecauseOfExceptionContainsCorrectDetails)
    {
        PerformCheckWithNonStdThrow();

        CHECK_EQUAL("arrayEqualTest", reporter.lastFailedTest);
        CHECK_EQUAL("arrayEqualSuite", reporter.lastFailedSuite);
        CHECK_EQUAL("filename", reporter.lastFailedFile);
        CHECK_EQUAL(line, reporter.lastFailedLine);
    }

    TEST_FIXTURE(CheckArrayEqualFixture, CheckFailureBecauseOfStdExceptionContainsCorrectDetails)
    {
        PerformCheckWithStdThrow();

        CHECK_EQUAL("arrayEqualTest", reporter.lastFailedTest);
        CHECK_EQUAL("arrayEqualSuite", reporter.lastFailedSuite);
        CHECK_EQUAL("filename", reporter.lastFailedFile);
        CHECK_EQUAL(line, reporter.lastFailedLine);
    }

    TEST_FIXTURE(CheckArrayEqualFixture, CheckFailsOnException)
    {
        PerformCheckWithNonStdThrow();

        CHECK(testResults.GetFailureCount() > 0);
    }

    TEST_FIXTURE(CheckArrayEqualFixture, CheckFailsOnStdException)
    {
        PerformCheckWithStdThrow();

        CHECK(testResults.GetFailureCount() > 0);
    }

    TEST_FIXTURE(CheckArrayEqualFixture, CheckFailureOnExceptionIncludesCheckContents)
    {
        PerformCheckWithNonStdThrow();

        CHECK(strstr(reporter.lastFailedMessage, "data"));
        CHECK(strstr(reporter.lastFailedMessage, "ThrowingObject()"));
    }

    TEST_FIXTURE(CheckArrayEqualFixture, CheckFailureOnStdExceptionIncludesCheckContents)
    {
        PerformCheckWithStdThrow();

        CHECK(strstr(reporter.lastFailedMessage, "data"));
        CHECK(strstr(reporter.lastFailedMessage, "StdThrowingObject()"));
    }

    TEST_FIXTURE(CheckArrayEqualFixture, CheckFailureOnStdExceptionIncludesWhat)
    {
        PerformCheckWithStdThrow();

        CHECK(strstr(reporter.lastFailedMessage, "exception (Test throw)"));
    }
}

SUITE(CheckArray2DExceptionTests)
{
    class ThrowingObject2D
    {
    public:
        float* operator[](int) const
        {
            throw "Test throw";
        }
    };

    class StdThrowingObject2D
    {
    public:
        float* operator[](int) const
        {
            throw std::runtime_error("Test throw");
        }
    };

    struct CheckArray2DCloseFixture
    {
        CheckArray2DCloseFixture()
          : reporter()
          , testResults(&reporter)
          , line(-1)
        {
        }

        void PerformCheckWithNonStdThrow()
        {
            UnitTest::TestDetails const testDetails("array2DCloseTest", "array2DCloseSuite", "filename", -1);
            ScopedCurrentTest scopedResults(testResults, &testDetails);
            const float data[2][2] = { {0, 1}, {2, 3} };
            CHECK_ARRAY2D_CLOSE(data, ThrowingObject2D(), 2, 2, 0.01f); line = __LINE__;
        }

        void PerformCheckWithStdThrow()
        {
            UnitTest::TestDetails const testDetails("array2DCloseTest", "array2DCloseSuite", "filename", -1);
            ScopedCurrentTest scopedResults(testResults, &testDetails);
            const float data[2][2] = { {0, 1}, {2, 3} };
            CHECK_ARRAY2D_CLOSE(data, StdThrowingObject2D(), 2, 2, 0.01f); line = __LINE__;
        }

        RecordingReporter reporter;
        UnitTest::TestResults testResults;
        int line;
    };

    TEST_FIXTURE(CheckArray2DCloseFixture, CheckFailureBecauseOfExceptionContainsCorrectDetails)
    {
        PerformCheckWithNonStdThrow();

        CHECK_EQUAL("array2DCloseTest", reporter.lastFailedTest);
        CHECK_EQUAL("array2DCloseSuite", reporter.lastFailedSuite);
        CHECK_EQUAL("filename", reporter.lastFailedFile);
        CHECK_EQUAL(line, reporter.lastFailedLine);
    }

    TEST_FIXTURE(CheckArray2DCloseFixture, CheckFailureBecauseOfStdExceptionContainsCorrectDetails)
    {
        PerformCheckWithStdThrow();

        CHECK_EQUAL("array2DCloseTest", reporter.lastFailedTest);
        CHECK_EQUAL("array2DCloseSuite", reporter.lastFailedSuite);
        CHECK_EQUAL("filename", reporter.lastFailedFile);
        CHECK_EQUAL(line, reporter.lastFailedLine);
    }

    TEST_FIXTURE(CheckArray2DCloseFixture, CheckFailsOnException)
    {
        PerformCheckWithNonStdThrow();

        CHECK(testResults.GetFailureCount() > 0);
    }

    TEST_FIXTURE(CheckArray2DCloseFixture, CheckFailsOnStdException)
    {
        PerformCheckWithStdThrow();

        CHECK(testResults.GetFailureCount() > 0);
    }

    TEST_FIXTURE(CheckArray2DCloseFixture, CheckFailureOnExceptionIncludesCheckContents)
    {
        PerformCheckWithNonStdThrow();

        CHECK(strstr(reporter.lastFailedMessage, "data"));
        CHECK(strstr(reporter.lastFailedMessage, "ThrowingObject2D()"));
    }

    TEST_FIXTURE(CheckArray2DCloseFixture, CheckFailureOnStdExceptionIncludesCheckContents)
    {
        PerformCheckWithStdThrow();

        CHECK(strstr(reporter.lastFailedMessage, "data"));
        CHECK(strstr(reporter.lastFailedMessage, "StdThrowingObject2D()"));
    }

    TEST_FIXTURE(CheckArray2DCloseFixture, CheckFailureOnStdExceptionIncludesWhat)
    {
        PerformCheckWithStdThrow();

        CHECK(strstr(reporter.lastFailedMessage, "exception (Test throw)"));
    }
}
}

#endif
