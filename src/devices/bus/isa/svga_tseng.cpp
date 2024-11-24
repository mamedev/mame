// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/***************************************************************************

  ISA SVGA Tseng wrapper

  TODO:
  - Implement Korean font ROM for Kasan 16

***************************************************************************/

#include "emu.h"
#include "svga_tseng.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA16_SVGA_ET4K, isa16_svga_et4k_device, "et4000", "SVGA Tseng ET4000AX Graphics Card")
DEFINE_DEVICE_TYPE(ISA16_SVGA_ET4K_KASAN16, isa16_svga_et4k_kasan16_device, "et4000_kasan16", "SVGA Kasan Hangulmadang-16 ET4000AX Graphics Card")
DEFINE_DEVICE_TYPE(ISA16_SVGA_ET4K_W32I, isa16_svga_et4k_w32i_device, "et4kw32i", "SVGA Tseng ET4000/W32i Graphics Card")

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

/*
 *  (tseng labs famous et4000 isa vga card (oem))
 *  ROM_LOAD("et4000b.bin", 0xc0000, 0x8000, CRC(a903540d) SHA1(unknown) )
 *
 *  Version "8.06X 04/15/92" (yay for semver)
 *  Tested, draws corrupted GFX, likely bad
 *  ROM_LOAD("et4000.bin.other", 0xc0000, 0x8000, CRC(f01e4be0) SHA1(95d75ff41bcb765e50bd87a8da01835fd0aa01d5) )
 */
ROM_START( et4000 )
	ROM_REGION(0x8000,"vga_rom", 0)
	ROM_SYSTEM_BIOS(0, "v801x", "Tseng Version 8.01X 04/07/93")
	ROMX_LOAD("et4000.bin", 0x00000, 0x8000, CRC(f1e817a8) SHA1(945d405b0fb4b8f26830d495881f8587d90e5ef9), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "v800x", "ColorImage Version 8.00X 09/18/90")
	ROMX_LOAD("cvet4kax.bin", 0x00000, 0x8000, CRC(a3ab496c) SHA1(8644b4178cd0e841139bfa06c9da493dd74d22e8), ROM_BIOS(1) )
ROM_END

const tiny_rom_entry *isa16_svga_et4k_device::device_rom_region() const
{
	return ROM_NAME( et4000 );
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa16_svga_et4k_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update(m_vga, FUNC(tseng_vga_device::screen_update));

	TSENG_VGA(config, m_vga, 0);
	m_vga->set_screen("screen");
	m_vga->set_vram_size(1*1024*1024);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa16_vga_device - constructor
//-------------------------------------------------

isa16_svga_et4k_device::isa16_svga_et4k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa16_svga_et4k_device(mconfig, ISA16_SVGA_ET4K, tag, owner, clock)
{
}

isa16_svga_et4k_device::isa16_svga_et4k_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_vga(*this, "vga")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
uint8_t isa16_svga_et4k_device::input_port_0_r() { return 0xff; } //return machine().root_device().ioport("IN0")->read(); }

void isa16_svga_et4k_device::device_start()
{
	set_isa_device();

	remap(AS_PROGRAM, 0, 0xfffff);
	remap(AS_IO, 0, 0x3ff);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa16_svga_et4k_device::device_reset()
{
}

//-------------------------------------------------
//  remap - remap ram/io since something
//  could have unmapped it
//-------------------------------------------------

void isa16_svga_et4k_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(tseng_vga_device::mem_r)), write8sm_delegate(*m_vga, FUNC(tseng_vga_device::mem_w)));
		m_isa->install_rom(this, 0xc0000, 0xc7fff, "vga_rom");
	}
	else if (space_id == AS_IO)
		m_isa->install_device(0x03b0, 0x03df, *this, &isa16_svga_et4k_device::io_isa_map);
}

void isa16_svga_et4k_device::io_isa_map(address_map &map)
{
	map(0x00, 0x2f).m(m_vga, FUNC(tseng_vga_device::io_map));
}

/*
 * Korean cards
 *
 * Same as regular ET4000AX with extra font I/Os
 */

isa16_svga_et4k_kasan16_device::isa16_svga_et4k_kasan16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: isa16_svga_et4k_device(mconfig, ISA16_SVGA_ET4K_KASAN16, tag, owner, clock)
	, m_hangul_rom(*this, "hangul")
{
}

ROM_START( kasan16 )
	ROM_REGION(0x8000,"vga_rom", 0)
	ROM_SYSTEM_BIOS(0, "v802x", "Version 8.02X 06/13/90, Kasan BIOS Ver 1.0a 05/01/91")
	ROMX_LOAD("et4000_kasan16.bin", 0x00000, 0x8000, CRC(57bcc3ad) SHA1(a55e7eb27ef2b4f118ea2028835e88988f07cf57), ROM_BIOS(0) )

	ROM_REGION(0x80000, "hangul", 0)
	ROM_LOAD("kasan_ksc5601.rom", 0x00000, 0x80000, CRC(a547c5ec) SHA1(1358feb2ccaca040a176bedc7c256ec481351b41) )
ROM_END

const tiny_rom_entry *isa16_svga_et4k_kasan16_device::device_rom_region() const
{
	return ROM_NAME( kasan16 );
}

/*
 * ET4000/w32i
 */

isa16_svga_et4k_w32i_device::isa16_svga_et4k_w32i_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA16_SVGA_ET4K_W32I, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_vga(*this, "vga")
{
}


ROM_START( et4kw32i )
	ROM_REGION(0x8000,"vga_rom", 0)
	ROM_SYSTEM_BIOS(0, "v800n", "Tseng Version 8.00N 04/28/95 ISA 06/17/95")
	ROMX_LOAD("et4kw32i.vbi", 0x00000, 0x8000, CRC(14542962) SHA1(d5aee7205af8bd1fef0ecf1db2c07308a2b10b17), ROM_BIOS(0) )
ROM_END

const tiny_rom_entry *isa16_svga_et4k_w32i_device::device_rom_region() const
{
	return ROM_NAME( et4kw32i );
}

void isa16_svga_et4k_w32i_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update(m_vga, FUNC(et4kw32i_vga_device::screen_update));

	ET4KW32I_VGA(config, m_vga, 0);
	m_vga->set_screen("screen");
	// 1MB default, 4MB max
	m_vga->set_vram_size(1*1024*1024);
}

void isa16_svga_et4k_w32i_device::device_start()
{
	set_isa_device();

	remap(AS_PROGRAM, 0, 0xfffff);
	remap(AS_IO, 0, 0xffff);
}

void isa16_svga_et4k_w32i_device::device_reset()
{
}

void isa16_svga_et4k_w32i_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(tseng_vga_device::mem_r)), write8sm_delegate(*m_vga, FUNC(tseng_vga_device::mem_w)));
		// TODO: can be disabled or narrowed thru TS Auxiliary Mode
		m_isa->install_rom(this, 0xc0000, 0xc7fff, "vga_rom");
	}
	else if (space_id == AS_IO)
	{
		m_isa->install_device(0x0000, 0xffff, *this, &isa16_svga_et4k_w32i_device::io_isa_map);
	}
}

void isa16_svga_et4k_w32i_device::io_isa_map(address_map &map)
{
	map(0x03b0, 0x03df).m(m_vga, FUNC(et4kw32i_vga_device::io_map));
	// IOD controls mirroring
	map(0x210a, 0x210a).mirror(0x70).rw(m_vga, FUNC(et4kw32i_vga_device::acl_index_r), FUNC(et4kw32i_vga_device::acl_index_w));
	map(0x210b, 0x210b).mirror(0x70).rw(m_vga, FUNC(et4kw32i_vga_device::acl_data_r), FUNC(et4kw32i_vga_device::acl_data_w));
}

