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

	void hideAll() { s_hideAll = true; }
	bool wantsHide() { return s_hideAll; }
	void clearHideFlag() { s_hideAll = false; }


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
	virtual void debugActClose();
	void debugActQuit();


protected:
	running_machine* m_machine;

	static bool s_refreshAll;
	static bool s_hideAll;
};


//=======================================================================
//  A way to store the configuration of a window long enough to use it.
//=======================================================================
class WindowQtConfig
{
public:
	// This is a holdover from the old debugger - TODO: remove
	enum WindowType
	{
		WIN_TYPE_MAIN       = 0x01,
		WIN_TYPE_MEMORY     = 0x02,
		WIN_TYPE_DISASM     = 0x04,
		WIN_TYPE_LOG        = 0x08,
		WIN_TYPE_UNKNOWN    = 0x10,
	};

public:
	WindowQtConfig() : 
		m_type(WIN_TYPE_MAIN), 
		m_size(800, 600),
		m_position(120, 120)
	{}
	~WindowQtConfig() {}

	WindowType m_type;
	QPoint m_size;
	QPoint m_position;
};

#endif
