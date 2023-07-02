// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/*
 * Sandbox for SiS based x86 PCs, targeting the new PCI model
 *
 * Notes:
 * - sis85c471 doesn't belong here, it's a full on ISA PC/AT
 *
 * TODO:
 * - Finish porting sis85c496 from at.cpp;
 * - Reports slower CPU speeds in Award BIOSes (i.e. "66 MHz" for actual 75);
 * - (Hack Inc.) Glue in a Voodoo 1 hookup;
 * - (Hack Inc.) Identify motherboard name(s);
 *
 */

#include "emu.h"
#include "bus/isa/isa_cards.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"
#include "machine/sis85c496.h"
#include "video/voodoo_pci.h"

class sis496_state : public driver_device
{
public:
	sis496_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void sis496(machine_config &config);

protected:
	required_device<i486dx4_device> m_maincpu;
private:
	void main_io(address_map &map);
	void main_map(address_map &map);
};

#define PCI_ID_VIDEO    "pci:08.0"

class sis496_voodoo1_state : public sis496_state
{
public:
	sis496_voodoo1_state(const machine_config &mconfig, device_type type, const char *tag)
		: sis496_state(mconfig, type, tag)
		, m_voodoo(*this, PCI_ID_VIDEO)
		, m_screen(*this, "screen")
	{ }

	void sis496_voodoo1(machine_config &config);

protected:
	required_device<voodoo_1_pci_device> m_voodoo;
	required_device<screen_device> m_screen;
};

void sis496_state::main_map(address_map &map)
{
	map.unmap_value_high();
}

void sis496_state::main_io(address_map &map)
{
	map.unmap_value_high();
}

void sis496_state::sis496(machine_config &config)
{
	// Basic machine hardware
	I486DX4(config, m_maincpu, 75000000); // I486DX4, 75 or 100 Mhz
	m_maincpu->set_addrmap(AS_PROGRAM, &sis496_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &sis496_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("pci:05.0:pic8259_master", FUNC(pic8259_device::inta_cb));

	PCI_ROOT(config, "pci", 0);
	SIS85C496_HOST(config, "pci:05.0", 0, "maincpu", 32*1024*1024);

	ISA16_SLOT(config, "isa1", 0, "pci:05.0:isabus",  pc_isa16_cards, "svga_et4k", false);
	ISA16_SLOT(config, "isa2", 0, "pci:05.0:isabus",  pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "pci:05.0:isabus",  pc_isa16_cards, nullptr, false);
}

void sis496_voodoo1_state::sis496_voodoo1(machine_config &config)
{
	sis496_state::sis496(config);

	VOODOO_1_PCI(config, m_voodoo, 0, m_maincpu, m_screen);
	m_voodoo->set_fbmem(2);
	m_voodoo->set_tmumem(4, 0);
	m_voodoo->set_status_cycles(1000);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// Screeen size and timing is re-calculated later in voodoo card
	m_screen->set_refresh_hz(57);
	m_screen->set_size(640, 480);
	m_screen->set_visarea(0, 640 - 1, 0, 480 - 1);
	m_screen->set_screen_update(PCI_ID_VIDEO, FUNC(voodoo_1_pci_device::screen_update));
}

// generic placeholder for unknown BIOS types
// Funworld BIOS is temporary until we rewrite funworld/photoply.cpp
ROM_START( sis85c496 )
	ROM_REGION32_LE(0x20000, "pci:05.0", 0)
	ROM_SYSTEM_BIOS(0, "funworld", "Award 486e BIOS with W83787")
	// Photoplay BIOS
	ROMX_LOAD("funworld_award_486e_w83787.bin", 0x000000, 0x20000, BAD_DUMP CRC(af7ff1d4) SHA1(72eeecf798a03817ce7ba4d65cd4128ed3ef7e68), ROM_BIOS(0) ) // 486E 96/7/19 W83787 PLUG & PLAY BIOS, AT29C010A

	// Chipset: SiS 85C496/85C497 - CPU: Socket 3 - RAM: 2xSIMM72, Cache - Keyboard-BIOS: JETkey V5.0
	// ISA16: 3, PCI: 3 - BIOS: SST29EE010 (128k) AMI 486DX ISA BIOS AA2558003 - screen remains blank
	ROM_SYSTEM_BIOS(1, "4sim002", "AMI ISA BIOS (unknown)")
	ROMX_LOAD( "4sim002.bin", 0x00000, 0x20000, BAD_DUMP CRC(ea898f85) SHA1(7236cd2fc985985f21979e4808cb708be8d0445f), ROM_BIOS(1) )
ROM_END

// A-Trend ATC-1425A - Chipset: SiS 85C496, 85C497 - RAM: 4xSIMM72, Cache: 4x32pin + TAG - ISA16: 4, PCI: 3
// on board: 2xIDE, Floppy, 2xser, par - BIOS: 32pin
ROM_START( atc1425a )
	ROM_REGION32_LE(0x20000, "pci:05.0", 0)
	// 0: Boot block - BIOS-String: 09/07/95-SiS-496-497/A/B-2A4IBA2HC-00 / 1425 SIS 496/7 BIOS VER : 1.8N   1995/09/25
	ROM_SYSTEM_BIOS(0, "ver18n", "ver1.8N")
	ROMX_LOAD( "atc-1425a_original.bin", 0x00000, 0x20000, CRC(040ebc6c) SHA1(266ed07ef13c363234c7a2a88719badeeed9dc4c), ROM_BIOS(0))
	// 1: Boot block - BIOS-String: 11/03/95-SiS-496-497/A/B-2A4IBA2HC-00 / ATC-1425A SIS496/7 BIOS VER:2.0N  11-04-95
	ROM_SYSTEM_BIOS(1, "ver20n", "ver2.0N")
	ROMX_LOAD( "atc-1425a_ver2_0n.bin", 0x00000, 0x20000, CRC(0af2f6c0) SHA1(a1ce34bdee5119b9ae1d8530fcf611ca2f9d592e), ROM_BIOS(1))
ROM_END

// A-Trend ATC-1425B - BIOS Version: Award 4.51PG 04/18/96 - Chipset: SiS 85C496/85C497, Winbond - Keyboard BIOS: Holtek HT6542B - CPU: Socket 3
// RAM: 4xSIMM72, Cache: 4xUM61512AK-15, 1xISSI IS61C256AH-15N - on board: 2xIDE, Floppy, par, 2xser - ISA16: 4, PCI: 3
ROM_START( atc1425b ) // Boot block - BIOS String: 04/18/96-SiS-496-497/A/B-2A4IBA2BC-00
	ROM_REGION32_LE(0x20000, "pci:05.0", 0)
	ROM_LOAD( "1425b231.rom", 0x00000, 0x20000, CRC(1a19f34d) SHA1(09bb5e35ef07b57942cbca933f2a0334615a687e))
ROM_END

// Abit AB-PI4(T) - BIOS: 32pin - Keyboard-BIOS: Winbond 83C42 - CPU: Socket 3 - ISA16: 4, PCI: 3 - Chipset: SiS 85C496, 85C497
// RAM: 4xSIMM72, Cache: 9x32pin (occupied: 4xW24512AK-20, 1xW2457AK) - On board: 2xIDE
ROM_START( abpi4 )
	ROM_REGION32_LE(0x20000, "pci:05.0", 0)
	// 0: BIOS-String: &09/25/95-SiS-496-497/A/B-2A4IBA11C-0B / GREEN 486 PCI SYSTEM BIOS - boots into "boot block" rescue BIOS
	ROM_SYSTEM_BIOS(0, "pi4092595", "AB-PI4(T) 09/25/95")
	ROMX_LOAD( "pi4_0b.bin", 0x00000, 0x20000, CRC(2cd67f19) SHA1(4cf0b4ff10645371361d3782c8be06c463e70219), ROM_BIOS(0))
	// 1: 486IP-B-2-A (ABIT PI4/PI4T PCI clone) REV:2B.31 - Chipset : SiS 496/497 (NV/NU) - BIOS : AWARD 2a4ibb61 - Keyboard BIOS: JETkey V5.0G
	// RAM: 4xSIMM72, Cache: 9x32pin DIP (filled: 9xUM61256FK-15 CPU: Socket 3 - on board: 2xIDE - ISA16: 4, PCI: 3
	// BIOS-String : 10/02/95-SiS-496-497/A/B-2A4IBB61C-00 - boots into "boot block" rescue BIOS
	ROM_SYSTEM_BIOS(1, "486ipb2a", "486IP-B-2-A")
	ROMX_LOAD( "486ip-b-2-a.bin", 0x00000, 0x20000, CRC(8b1e3094) SHA1(84e8269f310b53497e63791fd3c081d7f631b686), ROM_BIOS(1))
ROM_END

// Abit AB-PM4
// BIOS-String: 09/04/95-SiS-496-497/A/B-2A4IBA13C-0C / GREEN 486 PCI SYSTEM BIOS
ROM_START( abpm4 )
	ROM_REGION32_LE(0x20000, "pci:05.0", 0)
	ROM_LOAD( "pm4_0c.bin", 0x00000, 0x20000, CRC(eaad7812) SHA1(81670c44e30fa8b8ac0aa28a5c367819ff1ca73c))
ROM_END

// Abit AB-PV4
// BIOS-String: 09/26/95-SiS-496-497/A/B-2A4IBA12C-0A / GREEN 486 PCI SYSTEM BIOS
ROM_START( abpv4 )
	ROM_REGION32_LE(0x20000, "pci:05.0", 0)
	ROM_LOAD( "pv4v_0a.bin", 0x00000, 0x20000, CRC(91de48d5) SHA1(2e873de152870270f51b5b2c4a30f2611364e739))
ROM_END

// Aopen AP43 - CPU: Socket 3 - Chipset: SiS 85C496, 85C497, SMC FDC37C665GT - RAM: SIMM72x4, Cache: 9x32pin, used: 9xUM61256FK-15
// BIOS: 32pin - Keyboard-BIOS: AMIKEY-2 - on board: IDEx2, Floppy, par, 2xser
ROM_START( aoap43 )
	ROM_REGION32_LE(0x20000, "pci:05.0", 0)
	// 0:
	ROM_SYSTEM_BIOS(0, "original", "original")
	ROMX_LOAD( "aopen_ap43_original.bin", 0x00000, 0x20000, CRC(65075fe4) SHA1(9b150e0b37b4ff3cbfcd8bd2286e1e575c34de02), ROM_BIOS(0))
	// 1:
	ROM_SYSTEM_BIOS(1, "updated", "updated")
	ROMX_LOAD( "aopen_ap43_updated.rom", 0x00000, 0x20000, CRC(68a5595e) SHA1(94551037e9d0b3fb644726b7ba66e676aa58b81a), ROM_BIOS(1))
ROM_END

// ASUS PCI/I-A486S (4xSIMM72, Cache: 128/256/512KB, 1 EISA) - BIOS: 32pin
// SiS 85C496/85C497 chipset; SMC 37C665 I/O; AMIKEY-2, S3 Trio 64 on board VGA, the manual also mentions Trio 32
ROM_START( aa486s )
	ROM_REGION32_LE(0x20000, "pci:05.0", 0)
	// 0: BIOS-String: 05/22/95/SiS-496-497B-PCI-A486-0-00 / #401A0-0203
	ROM_SYSTEM_BIOS(0, "v203", "ASUS PCI/I-A486S V2.03")
	ROMX_LOAD( "si4a0203.awd", 0x00000, 0x20000, CRC(95fcb7c6) SHA1(c19164d67af18c774e6eb06bd1570d95a24b2856), ROM_BIOS(0))
	// 1: BIOS-String: 11/27/95-SiS-496-497B-PI-A486SC-00 / #401A0-0304 -  boots into "boot block" rescue BIOS
	ROM_SYSTEM_BIOS(1, "v304", "ASUS PCI/I-A486S V3.04")
	ROMX_LOAD( "si4a0304.awd", 0x00000, 0x20000, CRC(a00ad907) SHA1(598d97ea29f930a9359429dc540d27bfdd0fcd20), ROM_BIOS(1))
ROM_END

// ASUS PVI-486SP3 (Socket 3, 2xSIMM72, Cache: 128/256/512KB, 2 IDE, 3 PCI, 4 ISA, 1 VLB)
// SiS 85C496 + 85C497; UMC UM8669F; AMIKEY-2; BIOS: 29EE010 (32pin)
ROM_START( a486sp3 )
	ROM_REGION32_LE(0x20000, "pci:05.0", 0) // Winbond W29EE011-15
	// 0: BIOS-String: 07/22/94-SATURN-P/I-4SP3-00 / #401A0-0207
	ROM_SYSTEM_BIOS(0, "v207", "ASUS PVI-486SP3 V2.07")
	ROMX_LOAD( "awsi0207.bin", 0x00000, 0x20000, CRC(0cb862aa) SHA1(7ffead05c1df47ec36afba395191145279c5e789), ROM_BIOS(0))
	// 1: BIOS-String: 07/22/94-SATURN-P/I-4SP3-00 / #401A0-0207
	ROM_SYSTEM_BIOS(1, "v2737", "ASUS PVI-486SP3 V2.07 #2")
	ROMX_LOAD( "awsi2737.bin", 0x00000, 0x20000, CRC(8cd9a89c) SHA1(6c68c23cc5e8ae66261e9fe931f2ce07efe767b6), ROM_BIOS(1))
	// 2: BIOS-String: 06/25/96-SiS-496-497B-PVI-4SP3C-00 / #401A0-0306 - boots to Award BootBlock BIOS V1.0
	ROM_SYSTEM_BIOS(2, "v306", "ASUS PVI-486SP3 V3.06")
	ROMX_LOAD( "si4i0306.awd", 0x00000, 0x20000, CRC(fc70371a) SHA1(96b10cfa97c5d1d023687f01e8acb54f263069b2), ROM_BIOS(2))
	// 3: BIOS-String: 02/11/98-SiS-496-497B-PVI-4SP3C-00 / #401A0-0307 - boots to Award BootBlock BIOS V1.0
	ROM_SYSTEM_BIOS(3, "v307", "ASUS PVI-486SP3 V3.07")
	ROMX_LOAD( "si4i0307h.bin", 0x00000, 0x20000, CRC(99473cc0) SHA1(a01d253cf434a31e0ca6f6cd2b9026ca424eb463), ROM_BIOS(3))
	// 4: BIOS-String: 08/08/95-SiS-496-497B-PVI-4SP3C-00 / #401A0-0301 - boots to Award BootBlock BIOS
	ROM_SYSTEM_BIOS(4, "v301", "ASUS PVI-486SP3 V3.01")
	ROMX_LOAD( "4siw003.bin", 0x00000, 0x20000, CRC(47a1d815) SHA1(370bfb895646518884a2a82881721efc3aeb04d1), ROM_BIOS(4))
	// 5: BIOS-String: 11/23/94-SiS-496-497-PVI-4SP3-00 / #401A0-0101
	ROM_SYSTEM_BIOS(5, "v10101", "ASUS PVI-486SP3 V1.01 #1")
	ROMX_LOAD( "0101.bin", 0x00000, 0x20000, CRC(7862ca56) SHA1(e609585893b23db10c4ae7d2abd17cc9dda964b6), ROM_BIOS(5))
	// 6: BIOS-String: 11/23/94-SiS-496-497-PVI-4SP3-00 / #401A0-0101 - screen remains blank
	ROM_SYSTEM_BIOS(6, "v10102", "ASUS PVI-486SP3 V1.01 #2")
	ROMX_LOAD( "si4i0101.awd", 0x00000, 0x20000, CRC(18652037) SHA1(7460e90b0a9c825d2e47943a714049fe9e943760), ROM_BIOS(6))
	// 7: BIOS-String: 07/15/95-SiS-496-497B-PVI-4SP3C-00 / #401A0-0205 - boots to Award BootBlock BIOS
	ROM_SYSTEM_BIOS(7, "v205", "ASUS PVI-486SP3 V2.05")
	ROMX_LOAD( "si4i0205.awd", 0x00000, 0x20000, CRC(d90d91b0) SHA1(043151d121780ff56ce32b9a48e9bbccd324625f), ROM_BIOS(7))
	// 8: BIOS-String: 04/05/96-SiS-496-497B-PVI-4SP3C-00 / #401A0-0305 - boots to Award BootBlock BIOS
	ROM_SYSTEM_BIOS(8, "v305", "ASUS PCI/I-486SP3 V3.05")
	ROMX_LOAD( "si4i0305.awd", 0x00000, 0x20000, CRC(2f90e63e) SHA1(a4f16753b5a57d65fba7702ca28e44f10bd5bb6c), ROM_BIOS(8))
ROM_END

COMP( 199?, sis85c496, 0, 0, sis496_voodoo1, 0, sis496_voodoo1_state, empty_init, "Hack Inc.", "486 motherboards using the SiS 85C496/85C497 chipset + 3dfx Voodoo 1", MACHINE_NOT_WORKING ) // 4sim002 crashes while enabling cache?

COMP( 1995, atc1425a,  0, 0, sis496, 0, sis496_state, empty_init, "A-Trend", "ATC-1425A (SiS 85C496/85C497)", MACHINE_NOT_WORKING ) // -bios 2 punts to Award BootBlock, -bios 0 and 1 crashes
COMP( 1996, atc1425b,  0, 0, sis496, 0, sis496_state, empty_init, "A-Trend", "ATC-1425B (SiS 85C496/85C497)", MACHINE_NOT_WORKING ) // punts to Award BootBlock

COMP( 1995, abpi4,     0, 0, sis496, 0, sis496_state, empty_init, "ABit", "AB-PI4 / AB-PI4T (SiS 85C496/85C497)", MACHINE_NOT_WORKING ) // hangs during irq check
COMP( 1995, abpm4,     0, 0, sis496, 0, sis496_state, empty_init, "ABit", "AB-PM4 (SiS 85C496/85C497)", MACHINE_NOT_WORKING ) // hangs during irq check
COMP( 1995, abpv4,     0, 0, sis496, 0, sis496_state, empty_init, "ABit", "AB-PV4 (SiS 85C496/85C497)", MACHINE_NOT_WORKING ) // hangs during irq check

COMP( 199?, aoap43,    0, 0, sis496, 0, sis496_state, empty_init, "AOpen", "AP43 (SiS 85C496/85C497)",  MACHINE_NOT_WORKING ) // crashes while enabling cache?

COMP( 1994, a486sp3,   0, 0, sis496, 0, sis496_state, empty_init, "Asus", "PVI-486SP3 (SiS 85C496/85C497)", MACHINE_NOT_WORKING ) // hangs during irq check
COMP( 1995, aa486s,    0, 0, sis496, 0, sis496_state, empty_init, "Asus", "PCI/I-A486S (SiS 85C496/85C497)", MACHINE_NOT_WORKING ) // -bios 0 crashes on boot, -bios 1 hardlocks MAME
