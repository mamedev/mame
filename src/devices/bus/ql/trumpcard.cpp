// license:BSD-3-Clause
// copyright-holders:Curt Coder, Phill Harvey-Smith
/**********************************************************************

    Miracle Systems QL Trump Card emulation

**********************************************************************/

#include "trumpcard.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define WD1772_TAG      "wd1772"



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
	ROM_DEFAULT_BIOS("v131")
	ROM_SYSTEM_BIOS( 0, "v121a", "v1.21A" )
	ROMX_LOAD( "trump_card1v21a_256_bin", 0x0000, 0x8000, CRC(2eb0aa3a) SHA1(22da747afad5bc91184daf0d8b055a5e5264c67b), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "v125", "v1.25" )
	ROMX_LOAD( "trumpcard-125.rom", 0x0000, 0x8000, CRC(938eaa46) SHA1(9b3458cf3a279ed86ba395dc45c8f26939d6c44d), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "v127", "v1.27" )
	ROMX_LOAD( "trumpcard127.bin", 0x0000, 0x8000, CRC(3e053381) SHA1(69fa132cb73b9391a70e8fd3e5656890dbc0203f), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 3, "v128", "v1.28" )
	ROMX_LOAD( "trump_card_v1_28.bin", 0x0000, 0x8000, CRC(4591a924) SHA1(3ad584ee74b6a7e46685e6b7bece1abe4d6f6937), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 4, "v131", "v1.31" )
	ROMX_LOAD( "trumpcard131.bin", 0x0000, 0x8000, CRC(584c7835) SHA1(de0f67408021a3b33b3916514a5f81d9c8edad93), ROM_BIOS(5) )

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
	MCFG_DEVICE_ADD(WD1772_TAG, WD1772, 8000000)
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
	device_t(mconfig, QL_TRUMP_CARD, "QL Trump Card", tag, owner, clock, "ql_trump", __FILE__),
	device_ql_expansion_card_interface(mconfig, *this),
	m_fdc(*this, WD1772_TAG),
	m_floppy0(*this, WD1772_TAG":0"),
	m_floppy1(*this, WD1772_TAG":1"),
	m_rom(*this, "rom"),
	m_ram(*this, "ram"),
	m_ram_size(0), m_rom_en(false)
{
}

ql_trump_card_t::ql_trump_card_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, int ram_size) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, __FILE__),
	device_ql_expansion_card_interface(mconfig, *this),
	m_fdc(*this, WD1772_TAG),
	m_floppy0(*this, WD1772_TAG":0"),
	m_floppy1(*this, WD1772_TAG":1"),
	m_rom(*this, "rom"),
	m_ram(*this, "ram"),
	m_ram_size(ram_size), m_rom_en(false)
{
}

ql_trump_card_256k_t::ql_trump_card_256k_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ql_trump_card_t(mconfig, QL_TRUMP_CARD_256K, "QL Trump Card 256K", tag, owner, clock, "ql_trump256", __FILE__, 256*1024) { }

ql_trump_card_512k_t::ql_trump_card_512k_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ql_trump_card_t(mconfig, QL_TRUMP_CARD_512K, "QL Trump Card 512K", tag, owner, clock, "ql_trump512", __FILE__, 512*1024) { }

ql_trump_card_768k_t::ql_trump_card_768k_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ql_trump_card_t(mconfig, QL_TRUMP_CARD_768K, "QL Trump Card 768K", tag, owner, clock, "ql_trump768", __FILE__, 768*1024) { }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ql_trump_card_t::device_start()
{
	// allocate memory
	m_ram.allocate(m_ram_size);

	// state saving
	save_item(NAME(m_rom_en));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ql_trump_card_t::device_reset()
{
	m_fdc->set_floppy(NULL);
	m_fdc->dden_w(0);

	m_rom_en = false;
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

UINT8 ql_trump_card_t::read(address_space &space, offs_t offset, UINT8 data)
{
	if (offset >= 0xc000 && offset < 0x10000)
	{
		m_rom_en = true;
	}

	if (offset >= 0x10000 && offset < 0x18000)
	{
		m_rom_en = false;

		data = m_rom->base()[offset & 0x7fff];
	}

	if (offset >= 0x1c000 && offset <= 0x1c003)
	{
		data = m_fdc->read(space, offset & 0x03);
	}

	if (offset >= 0x40000 && offset < 0xc0000)
	{
		if ((offset - 0x40000) < m_ram_size)
		{
			data = m_ram[offset - 0x40000];
		}
	}

	if (offset >= 0xc0000)
	{
		if (m_rom_en)
		{
			if (offset < 0xc8000)
			{
				data = m_rom->base()[offset & 0x7fff];
			}
		}
		else
		{
			if ((offset - 0x40000) < m_ram_size)
			{
				data = m_ram[offset - 0x40000];
			}
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

		    bit     description

		    0       DRIVE1
		    1       DRIVE0
		    2       MOTOR
		    3       SIDE
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
			floppy->mon_w(!BIT(data, 2));
		}
	}

	if (offset >= 0x40000 && offset < 0xc0000)
	{
		if ((offset - 0x40000) < m_ram_size)
		{
			m_ram[offset - 0x40000] = data;
		}
	}

	if (offset >= 0xc0000)
	{
		if (!m_rom_en)
		{
			m_ram[offset - 0x40000] = data;
		}
	}
}
