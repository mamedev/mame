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

#define TIGEN_V9938_TAG   "vdp"
#define COLORBUS_TAG     "colorbus"

namespace bus::ti99::colorbus {

class v9938_colorbus_device;

/********************************************************************
    Common parent class of all devices attached to the color bus
********************************************************************/
class device_v9938_colorbus_interface : public device_interface
{
protected:
	device_v9938_colorbus_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_config_complete() override;

	v9938_colorbus_device* m_colorbus;
};

/********************************************************************
    Color bus port
********************************************************************/
class v9938_colorbus_device : public device_t, public device_single_card_slot_interface<device_v9938_colorbus_interface>
{
public:
	template <typename U>
	v9938_colorbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, U &&opts, const char *dflt)
		: v9938_colorbus_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	v9938_colorbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// For the extra button (V9938 only handles 2)
	auto extra_button_cb() { return m_extra_button.bind(); }

	// Called from the device
	void movex(int delta);
	void movey(int delta);
	void buttons(int bstate);

protected:
	void device_start() override ATTR_COLD;

private:
	required_device<v9938_device> m_v9938;
	devcb_write_line   m_extra_button;
};

} // end namespace bus::ti99::colorbus

DECLARE_DEVICE_TYPE_NS(V9938_COLORBUS, bus::ti99::colorbus, v9938_colorbus_device)

void ti99_colorbus_options(device_slot_interface &device);

#endif // MAME_BUS_TI99_COLORBUS_COLORBUS_H
