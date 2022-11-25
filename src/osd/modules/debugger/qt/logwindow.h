// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#ifndef MAME_DEBUGGER_QT_LOGWINDOW_H
#define MAME_DEBUGGER_QT_LOGWINDOW_H

#pragma once

#include "debuggerview.h"
#include "windowqt.h"


namespace osd::debugger::qt {

//============================================================
//  The Log Window.
//============================================================
class LogWindow : public WindowQt
{
	Q_OBJECT

public:
	LogWindow(running_machine &machine, QWidget *parent = nullptr);
	virtual ~LogWindow();

protected:
	virtual void saveConfigurationToNode(util::xml::data_node &node) override;

private:
	// Widgets
	DebuggerView *m_logView;
};


//=========================================================================
//  A way to store the configuration of a window long enough to read/write.
//=========================================================================
class LogWindowQtConfig : public WindowQtConfig
{
public:
	LogWindowQtConfig() :
		WindowQtConfig(WINDOW_TYPE_ERROR_LOG_VIEWER)
	{
	}

	~LogWindowQtConfig() {}

	void applyToQWidget(QWidget *widget);
	void recoverFromXmlNode(util::xml::data_node const &node);
};

} // namespace osd::debugger::qt

#endif // MAME_DEBUGGER_QT_LOGWINDOW_H
