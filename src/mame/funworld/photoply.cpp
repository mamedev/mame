// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Photo Play (c) 1998? Funworld

TODO:
- Implement microtouch as RS232 option;
- Reimplement Combo ISA card;
- EEPROM timings are hacked (writes mostly fail otherwise);
- AudioDrive ES688 / ES1688 / ES1788 & ES1868;
- photoply99nl: "Druk op het snijpunt van beide lijnen" -> "Press the intersection of both lines"
  on Touch Screen calibration screen, except there aren't any on screen, gd5446 bug?
- 100 MHz gets detected as 66 by BIOS;
- Flush non-default scores/settings in dumps (cfr. photoply2ksp hi-scores, photoply2k4 that starts
up with 17 credits in)

===================================================================================================

INFO ABOUT SECURITY DONGLES:

Photo Play machines were found with different types of security dongles. They weren't really
dependant on the version, and you can find the same game version with different dongle type.

Dongle types found on 486 machines:

Blue parallel (without passthrough) dongle with the following components under expoxy resin:
 -Atmel AT89C2051 MCU
 -Xtal 11.05MHz
 -24C08W6 SEEPROM
 -HC132

Blue or black Aladdin parallel dongle (with passthrough) with the following components:
 -Aladdin Marvin2 MCU (unknown core)
 -Custom Aladdin chip marked as ALDN1 V28 (serial EEPROM?)

Yellow Marx CBN/CBV parallel dongle (with passthrough) with the following components:
 -Marx CBN/CBV/CBS MCU (unknown core)
 -74HC00

Yellow serial dongle (with passthrough) with the following components:
 -Smartcard (SAM, ID-000 size), with ATR "3B8281317643C002C5" (probably Siemens CardOS/M2 V2.01 with SLE44CxxS MCU)
 -AT90S2313 MCU
 -3.6864 Xtal
 -3 x 74HC14D

On newer models (Pentium and better), other kinds of dongles can also be found:

Parallel (without passthrough) dongle with the following components:
 -SX28AC/DP MCU (different manufacturers: Parallax, Ubicom, etc.)
 -On some versions there's also a SEEPROM (93C46LN, etc.)

USB Dongle
 -GNT FNW USB Token (http://www.softidea.sk/doc/gnt_datasheet_en.pdf)

*******************************************************************************************************/

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

namespace {

class photoply_state : public driver_device
{
public:
	photoply_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
//      , m_eeprom(*this, "eeprom")
	{
	}

	void photoply(machine_config &config);
	void photoply_dx4_100(machine_config &config);

private:
	required_device<i486dx4_device> m_maincpu;
//  required_device<eeprom_serial_93cxx_device> m_eeprom;

//  uint8_t bios_r(offs_t offset);
//  void bios_w(offs_t offset, uint8_t data);
//  void eeprom_w(uint8_t data);

	static void winbond_superio_config(device_t *device);

	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};

#if 0
uint8_t photoply_state::bios_r(offs_t offset)
{
	uint8_t bit_mask = (offset & 0x38000) >> 15;

	if((m_pci_shadow_reg & 0x200) == 0x200)
	{
		if(m_pci_shadow_reg & (1 << bit_mask))
			return m_shadow_ram[offset];
	}

	// TODO: this mapping is a complete guesswork
	// TODO: worth converting this to bankdev when PCI gets de-legacized
	switch(bit_mask & 7)
	{
		// cirrus logic video bios
		case 0: // c0000-c7fff
			return m_video_bios[offset & 0x7fff];
		// multifunction board bios
		// Note: if filled with a reload on 0x4000-0x7fff it will repeat the "Combo I/O" EEPROM check (which looks unlikely)
		case 1: // c8000-cffff
			if ((offset & 0x7fff) == 0x3fff)
			{
				// other bits are unknown
				return m_eeprom->do_read() << 2;
			}
			return m_ex_bios[offset & 0x7fff];
		case 2: // d0000-dffff
		case 3:
			return 0;
	}

	// e0000-fffff
	return m_main_bios[offset & 0x1ffff];
}

void photoply_state::eeprom_w(uint8_t data)
{
	//logerror("Writing %X to EEPROM output port\n", data);
	m_eeprom->di_write(BIT(data, 0));
	m_eeprom->clk_write(BIT(data, 1));
	m_eeprom->cs_write(BIT(data, 2));

	// Bits 2-3 also seem to be used to bank something else.
	// Bits 4-7 are set for some writes, but may do nothing?
}
#endif

void photoply_state::main_map(address_map &map)
{
	map.unmap_value_high();
//  map(0x000c0000, 0x000fffff).rw(FUNC(photoply_state::bios_r), FUNC(photoply_state::bios_w));
}

void photoply_state::main_io(address_map &map)
{
	map.unmap_value_high();
//  map(0x0202, 0x0202).w(FUNC(photoply_state::eeprom_w));
}

static INPUT_PORTS_START( photoply )
INPUT_PORTS_END

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

void photoply_state::winbond_superio_config(device_t *device)
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

void photoply_state::photoply(machine_config &config)
{
	I486DX4(config, m_maincpu, 75'000'000); // I486DX4, 75 or 100 Mhz
	m_maincpu->set_addrmap(AS_PROGRAM, &photoply_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &photoply_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("pci:05.0:pic8259_master", FUNC(pic8259_device::inta_cb));

	PCI_ROOT(config, "pci", 0);
	SIS85C496_HOST(config, "pci:05.0", 0, "maincpu", 32*1024*1024);

	ISA16_SLOT(config, "superio", 0, "pci:05.0:isabus", isa_internal_devices, "w83787f", true).set_option_machine_config("w83787f", winbond_superio_config);
	ISA16_SLOT(config, "isa1", 0, "pci:05.0:isabus",  pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa2", 0, "pci:05.0:isabus",  pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "pci:05.0:isabus",  pc_isa16_cards, nullptr, false);

	// TODO: convert to Microtouch
	rs232_port_device &serport0(RS232_PORT(config, "serport0", isa_com, "logitech_mouse"));
	serport0.rxd_handler().set("superio:w83787f", FUNC(w83787f_device::rxd1_w));
	serport0.dcd_handler().set("superio:w83787f", FUNC(w83787f_device::ndcd1_w));
	serport0.dsr_handler().set("superio:w83787f", FUNC(w83787f_device::ndsr1_w));
	serport0.ri_handler().set("superio:w83787f", FUNC(w83787f_device::nri1_w));
	serport0.cts_handler().set("superio:w83787f", FUNC(w83787f_device::ncts1_w));

	rs232_port_device &serport1(RS232_PORT(config, "serport1", isa_com, nullptr));
	serport1.rxd_handler().set("superio:w83787f", FUNC(w83787f_device::rxd2_w));
	serport1.dcd_handler().set("superio:w83787f", FUNC(w83787f_device::ndcd2_w));
	serport1.dsr_handler().set("superio:w83787f", FUNC(w83787f_device::ndsr2_w));
	serport1.ri_handler().set("superio:w83787f", FUNC(w83787f_device::nri2_w));
	serport1.cts_handler().set("superio:w83787f", FUNC(w83787f_device::ncts2_w));

	PCI_SLOT(config, "pci:1", pci_cards, 10, 0, 1, 2, 3, "gd5446");
	PCI_SLOT(config, "pci:2", pci_cards, 11, 1, 2, 3, 0, nullptr);
	PCI_SLOT(config, "pci:3", pci_cards, 12, 2, 3, 0, 1, nullptr);

//  EEPROM_93C46_16BIT(config, "eeprom")
//      .write_time(attotime::from_usec(1))
//      .erase_all_time(attotime::from_usec(10));
}

// We asume that every Photo Play from 1999 onwards use a DX4 100MHz instead of a 75MHz one (both were compatible, the latter were recommended)
void photoply_state::photoply_dx4_100(machine_config &config)
{
	photoply(config);
	m_maincpu->set_clock(100'000'000);
}

ROM_START(photoply98sp)
	ROM_REGION(0x20000, "pci:05.0", 0) // Motherboard BIOS
	ROM_LOAD("funworld_award_486e_w83787.bin", 0x000000, 0x20000, CRC(af7ff1d4) SHA1(72eeecf798a03817ce7ba4d65cd4128ed3ef7e68) ) // 486E 96/7/19 W83787 PLUG & PLAY BIOS, AT27C010, Funworld sticker: Sept 1998

	ROM_REGION(0x8000, "ex_bios", ROMREGION_ERASE00 ) // Multifunction board with a ESS AudioDrive chip, Funworld sticker: Sept 1998
	ROM_LOAD("enhanced_bios_centos.bin", 0x0000, 0x8000, CRC(ee8ad003) SHA1(4814385117599a98da02155785d1e3fce4e485bd) ) // Centos CI-8000/PP2000 ROM BIOS Version 1.06, 27C256B

	ROM_REGION(0x10000, "hdd_fw", 0) // Hard disk firmware
	ROM_LOAD("m2_at29c512.bin", 0x0000, 0x10000, CRC(22a1c9ce) SHA1(6b695ee56867176d1702273e68b5584db1b94e02) ) // Seagate ST31722A

	// Seagate ST31722A
	// 3303 CYL 1704MB 16 HEADS 63 SECTORS
	// Funworld label: Feb 1998
	DISK_REGION( "pci:05.0:ide1:0:hdd" )
	DISK_IMAGE( "photoplay98sp", 0, BAD_DUMP SHA1(c0d7964edaff6b99184ca64e76c41eaa07abe019) ) // From an operated HDD. A clean one must be recreated from the CDs
ROM_END

/* Intel A80486DX4100
   4096KB RAM
   SiS 85C496 + 85C497
   5 x ISSI IS61C256AN-15N Cache RAM
   Winbond W83787IF (near Xtal 24.00 MHz)
   3 x ISA + 2 x PCI */
ROM_START(photoply99sp)
	ROM_REGION(0x20000, "pci:05.0", 0) // Motherboard BIOS
	ROM_LOAD("funworld_award_486e_w83787.bin", 0x000000, 0x20000, CRC(af7ff1d4) SHA1(72eeecf798a03817ce7ba4d65cd4128ed3ef7e68) ) // 486E 96/7/19 W83787 PLUG & PLAY BIOS, AT29C010A

	/* Multifunction board with a ESS AudioDrive chip ISA Sound + I/O (PP2000/CI-8000)
	   ESS AudioDrve ES1868F
	   NEC D71055L-10 Parallel Interface Unit
	   Winbond W83877AF
	   Xtal 24.000 MHz (near W83877AF and D71055L-10)
	   Xtal 14.31818 MHz (near ES1868F)
	   Atmel 93C46 Serial EEPROM
	   PALCE16V8H (UNDUMPED)
	   PALCE16V8H (UNDUMPED)
	   8 Dips */
	ROM_REGION(0x8000, "ex_bios", ROMREGION_ERASE00 )
	ROM_LOAD("enhanced_bios_1.06.u13", 0x0000, 0x8000, CRC(d05e9d20) SHA1(854501b7b3bf988b10516109d058f7ca2aa07d3e) ) // Centos Combo I/O ROM BIOS for CI-8000/PP2000 v1.06, W27E257

	/* Cirrus Logic PCI CL-GD5446-HC-A
	   512MB RAM (2 x M5416258B-30J) (??? -AS)
	   Xtal 14.31818 MHz */

	/* The Photo Play 1999 parallel port dongle contains, under expoxy resin:
	    -Atmel AT89C2051 MCU (2KBytes internal ROM)
	    -Xtal 11.05MHz
	    -24C08W6 SEEPROM
	    -HC132
	   On each dongle there's a sticker with the revision number, but we've found the same MCU program on different revisions and
	   different SEEPROM contents on the same revision. We're adding here every found combination of MCU program and SEEPROM content.
	    */
	ROM_REGION(0xC00, "dongle", 0)

	// Same MCU program as "pp_99_dongle_r3a", and "pp_99_dongle_r1" but with unique SEEPROM content
	ROM_SYSTEM_BIOS(0, "pp_99_dongle_r3",  "Parallel port dongle Rev. 3")
	ROMX_LOAD("dongle_photoply_1999_sp_r3_mcu.bin",         0x000, 0x800, CRC(5c31e231) SHA1(5242ff4ccca832c28998aaf9c9c310b66297fd21), ROM_BIOS(0)) // AT89C2051, same program as "pp_99_dongle_r1"
	ROMX_LOAD("dongle_photoply_1999_sp_r3_seeprom.bin",     0x800, 0x400, CRC(62f68a79) SHA1(72477e07db0982764aede1b7e723aedf58937426), ROM_BIOS(0)) // 24C08W6

	// Same MCU program as "pp_99_dongle_r3", and "pp_99_dongle_r1" but with unique SEEPROM content
	ROM_SYSTEM_BIOS(1, "pp_99_dongle_r3a", "Parallel port dongle Rev. 3 (alt)")
	ROMX_LOAD("dongle_photoply_1999_sp_r3_alt_mcu.bin",     0x000, 0x800, CRC(5c31e231) SHA1(5242ff4ccca832c28998aaf9c9c310b66297fd21), ROM_BIOS(1)) // AT89C2051, same program as "pp_99_dongle_r1"
	ROMX_LOAD("dongle_photoply_1999_sp_r3_alt_seeprom.bin", 0x800, 0x400, CRC(9442d1d7) SHA1(4426542c4dbb3f1df65e7ba798a7d7e0d8b98838), ROM_BIOS(1)) // 24C08W6

	// Same MCU program as "pp_99_dongle_r1a", but with unique SEEPROM content
	ROM_SYSTEM_BIOS(2, "pp_99_dongle_r2",  "Parallel port dongle Rev. 2")
	ROMX_LOAD("dongle_photoply_1999_sp_r2_mcu.bin",         0x000, 0x800, CRC(7fa7a6b5) SHA1(611a4debc3b3e00c72fbc59a8e100f67806d73e8), ROM_BIOS(2)) // AT89C2051, same program as "pp_99_dongle_r1a"
	ROMX_LOAD("dongle_photoply_1999_sp_r2_seeprom.bin",     0x800, 0x400, CRC(52274688) SHA1(786f7407e510b303401120b8e1b082cdb412e648), ROM_BIOS(2)) // 24C08W6

	// Same MCU program as "pp_99_dongle_r3" and "pp_99_dongle_r3a", and same SEEPROM content as "pp_99_dongle_r1a"
	ROM_SYSTEM_BIOS(3, "pp_99_dongle_r1",  "Parallel port dongle Rev. 1")
	ROMX_LOAD("dongle_photoply_1999_sp_r1_mcu.bin",         0x000, 0x800, CRC(5c31e231) SHA1(5242ff4ccca832c28998aaf9c9c310b66297fd21), ROM_BIOS(3)) // AT89C2051, same program as "pp_99_dongle_r3"
	ROMX_LOAD("dongle_photoply_1999_sp_r1_seeprom.bin",     0x800, 0x400, CRC(fe8f14d2) SHA1(1caad3200a22e0d510238ba44e5d96f561045ec1), ROM_BIOS(3)) // 24C08W6, Same content as "pp_99_dongle_r1a"

	// Same MCU program as "pp_99_dongle_r2", and same SEEPROM content as "pp_99_dongle_r1"
	ROM_SYSTEM_BIOS(4, "pp_99_dongle_r1a",  "Parallel port dongle Rev. 1 (alt)")
	ROMX_LOAD("dongle_photoply_1999_sp_r1_alt_mcu.bin",     0x000, 0x800, CRC(7fa7a6b5) SHA1(611a4debc3b3e00c72fbc59a8e100f67806d73e8), ROM_BIOS(3)) // AT89C2051, same program as "pp_99_dongle_r2"
	ROMX_LOAD("dongle_photoply_1999_sp_r1_alt_seeprom.bin", 0x800, 0x400, CRC(fe8f14d2) SHA1(1caad3200a22e0d510238ba44e5d96f561045ec1), ROM_BIOS(3)) // 24C08W6, Same content as "pp_99_dongle_r1"

	// Quantum Fireball EX3.2A
	// C/H/S: 3.2 - 6256/16/63
	// Funworld label: 09.02.1999
	DISK_REGION( "pci:05.0:ide1:0:hdd" )
	DISK_IMAGE( "photoplay99sp", 0, BAD_DUMP SHA1(887e5b8c931d6122a1c3a8eda5cb919eb162eced) ) // From an operated HDD. A clean one must be recreated from the CDs
ROM_END

// BIOS not provided on this set
ROM_START(photoply99nl)
	ROM_REGION(0x20000, "pci:05.0", 0) // Motherboard BIOS
	ROM_LOAD("funworld_award_486e_w83787.bin", 0x000000, 0x20000, BAD_DUMP CRC(af7ff1d4) SHA1(72eeecf798a03817ce7ba4d65cd4128ed3ef7e68) ) // 486E 96/7/19 W83787 PLUG & PLAY BIOS, AT29C010A

	ROM_REGION(0x8000, "ex_bios", ROMREGION_ERASE00 )
	ROM_LOAD("enhanced_bios_1.06.u13", 0x0000, 0x8000, CRC(d05e9d20) SHA1(854501b7b3bf988b10516109d058f7ca2aa07d3e) ) // Centos Combo I/O ROM BIOS for CI-8000/PP2000 v1.06, W27E257

	DISK_REGION( "pci:05.0:ide1:0:hdd" )
	DISK_IMAGE( "photoplay99nl", 0, BAD_DUMP SHA1(e3ff2a64f51e0ba07d08cd49cd56cdc866401b4f) ) // Recreated from the CDs using a VM
ROM_END

ROM_START(photoply2k)
	ROM_REGION(0x20000, "pci:05.0", 0) // Motherboard BIOS
	ROM_LOAD("funworld_award_486e_w83787_alt.bin", 0x000000, 0x20000, CRC(e96d1bbc) SHA1(64d0726c4e9ecee8fddf4cc39d92aecaa8184d5c) ) // 486E 96/7/19 W83787 PLUG & PLAY BIOS (same string as 'photoply99sp' and 'photoply99sp' BIOSes, but different hash)

	ROM_REGION(0x8000, "ex_bios", ROMREGION_ERASE00 ) // Multifunction board with a ESS AudioDrive chip
	ROM_LOAD("enhanced bios.bin", 0x000000, 0x4000, CRC(a216404e) SHA1(c9067cf87d5c8106de00866bb211eae3a6c02c65) ) // Centos Combo I/O ROM BIOS for CI-8000/PP2000 v1.06, M27128A
//  ROM_RELOAD(                   0x004000, 0x4000 )
//  ROM_RELOAD(                   0x008000, 0x4000 )
//  ROM_RELOAD(                   0x00c000, 0x4000 )

	DISK_REGION( "pci:05.0:ide1:0:hdd" )
	DISK_IMAGE( "pp201", 0, BAD_DUMP SHA1(23e1940d485d19401e7d0ad912ddad2cf2ea10b4) )
ROM_END

ROM_START(photoply2ksp)
	ROM_REGION(0x20000, "pci:05.0", 0) // Motherboard BIOS
	ROM_LOAD("funworld_award_486e_w83787.bin", 0x000000, 0x20000, CRC(af7ff1d4) SHA1(72eeecf798a03817ce7ba4d65cd4128ed3ef7e68) ) // 486E 96/7/19 W83787 PLUG & PLAY BIOS, AT27C010

	ROM_REGION(0x8000, "ex_bios", ROMREGION_ERASE00 ) // Multifunction board with a ESS AudioDrive chip
	ROM_LOAD("enhanced_bios_centos.bin", 0x0000, 0x8000, CRC(ee8ad003) SHA1(4814385117599a98da02155785d1e3fce4e485bd) ) // Centos CI-8000/PP2000 ROM BIOS Version 1.06, 27C256B

	/* The Photo Play 2000 parallel port dongle contains, under resin:
	   Unknown MCU labeled "MARX(C)95,97 CBN/V/S" (UNDUMPED)
	   74HC00 */
	ROM_REGION(0x800, "dongle", 0)
	ROM_LOAD("marx_cbn-v-s.bin", 0x000, 0x800, NO_DUMP ) // Size unknown

	// Western Digital PhD1000-00H
	// CHS: 2100,16,63
	DISK_REGION( "pci:05.0:ide1:0:hdd" )
	DISK_IMAGE( "photoplay2ksp", 0, BAD_DUMP SHA1(2b4b837d85bf8a41d832533afb9363fdb16f7a30) ) // From an operated HDD. A clean one must be recreated from the CDs
ROM_END

// BIOS not provided, might be different
ROM_START(photoply2knl)
	ROM_REGION(0x20000, "pci:05.0", 0) // Motherboard BIOS
	ROM_LOAD("funworld_award_486e_w83787.bin", 0x000000, 0x20000, BAD_DUMP CRC(af7ff1d4) SHA1(72eeecf798a03817ce7ba4d65cd4128ed3ef7e68) ) // 486E 96/7/19 W83787 PLUG & PLAY BIOS, AT27C010

	ROM_REGION(0x8000, "ex_bios", ROMREGION_ERASE00 ) // Multifunction board with a ESS AudioDrive chip
	ROM_LOAD("enhanced_bios_centos.bin", 0x0000, 0x8000, CRC(ee8ad003) SHA1(4814385117599a98da02155785d1e3fce4e485bd) ) // Centos CI-8000/PP2000 ROM BIOS Version 1.06, 27C256B

	/* The Photo Play 2000 parallel port dongle contains, under resin:
	   Unknown MCU labeled "MARX(C)95,97 CBN/V/S" (UNDUMPED)
	   74HC00 */
	ROM_REGION(0x800, "dongle", 0)
	ROM_LOAD("marx_cbn-v-s.bin", 0x000, 0x800, NO_DUMP ) // Size unknown

	DISK_REGION( "pci:05.0:ide1:0:hdd" )
	DISK_IMAGE( "photoplay2knl", 0, BAD_DUMP SHA1(75aa190913e798d04db88325006c61965f5034ef) ) // Recreated from the CDs using a VM
ROM_END

// BIOS not provided, might be different
ROM_START(photoply2k1it)
	ROM_REGION(0x20000, "pci:05.0", 0) // Motherboard BIOS
	ROM_LOAD("funworld_award_486e_w83787.bin", 0x000000, 0x20000, BAD_DUMP CRC(af7ff1d4) SHA1(72eeecf798a03817ce7ba4d65cd4128ed3ef7e68) ) // 486E 96/7/19 W83787 PLUG & PLAY BIOS, AT27C010

	ROM_REGION(0x8000, "ex_bios", ROMREGION_ERASE00 ) // Multifunction board with a ESS AudioDrive chip
	ROM_LOAD("enhanced_bios_centos.bin", 0x000000, 0x8000, CRC(ee8ad003) SHA1(4814385117599a98da02155785d1e3fce4e485bd) ) // Centos CI-8000/PP2000 ROM BIOS Version 1.06, 27C256B

	DISK_REGION( "pci:05.0:ide1:0:hdd" )
	DISK_IMAGE( "photoplay2k1it", 0, BAD_DUMP SHA1(274ea0ebc051d0f4846bc58a039d342241b4cc28) ) // Manually rebuilt by adding the resources for the folder C:\QP_MSTR from the 2001_NL version

	// Recovery discs for Photo Play 2001

	/* UPDATE 2001 - Disc 1
	   Info on the CD ring:
	     A0100344917-0102  35  A1
	     IFPI L555
	     Sony DADC
	     Inner ring, laser engraved: IFPI 94Z5
	*/
	DISK_REGION( "recover_upd2k1_d1" )
	DISK_IMAGE_READONLY( "update_2001_cd1", 0, SHA1(c271c03ba8118203b9790dd924aac77ad68c2b17) )

	/* UPDATE 2001 - Disc 2
	   Info on the CD ring:
	     A0100344917-0202  13  B1
	     IFPI L553
	     Sony DADC
	     Inner ring, laser engraved: IFPI 94W6
	*/
	DISK_REGION( "recover_upd2k1_d2" )
	DISK_IMAGE_READONLY( "update_2001_cd2", 0, SHA1(c5797d6a342407136af49443401e83fe9578f5f2) )

	/* SNAKE II Service Release Update 2001
	   Info on the CD ring:
	     A0100360706-0101  15  A3
	     IFPI L555
	     Sony DADC
	*/
	DISK_REGION( "recover_snake2_upd2k1_d1" )
	DISK_IMAGE_READONLY( "nokia_snake_ii_service_release_update_2001", 0, SHA1(09286cb8b63d3cc771cbbf4b9b3de77e3d15bb7b) )
ROM_END

// BIOS not provided, might be different
ROM_START(photoply2k1nl)
	ROM_REGION(0x20000, "pci:05.0", 0) // Motherboard BIOS
	ROM_LOAD("funworld_award_486e_w83787.bin", 0x000000, 0x20000, BAD_DUMP CRC(af7ff1d4) SHA1(72eeecf798a03817ce7ba4d65cd4128ed3ef7e68) ) // 486E 96/7/19 W83787 PLUG & PLAY BIOS, AT27C010

	ROM_REGION(0x8000, "ex_bios", ROMREGION_ERASE00 ) // Multifunction board with a ESS AudioDrive chip
	ROM_LOAD("enhanced_bios_centos.bin", 0x0000, 0x8000, CRC(ee8ad003) SHA1(4814385117599a98da02155785d1e3fce4e485bd) ) // Centos CI-8000/PP2000 ROM BIOS Version 1.06, 27C256B

	DISK_REGION( "pci:05.0:ide1:0:hdd" )
	DISK_IMAGE( "photoplay2k1nl", 0, BAD_DUMP SHA1(87c9417119e9566f65db0f1b0f2182db7712c634) ) // Recreated from the CDs using a VM

	// Recovery discs for Photo Play 2001

	/* UPDATE 2001 - Disc 1
	   Info on the CD ring:
	     A0100344917-0102  35  A1
	     IFPI L555
	     Sony DADC
	     Inner ring, laser engraved: IFPI 94Z5
	*/
	DISK_REGION( "recover_upd2k1_d1" )
	DISK_IMAGE_READONLY( "update_2001_cd1", 0, SHA1(c271c03ba8118203b9790dd924aac77ad68c2b17) )

	/* UPDATE 2001 - Disc 2
	   Info on the CD ring:
	     A0100344917-0202  13  B1
	     IFPI L553
	     Sony DADC
	     Inner ring, laser engraved: IFPI 94W6
	*/
	DISK_REGION( "recover_upd2k1_d2" )
	DISK_IMAGE_READONLY( "update_2001_cd2", 0, SHA1(c5797d6a342407136af49443401e83fe9578f5f2) )

	/* SNAKE II Service Release Update 2001
	   Info on the CD ring:
	     A0100360706-0101  15  A3
	     IFPI L555
	     Sony DADC
	*/
	DISK_REGION( "recover_snake2_upd2k1_d1" )
	DISK_IMAGE_READONLY( "nokia_snake_ii_service_release_update_2001", 0, SHA1(09286cb8b63d3cc771cbbf4b9b3de77e3d15bb7b) )
ROM_END

// BIOS not provided, might be different
ROM_START(photoply2k1mtnl)
	ROM_REGION(0x20000, "pci:05.0", 0) // Motherboard BIOS
	ROM_LOAD("funworld_award_486e_w83787.bin", 0x000000, 0x20000, BAD_DUMP CRC(af7ff1d4) SHA1(72eeecf798a03817ce7ba4d65cd4128ed3ef7e68) ) // 486E 96/7/19 W83787 PLUG & PLAY BIOS, AT27C010

	ROM_REGION(0x8000, "ex_bios", ROMREGION_ERASE00 ) // Multifunction board with a ESS AudioDrive chip
	ROM_LOAD("enhanced_bios_centos.bin", 0x0000, 0x8000, CRC(ee8ad003) SHA1(4814385117599a98da02155785d1e3fce4e485bd) ) // Centos CI-8000/PP2000 ROM BIOS Version 1.06, 27C256B

	DISK_REGION( "pci:05.0:ide1:0:hdd" )
	DISK_IMAGE( "photoplay2k1mtnl", 0, BAD_DUMP SHA1(cfa25ce036be9c2379a104a3b50d2aefd851ceeb) ) // Recreated from the CDs using a VM
ROM_END

// BIOS not provided, might be different
ROM_START(photoply2k2be)
	ROM_REGION(0x20000, "pci:05.0", 0) // Motherboard BIOS
	ROM_LOAD("funworld_award_486e_w83787.bin", 0x000000, 0x20000, BAD_DUMP CRC(af7ff1d4) SHA1(72eeecf798a03817ce7ba4d65cd4128ed3ef7e68) ) // 486E 96/7/19 W83787 PLUG & PLAY BIOS, AT27C010

	ROM_REGION(0x8000, "ex_bios", ROMREGION_ERASE00 ) // Multifunction board with a ESS AudioDrive chip
	ROM_LOAD("enhanced_bios_centos.bin", 0x0000, 0x8000, CRC(ee8ad003) SHA1(4814385117599a98da02155785d1e3fce4e485bd) ) // Centos CI-8000/PP2000 ROM BIOS Version 1.06, 27C256B

	DISK_REGION( "pci:05.0:ide1:0:hdd" )
	DISK_IMAGE( "photoplay2k2be", 0, BAD_DUMP SHA1(29719b5db60f4cc3787a3a9f6a5937226e282d46) ) // Recreated from the CDs using a VM
ROM_END

// BIOS not provided, might be different
ROM_START(photoply2k4)
	ROM_REGION(0x20000, "pci:05.0", 0) // Motherboard BIOS
	ROM_LOAD("funworld_award_486e_w83787_alt.bin", 0x000000, 0x20000, BAD_DUMP CRC(e96d1bbc) SHA1(64d0726c4e9ecee8fddf4cc39d92aecaa8184d5c) ) // 486E 96/7/19 W83787 PLUG & PLAY BIOS (same string as 'photoply99sp' and 'photoply99sp' BIOSes, but different hash)

	ROM_REGION(0x8000, "ex_bios", ROMREGION_ERASE00 ) // Multifunction board with a ESS AudioDrive chip, M27128A
	ROM_LOAD("enhanced bios.bin", 0x000000, 0x4000, BAD_DUMP CRC(a216404e) SHA1(c9067cf87d5c8106de00866bb211eae3a6c02c65) )
//  ROM_RELOAD(                   0x004000, 0x4000 )
//  ROM_RELOAD(                   0x008000, 0x4000 )
//  ROM_RELOAD(                   0x00c000, 0x4000 )

	DISK_REGION( "pci:05.0:ide1:0:hdd" )
	// CYLS:1023,HEADS:64,SECS:63,BPS:512.
	DISK_IMAGE( "pp2004", 0, SHA1(a3f8861cf91cf7e7446ec931f812e774ada20802) )
ROM_END

} // anonymous namespace


GAME( 1998, photoply98sp,    0,             photoply,         photoply, photoply_state, empty_init, ROT0, "Funworld", "Photo Play 1998 (Spain)",               MACHINE_NOT_WORKING|MACHINE_NO_SOUND|MACHINE_UNEMULATED_PROTECTION ) // "Non system disk or I/O error"
GAME( 1999, photoply99sp,    0,             photoply_dx4_100, photoply, photoply_state, empty_init, ROT0, "Funworld", "Photo Play 1999 (Spain)",               MACHINE_NOT_WORKING|MACHINE_NO_SOUND|MACHINE_UNEMULATED_PROTECTION ) // "CON device not opened. System halted" before PTS-DOS
GAME( 1999, photoply99nl,    photoply99sp,  photoply_dx4_100, photoply, photoply_state, empty_init, ROT0, "Funworld", "Photo Play 1999 (Netherlands)",         MACHINE_NOT_WORKING|MACHINE_NO_SOUND|MACHINE_UNEMULATED_PROTECTION ) // Boots to funworld logo, goes to touchscreen calibration menu
GAME( 2000, photoply2k,      0,             photoply_dx4_100, photoply, photoply_state, empty_init, ROT0, "Funworld", "Photo Play 2000 (v2.01)",               MACHINE_NOT_WORKING|MACHINE_NO_SOUND|MACHINE_UNEMULATED_PROTECTION ) // Fails PTS-DOS bootstrap around PC=7dc5, likely bad dump
GAME( 2000, photoply2ksp,    photoply2k,    photoply_dx4_100, photoply, photoply_state, empty_init, ROT0, "Funworld", "Photo Play 2000 (Spain)",               MACHINE_NOT_WORKING|MACHINE_NO_SOUND|MACHINE_UNEMULATED_PROTECTION ) // Attract mode
GAME( 2000, photoply2knl,    photoply2k,    photoply_dx4_100, photoply, photoply_state, empty_init, ROT0, "Funworld", "Photo Play 2000 (Netherlands)",         MACHINE_NOT_WORKING|MACHINE_NO_SOUND|MACHINE_UNEMULATED_PROTECTION ) // Punts in PTS-DOS after failing ES1868/microtouch detection, has diag inside for latter
GAME( 2001, photoply2k1it,   0,             photoply_dx4_100, photoply, photoply_state, empty_init, ROT0, "Funworld", "Photo Play 2001 (Italy)",               MACHINE_NOT_WORKING|MACHINE_NO_SOUND|MACHINE_UNEMULATED_PROTECTION ) // Attract mode
GAME( 2001, photoply2k1nl,   photoply2k1it, photoply_dx4_100, photoply, photoply_state, empty_init, ROT0, "Funworld", "Photo Play 2001 (Netherlands)",         MACHINE_NOT_WORKING|MACHINE_NO_SOUND|MACHINE_UNEMULATED_PROTECTION ) // "Non system disk or I/O error"
GAME( 2001, photoply2k1mtnl, photoply2k1it, photoply_dx4_100, photoply, photoply_state, empty_init, ROT0, "Funworld", "Photo Play Masters 2001 (Netherlands)", MACHINE_NOT_WORKING|MACHINE_NO_SOUND|MACHINE_UNEMULATED_PROTECTION ) // Fails touchscreen and ES1868 detection, executes few seconds of attract mode then throws a red screen with "HDONGLE not found"
GAME( 2002, photoply2k2be,   0,             photoply_dx4_100, photoply, photoply_state, empty_init, ROT0, "Funworld", "Photo Play 2002 (Belgium)",             MACHINE_NOT_WORKING|MACHINE_NO_SOUND|MACHINE_UNEMULATED_PROTECTION ) // Shows some sort of rebus indicating correct operation of touchscreen, hangs there
GAME( 2004, photoply2k4,     0,             photoply_dx4_100, photoply, photoply_state, empty_init, ROT0, "Funworld", "Photo Play 2004",                       MACHINE_NOT_WORKING|MACHINE_NO_SOUND|MACHINE_UNEMULATED_PROTECTION ) // Attract mode, "I.G.O. Edition 4" with Nederlands/English options
