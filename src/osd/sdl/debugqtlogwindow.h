#ifndef __DEBUG_QT_LOG_WINDOW_H__
#define __DEBUG_QT_LOG_WINDOW_H__

#include <QtGui/QtGui>

#include "debugqtview.h"
#include "debugqtwindow.h"


//============================================================
//  The Log Window.
//============================================================
class LogWindow : public WindowQt
{
    Q_OBJECT
    
public:
    LogWindow(running_machine* machine, QWidget* parent=NULL);
    virtual ~LogWindow() {}


private:
    // Widgets
    DebuggerView* m_logView;
};


#endif
