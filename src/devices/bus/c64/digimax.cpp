// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    DigiMax 8-Bit Stereo Sound Cart emulation

**********************************************************************/

#include "emu.h"
#include "digimax.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_DIGIMAX, c64_digimax_device, "c64_digimax", "DigiMAX")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c64_digimax_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "lspeaker").front_left();
	DAC_8BIT_R2R(config, m_dac[1], 0).add_route(ALL_OUTPUTS, "lspeaker", 0.25);
	DAC_8BIT_R2R(config, m_dac[3], 0).add_route(ALL_OUTPUTS, "lspeaker", 0.25);

	SPEAKER(config, "rspeaker").front_right();
	DAC_8BIT_R2R(config, m_dac[0], 0).add_route(ALL_OUTPUTS, "rspeaker", 0.25);
	DAC_8BIT_R2R(config, m_dac[2], 0).add_route(ALL_OUTPUTS, "rspeaker", 0.25);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_digimax_device - constructor
//-------------------------------------------------

c64_digimax_device::c64_digimax_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_DIGIMAX, tag, owner, clock),
	device_pet_user_port_interface(mconfig, *this),
	m_dac(*this, "%u", 0U)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_digimax_device::device_start()
{
	save_item(NAME(m_data));
	save_item(NAME(m_a));
}
