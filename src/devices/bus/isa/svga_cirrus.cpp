// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/***************************************************************************

  ISA SVGA Cirrus Logic wrapper

***************************************************************************/

#include "emu.h"
#include "svga_cirrus.h"

#include "screen.h"

// no ROMs were found for the Cirrus Logic CL-GD410/420, CL-GD6410, CL-GD6420, CL-GD6235, CL-GD6440, CL-GD7543, CL-GD7548, CL-GD5480
// *.vbi dumps were done with the NSSI utility

ROM_START( dm_clgd5430 )
	// Diamond Speedstar Pro SE V1.00 - 8bit VL card - Chip: CL-GD5430-446AM - OSC: 14.31818MHz - RAM: 1MB - Connector: DB15 - VESA feature connector
	ROM_REGION(0x8000,"dm_clgd5430", 0)
	ROM_LOAD("speedstar_pro_se_v1.00.u2", 0x00000, 0x8000, CRC(ed79572c) SHA1(15131e2b2db7a34971083a250e4a21ab7bd64a9d) )
	ROM_IGNORE( 0x8000 )
ROM_END

// unemulated Chipsets from the CL-GD5430 onwards

/*
    Cirrus Logic CL-GD543x - ISA cards

// Diamond Speedstar 64 ISA Rev. A3A - Chip: CL-GD5434-J-QC-F - RAM: 2MB - ROM: 32KB V2.02 - OSC: 14.31818MHz - Connector: DB15 - VESA feature connector
// a v2.01 exists as well
ROM_START( diass64 )
    ROM_REGION(0x08000, "clgd543x_isa", 0)
    ROM_LOAD("diamond_multimedia_speedstar64_v2.02.bin", 0x00000, 0x08000, CRC(7423bca7) SHA1(20bf727218688f8ecd4e7a1607ee5c6260ee01eb) )
ROM_END

// STB Nitro 64 ISA - ROM: CL-GD543x VGA BIOS Version 1.22 (a 1.1 exists) - RAM: 1MB, 2MB - Chip: Cirrus Logic CL-GD5434 - Connector: DB15 - VESA feature connector - OSC: 14.31818 MHz
ROM_START( nitro64 )
    ROM_REGION(0x08000, "clgd543x_isa", 0)
    ROM_LOAD("nitro64.vbi", "0x00000, 0x8000, CRC(269704bb), SHA1(8a7f5c3c107c3300a82a8cb42e19f9411d8ca0c1) )
ROM_END

*/

/*
    Cirrus Logic CL-GD543x - VLB cards

Source: ROM dump - MVA-CL5434 8bit VLB VGA card - Chips: CL-GD5434-HC-D - ROM: VL R1.10B, CKS: 6400H - OSC: 14.31818 - Connector: DB15 - VESA feature connector
ROM_START( clgd543x_vlb )
    ROM_REGION(0x10000, "clgd543x_vlb", 0)
    ROM_SYSTEM_BIOS(0, "mvacl5434", "MVA-CL5434 8bit VLB VGA card")
    ROMX_LOAD("mva_clgd5434_27c512.bin", 0x00000, 0x10000, CRC(811aecef) SHA1(0a65d2d3640936492d2f2a4ac8d07897343b15ef), ROM_BIOS(0) )
    Kelvin 64 / EZ - Source: EPROM dump, Pulled from a non-working card - ROM: CL-GD543x VGA BIOS Version 1.00a - OSC: 14.31818 MHz
    ROM_SYSTEM_BIOS(1, "kel64ez", "Orchid Kelvin 64 EZ VLB")
    ROMX_LOAD("ogvlb.bin", 0x08000, 0x8000, CRC(16af65bb) SHA1(16db72ebc67f5dea81418cf25e562eb6da00218c), ROM_BIOS(1) )
    // Orchid Kelvin 64 MPEG - 16bit VLB - Chips: Cirrus Logic CL-GD5434, Alliance ProMotion-6410, Analog Devices ADSP-2105 KP-40 ED/AA0888-1.1, MUSIC MU9C4870-80DC
    // ROM: Kelvin 64-VLB BIOS Ver 1.10B - OSC: 14.318180 Mhz, 55.000 MHz, unreadable - Connectors: DB15, 2x3.5mm jacks - Potentiometer on rear bracket
    ROM_SYSTEM_BIOS(2, "kelmpeg", "Orchid Kelvin MPEG Version 1.10B")
    ROMX_LOAD("kelvin64_mpeg.bin", 0x00000, 0x10000, CRC(403020a4) SHA1(1ee70f6cc3c7b9a9afb402a1bc939f9f2e0cc739), ROM_BIOS(2) )
ROM_END

*/


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA16_SVGA_CIRRUS,        isa16_svga_cirrus_device,        "dm_clgd5430", "Diamond Speedstar Pro SE ISA Graphics Card (BIOS v1.00)")
DEFINE_DEVICE_TYPE(ISA16_SVGA_CIRRUS_GD542X, isa16_svga_cirrus_gd542x_device, "clgd542x",    "Generic Cirrus Logic CD542 Graphics Card (BIOS v1.20)")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa16_svga_cirrus_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update("vga", FUNC(cirrus_gd5430_device::screen_update));

	CIRRUS_GD5430(config, m_vga, 0);
	m_vga->set_screen("screen");
	m_vga->set_vram_size(0x200000);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *isa16_svga_cirrus_device::device_rom_region() const
{
	return ROM_NAME( dm_clgd5430 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa16_vga_device - constructor
//-------------------------------------------------

isa16_svga_cirrus_device::isa16_svga_cirrus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_SVGA_CIRRUS, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_vga(*this, "vga")
{
}

void isa16_svga_cirrus_device::io_isa_map(address_map &map)
{
	map(0x00, 0x2f).m(m_vga, FUNC(cirrus_gd5430_device::io_map));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
uint8_t isa16_svga_cirrus_device::input_port_0_r() { return 0xff; } //return machine().root_device().ioport("IN0")->read(); }

void isa16_svga_cirrus_device::device_start()
{
	set_isa_device();

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "dm_clgd5430");

	m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(cirrus_gd5430_device::mem_r)), write8sm_delegate(*m_vga, FUNC(cirrus_gd5430_device::mem_w)));
	m_isa->install_device(0x03b0, 0x03df, *this, &isa16_svga_cirrus_device::io_isa_map);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa16_svga_cirrus_device::device_reset()
{
}

// unemulated chipsets up to the CL-GD542x

/*
    Cirrus Logic CL-GD510/520


ROM_START( clgd510 )
    // Source: ROM dump - 8bit ISA card - Chips: Cirrus Logic CL-GD520A-40PC-A, GL-GD510A-32PC-A, UMBC82C11 - OSC: 32.514MhZ, 25.175MHz, 28.322MHz
    // ROM: VGA/P-II 27256 U24 VER 2.00 - GAL: VGA/P(I.II) R0 U32 16L8A, VGA/P(I.II) R0 U6 16L8A - Connectors: DB15, DB9 - DIP: 8-way
    ROM_REGION(0x08000, "clgd510", 0)
    ROM_SYSTEM_BIOS(0, "eaglevgapii_v2.00", "Eagle VGA-PII V2.00")
    ROMX_LOAD("eagle_ii_vga_bios_vga_p-ii_27256_u24_ver2.00.bin", 0x00000, 0x08000, CRC(90b7e179) SHA1(d22988c3d31ba457c24ef9b798baf41dd3017b61), ROM_BIOS(0) )
    ROM_SYSTEM_BIOS(1, "eaglevgapii_v2.12", "Eagle VGA-PII V2.12")
    ROMX_LOAD("gd510520.vbi", 0x00000, 0x08000, CRC(750b13db) SHA1(4c8bbf5e5d88988d48ad708fe794d5bb2aa6d8a1), ROM_BIOS(1) )
    ROM_SYSTEM_BIOS(2, "morse_v.220", "Morse KP 800/16 V2.20")
    ROMX_LOAD("kp800-16.vbi", 0x00000, 0x08000, CRC(7ec61c15) SHA1(1deeba9dcfa920952c92b0e44752dd169f4a145c), ROM_BIOS(2) )
    // Morse KP 800 VGA - 8bit ISA VGA card - Source: EPROM dump - Chips: CL-GD520A-32PC-B, CL-GD510, PAL18L8ACN, Inmos A B171S-35C - ROM: Eagle II VGA BIOS Version 2.12
    // RAM: 256KB - OSC: 28.322000 MHz, 32.514000 MHz, 25.175000MHz - Connectors: DB15, DB9 - DIP: 8-way
    ROM_SYSTEM_BIOS(3, "morse_v212", "Morse KP 800 V2.12")
    ROMX_LOAD("morse_kp800_vga_cl-gd520a-32pc-b.bin", 0x00000, 0x08000, CRC(9a12e070) SHA1(536046c6c2549d8cc16a546d5ec1739d647486d7), ROM_BIOS(3) )
    // Videoseven VEGA VGA 8bit ISA graphics card - RAM: 256KB - Chips: Cirrus Logic CL-GD540A-32PC-C, AM81C178-50PC, CL-GD510A-32PC-B -
    // Connectors: DB9, DB15, 32pin, 6pin - DIP: 6 way
    ROM_SYSTEM_BIOS(4, "v7vegavga147", "Videoseven VEGA VGA Version 1.47")
    ROMX_LOAD("v7_vega_vga_62L1989V5_435-0016-47.bin", 0x00000, 0x08000, CRC(79daa514) SHA1(cc3fc6bf54ba2668ae0083f917e91beb78894e30), ROM_BIOS(4) )
    ROM_SYSTEM_BIOS(5, "v7vegavga178", "Videoseven VEGA VGA Version 1.78")
    ROMX_LOAD("vegavga.vbi", 0x00000, 0x08000, CRC(438c6790) SHA1(16abdc1bc3cd38f13ce16dc11fa4e99f169b9807), ROM_BIOS(5) )
ROM_END

*/

/*
    Cirrus Logic CL-GD610/620 flat panel controller

// KS-750V/3100V 16bit ISA LCD display card Ver:A3 - Chips: Inmos 99113-D IMSG171P-50G, CL-GD620-32QC-C, CL-GD610-320C-C - OSC: 25.175000 MHz, 32.514 MHz, 28.322MHz, 24.000MHz
// Connectors: internal, DB9, DB15 - RAM: 256KB - DIP: 8 way
ROM_START( clgd610 )
    ROM_REGION(0x08000, "clgd610", 0)
    ROM_LOAD("ks-750v.vbi", 0x00000, 0x08000, CRC(1bc7cd1d) SHA1(fdc29cc261b0efbfeec21b0c0f68d79323cf8604) )
ROM_END

*/

/*
    Cirrus Logic CL-GD5320

//  CVGA-V3-256 16bit ISA Cirrus Logic CL-GD5320 card - Chips: CL-GD5320-36QC-B, RAMDAC JT82C176-66 - Connector: DB15 - OSC: 25.175 MHz, 28.322 MHz, 36.000 MHz, 40.000 MHz, RAM: 256KB
ROM_START( clgd5320 )
    ROM_REGION(0x08000, "clgd532", 0)
    ROM_LOAD("5320.vbi", 0x00000, 0x08000, CRC(633ad9c6) SHA1(bc66f77b819ea00065216dbf94d4bf1f142f95c9) )
ROM_END

*/

/*
    Cirrus Logic  CL-GD5401  - Acumos AVGA1

ROM_START( clgd5401 )
    ROM_REGION(0x08000, "clgd5401", 0)
    // ROM: originally even/odd ROMs marked VA30 Ver 1.00 (c) 1991 EIZO Corp - RAM: 256KB - Connector: DB15 - VESA feature connector - Chip: AVGA1
    ROM_SYSTEM_BIOS(0, "eizova30", "Eizo VA30 V1.00")
    ROMX_LOAD("eizova30.vbi", 0x02000, 0x06000, CRC(17c2c190) SHA1(081ec7dea9aff0e4577892909802d6e9a620dcf7), ROM_BIOS(0) )
    //Acumos AVGA1 16bit ISA card - ROM: VGA1 BIOS Ver B 1.00 07 321-01 9210S - RAM: 256KB - Chip: acumos AVGA1 320-01 PC2561TK 9210 - Connector: DB15 - 26pin Edge connector
    ROM_SYSTEM_BIOS(1, "version_b", "Version B")
    ROMX_LOAD("32001withlogo.vbi", 0x02000, 0x06000, CRC(5619d4a9) SHA1(4f1b1f78c363e004124715c980b996bd1dcc54af), ROM_BIOS(1) )
    ROM_SYSTEM_BIOS(2, "version_d", "Version D")
    ROMX_LOAD("nousa.bin", 0x00000, 0x08000, CRC(797f0767) SHA1(a318367d385d7185cdd3d73838677f52c4a8cefb), ROM_BIOS(2) )
    ROM_SYSTEM_BIOS(3, "vga1_v1.00_07_d", "VGA1 BIOS v1.00 07 D"
    ROMX_LOAD("avga1.bin", 0x00000, 0x08000, CRC(6722ee8b) SHA1(1c90327888e803705e61a4d49629720f4ee9b7a5), ROM_SKIP(1) | ROM_BIOS(3) )
    ROM_CONTINUE(                                   0x00001, 0x04000 )
    ROM_SYSTEM_BIOS(4, "nover", "No revision shown"
    ROMX_LOAD("avga1.rom", 0x00000, 0x08000 CRC(0f562e1f) SHA1(8a0808fe0bc31ecd51ffa1a0a31902da0e9541d4), ROM_BIOS(4) )
    // ISA16 - Chip: CL-GD5401-42QC-B - RAM: 256KB - MicroMax M5401 V321-01 REV. 3  MADE IN U.S.A. - FCC ID: IWLVGAADAPTER 1V1
    ROM_SYSTEM_BIOS(5, "micromax", "MicroMax M5401 V321-01 rev.3")
    ROMX_LOAD("cl5401.bin", 0x00000, 0x08000, CRC(a40551d6) SHA1(db38190f06e4af2c2d59ae310e65883bb16cd3d6), ROM_SKIP(1) | ROM_BIOS(5) )
    ROM_CONTINUE(                                   0x00001, 0x04000 )
ROM_END

*/

/*
    Cirrus Logic CL-GD5402 / Acumos AVGA2

// 16bit ISA graphics card - Chips: acumos AVGA2 340-01 10908 9228 - RAM: 512KB - Connector: DB15 - VESA feature connector

ROM_START( clgd5402 )
    ROM_REGION(0x10000, "clgd5402", 0)
    ROM_LOAD16_BYTE("avga2vram.vbi", 0x08000, 0x10000, CRC(ef4fc687) SHA1(8be0fe065c652d6a39d742df8f217c914574b701) )
ROM_END

*/


/*
    Generic CL-GD542x video card
*/

ROM_START( clgd542x )
	ROM_REGION(0x08000, "clgd542x", 0)
	ROM_SYSTEM_BIOS(0, "techyosd_v1.2", "techyosd-isa-bios-v1.2")
	ROMX_LOAD("techyosd-isa-bios-v1.2.u8",          0x00000, 0x04000, CRC(6adf7e71) SHA1(2b07d964cc7c2c0aa560625b7c12f38d4537d652), ROM_SKIP(1) | ROM_BIOS(0) )
	ROM_CONTINUE(                                   0x00001, 0x04000 )
	// Source: ROM Dump, the chip is marked TECHYOSD VL-BIOS V1.2 9323
	// From an 8-Bit VL-Bus VGA Card GT M 94V-0 using the Cirrus Logic CL-GD5426-800C-A chip - 1 MB RAM - OSC 14.31818 - VESA feature connector, DB15 connector
	ROM_SYSTEM_BIOS(1, "techyosd_v1.2_a", "techyosd-isa-bios-v1.2 alt. dump")
	ROMX_LOAD("cl_gt_m_94v-0.bin",                  0x00000, 0x04000, CRC(9eea184a) SHA1(b70ff57405830110c3ccc5313bcd736c8dcd9b81), ROM_SKIP(1) | ROM_BIOS(1) )
	ROM_CONTINUE(                                   0x00001, 0x04000 )
	// Boca Research BRI4610 - ISA16 VGA card, Chips: CL-GD5428-800C-A, RAM: 1MB, Bios Version 1.30B, VESA feature connector, DB15 connector
	ROM_SYSTEM_BIOS(2, "boca_v1.3", "Boca Research BRI4610 v1.3")
	ROMX_LOAD("boca_gd5428_1.30b.bin",              0x00000, 0x04000, CRC(6cc8f318) SHA1(2cac4bf2537626cff41aa81ad4627ec6c40dcf99), ROM_SKIP(1) | ROM_BIOS(2) )
	ROM_CONTINUE(                                   0x00001, 0x04000 )
	ROM_SYSTEM_BIOS(3, "bri4610_v141a", "Boca Research BRI4610 V1.41a")
	ROMX_LOAD("isamalaysia.bin" , 0x00000, 0x8000, CRC(b15804d0) SHA1(9778320ec1560e78aa978a4ef21e098535cb0fcf), ROM_BIOS(3) )
	// Boca Research BRI4611 - ISA16 VGA card - Chip: CL-GD5422-80QC-C - ROM: CL-GD540X/542X VGA BIOS Version 1.00d - RAM: 1MB - Connector: DB15 - VESA feature connector
	ROM_SYSTEM_BIOS(4, "bri4611", "Boca Research BRI4611")
	ROMX_LOAD("gd5422_boca.vbi", 0x00000, 0x08000, CRC(dd1de1c2) SHA1(548e84e412563e4dc1c12736a8d355bf91aaa3b9), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS(5, "ver_1.00d", "Ver. 1.00d")
	ROMX_LOAD("cl-gd5422_bios_rom_ver_1.00d.bin",   0x00000, 0x04000, CRC(c63b1eb8) SHA1(01782fdef5c22fa6418bc881ee9781bf273c389b), ROM_SKIP(1) | ROM_BIOS(5) )
	ROM_CONTINUE(                                   0x00001, 0x04000 )
	// Diamond Speedstar PRO - Chipset: CL-GD5426-80QC-B - ROM: Ver 2.04 - RAM: 1MB - Connector: DB15 - VESA feature connectorVL card
	ROM_SYSTEM_BIOS(6, "speedstar_pro", "Diamond Speedstar PRO")
	ROMX_LOAD("diamond5426_v204.vbi", 0x00000, 0x08000, CRC(c98491b0) SHA1(19ae9d2bf39e5b2726d6d556ef2cfa84b06d4da9), ROM_BIOS(6) )
	// ISA16 - Chip: CL-GD5424-80QC-GW - RAM: 512KB
	ROM_SYSTEM_BIOS(7, "5424_ver_1.41", "5424 Ver 1.41")
	ROMX_LOAD("cl5424.bin", 0x0000, 0x04000, CRC(c3b0239a) SHA1(6327b98b26f481f993ebbd0dc18410e4fb5b3aad), ROM_SKIP(1) | ROM_BIOS(7) )
	ROM_CONTINUE(                                   0x00001, 0x04000 )
	ROM_IGNORE(0x8000)
	// UTD-01D CL542X - VLB - Chip: Cirrus Logic CL-GD5428-80QC-A - RAM: 1MB
	ROM_SYSTEM_BIOS(8, "5428_ver_1.41", "5428 Ver 1.41")
	ROMX_LOAD("cl5428.bin", 0x00000, 0x04000, CRC(baba9c04) SHA1(2b7a426e060f656c401586a1a6c1183edbf464a5), ROM_SKIP(1) | ROM_BIOS(8) )
	ROM_CONTINUE(                                   0x00001, 0x04000 )
	ROM_SYSTEM_BIOS(9, "cl5429_v1.00a", "CL-GD5429 Ver.1.00a")
	ROMX_LOAD("5429.vbi", 0x00000, 0x08000, CRC(a2d81c5b) SHA1(8060cd4b49052572bb5ec929f8ad21e2addce237), ROM_BIOS(9) )
	// VLB - CL5429 SOJ/SMT/4L/V1 - ROM: CL-GD5906-20DC-1.00 - Chipset: Cirrus Logic CL-GD5429-86QC-B - RAM: 1MB, max. 2MB
	ROM_SYSTEM_BIOS(10, "20dc-1.00", "CL-GD5906-20DC-1.00")
	ROMX_LOAD("cl5429.bin", 0x00000, 0x8000, CRC(c4af3673) SHA1(a987c6047141b3d478aeccccd07936210ddb374b), ROM_BIOS(10) )
	// Source: ROM dump - NCR 8bit VLB VGA card S17-0000248 - Chip: CL-GD5428-80QC-A - ROM: CL-GD5422/5424/5426/5428 VGA BIOS Version 1.3 - RAM: 1MB - Connector: DB15 - VESA feature connector
	ROM_SYSTEM_BIOS(11, "ncr_5428_v130", "NCR 5428 VLB V1.30")
	ROMX_LOAD("ncr_vlb_vga_cl-gd5428-80qc-a.bin", 0x00000, 0x08000, CRC(2d68bfb7) SHA1(f0e385e4d2c4a417ed21f10bf340ad6338b768bd), ROM_BIOS(11) )
	// Machspeed VGA GUI 2100 - 16bit VLB Cirrus Logic CL-GD5428 based VGA card - ROM: CL-GD542X VGA BIOS Version 1.41 - Chips: MACHSPEED GTK6189 9408 CL-GD5428-80QC-A
	// RAM: 1MB max. 2MB - Connectors: DB15 -  VESA feature connector
	ROM_SYSTEM_BIOS(12, "machspeed2100", "Machspeed VGA GUI 2100 V1.41a")
	ROMX_LOAD("machspeed_vga_gui_2100_vlb.vbi", 0x00000, 0x08000, CRC(da3b3261) SHA1(a09ceea0d8036e13ec501dd6cc32bca16f2f9204), ROM_BIOS(12) )
	// Octek VL-Combo rev.3.2 - This is a Cirrus Logic CL-GD5426 based VGA card that includes 2 serial ports, a parallel port, a floppy controller and an IDE HDD controller.
	// Chips: CL-GD5426, SMC FDC37C652QF, PROMISE PDC20230C
	// The 64KB original dump includes two identical 32KB halves with 16KB "quarters" interleaved in each.
	// The rev.3.3 card uses the same BIOS but a CL-GD5428
	ROM_SYSTEM_BIOS(13, "octec_vlcombo3.2", "Octek VL-Combo rev.3.2")
	ROMX_LOAD("octek_vl-combo_rev3_2_27256.bin", 0x00000, 0x04000, CRC(0dbabb0a) SHA1(693c384abf42420276a2727ec4a4c1792704eb30), ROM_SKIP(1) | ROM_BIOS(13) )
	ROM_CONTINUE(                                   0x00001, 0x04000 )
	// Octek VL-Combo rev.2.1 - PDC20230-B IDE-controller
	// Software dump using the MESS method, the Promise HDD-controller part is dumped from D800, the 32K from C800 are FF
	ROM_SYSTEM_BIOS(14, "octec_vlcombo2.1", "Octek VL-Combo rev.2.1")
	ROMX_LOAD("octek_vl_combo_v2.1_myc000.bin", 0x00000, 0x08000, CRC(1a97b6d5) SHA1(fde41c45347b66ed4b1f99c3f7057e7411cdf4da), ROM_BIOS(14) )
	// ROM_LOAD("octek_vl_combo_v2.1_myd800.bin", 0x00000, 0x02000, CRC(2c472bfe) SHA1(ab4879924f5826c38f6066ac769106584ac2d2e8) ) // HDD
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa16_svga_cirrus_gd542x_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update("vga", FUNC(cirrus_gd5428_device::screen_update));

	CIRRUS_GD5428(config, m_vga, 0);
	m_vga->set_screen("screen");
	m_vga->set_vram_size(0x200000);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *isa16_svga_cirrus_gd542x_device::device_rom_region() const
{
	return ROM_NAME( clgd542x );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa16_vga_device - constructor
//-------------------------------------------------

isa16_svga_cirrus_gd542x_device::isa16_svga_cirrus_gd542x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_SVGA_CIRRUS_GD542X, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_vga(*this, "vga")
{
}

void isa16_svga_cirrus_gd542x_device::io_isa_map(address_map &map)
{
	map(0x00, 0x2f).m(m_vga, FUNC(cirrus_gd5428_device::io_map));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
uint8_t isa16_svga_cirrus_gd542x_device::input_port_0_r() { return 0xff; } //return machine().root_device().ioport("IN0")->read(); }

void isa16_svga_cirrus_gd542x_device::device_start()
{
	set_isa_device();

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "clgd542x");

	m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(cirrus_gd5428_device::mem_r)), write8sm_delegate(*m_vga, FUNC(cirrus_gd5428_device::mem_w)));
	m_isa->install_device(0x03b0, 0x03df, *this, &isa16_svga_cirrus_gd542x_device::io_isa_map);
}

void isa16_svga_cirrus_gd542x_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(cirrus_gd5428_device::mem_r)), write8sm_delegate(*m_vga, FUNC(cirrus_gd5428_device::mem_w)));
		m_isa->install_rom(this, 0xc0000, 0xc7fff, "clgd542x");
	}
	else if (space_id == AS_IO)
		m_isa->install_device(0x03b0, 0x03df, *this, &isa16_svga_cirrus_gd542x_device::io_isa_map);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa16_svga_cirrus_gd542x_device::device_reset()
{
}
