// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#ifndef MAME_DEBUGGER_QT_DEBUGGERVIEW_H
#define MAME_DEBUGGER_QT_DEBUGGERVIEW_H

#include "debug/debugvw.h"

#include <QtWidgets/QAbstractScrollArea>
#include <QtWidgets/QMenu>


class DebuggerView : public QAbstractScrollArea
{
	Q_OBJECT

public:
	DebuggerView(debug_view_type type, running_machine &machine, QWidget *parent = nullptr);
	virtual ~DebuggerView();

	void paintEvent(QPaintEvent *event);

	// Setters and accessors
	void setPreferBottom(bool pb) { m_preferBottom = pb; }
	debug_view *view() { return m_view; }

signals:
	void updated();

protected:
	void keyPressEvent(QKeyEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void contextMenuEvent(QContextMenuEvent *event) override;

	virtual void addItemsToContextMenu(QMenu *menu);

private slots:
	void verticalScrollSlot(int value);
	void horizontalScrollSlot(int value);
	void copyVisibleSlot();
	void pasteSlot();

private:
	// Callback to allow MAME to refresh the view
	static void debuggerViewUpdate(debug_view &debugView, void *osdPrivate);

	running_machine &m_machine;
	debug_view *m_view;

	bool m_preferBottom;
};

#endif // MAME_DEBUGGER_QT_DEBUGGERVIEW_H
