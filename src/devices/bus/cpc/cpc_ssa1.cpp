// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * cpc_ssa1.cpp  --  Amstrad SSA-1 Speech Synthesiser, dk'Tronics Speech Synthesiser
 *
 *  Created on: 16/07/2011
 *
 */


#include "emu.h"
#include "cpc_ssa1.h"
#include "speaker.h"


void cpc_exp_cards(device_slot_interface &device);

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CPC_SSA1,     cpc_ssa1_device,     "cpc_ssa1",     "Amstrad SSA-1")
DEFINE_DEVICE_TYPE(CPC_DKSPEECH, cpc_dkspeech_device, "cpc_dkspeech", "DK'Tronics Speech Synthesiser")

//-------------------------------------------------
//  device I/O handlers
//-------------------------------------------------

READ8_MEMBER(cpc_ssa1_device::ssa1_r)
{
	uint8_t ret = 0xff;

	if(get_sby() == 0)
		ret &= ~0x80;

	if(get_lrq() != 0)
		ret &= ~0x40;

	return ret;
}

WRITE8_MEMBER(cpc_ssa1_device::ssa1_w)
{
	m_sp0256_device->ald_w(data);
}

READ8_MEMBER(cpc_dkspeech_device::dkspeech_r)
{
	uint8_t ret = 0xff;

	// SBY is not connected

	if(get_lrq() != 0)
		ret &= ~0x80;

	return ret;
}

WRITE8_MEMBER(cpc_dkspeech_device::dkspeech_w)
{
	m_sp0256_device->ald_w(data & 0x3f);
}

WRITE_LINE_MEMBER(cpc_ssa1_device::lrq_cb)
{
	set_lrq(state);
}

WRITE_LINE_MEMBER(cpc_ssa1_device::sby_cb)
{
	set_sby(state);
}

WRITE_LINE_MEMBER(cpc_dkspeech_device::lrq_cb)
{
	set_lrq(state);
}

WRITE_LINE_MEMBER(cpc_dkspeech_device::sby_cb)
{
	set_sby(state);
}

//-------------------------------------------------
//  Device ROM definition
//-------------------------------------------------

// Has no actual ROM, just that internal to the SP0256
ROM_START( cpc_ssa1 )
	ROM_REGION( 0x10000, "sp0256", 0 )
	ROM_LOAD( "sp0256-al2.bin",   0x1000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc) )
ROM_END

// Available in ROM and cassette versions.  We'll add the ROM for convenience.
ROM_START( cpc_dkspeech )
	ROM_REGION( 0x10000, "sp0256", 0 )
	ROM_LOAD( "sp0256-al2.bin",   0x1000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc) )

	ROM_REGION( 0x4000, "exp_rom", 0 )
	ROM_LOAD( "dkspeech.rom",   0x0000, 0x4000, CRC(4957c2f5) SHA1(bf28e07d5fada3678faab77f582b802164e82f62) )
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *cpc_ssa1_device::device_rom_region() const
{
	return ROM_NAME( cpc_ssa1 );
}

const tiny_rom_entry *cpc_dkspeech_device::device_rom_region() const
{
	return ROM_NAME( cpc_dkspeech );
}

// device machine config
void cpc_ssa1_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	SP0256(config, m_sp0256_device, XTAL(3'120'000));
	m_sp0256_device->data_request_callback().set(FUNC(cpc_ssa1_device::lrq_cb));
	m_sp0256_device->standby_callback().set(FUNC(cpc_ssa1_device::sby_cb));
	m_sp0256_device->add_route(ALL_OUTPUTS, "mono", 1.00);

	// pass-through
	cpc_expansion_slot_device &exp(CPC_EXPANSION_SLOT(config, "exp", DERIVED_CLOCK(1, 1), cpc_exp_cards, nullptr));
	exp.irq_callback().set(DEVICE_SELF_OWNER, FUNC(cpc_expansion_slot_device::irq_w));
	exp.nmi_callback().set(DEVICE_SELF_OWNER, FUNC(cpc_expansion_slot_device::nmi_w));
	exp.romdis_callback().set(DEVICE_SELF_OWNER, FUNC(cpc_expansion_slot_device::romdis_w));  // ROMDIS
}

void cpc_dkspeech_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	SP0256(config, m_sp0256_device, DERIVED_CLOCK(1, 1));  // uses the CPC's clock from pin 50 of the expansion port
	m_sp0256_device->data_request_callback().set(FUNC(cpc_dkspeech_device::lrq_cb));
	m_sp0256_device->standby_callback().set(FUNC(cpc_dkspeech_device::sby_cb));
	m_sp0256_device->add_route(ALL_OUTPUTS, "mono", 1.00);

	// pass-through
	cpc_expansion_slot_device &exp(CPC_EXPANSION_SLOT(config, "exp", DERIVED_CLOCK(1, 1), cpc_exp_cards, nullptr));
	exp.irq_callback().set(DEVICE_SELF_OWNER, FUNC(cpc_expansion_slot_device::irq_w));
	exp.nmi_callback().set(DEVICE_SELF_OWNER, FUNC(cpc_expansion_slot_device::nmi_w));
	exp.romdis_callback().set(DEVICE_SELF_OWNER, FUNC(cpc_expansion_slot_device::romdis_w));  // ROMDIS
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

cpc_ssa1_device::cpc_ssa1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CPC_SSA1, tag, owner, clock),
	device_cpc_expansion_card_interface(mconfig, *this), m_slot(nullptr), m_rom(nullptr),
	m_lrq(1), m_sby(0),
	m_sp0256_device(*this,"sp0256")
{
}

cpc_dkspeech_device::cpc_dkspeech_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CPC_DKSPEECH, tag, owner, clock),
	device_cpc_expansion_card_interface(mconfig, *this), m_slot(nullptr), m_rom(nullptr),
	m_lrq(1), m_sby(0),
	m_sp0256_device(*this,"sp0256")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cpc_ssa1_device::device_start()
{
	m_slot = dynamic_cast<cpc_expansion_slot_device *>(owner());
	address_space &space = m_slot->cpu().space(AS_IO);

	m_rom = memregion("sp0256")->base();

	space.install_readwrite_handler(0xfaee,0xfaee, read8_delegate(*this, FUNC(cpc_ssa1_device::ssa1_r)), write8_delegate(*this, FUNC(cpc_ssa1_device::ssa1_w)));
	space.install_readwrite_handler(0xfbee,0xfbee, read8_delegate(*this, FUNC(cpc_ssa1_device::ssa1_r)), write8_delegate(*this, FUNC(cpc_ssa1_device::ssa1_w)));
}

void cpc_dkspeech_device::device_start()
{
	m_slot = dynamic_cast<cpc_expansion_slot_device *>(owner());
	address_space &space = m_slot->cpu().space(AS_IO);

	m_rom = memregion("sp0256")->base();

	space.install_readwrite_handler(0xfbfe,0xfbfe, read8_delegate(*this, FUNC(cpc_dkspeech_device::dkspeech_r)), write8_delegate(*this, FUNC(cpc_dkspeech_device::dkspeech_w)));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cpc_ssa1_device::device_reset()
{
	m_sp0256_device->reset();
}

void cpc_dkspeech_device::device_reset()
{
	m_sp0256_device->reset();
}
