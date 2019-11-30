// license:BSD-3-Clause
// copyright-holders:Curt Coder, Phill Harvey-Smith
/**********************************************************************

    Miracle Systems QL Trump Card emulation

**********************************************************************/

#include "emu.h"
#include "trumpcard.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define WD1772_TAG      "wd1772"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(QL_TRUMP_CARD,      ql_trump_card_device,      "ql_trump",    "QL Trump Card")
DEFINE_DEVICE_TYPE(QL_TRUMP_CARD_256K, ql_trump_card_256k_device, "ql_trump256", "QL Trump Card 256K")
DEFINE_DEVICE_TYPE(QL_TRUMP_CARD_512K, ql_trump_card_512k_device, "ql_trump512", "QL Trump Card 512K")
DEFINE_DEVICE_TYPE(QL_TRUMP_CARD_768K, ql_trump_card_768k_device, "ql_trump768", "QL Trump Card 768K")


//-------------------------------------------------
//  ROM( ql_trump_card )
//-------------------------------------------------

ROM_START( ql_trump_card )
	ROM_REGION( 0x8000, "rom", 0 )
	ROM_DEFAULT_BIOS("v131")
	ROM_SYSTEM_BIOS( 0, "v121a", "v1.21A" )
	ROMX_LOAD( "trump_card1v21a_256_bin", 0x0000, 0x8000, CRC(2eb0aa3a) SHA1(22da747afad5bc91184daf0d8b055a5e5264c67b), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v125", "v1.25" )
	ROMX_LOAD( "trumpcard-125.rom", 0x0000, 0x8000, CRC(938eaa46) SHA1(9b3458cf3a279ed86ba395dc45c8f26939d6c44d), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v127", "v1.27" )
	ROMX_LOAD( "trumpcard127.bin", 0x0000, 0x8000, CRC(3e053381) SHA1(69fa132cb73b9391a70e8fd3e5656890dbc0203f), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "v128", "v1.28" )
	ROMX_LOAD( "trump_card_v1_28.bin", 0x0000, 0x8000, CRC(4591a924) SHA1(3ad584ee74b6a7e46685e6b7bece1abe4d6f6937), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "v131", "v1.31" )
	ROMX_LOAD( "trumpcard131.bin", 0x0000, 0x8000, CRC(584c7835) SHA1(de0f67408021a3b33b3916514a5f81d9c8edad93), ROM_BIOS(4) )

	ROM_REGION( 0x100, "plds", 0 )
	ROM_LOAD( "1u4", 0x000, 0x100, NO_DUMP )
	ROM_LOAD( "2u4", 0x000, 0x100, NO_DUMP )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *ql_trump_card_device::device_rom_region() const
{
	return ROM_NAME( ql_trump_card );
}


//-------------------------------------------------
//  SLOT_INTERFACE( ql_trump_card_floppies )
//-------------------------------------------------

static void ql_trump_card_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( ql_trump_card_device::floppy_formats )
	FLOPPY_QL_FORMAT
FLOPPY_FORMATS_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void ql_trump_card_device::device_add_mconfig(machine_config &config)
{
	WD1772(config, m_fdc, 8000000);
	FLOPPY_CONNECTOR(config, m_floppy0, ql_trump_card_floppies, "35dd", ql_trump_card_device::floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy1, ql_trump_card_floppies, nullptr, ql_trump_card_device::floppy_formats);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ql_trump_card_device - constructor
//-------------------------------------------------

ql_trump_card_device::ql_trump_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ql_trump_card_device(mconfig, QL_TRUMP_CARD, tag, owner, clock, 0)
{
}

ql_trump_card_device::ql_trump_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int ram_size) :
	device_t(mconfig, type, tag, owner, clock),
	device_ql_expansion_card_interface(mconfig, *this),
	m_fdc(*this, WD1772_TAG),
	m_floppy0(*this, WD1772_TAG":0"),
	m_floppy1(*this, WD1772_TAG":1"),
	m_rom(*this, "rom"),
	m_ram(*this, "ram"),
	m_ram_size(ram_size),
	m_rom_en(false)
{
}

ql_trump_card_256k_device::ql_trump_card_256k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ql_trump_card_device(mconfig, QL_TRUMP_CARD_256K, tag, owner, clock, 256*1024)
{
}

ql_trump_card_512k_device::ql_trump_card_512k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ql_trump_card_device(mconfig, QL_TRUMP_CARD_512K, tag, owner, clock, 512*1024)
{
}

ql_trump_card_768k_device::ql_trump_card_768k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ql_trump_card_device(mconfig, QL_TRUMP_CARD_768K, tag, owner, clock, 768*1024)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ql_trump_card_device::device_start()
{
	// allocate memory
	m_ram.allocate(m_ram_size);

	// state saving
	save_item(NAME(m_rom_en));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ql_trump_card_device::device_reset()
{
	m_fdc->set_floppy(nullptr);
	m_fdc->dden_w(0);

	m_rom_en = false;
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t ql_trump_card_device::read(offs_t offset, uint8_t data)
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
		data = m_fdc->read(offset & 0x03);
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

void ql_trump_card_device::write(offs_t offset, uint8_t data)
{
	if (offset >= 0x1c000 && offset <= 0x1c003)
	{
		m_fdc->write(offset & 0x03, data);
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

		floppy_image_device *floppy = nullptr;

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
