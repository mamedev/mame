// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**********************************************************************

    Commodore PET userport "CB2 sound" audio device emulation

    http://zimmers.net/cbmpics/cbm/PETx/petfaq.html

**********************************************************************/

#include "emu.h"
#include "cb2snd.h"

#include "sound/volt_reg.h"
#include "speaker.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PET_USERPORT_CB2_SOUND_DEVICE, pet_userport_cb2_sound_device, "petucb2", "PET Userport 'CB2 Sound' Device")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void pet_userport_cb2_sound_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.99);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pet_userport_cb2_sound_device - constructor
//-------------------------------------------------

pet_userport_cb2_sound_device::pet_userport_cb2_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PET_USERPORT_CB2_SOUND_DEVICE, tag, owner, clock),
	device_pet_user_port_interface(mconfig, *this),
	m_dac(*this, "dac")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pet_userport_cb2_sound_device::device_start()
{
}

DECLARE_WRITE_LINE_MEMBER( pet_userport_cb2_sound_device::input_m )
{
	m_dac->write(state);
}
