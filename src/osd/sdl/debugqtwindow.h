#ifndef __DEBUG_QT_WINDOW_H__
#define __DEBUG_QT_WINDOW_H__

#include <QtGui/QtGui>

#include "emu.h"


//============================================================
//  The Qt window that everyone derives from.
//============================================================
class WindowQt : public QMainWindow
{
	Q_OBJECT

public:
	WindowQt(running_machine* machine, QWidget* parent=NULL);
	virtual ~WindowQt() {}

	// The interface to an all-window refresh
	void refreshAll() { s_refreshAll = true; }
	bool wantsRefresh() { return s_refreshAll; }
	void clearRefreshFlag() { s_refreshAll = false; }


protected slots:
	void debugActOpenMemory();
	void debugActOpenDasm();
	void debugActOpenLog();
	void debugActRun();
	void debugActRunAndHide();
	void debugActRunToNextCpu();
	void debugActRunNextInt();
	void debugActRunNextVBlank();
	void debugActStepInto();
	void debugActStepOver();
	void debugActStepOut();
	void debugActSoftReset();
	void debugActHardReset();
	void debugActClose();
	void debugActQuit();


protected:
	running_machine* m_machine;

	static bool s_refreshAll;
};


#endif
