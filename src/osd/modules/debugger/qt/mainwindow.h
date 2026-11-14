// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#ifndef MAME_DEBUGGER_QT_MAINWINDOW_H
#define MAME_DEBUGGER_QT_MAINWINDOW_H

#pragma once

#include "debuggerview.h"
#include "windowqt.h"

#include "debug/dvdisasm.h"

#include <QtWidgets/QLineEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QComboBox>

#include <deque>


namespace osd::debugger::qt {

class DasmDockWidget;
class SrcdbgDockWidget;
class ProcessorDockWidget;


//============================================================
//  The Main Window.  Contains processor and dasm docks.
//============================================================
class MainWindow : public WindowQt
{
	Q_OBJECT

public:
	MainWindow(DebuggerQt &debugger, QWidget *parent = nullptr);
	virtual ~MainWindow();

	void setProcessor(device_t *processor);

	virtual void restoreConfiguration(util::xml::data_node const &node) override;

protected:
	virtual void saveConfigurationToNode(util::xml::data_node &node) override;

	// Used to intercept the user clicking 'X' in the upper corner
	virtual void closeEvent(QCloseEvent *event) override;

	// Used to intercept the user hitting the up arrow in the input widget
	virtual bool eventFilter(QObject *obj, QEvent *event) override;

protected slots:
	// Overrides allow different behavior when source-level debugging is active
	virtual void debugActStepInto() override;
	virtual void debugActStepOver() override;
	virtual void debugActStepOut() override;

private slots:
	void toggleBreakpointAtCursor(bool changedTo);
	void enableBreakpointAtCursor(bool changedTo);
	void runToCursor(bool changedTo);
	void rightBarChanged(QAction *changedTo);
	void srcdbgBarChanged(QAction *changedTo);

	void executeCommandSlot();
	void commandEditedSlot(QString const &text);

	void mountImage(bool changedTo);
	void unmountImage(bool changedTo);

	void codeViewUpdated();

	// Closing the main window hides the debugger and runs the emulated system
	virtual void debugActClose() override;
	virtual void debuggerExit() override;

private:
	enum
	{
		MENU_SHOW_SOURCE,
		MENU_SHOW_DISASM
	};

	void createImagesMenu();

	void executeCommand(bool withClear);

	bool sourceFrameActive() const;
	bool addressFromCursor(offs_t & address) const;
	const debug_breakpoint * breakpointFromAddress(offs_t address) const;

	// Widgets and docks
	QDockWidget *m_codeDock;            // A dock that shows disassembly or source code
	QLineEdit *m_inputEdit;
	DebuggerView *m_consoleView;
	ProcessorDockWidget *m_procFrame;
	DasmDockWidget *m_dasmFrame;
	SrcdbgDockWidget *m_srcdbgFrame;

	// Menu items
	QAction *m_breakpointToggleAct;
	QAction *m_breakpointEnableAct;
	QAction *m_runToCursorAct;

	// Terminal history
	CommandHistory m_inputHistory;

	bool m_exiting;
};


//============================================================
//  Docks with the Main Window.  Disassembly.
//============================================================
class DasmDockWidget : public QWidget
{
	Q_OBJECT

public:
	DasmDockWidget(running_machine &machine, QWidget *parent = nullptr) :
		QWidget(parent),
		m_machine(machine)
	{
		m_dasmView = new DebuggerView(DVT_DISASSEMBLY, m_machine, this);

		// Force a recompute of the disassembly region
		downcast<debug_view_disasm*>(m_dasmView->view())->set_expression("curpc");

		QVBoxLayout *dvLayout = new QVBoxLayout(this);
		dvLayout->addWidget(m_dasmView);
		dvLayout->setContentsMargins(4,0,4,0);
	}

	virtual ~DasmDockWidget();

	DebuggerView *view() { return m_dasmView; }

	QSize minimumSizeHint() const { return QSize(150, 150); }
	QSize sizeHint() const { return QSize(150, 200); }

private:
	running_machine &m_machine;

	DebuggerView *m_dasmView;
};


//============================================================
//  Docks with the Main Window.  Source-level view.
//============================================================
class SrcdbgDockWidget : public QWidget
{
	Q_OBJECT

public:
	SrcdbgDockWidget(running_machine &machine, QWidget *parent = nullptr);
	virtual ~SrcdbgDockWidget();

	DebuggerView *view() { return m_srcdbgView; }

	QSize minimumSizeHint() const { return QSize(150, 150); }
	QSize sizeHint() const { return QSize(150, 200); }

	void update();

private slots:
	void srcfileChanged(int index);

private:
	void populateSrcdbgCombo();

	running_machine &m_machine;

	QComboBox *m_srcdbgCombo;
	DebuggerView *m_srcdbgView;
};


//============================================================
//  Docks with the Main Window.  Processor information.
//============================================================
class ProcessorDockWidget : public QWidget
{
	Q_OBJECT

public:
	ProcessorDockWidget(running_machine &machine, QWidget *parent = nullptr) :
		QWidget(parent),
		m_machine(machine),
		m_processorView(nullptr)
	{
		m_processorView = new DebuggerView(DVT_STATE, m_machine, this);
		m_processorView->setFocusPolicy(Qt::NoFocus);

		QVBoxLayout *cvLayout = new QVBoxLayout(this);
		cvLayout->addWidget(m_processorView);
		cvLayout->setContentsMargins(4,0,4,2);
	}

	virtual ~ProcessorDockWidget();

	DebuggerView *view() { return m_processorView; }

	QSize minimumSizeHint() const { return QSize(150, 300); }
	QSize sizeHint() const { return QSize(200, 300); }

private:
	running_machine &m_machine;

	DebuggerView *m_processorView;
};

} // namespace osd::debugger::qt

#endif // MAME_DEBUGGER_QT_MAINWINDOW_H
