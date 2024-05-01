// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#ifndef MAME_DEBUGGER_QT_MEMORYWINDOW_H
#define MAME_DEBUGGER_QT_MEMORYWINDOW_H

#pragma once

#include "debuggerview.h"
#include "windowqt.h"

#include <QtWidgets/QComboBox>
#include <QtWidgets/QLineEdit>


namespace osd::debugger::qt {

class DebuggerMemView;


//============================================================
//  The Memory Window.
//============================================================
class MemoryWindow : public WindowQt
{
	Q_OBJECT

public:
	MemoryWindow(DebuggerQt &debugger, QWidget *parent = nullptr);
	virtual ~MemoryWindow();

	virtual void restoreConfiguration(util::xml::data_node const &node) override;

protected:
	virtual void saveConfigurationToNode(util::xml::data_node &node) override;

	// Used to intercept the user hitting the up arrow in the input widget
	virtual bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
	void memoryRegionChanged(int index);
	void expressionSubmitted();
	void expressionEdited(QString const &text);

	void formatChanged(QAction *changedTo);
	void addressChanged(QAction *changedTo);
	void radixChanged(QAction *changedTo);
	void reverseChanged(bool changedTo);
	void increaseBytesPerLine(bool changedTo);
	void decreaseBytesPerLine(bool checked = false);

private:
	void populateComboBox();
	void setToCurrentCpu();

	// Widgets
	QLineEdit *m_inputEdit;
	QComboBox *m_memoryComboBox;
	DebuggerMemView *m_memTable;

	// Expression history
	CommandHistory m_inputHistory;
};


//=========================================================================
//  The mem window gets its own debugger view to handle right click pop-ups
//=========================================================================
class DebuggerMemView : public DebuggerView
{
	Q_OBJECT

public:
	DebuggerMemView(const debug_view_type& type, running_machine &machine, QWidget *parent = nullptr)
		: DebuggerView(type, machine, parent)
	{}

	virtual ~DebuggerMemView() {}

protected:
	virtual void addItemsToContextMenu(QMenu *menu) override;

private slots:
	void copyLastPc();

private:
	QString m_lastPc;
};

} // namespace osd::debugger::qt

#endif // MAME_DEBUGGER_QT_MEMORYWINDOW_H
