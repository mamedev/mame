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

#ifndef __UI_DEVCTRL_H__
#define __UI_DEVCTRL_H__

template<class _DeviceType>
class ui_menu_device_control : public ui_menu
{
public:
	ui_menu_device_control(running_machine &machine, render_container *container, _DeviceType *device);

protected:
	_DeviceType *current_device() { return m_device; }
	int count() { return m_count; }

	int current_index();
	void previous();
	void next();
	std::string current_display_name();
	UINT32 current_display_flags();

private:
	// device iterator
	typedef device_type_iterator<&device_creator<_DeviceType>, _DeviceType> iterator;

	_DeviceType *   m_device;
	int             m_count;
};


//-------------------------------------------------
//  ctor
//-------------------------------------------------

template<class _DeviceType>
ui_menu_device_control<_DeviceType>::ui_menu_device_control(running_machine &machine, render_container *container, _DeviceType *device)
	: ui_menu(machine, container)
{
	iterator iter(machine.root_device());
	m_count = iter.count();
	m_device = device ? device : iter.first();
}


//-------------------------------------------------
//  current_index
//-------------------------------------------------

template<class _DeviceType>
int ui_menu_device_control<_DeviceType>::current_index()
{
	iterator iter(machine().root_device());
	return iter.indexof(*m_device);
}


//-------------------------------------------------
//  previous
//-------------------------------------------------

template<class _DeviceType>
void ui_menu_device_control<_DeviceType>::previous()
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

template<class _DeviceType>
void ui_menu_device_control<_DeviceType>::next()
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

template<class _DeviceType>
std::string ui_menu_device_control<_DeviceType>::current_display_name()
{
	std::string display_name;
	display_name.assign(current_device()->name());
	if (count() > 1)
	{
		std::string temp;
		strprintf(temp, " %d", current_index() + 1);
		display_name.append(temp);
	}
	return display_name;
}


//-------------------------------------------------
//  current_display_flags
//-------------------------------------------------

template<class _DeviceType>
UINT32 ui_menu_device_control<_DeviceType>::current_display_flags()
{
	UINT32 flags = 0;
	if (count() > 1)
	{
		if (current_index() > 0)
			flags |= MENU_FLAG_LEFT_ARROW;
		if (current_index() < count() - 1)
			flags |= MENU_FLAG_RIGHT_ARROW;
	}
	return flags;
}


#endif /* __UI_DEVCTRL_H__ */
