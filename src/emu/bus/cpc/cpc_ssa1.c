// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * cpc_ssa1.c  --  Amstrad SSA-1 Speech Synthesiser, dk'Tronics Speech Synthesiser
 *
 *  Created on: 16/07/2011
 *
 */


#include "emu.h"
#include "cpc_ssa1.h"
#include "includes/amstrad.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CPC_SSA1 = &device_creator<cpc_ssa1_device>;
const device_type CPC_DKSPEECH = &device_creator<cpc_dkspeech_device>;

//-------------------------------------------------
//  device I/O handlers
//-------------------------------------------------

READ8_MEMBER(cpc_ssa1_device::ssa1_r)
{
	UINT8 ret = 0xff;

	if(get_sby() == 0)
		ret &= ~0x80;

	if(get_lrq() != 0)
		ret &= ~0x40;

	return ret;
}

WRITE8_MEMBER(cpc_ssa1_device::ssa1_w)
{
	m_sp0256_device->ald_w(space, 0, data);
}

READ8_MEMBER(cpc_dkspeech_device::dkspeech_r)
{
	UINT8 ret = 0xff;

	// SBY is not connected

	if(get_lrq() != 0)
		ret &= ~0x80;

	return ret;
}

WRITE8_MEMBER(cpc_dkspeech_device::dkspeech_w)
{
	m_sp0256_device->ald_w(space, 0, data & 0x3f);
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

const rom_entry *cpc_ssa1_device::device_rom_region() const
{
	return ROM_NAME( cpc_ssa1 );
}

const rom_entry *cpc_dkspeech_device::device_rom_region() const
{
	return ROM_NAME( cpc_dkspeech );
}

// device machine config
static MACHINE_CONFIG_FRAGMENT( cpc_ssa1 )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("sp0256",SP0256,XTAL_3_12MHz)
	MCFG_SP0256_DATA_REQUEST_CB(WRITELINE(cpc_ssa1_device, lrq_cb))
	MCFG_SP0256_STANDBY_CB(WRITELINE(cpc_ssa1_device, sby_cb))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	// pass-through
	MCFG_DEVICE_ADD("exp", CPC_EXPANSION_SLOT, 0)
	MCFG_DEVICE_SLOT_INTERFACE(cpc_exp_cards, NULL, false)
	MCFG_CPC_EXPANSION_SLOT_OUT_IRQ_CB(DEVWRITELINE("^", cpc_expansion_slot_device, irq_w))
	MCFG_CPC_EXPANSION_SLOT_OUT_NMI_CB(DEVWRITELINE("^", cpc_expansion_slot_device, nmi_w))
	MCFG_CPC_EXPANSION_SLOT_OUT_ROMDIS_CB(DEVWRITELINE("^", cpc_expansion_slot_device, romdis_w))  // ROMDIS

MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( cpc_dkspeech )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("sp0256",SP0256,XTAL_4MHz)  // uses the CPC's clock from pin 50 of the expansion port
	MCFG_SP0256_DATA_REQUEST_CB(WRITELINE(cpc_dkspeech_device, lrq_cb))
	MCFG_SP0256_STANDBY_CB(WRITELINE(cpc_dkspeech_device, sby_cb))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	// pass-through
	MCFG_DEVICE_ADD("exp", CPC_EXPANSION_SLOT, 0)
	MCFG_DEVICE_SLOT_INTERFACE(cpc_exp_cards, NULL, false)
	MCFG_CPC_EXPANSION_SLOT_OUT_IRQ_CB(DEVWRITELINE("^", cpc_expansion_slot_device, irq_w))
	MCFG_CPC_EXPANSION_SLOT_OUT_NMI_CB(DEVWRITELINE("^", cpc_expansion_slot_device, nmi_w))
	MCFG_CPC_EXPANSION_SLOT_OUT_ROMDIS_CB(DEVWRITELINE("^", cpc_expansion_slot_device, romdis_w))  // ROMDIS

MACHINE_CONFIG_END

machine_config_constructor cpc_ssa1_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cpc_ssa1 );
}

machine_config_constructor cpc_dkspeech_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cpc_dkspeech );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

cpc_ssa1_device::cpc_ssa1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, CPC_SSA1, "SSA-1", tag, owner, clock, "cpc_ssa1", __FILE__),
	device_cpc_expansion_card_interface(mconfig, *this),
	m_lrq(1),
	m_sp0256_device(*this,"sp0256")
{
}

cpc_dkspeech_device::cpc_dkspeech_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, CPC_DKSPEECH, "DK'Tronics Speech Synthesiser", tag, owner, clock, "cpc_dkspeech", __FILE__),
	device_cpc_expansion_card_interface(mconfig, *this),
	m_lrq(1),
	m_sp0256_device(*this,"sp0256")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cpc_ssa1_device::device_start()
{
	device_t* cpu = machine().device("maincpu");
	address_space& space = cpu->memory().space(AS_IO);
	m_slot = dynamic_cast<cpc_expansion_slot_device *>(owner());

	m_rom = memregion("sp0256")->base();

//  m_sp0256_device = subdevice("sp0256");

	space.install_readwrite_handler(0xfaee,0xfaee,0,0,read8_delegate(FUNC(cpc_ssa1_device::ssa1_r),this),write8_delegate(FUNC(cpc_ssa1_device::ssa1_w),this));
	space.install_readwrite_handler(0xfbee,0xfbee,0,0,read8_delegate(FUNC(cpc_ssa1_device::ssa1_r),this),write8_delegate(FUNC(cpc_ssa1_device::ssa1_w),this));
}

void cpc_dkspeech_device::device_start()
{
	device_t* cpu = machine().device("maincpu");
	address_space& space = cpu->memory().space(AS_IO);
	m_slot = dynamic_cast<cpc_expansion_slot_device *>(owner());

	m_rom = memregion("sp0256")->base();

//  m_sp0256_device = subdevice("sp0256");

	space.install_readwrite_handler(0xfbfe,0xfbfe,0,0,read8_delegate(FUNC(cpc_dkspeech_device::dkspeech_r),this),write8_delegate(FUNC(cpc_dkspeech_device::dkspeech_w),this));
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
