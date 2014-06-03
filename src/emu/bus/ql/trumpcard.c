// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Miracle Systems QL Trump Card emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "trumpcard.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define WD1772_TAG		"wd1772"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type QL_TRUMP_CARD = &device_creator<ql_trump_card_t>;


//-------------------------------------------------
//  ROM( ql_trump_card )
//-------------------------------------------------

ROM_START( ql_trump_card )
	ROM_REGION( 0x8000, "rom", 0 )
	ROM_LOAD( "trumpcard-125.rom", 0x0000, 0x8000, CRC(938eaa46) SHA1(9b3458cf3a279ed86ba395dc45c8f26939d6c44d) )

	ROM_REGION( 0x100, "plds", 0 )
	ROM_LOAD( "1u4", 0x000, 0x100, NO_DUMP )
	ROM_LOAD( "2u4", 0x000, 0x100, NO_DUMP )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *ql_trump_card_t::device_rom_region() const
{
	return ROM_NAME( ql_trump_card );
}


//-------------------------------------------------
//  SLOT_INTERFACE( ql_trump_card_floppies )
//-------------------------------------------------

static SLOT_INTERFACE_START( ql_trump_card_floppies )
	SLOT_INTERFACE( "35dd", FLOPPY_35_DD )
SLOT_INTERFACE_END


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( ql_trump_card_t::floppy_formats )
	FLOPPY_QL_FORMAT
FLOPPY_FORMATS_END


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( ql_trump_card )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( ql_trump_card )
	MCFG_DEVICE_ADD(WD1772_TAG, WD1772x, 8000000)
	//MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(ql_trump_card_t, fdc_intrq_w))
	//MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(ql_trump_card_t, fdc_drq_w))
	MCFG_FLOPPY_DRIVE_ADD(WD1772_TAG":0", ql_trump_card_floppies, "35dd", ql_trump_card_t::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(WD1772_TAG":1", ql_trump_card_floppies, NULL, ql_trump_card_t::floppy_formats)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor ql_trump_card_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ql_trump_card );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ql_trump_card_t - constructor
//-------------------------------------------------

ql_trump_card_t::ql_trump_card_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, QL_TRUMP_CARD, "QL Trump Card", tag, owner, clock, "ql_trump_card", __FILE__),
	device_ql_expansion_card_interface(mconfig, *this),
	m_rom(*this, "rom"),
	m_ram(*this, "ram")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ql_trump_card_t::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ql_trump_card_t::device_reset()
{
}
