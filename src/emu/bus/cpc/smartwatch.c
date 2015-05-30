// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
    Dobbertin Smartwatch

    Created: 23/2/2015

    TODO: setting the time (requires the DS1315 core to be able to do this,
          at the moment it just reads the current time)
*/

#include "emu.h"
#include "smartwatch.h"
#include "includes/amstrad.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CPC_SMARTWATCH = &device_creator<cpc_smartwatch_device>;


static MACHINE_CONFIG_FRAGMENT( cpc_smartwatch )
	MCFG_DS1315_ADD("rtc")
	// no pass-through (?)
MACHINE_CONFIG_END

machine_config_constructor cpc_smartwatch_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cpc_smartwatch );
}

ROM_START( cpc_smartwatch )
	ROM_REGION( 0x4000, "exp_rom", 0 )
	ROM_LOAD( "timerom+.rom",   0x0000, 0x4000, CRC(ed42a147) SHA1(61750d0535a1fbf2a4addad9def332cbcf8917c3) )
ROM_END

const rom_entry *cpc_smartwatch_device::device_rom_region() const
{
	return ROM_NAME( cpc_smartwatch );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

cpc_smartwatch_device::cpc_smartwatch_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, CPC_SMARTWATCH, "Dobbertin Smartwatch", tag, owner, clock, "cpc_smartwatch", __FILE__),
	device_cpc_expansion_card_interface(mconfig, *this),
	m_rtc(*this,"rtc")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cpc_smartwatch_device::device_start()
{
	m_slot = dynamic_cast<cpc_expansion_slot_device *>(owner());
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cpc_smartwatch_device::device_reset()
{
	device_t* cpu = machine().device(":maincpu");
	address_space& space = cpu->memory().space(AS_PROGRAM);
	space.install_read_handler(0xc000,0xc001,0,0,read8_delegate(FUNC(cpc_smartwatch_device::rtc_w),this));
	space.install_read_handler(0xc004,0xc004,0,0,read8_delegate(FUNC(cpc_smartwatch_device::rtc_r),this));
	m_bank = membank(":bank7");
}

READ8_MEMBER(cpc_smartwatch_device::rtc_w)
{
	UINT8* bank = (UINT8*)m_bank->base();
	if(offset & 1)
		m_rtc->read_1(space,0);
	else
		m_rtc->read_0(space,0);
	return bank[offset & 1];
}

READ8_MEMBER(cpc_smartwatch_device::rtc_r)
{
	UINT8* bank = (UINT8*)m_bank->base();
	return ((bank[(offset & 1)+4]) & 0xfe) | (m_rtc->read_data(space,0) & 0x01);
}
