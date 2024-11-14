// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/***************************************************************************

  ISA SVGA Tseng wrapper

  TODO:
  - Implement Korean font ROM for Kasan 16

***************************************************************************/

#include "emu.h"
#include "svga_tseng.h"

/*
 *  (tseng labs famous et4000 isa vga card (oem))
 *  ROM_LOAD("et4000b.bin", 0xc0000, 0x8000, CRC(a903540d) SHA1(unknown) )
 *
 *  Version "8.06X 04/15/92" (yay for semver)
 *  Tested, draws corrupted GFX, likely bad
 *  ROM_LOAD("et4000.bin.other", 0xc0000, 0x8000, CRC(f01e4be0) SHA1(95d75ff41bcb765e50bd87a8da01835fd0aa01d5) )
 */
ROM_START( et4000 )
	ROM_REGION(0x8000,"et4000", 0)
	ROM_SYSTEM_BIOS(0, "v801x", "Tseng Version 8.01X 04/07/93")
	ROMX_LOAD("et4000.bin", 0x00000, 0x8000, CRC(f1e817a8) SHA1(945d405b0fb4b8f26830d495881f8587d90e5ef9), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "v800x", "ColorImage Version 8.00X 09/18/90")
	ROMX_LOAD("cvet4kax.bin", 0x00000, 0x8000, CRC(a3ab496c) SHA1(8644b4178cd0e841139bfa06c9da493dd74d22e8), ROM_BIOS(1) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_SVGA_ET4K, isa8_svga_et4k_device, "et4000", "SVGA Tseng ET4000AX Graphics Card")
DEFINE_DEVICE_TYPE(ISA8_SVGA_ET4K_KASAN16, isa8_svga_et4k_kasan16_device, "et4000_kasan16", "SVGA Kasan Hangulmadang-16 ET4000AX Graphics Card")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa8_svga_et4k_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update(m_vga, FUNC(tseng_vga_device::screen_update));

	TSENG_VGA(config, m_vga, 0);
	m_vga->set_screen("screen");
	m_vga->set_vram_size(0x100000);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *isa8_svga_et4k_device::device_rom_region() const
{
	return ROM_NAME( et4000 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_vga_device - constructor
//-------------------------------------------------

isa8_svga_et4k_device::isa8_svga_et4k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa8_svga_et4k_device(mconfig, ISA8_SVGA_ET4K, tag, owner, clock)
{
}

isa8_svga_et4k_device::isa8_svga_et4k_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
	, m_vga(*this, "vga")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
uint8_t isa8_svga_et4k_device::input_port_0_r() { return 0xff; } //return machine().root_device().ioport("IN0")->read(); }

void isa8_svga_et4k_device::device_start()
{
	set_isa_device();

	map_io();
	map_ram();
	map_rom();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_svga_et4k_device::device_reset()
{
}

//-------------------------------------------------
//  remap - remap ram/io since something
//  could have unmapped it
//-------------------------------------------------

void isa8_svga_et4k_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		map_ram();
		map_rom();
	}
	else if (space_id == AS_IO)
		map_io();
}

void isa8_svga_et4k_device::io_isa_map(address_map &map)
{
	map(0x00, 0x2f).m(m_vga, FUNC(tseng_vga_device::io_map));
}

void isa8_svga_et4k_device::map_io()
{
	m_isa->install_device(0x03b0, 0x03df, *this, &isa8_svga_et4k_device::io_isa_map);
}

void isa8_svga_et4k_device::map_ram()
{
	m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(tseng_vga_device::mem_r)), write8sm_delegate(*m_vga, FUNC(tseng_vga_device::mem_w)));
}

void isa8_svga_et4k_device::map_rom()
{
	m_isa->install_rom(this, 0xc0000, 0xc7fff, "et4000");
}

/*
 * Korean cards
 *
 * Same as regular ET4000AX with extra font I/Os
 */

isa8_svga_et4k_kasan16_device::isa8_svga_et4k_kasan16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: isa8_svga_et4k_device(mconfig, ISA8_SVGA_ET4K_KASAN16, tag, owner, clock)
	, m_hangul_rom(*this, "hangul")
{
}

ROM_START( kasan16 )
	ROM_REGION(0x8000,"et4000", 0)
	ROM_SYSTEM_BIOS(0, "v802x", "Version 8.02X 06/13/90, Kasan BIOS Ver 1.0a 05/01/91")
	ROMX_LOAD("et4000_kasan16.bin", 0x00000, 0x8000, CRC(57bcc3ad) SHA1(a55e7eb27ef2b4f118ea2028835e88988f07cf57), ROM_BIOS(0) )

	ROM_REGION(0x80000, "hangul", 0)
	ROM_LOAD("kasan_ksc5601.rom", 0x00000, 0x80000, CRC(a547c5ec) SHA1(1358feb2ccaca040a176bedc7c256ec481351b41) )
ROM_END

const tiny_rom_entry *isa8_svga_et4k_kasan16_device::device_rom_region() const
{
	return ROM_NAME( kasan16 );
}
