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

MACHINE_CONFIG_START(pet_userport_cb2_sound_device::device_add_mconfig)
	SPEAKER(config, "speaker").front_center();
	MCFG_DEVICE_ADD("dac", DAC_1BIT, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.99)
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE(0, "dac", 1.0, DAC_VREF_POS_INPUT)
MACHINE_CONFIG_END

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
