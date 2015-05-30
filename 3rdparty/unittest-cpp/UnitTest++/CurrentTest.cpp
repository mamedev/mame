#include "CurrentTest.h"
#include <cstddef>

namespace UnitTest {

UNITTEST_LINKAGE TestResults*& CurrentTest::Results()
{
	static TestResults* testResults = NULL;
	return testResults;
}

UNITTEST_LINKAGE const TestDetails*& CurrentTest::Details()
{
	static const TestDetails* testDetails = NULL;
	return testDetails;
}

}
