#ifndef __DEBUG_QT_WINDOW_H__
#define __DEBUG_QT_WINDOW_H__

#include <QtGui/QtGui>

#include "emu.h"
#include "config.h"
#include "debugger.h"


//============================================================
//  The Qt window that everyone derives from.
//============================================================
class WindowQt : public QMainWindow
{
	Q_OBJECT

public:
	WindowQt(running_machine* machine, QWidget* parent=NULL);
	virtual ~WindowQt();

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
	void debugActOpenPoints();
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


//=========================================================================
//  A way to store the configuration of a window long enough to read/write.
//=========================================================================
class WindowQtConfig
{
public:
	enum WindowType
	{
		WIN_TYPE_MAIN         = 0x01,
		WIN_TYPE_MEMORY       = 0x02,
		WIN_TYPE_DASM         = 0x04,
		WIN_TYPE_LOG          = 0x08,
		WIN_TYPE_BREAK_POINTS = 0x10,
		WIN_TYPE_UNKNOWN      = 0x20,
	};

public:
	WindowQtConfig(const WindowType& type=WIN_TYPE_UNKNOWN) :
		m_type(type),
		m_size(800, 600),
		m_position(120, 120)
	{}
	virtual ~WindowQtConfig() {}

	// Settings
	WindowType m_type;
	QPoint m_size;
	QPoint m_position;

	virtual void buildFromQWidget(QWidget* widget);
	virtual void applyToQWidget(QWidget* widget);
	virtual void addToXmlDataNode(xml_data_node* node) const;
	virtual void recoverFromXmlNode(xml_data_node* node);
};


#endif
