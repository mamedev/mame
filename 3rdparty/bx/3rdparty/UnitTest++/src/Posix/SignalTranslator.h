#ifndef UNITTEST_SIGNALTRANSLATOR_H
#define UNITTEST_SIGNALTRANSLATOR_H

#include <setjmp.h>
#include <signal.h>

namespace UnitTest {

class SignalTranslator
{
public:
    SignalTranslator();
    ~SignalTranslator();

#if defined(__native_client__)
#else
    static sigjmp_buf* s_jumpTarget;

private:
    sigjmp_buf m_currentJumpTarget;
    sigjmp_buf* m_oldJumpTarget;

    struct sigaction m_old_SIGFPE_action;
    struct sigaction m_old_SIGTRAP_action;
    struct sigaction m_old_SIGSEGV_action;
    struct sigaction m_old_SIGBUS_action;
//    struct sigaction m_old_SIGABRT_action;
//    struct sigaction m_old_SIGALRM_action;
#endif // defined(__native_client__)
};

#if !defined(__GNUC__) && !defined(__clang__)
#	define UNITTEST_EXTENSION
#else
#	define UNITTEST_EXTENSION __extension__
#endif

#if defined(__native_client__)
#	define UNITTEST_THROW_SIGNALS
#else
#	define UNITTEST_THROW_SIGNALS \
		UnitTest::SignalTranslator sig; \
		if (UNITTEST_EXTENSION sigsetjmp(*UnitTest::SignalTranslator::s_jumpTarget, 1) != 0) \
			throw ("Unhandled system exception"); 
#endif // defined(__native_client__)

}

#endif
