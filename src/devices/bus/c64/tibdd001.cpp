// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TIB Disc Drive DD-001 cartridge emulation

**********************************************************************/

/*

    PCB Layout
    ----------

    |---------------|
    |      CN1      |
    |               |
    |      FDC      |
    |          16MHz|
    | LS00     LS00 |
    |      ROM      |
    |               |
     |||||||||||||||

Notes: (All ICs shown)
    FDC     - GoldStar GM82C765B Floppy Disk Subsystem Controller
    ROM     - Texas Instruments TMS2764JL-25 8Kx8 EPROM
    CN1     - floppy connector

*/

#include "emu.h"
#include "tibdd001.h"

#include "formats/tibdd001_dsk.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define GM82C765B_TAG "u4"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_TIB_DD_001, c64_tib_dd_001_device, "c64_tibdd001", "C64 TIB Disc Drive DD-001 cartridge")


ROM_START( c64_tibdd001 )
	ROM_REGION( 0x2000, "rom", 0 )
	ROM_DEFAULT_BIOS("v11")
	ROM_SYSTEM_BIOS( 0, "v10", "v1.0" )
	ROMX_LOAD( "rom_dd001_v1.0.bin", 0x0000, 0x2000, CRC(58f83983) SHA1(f18cfb42c8e2c9152e18b129d01408f702271363), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v11", "v1.1" )
	ROMX_LOAD( "rom_dd001_v1.1.bin", 0x0000, 0x2000, CRC(d266b628) SHA1(24d178560dad3d955ebbe15f4274d966e031142d), ROM_BIOS(1) )
ROM_END

const tiny_rom_entry *c64_tib_dd_001_device::device_rom_region() const
{
	return ROM_NAME( c64_tibdd001 );
}


static void tib_dd_001_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

void c64_tib_dd_001_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_TIB_DD_001_FORMAT);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c64_tib_dd_001_device::device_add_mconfig(machine_config &config)
{
	WD37C65B(config, m_fdc, 16_MHz_XTAL);
	m_fdc->hdl_wr_callback().set(FUNC(c64_tib_dd_001_device::motor_w));

	FLOPPY_CONNECTOR(config, m_floppy, tib_dd_001_floppies, "35dd", c64_tib_dd_001_device::floppy_formats, true).enable_sound(true);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_tib_dd_001_device - constructor
//-------------------------------------------------

c64_tib_dd_001_device::c64_tib_dd_001_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_TIB_DD_001, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_fdc(*this, GM82C765B_TAG),
	m_floppy(*this, GM82C765B_TAG":0"),
	m_rom(*this, "rom")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_tib_dd_001_device::device_start()
{
	m_exrom = 0;
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_tib_dd_001_device::device_reset()
{
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_tib_dd_001_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml)
	{
		data = m_rom[offset & 0x1fff];
	}

	if (sphi2 && !io1)
	{
		if (BIT(offset, 0))
		{
			data = m_fdc->fifo_r();
		}
		else
		{
			data = m_fdc->msr_r();
		}
	}

	if (sphi2 && !io2)
	{
		m_fdc->reset();
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_tib_dd_001_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (sphi2 && !io1)
	{
		if (BIT(offset, 0))
		{
			m_fdc->fifo_w(data);
		}
	}

	if (sphi2 && !io2)
	{
		m_fdc->reset();
	}
}
