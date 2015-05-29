#include "ReportAssert.h"
#include "ReportAssertImpl.h"
#include "AssertException.h"
#include "CurrentTest.h"
#include "TestResults.h"
#include "TestDetails.h"

#ifdef UNITTEST_NO_EXCEPTIONS
	#include "ReportAssertImpl.h"
#endif

namespace UnitTest {

namespace
{
	bool& AssertExpectedFlag()
	{
		static bool s_assertExpected = false;
		return s_assertExpected;
	}
}

UNITTEST_LINKAGE void ReportAssert(char const* description, char const* filename, int lineNumber)
{
	Detail::ReportAssertEx(CurrentTest::Results(), CurrentTest::Details(), 
						   description, filename, lineNumber);
}

namespace Detail {

#ifdef UNITTEST_NO_EXCEPTIONS
UNITTEST_JMPBUF* GetAssertJmpBuf()
{
	static UNITTEST_JMPBUF s_jmpBuf;
	return &s_jmpBuf;
}
#endif

UNITTEST_LINKAGE void ReportAssertEx(TestResults* testResults,
									 const TestDetails* testDetails,
									 char const* description,
									 char const* filename,
									 int lineNumber)
{
	if (AssertExpectedFlag() == false)
	{
		TestDetails assertDetails(testDetails->testName, testDetails->suiteName, filename, lineNumber);
		testResults->OnTestFailure(assertDetails, description);
	}

	ExpectAssert(false);

#ifndef UNITTEST_NO_EXCEPTIONS
	throw AssertException();
#else
	UNITTEST_JUMP_TO_ASSERT_JUMP_TARGET();
#endif
}

UNITTEST_LINKAGE void ExpectAssert(bool expected)
{
	AssertExpectedFlag() = expected;
}

UNITTEST_LINKAGE bool AssertExpected()
{
	return AssertExpectedFlag();
}

}}
