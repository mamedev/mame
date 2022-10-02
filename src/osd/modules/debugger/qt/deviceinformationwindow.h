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
	DeviceInformationWindow(running_machine &machine, device_t *device = nullptr, QWidget* parent=nullptr);
	virtual ~DeviceInformationWindow();

	void set_device(const char *tag);

protected:
	virtual void saveConfigurationToNode(util::xml::data_node &node) override;

private:
	device_t *m_device;

	void fill_device_information();
};




//=========================================================================
//  A way to store the configuration of a window long enough to read/write.
//=========================================================================
class DeviceInformationWindowQtConfig : public WindowQtConfig
{
public:
	std::string m_device_tag;

	DeviceInformationWindowQtConfig() :
		WindowQtConfig(WINDOW_TYPE_DEVICE_INFO_VIEWER)
	{
	}

	~DeviceInformationWindowQtConfig() {}

	void applyToQWidget(QWidget *widget);
	void recoverFromXmlNode(util::xml::data_node const &node);
};

} // namespace osd::debugger::qt

#endif // MAME_DEBUGGER_QT_DEVICEINFORMATIONWINDOW_H
