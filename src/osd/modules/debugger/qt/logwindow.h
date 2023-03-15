// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#ifndef MAME_DEBUGGER_QT_LOGWINDOW_H
#define MAME_DEBUGGER_QT_LOGWINDOW_H

#pragma once

#include "debuggerview.h"
#include "windowqt.h"


namespace osd::debugger::qt {

//============================================================
//  The Log Window.
//============================================================
class LogWindow : public WindowQt
{
	Q_OBJECT

public:
	LogWindow(DebuggerQt &debugger, QWidget *parent = nullptr);
	virtual ~LogWindow();

	virtual void restoreConfiguration(util::xml::data_node const &node) override;

protected:
	virtual void saveConfigurationToNode(util::xml::data_node &node) override;

private:
	// Widgets
	DebuggerView *m_logView;
};

} // namespace osd::debugger::qt

#endif // MAME_DEBUGGER_QT_LOGWINDOW_H
