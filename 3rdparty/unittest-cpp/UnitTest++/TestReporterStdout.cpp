#include "TestReporterStdout.h"
#include <cstdio>

#include "TestDetails.h"

// cstdio doesn't pull in namespace std on VC6, so we do it here.
#if defined(UNITTEST_WIN32) && (_MSC_VER == 1200)
	namespace std {}
#endif

namespace UnitTest {

void TestReporterStdout::ReportFailure(TestDetails const& details, char const* failure)
{
    using namespace std;
#if defined(__APPLE__) || defined(__GNUG__)
    char const* const errorFormat = "%s:%d:%d: error: Failure in %s: %s\n";
    fprintf(stderr, errorFormat, details.filename, details.lineNumber, 1, details.testName, failure);
#else
    char const* const errorFormat = "%s(%d): error: Failure in %s: %s\n";
    fprintf(stderr, errorFormat, details.filename, details.lineNumber, details.testName, failure);
#endif
}

void TestReporterStdout::ReportTestStart(TestDetails const& /*test*/)
{
}

void TestReporterStdout::ReportTestFinish(TestDetails const& /*test*/, float)
{
}

void TestReporterStdout::ReportSummary(int const totalTestCount, int const failedTestCount,
                                       int const failureCount, float const secondsElapsed)
{
	using namespace std;

    if (failureCount > 0)
        printf("FAILURE: %d out of %d tests failed (%d failures).\n", failedTestCount, totalTestCount, failureCount);
    else
        printf("Success: %d tests passed.\n", totalTestCount);

    printf("Test time: %.2f seconds.\n", secondsElapsed);
}

}
