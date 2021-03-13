// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#ifndef MAME_DEBUGGER_QT_LOGWINDOW_H
#define MAME_DEBUGGER_QT_LOGWINDOW_H

#include "debuggerview.h"
#include "windowqt.h"


//============================================================
//  The Log Window.
//============================================================
class LogWindow : public WindowQt
{
	Q_OBJECT

public:
	LogWindow(running_machine &machine, QWidget *parent = nullptr);
	virtual ~LogWindow();

private:
	// Widgets
	DebuggerView *m_logView;
};


//=========================================================================
//  A way to store the configuration of a window long enough to read/write.
//=========================================================================
class LogWindowQtConfig : public WindowQtConfig
{
public:
	LogWindowQtConfig() :
		WindowQtConfig(WIN_TYPE_LOG)
	{
	}

	~LogWindowQtConfig() {}

	void buildFromQWidget(QWidget *widget);
	void applyToQWidget(QWidget *widget);
	void addToXmlDataNode(util::xml::data_node &node) const;
	void recoverFromXmlNode(util::xml::data_node const &node);
};


#endif // MAME_DEBUGGER_QT_LOGWINDOW_H
