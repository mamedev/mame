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

namespace bus { namespace ti99 { namespace colorbus {

class geneve_busmouse_device : public device_t, public device_ti99_colorbus_interface
{
public:
	geneve_busmouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void poll(int& delta_x, int& delta_y, int& buttons) override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual ioport_constructor device_input_ports() const override;

private:
	required_ioport m_buttons, m_xaxis, m_yaxis;
	int             m_last_mx;
	int             m_last_my;
};
} } } // end namespace bus::ti99::colorbus

DECLARE_DEVICE_TYPE_NS(TI99_BUSMOUSE, bus::ti99::colorbus, geneve_busmouse_device)

#endif // MAME_BUS_TI99_COLORBUS_BUSMOUSE_H
