#ifndef __DEBUG_QT_DASM_WINDOW_H__
#define __DEBUG_QT_DASM_WINDOW_H__

#include <QtGui/QtGui>

#include "debugqtview.h"
#include "debugqtwindow.h"


//============================================================
//  The Disassembly Window.
//============================================================
class DasmWindow : public WindowQt
{
    Q_OBJECT
    
public:
    DasmWindow(running_machine* machine, QWidget* parent=NULL);
    virtual ~DasmWindow() {}


private slots:
    void cpuChanged(int index);
    void expressionSubmitted();

    void toggleBreakpointAtCursor(bool changedTo);
    void runToCursor(bool changedTo);
    void rightBarChanged(QAction* changedTo);


private:
    void populateComboBox();

    
private:
    // Widgets
    QLineEdit* m_inputEdit;
    QComboBox* m_cpuComboBox;
    DebuggerView* m_dasmView;
};


#endif
