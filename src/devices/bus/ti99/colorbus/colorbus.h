// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    v9938 color bus

    Used with the Geneve 9640 and 80 column cards (like the EVPC)
    for the TI-99/4A

    Michael Zapf, 2017-03-18

*****************************************************************************/

#ifndef MAME_BUS_TI99_COLORBUS_COLORBUS_H
#define MAME_BUS_TI99_COLORBUS_COLORBUS_H

#pragma once

#include "video/v9938.h"

namespace bus { namespace ti99 { namespace colorbus {

class ti99_colorbus_device;

/********************************************************************
    Common parent class of all devices attached to the color bus
********************************************************************/
class device_ti99_colorbus_interface : public device_slot_card_interface
{
public:
	virtual void poll(int& delta_x, int& delta_y, int& buttons) = 0;

protected:
	using device_slot_card_interface::device_slot_card_interface;

	virtual void interface_config_complete() override;
	ti99_colorbus_device* m_colorbus = nullptr;
};

/********************************************************************
    Color bus port
********************************************************************/
class ti99_colorbus_device : public device_t, public device_slot_interface
{
public:
	ti99_colorbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	line_state left_button();  // left button is not connected to the V9938 but to a TMS9901 pin
	void poll();

protected:
	void device_start() override { }
	void device_config_complete() override;

private:
	device_ti99_colorbus_interface* m_connected;
	required_device<v9938_device> m_v9938;
	bool m_left_button_pressed;
};

} } } // end namespace bus::ti99::colorbus

void ti99_colorbus_port(device_slot_interface &device);

#define MCFG_COLORBUS_MOUSE_ADD( _tag )  \
	MCFG_DEVICE_ADD(_tag, TI99_COLORBUS, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(ti99_colorbus_port, "busmouse", false)

DECLARE_DEVICE_TYPE_NS(TI99_COLORBUS, bus::ti99::colorbus, ti99_colorbus_device)

#endif // MAME_BUS_TI99_COLORBUS_COLORBUS_H
