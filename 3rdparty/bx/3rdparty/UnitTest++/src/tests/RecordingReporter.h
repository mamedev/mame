#ifndef UNITTEST_RECORDINGREPORTER_H
#define UNITTEST_RECORDINGREPORTER_H

#include "../TestReporter.h"
#include <cstring>

#include "../TestDetails.h"

struct RecordingReporter : public UnitTest::TestReporter
{
private:
    enum { kMaxStringLength = 256 };

public:
    RecordingReporter()
        : testRunCount(0)
        , testFailedCount(0)
        , lastFailedLine(0)
        , testFinishedCount(0)
        , lastFinishedTestTime(0)
        , summaryTotalTestCount(0)
        , summaryFailedTestCount(0)
        , summaryFailureCount(0)
        , summarySecondsElapsed(0)
    {
        lastStartedSuite[0] = '\0';
        lastStartedTest[0] = '\0';
        lastFailedFile[0] = '\0';
        lastFailedSuite[0] = '\0';
        lastFailedTest[0] = '\0';
        lastFailedMessage[0] = '\0';
        lastFinishedSuite[0] = '\0';
        lastFinishedTest[0] = '\0';
    }

    virtual void ReportTestStart(UnitTest::TestDetails const& test)
    {
		using namespace std;

        ++testRunCount;
        strcpy(lastStartedSuite, test.suiteName);
        strcpy(lastStartedTest, test.testName);
    }

    virtual void ReportFailure(UnitTest::TestDetails const& test, char const* failure)
    {
		using namespace std;

		++testFailedCount;
        strcpy(lastFailedFile, test.filename);
        lastFailedLine = test.lineNumber;
        strcpy(lastFailedSuite, test.suiteName);
        strcpy(lastFailedTest, test.testName);
        strcpy(lastFailedMessage, failure);
    }

    virtual void ReportTestFinish(UnitTest::TestDetails const& test, float testDuration)
    {
		using namespace std;

		++testFinishedCount;
        strcpy(lastFinishedSuite, test.suiteName);
        strcpy(lastFinishedTest, test.testName);
        lastFinishedTestTime = testDuration;
    }

    virtual void ReportSummary(int totalTestCount, int failedTestCount, int failureCount, float secondsElapsed) 
    {
        summaryTotalTestCount = totalTestCount;
        summaryFailedTestCount = failedTestCount;
        summaryFailureCount = failureCount;
        summarySecondsElapsed = secondsElapsed;
    }

    int testRunCount;
    char lastStartedSuite[kMaxStringLength];
    char lastStartedTest[kMaxStringLength];

    int testFailedCount;
    char lastFailedFile[kMaxStringLength];
    int lastFailedLine;
    char lastFailedSuite[kMaxStringLength];
    char lastFailedTest[kMaxStringLength];
    char lastFailedMessage[kMaxStringLength];

    int testFinishedCount;
    char lastFinishedSuite[kMaxStringLength];
    char lastFinishedTest[kMaxStringLength];
    float lastFinishedTestTime;

    int summaryTotalTestCount;
    int summaryFailedTestCount;
    int summaryFailureCount;
    float summarySecondsElapsed;
};


#endif
