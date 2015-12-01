// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#ifndef __DEBUG_QT_MEMORY_WINDOW_H__
#define __DEBUG_QT_MEMORY_WINDOW_H__

#include <QtGui/QtGui>

#include "debuggerview.h"
#include "windowqt.h"

class DebuggerMemView;


//============================================================
//  The Memory Window.
//============================================================
class MemoryWindow : public WindowQt
{
	Q_OBJECT

public:
	MemoryWindow(running_machine* machine, QWidget* parent=NULL);
	virtual ~MemoryWindow();


private slots:
	void memoryRegionChanged(int index);
	void expressionSubmitted();
	void formatChanged(QAction* changedTo);
	void addressChanged(QAction* changedTo);
	void reverseChanged(bool changedTo);
	void increaseBytesPerLine(bool changedTo);
	void decreaseBytesPerLine(bool checked=false);


private:
	void populateComboBox();
	void setToCurrentCpu();
	QAction* dataFormatMenuItem(const QString& itemName);


private:
	// Widgets
	QLineEdit* m_inputEdit;
	QComboBox* m_memoryComboBox;
	DebuggerMemView* m_memTable;
};


//=========================================================================
//  The mem window gets its own debugger view to handle right click pop-ups
//=========================================================================
class DebuggerMemView : public DebuggerView
{
public:
	DebuggerMemView(const debug_view_type& type,
					running_machine* machine,
					QWidget* parent=NULL)
		: DebuggerView(type, machine, parent)
	{}
	virtual ~DebuggerMemView() {}

protected:
	void mousePressEvent(QMouseEvent* event);
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
		m_dataFormat(0),
		m_memoryRegion(0)
	{
	}

	~MemoryWindowQtConfig() {}

	// Settings
	int m_reverse;
	int m_addressMode;
	int m_dataFormat;
	int m_memoryRegion;

	void buildFromQWidget(QWidget* widget);
	void applyToQWidget(QWidget* widget);
	void addToXmlDataNode(xml_data_node* node) const;
	void recoverFromXmlNode(xml_data_node* node);
};


#endif
