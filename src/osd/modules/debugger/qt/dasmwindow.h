// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#ifndef MAME_DEBUGGER_QT_DASMWINDOW_H
#define MAME_DEBUGGER_QT_DASMWINDOW_H

#pragma once

#include "debuggerview.h"
#include "windowqt.h"

#include <QtWidgets/QComboBox>
#include <QtWidgets/QLineEdit>


namespace osd::debugger::qt {

//============================================================
//  The Disassembly Window.
//============================================================
class DasmWindow : public WindowQt
{
	Q_OBJECT

public:
	DasmWindow(DebuggerQt &debugger, QWidget *parent = nullptr);
	virtual ~DasmWindow();

	virtual void restoreConfiguration(util::xml::data_node const &node) override;

protected:
	virtual void saveConfigurationToNode(util::xml::data_node &node) override;

	// Used to intercept the user hitting the up arrow in the input widget
	virtual bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
	void cpuChanged(int index);
	void expressionSubmitted();
	void expressionEdited(QString const &text);

	void toggleBreakpointAtCursor(bool changedTo);
	void enableBreakpointAtCursor(bool changedTo);
	void runToCursor(bool changedTo);
	void rightBarChanged(QAction *changedTo);

	void dasmViewUpdated();

private:
	void populateComboBox();
	void setToCurrentCpu();

	// Widgets
	QLineEdit *m_inputEdit;
	QComboBox *m_cpuComboBox;
	DebuggerView *m_dasmView;

	// Menu items
	QAction *m_breakpointToggleAct;
	QAction *m_breakpointEnableAct;
	QAction *m_runToCursorAct;

	// Expression history
	CommandHistory m_inputHistory;
};

} // namespace osd::debugger::qt

#endif // MAME_DEBUGGER_QT_DASMWINDOW_H
