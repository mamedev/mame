#include "TestReporterStdout.h"
#include <cstdio>

#include "TestDetails.h"

// cstdio doesn't pull in namespace std on VC6, so we do it here.
#if defined(_MSC_VER) && (_MSC_VER == 1200)
	namespace std {}
#endif

#if defined(__ANDROID__)
#	include <android/log.h>
#	define outf(format, ...) __android_log_print(ANDROID_LOG_DEBUG, "", format, ##__VA_ARGS__)
#else
#	define outf(format, ...) printf(format, ##__VA_ARGS__)
#endif // defined(__ANDROID__)

namespace UnitTest {

void TestReporterStdout::ReportFailure(TestDetails const& details, char const* failure)
{
#if defined(__APPLE__) || defined(__GNUG__)
    char const* const errorFormat = "%s:%d: error: Failure in %s: %s\n";
#else
    char const* const errorFormat = "%s(%d): error: Failure in %s: %s\n";
#endif

    using namespace std;
    outf(errorFormat, details.filename, details.lineNumber, details.testName, failure);
}

void TestReporterStdout::ReportTestStart(TestDetails const& /*test*/)
{
}

void TestReporterStdout::ReportTestFinish(TestDetails const& /*test*/, float)
{
}

void TestReporterStdout::ReportSummary(int const totalTestCount, int const failedTestCount,
                                       int const failureCount, float secondsElapsed)
{
	using namespace std;

    if (failureCount > 0)
	{
        outf("FAILURE: %d out of %d tests failed (%d failures).\n", failedTestCount, totalTestCount, failureCount);
	}
    else
	{
        outf("Success: %d tests passed.\n", totalTestCount);
	}

    outf("Test time: %.2f seconds.\n", secondsElapsed);
}

}
