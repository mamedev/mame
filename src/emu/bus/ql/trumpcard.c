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
const device_type QL_TRUMP_CARD_256K = &device_creator<ql_trump_card_256k_t>;
const device_type QL_TRUMP_CARD_512K = &device_creator<ql_trump_card_512k_t>;
const device_type QL_TRUMP_CARD_768K = &device_creator<ql_trump_card_768k_t>;


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
	device_t(mconfig, QL_TRUMP_CARD, "QL Trump Card", tag, owner, clock, "trump", __FILE__),
	device_ql_expansion_card_interface(mconfig, *this),
	m_fdc(*this, WD1772_TAG),
	m_floppy0(*this, WD1772_TAG":0"),
	m_floppy1(*this, WD1772_TAG":1"),
	m_rom(*this, "rom"),
	m_ram(*this, "ram"),
	m_ram_size(0)
{
}

ql_trump_card_t::ql_trump_card_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, int ram_size) :
	device_t(mconfig, QL_TRUMP_CARD, "QL Trump Card", tag, owner, clock, "trump", __FILE__),
	device_ql_expansion_card_interface(mconfig, *this),
	m_fdc(*this, WD1772_TAG),
	m_floppy0(*this, WD1772_TAG":0"),
	m_floppy1(*this, WD1772_TAG":1"),
	m_rom(*this, "rom"),
	m_ram(*this, "ram"),
	m_ram_size(ram_size)
{
}

ql_trump_card_256k_t::ql_trump_card_256k_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ql_trump_card_t(mconfig, QL_TRUMP_CARD_256K, "QL Trump Card 256K", tag, owner, clock, "trump256k", __FILE__, 256*1024) { }

ql_trump_card_512k_t::ql_trump_card_512k_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ql_trump_card_t(mconfig, QL_TRUMP_CARD_512K, "QL Trump Card 512K", tag, owner, clock, "trump512k", __FILE__, 512*1024) { }

ql_trump_card_768k_t::ql_trump_card_768k_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ql_trump_card_t(mconfig, QL_TRUMP_CARD_768K, "QL Trump Card 768K", tag, owner, clock, "trump768k", __FILE__, 768*1024) { }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ql_trump_card_t::device_start()
{
	m_ram.allocate(m_ram_size);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ql_trump_card_t::device_reset()
{
	m_fdc->set_floppy(NULL);
	m_fdc->dden_w(0);

	m_rom_en = false;
	m_ram_en = false;
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

UINT8 ql_trump_card_t::read(address_space &space, offs_t offset, UINT8 data)
{
	if (!m_rom_en && offset >= 0xc000 && offset < 0x10000)
	{
		m_rom_en = true;

		data = m_rom->base()[offset & 0x3fff];
	}

	if (offset >= 0x10000 && offset < 0x18000)
	{
		if (m_ram_size == 768*1024)
		{
			m_ram_en = true;
		}

		data = m_rom->base()[offset & 0x7fff];
	}

	if (offset >= 0x1c000 && offset <= 0x1c003)
	{
		data = m_fdc->read(space, offset & 0x03);
	}

	if (offset >= 0x60000 && offset < 0xc0000)
	{
		if ((offset - 0x60000) < m_ram_size)
		{
			data = m_ram[offset - 0x60000];
		}
	}

	if (offset >= 0xc0000)
	{
		if (m_rom_en && offset < 0xc8000)
		{
			data = m_rom->base()[offset & 0x7fff];
		}

		if (m_ram_en)
		{
			data = m_ram[offset - 0x60000];
		}
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void ql_trump_card_t::write(address_space &space, offs_t offset, UINT8 data)
{
	if (offset >= 0x1c000 && offset <= 0x1c003)
	{
		m_fdc->write(space, offset & 0x03, data);
	}
	
	if (offset == 0x1e000)
	{
		/*

			bit		description

			0 		DRIVE1
			1 		DRIVE0
			2 		MOTOR
			3 		SIDE
			4 		
			5 		
			6 		
			7 		

		*/

		floppy_image_device *floppy = NULL;

		if (BIT(data, 1)) 
		{
			floppy = m_floppy0->get_device();
		}
		else if (BIT(data, 0))
		{
			floppy = m_floppy1->get_device();
		}

		m_fdc->set_floppy(floppy);

		if (floppy)
		{
			floppy->ss_w(BIT(data, 3));
			floppy->mon_w(BIT(data, 2));
		}
	}

	if (offset >= 0x60000 && offset < 0xc0000)
	{
		if ((offset - 0x60000) < m_ram_size)
		{
			m_ram[offset - 0x60000] = data;
		}
	}

	if (offset >= 0xc0000)
	{
		if (m_ram_en)
		{
			m_ram[offset - 0x60000] = data;
		}
	}
}
