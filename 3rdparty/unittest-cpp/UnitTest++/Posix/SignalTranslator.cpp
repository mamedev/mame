#include "SignalTranslator.h"

namespace UnitTest {

sigjmp_buf* SignalTranslator::s_jumpTarget = 0;

namespace {

void SignalHandler(int sig)
{
    siglongjmp(*SignalTranslator::s_jumpTarget, sig );
}

}


SignalTranslator::SignalTranslator()
{
    m_oldJumpTarget = s_jumpTarget;
    s_jumpTarget = &m_currentJumpTarget;

    struct sigaction action;
    action.sa_flags = 0;
    action.sa_handler = SignalHandler;
    sigemptyset( &action.sa_mask );

    sigaction( SIGSEGV, &action, &m_old_SIGSEGV_action );
    sigaction( SIGFPE , &action, &m_old_SIGFPE_action  );
    sigaction( SIGTRAP, &action, &m_old_SIGTRAP_action );
    sigaction( SIGBUS , &action, &m_old_SIGBUS_action  );
    sigaction( SIGILL , &action, &m_old_SIGBUS_action  );
}

SignalTranslator::~SignalTranslator()
{
    sigaction( SIGILL , &m_old_SIGBUS_action , 0 );
    sigaction( SIGBUS , &m_old_SIGBUS_action , 0 );
    sigaction( SIGTRAP, &m_old_SIGTRAP_action, 0 );
    sigaction( SIGFPE , &m_old_SIGFPE_action , 0 );
    sigaction( SIGSEGV, &m_old_SIGSEGV_action, 0 );

    s_jumpTarget = m_oldJumpTarget;
}


}
