// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#ifndef MAME_DEBUGGER_QT_DEBUGGERVIEW_H
#define MAME_DEBUGGER_QT_DEBUGGERVIEW_H

#pragma once

#include "debug/debugvw.h"

#include <QtWidgets/QAbstractScrollArea>
#include <QtWidgets/QMenu>


namespace osd::debugger::qt {

class DebuggerView : public QAbstractScrollArea
{
	Q_OBJECT

public:
	DebuggerView(debug_view_type type, running_machine &machine, QWidget *parent = nullptr);
	virtual ~DebuggerView();

	virtual void paintEvent(QPaintEvent *event) override;

	// Setters and accessors
	void setPreferBottom(bool pb) { m_preferBottom = pb; }
	debug_view *view() { return m_view; }
	template <typename T> T *view() { return downcast<T *>(m_view); }
	int sourceIndex() const;

	virtual void restoreConfigurationFromNode(util::xml::data_node const &node);
	virtual void saveConfigurationToNode(util::xml::data_node &node);

signals:
	void updated();

protected:
	virtual void keyPressEvent(QKeyEvent *event) override;
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void contextMenuEvent(QContextMenuEvent *event) override;

	virtual void addItemsToContextMenu(QMenu *menu);

private slots:
	void verticalScrollSlot(int value);
	void horizontalScrollSlot(int value);
	void copyVisibleSlot();
	void pasteSlot();

private:
	// Callback to allow MAME to refresh the view
	static void debuggerViewUpdate(debug_view &debugView, void *osdPrivate);

	// Helper to update scrollbar from data in view
	void updateScrollRangesAndValues();

	running_machine &m_machine;
	debug_view *m_view;

	bool m_preferBottom;
};

} // namespace osd::debugger::qt

#endif // MAME_DEBUGGER_QT_DEBUGGERVIEW_H
