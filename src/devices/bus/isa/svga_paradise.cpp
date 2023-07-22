// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/***************************************************************************

ISA SVGA Paradise / Western Digital wrapper

TODO:
- Add ISA8 and MCA variants for pvga1a_jk;
- Nokia NVGA2, wd90c00_jk: they don't boot, accesses areas $c6000-$c7fff
  that aren't covered in current dumps;
  \- Specifically they do:
     000C03AB: cmp     word ptr [10h],0h ; wd90c00 == 0x3000
     000C03B0: 75 09          jne     0C03BBh
     000C03B2: cmp     byte ptr [12h],7Eh ; wd90c00 == 0x00
     000C03B7: jne     0C03BBh
     000C03B9: pop     ds
     000C03BA: ret
     000C03BB: jmp     0FFFFh:0h ; reset machine


***************************************************************************/

#include "emu.h"
#include "svga_paradise.h"

#include "screen.h"

// TODO: some of these are also ISA8
DEFINE_DEVICE_TYPE(ISA16_PVGA1A,      isa16_pvga1a_device,      "pvga1a",      "Paradise Systems PVGA1A Graphics Card")
DEFINE_DEVICE_TYPE(ISA16_PVGA1A_JK,   isa16_pvga1a_jk_device,   "pvga1a_jk",   "Paradise Systems PVGA1A-JK Graphics Card")
DEFINE_DEVICE_TYPE(ISA8_WD90C90_JK,   isa8_wd90c90_jk_device,   "wd90c90_jk",  "Western Digital WD90C90-JK Graphics Card")
DEFINE_DEVICE_TYPE(ISA16_WD90C00_JK,  isa16_wd90c00_jk_device,  "wd90c00_jk",  "Western Digital WD90C00-JK Graphics Card")
DEFINE_DEVICE_TYPE(ISA16_WD90C11_LR,  isa16_wd90c11_lr_device,  "wd90c11_lr",  "Western Digital WD90C11-LR Graphics Card \"1024 CX\"")
DEFINE_DEVICE_TYPE(ISA16_WD90C30_LR,  isa16_wd90c30_lr_device,  "wd90c30_lr",  "Western Digital WD90C30-LR Graphics Card \"1024 DX\"")
DEFINE_DEVICE_TYPE(ISA16_WD90C31_LR,  isa16_wd90c31_lr_device,  "wd90c31_lr",  "Western Digital WD90C31-LR Graphics Card")
DEFINE_DEVICE_TYPE(ISA16_WD90C31A_LR, isa16_wd90c31a_lr_device, "wd90c31a_lr", "Western Digital WD90C31A-LR Graphics Card")
//DEFINE_DEVICE_TYPE(ISA16_WD90C31_ZS,  isa16_wd90c31_zs_device,  "wd90c31_zs",  "Western Digital WD90C31-ZS Graphics Card")
DEFINE_DEVICE_TYPE(ISA16_WD90C31A_ZS, isa16_wd90c31a_zs_device, "wd90c31a_zs", "Western Digital WD90C31A-ZS Graphics Card")
// TODO: Also VL-Bus
DEFINE_DEVICE_TYPE(ISA16_WD90C33_ZZ,  isa16_wd90c33_zz_device,  "wd90c33_zz",  "Western Digital WD90C33-ZZ Graphics Card")

isa16_pvga1a_device::isa16_pvga1a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_PVGA1A, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_vga(*this, "vga")
{
}

ROM_START( pvga1a )
	ROM_REGION(0x8000,"vga_rom", 0)
	ROM_SYSTEM_BIOS(0, "pvga1a", "Paradise Systems PVGA1A")
	// TODO: pinpoint what rom size are these
	// BIOS.BIN     1xxxxxxxxxxxxxxx = 0xFF
	ROMX_LOAD("bios.bin", 0x000000, 0x008000, CRC(2be5d405) SHA1(5e3b4ebae221b7ad02f3eaa052178cb39d1d9bbe), ROM_BIOS(0))
	ROM_IGNORE( 0x8000 )
	// Western Digital licensed
	ROM_SYSTEM_BIOS(1, "go481", "Olivetti GO481")
	ROMX_LOAD("oli_go481_hi.bin", 0x000001, 0x004000, CRC(cda447be) SHA1(f397e3ddd3885666e3151fb4f681abf47fef36ba), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_IGNORE( 0x4000 )
	ROMX_LOAD("oli_go481_lo.bin", 0x000000, 0x004000, CRC(7db97902) SHA1(b93aa9d45942e98fb4932b4db34b82d459060adf), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_IGNORE( 0x4000 )

	// There's also a:
	// ROMX_LOAD( "paradisepvga1a.bin", 0x000000, 0x008000, CRC(e7c6883a) SHA1(61ae199d3a9077844c8a1aa80c3f5804c29383e8), ROM_BIOS(N) )
	// BIOS.BIN     [1/4]      paradisepvga1a.BIN [1/2]      IDENTICAL
	// Which doesn't boot with invalid REP opcode, assume bad.
ROM_END

const tiny_rom_entry *isa16_pvga1a_device::device_rom_region() const
{
	return ROM_NAME( pvga1a );
}

void isa16_pvga1a_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update("vga", FUNC(pvga1a_vga_device::screen_update));

	PVGA1A(config, m_vga, 0);
	m_vga->set_screen("screen");
	m_vga->set_vram_size(0x100000);
}

void isa16_pvga1a_device::io_isa_map(address_map &map)
{
	map(0x00, 0x2f).m(m_vga, FUNC(pvga1a_vga_device::io_map));
}

void isa16_pvga1a_device::device_start()
{
	set_isa_device();

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "vga_rom");

	m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(pvga1a_vga_device::mem_r)), write8sm_delegate(*m_vga, FUNC(pvga1a_vga_device::mem_w)));
	m_isa->install_device(0x03b0, 0x03df, *this, &isa16_pvga1a_device::io_isa_map);
}

/******************
 *
 * PVGA1A-JK
 *
 *****************/

isa16_pvga1a_jk_device::isa16_pvga1a_jk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_PVGA1A_JK, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_vga(*this, "vga")
{
}

ROM_START( pvga1a_jk )
	ROM_REGION(0x8000,"vga_rom", ROMREGION_ERASEFF)
	// "Paradise Basic VGA"?
	ROM_SYSTEM_BIOS(0, "pvga1a_jk", "Paradise Systems PVGA1A-JK")
	// BIOS.BIN     1xxxxxxxxxxxxxxx = 0xFF
	ROMX_LOAD( "bios.bin",     0x000000, 0x08000, CRC(2bfe8adb) SHA1(afd8c33f24e28b025e43ae68d95fd6811659013b), ROM_BIOS(0) )
	ROM_IGNORE( 0x8000 )
	// TODO: consider splitting if a good dump surfaces
	ROM_SYSTEM_BIOS(1, "nvga2", "Nokia NVGA2")
	ROMX_LOAD( "nokia.vbi",    0x000000, 0x06000, BAD_DUMP CRC(9f430ae7) SHA1(3d37b86853347d43ebc85a7e92e4a609b13406bb), ROM_BIOS(1) )
ROM_END

const tiny_rom_entry *isa16_pvga1a_jk_device::device_rom_region() const
{
	return ROM_NAME( pvga1a_jk );
}

void isa16_pvga1a_jk_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update("vga", FUNC(pvga1a_vga_device::screen_update));

	// TODO: is there any real difference between PVGA1A and PVGA1A-JK VGA controller wise?
	PVGA1A(config, m_vga, 0);
	m_vga->set_screen("screen");
	// 256kB to 1MB
	m_vga->set_vram_size(0x100000);
}

void isa16_pvga1a_jk_device::io_isa_map(address_map &map)
{
	map(0x00, 0x2f).m(m_vga, FUNC(pvga1a_vga_device::io_map));
}

void isa16_pvga1a_jk_device::device_start()
{
	set_isa_device();

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "vga_rom");

	m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(pvga1a_vga_device::mem_r)), write8sm_delegate(*m_vga, FUNC(pvga1a_vga_device::mem_w)));
	m_isa->install_device(0x03b0, 0x03df, *this, &isa16_pvga1a_jk_device::io_isa_map);
}

/******************
 *
 * WD90C90-JK
 *
 *****************/

isa8_wd90c90_jk_device::isa8_wd90c90_jk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA8_WD90C90_JK, tag, owner, clock),
	device_isa8_card_interface(mconfig, *this),
	m_vga(*this, "vga")
{
}

ROM_START( wd90c90_jk )
	ROM_REGION(0x8000,"vga_rom", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "wd90c90_jk", "ZTECH ZPVGA")
	ROMX_LOAD( "bios.bin", 0x000000, 0x008000, CRC(a7f0e81c) SHA1(578ee0e8f9a5e56df6c05386978d0f41c638ddcf), ROM_BIOS(0) )
	ROM_IGNORE( 0x8000 )
ROM_END

const tiny_rom_entry *isa8_wd90c90_jk_device::device_rom_region() const
{
	return ROM_NAME( wd90c90_jk );
}

void isa8_wd90c90_jk_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update("vga", FUNC(pvga1a_vga_device::screen_update));

	PVGA1A(config, m_vga, 0);
	m_vga->set_screen("screen");
	// 256kB to 1MB
	m_vga->set_vram_size(0x100000);
}

void isa8_wd90c90_jk_device::io_isa_map(address_map &map)
{
	map(0x00, 0x2f).m(m_vga, FUNC(pvga1a_vga_device::io_map));
}

void isa8_wd90c90_jk_device::device_start()
{
	set_isa_device();

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "vga_rom");

	m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(pvga1a_vga_device::mem_r)), write8sm_delegate(*m_vga, FUNC(pvga1a_vga_device::mem_w)));
	m_isa->install_device(0x03b0, 0x03df, *this, &isa8_wd90c90_jk_device::io_isa_map);
}

/******************
 *
 * WD90C00-JK
 *
 *****************/

isa16_wd90c00_jk_device::isa16_wd90c00_jk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_WD90C00_JK, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_vga(*this, "vga")
{
}

ROM_START( wd90c00_jk )
	ROM_REGION(0x8000,"vga_rom", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS(0, "wd90c00_jk", "Western Digital WD90C00-JK")
	// "vga bios western digital wd90c00 1989 (part 1).bin"
	ROMX_LOAD( "wdh.bin", 0x000001, 0x004000, CRC(0f4c7aed) SHA1(bdcc298b863ea7f37b1352164107264a12030cb5), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "wdl.bin", 0x000000, 0x004000, CRC(6916523e) SHA1(
a4024e8a71b310fc95a37743bc255212c7dd73a5), ROM_SKIP(1) | ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "wd90c00_jk_1", "Western Digital WD90C00-JK (alt)")
	ROMX_LOAD( "wd90c00h.bin", 0x000001, 0x004000, CRC(b3279a8d) SHA1(d531c5e3d2c083190477b8d799dc5205c9357181), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "wd90c00l.bin", 0x000000, 0x004000, CRC(a4fdc1db) SHA1(9680ab2049a0fd49bb23eb0a08487518cb9fa861), ROM_SKIP(1) | ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(2, "wd90c00_jk_v", "Western Digital WD90C00-JK (alt .vbi)")
	ROMX_LOAD( "90c00jk-lh6673.vbi", 0x000000, 0x006000, BAD_DUMP CRC(595eda15) SHA1(a3d4a978f30a0669bcb4a74ce196fd7a798d456f), ROM_BIOS(2) )
ROM_END

const tiny_rom_entry *isa16_wd90c00_jk_device::device_rom_region() const
{
	return ROM_NAME( wd90c00_jk );
}

void isa16_wd90c00_jk_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update("vga", FUNC(wd90c00_vga_device::screen_update));

	WD90C00(config, m_vga, 0);
	m_vga->set_screen("screen");
	// 256kB, 512kB, 1MB
	m_vga->set_vram_size(0x100000);
}

void isa16_wd90c00_jk_device::io_isa_map(address_map &map)
{
	map(0x00, 0x2f).m(m_vga, FUNC(wd90c00_vga_device::io_map));
}

void isa16_wd90c00_jk_device::device_start()
{
	set_isa_device();

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "vga_rom");

	m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(wd90c00_vga_device::mem_r)), write8sm_delegate(*m_vga, FUNC(wd90c00_vga_device::mem_w)));
	m_isa->install_device(0x03b0, 0x03df, *this, &isa16_wd90c00_jk_device::io_isa_map);
}

/******************
 *
 * WD90C11-LR
 *
 *****************/

isa16_wd90c11_lr_device::isa16_wd90c11_lr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_WD90C11_LR, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_vga(*this, "vga")
{
}

ROM_START( wd90c11_lr )
	ROM_REGION(0x8000,"vga_rom", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS(0, "wd90c11_lr", "Western Digital WD90C11-LR")
	ROMX_LOAD( "wd90c11.vbi",  0x000000, 0x008000, CRC(9c1296d7) SHA1(10fd263ab0187d8960d4cb2954254732ac29472f), ROM_BIOS(0) )
ROM_END

const tiny_rom_entry *isa16_wd90c11_lr_device::device_rom_region() const
{
	return ROM_NAME( wd90c11_lr );
}

void isa16_wd90c11_lr_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update("vga", FUNC(wd90c11a_vga_device::screen_update));

	WD90C11A(config, m_vga, 0);
	m_vga->set_screen("screen");
	// 512KB (+ option for 1MB? Verify with interlace)
	m_vga->set_vram_size(0x100000);
}

void isa16_wd90c11_lr_device::io_isa_map(address_map &map)
{
	map(0x00, 0x2f).m(m_vga, FUNC(wd90c11a_vga_device::io_map));
}

void isa16_wd90c11_lr_device::device_start()
{
	set_isa_device();

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "vga_rom");

	m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(wd90c11a_vga_device::mem_r)), write8sm_delegate(*m_vga, FUNC(wd90c11a_vga_device::mem_w)));
	m_isa->install_device(0x03b0, 0x03df, *this, &isa16_wd90c11_lr_device::io_isa_map);
}

/******************
 *
 * WD90C30-LR
 *
 *****************/

isa16_wd90c30_lr_device::isa16_wd90c30_lr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_WD90C30_LR, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_vga(*this, "vga")
{
}

ROM_START( wd90c30_lr )
	ROM_REGION(0x8000,"vga_rom", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS(0, "wd90c30_lr", "Western Digital WD90C30-LR")
	ROMX_LOAD(  "90c30-lr.vbi", 0x000000, 0x008000, CRC(3356ad43) SHA1(6cd56cf274b3c9262b7ca12d49bae63afc331c58), ROM_BIOS(0) )
ROM_END

const tiny_rom_entry *isa16_wd90c30_lr_device::device_rom_region() const
{
	return ROM_NAME( wd90c30_lr );
}

void isa16_wd90c30_lr_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update("vga", FUNC(wd90c30_vga_device::screen_update));

	WD90C30(config, m_vga, 0);
	m_vga->set_screen("screen");
	// 512KB, 1MB
	m_vga->set_vram_size(0x100000);
}

void isa16_wd90c30_lr_device::io_isa_map(address_map &map)
{
	map(0x00, 0x2f).m(m_vga, FUNC(wd90c30_vga_device::io_map));
}

void isa16_wd90c30_lr_device::device_start()
{
	set_isa_device();

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "vga_rom");

	m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(wd90c30_vga_device::mem_r)), write8sm_delegate(*m_vga, FUNC(wd90c30_vga_device::mem_w)));
	m_isa->install_device(0x03b0, 0x03df, *this, &isa16_wd90c30_lr_device::io_isa_map);
}

/******************
 *
 * WD90C31-LR
 *
 *****************/

isa16_wd90c31_lr_device::isa16_wd90c31_lr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_WD90C31_LR, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_vga(*this, "vga")
{
}

ROM_START( wd90c31_lr )
	ROM_REGION(0x8000,"vga_rom", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS(0, "wdxlr831", "Western Digital WD90C31-LR")
	ROMX_LOAD( "62-003259-800.bin", 0x000001, 0x004000, CRC(e71090bd) SHA1(f784ebc14801a0944271aab9ba4746dd9d0e001d), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "62-003260-800.bin", 0x000000, 0x004000, CRC(1e66af70) SHA1(7f236d6acb34d07480584d51f6bb57836ff262c4), ROM_SKIP(1) | ROM_BIOS(0) )
ROM_END

const tiny_rom_entry *isa16_wd90c31_lr_device::device_rom_region() const
{
	return ROM_NAME( wd90c31_lr );
}

void isa16_wd90c31_lr_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update("vga", FUNC(wd90c30_vga_device::screen_update));

	WD90C31(config, m_vga, 0);
	m_vga->set_screen("screen");
	// 512KB, 1MB
	m_vga->set_vram_size(0x100000);
}

void isa16_wd90c31_lr_device::io_isa_map(address_map &map)
{
	map(0x00, 0x2f).m(m_vga, FUNC(wd90c31_vga_device::io_map));
}

void isa16_wd90c31_lr_device::device_start()
{
	set_isa_device();

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "vga_rom");

	m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(wd90c31_vga_device::mem_r)), write8sm_delegate(*m_vga, FUNC(wd90c31_vga_device::mem_w)));
	m_isa->install_device(0x03b0, 0x03df, *this, &isa16_wd90c31_lr_device::io_isa_map);
	m_isa->install_device(0x23c0, 0x23c7, *m_vga, &wd90c31_vga_device::ext_io_map);
}


/******************
 *
 * WD90C31A-LR
 *
 *****************/

isa16_wd90c31a_lr_device::isa16_wd90c31a_lr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_WD90C31A_LR, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_vga(*this, "vga")
{
}

ROM_START( wd90c31a_lr )
	ROM_REGION(0x8000,"vga_rom", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS(0, "speed24x", "Diamond Speedstar 24x (Vers. 1.04)")
	ROMX_LOAD( "wd90c31alrdiamondspeedstar24x2.bin", 0x000000, 0x008000, CRC(578cb3c3) SHA1(ca7d871f9589eb06ace8075dd2d87a59bd191744), ROM_BIOS(0) )
//  "wd90c31alrdiamondspeedstar24x1.bin" identical to above
	ROM_SYSTEM_BIOS(1, "wd90c31a_lr", "Western Digital WD90C31A-LR")
	ROMX_LOAD( "bios.bin",     0x000000, 0x008000, CRC(cdc4c32e) SHA1(52ed7b8301ec5ebab0d87bab8cddd9cc8612e2ab), ROM_BIOS(1) )
	ROM_IGNORE( 0x8000 )
ROM_END

const tiny_rom_entry *isa16_wd90c31a_lr_device::device_rom_region() const
{
	return ROM_NAME( wd90c31a_lr );
}

void isa16_wd90c31a_lr_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update("vga", FUNC(wd90c00_vga_device::screen_update));

	WD90C31(config, m_vga, 0); // WD90C31A
	m_vga->set_screen("screen");
	// 512KB, 1MB
	m_vga->set_vram_size(0x100000);
}

void isa16_wd90c31a_lr_device::io_isa_map(address_map &map)
{
	map(0x00, 0x2f).m(m_vga, FUNC(wd90c31_vga_device::io_map));
}

void isa16_wd90c31a_lr_device::device_start()
{
	set_isa_device();

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "vga_rom");

	m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(wd90c31_vga_device::mem_r)), write8sm_delegate(*m_vga, FUNC(wd90c31_vga_device::mem_w)));
	m_isa->install_device(0x03b0, 0x03df, *this, &isa16_wd90c31a_lr_device::io_isa_map);
	m_isa->install_device(0x23c0, 0x23c7, *m_vga, &wd90c31_vga_device::ext_io_map);
}

/******************
 *
 * WD90C31A-ZS
 *
 *****************/

isa16_wd90c31a_zs_device::isa16_wd90c31a_zs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_WD90C31A_ZS, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_vga(*this, "vga")
{
}

ROM_START( wd90c31a_zs )
	ROM_REGION(0x8000,"vga_rom", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS(0, "wd90c31a_zs", "Western Digital WD90C31A-ZS")
	ROM_LOAD( "wd90c31azs.bin", 0x000000, 0x008000, CRC(521db1dd) SHA1(52cefcc9a1b7374c7414bb93a7805207ba5b7fe3) )
ROM_END

const tiny_rom_entry *isa16_wd90c31a_zs_device::device_rom_region() const
{
	return ROM_NAME( wd90c31a_zs );
}

void isa16_wd90c31a_zs_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update("vga", FUNC(wd90c00_vga_device::screen_update));

	WD90C31(config, m_vga, 0); // WD90C31A
	m_vga->set_screen("screen");
	// 512KB, 1MB
	m_vga->set_vram_size(0x100000);
}

void isa16_wd90c31a_zs_device::io_isa_map(address_map &map)
{
	map(0x00, 0x2f).m(m_vga, FUNC(wd90c31_vga_device::io_map));
}

void isa16_wd90c31a_zs_device::device_start()
{
	set_isa_device();

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "vga_rom");

	m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(wd90c31_vga_device::mem_r)), write8sm_delegate(*m_vga, FUNC(wd90c31_vga_device::mem_w)));
	m_isa->install_device(0x03b0, 0x03df, *this, &isa16_wd90c31a_zs_device::io_isa_map);
	m_isa->install_device(0x23c0, 0x23c7, *m_vga, &wd90c31_vga_device::ext_io_map);
}

/******************
 *
 * WD90C33-ZZ
 *
 *****************/

isa16_wd90c33_zz_device::isa16_wd90c33_zz_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_WD90C33_ZZ, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_vga(*this, "vga")
{
}

ROM_START( wd90c33_zz )
	ROM_REGION(0x8000,"vga_rom", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS(0, "wd90c33_zz", "Western Digital WD90C33-ZZ")
	ROM_LOAD( "bios.bin",     0x000000, 0x008000, CRC(b719d557) SHA1(0f93213ed439059d23e0793478793ffd6188ce12) )
	ROM_IGNORE( 0x8000 )
ROM_END

const tiny_rom_entry *isa16_wd90c33_zz_device::device_rom_region() const
{
	return ROM_NAME( wd90c33_zz );
}

void isa16_wd90c33_zz_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update("vga", FUNC(wd90c33_vga_device::screen_update));

	WD90C33(config, m_vga, 0);
	m_vga->set_screen("screen");
	// 1MB, 2MB
	m_vga->set_vram_size(0x100000);
}

void isa16_wd90c33_zz_device::io_isa_map(address_map &map)
{
	map(0x00, 0x2f).m(m_vga, FUNC(wd90c33_vga_device::io_map));
}

void isa16_wd90c33_zz_device::device_start()
{
	set_isa_device();

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "vga_rom");

	m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(wd90c33_vga_device::mem_r)), write8sm_delegate(*m_vga, FUNC(wd90c33_vga_device::mem_w)));
	m_isa->install_device(0x03b0, 0x03df, *this, &isa16_wd90c33_zz_device::io_isa_map);
	m_isa->install_device(0x23c0, 0x23cf, *m_vga, &wd90c33_vga_device::ext_io_map);
	m_isa->install_device(0x23d0, 0x23d3, *m_vga, &wd90c33_vga_device::localbus_if_map);
}
