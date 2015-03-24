#ifndef UNITTEST_DEFERREDTESTREPORTER_H
#define UNITTEST_DEFERREDTESTREPORTER_H

#include "TestReporter.h"
#include "DeferredTestResult.h"
#include "Config.h"

#include <vector>

namespace UnitTest
{

class DeferredTestReporter : public TestReporter
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
