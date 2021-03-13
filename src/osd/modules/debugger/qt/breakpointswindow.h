// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#ifndef MAME_DEBUGGER_QT_BREAKPOINTSWINDOW_H
#define MAME_DEBUGGER_QT_BREAKPOINTSWINDOW_H

#include "debuggerview.h"
#include "windowqt.h"


//============================================================
//  The Breakpoints Window.
//============================================================
class BreakpointsWindow : public WindowQt
{
	Q_OBJECT

public:
	BreakpointsWindow(running_machine &machine, QWidget *parent = nullptr);
	virtual ~BreakpointsWindow();

private slots:
	void typeChanged(QAction *changedTo);

private:
	// Widgets
	DebuggerView *m_breakpointsView;
};


//=========================================================================
//  A way to store the configuration of a window long enough to read/write.
//=========================================================================
class BreakpointsWindowQtConfig : public WindowQtConfig
{
public:
	BreakpointsWindowQtConfig() :
		WindowQtConfig(WIN_TYPE_BREAK_POINTS),
		m_bwType(0)
	{
	}

	~BreakpointsWindowQtConfig() {}

	// Settings
	int m_bwType;

	void buildFromQWidget(QWidget *widget);
	void applyToQWidget(QWidget *widget);
	void addToXmlDataNode(util::xml::data_node &node) const;
	void recoverFromXmlNode(util::xml::data_node const &node);
};

#endif // MAME_DEBUGGER_QT_BREAKPOINTSWINDOW_H
