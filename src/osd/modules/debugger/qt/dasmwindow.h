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
	DasmWindow(running_machine &machine, QWidget *parent = nullptr);
	virtual ~DasmWindow();

protected:
	virtual void saveConfigurationToNode(util::xml::data_node &node) override;

private slots:
	void cpuChanged(int index);
	void expressionSubmitted();

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
};


//=========================================================================
//  A way to store the configuration of a window long enough to read/write.
//=========================================================================
class DasmWindowQtConfig : public WindowQtConfig
{
public:
	DasmWindowQtConfig() :
		WindowQtConfig(WINDOW_TYPE_DISASSEMBLY_VIEWER),
		m_cpu(0),
		m_rightBar(0)
	{
	}

	~DasmWindowQtConfig() {}

	// Settings
	int m_cpu;
	int m_rightBar;

	void applyToQWidget(QWidget *widget);
	void recoverFromXmlNode(util::xml::data_node const &node);
};

} // namespace osd::debugger::qt

#endif // MAME_DEBUGGER_QT_DASMWINDOW_H
