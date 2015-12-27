// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#ifndef __DEBUG_QT_DASM_WINDOW_H__
#define __DEBUG_QT_DASM_WINDOW_H__

#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>

#include "debuggerview.h"
#include "windowqt.h"


//============================================================
//  The Disassembly Window.
//============================================================
class DasmWindow : public WindowQt
{
	Q_OBJECT

public:
	DasmWindow(running_machine* machine, QWidget* parent=NULL);
	virtual ~DasmWindow();


private slots:
	void cpuChanged(int index);
	void expressionSubmitted();

	void toggleBreakpointAtCursor(bool changedTo);
	void enableBreakpointAtCursor(bool changedTo);
	void runToCursor(bool changedTo);
	void rightBarChanged(QAction* changedTo);

	void dasmViewUpdated();


private:
	void populateComboBox();


	// Widgets
	QLineEdit* m_inputEdit;
	QComboBox* m_cpuComboBox;
	DebuggerView* m_dasmView;

	// Menu items
	QAction* m_breakpointToggleAct;
	QAction* m_breakpointEnableAct;
	QAction* m_runToCursorAct;
};


//=========================================================================
//  A way to store the configuration of a window long enough to read/write.
//=========================================================================
class DasmWindowQtConfig : public WindowQtConfig
{
public:
	DasmWindowQtConfig() :
		WindowQtConfig(WIN_TYPE_DASM),
		m_cpu(0),
		m_rightBar(0)
	{
	}

	~DasmWindowQtConfig() {}

	// Settings
	int m_cpu;
	int m_rightBar;

	void buildFromQWidget(QWidget* widget);
	void applyToQWidget(QWidget* widget);
	void addToXmlDataNode(xml_data_node* node) const;
	void recoverFromXmlNode(xml_data_node* node);
};


#endif
