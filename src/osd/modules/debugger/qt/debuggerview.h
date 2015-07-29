// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#ifndef __DEBUG_QT_DEBUGGER_VIEW_H__
#define __DEBUG_QT_DEBUGGER_VIEW_H__

#include <QtWidgets/QAbstractScrollArea>

#include "debug/debugvw.h"


class DebuggerView : public QAbstractScrollArea
{
	Q_OBJECT

public:
	DebuggerView(const debug_view_type& type,
					running_machine* machine,
					QWidget* parent=NULL);
	virtual ~DebuggerView();

	void paintEvent(QPaintEvent* event);

	// Setters and accessors
	void setPreferBottom(bool pb) { m_preferBottom = pb; }
	debug_view* view() { return m_view; }

signals:
	void updated();

protected:
	void keyPressEvent(QKeyEvent* event);
	void mousePressEvent(QMouseEvent* event);

private slots:
	void verticalScrollSlot(int value);
	void horizontalScrollSlot(int value);


private:
	// Callback to allow MAME to refresh the view
	static void debuggerViewUpdate(debug_view& debugView, void* osdPrivate);

	bool m_preferBottom;

	debug_view* m_view;
	running_machine* m_machine;
};


#endif
