// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#ifndef MAME_DEBUGGER_QT_MEMORYWINDOW_H
#define MAME_DEBUGGER_QT_MEMORYWINDOW_H

#include "debuggerview.h"
#include "windowqt.h"

#include <QtWidgets/QComboBox>
#include <QtWidgets/QLineEdit>

class DebuggerMemView;


//============================================================
//  The Memory Window.
//============================================================
class MemoryWindow : public WindowQt
{
	Q_OBJECT

public:
	MemoryWindow(running_machine &machine, QWidget *parent = nullptr);
	virtual ~MemoryWindow();

private slots:
	void memoryRegionChanged(int index);
	void expressionSubmitted();
	void formatChanged(QAction *changedTo);
	void addressChanged(QAction *changedTo);
	void radixChanged(QAction *changedTo);
	void reverseChanged(bool changedTo);
	void increaseBytesPerLine(bool changedTo);
	void decreaseBytesPerLine(bool checked = false);

private:
	void populateComboBox();
	void setToCurrentCpu();
	QAction *dataFormatMenuItem(const QString &itemName);

	// Widgets
	QLineEdit *m_inputEdit;
	QComboBox *m_memoryComboBox;
	DebuggerMemView *m_memTable;
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


//=========================================================================
//  A way to store the configuration of a window long enough to read/write.
//=========================================================================
class MemoryWindowQtConfig : public WindowQtConfig
{
public:
	MemoryWindowQtConfig() :
		WindowQtConfig(WIN_TYPE_MEMORY),
		m_reverse(0),
		m_addressMode(0),
		m_addressRadix(0),
		m_dataFormat(0),
		m_memoryRegion(0)
	{
	}

	~MemoryWindowQtConfig() {}

	// Settings
	int m_reverse;
	int m_addressMode;
	int m_addressRadix;
	int m_dataFormat;
	int m_memoryRegion;

	void buildFromQWidget(QWidget *widget);
	void applyToQWidget(QWidget *widget);
	void addToXmlDataNode(util::xml::data_node &node) const;
	void recoverFromXmlNode(util::xml::data_node const &node);
};


#endif // MAME_DEBUGGER_QT_MEMORYWINDOW_H
