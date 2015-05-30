#ifndef UNITTEST_EXCEPTIONMACROS_H
#define UNITTEST_EXCEPTIONMACROS_H

#include "Config.h"

#ifndef UNITTEST_NO_EXCEPTIONS
	#define UT_TRY(x) try x
	#define UT_THROW(x) throw x
	#define UT_CATCH(ExceptionType, ExceptionName, CatchBody) catch(ExceptionType& ExceptionName) CatchBody
	#define UT_CATCH_ALL(CatchBody) catch(...) CatchBody
#else
	#define UT_TRY(x) x
	#define UT_THROW(x)
	#define UT_CATCH(ExceptionType, ExceptionName, CatchBody)
	#define UT_CATCH_ALL(CatchBody)
#endif

#endif
