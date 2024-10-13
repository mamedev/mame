// license:BSD-3-Clause
// copyright-holders: Ken White
/**********************************************************************

    Commodore PET user port dual joystick and CB2 sound emulation

**********************************************************************/

#ifndef MAME_BUS_PET_2JOYSND_H
#define MAME_BUS_PET_2JOYSND_H

#pragma once


#include "user.h"
#include "sound/dac.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pet_userport_joystick_and_sound_device

class pet_userport_joystick_and_sound_device : public device_t, public device_pet_user_port_interface
{
public:
	// construction/destruction
	pet_userport_joystick_and_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void input_m(int state) override;

	// device_pet_user_port_interface overrides
	void write_up1(int state) { m_up1 = state; update_port1(); }
	void write_down1(int state) { m_down1 = state; update_port1(); }
	void write_fire1(int state) { m_fire1 = state; update_port1(); }
	void write_up2(int state) { m_up2 = state; update_port2(); }
	void write_down2(int state) { m_down2 = state; update_port2(); }
	void write_fire2(int state) { m_fire2 = state; update_port2(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void update_port1();
	void update_port2();

	required_device<dac_bit_interface> m_dac;

	int m_up1;
	int m_down1;
	int m_fire1;
	int m_up2;
	int m_down2;
	int m_fire2;
};


// device type definition
DECLARE_DEVICE_TYPE(PET_USERPORT_JOYSTICK_AND_SOUND_DEVICE, pet_userport_joystick_and_sound_device)


#endif // MAME_BUS_PET_2JOYSND_H
