#ifndef UNITTEST_TESTDETAILS_H
#define UNITTEST_TESTDETAILS_H

#include "HelperMacros.h"

namespace UnitTest {

class UNITTEST_LINKAGE TestDetails
{
public:
    TestDetails(char const* testName, char const* suiteName, char const* filename, int lineNumber);
    TestDetails(const TestDetails& details, int lineNumber);

    char const* const suiteName;
    char const* const testName;
    char const* const filename;
    int const lineNumber;
    mutable bool timeConstraintExempt;

    TestDetails(TestDetails const&); // Why is it public? --> http://gcc.gnu.org/bugs.html#cxx_rvalbind
private:
    TestDetails& operator=(TestDetails const&);
};

}

#endif
