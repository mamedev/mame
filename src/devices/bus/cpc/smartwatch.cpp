// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
    Dobbertin Smartwatch

    Created: 23/2/2015
*/

#include "emu.h"
#include "smartwatch.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CPC_SMARTWATCH, cpc_smartwatch_device, "cpc_smartwatch", "Dobbertin Smartwatch")


void cpc_smartwatch_device::device_add_mconfig(machine_config &config)
{
	DS1216E(config, m_rtc);
}


ROM_START( cpc_smartwatch )
	ROM_REGION( 0x4000, "exp_rom", 0 )
	ROM_LOAD( "timerom+.rom",   0x0000, 0x4000, CRC(ed42a147) SHA1(61750d0535a1fbf2a4addad9def332cbcf8917c3) )
ROM_END

const tiny_rom_entry *cpc_smartwatch_device::device_rom_region() const
{
	return ROM_NAME( cpc_smartwatch );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

cpc_smartwatch_device::cpc_smartwatch_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CPC_SMARTWATCH, tag, owner, clock),
	device_cpc_expansion_card_interface(mconfig, *this), m_slot(nullptr),
	m_rtc(*this,"rtc"), m_bank(nullptr)
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
	address_space &space = m_slot->cpu().space(AS_PROGRAM);
	// FIXME: should cover the whole ROM address decode range
	space.install_read_handler(0xc000,0xc004, read8sm_delegate(*this, FUNC(cpc_smartwatch_device::rtc_r)));
	m_bank = membank(":bank7");
}

uint8_t cpc_smartwatch_device::rtc_r(offs_t offset)
{
	uint8_t* bank = (uint8_t*)m_bank->base();
	if (m_rtc->ceo_r())
		return m_rtc->read(offset);
	else
		m_rtc->read(offset);
	return bank[offset];
}
