// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

nVidia NV4 TNT
nVidia NV5 TNT2

TODO:
- Pinpoint where "Vanta LT" and "RIVA TNT2 Pro" collocates;
- rivatnt2_ultra, rivatnt2_m64 and vanta enables external clock, failing $3da VRetrace checks next;
- rivatnt2_m64 inno3d/ct6980 BIOSes (at least) fails booting doing a i2c-like check to $36/$37
  (ddc?). This pattern is also seen in later Geforce 256 cards.

**************************************************************************************************/

#include "emu.h"
#include "rivatnt.h"

#define LOG_WARN      (1U << 1)
#define LOG_TODO      (1U << 2) // log unimplemented registers

#define VERBOSE (LOG_GENERAL | LOG_WARN | LOG_TODO)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)
#define LOGTODO(...)            LOGMASKED(LOG_TODO, __VA_ARGS__)


DEFINE_DEVICE_TYPE(RIVATNT,         rivatnt_device,         "rivatnt",       "nVidia Riva TNT (NV4)")
DEFINE_DEVICE_TYPE(RIVATNT2,        rivatnt2_device,        "rivatnt2",      "nVidia Riva TNT2 (NV5)")
DEFINE_DEVICE_TYPE(RIVATNT2_ULTRA,  rivatnt2_ultra_device,  "rivatnt2_ultra","nVidia Riva TNT2 Ultra (NV5)")
DEFINE_DEVICE_TYPE(VANTA,           vanta_device,           "vanta",         "nVidia Vanta (NV5)")
DEFINE_DEVICE_TYPE(RIVATNT2_M64,    rivatnt2_model64_device,"rivatnt2_m64",  "nVidia Riva TNT2 Model 64 (NV5)")


rivatnt_device::rivatnt_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: riva128_device(mconfig, type, tag, owner, clock)
{

}

rivatnt_device::rivatnt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: rivatnt_device(mconfig, RIVATNT, tag, owner, clock)
{
	// device ID 0x10de nVidia
	// TODO: revision (A5 for Creative card)
	// 0x11021016 for Creative Graphics Blaster RIVATNT
	// 0x1092xxxx for Diamond Viper V550
	// 0x10b427xx for STB
	set_ids_agp(0x10de0020, 0x00, 0x10de0020);
}

ROM_START( rivatnt )
	ROM_REGION32_LE( 0x8800, "vga_rom", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "creative", "Creative Graphics Blaster RIVA TNT (V2.04.6.18)" )
	ROMX_LOAD( "nv4_creative.rom", 0x0000, 0x8000, CRC(fe527a82) SHA1(2bb22bbaa7d2b61bce403f3163197abef85abdaa), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "stb", "STB Velocity 4400 (ver. 1.01)" )
	ROMX_LOAD( "nv4_stb_velocity.rom", 0x0000, 0x8000, CRC(d5251dc9) SHA1(eac26ac45e1cdcf020041586fb4216fd8166c2da), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "diamondb", "Diamond Viper V550 (Rev B, v1.95e)" )
	ROMX_LOAD( "nv4_diamond_revb.rom", 0x0000, 0x8800, CRC(8d860d99) SHA1(12b183cacc29cf8229da1b84b5f640f2f0722828), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "diamonda", "Diamond Viper V550 (Rev A, v1.93e)" )
	ROMX_LOAD( "nv4_diamond_reva.rom", 0x0000, 0x8800, CRC(333ca8e3) SHA1(602ba5812e608bb7275336e9abab4b822f7b1b98), ROM_BIOS(3) )
ROM_END

const tiny_rom_entry *rivatnt_device::device_rom_region() const
{
	return ROM_NAME(rivatnt);
}

// NV4 removes the indirect_io_map from NV3
void rivatnt_device::device_start()
{
	pci_card_device::device_start();

	add_map( 16*1024*1024, M_MEM, FUNC(rivatnt_device::mmio_map));
	add_map(128*1024*1024, M_MEM, FUNC(rivatnt_device::vram_aperture_map));

	add_rom((u8 *)m_vga_rom->base(), 0x8000);
	expansion_rom_base = 0xc0000;

	// INTA#
	intr_pin = 1;
	save_item(NAME(m_vga_legacy_enable));
}

/********************************************
 *
 * Riva TNT2 overrides
 *
 *******************************************/

rivatnt2_device::rivatnt2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: rivatnt_device(mconfig, type, tag, owner, clock)
{
}

rivatnt2_device::rivatnt2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: rivatnt2_device(mconfig, RIVATNT2, tag, owner, clock)
{
	// device ID 0x10de nVidia
	set_ids_agp(0x10de0028, 0x00, 0x10de0028);
}

ROM_START( rivatnt2 )
	ROM_REGION32_LE( 0x10000, "vga_rom", ROMREGION_ERASEFF )
	// Despite the versioning it claims to be (C) 1999-2001
	ROM_SYSTEM_BIOS( 0, "leadtek",  "Leadtek WinFast 3D S320 II (V99.04.12)" )
	ROMX_LOAD( "rivatnt2leadteks320ii.bin", 0x000000, 0x010000, CRC(f1379a55) SHA1(1ea0216df1fee9ee1bcd66c26adafbbdb6a0f1f1), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "diamond",  "Diamond Viper V770 (202L4A00)")
	ROMX_LOAD( "nv5diamond.bin", 0x000000, 0x009c00, CRC(ac6c48ff) SHA1(709632adf1e156fab3af0723b73a1e6c2c39e987), ROM_BIOS(1) )
ROM_END

const tiny_rom_entry *rivatnt2_device::device_rom_region() const
{
	return ROM_NAME(rivatnt2);
}

// All NV5 definitely wants 64K ROM BIOS (would fail CRC checks)
void rivatnt2_device::device_start()
{
	pci_card_device::device_start();

	add_map( 16*1024*1024, M_MEM, FUNC(rivatnt2_device::mmio_map));
	add_map(128*1024*1024, M_MEM, FUNC(rivatnt2_device::vram_aperture_map));

	add_rom((u8 *)m_vga_rom->base(), 0x10000);
	expansion_rom_base = 0xc0000;

	// INTA#
	intr_pin = 1;
	save_item(NAME(m_vga_legacy_enable));
}

/********************************************/

rivatnt2_ultra_device::rivatnt2_ultra_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: rivatnt2_device(mconfig, RIVATNT2_ULTRA, tag, owner, clock)
{
	// device ID 0x10de nVidia
	set_ids_agp(0x10de0029, 0x00, 0x10de0029);
}

ROM_START( rivatnt2_ultra )
	ROM_REGION32_LE( 0x10000, "vga_rom", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "creative",  "Creative 3D Blaster RIVA TNT2 Ultra AGP" )
	ROMX_LOAD( "creative.bin", 0x000000, 0x010000, CRC(cf3828ce) SHA1(733d8c3f179f89f0e238ffb95717c0fafd1053aa), ROM_BIOS(0) )
ROM_END

const tiny_rom_entry *rivatnt2_ultra_device::device_rom_region() const
{
	return ROM_NAME(rivatnt2_ultra);
}

/********************************************/

vanta_device::vanta_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: rivatnt2_device(mconfig, VANTA, tag, owner, clock)
{
	// device ID 0x10de nVidia
	set_ids_agp(0x10de002c, 0x00, 0x10de002c);
}

// TODO: None of the Vanta BIOSes returns a real vendor inside, confirm OEM card names
ROM_START( vanta )
	ROM_REGION32_LE( 0x10000, "vga_rom", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "inno3d",  "InnoVISION Inno3D Vanta (V2.05.13)" )
	ROMX_LOAD( "inno3d_nv5.bin", 0x000000, 0x00a800, CRC(9574114f) SHA1(f0e684bca6cc9c1e51a91dccdd5f216b16ea4fe1), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "pine",    "Pine Vanta (V2.05.17.04.00)" )
	ROMX_LOAD( "pine_pci_nv5.rom", 0x000000, 0x00a800, CRC(9c24202a) SHA1(417644f744b2cd54c0b81e8982e81336907f355e), ROM_BIOS(1) )
	// "Vanta LT"?
	ROM_SYSTEM_BIOS( 2, "compaq",  "Compaq Vanta-16 (V3.05.00.10.45)" )
	ROMX_LOAD( "nv5_compaq_vantalt.rom", 0x000000, 0x009800, CRC(64800741) SHA1(b28a56bcf466fcd9308541acaa1c7c94f493149a), ROM_BIOS(2) )
ROM_END

const tiny_rom_entry *vanta_device::device_rom_region() const
{
	return ROM_NAME(vanta);
}

/********************************************/

rivatnt2_model64_device::rivatnt2_model64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: rivatnt2_device(mconfig, RIVATNT2_M64, tag, owner, clock)
{
	// device ID 0x10de nVidia
	set_ids_agp(0x10de002d, 0x00, 0x10de002d);
}

ROM_START( rivatnt2_m64 )
	ROM_REGION32_LE( 0x20000, "vga_rom", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "asus",     "ASUS AGP-V3800M (V3.05.00.10.31)" )
	ROMX_LOAD( "asus_125-143mhz32mb.rom", 0x000000, 0x00b000, CRC(be95dc0c) SHA1(35c367b4f9d2db1bfca6d829744d9ffbd2db6e8a), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "ct6980",  "Creative CT6984 AGP (V2.05.4.17.03)" )
	ROMX_LOAD( "creativect6980_125-125mhz.rom", 0x000000, 0x009400, CRC(6a50c279) SHA1(a81105745cc1b80c720a7e3b3216507d61e4e120), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "inno3d",  "InnoVISION Inno3D TNT2 M64 (V2.05.13)" )
	ROMX_LOAD( "inno3dpci_125-125mhz.bin", 0x000000, 0x010000, CRC(08ba7718) SHA1(479cc1cb12bdddc87174c71382b55f932a9a9610), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "cm64a",   "PowerColor CM64A (V2.05.13)" )
	ROMX_LOAD( "cm64a_120-125mhz.rom", 0x000000, 0x00a800, CRC(d087e5fa) SHA1(230475f5385feda97c29d52723ec96bad2be6e08), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "leadtek",  "Leadtek 16MB AGP (V3.05.00.10.56)" )
	ROMX_LOAD( "leadtek16mb_125-143mhz.rom", 0x000000, 0x009800, CRC(71e060e5) SHA1(4057e940ab2bc0bdc3a286dfc71bcb50342fce13), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "pine",     "Pine VARM645PS (V3.05.00.10.00)" )
	ROMX_LOAD( "pine_pv-t02a-br_125-143mhz.rom", 0x000000, 0x00b000, CRC(0212415b) SHA1(3c34c9bc7e8d521d6d8c8f51959b75378a73eef7), ROM_BIOS(5) )
	// TODO: unsure about the card name for these
	ROM_SYSTEM_BIOS( 6, "manli",    "Manli RIVA TNT2 M64 (V2.05.19.03.00)" )
	ROMX_LOAD( "manli_135-135mhz.rom", 0x000000, 0x00a800, CRC(d182f4d4) SHA1(e8615cceb3fd3c1bffdb2987313729b367b2deef), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS( 7, "msi",      "MSI RIVA TNT2 M64 (V2.05.20.02.25)" )
	ROMX_LOAD( "msi_ms-8808_125-150mhz.rom", 0x000000, 0x00a800, CRC(f204caa2) SHA1(7642f27235754d9a8d64e799a6b20017444b7722), ROM_BIOS(7) )
	ROM_SYSTEM_BIOS( 8, "sparkle",  "Sparkle RIVA TNT2 M64 (V3.05.00.10.00)" )
	ROMX_LOAD( "sparkle_sp5300_125-125mhz.rom", 0x000000, 0x00b400, CRC(518e9e79) SHA1(f6232418e0dd08f04ac5c6ace4e0d4ec06290d48), ROM_BIOS(8) )
	// Originally posted in misc/ez2d.cpp
	ROM_SYSTEM_BIOS( 9, "unknown",  "nVidia RIVA TNT2 M64 (V2.05.20.02.80)" )
	ROMX_LOAD( "62090211.rom", 0x000000, 0x00b000, CRC(5669135b) SHA1(b704ce0d20b71e40563d12bcc45bd1240227be74), ROM_BIOS(9) )
	// Provided inside ez2dm HDD image
	// "V10.26.19.99"
	ROM_SYSTEM_BIOS( 10, "winfast_oem",  "Leadtek WinFast 3D 325 OEM (V4.8)" )
	ROMX_LOAD( "w2137.rom", 0x000000, 0x010000, CRC(3b7986ff) SHA1(529a79296c914a2dcdd568221b9aac914869bade), ROM_BIOS(10) )

	// following is just an alias of "inno3dpci_125-125mhz.bin" with empty data in the 0x10000-0x1ffff bank (likely inaccessible too)
	// rivatnt2m64inno3d.BIN                                  1xxxxxxxxxxxxxxxx = 0x00
	// inno3dpci_125-125mhz.bin            rivatnt2m64inno3d.BIN [1/2]      IDENTICAL
//  ROMX_LOAD( "rivatnt2m64inno3d.bin", 0x000000, 0x020000, CRC(7241c671) SHA1(24f6f1fbcd3d42ec354185697f8e856d876e2a50), ROM_BIOS(0) )

//  "rivatnt2m64.bin" is another alias
//  inno3dpci_125-125mhz.bin            rivatnt2m64.BIN            IDENTICAL
ROM_END

const tiny_rom_entry *rivatnt2_model64_device::device_rom_region() const
{
	return ROM_NAME(rivatnt2_m64);
}
