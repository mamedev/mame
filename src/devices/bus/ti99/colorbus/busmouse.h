// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Mouse for use with the v9938 color bus

    Used with the Geneve 9640 and 80 column cards (like the EVPC)

    Michael Zapf, 2017-03-18

*****************************************************************************/

#ifndef MAME_BUS_TI99_COLORBUS_BUSMOUSE_H
#define MAME_BUS_TI99_COLORBUS_BUSMOUSE_H

#pragma once

#include "colorbus.h"

namespace bus::ti99::colorbus {

class v9938_busmouse_device : public device_t, public device_v9938_colorbus_interface
{
public:
	v9938_busmouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	DECLARE_INPUT_CHANGED_MEMBER( mouse_button_changed );
	DECLARE_INPUT_CHANGED_MEMBER( mouse_pos_changed );

protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	ioport_constructor device_input_ports() const override;

private:
	required_ioport m_buttons, m_xaxis, m_yaxis;
	int m_last_x;
	int m_last_y;
	int m_bstate;
};

} // end namespace bus::ti99::colorbus

DECLARE_DEVICE_TYPE_NS(V9938_BUSMOUSE, bus::ti99::colorbus, v9938_busmouse_device)

#endif // MAME_BUS_TI99_COLORBUS_BUSMOUSE_H
