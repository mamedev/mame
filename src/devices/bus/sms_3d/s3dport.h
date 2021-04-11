// license:BSD-3-Clause
// copyright-holders:Enik Land
/**********************************************************************

    Input Port for the Sega 3-D Glasses / SegaScope 3-D Glasses

**********************************************************************


**********************************************************************/

#ifndef MAME_BUS_SMS_3D_PORT_H
#define MAME_BUS_SMS_3D_PORT_H

#include "screen.h"

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sms_3d_port_device

class device_sms_3d_port_interface;

class sms_3d_port_device : public device_t,
								public device_single_card_slot_interface<device_sms_3d_port_interface>
{
public:
	// construction/destruction
	template <typename T>
	sms_3d_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: sms_3d_port_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	sms_3d_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~sms_3d_port_device();

	// configuration
	void set_screen_device(screen_device &screen) { m_screen = &screen; }

	// writing
	DECLARE_WRITE_LINE_MEMBER(write_sscope);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_resolve_objects() override;

private:
	screen_device *m_screen;
	device_sms_3d_port_interface *m_device;
};


// ======================> device_sms_3d_port_interface

// class representing interface-specific live sms_expansion card
class device_sms_3d_port_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_sms_3d_port_interface();

	// configuration
	virtual void set_screen_device(screen_device &screen) { m_screen = &screen; }

	// writing
	virtual DECLARE_WRITE_LINE_MEMBER(write_sscope) {}

protected:
	device_sms_3d_port_interface(const machine_config &mconfig, device_t &device);

	screen_device *m_screen;
};


// device type definition
DECLARE_DEVICE_TYPE(SMS_3D_PORT, sms_3d_port_device)


void sms_3d_port_devices(device_slot_interface &device);


#endif // MAME_BUS_SMS_3D_PORT_H
