// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#ifndef MAME_DEBUGGER_QT_DEVICEINFORMATIONWINDOW_H
#define MAME_DEBUGGER_QT_DEVICEINFORMATIONWINDOW_H

#pragma once

#include "windowqt.h"


namespace osd::debugger::qt {

//============================================================
//  The Device Information Window.
//============================================================
class DeviceInformationWindow : public WindowQt
{
	Q_OBJECT

public:
	DeviceInformationWindow(DebuggerQt &debugger, device_t *device = nullptr, QWidget* parent=nullptr);
	virtual ~DeviceInformationWindow();

	void set_device(const char *tag);

	virtual void restoreConfiguration(util::xml::data_node const &node) override;

protected:
	virtual void saveConfigurationToNode(util::xml::data_node &node) override;

private:
	device_t *m_device;

	void fill_device_information();
};

} // namespace osd::debugger::qt

#endif // MAME_DEBUGGER_QT_DEVICEINFORMATIONWINDOW_H
