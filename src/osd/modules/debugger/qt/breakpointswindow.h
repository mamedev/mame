// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#ifndef MAME_DEBUGGER_QT_BREAKPOINTSWINDOW_H
#define MAME_DEBUGGER_QT_BREAKPOINTSWINDOW_H

#include "debuggerview.h"
#include "windowqt.h"


namespace osd::debugger::qt {

//============================================================
//  The Breakpoints Window.
//============================================================
class BreakpointsWindow : public WindowQt
{
	Q_OBJECT

public:
	BreakpointsWindow(running_machine &machine, QWidget *parent = nullptr);
	virtual ~BreakpointsWindow();

protected:
	virtual void saveConfigurationToNode(util::xml::data_node &node) override;

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
		WindowQtConfig(WINDOW_TYPE_POINTS_VIEWER),
		m_bwType(0)
	{
	}

	~BreakpointsWindowQtConfig() {}

	// Settings
	int m_bwType;

	void applyToQWidget(QWidget *widget);
	void recoverFromXmlNode(util::xml::data_node const &node);
};

} // namespace osd::debugger::qt

#endif // MAME_DEBUGGER_QT_BREAKPOINTSWINDOW_H
