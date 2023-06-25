// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

nVidia NV4 TNT

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


DEFINE_DEVICE_TYPE(RIVATNT,  rivatnt_device,  "rivatnt", "nVidia Riva TNT (NV4)")

rivatnt_device::rivatnt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: riva128_device(mconfig, RIVATNT, tag, owner, clock)
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
	pci_device::device_start();

	add_map( 16*1024*1024, M_MEM, FUNC(rivatnt_device::mmio_map));
	add_map(128*1024*1024, M_MEM, FUNC(rivatnt_device::vram_aperture_map));

	add_rom((u8 *)m_vga_rom->base(), 0x8000);
	expansion_rom_base = 0xc0000;

	// INTA#
	intr_pin = 1;
	save_item(NAME(m_vga_legacy_enable));
}
