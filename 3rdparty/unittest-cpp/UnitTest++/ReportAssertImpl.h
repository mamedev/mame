#ifndef UNITTEST_REPORTASSERTIMPL_H
#define UNITTEST_REPORTASSERTIMPL_H

#include "Config.h"
#include "HelperMacros.h"

#ifdef UNITTEST_NO_EXCEPTIONS
	#include <csetjmp>
#endif

namespace UnitTest {

class TestResults;
class TestDetails;

namespace Detail {

UNITTEST_LINKAGE void ExpectAssert(bool expected);

UNITTEST_LINKAGE void ReportAssertEx(TestResults* testResults, 
									 const TestDetails* testDetails,
									 char const* description, 
									 char const* filename, 
									 int lineNumber);

UNITTEST_LINKAGE bool AssertExpected();

#ifdef UNITTEST_NO_EXCEPTIONS
	UNITTEST_LINKAGE UNITTEST_JMPBUF* GetAssertJmpBuf();

	#ifdef UNITTEST_WIN32
		#define UNITTEST_SET_ASSERT_JUMP_TARGET() \
			__pragma(warning(push)) __pragma(warning(disable:4611)) \
			UNITTEST_SETJMP(*UnitTest::Detail::GetAssertJmpBuf()) \
			__pragma(warning(pop))
	#else
		#define UNITTEST_SET_ASSERT_JUMP_TARGET() UNITTEST_SETJMP(*UnitTest::Detail::GetAssertJmpBuf())
	#endif

	#define UNITTEST_JUMP_TO_ASSERT_JUMP_TARGET() UNITTEST_LONGJMP(*UnitTest::Detail::GetAssertJmpBuf(), 1)
#endif

}
}

#endif
