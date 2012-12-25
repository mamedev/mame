#ifndef __DEBUG_QT_MEMORY_WINDOW_H__
#define __DEBUG_QT_MEMORY_WINDOW_H__

#include <QtGui/QtGui>

#include "debugqtview.h"
#include "debugqtwindow.h"


//============================================================
//  The Memory Window.
//============================================================
class MemoryWindow : public WindowQt
{
    Q_OBJECT
    
public:
    MemoryWindow(running_machine* machine, QWidget* parent=NULL);
    virtual ~MemoryWindow() {}


private slots:
    void memoryRegionChanged(int index);
    void expressionSubmitted();
    void chunkChanged(QAction* changedTo);
    void addressChanged(QAction* changedTo);
    void reverseChanged(bool changedTo);
    void increaseBytesPerLine(bool changedTo);
    void decreaseBytesPerLine(bool checked=false);


private:
    void populateComboBox();


private:
    // Widgets
    QLineEdit* m_inputEdit;
    QComboBox* m_memoryComboBox;
    DebuggerView* m_memTable;
};


#endif
