// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/*
 * Sandbox for SiS based 496/497 x86 PCs, targeting the new PCI model
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
#include "bus/pci/pci_slot.h"
#include "bus/rs232/hlemouse.h"
#include "bus/rs232/null_modem.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/sun_kbd.h"
#include "bus/rs232/terminal.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"
#include "machine/sis85c496.h"
#include "machine/w83787f.h"
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
	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;

	static void winbond_superio_config(device_t *device);
};

#define PCI_ID_VIDEO    "pci:09.0"

class sis496_voodoo1_state : public sis496_state
{
public:
	sis496_voodoo1_state(const machine_config &mconfig, device_type type, const char *tag)
		: sis496_state(mconfig, type, tag)
		, m_voodoo(*this, PCI_ID_VIDEO)
		, m_screen(*this, "voodoo_screen")
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

static void isa_com(device_slot_interface &device)
{
	device.option_add("microsoft_mouse", MSFT_HLE_SERIAL_MOUSE);
	device.option_add("logitech_mouse", LOGITECH_HLE_SERIAL_MOUSE);
	device.option_add("wheel_mouse", WHEEL_HLE_SERIAL_MOUSE);
	device.option_add("msystems_mouse", MSYSTEMS_HLE_SERIAL_MOUSE);
	device.option_add("rotatable_mouse", ROTATABLE_HLE_SERIAL_MOUSE);
	device.option_add("terminal", SERIAL_TERMINAL);
	device.option_add("null_modem", NULL_MODEM);
	device.option_add("sun_kbd", SUN_KBD_ADAPTOR);
}

static void isa_internal_devices(device_slot_interface &device)
{
	device.option_add("w83787f", W83787F);
}

void sis496_state::winbond_superio_config(device_t *device)
{
	w83787f_device &fdc = *downcast<w83787f_device *>(device);
//  fdc.set_sysopt_pin(1);
//  fdc.gp20_reset().set_inputline(":maincpu", INPUT_LINE_RESET);
//  fdc.gp25_gatea20().set_inputline(":maincpu", INPUT_LINE_A20);
	fdc.irq1().set(":pci:05.0", FUNC(sis85c496_host_device::pc_irq1_w));
	fdc.irq8().set(":pci:05.0", FUNC(sis85c496_host_device::pc_irq8n_w));
	fdc.txd1().set(":serport0", FUNC(rs232_port_device::write_txd));
	fdc.ndtr1().set(":serport0", FUNC(rs232_port_device::write_dtr));
	fdc.nrts1().set(":serport0", FUNC(rs232_port_device::write_rts));
	fdc.txd2().set(":serport1", FUNC(rs232_port_device::write_txd));
	fdc.ndtr2().set(":serport1", FUNC(rs232_port_device::write_dtr));
	fdc.nrts2().set(":serport1", FUNC(rs232_port_device::write_rts));
}

void sis496_state::sis496(machine_config &config)
{
	I486DX4(config, m_maincpu, 75000000); // I486DX4, 75 or 100 Mhz
	m_maincpu->set_addrmap(AS_PROGRAM, &sis496_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &sis496_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("pci:05.0:pic8259_master", FUNC(pic8259_device::inta_cb));

	PCI_ROOT(config, "pci", 0);
	SIS85C496_HOST(config, "pci:05.0", 0, "maincpu", 32*1024*1024);

	ISA16_SLOT(config, "board4", 0, "pci:05.0:isabus", isa_internal_devices, "w83787f", true).set_option_machine_config("w83787f", winbond_superio_config);
	ISA16_SLOT(config, "isa1", 0, "pci:05.0:isabus",  pc_isa16_cards, "wd90c31_lr", false);
	ISA16_SLOT(config, "isa2", 0, "pci:05.0:isabus",  pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "pci:05.0:isabus",  pc_isa16_cards, nullptr, false);

	rs232_port_device &serport0(RS232_PORT(config, "serport0", isa_com, "logitech_mouse"));
	serport0.rxd_handler().set("board4:w83787f", FUNC(w83787f_device::rxd1_w));
	serport0.dcd_handler().set("board4:w83787f", FUNC(w83787f_device::ndcd1_w));
	serport0.dsr_handler().set("board4:w83787f", FUNC(w83787f_device::ndsr1_w));
	serport0.ri_handler().set("board4:w83787f", FUNC(w83787f_device::nri1_w));
	serport0.cts_handler().set("board4:w83787f", FUNC(w83787f_device::ncts1_w));

	rs232_port_device &serport1(RS232_PORT(config, "serport1", isa_com, nullptr));
	serport1.rxd_handler().set("board4:w83787f", FUNC(w83787f_device::rxd2_w));
	serport1.dcd_handler().set("board4:w83787f", FUNC(w83787f_device::ndcd2_w));
	serport1.dsr_handler().set("board4:w83787f", FUNC(w83787f_device::ndsr2_w));
	serport1.ri_handler().set("board4:w83787f", FUNC(w83787f_device::nri2_w));
	serport1.cts_handler().set("board4:w83787f", FUNC(w83787f_device::ncts2_w));

	// TODO: 9-10-11-12 for PCI_SLOT (according to BIOS)
}

void sis496_voodoo1_state::sis496_voodoo1(machine_config &config)
{
	sis496_state::sis496(config);

	VOODOO_1_PCI(config, m_voodoo, 0, m_maincpu, m_screen);
	m_voodoo->set_fbmem(2);
	m_voodoo->set_tmumem(4, 0);
	// TODO: games are very annoyed with Direct3D 5 init/teardown fns around this.
	m_voodoo->set_status_cycles(1000);

	// TODO: wrong, needs VGA passthru
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(57);
	m_screen->set_size(640, 480);
	m_screen->set_visarea(0, 640 - 1, 0, 480 - 1);
	m_screen->set_screen_update(PCI_ID_VIDEO, FUNC(voodoo_1_pci_device::screen_update));

	PCI_SLOT(config, "pci:1", pci_cards, 10, 0, 1, 2, 3, nullptr);
	PCI_SLOT(config, "pci:2", pci_cards, 11, 1, 2, 3, 0, nullptr);
}

// generic placeholder for unknown BIOS types
// Funworld BIOS is temporary until we rewrite funworld/photoply.cpp
ROM_START( sis85c496 )
	ROM_REGION32_LE(0x20000, "pci:05.0", 0)
	ROM_SYSTEM_BIOS(0, "funworld", "Award 486e BIOS with W83787 (photoply)")
	// Photoplay BIOS
	// Lucky Star LS-486EF REV:B
	ROMX_LOAD("funworld_award_486e_w83787.bin", 0x000000, 0x20000, BAD_DUMP CRC(af7ff1d4) SHA1(72eeecf798a03817ce7ba4d65cd4128ed3ef7e68), ROM_BIOS(0) ) // 486E 96/7/19 W83787 PLUG & PLAY BIOS, AT29C010A

	// MegaTouch XL BIOSes
	// 09/11/96-SiS-496-SMC665-2A4IBU41C-00
	ROM_SYSTEM_BIOS(1, "merit", "Award 486e BIOS Telco (mtouchxl)")
	ROMX_LOAD( "094572516 bios - 486.bin", 0x000000, 0x020000, CRC(1c0b3ba0) SHA1(ff86dd6e476405e716ac7a4de4a216d2d2b49f15), ROM_BIOS(1) )
	// AMI BIOS, Jetway branded MB?
	// 40-040B-001276-00101111-040493-OP495SLC-0
	//ROMX_LOAD("prom.mb", 0x10000, 0x10000, BAD_DUMP CRC(e44bfd3c) SHA1(c07ec94e11efa30e001f39560010112f73cc0016) )

	// Chipset: SiS 85C496/85C497 - CPU: Socket 3 - RAM: 2xSIMM72, Cache - Keyboard-BIOS: JETkey V5.0
	// ISA16: 3, PCI: 3 - BIOS: SST29EE010 (128k) AMI 486DX ISA BIOS AA2558003 - screen remains blank
	ROM_SYSTEM_BIOS(2, "4sim002", "AMI ISA unknown BIOS")
	ROMX_LOAD( "4sim002.bin", 0x00000, 0x20000, BAD_DUMP CRC(ea898f85) SHA1(7236cd2fc985985f21979e4808cb708be8d0445f), ROM_BIOS(2) )
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

// Chaintech 486SPM - CPU: Socket 3 - Chipset: SiS 85C497, 85C496, UMC UM8663BF - RAM: 4xSIMM72, Cache: 8xIS61C1024-10N, W24512AK-10
// BIOS: Award E0822859 - Keyboard-BIOS: VIA VT82C42N - on board: 2xISA, Floppy, 2xser, par - ISA16: 4, PCI: 3
ROM_START( ch486spm )
	ROM_REGION32_LE(0x20000, "pci:05.0", 0)
	// 0: BIOS-String: 12/21/95-SiS-496-497/A/B-2A4IBC3IC-00 - stack crawl
	ROM_SYSTEM_BIOS(0, "original", "original")
	ROMX_LOAD( "chaintech_486spm.bin", 0x00000, 0x20000, CRC(a0c9045a) SHA1(1d0b1994574437549c13541d4b65374d94c9a648), ROM_BIOS(0))
	// 1: 12/21/95-SiS-496-497/A/B-2A4IBC3IC-00 / Chaintech 486SPM v2015 PS/2
	ROM_SYSTEM_BIOS(1, "ps2", "PS2 mouse enabled")
	ROMX_LOAD( "486spm-p.bin", 0x00000, 0x20000, CRC(35b5cb76) SHA1(965b212b28a5badd8d8f4769aa9edc88e47bc925), ROM_BIOS(1))
ROM_END

// Chaintech 4SPI - Chipset: SiS 85C496 85C497 - BIOS Version: Award v4.50G E0671975 - Keyboard BIOS: Lance Green LT38C41
// CPU: Socket 3 - RAM: 4xSIMM72, Cache: 9x32pin DIP (used: 4xW24512AK-15, 1xEM51256-15PL) - On board: 2xIDE
// ISA6: 5, PCI: 3
ROM_START( ch4spi ) // BIOS String: 02/16/95-SiS-496-497/A/B-2A4IBC31-B2 / 02/17/1995 / stack crawl
	ROM_REGION32_LE(0x20000, "pci:05.0", 0)
	ROM_LOAD( "4spibckp.bin", 0x00000, 0x20000, CRC(29b15737) SHA1(e9cb5402eb25a100a15d5ccc520cfa76c7be99a6))
ROM_END

// Freetech 486F55 - Chipset: SiS 496/497 - CPU: Socket 3 - RAM: 4xSIMM72, Cache: 128KB/256KB/512KB - ISA16: 4, PCI: 3 -
// On board: 2xser, par, 2xIDE, Floppy - BIOS: Award
ROM_START( ft486f55 )
	ROM_REGION32_LE(0x20000, "pci:05.0", 0)
	// 0:
	ROM_SYSTEM_BIOS(0, "xsf", "XS-F")
	ROMX_LOAD( "55xs-f.bin", 0x00000, 0x20000, CRC(b7ee53af) SHA1(6357241ac3c317f60465bf5ad77d821a7dc68b3b), ROM_BIOS(0))
	// 1:
	ROM_SYSTEM_BIOS(1, "xsg", "XS-G")
	ROMX_LOAD( "55xs-g.bin", 0x00000, 0x20000, CRC(adaa3a28) SHA1(27c36b564d11f1dc9a8c6f6d075eeaf850944c08), ROM_BIOS(1))
ROM_END

// Jamicon KM-S4-1 VER 1.1 - Chipset: SiS 85C496/85C497 (PR/NU revision), Winbond W837F - BIOS/Version: KM-S4-1 VER:4.2 - AWARD
// BIOS: Award PCI/PNP 486 S/N:024893105 - Keyboard BIOS: Winbond W83C42 - CPU: P24T - RAM: 4xSIMM72, Cache: 4xUM61512AK-15, 1xW24257AK-15
// on board: 2xser, Floppy, par, 2xIDE - ISA16: 3, PCI: 3
ROM_START( jakms41 ) // BIOS String: 10/30/95-SiS-496-497/A/B-2A4IBR22C-00
	ROM_REGION32_LE(0x20000, "pci:05.0", 0)
	ROM_LOAD( "km-s4-1.bin", 0x00000, 0x20000, CRC(0271356a) SHA1(893048c3390a23810a2af14da30520fbea10ad2f))
ROM_END

// Jetway J-446A - Chipset: SiS 85C497, 82C496 - CPU: Socket 3 - RAM: 4xSIMM72, Cache: 4+1 sockets - BIOS: 32pin
// Keyboard-BIOS: HOLTEK HT6542B - ISA16: 3, PCI: 3 - On board: 2xIDE, Floppy, par, 2xser
ROM_START( jwj446a )
	ROM_REGION32_LE(0x20000, "pci:05.0", 0)
	// 0:
	ROM_SYSTEM_BIOS( 0, "no1", "J-446A #1")
	ROMX_LOAD( "j446a_original.bin", 0x00000, 0x20000, BAD_DUMP CRC(79d2e360) SHA1(8bf3befa1c869e298ec346cc784fcbc2193e3912), ROM_BIOS(0))
	// 1: 02/02/96-SiS-496-497/A/B-2A4IBJ19C-00 / V.446 RP5 2-2-1996
	ROM_SYSTEM_BIOS( 1, "no2", "J-446A #2")
	ROMX_LOAD( "j446a.rom", 0x00000, 0x20000, CRC(3e3c6abd) SHA1(04952dc143baa7b51cb6fc5eb1961007ecf36aaf), ROM_BIOS(1))
ROM_END

// LuckyStar LS-486E  - Chipset : SiS496, SiS497, SMC FDC37C665GT - CPU: AMD 486DX4-100 (Socket 3) - RAM: 4xSIMM72, Cache: 4 sockets (UM61512AK-15)+1
// BIOS : AMIBIOS 486PCI ISA 393824, on a 27C010 type ROM chip - Keyboard-BIOS: AMIKEY-2 - ID string : 41-PH0D-001256-00101111-101094-SIS496AB-H
// On board: 2xISA, Floppy, par, 2xser - ISA16: 4, PCI: 3
ROM_START( ls486e )
	ROM_REGION32_LE(0x20000, "pci:05.0", 0)
	// 0: Rev:C
	ROM_SYSTEM_BIOS( 0, "revc01", "Rev.C #1")
	ROMX_LOAD( "ls486e_revc.bin", 0x00000, 0x20000, CRC(d678a26e) SHA1(603e03171b28f73bdb6ce27b0bbae2a4cfb13517), ROM_BIOS(0))
	// 1: LS486E Rev.D SiS496/497(PR/NU) EDO Support AWARD 10/21/96 - 10/21/96-SiS-496-497/A/B-2A4IBL12C-00 - 486E 96/10/24 UMC8669 PLUG & PLAY BIOS
	ROM_SYSTEM_BIOS( 1, "revd01", "Rev.D #1")
	ROMX_LOAD( "ls486-d.awa", 0x00000, 0x20000, CRC(5a51a3a3) SHA1(6712ab742676156802fdfc4d08d687c1482f2702), ROM_BIOS(1))
	// 2: Lucky Star LS486E rev.C,Winbond,SiS496/497  - BIOS Award PNP v4.50PG (486E 96/5/17 W83787) - BIOS-String: 03/14/96-SiS-496-497/A/B-2A4IBL13C-00 / 486E 96/5/17 W83787
	ROM_SYSTEM_BIOS( 2, "revc02", "Rev.C #2")
	ROMX_LOAD( "ls486e-c.awd", 0x00000, 0x20000, CRC(8c290f20) SHA1(33d9a96e5d6b3bd5776480f5535bb1eb1d7cff57), ROM_BIOS(2))
	//3: BIOS-String: 03/14/96-SiS-496-497/A/B-2A4IBL13C-00 / 486E 96/7/19 W83787 PLUG & PLAY BIOS - boots to BootBlock BIOS
	ROM_SYSTEM_BIOS( 3, "revc1", "Rev.C1") // also on a Rev.C2 board
	ROMX_LOAD( "ls486ec1.bin", 0x00000, 0x20000, CRC(e96d1bbc) SHA1(64d0726c4e9ecee8fddf4cc39d92aecaa8184d5c), ROM_BIOS(3))
	// 4: BootBlock BIOS
	ROM_SYSTEM_BIOS( 4, "lh5", "LH5")
	ROMX_LOAD( "ls-486e.bin", 0x00000, 0x20000, CRC(03ca4a97) SHA1(f9e5e2f2fabcb47960dfa91c37bf74fa93398092), ROM_BIOS(4))
	// 5: BIOS-String: 03/14/96-SiS-496-497/A/B-2A4IBL13C-00
	ROM_SYSTEM_BIOS( 5, "ls486eb", "LS-486E(B)")
	ROMX_LOAD( "4siw001.bin", 0x00000, 0x20000, CRC(d81d722d) SHA1(bb18324b3679b7419c230244891b626a61006486), ROM_BIOS(5))
ROM_END

// MSI MS-4144 - Chipset: SiS 85C497, 85C496, Winbond W83787F, W83758F - CPU: Socket 3 - RAM: 4xSIMM72, Cache: 8+1 sockets
// On board: 2xIDE, Floppy, 2xser, par - ISA16: 4, PCI: 3
ROM_START( ms4144 )
	ROM_REGION32_LE(0x20000, "pci:05.0", 0)
	// 0: no display
	ROM_SYSTEM_BIOS(0, "af53", "AF53")
	ROMX_LOAD( "ms-4144_af53.rom", 0x00000, 0x20000, CRC(931ebb7d) SHA1(fa7cf64c07a6404518e12c41c197354c7d05b2d2), ROM_BIOS(0))
	// 1: no display
	ROM_SYSTEM_BIOS(1, "af54", "AF54")
	ROMX_LOAD( "ms-4144_af54s.rom", 0x00000, 0x20000, CRC(1eb02779) SHA1(b18cc771fc5a820437a4daca06806188ee1a27a5), ROM_BIOS(1))
	// 2: BIOS-String: 03/20/96-SiS-496-497/A/B-2A4IBM49C-00 / WF53S 032096
	ROM_SYSTEM_BIOS(2, "wf53", "WF53")
	ROMX_LOAD( "ms-4144_wf53s.bin", 0x00000, 0x20000, CRC(df83f099) SHA1(b7dc61a2cb71754cddd06d12d3bf81ffce442c89), ROM_BIOS(2))
	// 3: BIOS-String: 02/07/96-SiS-496-497/A/B-2A4IBM49C-00 / WF54S 020896
	ROM_SYSTEM_BIOS(3, "wf54", "WF54")
	ROMX_LOAD( "ms-4144_wf54s.bin", 0x00000, 0x20000, CRC(c0ff31df) SHA1(4e138558781a220b340977d56ccbfa61a907d4f5), ROM_BIOS(3))
	// 4: no display - VER 2.1 - BIOS: AMI 486DX ISA BIOS AC8999569 (32pin)- Keyboard-BIOS: AMIKEY-2
	ROM_SYSTEM_BIOS(4, "v21", "Ver 2.1")
	ROMX_LOAD( "486-pci-ms4144.bin", 0x00000, 0x20000, CRC(8bd50381) SHA1(c9853642ac0946c2b1a7e469bcfacbb3351c4067), ROM_BIOS(4))
ROM_END

// SOYO 30H - CPU: Socket 3 - RAM: SIMM72x4 - Cache: 256K, 512K or 1024K - ISA16: 4, PCI: 3 - on board: 2xIDE
// BIOS-String: 12/07/95-SiS-496-497/A/B-2A4IBS2AC-00 / REV B2
ROM_START( so30h )
	ROM_REGION32_LE(0x20000, "pci:05.0", 0)
	ROM_LOAD( "30h-b2.bin", 0x00000, 0x20000, CRC(1dd22cef) SHA1(dd0ac15e7a792e8fba2f55d6a1b35256e74bcf4e))
ROM_END

// SOYO SY-4SAW2 - Chipset: SiS 85C497, 85C496, Winbond W83787F - CPU: Socket 3 - RAM: 4xSIMM72, Cache: 4xUM61512AK-15+W24129AK-15
// BIOS: Award (32pin) - Keyboard-BIOS: Via VT82C42N - ISA16: 3, ISA16/VL: 1, PCI: 4 - On board: 2xser, par, 2xIDE, Floppy
// keeping the ROMs for the 4SA boards here until the differences between the boards are clear, e.g. difference between SY-4SAW and 4SA2: L2-cache
ROM_START( so4saw2 )
	ROM_REGION32_LE(0x20000, "pci:05.0", 0)
	// 0: BIOS-String: 04/15/95-SiS-496-497/A/B-2A4IBS22-00 / REV IO-A
	ROM_SYSTEM_BIOS(0, "ioa", "IO-A")
	ROMX_LOAD( "4sa2_bios_isa_486_488755.bin", 0x00000, 0x20000, CRC(21708d9c) SHA1(be4596507df1f5cc8a4e1baafce52b96417ac029), ROM_BIOS(0))
	// 1: BIOS-String: 08/22/95-SiS-496-497/A/B-2A4IBS2hC-00 / REV IO-B1 (4SA2, http://www.elhvb.com/supportbios.info/Archives/BIOS/R-S/SOYO/4SA2/index.html)
	ROM_SYSTEM_BIOS(1, "iob1", "IO-B1")
	ROMX_LOAD( "4sa-iob1.bin", 0x00000, 0x20000, CRC(a74891b6) SHA1(974c3a854a4e83202555bcbcba191f902527b577), ROM_BIOS(1))
	// 2: BIOS-String: 07/30/97-SiS-496-497/A/B-2A4IBS2hC-00 / SA-0730 (4SA, http://www.elhvb.com/supportbios.info/Archives/BIOS/R-S/SOYO/4SA/index.html)
	ROM_SYSTEM_BIOS(2, "0730", "0730")
	ROMX_LOAD( "4sa0730.bin", 0x00000, 0x20000, CRC(dea32658) SHA1(2c89500d9904f61a5426de5f1351ca8004c9920b), ROM_BIOS(2))
	// 3: BIOS-String: 07/03/96-SiS-496-497/A/B-2A4IBS29C-00 / REV WA53 (4SAW/4SAW2)
	ROM_SYSTEM_BIOS(3, "wa53", "WA53")
	ROMX_LOAD( "4saw53.bin", 0x00000, 0x20000, CRC(2265a9d1) SHA1(bd625f0f11e64d2620648cf14e6b6faf09df80bc), ROM_BIOS(3))
	// 4: BIOS-String: 12/05/95-SiS-496-497/A/B-2A4IBS29C-00 / REV WA3 (4SAW/4SAW2)
	ROM_SYSTEM_BIOS(4, "wa3", "WA3")
	ROMX_LOAD( "4saw-wa3.bin", 0x00000, 0x20000, CRC(d47e727e) SHA1(c6ba38e72575127b763a8e5ead49dbaaef85ab06), ROM_BIOS(4))
	// 5: BIOS-String: 09/11/97-SiS-496-497/A/B-2A4IBS29C-00 / REV WA0911 (4SAW/4SAW2 http://www.elhvb.com/supportbios.info/Archives/BIOS/R-S/SOYO/4SAW/index.html)
	ROM_SYSTEM_BIOS(5, "0911", "0911")
	ROMX_LOAD( "4saw0911.bin", 0x00000, 0x20000, CRC(4056b35e) SHA1(bca2d2507b15800ad13bd8f8c6699b49b8e87011), ROM_BIOS(5))
	// 6: BIOS-String: 09/11/97-SiS-496-497/A/B-2A4IBS29C-00 / REV WA0911 128GB BETA ROM (4SAW)
	ROM_SYSTEM_BIOS(6, "0911b", "0911b")
	ROMX_LOAD( "4saw0911b.bin", 0x00000, 0x20000, CRC(000fca3e) SHA1(46ceb550ed08fb013f02e51e1d428a60e220ede6), ROM_BIOS(6))
ROM_END

// ZIDA Tomato board 4DPS - Chipset: SIS 85C497, SIS 85C496, Winbond W83787IF, W83768F, MX8318-01PC - CPU: 486/5x86 - BIOS: Winbond W29EE011-15 / AWARD PCI/PNP
// Keyboard-BIOS: HOLTEK HT6542B or AMIKEY-2 - ISA16: 3, PCI: 3 - OSC: 24.000 - On board: 2xIDE, Floppy, 2xCOM, 1xPRN, Mouse, GAME
// from v4.00 onward it needs FLASH instead of EPROM to update the ESCD at boot time
ROM_START( zito4dps )
	ROM_REGION32_LE(0x20000, "pci:05.0", 0)
	// 0: BIOS-String: 01/10/96-SiS-496-497/A/B-2A4IBZ11C-00 / 4DPS  VER 1.5 (2301952A4IBZ11)
	ROM_SYSTEM_BIOS( 0, "4dps01", "Tomato 4DPS #1")
	ROMX_LOAD( "4siw004.bin", 0x00000, 0x20000, CRC(0c57cc33) SHA1(04ce27dc89ae15d70c14076ad4f82b50a4f1e6dd), ROM_BIOS(0))
	// 1: BIOS-String: 06/17/1998-SiS-496-497/A-2A4IBZ11C-00 / 4DPS V4.00A (17/06/98)
	ROM_SYSTEM_BIOS( 1, "4dps02", "Tomato 4DPS #2")
	ROMX_LOAD( "4dps02.bin", 0x00000, 0x20000, CRC(757a5ef7) SHA1(e35146f34329a6a7033b1ed9d95a77692826a060), ROM_BIOS(1))
	// 2: BIOS-String: 10/17/96-SiS-496-497/A/B-2A4IBZ11C-00 / 4DPS  VER 1.71 (1710962A4IBZ11)
	ROM_SYSTEM_BIOS( 2, "171", "Tomato 4DPS v1.71")
	ROMX_LOAD( "4dps_170.bin", 0x00000, 0x20000, CRC(10b43a85) SHA1(d77bb2420b98c030add5de52fc90c88384b2036b), ROM_BIOS(2))
	// 3: BIOS-String: 07/08/97-SiS-496-497/A/B-2A4IBZ1AC-00 / 4DPS VER 1.72F (10072A4IBZ1A)
	ROM_SYSTEM_BIOS( 3, "172f", "Tomato 4DPS v1.72f")
	ROMX_LOAD( "4dps172g.bin", 0x00000, 0x20000, CRC(184eeeba) SHA1(248555567e35d4d6a0cfad5abc989e8193a72351), ROM_BIOS(3))
	// 4: BIOS-String: 06/17/1998-SiS-496-497/A-2A4IBZ11C-00 / 4DPS V4.00A (17/06/98)
	ROM_SYSTEM_BIOS( 4, "400a", "Tomato 4DPS v4.00a")
	ROMX_LOAD( "4dps400a.bin", 0x00000, 0x20000, CRC(494da2da) SHA1(9dcae9aa403627df03d5777c1b4de0b9f98bb24f), ROM_BIOS(4))
	// 5: BIOS-String: 01/10/96-SiS-496-497/A/B-2A4IBZ11C-00 / Tomato 4DPS v4.01 (Y2K ready)
	ROM_SYSTEM_BIOS( 5, "401e", "Tomato 4DPS v4.01e")
	ROMX_LOAD( "4dps401e.bin", 0x00000, 0x20000, CRC(e84b2bb2) SHA1(5dd8e801decf87af90ff90e3096819354f657b5a), ROM_BIOS(5))
	// 6: v2.11, also marked v400a - BIOS-String: 06/17/1998-SiS-496-497/A-2A4IBZ11C-00 / 4DPS V4.00A (17/06/98)
	ROM_SYSTEM_BIOS( 6, "4dps03", "Tomato 4DPS #3")
	ROMX_LOAD( "4dps400b.bin", 0x00000, 0x20000, CRC(5910fa95) SHA1(934845038298d2d50f5bd4b20e0a4ccd9aa74e82), ROM_BIOS(6))
	// 7: BIOS-String: 11/23/95-SiS-496-497/A/B-2A4IBZ11C-00
	ROM_SYSTEM_BIOS( 7, "4dps04", "Tomato 4DPS #4")
	ROMX_LOAD( "4dps04.bin", 0x00000, 0x20000, CRC(f704be6a) SHA1(536c17c2a26e8a0f3bc3ddf6b8daa2f694905c24), ROM_BIOS(7))
	// 8: 01/10/96-SiS496-497/A/B-2A4IBZ11C-00 / 4DPS VER 1.6 (2005962A4IBZ11)
	ROM_SYSTEM_BIOS( 8, "160", "Tomato 4DPS v1.6")
	ROMX_LOAD( "4dps_160.bin", 0x00000, 0x20000, CRC(27d23966) SHA1(3fea7573c1897a4bd6d09e4ffc4e26372a25e43a), ROM_BIOS(8))
ROM_END


/***************************************************************************
  Game driver(s)
***************************************************************************/

//    YEAR  NAME       PARENT   COMPAT   MACHINE    INPUT  CLASS         INIT            COMPANY        FULLNAME                FLAGS
COMP( 199?, sis85c496, 0, 0, sis496_voodoo1, 0, sis496_voodoo1_state, empty_init, "Hack Inc.", "486 motherboards using the SiS 85C496/85C497 chipset + 3dfx Voodoo 1", MACHINE_NOT_WORKING ) // 4sim002 crashes while enabling cache?

COMP( 1995, atc1425a,  0, 0, sis496, 0, sis496_state, empty_init, "A-Trend", "ATC-1425A (SiS 85C496/85C497)", MACHINE_NOT_WORKING ) // -bios 2 punts to Award BootBlock, -bios 0 and 1 crashes
COMP( 1996, atc1425b,  0, 0, sis496, 0, sis496_state, empty_init, "A-Trend", "ATC-1425B (SiS 85C496/85C497)", MACHINE_NOT_WORKING ) // punts to Award BootBlock

COMP( 1995, abpi4,     0, 0, sis496, 0, sis496_state, empty_init, "ABit", "AB-PI4 / AB-PI4T (SiS 85C496/85C497)", MACHINE_NOT_WORKING ) // hangs during irq check
COMP( 1995, abpm4,     0, 0, sis496, 0, sis496_state, empty_init, "ABit", "AB-PM4 (SiS 85C496/85C497)", MACHINE_NOT_WORKING ) // hangs during irq check
COMP( 1995, abpv4,     0, 0, sis496, 0, sis496_state, empty_init, "ABit", "AB-PV4 (SiS 85C496/85C497)", MACHINE_NOT_WORKING ) // hangs during irq check

COMP( 199?, aoap43,    0, 0, sis496, 0, sis496_state, empty_init, "AOpen", "AP43 (SiS 85C496/85C497)",  MACHINE_NOT_WORKING ) // crashes while enabling cache?

COMP( 1994, a486sp3,   0, 0, sis496, 0, sis496_state, empty_init, "Asus", "PVI-486SP3 (SiS 85C496/85C497)", MACHINE_NOT_WORKING ) // hangs during irq check
COMP( 1995, aa486s,    0, 0, sis496, 0, sis496_state, empty_init, "Asus", "PCI/I-A486S (SiS 85C496/85C497)", MACHINE_NOT_WORKING ) // -bios 0 crashes on boot, -bios 1 hardlocks MAME

COMP( 199?, ch486spm,  0, 0, sis496, 0, sis496_state, empty_init, "Chaintech", "486SPM", MACHINE_NOT_WORKING )  // both versions used to show Award BootBlock, now show a black screen
COMP( 199?, ch4spi,    0, 0, sis496, 0, sis496_state, empty_init, "Chaintech", "4SPI", MACHINE_NOT_WORKING )    // used to come up, now black screen

COMP( 1995, ft486f55,  0, 0, sis496, 0, sis496_state, empty_init, "Freetech", "486FT55", MACHINE_NOT_WORKING )  // used to show Award BootBlow, now black screen

COMP( 199?, jakms41,   0, 0, sis496, 0, sis496_state, empty_init, "Jamicon", "KM-S4-1 VER 1.1", MACHINE_NOT_WORKING )
COMP( 199?, jwj446a,   0, 0, sis496, 0, sis496_state, empty_init, "Jetway",  "J-446A", MACHINE_NOT_WORKING )    // BIOS 0 shows BootBlock, but hangs on beep, BIOS 1 hangs, used to show BootBlock

COMP( 199?, ls486e,    0, 0, sis496, 0, sis496_state, empty_init, "LuckyStar",   "LS-486E Rev:C", MACHINE_NOT_WORKING ) // All versions POST, except LH5 (BootBlock)

COMP( 199?, ms4144,    0, 0, sis496, 0, sis496_state, empty_init, "MSI",         "MS-4144", MACHINE_NOT_WORKING ) // WF53 and WF54 used to show Award BootBlock, now hang

COMP( 199?, so30h,     0, 0, sis496, 0, sis496_state, empty_init, "SOYO", "30H", MACHINE_NOT_WORKING ) // used to show Award BootBlock, now hangs
COMP( 199?, so4saw2,   0, 0, sis496, 0, sis496_state, empty_init, "SOYO", "SY-4SAW2", MACHINE_NOT_WORKING ) // BIOS 0-3 show a black screen (2-3 used to display BootBlock), #4 exits MAME with "Called modrm_to_EA with modrm value ED!"

COMP( 199?, zito4dps,  0, 0, sis496, 0, sis496_state, empty_init, "ZIDA", "Tomato board 4DPS", MACHINE_NOT_WORKING ) // BIOS 0,2,5,7,8 POST, BIOS 1,3,4,6 show a black screen

