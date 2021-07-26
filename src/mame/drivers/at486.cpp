// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic
/***************************************************************************

IBM AT compatibles using a 486 class CPU
split from at.cpp

***************************************************************************/

#include "emu.h"

/* mingw-gcc defines this */
#ifdef i386
#undef i386
#endif /* i386 */

#include "bus/isa/isa_cards.h"
#include "bus/lpci/pci.h"
#include "bus/lpci/vt82c505.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "cpu/i386/i386.h"
#include "machine/at.h"
#include "machine/cs8221.h"
#include "machine/ds128x.h"
#include "machine/idectrl.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "machine/vt82c496.h"
#include "emupal.h"
#include "softlist_dev.h"
#include "speaker.h"

class at486_state : public driver_device
{
public:
	at486_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mb(*this, "mb"),
		m_ram(*this, RAM_TAG)
	{ }

	void at486(machine_config &config);
	void at486l(machine_config &config);
	void ficpio2(machine_config &config);

	void init_at486();
	void init_at486pci();

protected:
	required_device<cpu_device> m_maincpu;
	required_device<at_mb_device> m_mb;
	required_device<ram_device> m_ram;

	void init_at486_common(int xmsbase);

	void at486_io(address_map &map);
	void at486_map(address_map &map);
	void at486l_map(address_map &map);
	void ficpio_io(address_map &map);
	void ficpio_map(address_map &map);
};

void at486_state::at486_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x0009ffff).bankrw("bank10");
	map(0x000e0000, 0x000fffff).rom().region("bios", 0);
	map(0x00800000, 0x00800bff).ram().share("nvram");
	map(0xfffe0000, 0xffffffff).rom().region("bios", 0);
}

void at486_state::at486_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).m(m_mb, FUNC(at_mb_device::map));
}

void at486_state::ficpio_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x0009ffff).bankrw("bank10");
	map(0x00800000, 0x00800bff).ram().share("nvram");
	map(0xfffe0000, 0xffffffff).rom().region("isa", 0x20000);
}

void at486_state::ficpio_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).m(m_mb, FUNC(at_mb_device::map));
	map(0x00a8, 0x00af).rw("chipset", FUNC(vt82c496_device::read), FUNC(vt82c496_device::write));
	map(0x0170, 0x0177).rw("ide2", FUNC(ide_controller_32_device::cs0_r), FUNC(ide_controller_32_device::cs0_w));
	map(0x01f0, 0x01f7).rw("ide", FUNC(ide_controller_32_device::cs0_r), FUNC(ide_controller_32_device::cs0_w));
	map(0x0370, 0x0377).rw("ide2", FUNC(ide_controller_32_device::cs1_r), FUNC(ide_controller_32_device::cs1_w));
	map(0x03f0, 0x03f7).rw("ide", FUNC(ide_controller_32_device::cs1_r), FUNC(ide_controller_32_device::cs1_w));
	map(0x0cf8, 0x0cff).rw("pcibus", FUNC(pci_bus_device::read), FUNC(pci_bus_device::write));
}

void at486_state::at486l_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x0009ffff).bankrw("bank10");
	map(0x000e0000, 0x000fffff).rom().region("bios", 0x20000);
	map(0x00800000, 0x00800bff).ram().share("nvram");
	map(0xfffe0000, 0xffffffff).rom().region("bios", 0x20000);
}


/**********************************************************
 Init functions
**********************************************************/

void at486_state::init_at486_common(int xmsbase)
{
	address_space& space = m_maincpu->space(AS_PROGRAM);

	/* MESS managed RAM */
	membank("bank10")->set_base(m_ram->pointer());

	if (m_ram->size() > xmsbase)
	{
		offs_t ram_limit = 0x100000 + m_ram->size() - xmsbase;
		space.install_ram(0x100000,  ram_limit - 1, m_ram->pointer() + xmsbase);
	}
}

void at486_state::init_at486()
{
	init_at486_common(0xa0000);
}

void at486_state::init_at486pci()
{
	init_at486_common(0x100000);
}

static void pci_devices(device_slot_interface &device)
{
	device.option_add_internal("vt82c505", VT82C505);
}


/**********************************************************
 Machine configurations
**********************************************************/

void at486_state::at486(machine_config &config)
{
	i486_device &maincpu(I486(config, m_maincpu, 25'000'000));
	maincpu.set_addrmap(AS_PROGRAM, &at486_state::at486_map);
	maincpu.set_addrmap(AS_IO, &at486_state::at486_io);
	maincpu.set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));

	config.set_maximum_quantum(attotime::from_hz(60));

	AT_MB(config, m_mb).at_softlists(config);
	m_mb->kbd_clk().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	m_mb->kbd_data().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// on-board devices
	ISA16_SLOT(config, "board1", 0, "mb:isabus", pc_isa16_cards, "fdcsmc", true); // FIXME: deteremine ISA bus clock
	ISA16_SLOT(config, "board2", 0, "mb:isabus", pc_isa16_cards, "comat", true);
	ISA16_SLOT(config, "board3", 0, "mb:isabus", pc_isa16_cards, "ide", true);
	ISA16_SLOT(config, "board4", 0, "mb:isabus", pc_isa16_cards, "lpt", true);
	// ISA cards
	ISA16_SLOT(config, "isa1", 0, "mb:isabus", pc_isa16_cards, "svga_et4k", false);
	ISA16_SLOT(config, "isa2", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa4", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa5", 0, "mb:isabus", pc_isa16_cards, nullptr, false);

	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL));
	pc_kbdc.out_clock_cb().set(m_mb, FUNC(at_mb_device::kbd_clk_w));
	pc_kbdc.out_data_cb().set(m_mb, FUNC(at_mb_device::kbd_data_w));

	/* internal ram */
	RAM(config, m_ram).set_default_size("4M").set_extra_options("1M,2M,8M,16M,20M,32M,64M,128M");
}

void at486_state::at486l(machine_config &config)
{
	at486(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &at486_state::at486l_map);
}

void at486_state::ficpio2(machine_config &config)
{
	i486_device &maincpu(I486(config, m_maincpu, 25'000'000));
	maincpu.set_addrmap(AS_PROGRAM, &at486_state::ficpio_map);
	maincpu.set_addrmap(AS_IO, &at486_state::ficpio_io);
	maincpu.set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));

	config.set_maximum_quantum(attotime::from_hz(60));

	AT_MB(config, m_mb).at_softlists(config);
	m_mb->kbd_clk().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	m_mb->kbd_data().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	ds12885_device &rtc(DS12885(config.replace(), "mb:rtc"));
	rtc.irq().set("mb:pic8259_slave", FUNC(pic8259_device::ir0_w)); // this is in :mb
	rtc.set_century_index(0x32);

	RAM(config, m_ram).set_default_size("4M").set_extra_options("1M,2M,8M,16M,32M,64M,128M");

	// on board devices
	ISA16_SLOT(config, "board1", 0, "mb:isabus", pc_isa16_cards, "fdcsmc", true); // FIXME: determine ISA bus clock
	ISA16_SLOT(config, "board2", 0, "mb:isabus", pc_isa16_cards, "comat", true);
	ISA16_SLOT(config, "board3", 0, "mb:isabus", pc_isa16_cards, "lpt", true);

	ide_controller_32_device &ide(IDE_CONTROLLER_32(config, "ide").options(ata_devices, "hdd", nullptr, true));
	ide.irq_handler().set("mb:pic8259_slave", FUNC(pic8259_device::ir6_w));
	ide_controller_32_device &ide2(IDE_CONTROLLER_32(config, "ide2").options(ata_devices, "cdrom", nullptr, true));
	ide2.irq_handler().set("mb:pic8259_slave", FUNC(pic8259_device::ir7_w));

	PCI_BUS(config, "pcibus", 0).set_busnum(0);
	PCI_CONNECTOR(config, "pcibus:0", pci_devices, "vt82c505", true);
	ISA16_SLOT(config, "isa1", 0, "mb:isabus", pc_isa16_cards, "svga_et4k", false);
	ISA16_SLOT(config, "isa2", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa4", 0, "mb:isabus", pc_isa16_cards, nullptr, false);

	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL));
	pc_kbdc.out_clock_cb().set(m_mb, FUNC(at_mb_device::kbd_clk_w));
	pc_kbdc.out_data_cb().set(m_mb, FUNC(at_mb_device::kbd_data_w));

	vt82c496_device &chipset(VT82C496(config, "chipset"));
	chipset.set_cputag(m_maincpu);
	chipset.set_ramtag(m_ram);
	chipset.set_isatag("isa");
}

/***************************************************************************
  ROM definitions
***************************************************************************/

/**************************************************************************
  IBM systems
***************************************************************************/

// https://en.wikipedia.org/wiki/IBM_PS/1
// http://ps-2.kev009.com/pcpartnerinfo/ctstips/937e.htm
// https://ps1stuff.wordpress.com/documentation/ibm-ps1-model-2011/
// https://barotto.github.io/IBMulator/#download

// From Wikipedia:
// 2133 Desktop case. The 3x3 references the available slots and drive bays.
// 2155 Desktop case larger than 2133. The 5x5 references the available slots and drive bays. Including a 5.25" bay.
// 2168 Tower unit. The 6x8 references the available slots and bays. Including 5.25" bays.
// Model     MB FRU      CPU                        RAM   SIMM          Video chip   VRAM   Hard-Drive          Notes
// 2133-711  93F2397  Intel 80386SX @ 25 MHz    2 MB  2×72 Pin FPM                   256KB  59G9567  85MB IDE
// 2133-811          Intel 80386SX @ 25 MHz     4 MB                                                 85MB IDE
// 2133-13   ???      Intel 80386SX @ 25 MHz    2 MB  2x72 Pin FPM                   256KB
// 2133-W13          Intel 80386SX @ 25 MHz     2 MB                                                129MB IDE
// 2133-13T  65G3766  Intel 80486SX @ 25 MHz    4 MB  2×72 Pin FPM                   256KB  93F2329 129MB IDE
// 2133-?43  34G1885  Intel 80486SX @ 20 MHz    4 MB  2×30 Pin FPM                   512KB  93F2329 129MB IDE
// 2133-?50  34G1848  Intel 80486SX @ 25 MHz    4 MB  2×30 Pin FPM                   512KB  93F2329 129MB IDE
// 2133-?53  34G1848  Intel 80486SX @ 25 MHz    4 MB  2×30 Pin FPM                   512KB  93F2329 129MB IDE
// 2133-652          Intel 80486SX @ 33 MHz     4 MB  4×72 Pin FPM  Cirrus CL-GD5424 512KB  84G3927 171MB IDE
// 2133-575          Intel 80486DX @ 33 MHz     4 MB  4×72 Pin FPM                   512KB          170MB IDE
// 2133-594          Intel 80486DX2 @66 MHz     4 MB  4×72 Pin FPM                   512KB          253MB IDE
// 2133-E11          Intel 80386SX @ 25 MHz     2 MB  2×72 Pin FPM  Cirrus CL-GD5424 512 KB          85MB IDE   Canada models, English model
// 2133-F11          Intel 80386SX @ 25 MHz     2 MB  2×72 Pin FPM  Cirrus CL-GD5424 512 KB          85MB IDE   Canada models, French model
// 2133-E43          Intel 80486SX @ 20 MHz     2 MB  8×30 Pin FPM  Tseng ET4000     512KB          129MB IDE   Canada models, English model
// 2133-F43          Intel 80486SX @ 20 MHz     2 MB  8×30 Pin FPM  Tseng ET4000     512KB          129MB IDE   Canada models, French model
// 2133-E53          Intel 80486SX @ 25 MHz     2 MB  8×30 Pin FPM  Tseng ET4000     512KB          129MB IDE   Canada models, English model
// 2133-F53          Intel 80486SX @ 25 MHz     2 MB  8×30 Pin FPM  Tseng ET4000     512KB          129MB IDE   Canada models, French model

ROM_START( ibm2133 )
	ROM_REGION32_LE( 0x20000, "bios", 0 )
	ROM_LOAD( "ps1_2133_52g2974_rom.bin", 0x00000, 0x20000, CRC(89fc7600) SHA1(758e161353f6781c39ac67f1ba293c14038b17dc))
ROM_END

/**************************************************************************
  Apricot systems

  http://bbs.actapricot.org/files/ , http://insight.actapricot.org/insight/products/main.htm

**************************************************************************/

// Apricot LS Pro (Bonsai Motherboard, on board: ethernet (Intel 82596), Chipset: VLSI SCAMP VL82C311 / VL82C333, ROM: 128KB)
ROM_START( aplsbon )
	ROM_REGION32_LE(0x20000 ,"bios", 0)
	// 0: BIOS-String: Phoenix BIOS A486 Version 1.01 / LS Pro BIOS Version 1.06, 4th July 1994 - Pointer device failure
	ROM_SYSTEM_BIOS(0, "bon106", "Bonsai 1-06")
	ROMX_LOAD( "bon106.bin",   0x00000, 0x20000, CRC(98a4eb76) SHA1(e0587afa78aeb9a8803f9b9f9e457e9847b0a2b2), ROM_BIOS(0))
	// 1: flashing screen
	ROM_SYSTEM_BIOS(1, "bon203", "Bonsai 2-03")
	ROMX_LOAD( "bon203.bin",   0x00000, 0x20000, CRC(32a0e125) SHA1(a4fcbd76952599993fa8b76aa36a96386648abb2), ROM_BIOS(1))
	// 2: BIOS-String: Phoenix BIOS A486 Version 1.01 / LS Pro BIOS Version 1.07.03, 2nd February 1995
	ROM_SYSTEM_BIOS(2, "bon10703", "Bonsai 1-07-03")
	ROMX_LOAD( "bon10703.bin",   0x00000, 0x20000, CRC(0275b3c2) SHA1(55ef4cbb7f3166f678aaa478234a42049deaba5f), ROM_BIOS(2))
	// 3: flashing screen
	ROM_SYSTEM_BIOS(3, "bon20402", "Bonsai 2.03")
	ROMX_LOAD( "bon20402.bin",   0x00000, 0x20000, CRC(ac5803fb) SHA1(b8fe92711c6a38a5d9e6497e76a0929c1685c631), ROM_BIOS(3))
ROM_END

// Apricot LS Pro (Caracal Motherboard,Chipset: VLSI VL82C483, ROM: 256KB Flash ROM, PCMCIA Type 2/3 slots)
ROM_START( aplscar )
	ROM_REGION32_LE(0x40000, "bios", 0)
	// 0: MAME exits with "Fatal error: i386: Called modrm_to_EA with modrm value C8!"
	ROM_SYSTEM_BIOS(0, "car306", "Caracal 3.06")
	ROMX_LOAD( "car306.bin",   0x00000, 0x40000, CRC(fc271dea) SHA1(6207cfd312c9957243b8157c90a952404e43b237), ROM_BIOS(0))
	// 1: no display
	ROM_SYSTEM_BIOS(1, "car307", "Caracal 3.07")
	ROMX_LOAD( "car307.bin",   0x00000, 0x40000, CRC(66a01852) SHA1(b0a68c9d67921d27ba483a1c50463406c08d3085), ROM_BIOS(1))
ROM_END

// Apricot FT//ex 486 (J3 Motherboard, Chipset: Opti 82C696)
ROM_START( aprfte ) // no display
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "1-2r2-4.486", 0x00000, 0x20000, CRC(bccc236d) SHA1(0765299363e68cf65710a688c360a087856ece8f))
ROM_END

// Apricot FTs (Panther Rev F 1.02.26)
ROM_START( aprpand ) // no display
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "pf10226.std", 0x00000, 0x10000, CRC(7396fb87) SHA1(a109cbad2179eec55f86c0297a59bb015461da21))
	ROM_CONTINUE( 0x00001, 0x10000 )
ROM_END

// Apricot VX FT server - no display
ROM_START( apvxft )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "ft10221.lo", 0x00000, 0x10000, CRC(8f339de0) SHA1(a6542406746eaf1ff7f9e3678c5cbe5522fb314a))
	ROM_LOAD16_BYTE( "ft10221.hi", 0x00001, 0x10000, CRC(3b16bc31) SHA1(0592d1d81e7fd4715b0612083482db122d78c7f2))
ROM_END

// Apricot XEN PC (A1 Motherboard) - no display
ROM_START( apxena1 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "a1-r26.bin",   0x00000, 0x20000, CRC(d29e983e) SHA1(5977df7f8d7ac2a154aa043bb6f539d96d51fcad))
ROM_END

// Apricot XEN-LS (Venus IV Motherboard) - no display
ROM_START( apxenls3 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "31020.lo", 0x10000, 0x8000, CRC(a19678d2) SHA1(d13c12fa7e94333555eabf58b81bad421e21cd91))
	ROM_LOAD16_BYTE( "31020.hi", 0x10001, 0x8000, CRC(4922e020) SHA1(64e6448323dad2209e004cd93fa181582e768ed5))
ROM_END

// Apricot XEN PC (P2 Motherboard, Chipset: M1429G/31, ROM: 128KB Flash ROM, on board: graphics Cirrus Logic GD5434 (via VL))
ROM_START( apxenp2 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: ACR97E00-M00-951005-R02-G2 / BIOS V2.0 - Keyboard Interface Error - Pointing DeviceInterface Error
	// after a while the boot continues to the message "Password Violated, System Halted !"
	ROM_SYSTEM_BIOS(0, "p2r02g2", "p2r02g2")
	ROMX_LOAD( "p2r02g2.bin",   0x00000, 0x20000, CRC(311bcc5a) SHA1(be6fa144322077dcf66b065e7f4e61aab8c278b4), ROM_BIOS(0))
	// 1: BIOS-String: ACR97E00-M00-951005-R01-F0 / BIOS V2.0 (error messages as above)
	ROM_SYSTEM_BIOS(1, "lep121s", "SCSI-Enabling ROMs")
	ROMX_LOAD("p2r01f0.bin",   0x00000, 0x20000, CRC(bbc68f2e) SHA1(6954a52a7dda5521794151aff7a04225e9c7df77), ROM_BIOS(1))
ROM_END

// Apricot XEN-LS II (Samurai Motherboard, on board: CD-ROM, graphics, ethernet (Intel 82596), Chipset: VLSI 82C425, VLSI 82C486)
ROM_START( apxlsam ) // no display
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "sam107", "ROM BIOS Version 1-07")
	ROMX_LOAD( "sam1-07.bin",   0x00000, 0x20000, CRC(65e05a8e) SHA1(c3cd198a129122cb05a28798e54331b06cfdd310), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "sam206", "ROM BIOS Version 2-06")
	ROMX_LOAD( "sam2-06.bin",   0x00000, 0x20000, CRC(9768bb0f) SHA1(8166b77b133072f72f23debf85984eb19578ffc1), ROM_BIOS(1))
ROM_END

// Apricot FTs (Scorpion) - no display, beep code L-1-1-3 (Extended CMOS RAM failure)
ROM_START( ftsserv )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "fts10226.lo", 0x00000, 0x10000, CRC(efbd738f) SHA1(d5258760bafdaf1bf13c4a49da76d4b5e7b4ccbd))
	ROM_LOAD16_BYTE( "fts10226.hi", 0x00001, 0x10000, CRC(2460853f) SHA1(a6bba8d2f800140afd129c4d5278f7ae8fe7e63a))
	/* FT Server series Front Panel */
	ROM_REGION(0x10000,"front", 0)
	ROM_LOAD( "fp10009.bin",     0x0000, 0x8000, CRC(8aa7f718) SHA1(9ee6c6a5bb92622ea8d3805196d42ff68887d820))
ROM_END

// Apricot Qi 900 (Scorpion Motherboard)  - no display, beep code L-1-1-3 (Extended CMOS RAM failure)
ROM_START( qi900 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "qi910224.lo", 0x00000, 0x10000, CRC(b012ad3c) SHA1(807e788a6bd03f5e983fe503af3d0b202c754b8a))
	ROM_LOAD16_BYTE( "qi910224.hi", 0x00001, 0x10000, CRC(36e66d56) SHA1(0900c5272ec3ced550f18fb08db59ab7f67a621e))
ROM_END


/***************************************************************************
  Commodore systems
***************************************************************************/

// Commodore Tower 486
ROM_START( comt486 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 40-0500-DG1112-00101111-070791-SOLUTION-0 - 4D3FF Rev.D (092892)
	ROM_SYSTEM_BIOS(0, "v0", "Tower 486 V0")
	ROMX_LOAD( "cbm-t486dx-bios-v-xxxxxx-xx.bin", 0x10000, 0x10000, CRC(f51c0ca0) SHA1(2b08a606ae2f37b3e72d687f890d729a58fd3ccd), ROM_BIOS(0))
	// continuous chirps
	ROM_SYSTEM_BIOS(1, "v1", "Tower 486 V1")
	ROMX_LOAD( "cbm-t486dx-66-bios-v1.01-391566-02.bin", 0x10000, 0x10000, CRC(3d740698) SHA1(888f23d85b41c07e15e2811b76194cf478bc80cd), ROM_BIOS(1))
	// BIOS-String: 40-0103-001283-00101111-0606-SYM_486-0 - Commodore 486DX2-66 BIOS Version 1.03 391684-02
	ROM_SYSTEM_BIOS(2, "v2", "Tower 486 V2")
	ROMX_LOAD( "cbm-t486dx-66-bios-v1.03-391684-02.bin", 0x10000, 0x10000, CRC(13e8b04b) SHA1(dc5c84d228f802f7580b3f3b8e70cf8f74de5d79), ROM_BIOS(2))
	// BIOS-String: 40-0103-001283-00101111-060692-SYM_486-0 - Commodore 486DX-50 BIOS Version 1.03 391522-03
	ROM_SYSTEM_BIOS(3, "v3", "Tower 486 V3")
	ROMX_LOAD( "cbm-t486dx-50-bios-v1.03-.bin", 0x10000, 0x10000, CRC(e02bb928) SHA1(6ea121b214403390d382ca4685cfabcbcca1a28b), ROM_BIOS(3))
ROM_END

// Commodore DT486 - BIOS contains Paradise VGA ROM - Keyboard error
ROM_START( dt486 ) // BIOS string: 41-0102-001283-00111111-060692-SYM_486-0 - Commodore 486DX-33 BIOS Version 1.01 391521-02
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "dt486", "DT486")
	ROM_LOAD( "cbm-dt486dx-33c-bios-u32--v1.01-391521-02.bin", 0x00000, 0x20000, BAD_DUMP CRC(a3977625) SHA1(83bc563fb41eae3dd5d260f13c6fe8979a77e99c))
ROM_END

// Commodore PC-70-III - complaining "time-of-day-clock stopped"
ROM_START( pc70iii )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: Commodore 80486 BIOS Rev.1.00 - 390934-01/390935-01
	ROM_SYSTEM_BIOS(0, "pc70v100", "PC70 V1.00")
	ROMX_LOAD("cbm-pc70c_bios-u117-lo-v1.00-390934-01.bin", 0x00000, 0x10000, CRC(3eafd811) SHA1(4deecd5dc429ab09e7c0d308250cb716f8b8e42a), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("cbm-pc70c_bios-u112-hi-v1.00-390935-01.bin", 0x00001, 0x10000, CRC(2d1dfec9) SHA1(d799b3579577108549d9d4138a8a32c35ac3ce1c), ROM_SKIP(1) | ROM_BIOS(0))
	// 1: Commodore PC70-III 80486/25MHz BIOS Rev.1.00.01 - xxxxxx - 00/xxxxxx-00
	ROM_SYSTEM_BIOS(1, "pc70v101", "PC70 V1.00.01")
	ROMX_LOAD("cbm-pc70c-bios-lo-v1.00.01-xxxxxx-00.bin", 0x00000, 0x10000, CRC(6c8bbd31) SHA1(63d1739a58a0d441ebdd543e3994984c433aedb4), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("cbm-pc70c-bios-hi-v1.00.01-xxxxxx-00.bin", 0x00001, 0x10000, CRC(ef279cdd) SHA1(d250368b2f731e842d6f280a6134f1e38846874b), ROM_SKIP(1) | ROM_BIOS(1))
ROM_END


/***************************************************************************
  80486 BIOS

  BIOS files that show "original.tmp" near the beginning are compressed
  and can be unpacked using, e.g., 7-ZIP.
***************************************************************************/

ROM_START( at486 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 30-0500-ZZ1130-00101111-070791-1219-0 /PAI JUNG ELECTRONIC IND. CO., LTD.
	ROM_SYSTEM_BIOS(0, "at486", "PC/AT 486")
	ROMX_LOAD( "at486.bin",   0x10000, 0x10000, CRC(31214616) SHA1(51b41fa44d92151025fc9ad06e518e906935e689), ROM_BIOS(0))
	// 1: BIOS-String: 40-0100-009999-11101111-070791-UMC480A-0 / United Microelectronics Corporation (UMC) MG-48602
	ROM_SYSTEM_BIOS(1, "mg48602", "UMC MG-48602")
	ROMX_LOAD( "mg48602.bin", 0x10000, 0x10000, CRC(45797823) SHA1(a5fab258aecabde615e1e97af5911d6cf9938c11), ROM_BIOS(1))
	// 2: BIOS-String: 40-0000-001470-00101111-060692-SIS3486-0 / 24X-VS-XX-B
	ROM_SYSTEM_BIOS(2, "ft01232", "Free Tech 01-232")
	ROMX_LOAD( "ft01232.bin", 0x10000, 0x10000, CRC(30efaf92) SHA1(665c8ef05ca052dcc06bb473c9539546bfef1e86), ROM_BIOS(2))

	/* 486 boards from FIC
	naming convention
	xxxxx101 --> for EPROM
	xxxxx701 --> for EEPROM using a Flash Utility v5.02
	xxxxBxxx --> NS 311/312 IO Core Logic
	xxxxCxxx --> NS 332 IO Core Logic
	xxxxGxxx --> Winbond W83787F IO Core Logic
	xxxxJxxx --> Winbond W83877F IO Core Logic
	*/
	// 3: BIOS-String: 06/16/97-VT82C486A-214L2000-00 / Version 3.276GN1
	/* this is the year 2000 beta bios from FIC, supports GIO-VT, GAC-V, GAC-2, VIP-IO, VIO-VP and GVT-2 */
	ROM_SYSTEM_BIOS(3, "ficy2k", "FIC 486 3.276GN1") /* includes CL-GD5429 VGA BIOS 1.00a */
	ROMX_LOAD( "3276gn1.bin",  0x00000, 0x20000, CRC(d4ff0cc4) SHA1(567b6bdbc9bff306c8c955f275e01ae4c45fd5f2), ROM_BIOS(3))
	// 4: BIOS-String: 04/29/94-VT82C486A-214L2000-00 / Award Modular BIOS v4.50
	ROM_SYSTEM_BIOS(4, "ficgac2", "FIC 486-GAC-2") /* includes CL-GD542X VGA BIOS 1.50 */
	ROMX_LOAD( "att409be.bin", 0x00000, 0x20000, CRC(c58e017b) SHA1(14c19e720ce62eb2afe28a70f4e4ebafab0f9e77), ROM_BIOS(4))
	// 5: BIOS-String: 04/08/96-VT82C486A-214L2000-00 / Version 3.27GN1
	ROM_SYSTEM_BIOS(5, "ficgacv", "FIC 486-GAC-V 3.27GN1") /* includes CL-GD542X VGA BIOS 1.41 */
	ROMX_LOAD( "327gn1.awd",   0x00000, 0x20000, CRC(017614d4) SHA1(2228c28f21a7e78033d24319449297936465b164), ROM_BIOS(5))
	// 6: BIOS-String: 05/06/94-VT82C486A-214L2000-00 / Version 3.15GN - ISA16:4, ISA/VL: 2
	ROM_SYSTEM_BIOS(6, "ficgiovp", "FIC 486-GIO-VP 3.15GN") // Chipset: VIP VT82C486A, Promise PDC20230C, one further VIA, one other unreadable
	ROMX_LOAD( "giovp315.rom", 0x10000, 0x10000, CRC(e102c3f5) SHA1(f15a7e9311cc17afe86da0b369607768b030ddec), ROM_BIOS(6))
	// 7: BIOS-String: 11/20/94-VT82C486A-214L2000-00 / Version 3.06G (11/25/94) - OSC: 24.0L3P - ISA16:3, ISA/VL: 2
	ROM_SYSTEM_BIOS(7, "ficgiovt", "FIC 486-GIO-VT 3.06G") // 1994-11-20 - Chipset: Winbond W83757AF, W83758P, VIA VT82C486A, VT8255N, VT82C482
	ROMX_LOAD( "306gcd00.awd", 0x10000, 0x10000, CRC(75f3ded4) SHA1(999d4b58204e0b0f33262d0613c855b528bf9597), ROM_BIOS(7))
	// 8: BIOS-String: 11/02/94-VT82C486A-214L2000-00 Version 3.07G - ISA8: 1, ISA16: 4, ISA/VL: 2
	ROM_SYSTEM_BIOS(8, "ficgvt2", "FIC 486-GVT-2 3.07G") // Chipset: VIA VT82C486A, VT82C482, VIA VT8255N
	ROMX_LOAD( "3073.bin",     0x10000, 0x10000, CRC(a6723863) SHA1(ee93a2f1ec84a3d67e267d0a490029f9165f1533), ROM_BIOS(8))
	// 9: BIOS-String: 06/27/95-VT82C505-2A4L4000-00 / Version 5.15S - Chipset: S3 Trio64, FDC 37665GT, VT82C496G, VT82C406MV
	ROM_SYSTEM_BIOS(9, "ficgpak2", "FIC 486-PAK-2 5.15S") /* includes Phoenix S3 TRIO64 Enhanced VGA BIOS 1.4-01 */
	ROMX_LOAD( "515sbd8a.awd", 0x00000, 0x20000, CRC(778247e1) SHA1(07d8f0f2464abf507be1e8dfa06cd88737782411), ROM_BIOS(9))
	// 10: BIOS-String: 04/01/96-VT496G-2A4L6F0IC-00 0000C-00 - runs into Award BootBlock BIOS - Chipset: VIA VT82C505, VT82C416, VT82C496G, Winbond W83787F
	ROM_SYSTEM_BIOS(10, "ficpio3g7", "FIC 486-PIO-3 1.15G705") // pnp  - ISA16: 4, PCI: 3
	ROMX_LOAD( "115g705.awd",  0x00000, 0x20000, CRC(ddb1544a) SHA1(d165c9ecdc9397789abddfe0fef69fdf954fa41b), ROM_BIOS(10))
	// 11: BIOS-String: 04/01/96-VT496G-2A4L6F0IC-00 0000C-00 - runs into Award BootBlock BIOS
	ROM_SYSTEM_BIOS(11, "ficpio3g1", "FIC 486-PIO-3 1.15G105") /* non-pnp */
	ROMX_LOAD( "115g105.awd",  0x00000, 0x20000, CRC(b327eb83) SHA1(9e1ff53e07ca035d8d43951bac345fec7131678d), ROM_BIOS(11))
	// 12: BIOS-String: 11/27/96-VT496G-2A4L6F0IC-00 0000C-00 - runs into Award BootBlock BIOS
	ROM_SYSTEM_BIOS(12, "ficpos", "FIC 486-POS")
	ROMX_LOAD( "116di6b7.bin", 0x00000, 0x20000, CRC(d1d84616) SHA1(2f2b27ce100cf784260d8e155b48db8cfbc63285), ROM_BIOS(12))
	// 13: BIOS-String: 06/27/95-VT82C505-2A4L4000-00 / Version 5.15 / Chipset: VIA VT82C496G PC/AT
	ROM_SYSTEM_BIOS(13, "ficpvt", "FIC 486-PVT 5.15") // ISA16: 6, ISA/VL: 2
	ROMX_LOAD( "5150eef3.awd", 0x00000, 0x20000, CRC(eb35785d) SHA1(1e601bc8da73f22f11effe9cdf5a84d52576142b), ROM_BIOS(13))
	// 14: BIOS-String: 10/05/95-VT82C505-2A4L4000-00 / Version 5.162W2(PCTIO)
	ROM_SYSTEM_BIOS(14, "ficpvtio", "FIC 486-PVT-IO 5.162W2")  // Chipset: VT82C406MV, VT82C496G, W83777/83787F, W83758P
	ROMX_LOAD( "5162cf37.awd", 0x00000, 0x20000, CRC(378d813d) SHA1(aa674eff5b972b31924941534c3c988f6f78dc93), ROM_BIOS(14))
	// 15: BIOS-String: 40-00AG-001247-00101111-060692-SIS3486-0 / AV4 ISA/VL-BUS SYSTEM BIOS / Chipset: SIS 85C460ATQ
	ROM_SYSTEM_BIOS(15, "ava4529j", "AVA4529J") // this is a board with two VLB slots
	ROMX_LOAD("amibios_486dx_isa_bios_aa4025963.bin", 0x10000, 0x10000, CRC(65558d9e) SHA1(2e2840665d069112a2c7169afec687ad03449295), ROM_BIOS(15))
	// 16: BIOS-String: 40-0200-001291-00101111-111192-OPT495SX-0 / 34C-OP-WBp-25/33/40/50-D5-ZZ
	// Chipset: OPTi 82C495SX - CPU: 486DX - BIOS: AMI 486DX ISA BIOS AA7524842 - ISA8: 1, ISA16: 4, ISA16/VL: 2
	ROM_SYSTEM_BIOS(16, "pat48pv", "PAT-48PV")
	ROMX_LOAD("pat48pv.bin", 0x10000, 0x10000, CRC(69e457c4) SHA1(7015b2bccb10ce6e1ad6e992eac785f9d59a7a24), ROM_BIOS(16))
	// 17: Morse P1 V3.10 - CPU: 486DX - ISA8: 2, ISA16: 6 - Chipset: Morse 91A401A- Award Modular BIOS v4.20 / V3.00 - KEYBOARD ERROR OR NO KEYBOARD PRESENT
	ROM_SYSTEM_BIOS(17, "p1", "P1")
	ROMX_LOAD("morse_p1.bin", 0x10000, 0x10000, CRC(23d99406) SHA1(b58bbf1f66af7ed56b5233cbe2eb5ab623cf9420), ROM_BIOS(17))
	// 18: Chipset: SiS 85C206 CONTAQ 82C592 82C591 - CPU/FPU: 486, socket provided - OSC: 33.333MHz, 14.31818 - BIOS: AMI 486DX ISA BIOS AA0083611 (28pin)
	// BIOS-String: 40-0700-D01508-00101111-070791-CTQ 486-0 - ISA8: 2, ISA16: 6
	ROM_SYSTEM_BIOS(18, "82c591", "82C591")
	ROMX_LOAD("486-contaq.bin", 0x10000, 0x10000, CRC(e5d2cf16) SHA1(1357a964ef78eaad6894dcc9dce62be50cdf6df5), ROM_BIOS(18))
	// 19: Chipset: PCCHIPS CHIP 16 (9430-AS), CHIP 18 (9432-AS) - CPU: i486DX2-66 - BIOS: AWARD (28pin) - ISA16: 4, ISA16/VL: 3 - OSC: 14.31818MHz
	// BIOS-String: 07/13/94--2C4X6H01-00 / Release 07/15/94'
	ROM_SYSTEM_BIOS(19, "chips", "Chips")
	ROMX_LOAD("486-pcchips.bin", 0x10000, 0x10000, CRC(4e49eca1) SHA1(2343ca9f4760037eb2ef6e7b011b9690e542d6ea), ROM_BIOS(19))
	// 20: CAM/33(50)-P8 M458(A)P80 - Chipset: Opti 82C495SX, F82C206Q 82C392SX - CPU: 486DX-33 (solder pads for 486sx and 486DX) - OSC: 14.318MHz, 33.000MHz
	// Keyboard-BIOS: AMI Keyboard BIOS PLUS A317473 - BIOS: AMI 486 BIOS PLUS 214097 (28pin) - RAM: SIMM30x8 - Cache: 1xIS61C256A, 8xUM61256BK-25 - ISA8: 1, ISA16: 6
	// BIOS-String: X0-0101-001105-00101111-060692-495SX_A-0 / 486DX/SX CAM/33,50-P8, CPM/25,33-P8, 12/14/1992
	ROM_SYSTEM_BIOS(20, "cam33", "CAM/33")
	ROMX_LOAD("486-cam.bin", 0x10000, 0x10000, CRC(d36a13ea) SHA1(14db51dbcf8decf1cb333c57a36971ef578c89b4), ROM_BIOS(20))
	// 21: 486-PIO3 1.1 - Chipset: Winbond W83787F, VIA VT82C505, VT82C416, VT82C496G - ISA16: 4, PCI:3 - BIOS: AWARD F 4825803 1.14G705 (32pin) - CPU: Socket 3
	// RAM: 2xSIMM72, Cache: 9 sockets marked SRAM 128Kx8 (2 banks +1) - On board: 2xIDE, Floppy, par, 2xser
	// BIOS-String: 02/01/96-VT496G-2A4L6F0IC-00 0000C-00 . runs into BootBlock BIOS
	ROM_SYSTEM_BIOS(21, "pio3", "486-PIO-3")
	ROMX_LOAD("486-pio3.bin", 0x00000, 0x20000, CRC(1edb5600) SHA1(36887cd08881dfa063b37c7c11a6b65c443bd741), ROM_BIOS(21))
	// 22: 486 G486IP IMS - Chipset: IMS 8848 IMS 8849 - CPU: i486DX2-66 - BIOS: AMI 486DX ISA BIOS AB5870352 - Keyboard-BIOS: MEGAKEY (AMI/Intel) - ISA8: 1, ISA16: 4, PCI: 3
	// RAM: SIMM30: 4, SIMM72: 2, Cache: 10 sockets (UM61256AK-15) - BIOS-String: 41-0000-ZZ1124-00101111-060692-IMS8849-0 / PCI BIOS, Dated JUN-16-94 / FOR G486IP
	ROM_SYSTEM_BIOS(22, "g486ip", "G486IP")
	ROMX_LOAD("g486ip_ims.bin", 0x00000, 0x20000, CRC(4431794a) SHA1(f70e8c326455229c3bb7f305c2f51c4ac11979ed), ROM_BIOS(22))
	// 23: EFA 486 UPIO
	// BIOS-String: 11/09/95-UMC-881/886A-2A4X5E39C-00 00 / N486U-PIO/A, Rev 1.03 ROM - NOT FOR SALE - boots into BootBlock BIOS
	ROM_SYSTEM_BIOS(23, "486upio", "486 UPIO")
	ROMX_LOAD("upio_103.bin", 0x00000, 0x20000, CRC(4e9139cd) SHA1(f2b00356957c712ca652c3751b31161b3110ec69), ROM_BIOS(23))
	// 24: Acer 486 Version 2.2 - Chipset: ALi M1429, M1431 - CPU: 486 - RAM: SIMM30x8
	// screen remains blank
	ROM_SYSTEM_BIOS(24, "acer48622", "Acer 486 V2.2.")
	ROMX_LOAD("4alm002.bin", 0x10000, 0x10000, CRC(88291af2) SHA1(7ff912e9f0550631377d1a4c3aa266a081e7dce9), ROM_BIOS(24))
	// 25: ACR6BE00-M00-940720-R01-E0 / BIOS V2.0 - Keyboard Interface Error, Pointing DeviceInterface Error
	ROM_SYSTEM_BIOS(25, "4alo001", "4ALO001")
	ROMX_LOAD("4alo001.bin", 0x00000, 0x20000, CRC(4afb9c50) SHA1(5e56682ba1e04bd0b074de3b2a93fb5322325d01), ROM_BIOS(25))
	// 26: dies after initialising the graphics card
	ROM_SYSTEM_BIOS(26, "4alp001", "4ALP001")
	ROMX_LOAD("4alp001.bin", 0x10000, 0x10000, CRC(9b4a2881) SHA1(f324bb0304164e9ede1dd2eebb085a76aae398be), ROM_BIOS(26))
	// 27: BIOS-String: 30-0500-ZZ1130-00101111-070791-1219-0
	ROM_SYSTEM_BIOS(27, "zz1130", "ZZ1130")
	ROMX_LOAD("4zzw001.bin", 0x10000, 0x10000, CRC(dc21c952) SHA1(affdc4efbca4dad561e4f0141463844ec84ae519), ROM_BIOS(27))
	// 28: screen remains blank
	ROM_SYSTEM_BIOS(28, "optimus", "Optimus")
	ROMX_LOAD("mb_bios_ami_930808.bin", 0x10000, 0x10000, CRC(89151d5b) SHA1(92a93cae054525adfdc6277a1236e699ea9fbc32), ROM_BIOS(28))
	// 29: 40-0100-DG1112-00101111-070791-UMC480A / Rev. 251191 UMC-486A - ASI board
	// The BIOS comes from http://www.elhvb.com/supportbios.info/Archives/BIOS/0-A/ASI/UMC-486A/index.html and contains 28 extra plain textattr
	// bytes at the end of the file. These have been lopped off, but the emulated machine complains about a ROM error, thus marked BAD_DUMP
	ROM_SYSTEM_BIOS(29, "umc486a", "UMC-486A")
	ROMX_LOAD("umc481icorr.ami", 0x10000, 0x10000, BAD_DUMP CRC(d27b2fd4) SHA1(e639dbc7d65b29ffca26701af766fa75bfe33787), ROM_BIOS(29))
	// 30: 40-0201-D41107-00101111-031591-OPBC-0
	ROM_SYSTEM_BIOS(30, "a9c11f1f", "a9c11f1f")
	ROMX_LOAD("ami_486_zz686886.bin", 0x10000, 0x10000, CRC(a9c11f1f) SHA1(2a27ecae9547ddd3d230c30a94deb83a4d6b4436), ROM_BIOS(30))
	//31: Award Modular BIOS v4.20 / Version 1.09K
	ROM_SYSTEM_BIOS(31, "109k", "1.09K")
	ROMX_LOAD("award_486dx_0097042.bin", 0x10000, 0x10000, CRC(b620534b) SHA1(d3777a82cb35639d386a1840dd5cf52527ec6f8b), ROM_BIOS(31))
	// 32: UNIC2 94V-0 9326 - OPTI chipset (3 chips)
	// BIOS-String: X0-0100-001378-00101111-060692-495SX_A
	ROM_SYSTEM_BIOS(32, "unic2", "UNIC 2 94V-0")
	ROMX_LOAD("amibios-486dx-1992-aa8707058-m27c512.bin", 0x10000, 0x10000, CRC(a2b3e326) SHA1(b29c5668fb3337893ef3a96f053f90b929bac0d6), ROM_BIOS(32))
	// 33: BIOS-String: 40-0100-001276-00101111-060692-495_X86-0
	// OPTi 82C495 SX, F82C206 - CPU: two solder pads and two CPU sockets - RAM: 8xSIMM30, Cach: 10x28pin DIP - ISA8: 2, ISA16: 3, ISA16/VLB: 3
	ROM_SYSTEM_BIOS(33, "495sx1", "495sx-1")
	ROMX_LOAD("495sx-1.bin", 0x10000, 0x10000, CRC(318a99a3) SHA1(e57380ed2a802cd1648a32317313ade5221f1213), ROM_BIOS(33))
	// 34: Chipset: marked QDI - CPU: Socket 3, 25MHz-50MHz - RAM: 4xSIMM30, 2xSIMM72, Cache: 8x32pin DIP (4x28pin DIP filled), 1x32pin DIP (filled with 28pin DIP)
	// BIOS: AMI 486DX ISA BIOS AC2849676 - OSC: 18.31818 - ISA8: 1, ISA16: 3, ISA16/VL: 3
	ROM_SYSTEM_BIOS(34, "qdi486", "QDI486") // screen remains blank
	ROMX_LOAD("qdi486.bin", 0x10000, 0x10000, CRC(f44a5a45) SHA1(57cfd7c6524eba21395642bd57b726b45eef4b6a), ROM_BIOS(34))
ROM_END


/***************************************************************************
  80486 motherboard
***************************************************************************/

// Chicony CH-486-33C REV 1.0 - Chipset: UMC UM82C481, UM82C482, Chips P82C206 - CPU: Intel Overdrive DX2ODPR66, FPU socket provided
// RAM: 16xSIMM30, Cache: 8xW24256AK-25, P4C188-20PC - BIOS: AMI - Keyboard-BIOS: Intel/AMI
// BIOS-String: 40-0300-ZZ1116-00101111-031591-UMCWB-F - OSC: 33.000MHz - ISA8: 2, ISA16: 5
ROM_START( ch48633c )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "ch-486-33c_ami_bios_z944944_am27.bin", 0x10000, 0x10000, CRC(1ca9bf11) SHA1(bb5fb4c7544e2ccb06d423ef4da0729a6bf8b231))
ROM_END

// Alaris Tornado 2 - CPU: 486 - Chipset: Opti/EFAR/SMC - ISA16: 4, PCI: 3, ISA16/VL: 2 - On board: Floppy, 1xIDE, parallel, 2xserial
ROM_START( alator2 ) // unknown beep code LH-HL
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "tornado2.bin", 0x00000, 0x20000, CRC(2478136d) SHA1(4078960032ca983e183b1c39ae98f7cdc34735d0))
ROM_END

// Addtech Research 4GLX3 Green-B 4GPV3.1 aka VisionEX 4GPV3 - Chipset: Contaq 82C596A - BIOS Version: AMI 01/10/94 486 ISA BIOS AA 6729627
// Keyboard BIOS: AMI - CPU: Socket 3, solder pads for 80486DX/SX provided - RAM: 8xSIMM30, Cache: 9x28pin DIP - ISA16: 8, ISA16/VLB: 2
ROM_START( ar4glx3 ) // BIOS-String: 40-0101-006666-00101111-011094-CTQ-596A / KIM Computer by CTL Corporation / Model : [4GLX3] --- Made in U.S.A.
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "4glx3_bios.bin", 0x10000, 0x10000, CRC(3367e0b2) SHA1(8dbe58ed783c56ca2cb61ded6f603314739dcfb8))
ROM_END

// TMC PAT48AV 1.4 - ALi M1429 A1, M1431 A2 - Bios: AMI 486DX ISA BIOS AB4179743 - Keyboard BIOS: AMI-KB-H-WP - CPU: 486DX socket
// RAM: 8xSIMM30 - Cache: 8xIS61C256AH-20N - OSC: 14.31818 - ISA16: 4, ISA16/VLB: 3
ROM_START( tmpat48av ) // screen remains blank - BIOS-String: 40-0106-001291-00101111-080893-ALI1429 / 486DX-AC-WBu-25/33/40/50-L6-ZZ
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "pat48av_bios_27c512.bin", 0x10000, 0x10000, CRC(a054b7b3) SHA1(edc2554a73aba94d586f8b49a5c5bbbe2890331c))
ROM_END

// BIOSTAR - MB-1433/50 AEA-P - V:1 - Chipset: VLSI 82C3480 & 83C3490 (marked BIOTEQ) - BIOS/Version: AMIBIOS 12/12/91 - Keyboard BIOS: MEGA-KB-F-WP
// CPU: It's ST 486Dx2-80 - RAM: 8xSIMM30, Cache: 8xW24M257AK-15, 1xW2465AK-15 - ISA8: 1, ISA16: 6 - OSC: 66.000MHz, 14.318
ROM_START( mb1433aeap ) // 40-0100-001223-00101111-121291-B82C3480-0 / M1433/50AEA-P
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "mb-1433-aea-p.bin", 0x10000, 0x10000, CRC(daac20f7) SHA1(b6889a4896f83b306e69efa87cae8d03147b6dbd))
ROM_END

// FIC ELI6-II (from Unisys ELI 46665 Desktop) - Chipset: VIA 82C486A, VT82C487, SMC FDC37C665QF P - BIOS label: ELI6-U555
// BIOS Version: Award U573 3/14/95 - CPU: Intel Overdrive DX40DPR100 in Socket 3 - RAM: 4xSIMM72, Cache: 9x256K-15 - ISA16:6 - On board: ISA, Floppy
ROM_START( ficeli6ii ) // BIOS String: 03/14/95-VT82C486A-214L2000-00 / Award Modular BIOS v4.50 /3.20G /F5 1.00 / UNISYS ELI6-II Version U573.BIN (03/14/95)
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "fic_eli6ii.bio", 0x10000, 0x10000, CRC(b61cc026) SHA1(59eae42bf1dea01ba04fecd9bb367e47d4a256d4))
ROM_END

// Mitac PWA-IH4077D - Chipset: EFAR EC802GL, EC100G, UMC UM82C863F, UM82C865F - CPU: Socket 3 - RAM: 4xSIMM72, Cache: 9xIS61C256AH-20N
// BIOS: known: R2.03/CKS: 2400H, ATT4077D BIOS R1.01.00/CKS: 3000H - Keyboard BIOS: Award KB-200 or VIA VT82C42N (on ATT4077D)
// On board: 2xser, par, Floppy, IDE - ISA16: 4, ISA17/VL: 2
ROM_START( pwaih4077d ) // BIOS-String: 04/02/98-EFAR-EC802G-B-2C403D31-00 / (IH4077D R2.08G)
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "ih4077d.208g", 0x10000, 0x10000, CRC(0f834ba2) SHA1(7e08e454dfa3cf5079845fe61b9ae74b1dcc7981))
ROM_END

// ASUS ISA-486SIO rev. 1.2 - Chipset : SiS 85C460 ATQ, Winbond W85C16B - BIOS : AMI 486DX/ISA BIOS AA2310181 - Keyboard BIOS: AMI
// CPU: Intel 80486SX-25 - RAM: 8xSIMM30, Cache: 9x28pin DIP (used: 4xM5M5276P-25, 1xUM6164BK-20) - ISA8: 1, ISA16: 6 - OSC: 25.000MHz, 14.31818
ROM_START( a486sio )// BIOS-String : 40-0104-001292-00101111-050591-I486SI-F
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "isa-486sio.bin", 0x10000, 0x10000, CRC(f339f8ff) SHA1(d53f0ff30cc7f0c70ffeeda33d16dddbeedd6098))
ROM_END

// Micronics JX30GP - Motherboard P/N: 09-00189-10 REV B1 - Chipset: MIC 471, MIC491, PC87312VF (Super I/O), KS82C6818A -
// CPU: Socket 3, solder pads for 80486QFP - RAM: 4xSIMM72, Cache: 6xUM61256FK-15, 1xW24257AK-15 - DIP4: 0000 - OSC: 14.318
// ISA16: 5, ISA16/VLB: 2 - on board: Floppy, ISA, PS/2 keyboard and mouse
ROM_START( mijx30gp ) // BIOS: Phoenix, 80486 ROM BIOS PLUS Version 0.10 GJX30G-04P, Gateway 2000 Local Bus BIOS - shows Error 8602 - Auxiliary Device Failure
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "micronics_gjx30g-04p_09-24-93.bin", 0x10000, 0x10000, CRC(66477a66) SHA1(549eecf707bbb43bcdc89715b36cc23e3cb1a074))
	ROM_IGNORE(0x10000) // the second half of the 128K ROM seems to contain BIOS source code
ROM_END

// AMI 80486 EISA Enterprise-II - Chipset: AMI/Intel 82357 - RAM: 8xSIMM30 (32MB max), 128K cache memory - CPU: i486DX 25/33, Weitek WTL4167 FPU socket provided
// on board: Floppy, PS/2 mouse, memory card socket, i/o expansion module slot - EISA: 8
// BIOS-String: 41-0001-004616-00111111-070791-AMI-EP-0 - EISA CMOS INOPERATIONAL / Software Port NMI INOPERATIONAL / Fail-safe Timer NMI INOPERATIONAL / Keyboard error
ROM_START( amient2 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "16092391.cs1", 0x00000, 0x20000, CRC(b0ae5c8d) SHA1(0d6cda74433a5d70e8fc2bec6a77ed97a09a984e))
ROM_END

// AMI Enterprise-III EISA Local Bus Series 68 - CPU: i486SX/DX/DX2 25/33/50/66MHz - RAM: 16xSIMM32 (up to 256MB RAM!!!), Cache 256K
// EISA: 6, EISA/VL: 2 - BIOS: Flash - repeated short single beeps (DRAM refresh error)
// BIOS-String: 41-0103-004668-00111111-111192-AMIS68
ROM_START( amient3 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "s68p.rom", 0x00000, 0x20000, CRC(7342b9ef) SHA1(da6b2312ccc175473443b1d562d5e4f3952cac5a))
ROM_END

// AMI ENTERPRISE-IV EISA VLB - CPU: 486SX/486SX/486DX/DX2/Pentium overdrive 25/33/50/66 MHz - RAM: 4xSIMM72 (256MB), Cache: 256KB
// EISA: 5, EISA/VL: 2 - on board: floppy, IDE, par, 2xser - screen remains blank
// BIOS-String: 41-0103-004687-00111111-111192-AMIS87
ROM_START( amient4 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "s87p.rom", 0x00000, 0x20000, CRC(ff092e7f) SHA1(4999278dca001de74dff518f1f1c9ea8212d7ed4))
ROM_END

// AMI Super Voyager PCI-II - Chipset: AMI, SMC - CPU: 486DX/DX2/DX4 - RAM: 4xSIMM72 (64MB), Cache: 5x32pin, 4x28pin+TAG (127/256K)
// BIOS: Flash - Keyboard-BIOS: AMIKEY - ISA16: 4, PCI: 3 - on board: Floppy, 2xIDE, 2xer, par - screen remains blank
ROM_START( amisvpci2 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "s724p.rom", 0x00000, 0x20000, CRC(fa9ea9b3) SHA1(b0f2206a7f0d6a00e094f7d151c16022a5292858))
ROM_END

// AMI Super Voyager VLB - screen remains blank
// BIOS-String: 40-0103-004669-00111111-111192-AMIS69B2
ROM_START( amisvvlb )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "s69b2p.rom", 0x00000, 0x20000, CRC(3a13944c) SHA1(9d7733ee50023edd5b0bf4b098c9c6c35a4dc2b0))
ROM_END

// AMI Super Voyager VLB-II - AMI Super Voyager VLB-II - CPU: 486/Overdrive - Chipset: AMI, SMC FDC37C665GT
// RAM: 4xSIMM72, Cache: 8x28pin + TAG - ISA16: 5, ISA16/VL: 2 - BIOS: 28pin in a 32pin socket - on board: Floppy, IDE
// BIOS-String: 40-0100-004682-00111111-111192-AMIS82E
ROM_START( amisvvlb2 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "s82p.rom", 0x00000, 0x20000, CRC(2e2d33ae) SHA1(349be4b29bc8a4447ab4c73fe4c695276565489f))
ROM_END

// AMI Super Voyager VLB-III - Chipset: FDC37C665GT, AMI - CPU: 486SX, 487DX, 486DX/DX2/DX4, Overdrive
// RAM: 4xSIMM72 - on board: mouse, par, 2xser, Floppy, IDE - ISA16: 5, ISA16/VL: 2 - screen remains blank, repeated single beeps
ROM_START( amisvvlb3 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "s707p.rom", 0x00000, 0x20000, CRC(5c0e55c0) SHA1(0c5232a21180d4541202ac6dd34677339b7cbecc))
ROM_END

// Edom 486VL3H MV020 - CPU: 486 - Chipset: HiNT CS8005, HMC HM82C206AQ - RAM: 8xSIMM30
// Cache: IS61C256AH-20Z (8), AE88128AK-15 - Keyboard-BIOS: JETkey V5.0 - ISA8: 2, ISA16: 3, ISA16/VLB: 3
// BIOS-String: 09/05/94-HINT-8005-214D1W00-00 / HiNT CS8005 FOR 486SX/DX/DX2 VL-BUS [MV020]
ROM_START( ed486vl3h )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "4hiw001.bin", 0x10000, 0x10000, CRC(760842fd) SHA1(2fe156f092c84cf385079da1209a8f1e06005f5e))
ROM_END

// MSI MS-4134 - Chipset: ALI M1429, M1431, M1435 - CPU: Socket 3, RAM: 4xSIMM30, 2xSIMM72, Cache: 8x32pin + TAG, used: 9xW2464AK-20
// ISA16: 3, ISA16/VL: 2, PCI: 3 - BIOS: AMI
// BIOS-String:
ROM_START( ms4134 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0:  BIOS-String: 40-0105-421169-00101111-080893-1429GPCI-0 / AL57 111594
	ROM_SYSTEM_BIOS(0, "al57", "AL57")
	ROMX_LOAD( "al57.rom", 0x00000, 0x20000, CRC(6b6c2a11) SHA1(1e40ef8a7a7b3be057ba6a121abfd0d983d5d5c9), ROM_BIOS(0))
	// 1: BIOS-String: 40-0105-421169-00101111-0890893-1429GPCI-0 / AL51 5/23/1994
	ROM_SYSTEM_BIOS(1, "al51", "AL51")
	ROMX_LOAD( "4alm001.bin", 0x00000, 0x20000, CRC(0ea9f232) SHA1(5af1a0cf047b68a7070b8c45081a80e817aade84), ROM_BIOS(1))
ROM_END

// Abit 486 EISA-AE4 - Chipset: SiS 85C406, 85C411, three other SiS chips unreadable - CPU: 486, FPU socket provided - RAM: 8xSIMM30, Cache: 8x28pin
// ISA16: 2, EISA: 6 - BIOS: Award EISA486/CU - Keyboard-BIOS: NEC KB-BIOS VER:400 JU-JET 1989 -  OSC: 50.000MHz, 14.31818MHz
// Award Modular BIOS v4.20 / AE4 EISA SYSTEM BIOS
ROM_START( abae4 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "ae4.bin", 0x10000, 0x10000, CRC(d9cbc3c6) SHA1(eeeaef7fd188598d477897f0248c99940cd1a5d7))
ROM_END

// FIC 486-KVD - Chipset: VIA VT82C485 - CPU: 486, solder pad for 486sx present - RAM: 8xSIMM30, Cache: 4xIS61C256A-20N+1xCY7C185-20PC, 4 empty sockets (28pin)
// ISA16: 5, ISA16/VL: 2 - BIOS: AMI 486DX ISA BIOS AA7211137 - Keyboard-BIOS: MEGA-KB-H-WP

ROM_START( fic486kvd )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: X0-0100-001121-00101111-021993-VIA-0
	ROM_SYSTEM_BIOS(0, "021993", "021993")
	ROMX_LOAD( "486kvd_aa72111137.bin", 0x10000, 0x10000, CRC(a1f1810f) SHA1(405afbf1635c6b41343aabfeeb3cf4cdc947a5ba), ROM_BIOS(0))
	// 1: BIOS-String: X0-0100-001121-00101111-021993-VIA-0
	ROM_SYSTEM_BIOS(1, "060692", "060692")
	ROMX_LOAD( "486-aa9615811.bin", 0x10000, 0x10000, CRC(b6b1a8e4) SHA1(be537fc27f6dedbd7fd935a7900ec075d2183837), ROM_BIOS(1))
ROM_END

// Eagle EAGLEN486 GC10A - Chipset: NEC ADC006, LGS Prime 3B 9543 - CPU: Socket 3 - RAM: 2xSIMM72, Cache: fake (not connected, marked write back)
// On board: IDE, Floppy, 2xser, par - ISA16: 4, PCI: 2 - BIOS: 32pin (sst29ee010), only the first half is occupied - // BIOS-String: Phoenix NuBIOS Version 4.04
ROM_START( gc10a )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "nec_sst29ee010_orig.bin", 0x10000, 0x10000, CRC(7b1feabb) SHA1(468734b766b9c438b2659fddf2cabcfde5a574a2))
	ROM_IGNORE(0x10000)
ROM_END

// Arstoria AS496 - Chipset: SiS 85C495, 95C497, Winbond - CPU: Socket 3 - RAM: SIMM72x4, Cache: 4+1 - BIOS: 32pin  Keyboard-BIOS: BESTKEY - ISA16: 4, PCI: 3
// BIOS-String: 09/12/96-SiS-496-497/A/B-2A4IBR2CC-00 / ARSTORIA AS496 V2 09/12/96 - boots to Boot Block BIOS
ROM_START( as496 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "as496.bin", 0x00000, 0x20000, CRC(745f8cc8) SHA1(46b9be25a7027a879482a412c9fe5687bbb28f08))
ROM_END

// Peacock PCK 486 DX DOC 50-60064-00 - Chipset: Symphony SL82C465 SL82C461 SL82C362 Chips F82C721 - CPU: i486DX-33, FPU socket privoded
// BIOS: AMI 486DX ISA BIOS AA3364567 - Keyboard-BIOS: AMI/Intel P8942AHP - On board: 2xser, Floppy, IDE, par - OSC: 33.000MHz
// BIOS-String: 40-0100-806294-00101111-060692-SYMP-0 / Peacock Computer 486 BIOS Rev. 2.0 / 30.11.92 - ISA16: 6
ROM_START( pck486dx )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "pck486dx.bin", 0x10000, 0x10000, CRC(d0edeba8) SHA1(b5b9492f32e35764c802be2b05a387a9b3aa7989))
ROM_END

// FIC 486-GIO-VT2 - Chipset: Winbond W83758P, Winbond W83757AF, VIA VT82C482, VT82C486A, VT82C461 - ISA8: 1, ISA16: 3, ISA/VL: 2
// On board: Game, 2xIDE, 2xser, par, Floppy
ROM_START( ficgiovt2 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 07/06/VT82C486A-214L2000-00 / Version  3.26G
	ROM_SYSTEM_BIOS(0, "ficgiovt2_326", "FIC 486-GIO-VT2 3.26G")
	ROMX_LOAD( "326g1c00.awd", 0x10000, 0x10000, CRC(2e729ab5) SHA1(b713f97fa0e0b62856dab917f417f5b21020b354), ROM_BIOS(0))
	// 1: BIOS-String: 06/19/95-VT82C486A-214L2000-00 / Version VBS1.08H 486-GIO-VT2
	ROM_SYSTEM_BIOS(1, "vt2vbs108","VBS1.08H 486-GVT-2")
	ROMX_LOAD( "award_486_gio_vt2.bin", 0x10000, 0x10000, CRC(58d7c7f9) SHA1(097f15ec2bd672cb3f1763298ca802c7ff26021f), ROM_BIOS(1)) // Vobis version, Highscreen boot logo
	// 2: BIOS-String: 07/17/97-VT82C486A-214L2000-00 / Version 3.276
	ROM_SYSTEM_BIOS(2, "ficgiovt2_3276", "FIC 486-GIO-VT2 3.276")
	ROMX_LOAD( "32760000.bin", 0x10000, 0x10000, CRC(ad179128) SHA1(595f67ba4a1c8eb5e118d75bf657fff3803dcf4f), ROM_BIOS(2))
	// 3: BIOS-String: 08/30/94-VT82C486A-214L2000-00 / Version VBS1.04 486-GIO-VT2 - Keyboard-BIOS: VT82C42N
	ROM_SYSTEM_BIOS(3, "vt2vbs104","VBS1.04 486-GVT-2")
	ROMX_LOAD( "486-gio-vt2.bin", 0x10000, 0x10000, CRC(7282133d) SHA1(c78606027eca509cd6d439e4689b8d50753ee80c), ROM_BIOS(3)) // Vobis version, Highscreen boot logo
ROM_END

// FIC 486-GVT - VIA VT82C486, VIA VT82C482 - AMIBIOS 08/08/93 - CPU: P24T, solder pads for 486 provided - RAM: SIMM30: 4, SIMM72: 2, Cache: 9x28pin DIP
// ISA16: 4, ISA16/VLB: 2
ROM_START( fic486gvt ) // BIOS-String: X0-0100-001121-00101111-112593-VT486N8-0
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486gvt.bin", 0x10000, 0x10000, CRC(4c5b4bde) SHA1(04711725fe89d9c793a369d82d411a5495ae3aea))
ROM_END

// Octek Hawk REV 1.1 - BIOS: AMI AA1481746 486DX ISA BIOS 28pin - Keyboard-BIOS: Intel/AMI - Chipset: OPTi F82C206L, 82C496 - OSC: 66.667MHz, 14.31818MHz
// BIOS-String: 40-0100-000000-00101111-121291-OPTIDXBB-0 / HAWK -011 - CPU: Intel Overdrive DX2ODPR66 - ISA16: 7
ROM_START( ochawk )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "hawk.bio", 0x10000, 0x10000, CRC(365b925d) SHA1(3a1776c80540b6878ff79857c2d4e19320a2792a))
ROM_END

// Abit AB-PW4 - Chipset: Winbond W83C491F, W83C492F (SL82C491 Symphony Wagner) - BIOS/Version: Award D2144079 - CPU: i486sx-25 - ISA8: 1, ISA16: 3, ISA16/VL: 3
ROM_START( abpw4 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 03/21/95-Winbond-83C491-2C4J6A11-46 / Award v4.50G / GREEN CACHE 486 VESA SYSTEM BIOS
	ROM_SYSTEM_BIOS(0, "2c4j6a11", "2C4J6A11-46")
	ROMX_LOAD( "award_486_bios_d2144079_c1984-1995.bin",0x10000, 0x10000, CRC(c69184da) SHA1(e8a799f9a3eebfd09c1d19a909574fca17fce7a0), ROM_BIOS(0))
	// 1: BIOS-String: 09/12/95-Winbond-83C491-2C4J6A12-2E
	ROM_SYSTEM_BIOS(1, "2c4j6a12", "2C4J6A12-2E")
	ROMX_LOAD( "pw4_2e.bin", 0x10000, 0x10000, CRC(c4aeac4d) SHA1(e58f2e2d5c337f447808535629686dde54c09fab), ROM_BIOS(1))
ROM_END

// Vintage Sprite SM 486-50USC - Chipset: UM82C491F - BIOS: EPROM/MR-BIOS 1.50 - Keyboard-BIOS: JETkey V3.0
// CPU: Intel 486DX2-66 - OSC: 33.333000MHz, 14.31818MHz - ISA16: 5, ISA16/VL: 2
ROM_START( sm48650usc ) // constant reset
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "mrbios_1150usc_um82c491f.bin", 0x10000, 0x10000, CRC(b6ef1220) SHA1(94511df49713ec30467c8d9b18eb04e83fa7a809))
ROM_END

// Octek Hippo VL+ - CPU: 486 - BIOS: EPROM/MR - Keyboard-BIOS: MR/Amikey - Chipset: DCA/Octek (label stickers) - ISA16: 3, ISA16/VL: 3
// MR BIOS (r) V1.52 / 486SLC CPU 28MHz
ROM_START( ochipvlp )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0:  // reset loop
	ROM_SYSTEM_BIOS( 0, "v152", "V1.52")
	ROMX_LOAD( "vlmr152.rom", 0x10000, 0x10000, CRC(b4febf98) SHA1(a28ffa20fe772eac5fd149821d5637af63965371), ROM_BIOS(0))
	// 1: MR BIOS (r) V3.21 2GB support
	ROM_SYSTEM_BIOS( 1, "v321", "V3.21 with 2GB support")
	ROMX_LOAD( "v053b407.rom", 0x10000, 0x10000, CRC(415d92b1) SHA1(e5a9f2a677002368d20f1281e2ac3469b19079f9), ROM_BIOS(1))
ROM_END

// Octek Hippo COM - Chipset: UMC UM82C865F, UM82C863F, UM82C491F - CPU: 486sx - BIOS: EPROM/AMI 486DX ISA BIOS - Keyboard-BIOS: MEGATRENDS MEGA-KB-H-WP / Intel
// BIOS-String: 40-0102-428003-00101111-080893-UMC491F-0 / U491/3 GREEN 486 MAIN BOARD INV1.1 94.2.21 - ISA16: 4 - On board: 1xIDE, Floppy, Game, 2xserial, 1xparallel
ROM_START( ochipcom )
	ROM_REGION32_LE( 0x20000, "bios", 0)
	ROM_LOAD( "hippo_com_bios.bin", 0x10000, 0x10000, CRC(d35f65a1) SHA1(885f55f87d2070c6a846768e5cf76499dad8d15c))
ROM_END

// J-Bond A433C-C/A450C-C RAM: 8xSIMM30, Cache: 8xCY7199-25PC/2xCY7C166-20PC - 2 8-bit ISA, 6 16-bit ISA)
// Chipset: ETEQ ET82C491 + ET82C493; CHIPS P82C206; AMI KB-BIOS-VER-F P8042AHP - OSC: 33.000MHz - CPU: i486DX-33
// BIOS: AMI 486 BIOS ZZ566787 - BIOS-String: 40-0200-001353-0010111-070791-ETEQ4/1C-0 / ETEQ 486 Mar. 05, 1992
ROM_START( a433cc )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "ami_j-bond_a433c-c.bin", 0x10000, 0x10000, CRC(66031e98) SHA1(d2d1a26837d3ca943a6ef09ec3e6fbfaaa62cc46))
ROM_END

// ASUS PVI-486AP4 (Socket 3, 4xSIMM72, Cache: 128/256/512KB, 4 PCI, 4 ISA, 1 VLB)
// Intel Aries PCIset S82425EX + S82426EX; DS12887 RTC; VIA VT82C42N - BIOS: 32pin
ROM_START( a486ap4 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 07/20/94-ARIES-P/I-AP4G-00 / #401A0-0104
	ROM_SYSTEM_BIOS(0, "486ap4v104", "ASUS PVI-486AP4 V1.04")
	ROMX_LOAD( "awai0104.bin", 0x00000, 0x20000, CRC(52ea7123) SHA1(3d242ea6d1bcdddd41e32e40708133c72f2bd060), ROM_BIOS(0))
	// 1: BIOS-String: 10/21/94-ARIES-P/I-AP4G-00 / #401A0-0203
	ROM_SYSTEM_BIOS(1, "486ap4v203", "ASUS PVI-486AP4 V2.03")
	ROMX_LOAD( "awai0203.bin", 0x00000, 0x20000, CRC(68d3a3f4) SHA1(6eee0c9aed2ede028eb170f8dd7921563293b99f), ROM_BIOS(1))
	// 2: BIOS-String: 11/08/94-ARIES-P/I-AP4G-00 / #401A0-0204
	ROM_SYSTEM_BIOS(2, "486ap4v204", "ASUS PVI-486AP4 V2.04")
	ROMX_LOAD( "awai0204.bin", 0x00000, 0x20000, CRC(b62b35bb) SHA1(b6fa3d7b1c88da37ce74aca329a31d2587652d97), ROM_BIOS(2))
	// 3: BIOS-String: 11/25/97/ARIES-P/I-AP4G-00 / #401A0-0205-2
	ROM_SYSTEM_BIOS(3, "486ap4v205-2", "ASUS PVI-486AP4 V2.05-2")
	ROMX_LOAD( "0205.002", 0x00000, 0x20000, CRC(632e8ee6) SHA1(3cf57b2654b0365e41ef5f5c82f68eeadf0e7a21), ROM_BIOS(3))
ROM_END

// ASUS PCI/I-486SP3G V3.02 (Socket 3, RAM: 4xSIMM72, Cache: 128/256/512K, 1 IDE, 1 SCSI, 3 PCI, 4 ISA) - BIOS: 32pin
// Intel Saturn II chipset: 82424ZX CDC + 82423TX DPU + 82378ZB SIO; NCR 53C820; National PC87332; DS12887 RTC; VIA VT82C42N
ROM_START( a486sp3g )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 10/21/94-SATURN-II-P/I-SP3G-00 / #401A0-302
	ROM_SYSTEM_BIOS(0, "v302", "ASUS PCI/I-486SP3G V3.02")
	ROMX_LOAD( "awsg0302.bin", 0x00000, 0x20000, CRC(21e918a0) SHA1(c7f937e3e90a43d7c7f867e686625b28a9c2484c), ROM_BIOS(0))
	// 1: BIOS-String: 08/15/95-SATURN-II-P/I-SP3G-00 / #401A0-304
	ROM_SYSTEM_BIOS(1, "v304", "ASUS PCI/I-486SP3G V3.04")
	ROMX_LOAD( "awsg0304.bin", 0x00000, 0x20000, CRC(f4d830d2) SHA1(086ccd14c7b0c521be1958d58b3539c4bfe4721f), ROM_BIOS(1))
	// 2: BIOS-String: 04/21/99-SATURN-II-P/I-SP3G-00 / #401A0-0306-1
	ROM_SYSTEM_BIOS(2, "v306", "ASUS PCI/I-486SP3G V3.06")
	ROMX_LOAD( "0306.001.bin", 0x00000, 0x20000, CRC(278e1025) SHA1(75835e59cf28bb6b9258f676766633cbffa56848), ROM_BIOS(2))
ROM_END

// ASUS VL/EISA-486SV1 (8 EISA, 1 VLB) -
ROM_START( a486sv1 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// BIOS-String: 05/20/94-SIS-486/EISA-E-486SV1-00 / #401A0-0112
	//ROM_SYSTEM_BIOS(0, "v112", "Award BIOS V1.12")
	ROM_LOAD( "e4sv0112.awd", 0x10000, 0x10000, CRC(d1d42fc9) SHA1(61549bf597517bb3c33e724e32b3cca981e65000))
ROM_END

// FIC 486-VIP-IO (3 ISA, 4 PCI)
// VIA GMC chipset: VT82C505 + VT82C486A + VT82C482 + VT82C483 + VT83C461 IDE; DS12885Q RTC; National PC87332VLJ-S I/O
ROM_START( ficvipio )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 12/07/94-VT82C505-2A4L4000-00 / Version 4.26GN2(ES2) (12/07/94)
	ROM_SYSTEM_BIOS(0, "426gn2", "FIC 486-VIP-IO 4.26GN2")
	ROMX_LOAD( "426gn2.awd",   0x00000, 0x20000, CRC(5f472aa9) SHA1(9160abefae32b450e973651c052657b4becc72ba), ROM_BIOS(0))
	// 1: BIOS-String: 02/08/96-VT82C505-2A4L4000-00 / Version 4.27GN2A (02/14/96)
	ROM_SYSTEM_BIOS(1, "427gn2a", "FIC 486-VIP-IO 4.27GN2A")
	ROMX_LOAD( "427gn2a.awd",  0x00000, 0x20000, CRC(035ad56d) SHA1(0086db3eff711fc710b30e7f422fc5b4ab8d47aa), ROM_BIOS(1))
	// 2: BIOS-String: 01/18/95-VT82C505-2A4L4000-00 / Version 4.26GN2A (01/18/95)
	ROM_SYSTEM_BIOS(2, "426gn2a", "FIC 486-VIP-IO 4.26GN2A")
	ROMX_LOAD( "486-vip-io.bin", 0x00000, 0x20000, CRC(907ed412) SHA1(5d2c584a230826935f56151a7c74419baf54796b), ROM_BIOS(2))
ROM_END

// Shuttle HOT-409 (6 16-bit ISA incl. 2 VLB, 2 8-bit ISA, 8 SIMM30, Cache: 64/128/256K+Tag in 2 banks)
// OPTi 82C495SX + 82C392SX + F82C206; MEGA-KB-1-WP
ROM_START( hot409 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 40-0200-001343-00101111-111192-OPT495SX-0 / Version 2.0
	ROM_SYSTEM_BIOS(0, "hot409v20", "Shuttle HOT-409 V2.0")
	ROMX_LOAD( "ami1992.bin", 0x10000, 0x10000, CRC(a19c3fd4) SHA1(404822c98344061b60883533395a89fe4902c177), ROM_BIOS(0))
	// 1: BIOS-String: 40-0204-001343-00101111-080893-OPT495SX-0 / OPTi495SX Version 3.0
	ROM_SYSTEM_BIOS(1, "hot409lba", "Shuttle HOT-409 V3.0 with LBA")
	ROMX_LOAD( "409lba.rom", 0x10000, 0x10000, CRC(78c5e47e) SHA1(7f14a88a5548fc67dd00e73fd09745e899b93a89), ROM_BIOS(1))
	// 2: BIOS-String: 40-0200-001343-00101111-111192-OPT495SX-0 / VERSION 1.1
	ROM_SYSTEM_BIOS(2, "hot409v11", "Shuttle HOT-409 V1.1")
	ROMX_LOAD( "amibios_hot409.bin", 0x10000, 0x10000, CRC(17729ee5) SHA1(ea3f5befe16ede7e9f4be3b367624745a6935ece), ROM_BIOS(2))
ROM_END

// Siemens-Nixdorf 486 mainboards and BIOS versions
// The same mainboards were used in various case versions to get the different model lines, so an identification by the mainboard number (Dxxx) is safest
ROM_START( pcd4x )
	ROM_REGION32_LE( 0x20000, "bios", 0 )
	// D756, was used in PCD-4Lsx, contains Cirrus Logic VGA ROM
	ROM_SYSTEM_BIOS(0, "d756v320r316", "D756 BIOS V3.20 R3.16")
	ROMX_LOAD( "fts_biosupdated756noflashbiosepromv320_320316_149.bin", 0x00000, 0x20000, CRC(2ab60725) SHA1(333b64424c08ecbbaf47110c99ad0335da211489), ROM_BIOS(0) )
	// D674, was used in PCD-4M, PCD-4Msx, PCD-4RSXA/4RA - this is a CPU card that is plugged into an ISA backplane; OSC: 14.31818
	// Chipset: LSI HT342-B-07, LSI HT321-D, Intel B6842-V31 AWARD(c)SNI UPI V4.3, IS9412BL PC87311AVF US4823312 - CPU: 486sx soldered onto the mainboard, but a socket for a 486DX is present - RAM: SIMM30x8
	// the CPU card can accept 16 bit "piggyback" modules, e.g. an ET4000 graphics card or a MFM harddisk controller to save ISA slots
	// on board: IDE, Floppy, beeper, keyboard connector, parallel, 2xserial, RTC DS12887, connectors for NMI and keylock, 4 DIP switches labelled DX2, Upgrade, 25, color
	// jumper X7: skip on/off, jumper x6: drv l/h
	ROM_SYSTEM_BIOS(1, "d674v320r316", "D674 BIOS V3.20 R3.16")
	ROMX_LOAD( "fts_biosupdated674noflashbiosepromv320_320316_144.bin", 0x00000, 0x20000, CRC(1293d27c) SHA1(22f36c4a5a0912011ed54ff917244f412208ffc0), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(2, "d674v320r304", "D674 BIOS V3.20 R3.04")
	ROMX_LOAD( "d674_27c1024_3.20.bin", 0x00000, 0x20000, CRC(dfdad89e) SHA1(6cb78d8b5c8822dc84970ba912bc66a5e7cd2fb4), ROM_BIOS(2) )
	// D802, was used in PCD-4HVL
	ROM_SYSTEM_BIOS(3, "d802v320r316", "D802 BIOS V3.20 R3.34.802")
	ROMX_LOAD( "fts_biosupdated802noflashbiosepromv320_320334_152.bin", 0x00000, 0x20000, CRC(fb1cd3d2) SHA1(98043c6f0299e1c56e5f266ea5f117ae456447ff), ROM_BIOS(3) )
	// D620
	ROM_SYSTEM_BIOS(4, "d620", "D620")
	ROMX_LOAD( "w26361-d620-z4-01-5_award_v3.10_r2.02.bin", 0x00000, 0x20000, CRC(2708cc2a) SHA1(a399c938ffeba4cb28a22e54235f3f9c5e2892f6), ROM_BIOS(4) )
ROM_END


/***** 486 motherboards using the ALi M1487 M1489 chipset *****/

// Abit AB-PB4 REV.:1.2 - Chipset: ALi M1487 M1489, Winbond W83787F, W83768F - On board: Floppy, 2xser, 2xIDE, par
// ISA16: 3, PCI: 3, PISA: 1 - OSC: 14.3F5P - CPU: Socket 3 - BIOS: Award D2317569, 32pin
ROM_START( abpb4 ) // both BIOS versions end up in the Boot Block BIOS
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 10/30/95-ALI-1487/89-2A4KDA12C-5E / GREEN 486 PCI SYSTEM BIOS
	ROM_SYSTEM_BIOS( 0, "pb4", "PB4")
	ROMX_LOAD( "486-ab-pb4.bin", 0x00000, 0x20000, CRC(90884abc) SHA1(1ee11b026cb783b28cc4728ab896dbeac14eb954), ROM_BIOS(0))
	// 1: BIOS-String: 07/03/96-ALI-1487/89-2A4KDA1BC-F2 / GREEN PCI/ISA SYSTEM ROM
	ROM_SYSTEM_BIOS( 1, "pb4pf2", "PB4P-F2")
	ROMX_LOAD( "pb4p_f2.bin", 0x00000, 0x20000, CRC(9ab8d277) SHA1(10e424f5dd5c98877a5a7c9ae6205b2c442ac0e0), ROM_BIOS(1))
ROM_END

// EFA 486 APIO all BIOS versions boot into BootBlock BIOS
ROM_START( 486apio )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0:
	ROM_SYSTEM_BIOS(0, "a2", "A2")
	ROMX_LOAD( "apioa2.bin", 0x00000, 0x20000, CRC(0cf343be) SHA1(3a5757c802a30fb0d8d4fd623bee02af3b91fdd7), ROM_BIOS(0))
	// 1: 07/23/96-ALI-1487/89-2A4KDE3HC-00 / N486APIO Ver 2.00 SMC665GT
	ROM_SYSTEM_BIOS(1, "20sm", "2.0SM")
	ROMX_LOAD( "apio2smc.bin", 0x00000, 0x20000, CRC(1ced0692) SHA1(8afca17f0d793a3266b04ce8d70a359a29de3af7), ROM_BIOS(1))
	// 2: BIOS-String: 03/05/96-ALI-1487/89-2A4KDE3JC-00 / N486APIO, Rev 2.1
	ROM_SYSTEM_BIOS(2, "ag2", "AG2")
	ROMX_LOAD( "1019ag2.bin", 0x00000, 0x20000, CRC(4066124e) SHA1(7adbf528d8132122da4f950ee78931abd5d949e4), ROM_BIOS(2))
ROM_END

// MSI MS-4145 - Chipset: ALi M1487, M1489, W83787F, W83758F - CPU: Socket 3 - RAM: 3xSIMM72 - Cache: 8x28/32pin + TAG - ISA16: 4, PCI: 3
// BIOS: 32pin - on board: 2xIDE, Floppy, par, 2ser
ROM_START( ms4145 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0:
	ROM_SYSTEM_BIOS(0, "ag56", "AG56")
	ROMX_LOAD( "ag56.rom", 0x00000, 0x20000, CRC(217d7258) SHA1(bd0d484607fbcf54821822e20e4bf5fcaf456591), ROM_BIOS(0))
	// 1:
	ROM_SYSTEM_BIOS(1, "ag56p", "AG56P")
	ROMX_LOAD( "ag56p.rom", 0x00000, 0x20000, CRC(f2737425) SHA1(e6058a98a7ca4c03d1c1b7d30602fe97d88bc04a), ROM_BIOS(1))
	// 2:
	ROM_SYSTEM_BIOS(2, "ag56s", "AG56S")
	ROMX_LOAD( "ag56s.rom", 0x00000, 0x20000, CRC(c015ed49) SHA1(4c447bab1cba9d38b99c2e36e0824809e876931e), ROM_BIOS(2))
ROM_END

// TMC Research Corporation PCI48AF - Chipset: ALi M1487, M1489, TMSIA 9347W/TC4069UBP, FDC37C665GT - CPU: Socket 3 - RAM: SIMM72x4, Cache: 4x32pin + TAG
// ISA16: 4, PCI: 4 - On board: 2xISA, Floppy, 2xpar, 2xser - Keyboard-BIOS: AMIKEY-2
ROM_START( pci48af )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0:
	ROM_SYSTEM_BIOS(0, "af2", "AF2")
	ROMX_LOAD( "pci48af2.rom", 0x00000, 0x20000, CRC(556113cc) SHA1(cbbbcaa300253e766bce5292ffdfedf72c76e287), ROM_BIOS(0))
	// 1: 03/25/96-ALI-1487/89-2A4KDM29C-00 / 486DX-AC-WBc-25/33/40/50/66/80/100/120/133-A2-ZG - boots to BootBlock BIOS
	ROM_SYSTEM_BIOS(1, "afa", "AFA")
	ROMX_LOAD( "pci48afa.bin", 0x00000, 0x20000, CRC(9127efb5) SHA1(cf77fcca00b6e48067caefa518bedb287f945147), ROM_BIOS(1))
	// 2:
	ROM_SYSTEM_BIOS(2, "f2a", "F2A")
	ROMX_LOAD( "48af-f2a.rom", 0x00000, 0x20000, CRC(1e0d8216) SHA1(c6e4342dea2b7feac6e239dc99aee65508f9c297), ROM_BIOS(2))
ROM_END

ROM_START( alim1489 ) // no display
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: // V1.2A (with fake cache SRAM) - Chipset: ALi M1489, M1487, UM8663AF, UM8667 - BIOS: 10/10/94 AMI AD0153466 (32pin) - ISA16: 4, PCI: 3
	// On board: 2xser, Game, par, Floppy, 2xIDE - OSC: 14.31818
	ROM_SYSTEM_BIOS(0, "ali148901", "ALi M1489 #1")
	ROMX_LOAD( "ali.bin", 0x00000, 0x20000, CRC(d894223b) SHA1(088a94d2425f0abc85fafa922a5c6792da608d28), ROM_BIOS(0))
	// 1: Chipset: ALi M1489 A1, M1487 B1, GoldStar Prime 3b - CPU: Am486DX4-100 - RAM: 2xSIMM72, Cache: 9x32pin, used: 5xCY7C199-15PC
	// BIOS: AMI 32pin - ISA16: 4, PCI: 3 - On board: 2xIDE, Floppy, 2xser, par
	ROM_SYSTEM_BIOS(1, "ali148902", "ALi M1489 #2")
	ROMX_LOAD( "ali_pci486v1-hj3.bin", 0x00000, 0x20000, CRC(fca45439) SHA1(a4bad38301c9e7f780a95a07b7062f0a277a7a10), ROM_BIOS(1))
ROM_END


/***** 486 motherboards using the CONTAQ 82C596 chipset *****/

// MSI MS-4125 - Chipset: CONTAQ 82C596 SiS 85C206 - ISA8: 1, ISA16: 3, ISA16/VL: 2 - BIOS: AMI 486DX ISA BIOS AA65441044 (28pin) - Keyboard-BIOS: AMI/Intel P8942AHP
// BIOS-String: 40-0104-001169-00101111-111192-CTQ596-0 / AC5E 052193
ROM_START( ms4125 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD("ms4125.bin", 0x10000, 0x10000, CRC(0e56b292) SHA1(9db26e8167b477c550d756d1ca2363283ebff3ed))
ROM_END

// Diamond Flower, Inc. (DFI) 486-CCV Rev B - Chipset: CONTAQ 82C596, KS83C206EQ - BIOS: 11/11/92 AMI AB8644083 (28pin) - Keyboard-BIOS: AMIKEY-2
// BIOS-String: 40-0100-ZZ1211-00101111-111192-CONTAQ/5-0 - OSC: 14.31818MHz - ISA8: 2, ISA16: 4, ISA16/VL: 2
ROM_START( 486ccv )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "contaq.bin", 0x10000, 0x10000, CRC(2ac46033) SHA1(a121c22ded4932e3ba8d65c2b097b898f02147c7))
ROM_END


/***** 486 motherboards using the OPTi495SLC chipset *****/

// QDI PX486P3 - Chipset: OPTi 82C495SLC, F82C206 - CPU: 486 - BIOS: 11/11/92 AMI (28pin)
// Keyboard-BIOS: AMIKEY - ISA8: 1, ISA16: 3, ISA16/VL: 3 (one marked MASTER/SLAVE, two marked SLAVE)
ROM_START( px486p3 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 40-0402-428003-00101111-111192-OP495SLC-0 / PX486DX33/50P3 IVN 2.0 19/11/1993
	ROM_SYSTEM_BIOS(0, "ivn20", "IVN 2.0")
	ROM_LOAD( "px486p3.bin", 0x10000, 0x10000, CRC(4d717aad) SHA1(2d84cf197845d58781f77e4d539ca994fd8733c8))
	// 1: BIOS-String: 40-0401-428003-00101111-111192-OP495SLC-0 / PX486DX33/50P3 IVN 1.0 25/06/1993
	ROM_SYSTEM_BIOS(1, "ivn10", "IVN 1.0")
	ROMX_LOAD( "qdi_px486.u23", 0x10000, 0x10000, CRC(c80ecfb6) SHA1(34cc9ef68ff719cd0771297bf184efa83a805f3e), ROM_BIOS(1))
ROM_END

// NAT48PV-1.0 VL - Chipset:82C495SLC, Chips F82C206J - RAM: 8xSIMM30, Cache: 9xIS61C256A - OSC: 14.31818 - BIOS: AMI 486DX ISA BIOS (28pin) AA5312581
// Keyboard-BIOS: MEGA-KB-F-WP P8042AHP - ISA8: 2, ISA16: 3, ISA16/VL: 2
// 40-040A-001291-00101111-111192-OP495SLC-0 / 486DX-OP-WBq-25/33/50-K2-ZZ
ROM_START( nat48pv )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "amibios_aa5312581.bin", 0x10000, 0x10000, CRC(8a788c79) SHA1(050972b7a369a463d6654ec52c0804002e9bcb37))
ROM_END


/***** 486 motherboards using the OPTi OPTi 82C392, 82C493, 82C206 chipset *****/

// Auva-Cam-33-P2 = See-Thru Sto486Wb - CPU: 486 - ISA8: 1, ISA16: 7 - Chipset: OPTi 82C392, 82C493, 82C206
// MR BIOS (tm) V1.30
ROM_START( sto486wb )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "opti_82c493_486_mr_bios_v130.rom", 0x10000, 0x10000, CRC(350d5495) SHA1(4f771ef5fe627e0556fb28f8972e545a0823a74d))
ROM_END

// Silicon Valley Computer, Inc. 486WB6A3.B1 - Chipset: OPTi 82C493/392, F82C206 - BIOS: AMI 486 BIOS ZZ342708 - Keyboard BIOS:AMI KB-BIOS-VER-F
// CPU: Intel 80486DX-33, secondary socket - RAM: 8xSIMM30, Cache: 9xMosel MS6264A-20NC - OSC: 33.333MHz, 14.31818 - ISA8: 1, ISA16: 6, ISA16/RAM extension: 1
ROM_START( 486wb6a3 ) // BIOS String: 40-0101-ZZ9999-00101111-060691-OPWBSX-F
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486_ami.bin", 0x10000, 0x10000, CRC(1f5e9263) SHA1(534a6ace19ba6185614e04e3bd2d0aabe1193e2c))
ROM_END

ROM_START( op82c392 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: 486-A4865-A4866-XX V2 1 COMP - CPU: 486DX-33 - Chipset: Opti 82C392, 82C493, Opti F82C206 - BIOS: 486DX AMI (28pin) - Keyboard-BIOS: AMI
	// BIOS-String: - ECB: 1, ISA8: 2, ISA16: 5 - OSC: 14.318, 66.000000MHz - RAM: 8xSIMM30, Cache: 16 sockets +1 provided
	ROM_SYSTEM_BIOS(0, "a4865", "A4865")
	ROMX_LOAD( "a4865-a4866.bin", 0x10000, 0x10000, CRC(9c726164) SHA1(b6ad8565a489b9d5991eea37905be2e6fc59fa48), ROM_BIOS(0))
	// 1: Chipset: OPTi 82C392, 82C493, UMC UM82C206L - CPU: i486DX-33, FPU socket provided - OSC: 34.000MHz, 14.31818 - Keyboard-BIOS: AMI/Intel P8942AHP
	// BIOS: AMI 486 BIOS Z600436 - BIOS-String: 40-0131-425004-01001111-070791-OPWB493-0 / ABC COMPUTER CO., LTD. - 40-0101-DK1343-00101111-00101111-060691-OPWBSX-0 - ISA8: 2, ISA16: 6
	ROM_SYSTEM_BIOS( 1, "82c493", "82C493")
	ROMX_LOAD( "486-920087335.bin", 0x10000, 0x10000, CRC(38571ffe) SHA1(aa6048213139c88901aca9cd38251a3937b6e52d), ROM_BIOS(1))
	// 2: Chipset: OPTi 82C392, 82C493, Chips 206 - CPU: two sockets provided - RAM: 8xSIPP30, on board RAM/Cache? photo too blurry - Keyboard BIOS: Intel/AMI
	// OSC: 66.666MHz, 14.31818 - ISA8: 1 (solder pads for memory slot provided), ISA16: 7 - BIOS-String: 40-0101-009999-00101111-060691-OPWBSX-0 / OPTi-WB GW486SX/DX BIOS, July 3, 1992
	ROM_SYSTEM_BIOS( 2, "060691", "06/06/91")
	ROMX_LOAD( "gw486sxdx.bin", 0x10000, 0x10000, CRC(99ecd9ce) SHA1(616bc1192c0ffb9d90f8aa32d93a8badc45f9d56), ROM_BIOS(2))
ROM_END


/***** motherboards using the OPTi 82C802A, 82C602A chipset *****/

// Edom MV035F - Chipset: OPTi 82C802A, 82C602A - CPU: TI 486DX2-80 - RAM: 4xSIMM30, 3xSIMM72, Cache: 4x32pin, 4x28pin, TAG
// BIOS: 28pin - Keyboard-BIOS: VIA - ISA16: 5, ISA16/VL: 3
// From the source: "The seller claimed that it POSTed, but all the BIOS options were grayed out. I pulled and dumped the BIOS
// chip, ran it through MODBIN and found out the OEM of the board simply set all BIOS setup options to 'SHOW-ONLY'. I set all
// except PCI CONFIGURATION to 'Enabled' (since this board doesn't have any PCI slots), blanked the EPROM and burned the modified
// BIOS to it, and now I can set all CMOS setup options normally."
// BIOS-String reflects the edit: 11/17/95-OPTi-802G-2C4UKW01-00 / Computer Spirit v1.0 (Hack 1.0 by Eep386
ROM_START( edmv035f )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "mf035fed.bin", 0x10000, 0x10000, BAD_DUMP CRC(5c1ce352) SHA1(c7944b06e4a3473bb720caa5043f4b55bccf3835))
ROM_END

// Octek Hippo DCA2 - Chipset: OPTi 802G - BIOS: 28pin - CPU: Socket 3 - ISA8: 2, ISA16: 3, ISA16/VL: 3 - RAM: 4xSIMM72, Octek claimed, Cache would be taken out of main RAM
ROM_START( ochipdca2 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 10/27/94-OPTI-802G-2C4UKO01-00 / (2C4UKO01) EVALUATION ROM - NOT FOR SALE
	ROM_SYSTEM_BIOS(0, "hv2433", "AWARD HV2433")
	ROMX_LOAD( "hv2433.awa", 0x10000, 0x10000, CRC(d6179601) SHA1(8a9c7ec959f6626268e0e242760439272fc9e28c), ROM_BIOS(0))
	// 1: beep code
	ROM_SYSTEM_BIOS(1, "h2433", "AMI H2433")
	ROMX_LOAD( "h2433.ami", 0x10000, 0x10000, CRC(a646a191) SHA1(086ae94554e3c2b292f2e32b5cb080c15dfa3e0b), ROM_BIOS(1))
	// 2: beep code L-H-H-L
	ROM_SYSTEM_BIOS(2, "mr321", "MR-BIOS 3.21") // supports AMD X5-133
	ROMX_LOAD( "095061.bin", 0x10000, 0x10000, CRC(0a58cab2) SHA1(e64d6ca0bad6eeed492260853d7d60cd2a60a222), ROM_BIOS(2))
	// 3: beep code L-H-H-L
	ROM_SYSTEM_BIOS(3, "mr31", "MR-BIOS 3.1")
	ROMX_LOAD( "dca2mr31.rom", 0x10000, 0x10000, CRC(43b7415f) SHA1(45df892d146b8e2594274773c93d1623207b40fc), ROM_BIOS(3))
ROM_END


/**** Motherboards using the Opti 82C895 82C602A chipset *****/

// ExpertChip EXP4044 - CPU: Socket3 - Chipset: OPTi 82C895, 82C602 - RAM: 4xSIMM30, 2xSIMM72, Cache: 4x28pin, 4x32pin + TAG
// Keyboard-BIOS: MEGAKEY or Winbond W83C42 - ISA16: 3, ISA16/VL: 3
// BIOS-String: 06/23/94-OPTI-802G-2C4UKD01-00 / EXP4044 GREEN BIOS Ver 1.1
ROM_START( exp4044 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD("4ecw001.bin", 0x10000, 0x10000, CRC(cf186fa4) SHA1(d65cc2f2c6feaa1a537319aaef86df12b44afdec))
ROM_END

// Jetway J-403TG
ROM_START( jwj403tg )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: JETWAY J-403TG (VER.D2E) Chipset: OPTi 82C895, 85C602 - CPU: Socket 3 - Keyboard BIOS: AMIKEY-2 -
	// RAM: 4xSIMM32, 3xSIMM72, Cache: 8xW24257AK-15, 1xW24129AK - ISA16: 5, ISA16/VLB: 3 (2xMASTER, 1xSLAVE)
	ROM_SYSTEM_BIOS(0, "verd2e", "VER.D2E") // BIOS-String: 40-0101-428060-00101111-111192-UMC491C-0
	ROMX_LOAD( "ver_d2e.bin", 0x10000, 0x10000, CRC(5d3b86bb) SHA1(d7c3cfbb5b858efacc7cee872a4ef5c9666f9d06), ROM_BIOS(0))
	// 1: JETWAY J-403TG GREEN VLB ver 2.0 - Chipset: OPTi 82C895, second IC with Energy Star logo - BIOS: Award 486DX J156079 - Keyboard BIOS: VIA VT82C42N
	// CPU: Socket 3 - RAM: 4xSIMM30, 3xSIMM72, Cache: 8xW24257AK-15 - ISA8: 2, ISA16: 3, ISA16/VLB: 3
	ROM_SYSTEM_BIOS(1, "greenver20", "GREEN VLB ver 2.0") // BIOS-String: 07/21/94-OPTI-802G-2C4UKJ11-00 / V1.A
	ROMX_LOAD( "403tg.bin", 0x10000, 0x10000, CRC(41ebe3e8) SHA1(567d61c5912cfb5fbfc9a1b674e7edad09a2165c), ROM_BIOS(1))
	// 2: JETWAY J-403TG VLB Rev D - Chipset: OPTi 82C895, 82C602 - BIOS: AMI 486DX ISA BIOS AB5257763 - Keyboard BIOS: JETkey V5.0
	// CPU: 80486DX - RAM: 4xSIMM30, 3xSIMM72, Cache: 8x32pin DIP (used: 8xH61256-20) - ISA8: 2, ISA16: 3, ISA16/VLB: 3
	ROM_SYSTEM_BIOS(2, "revd", "Rev D") // BIOS-String: 40-P301-001276-00101110-121593-OPTi895-H - blank screen
	ROMX_LOAD( "b2790126", 0x10000, 0x10000, CRC(b2790126) SHA1(dad277c91dac9daffcd1e3f3e9a1a1e59c92e72e), ROM_BIOS(2))
	// 3: MR BIOS for the 82C895 chipset - MR BIOS (r) V2.02
	ROM_SYSTEM_BIOS(3, "82c895", "82C895")
	ROMX_LOAD("opt895mr.mr", 0x10000, 0x10000, CRC(516cb091) SHA1(4c5b51cd05974001da4b764b4b14987657770a45), ROM_BIOS(3))
ROM_END

// QDI V4P895P3/SMT V5.0 - Chipset: Opti 82C895 82C602A - CPU: Am486DX2-66 - ISA8: 1, ISA16: 3, ISA16/VL: 3
// RAM: 4xSIMM30, 2xSIMM72, Cache: 8xUM61256FK-15 - BIOS: AMI 486DX ISA BIOS Ac0928698 (28pin in a 32pin socket) - Keyboard-BIOS: AMIKEY-2
ROM_START( v4p895p3 ) // no display
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486-v4p895p3-smt.bin", 0x10000, 0x10000, CRC(683f8470) SHA1(eca1c21a8f8c57389d9fdf1cd76d2dec0928524a))
ROM_END

// Shuttle HOT-419 - Chipset: OPTi 92C895A, 82C602A - CPU: Socket 3 - RAM: 4xSIMM72, Cache: 8+1 UM61256K-15 - ISA8: 2, ISA16:3, ISA16/VL: 3
// BIOS: AMI AB0585433 (28pin) - Keyboard-BIOS: AMIKEY-2
ROM_START( hot419 ) // no display
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0:
	ROM_SYSTEM_BIOS(0, "072594", "07/25/94")
	ROMX_LOAD( "hot419_original_bios.bin", 0x10000, 0x10000, CRC(ff882008) SHA1(1a98d61fd49a2a07ff4f12ccba55cba11e4fde23), ROM_BIOS(0))
	// 1:
	ROM_SYSTEM_BIOS(1, "419aip06", "419AIP06")
	ROMX_LOAD( "419aip.rom", 0x10000, 0x10000, CRC(389ca65d) SHA1(457491c60aa45499e2cd8dad9db3bf3312977a4f), ROM_BIOS(1))
ROM_END

// TMC PAT48PG4-V1.20 - BIOS Version: Award 10/13/95 - Chipset: OPTi 82C895+82C602 - EPROM Label: 486 AWARD SOFTWARE 1984-1995 T1103040
// Keyboard BIOS: AMIKEY-2 - CPU: socket for 80486 - RAM: 4xSIMM30, 2xSIMM72, Cache: 9xISSI IS61M256-15N - ISA16: 4, ISA16, VLB: 3
ROM_START( tmpat48pg4 ) // BIOS-String: 10/13/95-OPTI-802G-2C4UKM21-00.00-00 / 486DX-OP-WOe-25/33/40/50/66/80/100/120-C7-ZG
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "award.bin", 0x10000, 0x10000, CRC(c217214b) SHA1(583820b2fe96ca4bfacf9267800afe7cc76e5ffa))
ROM_END


/***** Motherboards using the SiS 85C461 chipset *****/

// Abit AB-AV4 (aka VL-BUS 486) - Chipset: SiS 85C461, HM5818A - BIOS Version: AMI 11/11/92 486DX ISA BIOS AA7247480
// Keyboard BIOS: AMI-KB-H-WP - CPU: socket for 80486PGA, solder pads for QFP486sx - RAM: 8xSIMM30, Cache: 7xW24257AK-20, 1xEm81256B-20P, 1x71256S20TP
// ISA16: 5, ISA16/VLB: 3 - OSC: 14.31818
ROM_START( abav4 ) // BIOS String: 40-01BB-001247-00101111-111192-SIS461 / CACHE 486 SYSTEM BIOS
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "amibios.bin", 0x10000, 0x10000, CRC(7bf8142e) SHA1(17e09eebabc0d2c393d27db38b571af3c0ccbb41))
ROM_END

// ASUS ISA-486SV2 - Chipset: SiS 85C461 - BIOS: AMI 486DX ISA BIOS AA7892378 28pin - Keyboard-BIOS: Intel/AMI
// BIOS-String: 40-110A-001292-00101111-111192-I486SI-0 - ISA16: 5, ISA16/VL: 2 - CPU: 486DX in a blue socket (overdrive ready)
ROM_START( a486sv2 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486-asus isa-486sv2.bin", 0x10000, 0x10000, CRC(de925130) SHA1(2e3db7a1d4645082290d6303a16446af2959f34a))
ROM_END

// GENOA TurboExpress 486 VL ASY 01-00302 - Chipset: SiS 85C407 85C461 - CPU: Socket3 - OSC: 14.31818MHz - ISA16: 4, ISA16/VL: 3 - BIOS: AMI 486DX ISA BIOS AB0562153 (28pin)
// BIOS-String: 40-0100-006156-00101111-080893-SIS461-0 / GENOA TurboExpress 486VL - 3 (Ver. C) - Keyboard-BIOS: AMIKEY
ROM_START( gete486vl )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486-genoa_vlb.bin", 0x10000, 0x10000, CRC(9be0f329) SHA1(3b1adedd6aad40c623757e4976e0dcadb253f255))
ROM_END

// Lucky Star UCM-486V30 (aka SIS486 3-VLBUS) - BIOS/Version: AMI 01/14/1993 SUPPORT VESA, 486DX ISA BIOS AA8580239 - Keyboard BIOS: AMI MEGA-KB-H-WP
// Chipset: SIS 85C461, HM6818A - CPU: P24T socket - OSC: 14.31818 - RAM: 8xSIMM30, Cache: 9x28pin DIP
ROM_START( lsucm486v30 ) // BIOS string: 40-0100-001256-00101111-111192-SIS3486-0
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "ucm-486v30.bin", 0x10000, 0x10000, CRC(90ec73a2) SHA1(2fc17f7eed09c2f7d0139670677cc84bbc2964de))
ROM_END

// SOYO 486 VESA 025D2 - Chipset: SiS 85C461, 85C407 - BIOS: AMI - Keyboard BIOS: AMI (c)1988 - CPU: i486sx-33, full 486 socket provided
// RAM: 8xSIMM30, Cache: 8x28pin - ISA8: 1, ISA16: 4, ISA16/VL: 2 - BIOS-String: 40-0100-001102-00101111-080893-SIS461 / REV MG
ROM_START( so025d2 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "soyo-486-vesa.bin", 0x10000, 0x10000, CRC(da798d7b) SHA1(15a8c1e244ee29ef5c61e05659f8ec7f8eaa8ab7))
ROM_END

/***** 486 motherboards using the SiS BTQ 85C401/85C402 + 85C206 chipset *****/

// ABIT AB-AX4 - Chipset: SIS BTQ 85C401, 85C402, 85C206 - BIOS: AMIBIOS 06/06/92 - Keyboard BIOS: AMI - CPU: socket for 80486
// RAM: 8xSIMM30, Cache: 8xEtronTech Em51256A-20P, 1X AS7C256-20PC - ISA8: 1, ISA16: 6 - OSC: 33.333MHz, 14.31818
ROM_START( abax4 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: X0-01AA-001247-00101111-060692-SISAUTO-0 / AX4 ISA SYSTEM BIOS
	ROM_SYSTEM_BIOS(0, "060692", "ABIT AB-AX4 06/06/92")
	ROMX_LOAD( "486dx_dump.bin", 0x10000, 0x10000, CRC(c7af4380) SHA1(59507a0f7d929ac19c5b56334f54643127c0d2be), ROM_BIOS(0))
	// 1: BIOS-String: 30-0200-D01247-00101111-070791-SIS486 / AT486DX 33MHz BIOS
	ROM_SYSTEM_BIOS(1, "070791", "ABIT AB-AX4 07/07/91")
	ROMX_LOAD( "486-aa1177369.bin", 0x10000, 0x10000, CRC(530535de) SHA1(7f6e627a77ebcaec97f08e6c797d31e9321e26fc), ROM_BIOS(1))
ROM_END

// ASUS ISA-486 - Rev. 1.4 - Chipset: SiS BTQ 85C401/85C402 + 85C206 - BIOS/Version: AMI 486DX ISA BIOS 05/05/91 AA1258865 - CPU: Intel 80486DX-33, FPU socket for 4167 provided
// RAM: 8xSIMM30, Cache: 8x28pin DIP (4x71256 fitted) - OSC: 14.31818, 33.000MHz - DIP6: 111000 - ISA8/RAM extension: 1, ISA16: 7 -
ROM_START( a486isa ) // BIOS String: 40-0102-001292-00101111-050591-SIS-486-0
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "isa-486.bin", 0x10000, 0x10000, CRC(57375912) SHA1(8035b5d1cfe824a20a94571a57b86fdb4018f073))
ROM_END

// Mitac MBA-029 - Chipset: SIS BTQ 85C401, 85C402, 85C206 - BIOS: AMI - CPU: 486 socket - OSC: 14.34818, xxxxx (unreadable)
// RAM: 4xSIMM30, Cache: 8x28pin - ISA 8: 1 (not soldered in), ISA16: 6
ROM_START( mba029 ) // BIOS-String: 30-0200-ZZ1594-00101111-070791-SISAUTO-0
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "mba-029_bios_27c512.bin", 0x10000, 0x10000, CRC(8d660697) SHA1(6b2be9ec9a2d12c9348c26ac25514af406fa752e))
ROM_END


/***** 486 motherboards using the SiS 85C496/85C497 chipset *****/

// A-Trend ATC-1425A - Chipset: SiS 85C496, 85C497 - RAM: 4xSIMM72, Cache: 4x32pin + TAG - ISA16: 4, PCI: 3
// on board: 2xIDE, Floppy, 2xser, par - BIOS: 32pin
ROM_START( atc1425a )
	ROM_REGION32_LE(0x20000, "bios", 0)
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
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "1425b231.rom", 0x00000, 0x20000, CRC(1a19f34d) SHA1(09bb5e35ef07b57942cbca933f2a0334615a687e))
ROM_END

// Abit AB-PI4(T) - Bios: 32pin - Keyboard-BIOS: Winbond 83C42 - CPU: Socket 3 - ISA16: 4, PCI: 3 - Chipset: SiS 85C495, 85C497
// RAM: 4xSIMM72, Cache: 9x32pin (occupied: 4xW24512AK-20, 1xW2457AK) - On board: 2xIDE
ROM_START( abpi4 )
	ROM_REGION32_LE(0x20000, "bios", 0)
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
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "pm4_0c.bin", 0x00000, 0x20000, CRC(eaad7812) SHA1(81670c44e30fa8b8ac0aa28a5c367819ff1ca73c))
ROM_END

// Abit AB-PV4
// BIOS-String: 09/26/95-SiS-496-497/A/B-2A4IBA12C-0A / GREEN 486 PCI SYSTEM BIOS
ROM_START( abpv4 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "pv4v_0a.bin", 0x00000, 0x20000, CRC(91de48d5) SHA1(2e873de152870270f51b5b2c4a30f2611364e739))
ROM_END

// Aopen AP43 - CPU: Socket 3 - Chipset: SiS 85C496, 85C497, SMC FDC37C665GT - RAM: SIMM72x4, Cache: 9x32pin, used: 9xUM61256FK-15
// BIOS: 32pin - Keyboard-BIOS: AMIKEY-2 - on board: IDEx2, Floppy, par, 2xser
// constant chirping
ROM_START( aoap43 )
	ROM_REGION32_LE(0x20000, "bios", 0)
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
	ROM_REGION32_LE(0x20000, "bios", 0)
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
	ROM_REGION32_LE(0x20000, "bios", 0) // Winbond W29EE011-15
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
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 12/21/95-SiS-496-497/A/B-2A4IBC3IC-00
	ROM_SYSTEM_BIOS(0, "original", "original")
	ROMX_LOAD( "chaintech_486spm.bin", 0x00000, 0x20000, CRC(a0c9045a) SHA1(1d0b1994574437549c13541d4b65374d94c9a648), ROM_BIOS(0))
	// 1: 12/21/95-SiS-496-497/A/B-2A4IBC3IC-00 / Chaintech 486SPM v2015 PS/2
	ROM_SYSTEM_BIOS(1, "ps2", "PS2 mouse enabled")
	ROMX_LOAD( "486spm-p.bin", 0x00000, 0x20000, CRC(35b5cb76) SHA1(965b212b28a5badd8d8f4769aa9edc88e47bc925), ROM_BIOS(1))
ROM_END

// Chaintech 4SPI - Chipset: SiS 85C496 85C497 - BIOS Version: Award v4.50G E0671975 - Keyboard BIOS: Lance Green LT38C41
// CPU: Socket 3 - RAM: 4xSIMM72, Cache: 9x32pin DIP (used: 4xW24512AK-15, 1xEM51256-15PL) - On board: 2xIDE
// ISA6: 5, PCI: 3
ROM_START( ch4spi ) // BIOS String: 02/16/95-SiS-496-497/A/B-2A4IBC31-B2 / 02/17/1995
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "4spibckp.bin", 0x00000, 0x20000, CRC(29b15737) SHA1(e9cb5402eb25a100a15d5ccc520cfa76c7be99a6))
ROM_END

// Freetech 486F55 - Chipset: SiS 496/497 - CPU: Socket 3 - RAM: 4xSIMM72, Cache: 128KB/256KB/512KB - ISA16: 4, PCI: 3 -
// On board: 2xser, par, 2xIDE, Floppy - BIOS: Award
ROM_START( ft486f55 )
	ROM_REGION32_LE(0x20000, "bios", 0)
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
ROM_START( jakms41 ) // BIOS String: 10/30/95-SiS-496-497/A/B-2A4IBR22C-00 - boots into BootBlock BIOS
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "km-s4-1.bin", 0x00000, 0x20000, CRC(0271356a) SHA1(893048c3390a23810a2af14da30520fbea10ad2f))
ROM_END

// SOYO SY-4SAW2 - Chipset: SiS 85C497, 85C496, Winbond W83787F - CPU: Socket 3 - RAM: 4xSIMM72, Cache: 4xUM61512AK-15+W24129AK-15
// BIOS: Award (32pin) - Keyboard-BIOS: Via VT82C42N - ISA16: 3, ISA16/VL: 1, PCI: 4 - On board: 2xser, par, 2xIDE, Floppy
// keeping the ROMs for the 4SA boards here until the differences between the boards are clear, e.g. difference between SY-4SAW and 4SA2: L2-cache
ROM_START( so4saw2 )
	ROM_REGION32_LE(0x20000, "bios", 0)
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
ROM_START( zito4dps ) // all revisions land in the Award Boot block BIOS
	ROM_REGION32_LE(0x20000, "bios", 0)
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

// Jetway J-446A - Chipset: SiS 85C497, 82C496 - CPU: Socket 3 - RAM: 4xSIMM72, Cache: 4+1 sockets - BIOS: 32pin
// Keyboard-BIOS: HOLTEK HT6542B - ISA16: 3, PCI: 3 - On board: 2xIDE, Floppy, par, 2xser
ROM_START( jwj446a ) // BootBlock BIOS
	ROM_REGION32_LE(0x20000, "bios", 0)
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
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: Rev:C - no display
	ROM_SYSTEM_BIOS( 0, "revc01", "Rev.C #1")
	ROMX_LOAD( "ls486e_revc.bin", 0x00000, 0x20000, CRC(d678a26e) SHA1(603e03171b28f73bdb6ce27b0bbae2a4cfb13517), ROM_BIOS(0))
	// 1: LS486E Rev.D SiS496/497(PR/NU) EDO Support AWARD 10/21/96 - 10/21/96-SiS-496-497/A/B-2A4IBL12C-00 - 486E 96/10/24 UMC8669 PLUG & PLAY BIOS
	ROM_SYSTEM_BIOS( 1, "revd01", "Rev.D #1") // boots to BootBlock BIOS
	ROMX_LOAD( "ls486-d.awa", 0x00000, 0x20000, CRC(5a51a3a3) SHA1(6712ab742676156802fdfc4d08d687c1482f2702), ROM_BIOS(1))
	// 2: Lucky Star LS486E rev.C,Winbond,SiS496/497  - BIOS Award PNP v4.50PG (486E 96/5/17 W83787) - BIOS-String: 03/14/96-SiS-496-497/A/B-2A4IBL13C-00 / 486E 96/5/17 W83787
	ROM_SYSTEM_BIOS( 2, "revc02", "Rev.C #2") // boots to BootBlock BIOS
	ROMX_LOAD( "ls486e-c.awd", 0x00000, 0x20000, CRC(8c290f20) SHA1(33d9a96e5d6b3bd5776480f5535bb1eb1d7cff57), ROM_BIOS(2))
	//3: BIOS-String: 03/14/96-SiS-496-497/A/B-2A4IBL13C-00 / 486E 96/7/19 W83787 PLUG & PLAY BIOS - boots to BootBlock BIOS
	ROM_SYSTEM_BIOS( 3, "revc1", "Rev.C1") // also on a Rev.C2 board
	ROMX_LOAD( "ls486ec1.bin", 0x00000, 0x20000, CRC(e96d1bbc) SHA1(64d0726c4e9ecee8fddf4cc39d92aecaa8184d5c), ROM_BIOS(3))
	// 4: BootBlock BIOS
	ROM_SYSTEM_BIOS( 4, "lh5", "LH5")
	ROMX_LOAD( "ls-486e.bin", 0x00000, 0x20000, CRC(03ca4a97) SHA1(f9e5e2f2fabcb47960dfa91c37bf74fa93398092), ROM_BIOS(4))
	// 5: BIOS-String: 03/14/96-SiS-496-497/A/B-2A4IBL13C-00 - boots to BootBlock BIOS
	ROM_SYSTEM_BIOS( 5, "ls486eb", "LS-486E(B)")
	ROMX_LOAD( "4siw001.bin", 0x00000, 0x20000, CRC(d81d722d) SHA1(bb18324b3679b7419c230244891b626a61006486), ROM_BIOS(5))
ROM_END

// MSI MS-4144 - Chipset: SiS 85C497, 85C496, Winbond W83787F, W83758F - CPU: Socket 3 - RAM: 4xSIMM72, Cache: 8+1 sockets
// On board: 2xIDE, Floppy, 2xser, par - ISA16: 4, PCI: 3
ROM_START( ms4144 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: no display
	ROM_SYSTEM_BIOS(0, "af53", "AF53")
	ROMX_LOAD( "ms-4144_af53.rom", 0x00000, 0x20000, CRC(931ebb7d) SHA1(fa7cf64c07a6404518e12c41c197354c7d05b2d2), ROM_BIOS(0))
	// 1: no display
	ROM_SYSTEM_BIOS(1, "af54", "AF54")
	ROMX_LOAD( "ms-4144_af54s.rom", 0x00000, 0x20000, CRC(1eb02779) SHA1(b18cc771fc5a820437a4daca06806188ee1a27a5), ROM_BIOS(1))
	// 2: BIOS-String: 03/20/96-SiS-496-497/A/B-2A4IBM49C-00 / WF53S 032096 - boots to BootBlock BIOS
	ROM_SYSTEM_BIOS(2, "wf53", "WF53")
	ROMX_LOAD( "ms-4144_wf53s.bin", 0x00000, 0x20000, CRC(df83f099) SHA1(b7dc61a2cb71754cddd06d12d3bf81ffce442c89), ROM_BIOS(2))
	// 3: BIOS-String: 02/07/96-SiS-496-497/A/B-2A4IBM49C-00 / WF54S 020896 - boots to BootBlock BIOS
	ROM_SYSTEM_BIOS(3, "wf54", "WF54")
	ROMX_LOAD( "ms-4144_wf54s.bin", 0x00000, 0x20000, CRC(c0ff31df) SHA1(4e138558781a220b340977d56ccbfa61a907d4f5), ROM_BIOS(3))
	// 4: no display - VER 2.1 - BIOS: AMI 486DX ISA BIOS AC8999569 (32pin)- Keyboard-BIOS: AMIKEY-2
	ROM_SYSTEM_BIOS(4, "v21", "Ver 2.1")
	ROMX_LOAD( "486-pci-ms4144.bin", 0x00000, 0x20000, CRC(8bd50381) SHA1(c9853642ac0946c2b1a7e469bcfacbb3351c4067), ROM_BIOS(4))
ROM_END

// SOYO 30H - CPU: Socket 3 - RAM: SIMM72x4 - Cache: 256K, 512K or 1024K - ISA16: 4, PCI: 3 - on board: 2xIDE
// BIOS-String: 12/07/95-SiS-496-497/A/B-2A4IBS2AC-00 / REV B2
ROM_START( so30h )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "30h-b2.bin", 0x00000, 0x20000, CRC(1dd22cef) SHA1(dd0ac15e7a792e8fba2f55d6a1b35256e74bcf4e))
ROM_END

ROM_START( sis85c496 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// Chipset: SiS 85C496/85C497 - CPU: Socket 3 - RAM: 2xSIMM72, Cache - Keyboard-BIOS: JETkey V5.0
	// ISA16: 3, PCI: 3 - BIOS: SST29EE010 (128k) AMI 486DX ISA BIOS AA2558003 - screen remains blank
	ROM_LOAD( "4sim002.bin", 0x00000, 0x20000, CRC(ea898f85) SHA1(7236cd2fc985985f21979e4808cb708be8d0445f))
ROM_END


/***** 486 motherboards using the SiS 85C471 + 85C407 chipset *****/

// 486IG-B-2-1 - CPU: 486 - RAM: 8xSIMM30, Cache: 4x32pin, 4x28pin, TAG - Chipset: SIS85C471/407 - BIOS: AMI WIN BIOS 07/25/94
// Keyboard-BIOS: AMIKEY - BIOS-String: 40-0002-428020-00101111-080893-SIS471B-F - ISA8: 1, ISA16: 3, ISA16/VL: 3
ROM_START( 486igb21 ) // display remains blank
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486igb21.amw", 0x10000, 0x10000, CRC(b62cc7af) SHA1(048f80ad995a1516f97bb7544d3fb608a93893b1))
ROM_END

// Abit AH4/AH4T/AN4R2 ( the T model has a voltage regulator for DX4 CPUs) - CPU: Socket 3 - Chipset: SIS 85C471 / SIS 85C407
// RAM: 4xSIMM72, Cache: 9x28pin (32pin sockets except TAG) - BIOS: AMI - Keyboard-BIOS: AMIKEY - OSC: 14.31818 - ISA8: 1, ISA16: 4, ISA16/VL: 3
// BIOS-String: 08/30/95-SIS-85C471-2C4I9A12-02
ROM_START( abah4 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "ah4t_an4r2_02.bin", 0x10000, 0x10000, CRC(b45dc3b7) SHA1(94206ac9ed50fc37d954cc3cd1fb062fd75ea984))
ROM_END

// Aopen VI15G - Chipset: SiS 85C471, 85C407 - CPU: Socket 3 - RAM: 4xSocket72, Cache: 4x32pin, 4x28pin, TAG - BIOS: 28pin
// Keyboard-BIOS: AMIKEY-2 - ISA16: 4, ISA16/VL: 3
ROM_START( aovi15g )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0:
	ROM_SYSTEM_BIOS(0, "072594", "07/25/94")
	ROMX_LOAD( "amiwinch.bin", 0x10000, 0x10000, CRC(9ee72bef) SHA1(25f08714a384d68777d1570cf28a4db8b390257e), ROM_BIOS(0))
	// 1:
	ROM_SYSTEM_BIOS(1, "r23", "R23")
	ROMX_LOAD( "vi15gr23.rom", 0x10000, 0x10000, CRC(b704f571) SHA1(485795b8756069723dac865919dc3915a3162c12), ROM_BIOS(1))
ROM_END

// ASUS VL/I-486SV2G (GX4) (4xSIMM72, Cache: 128/256/512/1024KB, 7 ISA, 2 VLB)
// SiS 85C471 + 85C407; AMIKEY-2
ROM_START( a486sv2g )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 11/17/94-SIS-85C471-I486SV2G-00 / #401A0-0304
	ROM_SYSTEM_BIOS(0, "v304", "ASUS VL/I-486SV2G (GX4) V3.04")
	ROMX_LOAD( "sv2g0304.bin", 0x10000, 0x10000, CRC(cceabe6f) SHA1(45d0e25603045255d1ccaf5cbddd1a9146f61529), ROM_BIOS(0))
	// 1: BIOS-String: 01/11/95-SIS-85C471-I486SV2G-00 / #401A0-0305-1
	ROM_SYSTEM_BIOS(1, "v305", "ASUS VL/I-486SV2G (GX4) V3.05")
	ROMX_LOAD( "0305.001", 0x10000, 0x10000, CRC(9f2f9b75) SHA1(789807d82e39d69f948f7897f99b2fe362330dd1), ROM_BIOS(1))
	// 2: BIOS-String: 03/28/95-SIS-85C471-I486SV2G-00 / #401A0-0306 - complains about BIOS ROM checksum error
	ROM_SYSTEM_BIOS(2, "v306", "ASUS VL/I-486SV2G (GX4) V3.06")
	ROMX_LOAD( "asus_0306.bio", 0x10000, 0x10000, BAD_DUMP CRC(c87b7b55) SHA1(651938bcfdf6813a1e66c0e1b4812efe91740c91), ROM_BIOS(2))
	// 3: BIOS-String: 08/22/95-SIS-85C471-I486SV2G-00 / #401A0-0401
	ROM_SYSTEM_BIOS(3, "v401", "ASUS VL/I-486SV2G (GX4) V4.01")
	ROMX_LOAD( "sv2g0401.bin", 0x10000, 0x10000, CRC(f544f65a) SHA1(9a5e39cfbd545a0026f959b42dbc742246205b3c), ROM_BIOS(3))
	// 4: BIOS-String: 11/03/95-SIS-85C471-I486SV2G-00 / #401A0-0402-1
	ROM_SYSTEM_BIOS(4, "v402", "ASUS VL/I-486SV2G (GX4) V4.02")
	ROMX_LOAD( "sv2g0402.bin", 0x10000, 0x10000, CRC(db8fe666) SHA1(e499da86261bc6b312a6bc3d94b9465e17c5a449), ROM_BIOS(4))
	// 5: BIOS-String: 11/19/97-SIS-85C471-I486SV2GC-00 / #401A0-0402-1
	ROM_SYSTEM_BIOS(5, "v402b", "ASUS VL/I-486SV2G (GX4) V4.02 beta")
	ROMX_LOAD( "0402.001.bin", 0x10000, 0x10000, CRC(4705a480) SHA1(334c3d57cb6cb157798cd189207288c731a4dd7b), ROM_BIOS(5))
ROM_END

// Chaintech 486SLE M106 4SLE-Z1 - Chipset: SiS 85C407 85C471 - CPU: i486DX2-66 - BIOS: Award v4.50G - Keyboard-BIOS: Lance LT48C41
// BIOS-String: 11/09/94-SIS-85C471E-2C4I9C31-00 / 11/24/94 - ISA8: 1, ISA16: 3, ISA16/VL: 3 - OSC: 14.31818
ROM_START( ch4slez1 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486-chaintech_486_sle.bin", 0x10000, 0x10000, CRC(8292bdb7) SHA1(461d582ea9fee4113d3a8ac050f76c7057ead7c7))
ROM_END

// Gemlight GMB-486SG rev 2.2 - Chipset: SiS 85C471 85C407 - BIOS/Version: Award - Keyboard BIOS: JETkey V5.0G
// CPU: 80486DX2-66 - RAM: 4xSIMM72, Cache: 5x 28pin DIP (TI256 SA 20TP fittet), 4x32pin DIP (W24257AK-15 fitted) - ISA8: 1, ISA16: 2, ISA16/VLB: 3
ROM_START( gmb486sg ) // BIOS-String: 01/10/95-SIS-85C471B/E/G-2C4I9G30-00
	ROM_REGION32_LE(0x20000, "bios", 0) // screen remains blank
	ROM_LOAD( "gmb486sg.bin", 0x10000, 0x10000, CRC(1f199b35) SHA1(0c4b19762426a30f7121c5c17f1b25a54a5df1f0))
ROM_END

// Gigabyte GA-486VF REV.6 - Chipset: SiS 85C407 85C471 - CPU: Cyrix Cx486 DX 40 - BIOS: Award L4162439, 28pin - Keyboard-BIOS: Lance LT38C41
// BIOS-String: 04/27/94-SIS-85C471-2C4I8G01-00 - ISA8: 1, ISA16: 3, ISA16/VL: 3 - OSC: 14.318MHz
ROM_START( ga486vf )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "ga-486svf.bin", 0x10000, 0x10000, CRC(e9fb3153) SHA1(b8e307658f95c3e910728ac9316ad83e7afdb551))
ROM_END

// Gigabyte GA-486VS - CPU: 486 - Chipset: SiS 85C471, 85C407 - Keyboard-BIOS: Lance LT38C41 - ISA16: 3, ISA16/VL: 3
// BIOS-String: 11/21/94-SIS-85C471B/E/G/2C4I9G01-00 / Nov 21, 1994 Rev.A
ROM_START( ga486vs )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "vs1121.rom", 0x10000, 0x10000, CRC(0afadecf) SHA1(66c0655b5c4905438603097998a98407bfa376e6))
ROM_END

// MSI MS-4132 G VER:1 - Chipset: SiS 85C471, 85C407, BIOS: AMI 486DX ISA BIOS 08/08/93 AB4827039 - CPU: SOCKET 3, solder pads for 8486
// RAM: 4xSIMM30, 2xSIMM72, Cache: 5xW24257AK-15, 4xIS61C256AH-15N - ISA16: 4, ISA16/VLB: 3 (2 master, 1 slave) - OSC: 14.31818MHz
ROM_START( ms4132 ) // BIOS String: 40-0100-001169-00101111-080893-SIS471B / A75A 033194
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "ms4132g.bin", 0x10000, 0x10000, CRC(23385e9d) SHA1(3637febf6e037aec9328d99877550ee9dee4c78c))
ROM_END

// MSI MS:4138 VER:1.3 - Chipset: SiS 85C471, 85C407 - CPU: Socket 3 - BIOS: EPROM/AMI 486DX ISA BIOS AC0250679
// Keyboard-BIOS: Winbond W83C42 - BIOS-String: - ISA16: 4, ISA16/VL: 3
ROM_START( ms4138 )
	ROM_REGION32_LE( 0x20000, "bios", 0)
	// 0: no display
	ROM_SYSTEM_BIOS( 0, "a75n", "A75N")
	ROMX_LOAD( "a75n.rom", 0x10000, 0x10000, CRC(f9b2130c) SHA1(7798b68275e547e858ba162abc5cf94dd6a85f4c), ROM_BIOS(0))
	// 1: no display
	ROM_SYSTEM_BIOS( 1, "msi4138", "MSI MS-4138")
	ROMX_LOAD( "ms-4138.bin", 0x10000, 0x10000, CRC(5461c523) SHA1(adb9fe0afa860897d575403a810ff44c85b9f93c), ROM_BIOS(1))
	// 2: BIOS-String: 08/14/95-SIS-85C471B/E/G-2C3I9W40-00 / W753BETA 26JAN96
	ROM_SYSTEM_BIOS( 2, "w753beta", "W753BETA")
	ROMX_LOAD( "w753beta.bin", 0x10000, 0x10000, CRC(4aeeba0b) SHA1(9d088c940599110ce3acea84bb881a61d42b6dcf), ROM_BIOS(2))
ROM_END

// DTK PKM-0038S E-2A  aka Gemlight GMB-486SG - Chipset: SIS 85C471, 85C407 - BIOS/Version: 01/10/95 Award (DTK PKM0038S.P02.03.02), 28pin - Keyboard-BIOS: JETkey V5.0
// CPU: Socket 3 - ISA8: 1, ISA16: 3, ISA16/VL: 3 - OSC: 14.318
ROM_START( pkm0038s )
	ROM_REGION32_LE( 0x20000, "bios", 0)
	// 0: BIOS-String: 01/10/95-SIS-85C471B/E/G-2C4I9G30-00 / (2C4I9G30) DTKPKM0038S.P2.03.02
	ROM_SYSTEM_BIOS(0, "p20302", "P2.03.02")
	ROMX_LOAD( "pkm0038s.bin", 0x10000, 0x10000, BAD_DUMP CRC(f6e7dd88) SHA1(5a2986ff0e6352ade8d5b0abaa86e436dddcf226), ROM_BIOS(0)) // BIOS ROM checksum error
	// 1: BIOS-String: 07/11/94-SIS-85C471E-2C4I9G30-00 / (2C4I9G30) DTKPKM0038S.P02.02.0
	ROM_SYSTEM_BIOS(1, "p2020", "P2.02.0")
	ROMX_LOAD( "4siw005.bin", 0x10000, 0x10000, CRC(0b8794d3) SHA1(153b38f6815a8a3d6723eb17df7ffc37054b3194), ROM_BIOS(1))
ROM_END

// EFA 4DMS HL3G-L4-VI - Chipset: SIS-85C471B/E/G / SIS471E/G - CPU: Socket 3 - RAM: 2xSIMM72, Cache: 8x32pin+TAG - ISA16: 5, ISA16/VL: 3
// BIOS: Award - BIOS-String: 12/07/95-SIS-85C471B/E/G-2C4I9E30-00 / 4DMS-HL3GC Award BIOS REV 1.05 01/12/'96
ROM_START( 4dmshl3g )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "3g105.bin", 0x00000, 0x20000, CRC(60e4841f) SHA1(60ad11e4e4a60eef858d837470a9014706e7576a))
ROM_END

// Soyo 025K2 - Chipset: SiS 85C471, 85C407 - BIOS: Award BIOS ISA 486 S/N 240383 - Rev G2 - 09/26/94 - Keyboard BIOS: JETkey V5.0
// CPU: Socket 3 - RAM: 4xSIMM30, 2xSIMM72, Cache: 8xtm11256-20, 1xAster AE88128AK-15 - ISA16: 4, ISA16/VLB: 3 (one marked MASTER)
ROM_START( so025k2 ) // BIOS-String: 09/26/94-SIS-85C471B/E/G-2C4I9S23-00 / REV .G2.
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "sy025k2.bin", 0x10000, 0x10000, CRC(a75fdf9a) SHA1(ce7a595ec3bb33acac76a72f704a58e08d54847a))
ROM_END

// Zida 4DVS - Chipset:  SiS 85C471, 85C407 - CPU: Socket 3, RAM: 4xSIMM72, Cache: 4x28pin, 4x32pin, TAG
// ISA8: 1, ISA16: 3, ISA16/VL: 3 - Keyboard-BIOS: AMIKEY-2
// BIOS-String:
ROM_START( zi4dvs )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: 02/09/95-SIS-85C471B/E/G-2C4I9000-00 / 12/10/95 SIS 85C471G
	ROM_SYSTEM_BIOS(0, "4dvs20", "4DVS20")
	ROMX_LOAD( "4dvs20.awa", 0x10000, 0x10000, CRC(831d33cb) SHA1(e5c3f01a9c93a7cf9dbcdc750e87952a5b6a5cf4), ROM_BIOS(0))
	// 1: blank screen
	ROM_SYSTEM_BIOS(1, "072594", "AMI 07/25/94")
	ROMX_LOAD( "4dvs-471.amw", 0x10000, 0x10000, CRC(da749314) SHA1(686321ffa59cd2259f4fe65a28b86c88cf739393), ROM_BIOS(1))
	// 2: blank screen
	ROM_SYSTEM_BIOS(2, "121593", "AMI 12/15/93")
	ROMX_LOAD( "486-ab8068594.bin", 0x10000, 0x10000, CRC(92eee700) SHA1(bd34360cf9a9849e0805cdb575cd7a0e007dd2f5), ROM_BIOS(2))
ROM_END

ROM_START( sis85c471 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: Chipset: SiS 85C407, another chip with the Energy Star/Green PC label (85C441) - CPU: 486 - BIOS: Award BIOS ISA 486 036875 - Keyboard-BIOS: Lance LT38C41
	// BIOS-String: 04/28/94-SIS-85C471-2C4I8S21-00 / REV. B - ISA16: 4, ISA16/VL: 3 (2 master)
	ROM_SYSTEM_BIOS(0, "revb", "REV. B")
	ROMX_LOAD("486-sis_green.bin", 0x10000, 0x10000, CRC(9d3b5022) SHA1(f11b27bb24deb2466226486cf8ba66ddbeff87d6), ROM_BIOS(0))
	// 1: Chipset: SiS 85C407 85C471 - CPU: Cyrix Cx486DX2-66 - BIOS: Award E0042537 - Keyboard-BIOS: Lance LT38C41 - ISA8: 1, ISA16: 3, ISA16/VL: 3
	// BIOS-String: 02/07/94-SIS-85C371-2C4I8C30-00 / 02/17/94
	ROM_SYSTEM_BIOS(1, "486sl", "486SL")
	ROMX_LOAD("486-sis_486sl.bin", 0x10000, 0x10000, CRC(7261263e) SHA1(d5c4ee484941bbb8ca756c5f6e53382748bbcfd6), ROM_BIOS(1))
	// 2: REV:1.2 - Chipset: SiS 85C471 P85C407 - CPU: Socket 3 - BIOS: AMI 486DX ISA BIOS AC03601316 (28pin) - Keyboard-BIOS: JETkey V5.0G - RAM: SIMM72x4, Cache: 8 sockets+1
	// ISA8: 1, ISA16: 4, ISA16/VL: 3
	ROM_SYSTEM_BIOS(2, "rev12", "REV.1.2") // no display
	ROMX_LOAD("486-sis_ac0360136.bin", 0x10000, 0x10000, CRC(940e3643) SHA1(f931f5c2b39ebb6c509033984ab050ffa1ff4334), ROM_BIOS(2))
ROM_END


/***** 486 motherboards using the UMC UM8498F, UM8496F chipset *****/

// Aquarius MD-4DUVC VER:2.1 / Aquarius MD-4DUV VER:2.1
ROM_START( md4duvc ) // "Memory test fail"
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: CPU: Socket 3/Overdrive - Chipset: UMC UM8496F, UM8498F - RAM: 2xSIMM72, 4xSIMM30, Cache: 9xW2457AX-15, sockets are 32pin
	// ISA16: 4, ISA16/VL: 3 - BIOS: 28/32pin
	ROM_SYSTEM_BIOS(0, "md4duvc", "MD-4DUVC") // BIOS-String: 01/12/94--2C4X6H01-00 / Release 01/04/95
	ROMX_LOAD( "md-4duvc.dmg", 0x10000, 0x10000, CRC(40d208bb) SHA1(c879599d2635c093fce420d1e7081631d27c621a), ROM_BIOS(0))
	// 1: Chipset: UMC UM8498F, UM8496F - BIOS/Version: 486SX/DX Award, R3.1,  I194220, 1994-1995 - CPU: Socket 3
	// RAM: 4xSIMM72, Cache: 9xW24257AK-15 - ISA16: 4, ISA16/VLB: 3
	ROM_SYSTEM_BIOS(1, "md4duv", "MD-4DUV") // BIOS-String: 11/1794-UMC-498GP-2C4X6A31-00 / MB-4DUV/UVC VER 3.1
	ROMX_LOAD( "atrom.bin", 0x10000, 0x10000, CRC(ecb764f5) SHA1(f34a7671e9efc6a6cd7ff1516c0c8ecbbfcd55e0), ROM_BIOS(1))
ROM_END

// BIOSTAR MB-1433UIV - Chipset: BIOTEQ 83C3498, 83C3496 - CPU: Socket 3 - RAM: 3xSIMM30, 4xSIMM72, Cache: 4x32pin, 4x28pin + TAG
// ISA16: 4, ISA16/VL: 3 - BIOS: 28pin
ROM_START( mb1433uiv )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0:
	ROM_SYSTEM_BIOS(0, "um8496fw", "UM8496FW")
	ROMX_LOAD( "um8496fw.ami", 0x10000, 0x10000, CRC(4a6dcc36) SHA1(f159f67eb662272244cd1781814ebcb5204a2625), ROM_BIOS(0))
	// 1:
	ROM_SYSTEM_BIOS(1, "w4", "W4")
	ROMX_LOAD( "um498-w4.amw", 0x10000, 0x10000, CRC(12fe6697) SHA1(2506dea874728916dc37f7dad8e8caf214a28525), ROM_BIOS(1))
ROM_END

// PC-Chips M912 - Chipset: UM8498F, UM8496F - CPU: 486 - BIOS: AMI - ISA16: 4, ISA16/VL: 3
ROM_START( pccm912 ) // no display
	ROM_REGION32_LE( 0x20000, "bios", 0)
	// 0:
	ROM_SYSTEM_BIOS( 0, "072594", "07/25/94")
	ROMX_LOAD( "m912.bin", 0x10000, 0x10000, CRC(7784aaf5) SHA1(f54935c5da12160251104d0273040fea22ccbc70), ROM_BIOS(0))
	// 1:
	ROM_SYSTEM_BIOS( 1, "120295", "12/02/95")
	ROMX_LOAD( "m912_12-02-1995x.bin", 0x10000, 0x10000, CRC(28a4a140) SHA1(a58989ab5ad5d040ad4f25888c5b7d77f31a4d82), ROM_BIOS(1))
ROM_END

// Pine Technology PT-430 - Chipset: UMC UM8498F UM8496F - BIOS: AMI 486DX ISA BIOS AB8906726 28pin - Keyboard-BIOS: silkscreen 8742, but socket empty
// BIOS-String: - ISA8: 1, ISA16: 3, ISA16/VL: 3 - OSC: 14.31818MHz
ROM_START( pt430 ) // no display
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "pt430.bin", 0x10000, 0x10000, CRC(d455c949) SHA1(c57c82ed015528f3d223f59c94ed6b8a9c323c39))
ROM_END

// PowerTech MB457 aka Pine PT-2068.1 - Chipset: UMC UM8498F & UMC UM8496F - CPU: Intel 80486DX4-100, solder pads for UMC U5 Green CPU
// RAM: 4xSIMM30, 1xSIMM72 - ISA8: 1, ISA16: 4 - BIOS/Version: Award 486DX J314549
ROM_START( ptmb457 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 10/19/94-UMC-498GP-2C4X6S21-00 / REV A
	ROM_SYSTEM_BIOS(0, "101994", "10/19/94")
	ROMX_LOAD( "umc-mb457-1.bin", 0x10000, 0x10000, CRC(71f66fec) SHA1(c798d68eb851fc93cc8a3dd67009b47388488e51), ROM_BIOS(0))
	// 1: BIOS-String: 02/07/95-UMC-498GP-2C4X6000-00 / PT-2068.1
	ROM_SYSTEM_BIOS( 1, "020795", "02/07/95")
	ROMX_LOAD( "umc-mb457-2.bin", 0x10000, 0x10000, CRC(2654aefd) SHA1(b888f4f891108a5ef268688840ff20be3a8e6aa5), ROM_BIOS(1))
ROM_END

// Soyo 025R2 - Chipset: UM8498F, UM8486F - CPU: Socket 3 - RAM: 4xSIMM30, 2xSIMM72, Cache: 4x28pin, 4x32pin, TAG, used: 4xUM61512AK-15, AE88128AK-15
// BIOS: 28pin - ISA16: 4, ISA16/VL: 3
// BIOS-String: 08/28/UMC-498GP-2C4X6S21-00 / REV B2
ROM_START( so025r2 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD("25r2-b2.bin", 0x10000, 0x10000, CRC(3b73360c) SHA1(eaaf47236154a9cc81ffda4c11086960aed0dadf))
ROM_END

// ID: ADI F4DXL-UC4 - Chipset: UM8498F, one unreadable - BIOS: AMI 486DX BIOS AB345213 - CPU: Socket 3 - RAM: 4xSIMM30, 2xSIMM72
// Cache: 4x32pin DIP, 5x28pin DIP (used: 9xCL63C256N-20) - OSC: FM14318 - ISA16: 4, ISA16/VLB: 3
ROM_START( f4dxluc4 ) // BIOS-String: 40-0E0-008060-00101111-12159-UM498-0 / ADI/UC4/V 1.0
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486_ab345213.bin", 0x10000, 0x10000, CRC(e5b85a92) SHA1(aade1fb1463b07a616c2594293bf0215c9652511))
ROM_END

ROM_START( um8498f ) // no display
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_DEFAULT_BIOS("498gp")
	// 0: BIOS-String: 40-P101-001437-00101111-072594-GREEN-H - CPU: Socket 3 - RAM: 4xSIMM30, 2xSIMM72, Cache: 9xUM61256AK-15
	// BIOS: AMI AB9300757
	ROM_SYSTEM_BIOS(0, "v14", "V1.4") // no display
	ROMX_LOAD( "4umm001.bin", 0x10000, 0x10000, CRC(a5b768b4) SHA1(904ce2814d6542b65acec0c84532946172f2296d), ROM_BIOS(0))
	// 1:
	ROM_SYSTEM_BIOS(1, "um849801", "UM8498 #1") // no display
	ROMX_LOAD( "um8498.ami", 0x10000, 0x10000, CRC(51f71bc7) SHA1(0986d60081d2c578a66789c0c53fe1d5919c553f),ROM_BIOS(1))
	// 2: Chipset: UM8498F + UM8496F - BIOS label: Award BIOS ISA 486 485427 - BIOS version: Award Modular BIOS v4.50G - CPU: UMC U55X 486-33F, solder pads for 80486socket
	// RAM: 4xSIMM30, 2xSIMM72 - ISA8: 1, ISA16: 5
	// BIOS-String: 12/08/94-UMC-498GP-2C4X6S21-00 / REV A1
	ROM_SYSTEM_BIOS(2, "498gp", "498GP")
	ROMX_LOAD( "award_bios_isa_486.bin", 0x10000, 0x10000, CRC(ce3ccaa4) SHA1(3fdc9282d9934e18e45b46b50644022fc0387f33), ROM_BIOS(2))
ROM_END


/***** 486 motherboards using the UM82C482A UM82C481A chipset *****/

// Elitegroup UM486/UM486sx Rev.1.4. - Chipset: UMC UM82C482A UM82C481A UM82C206F - ISA8: 2, ISA16: 6 - CPU: i486DX-50, FPU socket provided
// RAM: 8xSIMM30 in 2 banks, Cache: 8xW24256AK-25+4xCY7C164-20PC - OSC: 33.000MHz, 14.31818MHz - BIOS: AMI 486 BIOS ZZ364969 - Keyboard-BIOS: AMI KB-BIOS-VER-F
// BIOS-String: 40-0500-D01131-00101111-070791-UMCWB-0 / UM486/486SX V1.3 09-24-91
ROM_START( um486 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486-um486-um486sx.bin", 0x10000, 0x10000, CRC(0f20d193) SHA1(e9c7365b0343a815e5abc9726d128353becc18d3))
ROM_END

// Elitegroup UM486V-AIO - Chipset: UMC UM82C482AF, UM82C481BF, UM82C863F, UM82C865F, UM82C206F - ISA16: 4, ISA16/VL: 2
// BIOS: AMI - CPU: 486 - On board: Floppy, 1xIDE, parallel, 2x serial
// BIOS-String: 40-0100-001131-00101111-111192-UMC480-0 / UM100 V2.1 04-26-93
ROM_START( um486v )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "um486v.bin", 0x10000, 0x10000, CRC(eb52d3fd) SHA1(84f63904cfceca9171b5c469545068e19ae280a8))
ROM_END


/***** 486 motherboards using the UM8886BF, UM8881F chipset *****/

// A-Trend ATC-1415 - Chipset: UMC UM8881F/UM8886BF, OEC12C885, VT8235N - ISA8: 1, ISA16: 3, PCI: 3
// CPU: Socket 3 - BIOS: Award C 0247007 UMC8881/6 BIS VER : 3.20 95-12-08 / M271001 (128k)
// BIOS-String: 11/24/95-UMC-881/886A-2A4X5A2HC-00
ROM_START( atc1415 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "4umw002.bin", 0x00000, 0x20000, CRC(eacaa3de) SHA1(88c828def72aeae0798ba5a4a5c31cd465545c0b))
ROM_END

// Biostar MB8433UUD-A (4 SIMM, 2 IDE, 3 PCI, 4 ISA)
// UMC UM8881F, UM8886BF, UM8663AF; DS12887 RTC
ROM_START( mb8433uud ) // BootBlock BIOS
	ROM_REGION32_LE(0x20000, "bios", 0) // Intel Flash P28F010
	// 0: BIOS-String: 05/20/96-UMC-881E/886B-2A4X5B08C-00 / UUD960520S EVALUATION ROM - NOT FOR SALE
	ROM_SYSTEM_BIOS(0, "520s", "520S")
	ROMX_LOAD( "uud0520s.bin", 0x00000, 0x20000, CRC(0e347559) SHA1(060d3040b103dee051c5c2cfe8c53382acdfedad), ROM_BIOS(0))
	// 1: BIOS-String: 05/20/96-UMC-881E/886B-2A4X5B08C-00 / BIOSTAR MB-8433UUD v2014
	ROM_SYSTEM_BIOS(1, "2014", "2014")
	ROMX_LOAD( "uud2014.bin", 0x00000, 0x20000, CRC(315f7519) SHA1(e0174e4982d1861c64d871a7806b793a914f2366), ROM_BIOS(1))
	// 2: BIOS-String: 12/05/95-UMC-881E/886B-2A4X5B08C-00 / UUD951222S
	ROM_SYSTEM_BIOS(2, "8881d", "8881D")
	ROMX_LOAD( "um8881_d.awa", 0x00000, 0x20000, CRC(a47327b9) SHA1(6b57155cf34e2c597b7621e49ae8354c05cebfde), ROM_BIOS(2))
	// 3: BIOS-String: 11/07/95-UMC-881E/886B-2A4X5B08C-0 / UUD951108A
	ROM_SYSTEM_BIOS(3, "08a", "08A")
	ROMX_LOAD( "8433uud.awd", 0x00000, 0x20000, CRC(17ca5c2a) SHA1(7cda78f87f35fe8574a1f2d70d548cc2bc2af05b), ROM_BIOS(3))
ROM_END

// Gigabyte GA-486AM/S - CPU: Socket 3 - Chipset: UMC UM8886AF, UM8881F, UM88663AF, UM8667 - RAM: 4xSIMM72, Cache: 9x32pin, occupied: 4xUM6152AK-15, 1xUM61M256K-15
// BIOS: 32pin - OSC: 14.31818 - On board: 2xIDE, Floppy, par, 2xser - ISA16: 4, PCI: 3
ROM_START( ga486am ) // BootBlock
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "4am1107.bin", 0x00000, 0x20000, CRC(e836798d) SHA1(afd72654ed210bb19e37f9df96b9ecb75c84712a))
ROM_END

// PC-Chips M915i - CPU: 486 - Chipset: UM8881F, UM8886F - ISA16: 2, ISA16/VL: 2, PCI: 4 - On board: 2xIDE
ROM_START( pccm915i )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: no display
	ROM_SYSTEM_BIOS(0, "9151108s", "9151108S")
	ROMX_LOAD( "9151108s.rom", 0x00000, 0x20000, CRC(cba5525c) SHA1(9bdb586090f613a7172f6b46ed78e36331bf2135), ROM_BIOS(0))
	// 1: Amptron DX9200 aka PcChips M915i with Fake cache - Chipset: UMC UM8881F + UM8886BF, SECKS82C6818A - EPROM: AMD AM27C010
	// BIOS label: AMI 486DX ISA BIOS 1993 AC9051796 - BIOS Version: AMI 10/10/1995: 11-08-1995 - on board: 2xIDE
	// CPU: Socket 3 - RAM: 4xSIMM72, Cache: 9xblack blocks of plastic - OSC: 14.31818 - ISA16: 2, ISA16/VLB: 2, PCI: 4
	ROM_SYSTEM_BIOS(1, "dx9200", "Amptron DX9200") // BIOS String: 41-p400-001437-00101111-101094-486AVIP-H
	ROMX_LOAD( "m915_fake_cache.bin", 0x00000, 0x20000, CRC(82a4e810) SHA1(b20a6e128d6298adf8487d190dd182a751dfccf9), ROM_BIOS(1))
ROM_END

// PC-Chips M919 - this motherboard showcased the issues that gave PC-Chips its bad name, it was available with fake cache, a proprietary cache socket or with fully operational cache
// Chipset: UMC UM8881F/9714-EYS and UM8886BF/9652-FXS (V3.4B/F), UMC UM8886BF/9618-FXS and UM8881F/9622-EYS (Rev. 1.5)
// http://th2chips.freeservers.com/m919/ this mentions that the BIOS requires a flashable chip
ROM_START( pccm919 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "m919v1", "PC-Chips M919 v1")
	ROMX_LOAD( "9190914s.rom", 0x00000, 0x20000, CRC(bb18ff2d) SHA1(530d13df21f2d483ec0dddda44fb4fe7e29ec040), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "m919v2", "PC-Chips M919 v2")
	ROMX_LOAD( "9191016s.rom", 0x00000, 0x20000, CRC(2a2125a6) SHA1(753061ae6f80c0ca42d1af91aada657910feae18), ROM_BIOS(1))
ROM_END

// Shuttle HOT-433 - Chipset: UM8886BF, UM8881F, UM8669F, ??667
// CPU: Cyrix 5x86-120GP - ISA16: 4, PCI: 4 - On board: PS2-Mouse, 2xser, Floppy, par, 2xIDE
// Versions 1-3 can use Flash or EPROM, Version 4 only EPROM
ROM_START( hot433 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 07/25/95-UMC-881/886A-2A4X5H21-00 / (433WIE10) UMC880A For486PCI Green_PC
	ROM_SYSTEM_BIOS(0, "wie10", "WIE10")
	ROMX_LOAD( "433wie10.bin", 0x00000, 0x20000, CRC(90604ef4) SHA1(61e160678d48cb5752c84090ca990e09382ae01d), ROM_BIOS(0))
	// 1: BIOS: 10/10/94 AMI (or 02/02/95 depending on where you look), 486PCI/ISA 057890 in a Winbond W29EE011-15 - no display
	ROM_SYSTEM_BIOS(1, "v401", "V4.0 #1")
	ROMX_LOAD( "hot433.bin", 0x00000, 0x20000, CRC(1c279c6f) SHA1(4a0e99fafc5719959fb5800a61629c3f36778240), ROM_BIOS(1))
	// 2: Original AMI BIOS for rev 1-3 w/mouse support
	ROM_SYSTEM_BIOS(2, "aip16", "AIP16")
	ROMX_LOAD( "433aip16.rom", 0x00000, 0x20000, CRC(a9503fc6) SHA1(0ebd936f5478477e37528e6e487c567b064248f7), ROM_BIOS(2))
	// 3: AMI BIOS for the EPROM Programmer, not flashable
	ROM_SYSTEM_BIOS(3, "aue2a", "AUE2A")
	ROMX_LOAD( "433aue2a.rom", 0x00000, 0x20000, CRC(35f5633f) SHA1(01148eba919985165ab9cd12b5e6f509d6d1385f), ROM_BIOS(3))
	// 4: AMI BIOS for the EPROM Programmer, not flashable
	ROM_SYSTEM_BIOS(4, "aue33", "AUE33")
	ROMX_LOAD( "433aue33.rom", 0x00000, 0x20000, CRC(803c4b1e) SHA1(483c799c08eed0d446384d67e9d23341499806b1), ROM_BIOS(4))
	// 5: AMI BIOS for rev 1-3.  Some reports say for rev4
	ROM_SYSTEM_BIOS(5, "aus2a", "AUS2A")
	ROMX_LOAD( "433aus2a.rom", 0x00000, 0x20000, CRC(766d1f3f) SHA1(1e59140bc91ab98fcadcf7bb77e222932696419f), ROM_BIOS(5))
	// 6: Latest AMI BIOS for rev 1-3
	ROM_SYSTEM_BIOS(6, "aus2c", "AUS2C")
	ROMX_LOAD( "433aus2c.rom", 0x00000, 0x20000, CRC(bdc65766) SHA1(e87cc4aed14ae7fcdf6423063b0ababe65b41044), ROM_BIOS(6))
	// 7: AMI Bios for rev 1-3 w/mouse support
	ROM_SYSTEM_BIOS(7, "aus26", "AUS26")
	ROMX_LOAD( "433aus36.rom", 0x00000, 0x20000, CRC(8f864716) SHA1(0bf4b8114cbb406646d89eed7833556611e1fbe6), ROM_BIOS(7))
	// 8: Latest AMI BIOS for rev4 of the Shuttle HOT-433 motherboard.
	ROM_SYSTEM_BIOS(8, "aus33", "AUS33")
	ROMX_LOAD( "433aus33.rom", 0x00000, 0x20000, CRC(278c9cc2) SHA1(ecd348106d5118eb1e1a8c6bd25c1a4bf322f3e6), ROM_BIOS(8))
	// 9: boots to BootBlock BIOS
	ROM_SYSTEM_BIOS(9, "2a4x5h21", "2A4X5H21")
	ROMX_LOAD( "2a4x5h21.bin", 0x00000, 0x20000, CRC(27c47b90) SHA1(09d17bc5edcd02a0ff4a3a7e9f1072202880251a), ROM_BIOS(9))
	// 10: no display
	ROM_SYSTEM_BIOS(10, "shuttle", "Shuttle HOT-433")
	ROMX_LOAD("bios.bin", 0x00000, 0x20000, CRC(a5ffafac) SHA1(0ec71a82c80a96a5724ba375934c709db468dabc), ROM_BIOS(10))
ROM_END

// PROTECH PM486PU-S7 - Chipset: UMC 881/886A (UM8881F/UM8886AF), SMC FDC, Winbond
// BIOS Chip: TI/TMS 27C010A (128K) - CPU: i486DX-33 - On board: 2xIDE, FDC, 2xser, par - RAM: 2xSIMM72, Cache: 4xGLT721208-15 +1 TAG - ISA16: 4, PCI: 3
// BIOS-String: 10/11/95-UMC-881/886A-2A4X5P62-00 / (PM486PU-S7) 486 WITH I/O PCI LOCAL BUS SYSTEM ...
ROM_START( pm486pu )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "pm486pu-s7-2a4x5p62-00.bin", 0x00000, 0x20000, CRC(143bdc07) SHA1(e2cf2ac61fd3e4797e5d737dfec4a2b214469190))
ROM_END

// Pine PT-432b - Chipset: UMC UM8886BF, UM8881F, UM8663F, UM8287, UM8667 - CPU: Am486DX4-100 - RAM: 4xSIMM72, Cache: 8+1 sockets
// ISA16: 4, PCI: 3 - BIOS: AMI 486PCI ISA BIOS AA0841149 (32pin) - On board: 2xser, par, 2xIDE, Floppy
ROM_START( pt432b ) // no display
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "sr_m401-a.bin", 0x00000, 0x20000, CRC(ff8cd351) SHA1(a9c6a54f38b1b548fba4d7d42643f117441b09a6))
ROM_END

// TD-4IP-UMC-AIO - CPU: Socket 3 - Chipset: UMC UM8881F, UM8886BF, UM8663BF, UM8667, bq3285P - RAM: 2xSIMM72, Cache: W24257AK-15, W24512AK-20 (4)
// BIOS: Award 020175447 / 29EE010 (128k) - BIOS-String: 12/19/95-UMC-881E/886B-2A4X5CF9C-00 - ISA16: 4, PCI: 3
ROM_START( td4ipaio )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "4ummw001.bin", 0x00000, 0x20000, CRC(4e716d77) SHA1(1b7af88c4da5ca388acbc0cb66bd26a0ae8f4931))
ROM_END

ROM_START( um8886 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: no display - UMC PCI 95C-0123 - Chipset: UMC UM8886AF, UM8881F, 4xUM8002, UM8663AF, UM8667 - CPU: Socket 3 - On board: 2xser, par, Floppy, 2xIDE - 4xISA16, 4xPCI
	// BIOS: AMI 486 PCI ISA in M27C1001 EPROM
	ROM_SYSTEM_BIOS( 0, "pci95c", "PCI 95C-0123")
	ROMX_LOAD( "486-umc_pci_95c-0123.bin", 0x00000, 0x20000, CRC(9db58de4) SHA1(5441f3181fb26911d796c4bf019136aa8e4c060b), ROM_BIOS(0))
	// 1: no display - V1.1A - Chipset: UMC UM8886AF UM8881F, UM8667, UM8663AF - CPU: i486DX2-66 - On board: 2xser, 2xIDE, Floppy, par - BIOS: AMI 486DX ISA BIOS AC6288199 - ISA16: 4, PCI: 3
	ROM_SYSTEM_BIOS( 1, "pcimini", "PCI mini")
	ROMX_LOAD( "486-umc-pci mini.bin", 0x00000, 0x20000, CRC(4ee12b46) SHA1(9397f67b21f11cfda57abd5ab28f93055909ee97), ROM_BIOS(1))
ROM_END

// Elitegroup UM8810 PAIO - Chipset: UMC UM8881F, UM8886F, UM82C865FSMC FDC37C665GT, PCI0640B - CPU: Socket 3 - RAM: 4xSIMM72, Cache: 32pin sockets, 9xUM61256FK-15 occupied
// BIOS: 32pin - Keyboard-BIOS: AMIKEY-2 - ISA16: 4, PCI: 3 - On board: 2xIDE, Floppy, par, 2xser
ROM_START( um8810paio )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 09/27/94-UMC-881/886-2A4X5E11-00 / UM8810P-AIO V1.0 09-27-94'
	ROM_SYSTEM_BIOS(0, "awd", "AWD")
	ROMX_LOAD( "awd_09-27-94.bin", 0x00000, 0x20000, CRC(25c671ba) SHA1(6b7641fb0f619e233d956e7cdb4e246f3c525673), ROM_BIOS(0))
	// 1: BootBlock BIOS
	ROM_SYSTEM_BIOS(1, "aw123", "AW_1-23")
	ROMX_LOAD( "aw_1-23.bin", 0x00000, 0x20000, CRC(420c3a43) SHA1(e0744b0bf171d279650d1f3a731e231562fe60ae), ROM_BIOS(1))
	// 2: blank screen, possibly for update only
	ROM_SYSTEM_BIOS(2, "v21", "V21")
	ROMX_LOAD( "881_v21_01-27-95.rom", 0x00000, 0x20000, CRC(2443759b) SHA1(02a9e51b51c2d2d21d5ae821b6d8aa97f62c3327), ROM_BIOS(2))
	// 3: blank screen
	ROM_SYSTEM_BIOS(3, "21e", "21E")
	ROMX_LOAD( "881avio_01-08-96.21e", 0x00000, 0x20000, CRC(446cd0c3) SHA1(32a61c0578326d17840db37855644543b6406535), ROM_BIOS(3))
	// 4: blank screen
	ROM_SYSTEM_BIOS(4, "31e", "31E")
	ROMX_LOAD( "881avio_01-08-96.31e", 0x00000, 0x20000, CRC(b5ec7dca) SHA1(390f79f50de986c3874464202251398baa87f4b5), ROM_BIOS(4))
	// 5: blank screen
	ROM_SYSTEM_BIOS(5, "32h", "32H")
	ROMX_LOAD( "8810aio_06-27-96.32h", 0x00000, 0x20000, CRC(b7e9d590) SHA1(bfdfa64bc87f2eb0d362e87755dce70584675427), ROM_BIOS(5))
	// 6: blank screen, possibly for update only
	ROM_SYSTEM_BIOS(6, "32j", "32J")
	ROMX_LOAD( "8810aio_12-11-96.32j", 0x00000, 0x20000, CRC(c6baec8b) SHA1(79bccaefb75e2df071d6edd191eea74aa82dc620), ROM_BIOS(6))
	// 7: blank screen
	ROM_SYSTEM_BIOS(7, "32f", "32F")
	ROMX_LOAD( "881032f.rom", 0x00000, 0x20000, CRC(5c337ae6) SHA1(29bc48a3d5165afa79612baf6deb9c04c7e55b32), ROM_BIOS(7))
	// 8: blank screen
	ROM_SYSTEM_BIOS(8, "v145", "V145")
	ROMX_LOAD( "8810aio_v145_09-21-94.bin", 0x00000, 0x20000, CRC(a253c017) SHA1(8273defe95a13ea0a260d4a410d601f82a947ad9), ROM_BIOS(8))
	// 9: 06/23/95-UMC-881/886-2A4X5E11-00 / Version VBS1.10H UM8810 ECS (Highscreen boot logo)
	ROM_SYSTEM_BIOS(9, "v110h", "V1.10H")
	ROMX_LOAD( "vbs1.10h.bin", 0x00000, 0x20000, CRC(1bf29727) SHA1(77ccd34110ec0387cdcfa260332b403d0c197d17), ROM_BIOS(9))
ROM_END


/***** 486 motherboards using the UMC UM82C491F UM82C493F or clones (BIOTEQ) chipset */

// Chicony CH-491E Rev. 1.4 - Chipset: UMC UM82C491F UM82C493F - BIOS: 04/04/93 AMI AB1987679 28pin - Keyboard-BIOS: AMIKEY
// BIOS-String: 40-0102-001116-00101111-040493-UMC491F-0 / UMC 491 for 80486 AUTO - ISA16: 4, ISA16/VL: 3
ROM_START( ch491e )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "ch491e.bin", 0x10000, 0x10000, CRC(2d24ff24) SHA1(72f35c19e907c6d0a03a49bd362c4f57cc89da1c))
ROM_END

// Aquarius System (ASI) MB-4D33/50NR VER:01 - Chipset: UMC UM82C491F UM82C493F - CPU: AM486DX2-66 - BIOS: Award 1060176, 28pin - Keyboard-BIOS: JETkey V5.0
// BIOS-String: 03/09/94-UMC-491-2C4X2A30-00 / MB-4D33/50NR-02 - ISA8: 1, ISA16: 3, ISA16/VL: 3
ROM_START( mb4d33 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486-mb-4d33.bin", 0x10000, 0x10000, CRC(f1299131) SHA1(d8e2749e180135e23483e36a0a05479e64f23d8c))
ROM_END

	// BIOS-String: 40-0100-001281-00101111-080893-UMC491F-0 / HL3SC/SM AMI BIOS VER 4.9 01/09/'95
ROM_START( 4dmuhl3s )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "3s-49-1.rom", 0x10000, 0x10000, CRC(78a8ea39) SHA1(35e2d0103da28b93a8c1addb1f083bd6e4239d2a))
ROM_END

// Elitegroup ECS UC4915 A AIO - Chipset: UMC UM82C491F UM82C493F UM82C865F SMC FDC37C662QF P,  PROCHIP PR 4030 - CPU: Socket 3
// BIOS: AMI 486DX ISA BIOS AB2683223 28pin in 32pin socket - Keyboard-BIOS: Intel/AMI MEGA-KB-H-WP
// BIOS-String: 40-0401-001131-00101111-040493-UMC491C-0 / VOBIS UC4915-A V1.1 11-05-93' - ISA16: 4, ISA16/VL: 2 - OSC: 14.31818 - On board: IDE, Floppy, 2xser, par, Game
ROM_START( ec4915aio )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486-ecs-uc4915-a-aio.bin", 0x10000, 0x10000, CRC(5b3429a3) SHA1(a1b3ddb6a0939d20ae66e034914ea94648ca7149))
ROM_END

//  Elitegroup UC4913 REV:1.1 (Peacock sticker)- Chipset: UMC UM 82491F 82C493F - CPU: 486 - BIOS: AMI 486DX ISA BIOS AA8960448 (28pin) - Keyboard-BIOS: AMI/Intel
// OSC: 14.31818 - RAM: SIMM30x8, Cache: 9 sockets, 4 (UM61256CK-20)+1 (MS6264A-20NC) filled - ISA8: 2, ISA16: 3, ISA16/VL: 3
// BIOS-String: 40-0401-001131-00101111-040493-UMC491C-0 / Peacock AG UC4913 BIOS Ver. 1.0 01.09.93
ROM_START( ec4913 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486-peacock-uc4913-aa8960338.bin", 0x10000, 0x10000, CRC(58e6753c) SHA1(077c11532572ca0399f76a7ba2d31b8c1ca75d48))
ROM_END

// Biostar MB-1433UCV - Chipset: BIOTEQ 82C3491, 82C3493 (check mb133340 for a 386 motherboard using the same chipset)
// CPU: 486DX2-66 - RAM: 8xSIMM30, Cache: 8+1x28pin(AS57C256-20PC) - ISA8: 1, ISA16: 3, ISA16+VL: 3 - BIOS: AMI AB0975913 - Keyboard-BIOS: JETkey V5.0 - RTC: TH6887A 9410
ROM_START( mb1433ucv )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 40-0100-001223-00101111-040493-UMC491F-0 / MB-1333/40UCG-A, MB-1333/40UCQ-B / MB-1433-40UDV-A, MB-1433/50UCV-C, MB-6433/50UPC-A for EXT. RTC
	ROM_SYSTEM_BIOS(0, "ucvd", "UCV-D")
	ROMX_LOAD( "biostar_bios_mb-1433-50ucv-d_pcb_ver_2.bin", 0x10000, 0x10000, CRC(e5ff2d76) SHA1(d2abe00eb2051ec7cb9423cdb8b52e91f7e2d416), ROM_BIOS(0))
	// 1: BIOS-String: 40-0100-001223-00101111-04093-UMC491f-0 / UCV-G
	ROM_SYSTEM_BIOS(1, "ucvg", "UCV-G")
	ROMX_LOAD( "bioteq.rom", 0x10000, 0x10000, CRC(93321e89) SHA1(450e35787607a4b6aecd3159d6c0599a03cd42b1), ROM_BIOS(1))
ROM_END

// Mitac PWA-IH4077C - Chipset: UMC UM82C491F + UM82c493F - CPU: Socket 3, solder pads for PQFP CPU - RAM: 4xSIMM30, Cache: 9xUM61256AK-15 - OSC: 14.31818
ROM_START( pwaih4077c )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS String: 04/13/94-UMC-491-2C4X2m31-00 - UMC491F 80486 BIOS...1.01.00 (3371-3372) - BIOS label: IH4077CN BIOS R1.01.00
	// BIOS Version: Award 4.50G 04/13/94 - Keyboard BIOS: VIA VT82C42N
	ROM_SYSTEM_BIOS(0, "012694", "01/26/94")
	ROMX_LOAD( "ih4077c-1.00.00.bin", 0x10000, 0x10000, CRC(e75cca73) SHA1(4f27d16f4f8fce9d3410821ec62780f7df669776), ROM_BIOS(0))
	// 1: BIOS-String: 04/13/94-UMC-491-2C4X2m31-00 - UMC491F 80486 BIOS...1.01.00 (3371-3372)
	ROM_SYSTEM_BIOS(1, "041394", "04/13/94")
	ROMX_LOAD( "4077c101.bio", 0x10000, 0x10000, CRC(b6a27c48) SHA1(18ee3b2fc4897cbaafc0e0298938ba58a3a7f84c), ROM_BIOS(1))
ROM_END


/***** motherboards using the Unichip U4800 chipset *****/

// Gemlight GMB-486UNP v2.1 - Chipset: Unichip U4800-VLX, SIS85C206 - CPU: solder pads for 486FQFP, i486DX2-66 - RAM: 8xSIMM30, Cache: 8xUM61256AK-15
// BIOS: AMI 486DX ISA BIOS - ISA16: 4, ISA16/VLB: 3
ROM_START( gmb486unp ) // BIOS String: 40-0405-428036-00101111-080893-U4800-VLX-H / screen remains blank
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "gmb-486unp.bin", 0x10000, 0x10000, CRC(17d770c7) SHA1(8655610ceaf7bd9c17d7c0a550805ae55f128660))
ROM_END

// UNICHIP 486 WB 4407 REV 1.0 - Chipset: KS83C206Q UNICHIP U4800-VLX - BIOS: AMI 486 ISA BIOS AA6562949, 28pin - Keyboard-BIOS: AMI 2050778
// BIOS-String: 40-0200-001107-0010111-111192-U4800VLX-0 / 4407 UNICHIP BIOS VER 1.0 - OSC: 14.31818 - ISA16: 4, ISA16/VL: 3
ROM_START( uniwb4407 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "unichip_486_wb_4407.bin", 0x10000, 0x10000, CRC(91237686) SHA1(7db14451cc3e00a2273a453152a817bccbdfb10e))
ROM_END

ROM_START( uni4800 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// BIOS-String: 40-05TB-006257-00101111-060692-UNI4800-0
	ROM_LOAD( "uni4800.ami", 0x10000, 0x10000, CRC(555c4c34) SHA1(0a907d7a0ec16e9369d77a1ac1afb0e69e3d6c1a))
ROM_END


/***** 486 motherboards using the VIA VT82C495 VT82C481 chipset *****/

// FIC 4386-VC-V - CPU: 486 - Chipset: VIA VT82C495 VT82C481 - ISA8: 2, ISA16: 3, ISA16/VL: 2 - OSC: 33.333MHz - BIOS: AMI 486DX ISA BIOS AA6387315 (28pin) -
// BIOS-String: X0-0100-001121-00101111-021993-VIA-0 / Version 1.02 - Keyboard-BIOS: Lance LT38C41
ROM_START( fic4386vcv )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486-4386-vc.bin", 0x10000, 0x10000, CRC(659210c2) SHA1(a730a547f3af215459632160fa670fde7e9c4f9a))
ROM_END

// HIGHSCREEN 486 Universal Board C82C33-A VIA4386-VIO - Chipset: VIA VT82C495 VT82C481, Winbond WB3757F - CPU: AM486DX2-66
// BIOS: Award F0599630 - Keyboard BIOS: AMI 1131 KEYBOARD BIOS PLUS - BIOS-String: Award Modular BIOS v4.20 / Version 1.143K
// On board: IDE, Floppy, 2xser, par, game - OSC: 32.0000MHz - ISA16: 6
ROM_START( via4386vio ) // probably a FIC board - KEYBOARD ERROR OR NO KEYBOARD PRESENT
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486-highscreen.bin", 0x10000, 0x10000, CRC(059b6e51) SHA1(f8ede823e41cfa6f72bd9717ec75419079f9c40b))
ROM_END

// FIC 4386-VC-HD - Chipset: VIA VT82C481, VT82C495 - this board can take either 386 or 486 CPUs
// Keyboard-BIOS: Lance LT38C41 - CPU: AMD AMD386DX-40, FPU: IIT 3C87-40 - ISA16: 6
ROM_START( fic4386vchd )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS: AMI; Version 1.04; 06/06/92 - BIOS-String: X0-0100-001121-00101111-021993-VIA-0 / Version 1.04
	ROM_SYSTEM_BIOS(0, "ami104", "AMI V1.04")
	ROMX_LOAD( "3vim001.bin", 0x10000, 0x10000, CRC(668d8cab) SHA1(409b81e33ca07b0a9724dbb6ca395a3a0887aa02), ROM_BIOS(0))
	// 1: BIOS: Award F0111730 v1.15K 03/12/93-VENUS-VIA - BIOS-String: Award Modular BIOS v4.20 / Version 1.15K
	ROM_SYSTEM_BIOS(1, "awav115k", "Award V1.15k") // KEYBOARD ERROR OR NO KEYBOARD PRESENT
	ROMX_LOAD( "4386-vc-hd v1.15k.bin", 0x10000, 0x10000, CRC(acc5db45) SHA1(cb93322735e96614d3c54fbfcd4291ff1b3ca57c), ROM_BIOS(1))
	// 2: AWARD v4.20 F0166061 (28pin) - Keyboard-BIOS: Lance LT38C41 - CPU: 486 - BIOS-String
	ROM_SYSTEM_BIOS(2, "awav110k", "Award V1.10K") // KEYBOARD ERROR OR NO KEYBOARD PRESENT
	ROMX_LOAD("486-4386-vc-hd.bin", 0x10000, 0x10000, CRC(a32d30fc) SHA1(815a63e624b3145d9955aa3ce8c4c1e34fb438bb), ROM_BIOS(2))
	// 3: ID: Peacock 4386-VCHD - Chipset: VIA VT82C481, VT82C495 - CPU: AMD Am386DX-40, socket for 486 provided
	// RAM: 8xSIMM30, Cache: 10x28pin sockets - BIOS: Award 386DX F0121091 PEA 2_0 - ISA16: 6 - OSC: 80.0000MHz
	ROM_SYSTEM_BIOS(3, "pea20", "Pea 2_0") // BIOS-String: Award Modular BIOS V4.20 / 4386 BIOS Ver. 2.0 01.04.93 - KEYBOARD ERROR OR NO KEYBOARD PRESENT
	ROMX_LOAD("386-peacock-4386-vchd.bin", 0x10000, 0x10000, CRC(1cd08629) SHA1(9a2b359ade2e93ab1d164e3e4f2cb9e8604cd43d), ROM_BIOS(3))
ROM_END

// FIC 486-VC-HD - BIOS Version: AMI 05/05/1991 - Chipset: VIA VT82C495 VT82C481, DS1287/1187 - EPROM Label: 486EB
// Keyboard BIOS: Lance LT38C41 - CPU: Intel 80486DX-33, solder pads for 80486 - RAM: 8xSIMM30, Cache: 10x28pin DIP (4xKM68257BP-25, 1xMCM6206CP fitted)
// OSC: 33.333MHz - ISA16: 6
ROM_START( fic486vchd ) // BIOS ID String: 40-04C1-ZZ1124-00101111-050591-ET/486H-0
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "486-vc-hd_doc11670.bin", 0x10000, 0x10000, CRC(607ebe18) SHA1(870080bb49bad42fb4433f9208c17ad1c7ee437d))
ROM_END


/***** 486 motherboards using the VIA VT82C505 + VT82C496G + VT82C406MV chipset *****/

// FIC 486-PIO-2 (4 ISA, 4 PCI)
// VIA VT82C505 (ISA/VL to PCI bridge) + VT82C496G (system chipset) + VT82C406MV (keyboard controller, RTC, CMOS), NS311/312 or NS332 I/O
ROM_START( ficpio2 )
	ROM_REGION32_LE(0x40000, "isa", ROMREGION_ERASEFF)
	ROM_DEFAULT_BIOS("ficpio2c1")
	// 0
	ROM_SYSTEM_BIOS(0, "ficpio2c7", "FIC 486-PIO-2 1.15C701") /* pnp, i/o core: NS 332, doesn't boot, requires cache emulation? */
	ROMX_LOAD( "115c701.awd",  0x020000, 0x20000, CRC(b0dd7975) SHA1(bfde13b0fbd141bc945d37d92faca9f4f59b716d), ROM_BIOS(0))
	// 1
	ROM_SYSTEM_BIOS(1, "ficpio2b7", "FIC 486-PIO-2 1.15B701") /* pnp, i/o core: NS 311/312, doesn't boot, requires cache emulation? */
	ROMX_LOAD( "115b701.awd",  0x020000, 0x20000, CRC(ac24abad) SHA1(01174d84ed32fb1d95cd632d09f773acb8666c83), ROM_BIOS(1))
	// 2: BIOS-String: 04/18/96-VT496G-2A4LF0IC-00 / Version 1.15C101
	ROM_SYSTEM_BIOS(2, "ficpio2c1", "FIC 486-PIO-2 1.15C101") /* non-pnp, i/o core: NS 332, working  */
	ROMX_LOAD( "115c101.awd",  0x020000, 0x20000, CRC(5fadde88) SHA1(eff79692c1ecf34b6ea3f02409d14ce1f5c51bf9), ROM_BIOS(2))
	// 3: BIOS-String: 04/18/96-VT496G-2A4LF0IC-00 / Version 1.15B101
	ROM_SYSTEM_BIOS(3, "ficpio2b1", "FIC 486-PIO-2 1.15B101") /* non-pnp, i/o core: NS 311/312, working  */
	ROMX_LOAD( "115b101.awd",  0x020000, 0x20000, CRC(ff69617d) SHA1(ecbfc7315dcf6bd3e5b59e3ae9258759f64fe7a0), ROM_BIOS(3))
	// 4: no display - CPU: Socket3 - On board: 2xser, par, 2xIDE, Floppy, par - BIOS: Award F4215801, 32pin - ISA16: 4, PCI: 4
	ROM_SYSTEM_BIOS(4, "ficpio2", "FIC 486-PIO-2 DOC 14580")
	ROMX_LOAD( "486-pio2.bin", 0x20000, 0x20000, CRC(4609945d) SHA1(7ad446bc3b27f3f636fb5884e58b055681f081eb), ROM_BIOS(4))
ROM_END

// FIC 486-VIP-IO2 (3 ISA, 4 PCI)
// VIA VT82C505 + VT82C496G + VT82C406MV
// Boot block - BIOS-String: 05/28/96-VT496G-2A4L6F0IC-00 / 1.164G701
ROM_START( ficvipio2 ) // switching this to ficpio2 results in the Boot block BIOS no longer being shown
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "1164g701.awd", 0x00000, 0x20000, CRC(7b762683) SHA1(84debce7239c8b1978246688ae538f7c4f519d13))
ROM_END


/***************************************************************************
  80486 Desktop
***************************************************************************/

// NCR Class 3433 - CPU: 486SX/DX 25/33, coprocessor socket provided - Chipset: NCR WPD CLEMSON 006-2001325 CQO1842 9209N
// LSI LOGIC L1A5840 006-2000654B NAR 9212Delta WG35494 GERMANY, NCR 006-2001895 WPD FALCON E CQO 2291 9218N,
// WD58C65JM, VLSI 9210AV 211411 VGA8203C4570 NCR PB 006-2001329, Dallas DS1387
// OSC: 16.000000MHz, 1.8432MHz,  65.00000MHz, 25.175/26.322, 36.000000MHz, 50.000000MHz, 14.31818MHz, 66.66600MHz
// on board VGA: NCR 77C22 VGA-2 609-3400639 CQO1570 9210R, 8xMCM514245AJ70
// Slots: 4xMCA, 3x32-bit external memory card, 1xSCSI host adapter - ports: PS/2 mouse, keyboard, 2xserial, parallel, floppy
ROM_START( ncr3433 )
	ROM_REGION32_LE( 0x20000, "bios", 0 )
	// 0: NCR ROM BIOS Version 82.01.00 (3433, 3434, 3432, 3437)
	ROM_SYSTEM_BIOS(0, "82.01.00", "82.01.00")
	ROMX_LOAD( "rbios206.bin", 0x00000, 0x20000, CRC(06e77547) SHA1(41c517b284ed6335024c2b8398504cf3eb8a09e1), ROM_BIOS(0) )
	// 1: NCR ROM BIOS Version 83.01.00 (3433, 3434, 3432, 3437)
	ROM_SYSTEM_BIOS(1, "83.01.00", "83.01.00")
	ROMX_LOAD( "rbios1.bin", 0x00000, 0x20000, CRC(cf793ce9) SHA1(69d531d14a86bdada8940ba2466515ebcd870d0d), ROM_BIOS(1) )

	ROM_REGION( 0x0800, "keyboard", 0 )
	ROM_LOAD( "i8742_150-0008390_vers_3.1.bin", 0x000, 0x800, CRC(1bf17f29) SHA1(031ea1bb40756e3e5a1f01b01a53cc3a7c074338) )
ROM_END

// Amstrad PC9486 - Board Type - UX486VIO-A Rev. 1.0 - Chipset: UMC - UM82C481BF/UM82C482AF/UM82C206F - BIOS/Version: AMI 486DX ISA BIOS 1993 - AA9222968
// BIOS String: 40-0100-001131-00111111-111192-UMC480-0 / Amstrad PC9486 - CPU: 80486sx-25 in Socket 3, solder pads for 486 CPU - RAM: 4xSIMM30, 1xSIMM72, Cache: 64K/128K256K
// on board VGA: Cirrus Logic CL-GD5426-80QC-A, 2xKM416C256AJ-7 - Jumpers: Parallel out/bidirectional, VGA enable/disable, SIMM type, PQFP or socket,
// 20/25/33/40/50 MHz, Parity enable/disable, -  CPU type - on board: VGA, Floppy, IDE - slots: 1, for riser card
ROM_START( pc9486 )
	ROM_REGION32_LE( 0x20000, "bios", 0 )
	ROM_LOAD ( "9486_saverom.bin", 0x00000, 0x20000, CRC(cbc35a4e) SHA1(dfa614c8255a1407c9850fa4ff99a6b2a52e1a4f) )
ROM_END

/**************************************************************************
  80486 Laptop/Notebook
**************************************************************************/

// Highscreen Colani Blue Note
// Chipset: Cirrus Logic CL-GD6235-65QC-A, CL-PD6720-QC-A ETEQ ET6000 A, Appian ADI2, SMC FDC37C651 QF P, DIA UA0300-QA, Chips F82C206J
// Video: Dual Scan Color LCD (monochrome available) - RAM: 4MB on board, RAM expansion board - on board: trackball, ser, par, PS2, VGA
// mass storage: Floppy, IDE - expansion: 2xPCMCIA, expansion bus
ROM_START( bluenote )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "7500d_rev26_121593.bin", 0x00000, 0x20000, CRC(d564f855) SHA1(181e4097c3b4ca2e8e79f1732d4aef9edd5b4586))
ROM_END

// Lion 3500C/T
// Info: BIOS saved according to http://mess.redump.net/dumping/dump_bios_using_debug from a 3560C machine
// Form factor: notebook
// CPU: Intel 486DX2-66
// RAM: 2MB, 4MB, 8MB or 16MB
// Chipset: ETEQ ET/486H (ET82C491 & ET82C492), 82C206, 82C712
// ROM: 128K Video (E0000-EFFFF) & BIOS ROM  (F0000-FFFFF)
// Video: Cirrus Logic GD-6420BF/6430 6342 internal VGA, 640x480 256 color display
// Mass storage: Floppy 3.5" 1.44MB, 3.5" HDD, 120MB
// Input: Trackball connected as a PS/2 mouse
// Options: 100 pin expansion port for 3305 Docking station (2xISA16 slots), external keypad
// Ports: External VGA, external keyboard, COM1, external keypad, COM2, LPT1, buzzer
// Variants: T denotes an active 8.4" display, C a passive 9.5" color display. 3560T/C (486DX2-66), 3530T/C(486DX2-50), 3500T/C (486DX-33), 3500SXT/SXC(486SX-25)
ROM_START( lion3500 )
	ROM_REGION32_LE( 0x20000, "bios", 0 )
	ROM_LOAD( "lion3500.bin", 0x00000, 0x20000, CRC(fc409ac3) SHA1(9a7aa08b981dedffff31fda5d3496469ae2ec3a5) )
ROM_END

// Siemens-Nixdorf PCD-4NL 486 subnotebook
// PhoenixBIOS(TM) A486 Version 1.03
// complains about "Pointer device failure" and "Memory failure at 00100000, read AA55 expecting 002C
ROM_START( pcd4nl )
	ROM_REGION32_LE( 0x20000, "bios", 0 )
	ROM_LOAD( "pcd4nl.bin", 0x00000, 0x20000, CRC(8adb4900) SHA1(a01c665fed769ff815bc2e5ae30901f7e12d721b) )
ROM_END

// Siemens-Nixdorf PCD-4ND 486 notebook - display remains blank
// Graphics chip: WDC WD90C24A-ZZ on VESA Local Bus, 4MB on mainboard, 4MB/8MB/16MB on CF card like RAM modules
// CPU: Intel 486 SX, 486DX2, 486DX4-75 or -100,  128KB Flash-Eprom for system and video BIOS, ESS688 soundchip
ROM_START( pcd4nd )
	ROM_REGION32_LE( 0x20000, "bios", 0 )
	ROM_SYSTEM_BIOS(0, "pcd4ndno1", "pcd4ndno1")
	ROMX_LOAD( "bf3m51.bin", 0x00000, 0x20000, CRC(6a2f90dd) SHA1(75704a83976e4bb02a028e761d01bd053cc0d4e7), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "pcd4ndno2", "pcd4ndno2")
	ROMX_LOAD( "bf3q42.bin", 0x00000, 0x20000, CRC(fa81cf6e) SHA1(91313a6856ca22f40710a6c9c8a65f8e340784ab), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(2, "pcd4ndno3", "pcd4ndno3")
	ROMX_LOAD( "pcd-4nd_flash_28010.bin", 0x00000, 0x20000, CRC(53c0beea) SHA1(bfa17947529c51a8c9315884e156c01ddd23c0d8), ROM_BIOS(2) )
ROM_END

// Siemens PG-750 486 EISA
// blank screen, beeps
ROM_START( pg750eisa )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "pg_pg750_486_eisa.bin", 0x10000, 0x10000, CRC(2e6149a9) SHA1(9fcf29a6169efa1359c7c3eff09326dd3e4001dc))
ROM_END

// Highscreen 486-25 aka Midwest Micro Elite TS34T-25 notebook
// integrated trackball - CPU: i486sx-25 - Chipset: Chips F82C721, Intel ?80C51SLBG, MCCS1468188F, AvaSem AV9129-08CW28, ACC Micro 2046, LT1137CS
// Video: CL-GD6410-320C-A - HD: Maxtor 2585AT
ROM_START( ts34t25 )// blank display
	ROM_REGION32_LE( 0x20000, "bios", 0 )
	ROM_LOAD( "p101a002_sys_bios_62fc.u24", 0x00000, 0x20000, CRC(2ce568bc) SHA1(84dc595abf0bf1948a6479afdea4a169f40e3b1b))

	ROM_REGION( 0x8000, "pmu", 0 ) // ROM contains "PMU" string
	ROM_LOAD( "s34t0003_51slbios_019f.u31", 0x00000, 0x8000, CRC(40467716) SHA1(f976f2ce13eb22e0ed164d31d6382eda489545c1))
ROM_END


/***************************************************************************
  Game driver(s)
***************************************************************************/

//    YEAR  NAME          PARENT   COMPAT   MACHINE    INPUT  CLASS         INIT           COMPANY        FULLNAME                FLAGS
COMP( 199?, 486apio,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "EFA",   "486 APIO", MACHINE_NOT_WORKING )
COMP( 199?, 486ccv,       ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Diamond Flower, Inc. (DFI)", "486-CCV", MACHINE_NOT_WORKING )
COMP( 199?, 486igb21,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "<unknown>", "486IG-B-2-1", MACHINE_NOT_WORKING )
COMP( 199?, 486wb6a3,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Silicon Valley Computer, Inc.", "486WB6A3.B1", MACHINE_NOT_WORKING )
COMP( 199?, 4dmshl3g,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "EFA",   "4DMS HL3G-L4-VI", MACHINE_NOT_WORKING )
COMP( 199?, 4dmuhl3s,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "EFA",   "4DMU HL3S", MACHINE_NOT_WORKING )
COMP( 1992, a433cc,       ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "J-Bond",      "A433C-C/A450C-C", MACHINE_NOT_WORKING )
COMP( 1994, a486ap4,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Asus",        "PVI-486AP4", MACHINE_NOT_WORKING )
COMP( 199?, a486isa,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Asus",        "ISA-486", MACHINE_NOT_WORKING )
COMP( 199?, a486sio,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Asus",        "ISA-486SIO rev. 1.2", MACHINE_NOT_WORKING )
COMP( 1994, a486sp3,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Asus",        "PVI-486SP3", MACHINE_NOT_WORKING )
COMP( 1994, a486sp3g,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Asus",        "PCI/I-486SP3G", MACHINE_NOT_WORKING )
COMP( 199?, a486sv2,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Asus",        "ISA-486SV2", MACHINE_NOT_WORKING )
COMP( 1994, a486sv2g,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Asus",        "VL/I-486SV2G", MACHINE_NOT_WORKING )
COMP( 1994, a486sv1,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Asus",        "VL/EISA-486SV1", MACHINE_NOT_WORKING )
COMP( 1995, aa486s,       ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Asus",        "PCI/I-A486S", MACHINE_NOT_WORKING )
COMP( 199?, abae4,        ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Abit", "486 EISA-AE4", MACHINE_NOT_WORKING )
COMP( 199?, abah4,        ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Abit", "AB-AH4", MACHINE_NOT_WORKING )
COMP( 199?, abav4,        ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Abit", "AB-AV4", MACHINE_NOT_WORKING )
COMP( 199?, abax4,        ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Abit", "AB-AX4", MACHINE_NOT_WORKING )
COMP( 199?, abpb4,        ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Abit", "AB-PB4", MACHINE_NOT_WORKING )
COMP( 199?, abpi4,        ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Abit", "AB-PI4", MACHINE_NOT_WORKING )
COMP( 199?, abpm4,        ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Abit", "AB-PM4", MACHINE_NOT_WORKING )
COMP( 199?, abpv4,        ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Abit", "AB-PV4", MACHINE_NOT_WORKING )
COMP( 199?, abpw4,        ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Abit", "AB-PW4", MACHINE_NOT_WORKING )
COMP( 199?, alator2,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Alaris",      "Tornado 2", MACHINE_NOT_WORKING )
COMP( 199?, alim1489,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "<unknown>", "486 motherboards using the ALi 1487/1489 chipset", MACHINE_NOT_WORKING )
COMP( 199?, amient2,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "AMI",         "EISA Enterprise-II", MACHINE_NOT_WORKING )
COMP( 199?, amient3,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "AMI",         "EISA Enterprise-III", MACHINE_NOT_WORKING )
COMP( 199?, amient4,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "AMI",         "EISA Enterprise-IV", MACHINE_NOT_WORKING )
COMP( 199?, amisvpci2,    ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "AMI",         "Super Voyager PCI-II", MACHINE_NOT_WORKING )
COMP( 199?, amisvvlb,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "AMI",         "Super Voyager VLB", MACHINE_NOT_WORKING )
COMP( 199?, amisvvlb2,    ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "AMI",         "Super Voyager VLB-II", MACHINE_NOT_WORKING )
COMP( 199?, amisvvlb3,    ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "AMI",         "Super Voyager VLB-III", MACHINE_NOT_WORKING )
COMP( 199?, aoap43,       ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Aopen", "AP43",  MACHINE_NOT_WORKING )
COMP( 199?, aovi15g,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Aopen", "VI15G", MACHINE_NOT_WORKING )
COMP( 1992, aplsbon,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Apricot",     "Apricot LS Pro (Bonsai Motherboard)", MACHINE_NOT_WORKING )
COMP( 1992, aplscar,      ibm5170, 0,       at486l,    0,     at486_state,     init_at486,        "Apricot",     "Apricot LS Pro (Caracal Motherboard)", MACHINE_NOT_WORKING )
COMP( 1991, aprfte,       ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Apricot",     "Apricot FT//ex 486 (J3 Motherboard)", MACHINE_NOT_WORKING )
COMP( 1992, aprpand,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Apricot",     "Apricot FTs (Panther Rev F 1.02.26)", MACHINE_NOT_WORKING )
COMP( 1989, apvxft,       ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Apricot",     "Apricot VX FT server", MACHINE_NOT_WORKING )
COMP( 1993, apxena1,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Apricot",     "Apricot XEN PC (A1 Motherboard)", MACHINE_NOT_WORKING )
COMP( 1991, apxenls3,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Apricot",     "Apricot XEN-LS (Venus IV Motherboard)", MACHINE_NOT_WORKING )
COMP( 1993, apxenp2,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Apricot",     "Apricot XEN PC (P2 Motherboard)", MACHINE_NOT_WORKING )
COMP( 1993, apxlsam,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Apricot",     "Apricot XEN-LS II (Samurai Motherboard)", MACHINE_NOT_WORKING )
COMP( 199?, ar4glx3,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Addtech Research", "4GLX3 Green-B 4GPV3.1", MACHINE_NOT_WORKING )
COMP( 199?, as496,        ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Arstoria",    "AS496", MACHINE_NOT_WORKING )
COMP( 1990, at486,        ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "<generic>",   "PC/AT 486 (25 MHz, MF2 Keyboard)", MACHINE_NOT_WORKING )
COMP( 199?, atc1415,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "A-Trend", "ATC-1415", MACHINE_NOT_WORKING )
COMP( 199?, atc1425a,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "A-Trend", "ATC-1425A", MACHINE_NOT_WORKING )
COMP( 199?, atc1425b,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "A-Trend", "ATC-1425B", MACHINE_NOT_WORKING )
COMP( 199?, bluenote,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Highscreen",  "Colani Blue Note", MACHINE_NOT_WORKING )
COMP( 199?, ch48633c,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Chicony",   "CH-486-33C", MACHINE_NOT_WORKING )
COMP( 199?, ch486spm,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Chaintech", "486SPM", MACHINE_NOT_WORKING )
COMP( 199?, ch491e,       ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Chicony",   "CH-491E", MACHINE_NOT_WORKING )
COMP( 199?, ch4slez1,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Chaintech", "486SLE M106 4SLE-Z1", MACHINE_NOT_WORKING )
COMP( 199?, ch4spi,       ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Chaintech", "4SPI", MACHINE_NOT_WORKING )
COMP( 199?, comt486,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Commodore Business Machines",  "Tower 486", MACHINE_NOT_WORKING )
COMP( 199?, dt486,        ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Commodore Business Machines", "DT486", MACHINE_NOT_WORKING )
COMP( 199?, ec4913,       ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Elitegroup", "UC4913 REV:1.1", MACHINE_NOT_WORKING )
COMP( 199?, ec4915aio,    ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Elitegroup", "UC4915 A AIO", MACHINE_NOT_WORKING )
COMP( 199?, ed486vl3h,    ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Edom", "486VL3H", MACHINE_NOT_WORKING )
COMP( 199?, edmv035f,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Edom", "MV035F", MACHINE_NOT_WORKING )
COMP( 199?, exp4044,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "ExpertChip", "EXP4044", MACHINE_NOT_WORKING )
COMP( 199?, f4dxluc4,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "ADI", "F4DXL-UC4", MACHINE_NOT_WORKING )
COMP( 199?, fic4386vchd,  ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "First International Computer", "4386-VC-HD", MACHINE_NOT_WORKING )
COMP( 199?, fic4386vcv,   ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "First International Computer", "4386-VC-V", MACHINE_NOT_WORKING )
COMP( 199?, fic486gvt,    ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "First International Computer", "486-GVT", MACHINE_NOT_WORKING )
COMP( 199?, fic486kvd,    ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "First International Computer", "486 KVD", MACHINE_NOT_WORKING )
COMP( 199?, fic486vchd,   ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "First International Computer", "486-VC-HD", MACHINE_NOT_WORKING )
COMP( 199?, ficeli6ii,    ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "First International Computer", "FIC ELI6-II", MACHINE_NOT_WORKING )
COMP( 1994, ficgiovt2,    ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "First International Computer", "486-GIO-VT2", MACHINE_NOT_WORKING )
COMP( 1995, ficpio2,      ibm5170, 0,       ficpio2,   0,     at486_state,     init_at486pci,     "First International Computer", "486-PIO-2", MACHINE_NOT_WORKING )
COMP( 1994, ficvipio,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "First International Computer", "486-VIP-IO", MACHINE_NOT_WORKING )
COMP( 199?, ficvipio2,    ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "First International Computer", "486-VIP-IO2", MACHINE_NOT_WORKING )
COMP( 1995, ft486f55,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Freetech", "486FT55", MACHINE_NOT_WORKING )
COMP( 1991, ftsserv,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Apricot",     "Apricot FTs (Scorpion)", MACHINE_NOT_WORKING )
COMP( 199?, ga486am,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Gigabyte",    "GA-486AM/S", MACHINE_NOT_WORKING )
COMP( 199?, ga486vf,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Gigabyte",    "GA-486VF", MACHINE_NOT_WORKING )
COMP( 199?, ga486vs,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Gigabyte",    "GA-486VS", MACHINE_NOT_WORKING )
COMP( 199?, gc10a,        ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Eagle", "EAGLEN486 GC10A", MACHINE_NOT_WORKING )
COMP( 199?, gete486vl,    ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "GENOA",       "TurboExpress 486 VL", MACHINE_NOT_WORKING )
COMP( 199?, gmb486sg,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Gemlight", "GMB-486SG v2.2", MACHINE_NOT_WORKING )
COMP( 199?, gmb486unp,    ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Gemlight", "GMB-486UNP v2.1", MACHINE_NOT_WORKING )
COMP( 199?, hot409,       ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Shuttle Computer International", "HOT-409", MACHINE_NOT_WORKING )
COMP( 199?, hot419,       ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Shuttle Computer International", "HOT-419", MACHINE_NOT_WORKING )
COMP( 199?, hot433,       ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Shuttle Computer International", "HOT-433", MACHINE_NOT_WORKING )
COMP( 199?, ibm2133,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "International Business Machines",  "PS/1 2133", MACHINE_NOT_WORKING )
COMP( 199?, jakms41,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Jamicon", "KM-S4-1 VER 1.1", MACHINE_NOT_WORKING )
COMP( 199?, jwj403tg,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Jetway", "J-403TG", MACHINE_NOT_WORKING )
COMP( 199?, jwj446a,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Jetway",      "J-446A", MACHINE_NOT_WORKING )
COMP( 1993, lion3500,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Lion",        "3500", MACHINE_NOT_WORKING )
COMP( 199?, ls486e,       ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "LuckyStar",   "LS-486E Rev:C", MACHINE_NOT_WORKING )
COMP( 199?, lsucm486v30,  ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Lucky Star", "UCM-486V30", MACHINE_NOT_WORKING )
COMP( 199?, mb1433aeap,   ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Biostar",     "MB-1433/50 AEA-P - V:1", MACHINE_NOT_WORKING )
COMP( 199?, mb1433ucv,    ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Biostar",     "MB-1433UCV", MACHINE_NOT_WORKING )
COMP( 199?, mb1433uiv,    ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Biostar",     "MB-1433UIV", MACHINE_NOT_WORKING )
COMP( 199?, mb4d33,       ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Aquarius System (ASI)", "MB-4D33/50NR", MACHINE_NOT_WORKING )
COMP( 199?, mb8433uud,    ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Biostar",     "MB8433-UUD-A", MACHINE_NOT_WORKING ) // boots to Award BootBlock BIOS
COMP( 199?, mba029,       ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Mitac", "MBA-029", MACHINE_NOT_WORKING )
COMP( 199?, md4duvc,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Aquarius System (ASI)", "MD-4DUV VER:2.1", MACHINE_NOT_WORKING )
COMP( 199?, mijx30gp,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Micronics",   "JX30GP, Motherboard P/N: 09-00189-10 REV B1", MACHINE_NOT_WORKING )
COMP( 199?, ms4125,       ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "MSI",         "MS-4125", MACHINE_NOT_WORKING )
COMP( 199?, ms4132,       ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "MSI",         "MS-4132 G VER:1", MACHINE_NOT_WORKING )
COMP( 199?, ms4134,       ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "MSI",         "MS-4134", MACHINE_NOT_WORKING )
COMP( 199?, ms4138,       ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "MSI",         "MS-4138", MACHINE_NOT_WORKING )
COMP( 199?, ms4144,       ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "MSI",         "MS-4144", MACHINE_NOT_WORKING )
COMP( 199?, ms4145,       ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "MSI",         "MS-4145", MACHINE_NOT_WORKING )
COMP( 199?, nat48pv,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "<unknown>", "NAT48PV-1.00 VL", MACHINE_NOT_WORKING )
COMP( 199?, ncr3433,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "NCR", "Class 3433", MACHINE_NOT_WORKING )
COMP( 199?, ochawk,       ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Octek",       "Hawk", MACHINE_NOT_WORKING )
COMP( 199?, ochipcom,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Octek",       "Hippo COM", MACHINE_NOT_WORKING )
COMP( 1994, ochipdca2,    ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Octek",       "Hippo DCA2", MACHINE_NOT_WORKING )
COMP( 199?, ochipvlp,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Octek",       "Hippo VL+", MACHINE_NOT_WORKING )
COMP( 199?, op82c392,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "<unknown>", "486 motherboards using the OPTi OPTi 82C392, 82C493 chipset", MACHINE_NOT_WORKING )
COMP( 199?, pc70iii,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Commodore Business Machines",  "PC 70-III", MACHINE_NOT_WORKING )
COMP( 199?, pc9486,       ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Amstrad",     "PC9486", MACHINE_NOT_WORKING )
COMP( 199?, pccm912,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "PC-Chips", "M912", MACHINE_NOT_WORKING )
COMP( 199?, pccm915i,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "PC-Chips", "M915i", MACHINE_NOT_WORKING )
COMP( 199?, pccm919,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "PC-Chips", "M919", MACHINE_NOT_WORKING )
COMP( 1993, pcd4nd,       ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Siemens-Nixdorf", "PCD-4ND", MACHINE_NOT_WORKING )
COMP( 1995, pcd4nl,       ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Siemens-Nixdorf", "PCD-4NL", MACHINE_NOT_WORKING )
COMP( 199?, pcd4x,        ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Siemens-Nixdorf", "PCD-4H, PCD-4M", MACHINE_NOT_WORKING )
COMP( 199?, pci48af,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "TMC Research Corporation", "PCI48AF", MACHINE_NOT_WORKING )
COMP( 199?, pck486dx,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Peacock",  "PCK 486 DX", MACHINE_NOT_WORKING )
COMP( 199?, pg750eisa,    ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Siemens", "PG-750 486 EISA", MACHINE_NOT_WORKING )
COMP( 199?, pkm0038s,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "DTK", "PKM-0038S aka Gemlight GMB-486SG", MACHINE_NOT_WORKING )
COMP( 199?, pm486pu,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "PROTECH",  "PM486PU-S7", MACHINE_NOT_WORKING )
COMP( 199?, pt430,        ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Pine Technology", "PT-430", MACHINE_NOT_WORKING )
COMP( 199?, pt432b,       ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Pine Technology", "PT-432b aka SR-M401-A", MACHINE_NOT_WORKING )
COMP( 199?, ptmb457,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "PowerTech", "MB457", MACHINE_NOT_WORKING )
COMP( 199?, pwaih4077c,   ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Mitac", "PWA-IH4077C", MACHINE_NOT_WORKING )
COMP( 199?, pwaih4077d,   ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Mitac", "PWA-IH4077D", MACHINE_NOT_WORKING )
COMP( 199?, px486p3,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "QDI", "PX486P3", MACHINE_NOT_WORKING )
COMP( 1990, qi900,        ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Apricot",     "Apricot Qi 900 (Scorpion Motherboard)", MACHINE_NOT_WORKING )
COMP( 199?, sis85c471,    ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "<unknown>", "486 motherboards using the SiS 85C471/85C407 chipset", MACHINE_NOT_WORKING )
COMP( 199?, sis85c496,    ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "<unknown>", "486 motherboards using the SiS 85C496/85C497 chipset", MACHINE_NOT_WORKING )
COMP( 199?, sm48650usc,   ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Vintage Sprite", "SM 486-50USC", MACHINE_NOT_WORKING )
COMP( 199?, so025d2,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "SOYO", "025D2", MACHINE_NOT_WORKING )
COMP( 199?, so025k2,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "SOYO", "025K2", MACHINE_NOT_WORKING )
COMP( 199?, so025r2,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "SOYO", "025R2", MACHINE_NOT_WORKING )
COMP( 199?, so30h,        ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "SOYO", "30H", MACHINE_NOT_WORKING )
COMP( 199?, so4saw2,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "SOYO", "SY-4SAW2", MACHINE_NOT_WORKING )
COMP( 199?, sto486wb,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "See-Thru", "Sto486Wb aka AUVA Cam-33-P2", MACHINE_NOT_WORKING )
COMP( 199?, td4ipaio,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "<unknown>", "TD-4IP-UMC-AIO", MACHINE_NOT_WORKING )
COMP( 199?, tmpat48pg4,   ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "TMC", "PAT48PG4", MACHINE_NOT_WORKING )
COMP( 199?, tmpat48av,    ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "TMC", "PAT48AV", MACHINE_NOT_WORKING )
COMP( 199?, ts34t25,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Highscreen",  "486-25", MACHINE_NOT_WORKING )
COMP( 199?, um486,        ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Elitegroup", "UM486/UM486sx", MACHINE_NOT_WORKING )
COMP( 199?, um486v,       ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Elitegroup", "UM486V-AIO", MACHINE_NOT_WORKING )
COMP( 199?, um8810paio,   ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "Elitegroup", "UM8810 PAIO", MACHINE_NOT_WORKING )
COMP( 199?, um8886,       ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "<unknown>", "486 motherboards using the UMC UM8886/UM8881 chipset", MACHINE_NOT_WORKING )
COMP( 199?, um8498f,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "<unknown>", "486 motherboards using the UMC UM8498F, UM8496F chipset", MACHINE_NOT_WORKING )
COMP( 199?, uni4800,      ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "<unknown>", "486 motherboards using the UNI4800 chipset", MACHINE_NOT_WORKING )
COMP( 199?, uniwb4407,    ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "UNICHIP", "486 WB 4407 REV 1.0", MACHINE_NOT_WORKING )
COMP( 199?, v4p895p3,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "QDI", "V4P895P3/SMT V5.0", MACHINE_NOT_WORKING )
COMP( 199?, via4386vio,   ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "<unknown>", "Via 4386 VIO / Highscreen universal board", MACHINE_NOT_WORKING )
COMP( 199?, zi4dvs,       ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "ZIDA", "4DVS", MACHINE_NOT_WORKING )
COMP( 199?, zito4dps,     ibm5170, 0,       at486,     0,     at486_state,     init_at486,        "ZIDA", "Tomato board 4DPS", MACHINE_NOT_WORKING )
