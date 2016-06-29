// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    ui/devctrl.h

    Device specific control menu
    This source provides a base class for any device which need a specific
    submenu and which can occur multiple times in the same driver (at the
    moment, cassette tapes and barcode readers, in future possibly other like
    printers)
    The base class contains calls to get the total number of devices of
    the same kind connected to the driver, and shortcuts to switch current
    device to next one or previous one attached. This allows, for instance,
    users to pass from a device to another one by simply pressing left/right
    and the menu is rebuilt accordingly, without the need of a preliminary
    submenu listing available devices of the same kind.

***************************************************************************/

#pragma once

#ifndef MAME_FRONTEND_UI_DEVCTRL_H
#define MAME_FRONTEND_UI_DEVCTRL_H

#include "ui/menu.h"

namespace ui {
template<class DeviceType>
class menu_device_control : public menu
{
public:
	menu_device_control(mame_ui_manager &mui, render_container *container, DeviceType *device);

protected:
	DeviceType *current_device() { return m_device; }
	int count() { return m_count; }

	int current_index();
	void previous();
	void next();
	std::string current_display_name();
	UINT32 current_display_flags();

private:
	// device iterator
	typedef device_type_iterator<&device_creator<DeviceType>, DeviceType> iterator;

	DeviceType *    m_device;
	int             m_count;
};


//-------------------------------------------------
//  ctor
//-------------------------------------------------

template<class DeviceType>
menu_device_control<DeviceType>::menu_device_control(mame_ui_manager &mui, render_container *container, DeviceType *device)
	: menu(mui, container)
{
	iterator iter(mui.machine().root_device());
	m_count = iter.count();
	m_device = device ? device : iter.first();
}


//-------------------------------------------------
//  current_index
//-------------------------------------------------

template<class DeviceType>
int menu_device_control<DeviceType>::current_index()
{
	iterator iter(machine().root_device());
	return iter.indexof(*m_device);
}


//-------------------------------------------------
//  previous
//-------------------------------------------------

template<class DeviceType>
void menu_device_control<DeviceType>::previous()
{
	// left arrow - rotate left through cassette devices
	if (m_device != nullptr)
	{
		iterator iter(machine().root_device());
		int index = iter.indexof(*m_device);
		if (index > 0)
			index--;
		else
			index = m_count - 1;
		m_device = iter.byindex(index);
	}
}


//-------------------------------------------------
//  next
//-------------------------------------------------

template<class DeviceType>
void menu_device_control<DeviceType>::next()
{
	// right arrow - rotate right through cassette devices
	if (m_device != nullptr)
	{
		iterator iter(machine().root_device());
		int index = iter.indexof(*m_device);
		if (index < m_count - 1)
			index++;
		else
			index = 0;
		m_device = iter.byindex(index);
	}
}


//-------------------------------------------------
//  current_display_name
//-------------------------------------------------

template<class DeviceType>
std::string menu_device_control<DeviceType>::current_display_name()
{
	std::string display_name;
	display_name.assign(current_device()->name());
	if (count() > 1)
		display_name.append(string_format("%d", current_index() + 1));
	return display_name;
}


//-------------------------------------------------
//  current_display_flags
//-------------------------------------------------

template<class DeviceType>
UINT32 menu_device_control<DeviceType>::current_display_flags()
{
	UINT32 flags = 0;
	if (count() > 1)
	{
		if (current_index() > 0)
			flags |= FLAG_LEFT_ARROW;
		if (current_index() < count() - 1)
			flags |= FLAG_RIGHT_ARROW;
	}
	return flags;
}

} // namespace ui

#endif /* MAME_FRONTEND_UI_DEVCTRL_H */
