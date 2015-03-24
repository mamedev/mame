#include "CurrentTest.h"
#include <cstddef>

namespace UnitTest {

TestResults*& CurrentTest::Results()
{
	static TestResults* testResults = NULL;
	return testResults;
}

const TestDetails*& CurrentTest::Details()
{
	static const TestDetails* testDetails = NULL;
	return testDetails;
}

}
