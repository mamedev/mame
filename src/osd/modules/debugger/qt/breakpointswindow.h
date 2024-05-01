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
	BreakpointsWindow(DebuggerQt &debugger, QWidget *parent = nullptr);
	virtual ~BreakpointsWindow();

	virtual void restoreConfiguration(util::xml::data_node const &node) override;

protected:
	virtual void saveConfigurationToNode(util::xml::data_node &node) override;

private slots:
	void typeChanged(QAction *changedTo);

private:
	// Widgets
	DebuggerView *m_breakpointsView;
};

} // namespace osd::debugger::qt

#endif // MAME_DEBUGGER_QT_BREAKPOINTSWINDOW_H
