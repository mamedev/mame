// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

nVidia NV10 "Celsius"
a.k.a. first gen GeForce

TODO:
- Compared to NV5 VGA controller extensively writes to extended register $40;
- All "elsa" BIOSes prompt user to "buy the card", copy protection related to EDID?
- geforce256 DDR "creative" BIOS draws garbage for RAM detection during POST;

**************************************************************************************************/

#include "emu.h"
#include "geforce.h"

DEFINE_DEVICE_TYPE(GEFORCE256,     geforce256_device,     "geforce256",    "nVidia GeForce 256 SDR (NV10)")
DEFINE_DEVICE_TYPE(GEFORCE256_DDR, geforce256_ddr_device, "geforce256ddr", "nVidia GeForce 256 DDR (NV10)")
//DEFINE_DEVICE_TYPE(GEFORCE256_ULTRA, geforce256_ultra_device, "geforce256ultra", "nVidia GeForce 256 Ultra (NV10)")
DEFINE_DEVICE_TYPE(QUADRO,         quadro_device,         "quadro",        "nVidia Quadro (NV10)")

geforce256_device::geforce256_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: rivatnt2_device(mconfig, type, tag, owner, clock)
{
}

geforce256_device::geforce256_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: geforce256_device(mconfig, GEFORCE256, tag, owner, clock)
{
	// 0x0100 GeForce 256 SDR
	// 0x0101 ^ DDR
	// 0x0102 ^ Ultra
	// 0x0103 Quadro
	// ELSA Synergy Force subvendor id
	set_ids_agp(0x10de0100, 0x00, 0x10480c48);
}


ROM_START( geforce256 )
	ROM_REGION32_LE( 0x10000, "vga_rom", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "elsa", "ELSA Synergy Force (Ver 7.03.00 01/11/00)" )
	ROMX_LOAD( "nv10.bin", 0x0000, 0xac00, BAD_DUMP CRC(3525816d) SHA1(6fef6773dd1b18148890f8c3d45f988e5e06b50d), ROM_BIOS(0) )
ROM_END

const tiny_rom_entry *geforce256_device::device_rom_region() const
{
	return ROM_NAME(geforce256);
}

void geforce256_device::device_start()
{
	pci_card_device::device_start();

	add_map( 16*1024*1024, M_MEM, FUNC(geforce256_device::mmio_map));
	add_map(128*1024*1024, M_MEM, FUNC(geforce256_device::vram_aperture_map));

	add_rom((u8 *)m_vga_rom->base(), 0x10000);
	expansion_rom_base = 0xc0000;

	// INTA#
	intr_pin = 1;
	save_item(NAME(m_vga_legacy_enable));
}

/********************************************/

geforce256_ddr_device::geforce256_ddr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: geforce256_device(mconfig, GEFORCE256_DDR, tag, owner, clock)
{
	// device ID 0x10de nVidia
	// TODO: Rev A6
	set_ids_agp(0x10de0101, 0x00, 0x10de0101);
}

ROM_START( geforce256_ddr )
	ROM_REGION32_LE( 0x10000, "vga_rom", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "creative", "Creative GeForce CT6971 AGP (Ver 2.10.4.02.04)" )
	ROMX_LOAD( "creative.rom", 0x000000, 0x009800, CRC(fd956e0b) SHA1(aaac61e99193221280c5913cf3ca503e5b5ea408), ROM_BIOS(0) )
ROM_END

const tiny_rom_entry *geforce256_ddr_device::device_rom_region() const
{
	return ROM_NAME(geforce256_ddr);
}

/********************************************/

// TODO: GeForce 256 Ultra
// set_ids_agp(0x10de0102, 0x00, 0x10de0102);

/********************************************/

quadro_device::quadro_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: geforce256_device(mconfig, QUADRO, tag, owner, clock)
{
	// device ID 0x10de nVidia
	set_ids_agp(0x10de0103, 0x00, 0x10de0103);
}

ROM_START( quadro )
	ROM_REGION32_LE( 0x10000, "vga_rom", ROMREGION_ERASEFF )
	// NB: difference between these two is type of RAM used (SDR vs. DDR) and
	//     interface type (AGP x4 vs. Pro)
	ROM_SYSTEM_BIOS( 0, "elsa",    "Elsa GLoria II (Ver 7.02.02)" )
	ROMX_LOAD( "elsagloriaii.bin", 0x000000, 0x00ac00, CRC(78a6fbad) SHA1(5517fefc314bd34de7839fb8077aeb252689595d), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "elsapro", "Elsa GLoria II Pro (Ver 7.02.00)" )
	ROMX_LOAD( "elsagloriaiipro.bin", 0x000000, 0x00ac00, CRC(b4ef6426) SHA1(006b72eb2bcb549686e4e39f8ca17972435178e4), ROM_BIOS(1) )
ROM_END

const tiny_rom_entry *quadro_device::device_rom_region() const
{
	return ROM_NAME(quadro);
}
