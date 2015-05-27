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

    static sigjmp_buf* s_jumpTarget;

private:
    sigjmp_buf m_currentJumpTarget;
    sigjmp_buf* m_oldJumpTarget;

    struct sigaction m_old_SIGFPE_action;
    struct sigaction m_old_SIGTRAP_action;
    struct sigaction m_old_SIGSEGV_action;
    struct sigaction m_old_SIGBUS_action;
    struct sigaction m_old_SIGABRT_action;
    struct sigaction m_old_SIGALRM_action;
};

#if !defined (__GNUC__)
    #define UNITTEST_EXTENSION
#else
    #define UNITTEST_EXTENSION __extension__
#endif

#define UNITTEST_THROW_SIGNALS_POSIX_ONLY \
	UnitTest::SignalTranslator sig; \
	if (UNITTEST_EXTENSION sigsetjmp(*UnitTest::SignalTranslator::s_jumpTarget, 1) != 0) \
        throw ("Unhandled system exception"); 

}

#endif
