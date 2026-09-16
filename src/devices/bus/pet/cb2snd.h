// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**********************************************************************

    Commodore PET userport "CB2 sound" audio device emulation

**********************************************************************/

#ifndef MAME_BUS_PET_CB2SND_H
#define MAME_BUS_PET_CB2SND_H

#pragma once

#include "user.h"
#include "sound/dac.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class pet_userport_cb2_sound_device : public device_t,
	public device_pet_user_port_interface
{
public:
	// construction/destruction
	pet_userport_cb2_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void input_m(int state) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<dac_bit_interface> m_dac;
};


// device type definition
DECLARE_DEVICE_TYPE(PET_USERPORT_CB2_SOUND_DEVICE, pet_userport_cb2_sound_device)

#endif // MAME_BUS_PET_CB2SND_H
