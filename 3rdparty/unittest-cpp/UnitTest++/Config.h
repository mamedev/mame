#ifndef UNITTEST_CONFIG_H
#define UNITTEST_CONFIG_H

// Standard defines documented here: http://predef.sourceforge.net

#if defined(_MSC_VER)
	#pragma warning(disable:4702) // unreachable code
	#pragma warning(disable:4722) // destructor never returns, potential memory leak

#if (_MSC_VER == 1200)  // VC6
		#define UNITTEST_COMPILER_IS_MSVC6
		#pragma warning(disable:4786)
		#pragma warning(disable:4290)
	#endif

	#ifdef _USRDLL
		#define UNITTEST_WIN32_DLL
	#endif
	#define UNITTEST_WIN32
#endif

#if defined(unix) || defined(__unix__) || defined(__unix) || defined(linux) || \
    defined(__APPLE__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__FreeBSD__)        
    #define UNITTEST_POSIX
#endif

#if defined(__MINGW32__)
    #define UNITTEST_MINGW
#endif


// By default, MemoryOutStream is implemented in terms of std::ostringstream.
// This is useful if you are using the CHECK macros on objects that have something like this defined:
// std::ostringstream& operator<<(std::ostringstream& s, const YourObject& value)
// 
// On the other hand, it can be more expensive.
// Un-comment this line to use the custom MemoryOutStream (no deps on std::ostringstream).

// #define UNITTEST_USE_CUSTOM_STREAMS

// Developer note: This dual-macro setup is to preserve compatibility with UnitTest++ 1.4 users
// who may have used or defined UNITTEST_USE_CUSTOM_STREAMS outside of this configuration file, as
// well as Google Code HEAD users that may have used or defined
// UNITTEST_MEMORYOUTSTREAM_IS_STD_OSTRINGSTREAM outside of this configuration file.
#ifndef UNITTEST_USE_CUSTOM_STREAMS
    #define UNITTEST_MEMORYOUTSTREAM_IS_STD_OSTRINGSTREAM
#endif

// DeferredTestReporter uses the STL to collect test results for subsequent export by reporters like
// XmlTestReporter.  If you don't want to use this functionality, uncomment this line and no STL
// headers or code will be compiled into UnitTest++

//#define UNITTEST_NO_DEFERRED_REPORTER


// By default, asserts that you report via UnitTest::ReportAssert() abort the current test and
// continue to the next one by throwing an exception, which unwinds the stack naturally, destroying
// all auto variables on its way back down.  If you don't want to (or can't) use exceptions for your 
// platform/compiler, uncomment this line.  All exception code will be removed from UnitTest++,
// assert recovery will be done via setjmp/longjmp, and NO correct stack unwinding will happen!

//#define UNITTEST_NO_EXCEPTIONS


// std namespace qualification: used for functions like strcpy that 
// may live in std:: namespace (cstring header).
#if defined( UNITTEST_COMPILER_IS_MSVC6 )
	#define UNIITEST_NS_QUAL_STD(x) x
#else
	#define UNIITEST_NS_QUAL_STD(x) ::std::x
#endif

#endif
