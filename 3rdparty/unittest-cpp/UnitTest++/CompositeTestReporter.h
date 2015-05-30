#ifndef UNITTEST_COMPOSITETESTREPORTER_H
#define UNITTEST_COMPOSITETESTREPORTER_H

#include "TestReporter.h"

namespace UnitTest {

class UNITTEST_LINKAGE CompositeTestReporter : public TestReporter
{
public:
    CompositeTestReporter();

	int GetReporterCount() const;
    bool AddReporter(TestReporter* reporter);
    bool RemoveReporter(TestReporter* reporter);

    virtual void ReportTestStart(TestDetails const& test);
    virtual void ReportFailure(TestDetails const& test, char const* failure);
    virtual void ReportTestFinish(TestDetails const& test, float secondsElapsed);
    virtual void ReportSummary(int totalTestCount, int failedTestCount, int failureCount, float secondsElapsed);

private:
	enum { kMaxReporters = 16 };
	TestReporter* m_reporters[kMaxReporters];
	int m_reporterCount;

	// revoked
	CompositeTestReporter(const CompositeTestReporter&);
	CompositeTestReporter& operator =(const CompositeTestReporter&);
};

}

#endif 
