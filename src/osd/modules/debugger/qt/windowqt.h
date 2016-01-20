// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#ifndef __DEBUG_QT_WINDOW_QT_H__
#define __DEBUG_QT_WINDOW_QT_H__

#include <QtWidgets/QMainWindow>

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
	void debugActOpenDevices();
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
		WIN_TYPE_UNKNOWN,
		WIN_TYPE_MAIN,
		WIN_TYPE_MEMORY,
		WIN_TYPE_DASM,
		WIN_TYPE_LOG,
		WIN_TYPE_BREAK_POINTS,
		WIN_TYPE_DEVICES,
		WIN_TYPE_DEVICE_INFORMATION
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
