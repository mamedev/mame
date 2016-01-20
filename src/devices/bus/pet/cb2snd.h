// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**********************************************************************

    Commodore PET userport "CB2 sound" audio device emulation

**********************************************************************/

#pragma once

#ifndef __PETUSER_CB2__
#define __PETUSER_CB2__

#include "emu.h"
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
	pet_userport_cb2_sound_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual DECLARE_WRITE_LINE_MEMBER( input_m ) override;

	required_device<dac_device> m_dac;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// device type definition
extern const device_type PET_USERPORT_CB2_SOUND_DEVICE;

#endif
