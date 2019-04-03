// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    IQ151 Disc2 cartridge emulation

***************************************************************************/

#include "emu.h"
#include "disc2.h"
#include "formats/iq151_dsk.h"


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

FLOPPY_FORMATS_MEMBER( iq151_disc2_device::floppy_formats )
	FLOPPY_IQ151_FORMAT
FLOPPY_FORMATS_END

static void iq151_disc2_floppies(device_slot_interface &device)
{
	device.option_add("8sssd", FLOPPY_8_SSSD);
}

ROM_START( iq151_disc2 )
	ROM_REGION(0x0800, "disc2", 0)
	ROM_LOAD( "iq151_disc2_12_5_1987_v4_0.rom", 0x0000, 0x0800, CRC(b189b170) SHA1(3e2ca80934177e7a32d0905f5a0ad14072f9dabf))
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(IQ151_DISC2, iq151_disc2_device, "iq151_disc2", "IQ151 Disc2")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  iq151_disc2_device - constructor
//-------------------------------------------------

iq151_disc2_device::iq151_disc2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, IQ151_DISC2, tag, owner, clock)
	, device_iq151cart_interface(mconfig, *this)
	, m_fdc(*this, "fdc"), m_rom(nullptr), m_rom_enabled(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void iq151_disc2_device::device_start()
{
	m_rom = (uint8_t*)memregion("disc2")->base();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void iq151_disc2_device::device_reset()
{
	m_rom_enabled = false;
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void iq151_disc2_device::device_add_mconfig(machine_config &config)
{
	UPD765A(config, m_fdc, 8'000'000, false, true);
	FLOPPY_CONNECTOR(config, "fdc:1", iq151_disc2_floppies, "8sssd", iq151_disc2_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:2", iq151_disc2_floppies, "8sssd", iq151_disc2_device::floppy_formats);
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *iq151_disc2_device::device_rom_region() const
{
	return ROM_NAME( iq151_disc2 );
}

//-------------------------------------------------
//  read
//-------------------------------------------------

void iq151_disc2_device::read(offs_t offset, uint8_t &data)
{
	// internal ROM is mapped at 0xe000-0xe7ff
	if (offset >= 0xe000 && offset < 0xe800 && m_rom_enabled)
		data = m_rom[offset & 0x7ff];
}


//-------------------------------------------------
//  IO read
//-------------------------------------------------

void iq151_disc2_device::io_read(offs_t offset, uint8_t &data)
{
	if (offset == 0xaa)
		data = m_fdc->msr_r();
	else if (offset == 0xab)
		data = m_fdc->fifo_r();
}

//-------------------------------------------------
//  IO write
//-------------------------------------------------

void iq151_disc2_device::io_write(offs_t offset, uint8_t data)
{
	if (offset == 0xab)
		m_fdc->fifo_w(data);
	else if (offset == 0xac)
		m_rom_enabled = (data == 0x01);
}
