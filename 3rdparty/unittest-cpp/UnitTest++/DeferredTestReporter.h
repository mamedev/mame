#ifndef UNITTEST_DEFERREDTESTREPORTER_H
#define UNITTEST_DEFERREDTESTREPORTER_H

#include "Config.h"

#ifndef UNITTEST_NO_DEFERRED_REPORTER

#include "TestReporter.h"
#include "DeferredTestResult.h"

#include <vector>

UNITTEST_STDVECTOR_LINKAGE(UnitTest::DeferredTestResult)

namespace UnitTest
{

class UNITTEST_LINKAGE DeferredTestReporter : public TestReporter
{
public:
    virtual void ReportTestStart(TestDetails const& details);
    virtual void ReportFailure(TestDetails const& details, char const* failure);
    virtual void ReportTestFinish(TestDetails const& details, float secondsElapsed);

    typedef std::vector< DeferredTestResult > DeferredTestResultList;
    DeferredTestResultList& GetResults();

private:
    DeferredTestResultList m_results;
};

}

#endif
#endif
