// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Silicon Disc

***************************************************************************/

#include "emu.h"
#include "silicon_disc.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(EINSTEIN_SILICON_DISC, einstein_silicon_disc_device, "einstein_sd", "Einstein Silicon Disc")

//-------------------------------------------------
//  device_address_map
//-------------------------------------------------

void einstein_silicon_disc_device::map(address_map &map)
{
	map(0x08, 0x08).mirror(0xff00).w(FUNC(einstein_silicon_disc_device::sector_low_w));
	map(0x09, 0x09).mirror(0xff00).w(FUNC(einstein_silicon_disc_device::sector_high_w));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( silicon_disc )
	ROM_REGION(0x2000, "rom", 0)
	ROM_LOAD("sd11.bin", 0x0000, 0x2000, CRC(0e4f5e6d) SHA1(d0bc01e533d8963c596154435c5b2d156a96d470))
ROM_END

const tiny_rom_entry *einstein_silicon_disc_device::device_rom_region() const
{
	return ROM_NAME( silicon_disc );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  einstein_silicon_disc_device - constructor
//-------------------------------------------------

einstein_silicon_disc_device::einstein_silicon_disc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, EINSTEIN_SILICON_DISC, tag, owner, clock),
	device_tatung_pipe_interface(mconfig, *this),
	m_rom(*this, "rom"),
	m_bios(*this, ":bios"),
	m_sector(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void einstein_silicon_disc_device::device_start()
{
	// setup ram
	m_ram = std::make_unique<uint8_t[]>(0x40000);
	memset(m_ram.get(), 0xff, 0x40000);

	// register for save states
	save_pointer(NAME(m_ram), 0x40000);
	save_item(NAME(m_sector));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void einstein_silicon_disc_device::device_reset()
{
	// copy our rom into the main bios region so it will get picked up by the bank switching
	// not really part of the pipe interface cartridge but here for convenience, in reality
	// the rom gets put directly onto the main motherboard into the second empty socket
	memcpy(m_bios->base() + 0x4000, m_rom->base(), 0x2000);

	// install i/o ports
	io_space().install_device(0xf0, 0xff, *this, &einstein_silicon_disc_device::map);
	io_space().install_readwrite_handler(0xfa, 0xfa, 0, 0, 0xff00,
			read8_delegate(*this, FUNC(einstein_silicon_disc_device::ram_r)),
			write8_delegate(*this, FUNC(einstein_silicon_disc_device::ram_w)));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

WRITE8_MEMBER( einstein_silicon_disc_device::sector_low_w )
{
	m_sector &= 0xff00;
	m_sector |= data;
}

WRITE8_MEMBER( einstein_silicon_disc_device::sector_high_w )
{
	m_sector &= 0x00ff;
	m_sector |= ((data & 0x07) << 8);
}

// a8 to a14 are used to specify the byte in a 128-byte sector
READ8_MEMBER( einstein_silicon_disc_device::ram_r )
{
	return m_ram[(m_sector * 0x80) | ((offset >> 8) & 0x7f)];
}

WRITE8_MEMBER( einstein_silicon_disc_device::ram_w )
{
	m_ram[(m_sector * 0x80) | ((offset >> 8) & 0x7f)] = data;
}
