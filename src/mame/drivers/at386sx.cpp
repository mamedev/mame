// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic
/***************************************************************************

IBM AT compatibles using a 386sx class CPU
split from at.cpp

***************************************************************************/

#include "emu.h"

/* mingw-gcc defines this */
#ifdef i386
#undef i386
#endif /* i386 */

#include "bus/isa/isa_cards.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "cpu/i386/i386.h"
#include "machine/at.h"
#include "machine/cs8221.h"
// #include "machine/ds128x.h"
// #include "machine/idectrl.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "emupal.h"
#include "softlist_dev.h"
#include "speaker.h"

class at386sx_state : public driver_device
{
public:
	at386sx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mb(*this, "mb"),
		m_ram(*this, RAM_TAG)
	{ }

	void at386sx(machine_config &config);
	void ct386sx(machine_config &config);

	void init_at386sx();

protected:
	required_device<cpu_device> m_maincpu;
	required_device<at_mb_device> m_mb;
	required_device<ram_device> m_ram;

	void init_at386sx_common(int xmsbase);

	void at386sx_map(address_map &map);
	void at386sx_io(address_map &map);
	void neat386sx_io(address_map &map);
};

void at386sx_state::at386sx_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x09ffff).bankrw("bank10");
	map(0x0e0000, 0x0fffff).rom().region("bios", 0);
	map(0xfe0000, 0xffffff).rom().region("bios", 0);
}

void at386sx_state::at386sx_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).m(m_mb, FUNC(at_mb_device::map));
}

void at386sx_state::neat386sx_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).m(m_mb, FUNC(at_mb_device::map));
	map(0x0022, 0x0023).m("cs8221", FUNC(cs8221_device::map));
}


/**********************************************************
 Init functions
**********************************************************/

 void at386sx_state::init_at386sx_common(int xmsbase)
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

void at386sx_state::init_at386sx()
{
	init_at386sx_common(0xa0000);
}


/**********************************************************
 Machine configurations
**********************************************************/

void at386sx_state::at386sx(machine_config &config)
{
	/* basic machine hardware */
	i386sx_device &maincpu(I386SX(config, m_maincpu, 16'000'000)); /* 386SX */
	maincpu.set_addrmap(AS_PROGRAM, &at386sx_state::at386sx_map);
	maincpu.set_addrmap(AS_IO, &at386sx_state::at386sx_io);
	maincpu.set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));

	config.set_maximum_quantum(attotime::from_hz(60));

	AT_MB(config, m_mb).at_softlists(config);
	m_mb->kbd_clk().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	m_mb->kbd_data().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	// FIXME: determine ISA bus clock
	ISA16_SLOT(config, "isa1", 0, "mb:isabus", pc_isa16_cards, "svga_et4k", false);
	ISA16_SLOT(config, "isa2", 0, "mb:isabus", pc_isa16_cards, "fdc", false);
	ISA16_SLOT(config, "isa3", 0, "mb:isabus", pc_isa16_cards, "comat", false);
	ISA16_SLOT(config, "isa4", 0, "mb:isabus", pc_isa16_cards, "ide", false);
	ISA16_SLOT(config, "isa5", 0, "mb:isabus", pc_isa16_cards, nullptr, false); // FIXME: determine ISA bus clock

	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL));
	pc_kbdc.out_clock_cb().set(m_mb, FUNC(at_mb_device::kbd_clk_w));
	pc_kbdc.out_data_cb().set(m_mb, FUNC(at_mb_device::kbd_data_w));

	/* internal ram */
	RAM(config, m_ram).set_default_size("1664K").set_extra_options("640K,1024K,2M,4M,8M,15M");
}

void at386sx_state::ct386sx(machine_config &config)
{
	at386sx(config);
	m_maincpu->set_addrmap(AS_IO, &at386sx_state::neat386sx_io);
	CS8221(config, "cs8221", 0, "maincpu", "mb:isa", "maincpu");
}

/***************************************************************************
  ROM DEFINITIONS
***************************************************************************/

/***************************************************************************
  IBM systems
***************************************************************************/

// https://en.wikipedia.org/wiki/IBM_PS/1
// http://ps-2.kev009.com/pcpartnerinfo/ctstips/937e.htm
// https://ps1stuff.wordpress.com/documentation/ibm-ps1-model-2011/
// https://barotto.github.io/IBMulator/#download

// From Wikipedia:
// Model     MB FRU    CPU                   ISA Sl.  RAM   VRAM   Hard-Drive         Serial/Modem
// 2121-C42  92F9690   Intel 80386SX @ 16 MHz  0      2 MB  256KB  95F4720  40MB IDE  2400 baud modem
// 2121-B82  92F9690   Intel 80386SX @ 16 MHz  2      2 MB  256KB  92F9943  80MB IDE  2400 baud modem
// 2121-C92            Intel 80386SX @ 16 MHz  0      2 MB  256KB          129MB IDE  2400 baud modem
// 2121-G42            Intel 80386SX @ 20 MHz  0      2 MB  256KB           40MB IDE  2400 baud modem
// 2121-A82            Intel 80386SX @ 20 MHz  2      2 MB  256KB           40MB IDE  2400 baud modem
// 2121-S92            Intel 80386SX @ 20 MHz  0      2 MB  256KB          129MB IDE  2400 baud modem
// 2121-M82            Intel 80386SX @ 20 MHz  2      2 MB  256KB           80MB IDE  2400 baud modem
// 2121-A62                                           2     256KB  56F8863 160MB IDE  2400 baud modem
// 2121-A92                                                 256KB                     serial port
// 2121-A94            Intel 80386SX @ 20 MHz  2    6 MB    256KB          129MB IDE  2400 baud modem

ROM_START( ibm2121 )
	ROM_REGION16_LE( 0x40000, "bios", 0)
	ROM_SYSTEM_BIOS( 0, "2121", "IBM PS/1 2121" )
	ROMX_LOAD( "fc0000.bin", 0x00000, 0x40000, CRC(96bbaf52) SHA1(8737d805444837023a58702279f8fe6e7f08e7ba), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "2121us", "IBM PS/1 2121 US" )
	ROMX_LOAD( "ibm2121us_fc0000.bin", 0x00000, 0x40000, CRC(817aad71) SHA1(43b7b84390fcc081a946cdb4bdce4ba7a4a88074), ROM_BIOS(1))
ROM_END

ROM_START( ibm2121rd ) // international versions shipped with ROM DOS, need a different memory map at least
	ROM_REGION16_LE( 0x80000, "bios", 0)
	ROM_SYSTEM_BIOS( 0, "2121sp", "IBM PS/1 2121 Spanish" )
	ROMX_LOAD( "ibm2121sp_f80000.bin", 0x00000, 0x40000, CRC(90505c4b) SHA1(59becaec25644820a78464d66e472a8a225d94cc), ROM_BIOS(0))
	ROMX_LOAD( "ibm2121sp_fc0000.bin", 0x40000, 0x40000, CRC(f83fac75) SHA1(a42b1b9465983392eaa0159d4bfc30620a7af499), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "2121fr", "IBM PS/1 2121 French" )
	ROMX_LOAD( "ibm2121fr_f80000.bin", 0x00000, 0x40000, CRC(9c6de65d) SHA1(6b219c9480a06bc9218e8212acc7cfd1ceaccd4b), ROM_BIOS(1))
	ROMX_LOAD( "ibm2121fr_fc0000.bin", 0x40000, 0x40000, CRC(f83fac75) SHA1(a42b1b9465983392eaa0159d4bfc30620a7af499), ROM_BIOS(1))
ROM_END

// http://ps-2.kev009.com/pcpartnerinfo/ctstips/937e.htm
ROM_START( ibm2123 )
	ROM_REGION16_LE( 0x20000, "bios", 0)
	ROM_LOAD( "ps1_2123_87f4794_rom.bin", 0x00000, 0x20000, CRC(64f921b8) SHA1(e1856bf3dd3ce21f44078aeca1f58c491b202ad2))
ROM_END


/***************************************************************************
  80386 SX BIOS
***************************************************************************/

ROM_START( at386sx )
	ROM_REGION16_LE(0x20000, "bios", 0 )
	// 0: NCR 386 CPU card - Chipset: TACT82301PB, TACT82302PB, TACT82303PB
	ROM_SYSTEM_BIOS( 0, "ncr386sx", "NCR 386sx card" ) // Upgrade card for e.g. NCR PC-8 - Setup Version 2.7.1
	ROMX_LOAD( "ncr_386sx_u12-19_7.3.bin", 0x10001, 0x8000, CRC(9e4c9a2a) SHA1(0a45d9f04f03b7ae39734916af7786bc52e5e917), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "ncr_386sx_u46-17_7.3.bin", 0x10000, 0x8000, CRC(73ad83a2) SHA1(bf6704fb4a0da37251f192cea3af2bc8cc2e9cdb), ROM_SKIP(1) | ROM_BIOS(0))
	// ROM_LOAD( "ncr_386sx_card_150-0004508_u1_v1.1.bin", 0x0000, 0x800, CRC(dd591ac1) SHA1(5bc40ca7340fa57aaf5d707be45a288f14085807))
	// 1: Dell 386SX-33 with 2x 72-pin SIMMs, ISA riser slot -  Chipset: VLSI 82C311, Cirrus Logic GD5420 - BIOS: 27C010 EPROM containing Quadtel VGA BIOS and Phoenix system BIOS 02/09/93
	// BIOS-String: Phoenix 80386 ROM BIOS PLUS Version 1.10 J01 - Copyrights by Phoenix and Dell - Jostens Learning Corporation 386/33SX - CPU: Intel 386sx
	ROM_SYSTEM_BIOS( 1, "dell386sx", "Dell 386sx" ) // emulate integrated VGA card
	ROMX_LOAD( "dell386.bin", 0x00000, 0x20000, CRC(d670f321) SHA1(72ba3a76874e0c76231dc6138eb56a8ca46b4b12), ROM_BIOS(1))
	// 2: A3286/A3886-01 COMP V4.0 - Chipset: Intel S82344A (VLSI), S82343 (VLSI) - BIOS: AMI P9 (386SX) BIOS 910520
	// BIOS-String: - 30-05T1-425004-00101111-050591-ITOPSX-0 / MULTITRONIC PERSONAL COMPUTER - Keyboard-BIOS: AMI P9(386SX) Keyboard BIOS 910520 - OSC: 8.000, 14.318180MHz, (unreadable) - CPU: Intel SMD, unreadable - ISA8: 1, ISA16: 5
	ROM_SYSTEM_BIOS( 2, "a3286a3886", "A3286/A3886-01 COMP V4.0")
	ROMX_LOAD( "386-a3286-a3886-01-even_32k.bin", 0x10000, 0x8000, CRC(56ed3332) SHA1(9d113e57228ee596c0c24eabb193d3670fb9a309), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "386-a3286-a3886-01-odd_32k.bin", 0x10001, 0x8000, CRC(9dbe4874) SHA1(589379055cfedd4268d8b1786491e80527f7fad5), ROM_SKIP(1) | ROM_BIOS(2))
	// 3: CPU/FPU: 386SX/486SLC - Chipset: ALD 93C308-A (93C206 ???)
	// BIOS-String: X0-0100-000000-00101111-060692-386SX-0 / CC-070794-P01
	ROM_SYSTEM_BIOS( 3, "ald93c308", "ALD 93C308" )
	ROMX_LOAD( "3ldm001.bin", 0x10000, 0x10000, CRC(56bab3c7) SHA1(6970bdc7407b4b57c8e1d493f9e3d9ae70671b9c), ROM_BIOS(3))
	// 4: BIOS: Phoenix; 01/15/88 - CPU: 386sx-16 - Chipset: Intel - BIOS-String: Phoenix 80386 ROM BIOS PLUS Version 1.10 02 / 386SX, ADI CORP.
	ROM_SYSTEM_BIOS( 4, "intel", "Intel chipset")
	ROMX_LOAD( "3iip001l.bin", 0x10000, 0x8000, CRC(f7bef447) SHA1(a6d34c3bf0de93c2b71010948c1f16354996b5ab), ROM_SKIP(1) | ROM_BIOS(4))
	ROMX_LOAD( "3iip001h.bin", 0x10001, 0x8000, CRC(f46dc8a2) SHA1(b6566fd761e2e6ec34b61ee3bb043ef62d696b5e), ROM_SKIP(1) | ROM_BIOS(4))
	// 5: BIOS-String: X0-0100-000000-00101111-060692-386sx-0 / Ver. 5.14 - continuous reset
	ROM_SYSTEM_BIOS( 5, "v514", "V. 5.14")
	ROMX_LOAD( "3zzm001.bin", 0x10000, 0x10000, CRC(f465b03d) SHA1(8294825dcaa254c606cee21db7c74f61c1394ade), ROM_BIOS(5))
ROM_END

// NEATsx chipset: Chips 82C811 CPU/Bus controller, 82C812 Page interleave/EMS memory controller, 82C215 Data/Address buffer and 82C206 Integrated Peripheral Controller
ROM_START( ct386sx )
	ROM_REGION16_LE(0x20000, "bios", 0)
	// 0: BIOS-String: ENSX-1131-0040990-K0 / AMI 386SX-BIOS / NEATSX V1.1 05-31-90
	ROM_SYSTEM_BIOS(0, "neatsx", "NEATsx 386sx")
	ROMX_LOAD( "012l-u25.bin", 0x10000, 0x8000, CRC(4ab1862d) SHA1(d4e8d0ff43731270478ca7671a129080ff350a4f),ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "012h-u24.bin", 0x10001, 0x8000, CRC(17472521) SHA1(7588c148fe53d9dc4cb2d0ab6e0fd51a39bb5d1a),ROM_SKIP(1) | ROM_BIOS(0))
	ROM_FILL(0x1e2c9, 1, 0x00) // skip incompatible keyboard controller test
	ROM_FILL(0x1e2cb, 1, 0xbb) // fix checksum
	// 1: VIP-M345000 NPM-16 - BIOS-String: Phoenix 80386 ROM BIOS PLUS Version 1.10 20 / AUVA - Keyboard-BIOS: M5L8042-165P
	// ISA8: 2, ISA16: 5, Memory: 1 - OSC: 32.000MHz, 14.31818
	ROM_SYSTEM_BIOS(1, "m345000", "VIP-M345000 NPM-16")
	ROMX_LOAD( "386-vip-m345000 a1_32k.bin", 0x10001, 0x8000, CRC(8119667f) SHA1(343221a9729f841eb23eafe5505f1216783e5550), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "386-vip-m345000 a2_32k.bin", 0x10000, 0x8000, CRC(ada1a375) SHA1(74128270aa8fed504e8785c5d490b0fa25cc3895), ROM_SKIP(1) | ROM_BIOS(1))
	// 2: DTK Corp. 386sx COMPUTER / DTK 386sx Chipset ROM BIOS Version 4.26 / #96120590N
	ROM_SYSTEM_BIOS(2, "dtk386sx", "DTK 386sx")
	ROMX_LOAD( "3cso001.bin", 0x10000, 0x10000, CRC(8a0e26da) SHA1(94aefc745b51015426a73015ab7892b88e7c8bcf), ROM_BIOS(2))
	// 3: Chipset is labeled SOLUTIONS 88C211, 88C212, 88C215, P82C206
	// BIOS-String: ENSX-1107-040990-K0 - CPU: 386SX-16
	ROM_SYSTEM_BIOS(3, "solutions", "SOLUTIONS NEATsx")
	ROMX_LOAD( "3som001l.bin", 0x10000, 0x8000, CRC(ecec5d42) SHA1(b1aaed408fe9c3b73dff3fa8b19e62600a49cdb2), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD( "3som001h.bin", 0x10001, 0x8000, CRC(85d64a86) SHA1(528506724668ea3aef6aa0bd8d68cfcaa58bf519), ROM_SKIP(1) | ROM_BIOS(3))
	// 4: Manufacturer/Identifier: ELITEGROUP COMPUTER SYSTEMS, INC. NEATSX REV 1.0. - Chipset: NEATsx: Chips P82C206, Chips P82C215, Chips P82C811, Chips P82C812
	// CPU: Intel NC80386sx-20, FPU socket provided - RAM: DIP 1MB (DIP sockets for up to 4MB), 4xSIPP30 - BIOS EPROMs: 2x S27C256 NEATsx-013 PLUS
	// Keyboard BIOS: 1988 AMI 1131 KEYBOARD BIOS PLUS - OSC: 32.0000MHz, 40.000MHz, 14.31818 - ISA8: 3, ISA16: 5
	ROM_SYSTEM_BIOS(4, "neatsx013", "NEATsx-013 PLUS") // initializes the graphics card, then dies
	ROMX_LOAD( "neatsx-013_l.bin", 0x10000, 0x8000, CRC(7cd4d870) SHA1(c7a5b629dadb43779939043ae4adb5e78c770dc3), ROM_SKIP(1) | ROM_BIOS(4))
	ROMX_LOAD( "neatsx-013_h.bin", 0x10001, 0x8000, CRC(388587d4) SHA1(8ae6f6b14a2f53438b6a02c4f032088edb2df484), ROM_SKIP(1) | ROM_BIOS(4))
ROM_END


/**************************************************************************
  80386 SX motherboard
**************************************************************************/

// ZEOS 386 SX-16 - Chipset: VLSI VL82C201-16QC, 82C202, 82C203, 82C204, 82C205A, 82C100
// DVLX-6099-091589-K0 - AMI 386 BIOS for ZEOS INTERNATIONAL, LTD. / ST. PAUL, MINNESOTA 55112 USA
// (C)1987 American Megatrends Inc.386-BIOS (C) 1989 AMI, for ZEOS INTERNATIONAL - CPU: Intel 386sx-33, FPU sockets provided
// RAM: 36x16/18pin - ISA8: 2, ISA16: 6 - OSC: 64.000000, 14.31818
ROM_START( zeos386sx )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "zeos_386_16 even.bin", 0x10000, 0x8000, CRC(9897f140) SHA1(b01de83d438ceda4a80e92dc1fc7d725323427b6), ROM_SKIP(1))
	ROMX_LOAD( "zeos_386_16 odd.bin", 0x10001, 0x8000, CRC(0f930857) SHA1(c63670eb336f1998532a4f601d1845902be279e7), ROM_SKIP(1))
ROM_END

// 486MMBO4088 - Chipset : ETEQ ET486SLC2 A, P82C206 - BIOS : MR 386SX/86SLC BIOS V.1.61
// Keyboard-BIOS: AMI MEGA-KB-F-WP - CPU: TI 486 TX486SLC/E -33MAB / FPU: IIT XC87SLC-33 - RAM: 8xSIMM30, Cache: 3xISSI IS61C256AH-15N, 2xIS61C256A-20N
// OSC: 14.31818MHz, 66.666MHz - ISA16: 7
ROM_START( mmbo4088 ) // screen remains blank
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "eteq-mrb.bin", 0x10000, 0x10000, CRC(10267465) SHA1(9cc1e7e8b0ec6e0798229489dce5b2b0300bfbd8))
ROM_END

// ILON USA, INC. M-396B - Chipset: PC Chip (silk screen sanded off), KS82C6818A - BIOS: AMIBIOS - BIOS-string: 30-0500-ZZ1437-001001111-070791-PC CHIP-F
// Keyboard-BIOS: Regional HT6542 - CPU: i386sx-25, FPU socket provided - RAM: 4xSIMM30, 8x20pin DIP - OSC: 14.31818, 50.000MHz - ISA16: 6
ROM_START( ilm396b )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "m396b.bin", 0x10000, 0x10000, CRC(e46bd610) SHA1(e5899f126ae6478f4db238cd1db835e0d1877893))
ROM_END

// Prolink P386SX-25PW VER:2.00 - Chipset: OPTi 82C281, F82C206 - BIOS/Version: AMI V007B316 - Keyboard-BIOS: AMI KB-BIOS-VER-F - OSC: 14.31818MHz, 50.000000MHz
// CPU: AMD Am386SX-25, FPU: IIT 3C87SX-33 - RAM: 8xSIMM30, Cache: 4x28pin, 2x24pin - ISA16: 8 -
// BIOS-String: 30-0100-008003-00101111-042591-OPSX-0 / OPSX 386SX BIOS / P386SX/25 PW IVN 1.0 1991.7
ROM_START( p386sx25pw )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "p386sx25pw.bin", 0x10000, 0x10000, CRC(9cbebe61) SHA1(ef90c1a9cc7fc3accdac9738aaf519c1b3d8260d))
ROM_END

// ELT-P9 / Most likely ELT-386SX-160D - Chipset: NEC ELT3000A - CPU: Intel 80386sx-16 - RAM: 8xSIPP30, 1MB DIP - BIOS: Phoenix
// Keyboard BIOS: Intel P8242 - DIP6: 000000 - OSC: 14.31818MHz, 32.000MHz  - ISA8: 1, ISA16: 6, ISA8/RAM: 1
ROM_START( eltp9 ) // Phoenix 80386 ROM BIOS PLUS Version 1.10.20
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "386_676849_l.bin", 0x10000, 0x8000, CRC(ab477e5e) SHA1(78d784f9c320fe7206c2601c4dbb47a90e5cbf96), ROM_SKIP(1))
	ROMX_LOAD( "386_676849_h.bin", 0x10001, 0x8000, CRC(02241258) SHA1(2bd80bab573cd88829edfb85522e978e5e477806), ROM_SKIP(1))
ROM_END

// QTC-SXM KT X20T02/HI Rev.3 - Chipset: VLSI / Intel S82343 + S82344A - BIOS Version: QTC-SXN 03.05.07
// Keyboard BIOS: NEC Performance 204 (c) Quadtel 265 - CPU: i386sx-20, FPU: Intel 387sx
// RAM: 4xSIMM30, sockets for 1MB DIP - ISA8: 2, ISA16: 4 - OSC: 15.31818MHz, 16.000MHz, 40.0000
ROM_START( ktx20t02 ) // BIOS-String: Quadtel VL82C286 386SX BIOS Version 3.05.07
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "qtc-sxm-even-u3-05-07.bin", 0x10000, 0x8000, CRC(21f2ca4c) SHA1(7131de700cb95c825e61d611001bab1d3d3bb195), ROM_SKIP(1))
	ROMX_LOAD( "qtc-sxm-odd-u3-05-07.bin", 0x10001, 0x8000, CRC(1543d0f7) SHA1(12281ef81d7aabf291586f96b678074216f0c23a), ROM_SKIP(1))
ROM_END

// Packard Bell PCB-303 Rev.01 - Chipset: ACC Micro 2036 25MHz, UM82C862F - CPU: i386sx-25, FPU socket provided - RAM: 6xSIMM30 - ISA16: 1
// VGA on board: OTI 512KB - on board: Floppy, IDE, ser, par, PS/2 mouse and keyboard - OSC: 50.000MHz x2
ROM_START( pcb303 ) // Phoenix 80386 ROM BIOS PLUS Version 1.10 2715 - 19920411102517 - Error 8602 - Auxiliary Device Failure
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "386-packard-pcb-303.bin", 0x00000, 0x20000, CRC(bbb18c76) SHA1(94f3785c3b96dfe7ab53a97d097bfb17c57229b7))
ROM_END

// Philips P3239 (aka Headstart / MaxStation / Magnum / Professional 1200, 48CD, 1600, 64CD, P160, SR16CDPhilips 5107-100-36154 (motherboard),
// 5107-200-35452 (CPU card) - Chipset: VLSI VL82C311-FC2, VL82C107-VC, WD37C65CJM
// CPU (on card): Intel 386sx-20, FPU socket provided - BIOS: M27C1001, 48805 P3239 - on board: 2xser, par, VGA, Floppy, IDE
// RAM: 1MB, 1xSIMM72 - VGA on board: CL-GD5325-40QC-A, MUSIC TR9CI710-50DCA, 256KB - OSC: 9.600, 1.8432MHz, 14.31818, 16.000, 40.000 (on CPU card)
// DIP4: 0001
ROM_START( php3239 ) // no display, beep code sounds like morse
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "386-philips_p3239.bin", 0x00000, 0x20000, CRC(9178e136) SHA1(e3bad39fcf028f459516d4e9a895035891eb801e))
ROM_END

// Diamond Flower International 386SX-16/20CN Rev 1.0 - Chipset : CHIPS P82C206, P82C215A, P82C812,P82C811 - BIOS: AMI
// CPU: Intel 80386SX-20, FPU socket privided - RAM: 8xSIMM30 - ISA8: 2, ISA16: 6
ROM_START( dfi386sx ) // Hangs after initialising the graphics card
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "dfi_386sx_16cn_20cn_27c256_low.bin", 0x10000, 0x8000, CRC(58456dc1) SHA1(51f7052366106fe564aedb99ddf6974f6ad4c5cc), ROM_SKIP(1))
	ROMX_LOAD( "dfi_386sx_16cn_20cn_27c256_high.bin", 0x10001, 0x8000, CRC(1ab36d4b) SHA1(640af1c333255cca85543bc369cb0cede45e1ef6), ROM_SKIP(1))
ROM_END

// 3SIUD-1.1 - CPU: AMD Am386SX/SXL-25 - Chipset: SiS 85C206, UMC (unreadable) - RAM: SIMM30x4, 8x20pin, 4x16pin
// BIOS: AMI 386SX BIOS 70167 - Keyboard-BIOS: NEC KB-BIOS VER7 - ISA8: 1, ISA16: 5 - OSC: 50.000000MHz, 14.31818MHz
// BIOS-String: 30-0200-ZZ1266-00101111-050591-UMC386SX-0 / 3SIUD-1.0
ROM_START( 3siud )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "3siud_bios.bin", 0x10000, 0x10000, CRC(24fa8491) SHA1(635ce3db872e39d84f299356b960b0a16e2cf082))
ROM_END

// Elitegroup ELT-386SX-160BE - Chips P82C206 - CPU: Intel 386sx-16, FPU: socket provided - BIOS:Phoenix 679006 - Keyboard-BIOS: Intel P8242/Phoenix
// ISA8: 2, ISA16: 5 - OSC: 14.31818MHz, 32.000MHz - BIOS-String: Phoenix 80386 ROM BIOS PLUS Version 1.10 22 / ELT-386SX(P9)
ROM_START( elt386sx )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "elt386l.bin", 0x10000, 0x8000, CRC(95fd5508) SHA1(a66cd78f52f3931c6f8486db0d39f4e55244dcea), ROM_SKIP(1))
	ROMX_LOAD( "elt386h.bin", 0x10001, 0x8000, CRC(90c0597a) SHA1(b67b39662a0bb8c0cde1635d3fd3c1f9fbaad3c0), ROM_SKIP(1))
ROM_END

// TD70N motherboard - Chipset: Citygate D100-011 - ISA16: 6 - Keyboard-BIOS: JETkey V5.0 - CPU/FPU: Am386SX/SXL-33, i387SX
ROM_START( td70n )
	ROM_REGION16_LE(0x20000, "bios", 0)
	// 0: BIOS: AMI, Version 3.10 -  BIOS-String: 20-0100-009999-00101111-060692-CGD90-F / TD70N BIOS VERSION 3.10
	ROM_SYSTEM_BIOS( 0, "td70nv310", "TD70N V3.10" )
	ROMX_LOAD( "3cgm001.bin", 0x10000, 0x8000, CRC(8e58f42c) SHA1(56e2833457424d7176f8360470556629115493df), ROM_SKIP(1) | ROM_BIOS(0) )
	ROM_CONTINUE( 0x10001, 0x8000 )
	// 1: BIOS: AMI, Version 3.23T - BIOS-String: 20-0100-009999-00101111-060692-CGD90-F / BIOS VERSION 3.23T
	ROM_SYSTEM_BIOS( 1, "td70nv323", "TD70N V3.23T" )
	ROMX_LOAD( "3cgm002.bin", 0x10000, 0x8000, CRC(bca54fd8) SHA1(35b568c675e58965074162a93cf04918fc8d240f), ROM_SKIP(1) | ROM_BIOS(1) )
	ROM_CONTINUE( 0x10001, 0x8000 )
ROM_END

// TD70A and TD70AN motherboards - Chipset: Citygate D110-014, KS83C206Q - ISA8: 1, ISA16: 5 - Keyboard-BIOS: JETkey V5.0 - CPU: Am386SX-40
ROM_START( td70a ) // 8042 GATE-A20 ERROR - SYSTEM HALTED
	ROM_REGION16_LE(0x20000, "bios", 0)
	// BIOS: AMI, Version 2.60 - BIOS-String: 20-0100-009999-00101111-060692-CGD90-F / TD70A BIOS VERSION 2.60
	ROM_SYSTEM_BIOS( 0, "td70a", "TD70A" )
	ROMX_LOAD( "3cgm003.bin", 0x10000, 0x8000, CRC(1a92bf18) SHA1(520cd6923dd7b42544f8874813fbf81841778519), ROM_SKIP(1) | ROM_BIOS(0) )
	ROM_CONTINUE( 0x10001, 0x8000 )
	// 1: BIOS-String: 20-0100-009999-00101111-060692-CGD90-F / TD70A BIOS VERSION 2.60G
	ROM_SYSTEM_BIOS( 1, "td70an", "TD70AN")
	ROMX_LOAD( "bios.bin", 0x10000, 0x8000, CRC(0924948b) SHA1(e66b5223a7fb0b3ddb30ad0873ff099abf331262), ROM_SKIP(1) | ROM_BIOS(1) )
	ROM_CONTINUE( 0x10001, 0x8000 )
ROM_END

// MORSE KP 386SX V2.21 - Chipset: MORSE 91A300 (sticker), UMC UM82C206L - BIOS: AMI 386SX BIOS (Ver. 2.10) C-1216 - ISA8: 2, ISA16: 6
// BIOS-String: - 30-0200-ZZ1216-00101111-050591-386SX-0 - Keyboard-BIOS: AMI KEYBOARD BIOS PLUS C-1216 - CPU: AM-386SX/SXL-25, FPU: iN80287-12 - OSC: 8.000, 14.31818, 50.000 MHz
ROM_START( mokp386sx )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "386-morse_kp386sx.bin", 0x10000, 0x10000, CRC(f3a9c69f) SHA1(6e028a11f3770d7cda814dfa698f2ab5d6dba535))
ROM_END

// SCsxAIO - Chipset: Chips 82C236 (SCATsx), Acer M5105 A3E - On board: 2xCOM, Floppy, ISA
// BIOS-String: Peacock 386sx Ver. 2.0 24.03.92 30-0000-D01131-00101111-070797-SCATsx-8 - ISA16: 6
ROM_START( scsxaio )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "386-peacock_scsxaio.bin", 0x10000, 0x10000, CRC(54c3cacd) SHA1(b3c821b30052d0c771b5004a3746eb2cfd186c79))
ROM_END

// Shuttle 386SX REV 2.0A - Chipset: KU82335 SX042, Intel N82230-2 (Zymos); Intel N82231-2 (Zymos), BIOS: AMI 80386SX BIOS PLUS Ser #039747
// BIOS-String: - DINT-1216-073089-K0 / 386-BIOS AMI for MORSE 386SX Personal Computer
// Keyboard-BIOS: AMI 386 Keyboard BIOS PLUS Ser.# 039747, CPU: unreadable (SMD), FPU: empty socket - OSC: 32.000 MHz, 14.31818 - ISA8: 2, ISA16: 6
ROM_START( sh386sx20 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROMX_LOAD( "386-shuttle386sx-even_32k.bin", 0x10000, 0x8000, CRC(8b0c3d96) SHA1(73b6315928161a013cfe81b226606dfae5a8ef94), ROM_SKIP(1) )
	ROMX_LOAD( "386-shuttle386sx-odd_32k.bin", 0x10001, 0x8000, CRC(9c547735) SHA1(3cef5290324aab9d7523e98bf511eaea351e580d), ROM_SKIP(1) )
ROM_END


/***** 386sx motherboards using the Chips SCAMPSX chipset *****/

// ANIX CH-386S-16/20/25G P/N:001-0386S-016 VER 1.0 - Chipset: CHIPS F82C836 - BIOS: AMI 386sx BIOS PLUS S/NO. 141334
// BIOS-String: 30-0100-D01425-00101111-050591-SCAMPSX-0 - Keyboard-BIOS: Intel/AMI - CPU: Intel (SMD), label not readable - FPU: socket available - ISA16: 6 - OSC: 14.31818 - 32.000 MHz
ROM_START( anch386s )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "386-ch-386s.bin", 0x10000, 0x10000, CRC(8902c64b) SHA1(3234bac6240a3a0bd05302c9ca587f5ae083f2f4))
ROM_END

// 80386SX-VH-COM - Chipset: VLSI VL82C311 VL82C113 - BIOS: Award 386SX-BIOS - BIOS-String: 386SX Modular BIOS v3.15 Copyright(c)1984-91 Award Software Inc.
// CPU: i386sx-25, FPU socket provided - RAM: 8xSIMM8 - OSC: 16MHz, 14.31818, 50.000MHz - ISA8: 2, ISA16: 6
ROM_START( 386sxvhcom )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "80386sx-vh-com.bin", 0x10000, 0x10000, CRC(65f5d279) SHA1(81c40ad1f7dde3235a074d97768ed7e09cf05f52))
ROM_END

ROM_START( scamp386sx )
	ROM_REGION16_LE(0x20000,"bios", 0)
	// 0: BIOS-String: 30-0100-D61204-00101111-050591-SCAMPSX-0 / MB-1316/20/25VST
	ROM_SYSTEM_BIOS(0, "mb386sx", "mb386sx-25spb") // VLSI SCAMPSX
	ROMX_LOAD( "386sx_bios_plus.bin", 0x10000, 0x10000, CRC(f71e5a8d) SHA1(e73fda2547d92bf578e93623d5f2349b97e22393), ROM_BIOS(0))
	// 1: BIOS-String: 30-0400-428027-00101111-070791-SCMPSX-0 / VLSI SCAMP 386SX 16/20/25MHz - seen on a PC-Chips/Amtron board
	ROM_SYSTEM_BIOS(1, "scamp01", "VLSI SCAMPSX #1")
	ROMX_LOAD( "ami_386sx_vlsi_scamp_070791.bin", 0x10000, 0x10000, CRC(082d071c) SHA1(69af9a951f138146036b3c9ac3761cc6589b6cf5), ROM_BIOS(1))
	// 2: Dataexpert (Unknown model) - Chipset: VLSI VL82C311-25FC2 (SCAMPSX), HM6818P - BIOS: AMI 05/05/1991 on a 64KB "Fairchild FM27C512"
	// BIOS-String: 30-0100-D41107-00101111-050591-SCAMPSX-0 - SCAMP 386SX WITHOUT COMBO - Keyboard BIOS: 247076 - CPU: AM386SX/SXL-25 - OSC: 50.000MHz, 16.000MHz, 14.31818 - ISA16: 7
	ROM_SYSTEM_BIOS(2, "datax386sx", "Dataexpert 386sx")
	ROMX_LOAD( "bios.bin", 0x10000, 0x10000, CRC(0ba46059) SHA1(b152796e9ace0cd17c413df14d989b9cb23aa529), ROM_BIOS(2))
	// 3: VLSI 311 386SX VER 1.0 - CPU: AM386 SX/SXL-25 - Chipset: VPL201, VPL101 - BIOS: AMI 386sx 409425 - OSC: 50.000 MHz - ISA16: 6
	// BIOS-String: 30-0400-D41107-00101111-070791-SCMPSX-0
	ROM_SYSTEM_BIOS( 3, "vlsi311", "VLSI 311")
	ROMX_LOAD( "vlsi_311_386sx_ver_1.0_bios.bin", 0x10000, 0x10000, CRC(98056235) SHA1(3a3ff07808c4d43e4935c7463741e3ed8e515af9), ROM_BIOS(3))
	// 4: BIOS-String: 30-0100-ZZ1379-00101111-050591-SCAMPSX-0 / SCAMP 386SX - Chipset: (VLSI ???) 82C310, 82C311 - CPU: 386SX-25
	ROM_SYSTEM_BIOS(4, "scamp02", "VLSI SCAMPSX #2")
	ROMX_LOAD( "3vlm002.bin", 0x10000, 0x10000, CRC(4d2b27b3) SHA1(3c67d7bd507ceb4d1762866f69c2cb94cd799a15), ROM_BIOS(4))
ROM_END


/***** 386sx motherboards using the ALi 1217 chipset *****/

ROM_START( alim1217 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	//0: BIOS-String: 30-0100-ZZ1453-00101111-070791-ACER1217-0 / CPU: 386SX-40
	ROM_SYSTEM_BIOS( 0, "m121701", "ALi M1217 #1" )
	ROMX_LOAD( "3alm005.bin", 0x10000, 0x10000, CRC(8708727c) SHA1(9be25b1af080aee863441cf0d25d0f984accb086), ROM_BIOS(0))
	// 1: BIOS-String: 30-0100-D01131-00101111-070791-ALI1217-F
	ROM_SYSTEM_BIOS( 1, "m121702", "ALi M1217 #2" )
	ROMX_LOAD( "3alm006.bin", 0x10000, 0x10000, CRC(e448c436) SHA1(dd37127920a945f1273c70c41e79e4fc70a5de01), ROM_BIOS(1))
	// 2: BIOS-String: 30-0501-D81105-00101111-070791-ACER1217-0 - 386SX NPM/33,40-A0(2) 05/12/1993
	ROM_SYSTEM_BIOS( 2, "m919a00", "386SX NPM/33,40-A0" )
	ROMX_LOAD( "m919a00_npm-40.bin", 0x10000, 0x10000, CRC(4f330d82) SHA1(08224c7bcfb2a859b682bf44ac1ac7fd9f2ade78),ROM_BIOS(2))
	// 3: GMB-386SAT - Am386SX-40, IIT 3C87SX-33 - flashing "K/B controller incorrect" - Chipset: ALi M1217-40 - ISA8: 1, ISA16: 5
	// BIOS: AMI 386SX BIOS AA1280569 - BIOS-String: 30-0100-428036-00101111-111192-ALI1217-F - Keyboard-BIOS: JETkey V5
	ROM_SYSTEM_BIOS( 3, "gmb386sat", "GMB-386SAT_V1.0" )
	ROMX_LOAD( "gmb-386sat_v1.0.bin", 0x10000, 0x10000, CRC(59ecc773) SHA1(f2007fce76b3a91f51bfb5f43c1539d5ae06d35f), ROM_BIOS(3))
	// 4: ML-765 V2 - BIOS: AMI AA0508210 - BIOS-String: 30-0103-DJ1113-00101111-070791-ACER1217-0
	ROM_SYSTEM_BIOS( 4, "ml765", "ML-765" )
	ROMX_LOAD( "3alm011.bin", 0x10000, 0x10000, CRC(27a799d4) SHA1(873cf5968923c5a655ff32f3d968b7cddcb08e76), ROM_BIOS(4))
	// 5: BIOS: AA0030659 - BIOS-String: 30-0100-428029-00101111-070791-ACER1217-0
	ROM_SYSTEM_BIOS( 5, "m121703", "ALi M1217 #3" )
	ROMX_LOAD( "3alm012.bin", 0x10000, 0x10000, CRC(5b822a2a) SHA1(e61b27f06cfec54973fbabff277bde617847b1e2), ROM_BIOS(5))
	// 6: 303N1 - Chipset: ALi M1217, M5818 - BIOS: MR BIOS MR001-SX V.1.41 - Keyboard BIOS: JETkey V3.0 - CPU: i386sx, FPU socket provided
	// RAM: 4xSIMM30 - OSC: 80.000MHz - ISA8: 1, ISA16: 5
	ROM_SYSTEM_BIOS( 6, "acer310", "Acer 310" )
	ROMX_LOAD( "3alr001.bin", 0x10000, 0x10000, CRC(b45e5c73) SHA1(81ef79faed3914ccff23b3da5e831d7a99626538), ROM_BIOS(6))
	// 7: 8517 V3.2 - Chipset : ALI M1217-40, M5818 A1 - CPU: AMD Am386SX-40, FPU: ULSI Advanced Math Coprocessor SX/SLC US83S87 - BIOS : MR 386SX BIOS V.1.40 SX M1217
	// BIOS-String: - Keyboard-BIOS: JETkey V3.0 - OSC: 80.000MHz - ISA8: 1, ISA16: 5
	ROM_SYSTEM_BIOS( 7, "8517v32", "8517 V3.2" ) // MR BIOS (r) V1.40
	ROMX_LOAD( "m1217mrb.bin", 0x10000, 0x10000, CRC(8ac66b9d) SHA1(909e5129066d1b0563b03c4834f9894c9291c287), ROM_BIOS(7))
ROM_END

// Computechnik ASC486SLC, 386sx slot CPU - Chipset: ALi M1217-40, PIC P4020 - CPU: TI 486SLC/E -33P AF - RAM: 4xSIMM30
// OSC: 66.000 - BIOS: AMI 386SX BIOS AA1330085 - Keyboard-BIOS: AMI MEGAKEY - On board: 2xser, par, Floppy, IDE
// BIOS-String: 30-0100-001588-00101111-111192-ALI1217-0 / A-03
ROM_START( asc486slc )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD("asc486slc.bin", 0x10000, 0x10000, CRC(aadbd3b6) SHA1(635df5b3cfd4525bc8ad9067703c8860dacd987a))
ROM_END

// Manufacturer/Identifier: ECS 8517 v3.3 - Chipset: ALI-M1217-40, M5818 A1 9347 TS 10 A45296 - CPU: 386sx, FPU socket provided
// RAM: 4xSIMM30 - BIOS Version: AMI 386sx BIOS AA1021639 - Keyboard BIOS: JETkey V5 - BIOS String: 40-0000-zz1218-00101111-070791-ALI1217-0
// Board marking: 8517 V3.3 and 200-03690-341-4 - OSC: 80.000MHz - ISA8: 1, ISA16: 5
ROM_START( ecs8517 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD("8517_v33.bio", 0x10000, 0x10000, CRC(c2b18ace) SHA1(fb10108a8d7a4782442f7a518e0ebab01f2e54bd))
ROM_END


/***** 386sx motherboards using the Headland HT18/C chipset *****/

// moved here from 286, original comment: not a bad dump, sets unknown probably chipset related registers at 0x1e8 before failing post
ROM_START( ht18c )
	ROM_REGION16_LE(0x20000, "bios", 0)
	// 0: (BIOS release date:: 07-07-1991) - Chipset: Headland HT18/C
	ROM_SYSTEM_BIOS(0, "ami181", "AMI HT 18.1")
	ROMX_LOAD( "ht18.bin", 0x10000, 0x10000, CRC(f65a6f9a) SHA1(7dfdf7d243f9f645165dc009c5097dd515f86fbb), ROM_BIOS(0) )
	// 1: CPU: 386SX-25 - BIOS: AMI; 12/12/91
	ROM_SYSTEM_BIOS(1, "ami182", "AMI HT 18.2")
	ROMX_LOAD( "3hlm001.bin", 0x10000, 0x10000, CRC(b1434d6f) SHA1(1863dd60ad2b494141b4b30fe7b02f454bec82a3), ROM_BIOS(1) )
	// 2: CPU: 386SX-25 - BIOS: AMI; 07/07/91
	ROM_SYSTEM_BIOS(2, "ami183", "AMI HT 18.3")
	ROMX_LOAD( "3hlm002.bin", 0x10000, 0x10000, CRC(10a78d11) SHA1(0500d92e2691164bdc5c71b3d6fd0a154f7279d4), ROM_BIOS(2) )
	// 3: CPU: 386SX-25 - BIOS: AMI; 04/30/91
	// BIOS-String: 30-01]1-ZZ1372-00101111-0403091-HT18SX-0
	ROM_SYSTEM_BIOS( 3, "ami184", "AMI HT 18.4") // marked as BAD_DUMP for the "]" in the BIOS string ... and because it actually runs :)
	ROMX_LOAD( "3hlm003.bin", 0x10000, 0x10000, BAD_DUMP CRC(50f7a543) SHA1(8962f7ce2fc5c60059894cae04cf5fccd6cee279), ROM_BIOS(3) )
	// 4: MBA-025 - Chipset: Headland HT18/B, HM6818A - BIOS: AMI 386SX BIOS PLUS T.B 238958 - BIOS-String: 30-0100-009999-00101111-043091-HT18SX-0
	// Keyboard-BIOS: AMI Keyboard BIOS PLUS T.B. 238958 - CPU: AMD AM386 SX/SXL-25 - FPU: empty socket - OSC: 32.000 MHz - 50.000 MHz - 14.31818 - ISA8: 1, ISA16: 6
	ROM_SYSTEM_BIOS( 4, "mba025", "MBA-025" )
	ROMX_LOAD( "386-mba-025-low_32k.bin", 0x10000, 0x8000, CRC(4df55219) SHA1(7dc1adb130ae8c3c88e2c58bde6e3b793fa0c78e), ROM_SKIP(1) | ROM_BIOS(4))
	ROMX_LOAD( "386-mba-025-high_32k.bin", 0x10001, 0x8000, CRC(0406fdc9) SHA1(ee21d584c98b0c11ec2cfb609de83c38b0a893c7), ROM_SKIP(1) | ROM_BIOS(4))
ROM_END


/***** 386sx motherboards using the Opti F82C206, Opti 82C283 chipset *****/

ROM_START( op82c283 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	// 0: Chipset: Opti F82C206, Opti 82C283 - BIOS: ARMAS AMI 386SX BIOS PLUS 9014775 - Keyboard-BIOS: NEC D80C42C
	// BIOS-String: 30-013X-D21185-00001111-031591-OPSX-0 - OSC: 50.000MHz, 14.31818MHz - CPU: AM386 SX/SXL-25 - ISA8: 2, ISA16: 6
	ROM_SYSTEM_BIOS( 0, "armas", "386sx ARMAS" )
	ROMX_LOAD( "386-opti-armas.bin", 0x10000, 0x10000, CRC(d9c696bc) SHA1(467617ab4a211ce460766daa3e5803e190368703), ROM_BIOS(0))
	// 1: 386SX MAIN BOARD REV:A1-1M/N: 3805 - Chipset: OPTi F82C206 / OPTi (unreadable) - BIOS:AMI 386SX BIOS ZZ908380
	// BIOS-String: 30-0100-DG112-00101111-031591-OPSX-0 / A10001B / 128KB RESERVED FOR RAM SHADOW.
	// Keyboard-BIOS: AMI KB-BIOS-VER-F (Intel P8942AHP) - CPU: AM386 SX/SXL-25 - OSC: 14.31818MHz, 50.000MHz - ISA8: 2, ISA16: 6
	ROM_SYSTEM_BIOS( 1, "3805", "386sx 3805" )
	ROMX_LOAD( "386sx-opti-908380.bin", 0x10000, 0x10000, CRC(38502567) SHA1(d65d272aa60642197c9b639a8679f8f41c4a697b), ROM_BIOS(1))
	// 2: CPU: 386SX-20 - BIOS: AMI; 03/15/91 - no display - unknown ASI motherboard
	// BIOS-String: 30-0100-DG1112-00101111-031591-OPSX
	ROM_SYSTEM_BIOS( 2, "c28301", "OPTi 82C283 #1")
	ROMX_LOAD( "3opm010.bin", 0x10000, 0x10000, CRC(7c2acf57) SHA1(d40605621da40204dc6370d2d00b783b3a7f8dce), ROM_BIOS(2))
ROM_END

// Octek Panther II - Chipset: OPTi 82C283, F82C206L/Chips 82C206 - CPU: 386sx - BIOS: AMI 386sx BIOS - Keyboard-BIOS: Intel/AMI
// BIOS-String: 30-0200-420000-00101111-050591-OPSX-0 / OPSX 386SX PAGE INTERLEAVE BIOS - ISA8: 2, ISA16: 4
ROM_START( ocpanii )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "octek_panther_ii_386sx.bin", 0x10000, 0x10000, CRC(866192d5) SHA1(fe6133ee3ba0d71c0d4690a0843ca82106effcf6))
ROM_END

// RYC Alaris LEOPARD LX REV D - Chipset: Opti 82C283 82C206Q - CPU: 486SLC2 (IBM 14 Q) - ISA16: 7
// BIOS: AMI 486SLC ISA BIOS AA0735388 - Keyboard-BIOS: Intel/AMI MEGA-KB-H-WP
// BIOS-String: 30-0100-006328-00101111-060692-OPSXPI-0
ROM_START( alaleolx )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "486-ryc-leopard-lx.bin", 0x10000, 0x10000, CRC(bbc7bfd2) SHA1(49833b482efb8e361be88f48e252621b147a3b1b))
ROM_END

/***** 386sx motherboards using the OPTi 82C291 chipset *****/

ROM_START( op82c291 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	// 0: MR BIOS (r) V1.43
	ROM_SYSTEM_BIOS(0, "mr", "MR")
	ROMX_LOAD( "opti_82c291_mr-bios_v143.bin", 0x10000, 0x10000, CRC(f7989a65) SHA1(cc729b6baac486ac3116f08e78eb58bb39365dd5), ROM_BIOS(0))
	// 1: CPU: 386SX-40 - BIOS: AMI - no display, nine beeps, so probably bad dump
	ROM_SYSTEM_BIOS( 1, "ami", "AMI")
	ROMX_LOAD( "3opm007.bin", 0x10000, 0x10000, CRC(eed82365) SHA1(45f5a608740d161c5a74415ff3f7b573d7e61f58), ROM_BIOS(1))

ROM_END

// DTK Computer PPM-3333P - Chipset: OPTi 82C291 - Opti F82C206 - ISA16: 6 - CPU: AMD Am386SX/SXL-33, FPU: empty socket - OSC: 14.31818 - 66.0000 MHz
ROM_START( ppm3333p )
	ROM_REGION16_LE(0x20000, "bios", 0)
	//0: Award Modular BIOS v4.20 (80386DX) / (119U906X) DTK Computer
	ROM_SYSTEM_BIOS(0, "ppmawa", "PPM-3333P Award")
	ROMX_LOAD( "386sx_opti291-award.bin", 0x10000, 0x10000, CRC(4855b394) SHA1(94dd1a38852eecac538ef4b8bf04bb7c1e4317d2), ROM_BIOS(0))
	//1: BIOS-String: 30-0200-001107-00001111-121291-OPTX 291-0 / OPTI-291WB BIOS VER 1.2
	ROM_SYSTEM_BIOS(1, "ppmami", "PPM-3333P AMI")
	ROMX_LOAD( "386sx_opti291-ami.bin", 0x10000, 0x10000, CRC(35727f8f) SHA1(3fb14cd6ea0d7a2bd545beb1586403cc36a77668), ROM_BIOS(1))
ROM_END


/***** 386sx motherboards using the SARC (or CYCLONE) RC2016A5 chipset *****/

// Pine PT-319A rev2.2a - CPU: 386sx - BIOS: AMI; 06/06/92
// BIOS-String: X0-0100-000000-00101111-060692-386SX-0 / Ver 5.20
ROM_START( pt319a )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "3sam001.bin", 0x10000, 0x10000, CRC(cad22030) SHA1(85bb6027579a87bfe7ea0f7df3676fdaa64920ac))
ROM_END


// CX Technology, Inc. Model SXD (4x SIMM30, Cache: 64/128/256KB in 2 banks plus TAG, 4x 16-bit ISA) - Chipset: SARC RC2016A5; HM6818P; CX109; LT38C41 © Lance Corp. (keyboard controller?)
// additional info from chukaev.ru54.com: Chipset: CYCLONE RC2016A5 - ISA16: 6 - ROM: CX109 340C3A62D0A - CPU/FPU: Am386SX/SXL-33, 387
ROM_START( cxsxd )  // BIOS-String: 03/25/93-SARC_RC2016A-219v0000 / CX 386SX System
	ROM_REGION16_LE(0x20000,"bios", 0)
	ROM_LOAD( "award_cx-sxd_v4.50.srd.bin", 0x10000, 0x10000, CRC(ef1c74d7) SHA1(b40b1cb7143c4e352798bdf3b488d9342a4029a7))
ROM_END

// PC-Chips M396F VER 2.2 - CPU: 386SX, 387SX - ISA16: 6
ROM_START( pccm396f )
	ROM_REGION16_LE(0x20000, "bios", 0)
	// 0: Chipset: PCCHIPS CHIP2 310, HT6542, HM6818A
	// BIOS-String: X0-0100-001437-00101111-060692-M396C-0 - BIOS: AMI 386SX BIOS Ver. 2.10 C-1216
	ROM_SYSTEM_BIOS(0, "chips2", "Chips 2")
	ROMX_LOAD( "3pcm003.bin", 0x10000, 0x10000, CRC(b7fc6737) SHA1(670e38b628cb71dc09742f097349ac48ccf28696), ROM_BIOS(0))
	// 1: Chipset: SARC RC2016A5 - CPU: 386SX-40/486SLC, 387SX - BIOS: AMI; 06/06/92
	// BIOS-String: X0-0100-001437-00101111-060692-M396F-0
	ROM_SYSTEM_BIOS(1, "sarc01", "SARC RC2016A5 #1")
	ROMX_LOAD( "3sam002.bin", 0x10000, 0x10000, CRC(8d5ef8e8) SHA1(5ca2b36d5bee2870f894984909aa2013b5c4d6cf), ROM_BIOS(1))
	// 2: BIOS-String: X0-0100-001437-00101111-060692-M396F-0 - CPU: 386SX-40 (ALI M1386SX A1P)
	ROM_SYSTEM_BIOS(2, "sarc02", "SARC RC2016A5 #2")
	ROMX_LOAD( "3sam003.bin", 0x10000, 0x10000, CRC(95ea08d8) SHA1(812e8488ad63ca24250e245a2f0273f1d1703fc3), ROM_BIOS(2))
ROM_END


/**************************************************************************
  80386 SX Desktop
**************************************************************************/

// Commodore SL 386SX
ROM_START( c386sx16 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	// actual VGA BIOS not dumped - uses a WD Paradise according to http://www.cbmhardware.de/pc/pc.php
	// complains "Time-of-day clock stopped"
	// photos of the system show one ISA16 slot for a riser card, an Acumos AVGA2 chip, a VLSI 82C311 IC, one other VLSI and an Acer chip.
	ROM_SYSTEM_BIOS(0, "c386sxv100", "SL 386SX V1.00") // Commodore 80386SX BIOS Rev. 1.00 - 390914-01/390915-01 - continuous beeps after POST
	ROMX_LOAD( "cbm-sl386sx-bios-lo-v1.0-390914-01.bin", 0x10000, 0x8000, CRC(03e00583) SHA1(8be8478cabd9de3d547a08207ffdcd39bf1bcd94), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "cbm-sl386sx-bios-hi-v1.0-390915-01.bin", 0x10001, 0x8000, CRC(cbe31594) SHA1(d6ace0b5ae4a0f63d047c2918210188f4c77c0c0), ROM_SKIP(1) | ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "c386sxv101", "SL 386SX V1.01") // Rev. 1.01 - 390914-02/390915-02 - continuous beeps after POST
	ROMX_LOAD( "cbm-sl386sx-bios-lo-v1.01-390914-02-2700.bin", 0x10000, 0x8000, CRC(711f1523) SHA1(5318127cd42e60dabd221ae8dd16812726a0e889), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "cbm-sl386sx-bios-hi-v1.01-390915-02-3b00.bin", 0x10001, 0x8000, CRC(a1390cbc) SHA1(12aef4b95581e8c4489036c75697f18e9f3727b5), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "c386sxv102", "SL 386SX V1.02") // Rev. 1.02 - 390914-03/390914-03/390915-03
	ROMX_LOAD( "cbm-sl386sx-bios-lo-v1.02-390914-03-0300.bin", 0x10000, 0x8000, CRC(301eb832) SHA1(6c599792b254b6d98dc130040d4f7858fd504f15), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "cbm-sl386sx-bios-hi-v1.02-390915-03-3800.bin", 0x10001, 0x8000, CRC(01815d9d) SHA1(0af291626e71ed65ff6dfee2fe4776a29f2bbb97), ROM_SKIP(1) | ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "c386sxv103", "SL 386SX V1.03") // Commodore 80386SX BIOS Rev. 1.03 -
	// this was replaced with the consistently named ROMs from http://www.zimmers.net/cbmpics/cpcs3.html, the 'hi' ROM looks like a bad dump, with its alternative the POST comes up
	// ROMX_LOAD( "390914-01.u39", 0x10000, 0x8000, CRC(8f849198) SHA1(550b04bac0d0807d6e95ec25391a81272779b41b), ROM_SKIP(1) | ROM_BIOS(3)) /* 390914-01 V1.03 CS-2100 U39 Copyright (C) 1990 CBM */
	// ROMX_LOAD( "390915-01.u38", 0x10001, 0x8000, CRC(ee4bad92) SHA1(6e02ef97a7ce336485814c06a1693bc099ce5cfb), ROM_SKIP(1) | ROM_BIOS(3)) /* 390915-01 V1.03 CS-2100 U38 Copyright (C) 1990 CBM */
	ROMX_LOAD( "cbm-sl386sx-bios-lo-v1.03-390914-03.bin", 0x10000, 0x8000, CRC(8f849198) SHA1(550b04bac0d0807d6e95ec25391a81272779b41b), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD( "cbm-sl386sx-bios-hi-v1.03-390915-03.bin", 0x10001, 0x8000, CRC(ebdd5097) SHA1(2e4d2375efb9c1ebc0ccf3bb1ff2bb64c449af32), ROM_SKIP(1) | ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "c386sxv104", "SL 386SX V1.04") // Rev. 1.04 - 390914-04/390915-04
	ROMX_LOAD( "cbm-sl386sx-bios-lo-v1.04-390914-04.bin", 0x10000, 0x8000, CRC(377a8e1c) SHA1(9a36f10ad496e44f190937426f3e7de368d6ab7b), ROM_SKIP(1) | ROM_BIOS(4))
	ROMX_LOAD( "cbm-sl386sx-bios-hi-v1.04-390915-04.bin", 0x10001, 0x8000, CRC(4149f5d9) SHA1(9a62b235ac45145ca6720d11b2cbc17b8c25704a), ROM_SKIP(1) | ROM_BIOS(4))
ROM_END

// Commodore 386SX-25 - Form factor: slimline desktop - Chipset: : VLSI 82C311, unreadable, Acer - CPU: i386sx-25, FPU socket provided - On board: Keyboard (DIN), mouse (mini DIN), par,
// 2xser, beeper, Floppy, IDE - VGA on board: acumos AVGA2-340-1 - Mass storage: 3.5" FDD, 80MB IDE HDD - RAM: 8xSIMM30 - OSC: 24.000MHz, 50.000MHz, unreadable - ISA16: 1 (for riser card)
ROM_START( c386sx25 ) // BIOS-String: Award Modular BIOS v4.20 (80386DX) Commodore 386sx-25 BIOS-Version 1.09 391443-09
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "c386sx25_f000.rom", 0x10000, 0x10000, CRC(1322064d) SHA1(6ce16a956b8454d3746ccb923fac45924e4302a4))

	ROM_REGION( 0x8000, "vga", 0)
	ROM_LOAD( "c386sx25_c000.rom", 0x0000, 0x8000, CRC(92212c6b) SHA1(61d222872bf4e21de053705c267103e409adb0ab))
ROM_END

// Commodore PC 50-II - a photo of the mainboard shows four ROMs (two each for BIOS and VGA), so the 128K dumps available were probably made from a running system.
ROM_START( pc50ii ) // Chipset: Chips P82C211-12 C(16MHz), P82C212B-12 (16MHz), P82C215-12, P82C206, VLSI 8942VT - ISA8: 1, ISA16: 5
	ROM_REGION16_LE(0x20000, "bios", 0) // keyboard MCU is P8042AH MITAC V2.48 (undumped), onboard video PVGA1A-JK
	// 0: Commodore PC50-II BIOS Rev1.0 - 609200-03
	ROM_SYSTEM_BIOS(0, "pc50iiv100", "PC 50-II V1.00") // complains "Time-of-day clock stopped" and reboots
	ROMX_LOAD( "cbm-pc50b-bios-lo-v1.00-390339-01.bin", 0x10001, 0x8000, CRC(0f0e2fd6) SHA1(61a8043ac919c2a8fe668bf25e5f0b67868d11ae),ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "cbm-pc50b-bios-hi-v1.00-390340-01.bin", 0x10000, 0x8000, CRC(87008421) SHA1(cf41973a7bd439441baec1138dd63044fafe7391),ROM_SKIP(1) | ROM_BIOS(0) )
	// 1: Commodore PC50-II BIOS Rev1.01 - 609200-03
	ROM_SYSTEM_BIOS(1, "pc50iiv101", "PC 50-II V1.01") // same behaviour as above
	ROMX_LOAD( "cbm-pc50b-bios-lo-u31-v1.01-xxxxxx-xx-a800.bin", 0x10001, 0x8000, CRC(bf2c7009) SHA1(6b94df37861b30ef6a39a4ed64d4c9ac1e96043a),ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "cbm-pc50b-bios-hi-u28-v1.01-xxxxxx-xx-cd00.bin",  0x10000, 0x8000, CRC(628fcb2f) SHA1(74241cbcb4e183015d5e7a516d46b08d6f47504a),ROM_SKIP(1) | ROM_BIOS(1) )
	// 2: Commodore PC50-II BIOS Rev1.02 - 609200-03
	ROM_SYSTEM_BIOS(2, "pc50iiv102", "PC 50-II V1.02") // same behaviour as above
	ROMX_LOAD( "cbm-pc50b-bios-lo-u32-v1.02-609200-03o-9e00.bin", 0x10001, 0x8000, CRC(57225c22) SHA1(3b2ded119480ce2dd5bb7c113c5814ce47e17d4c),ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "cbm-pc50b-bios-hi-u27-v1.02-609200-03e-c400.bin", 0x10000, 0x8000, CRC(4ec903af) SHA1(fb70e22c0538d7310c9034626d4d9c0e4f63dfd7),ROM_SKIP(1) | ROM_BIOS(2))

	// VGA BIOS
	// ROM_LOAD( "m_pc50-ii_1bad_pvgadk_odd.bin", 0x00000, 0x8000, CRC(f36eca7e) SHA1(4335fa4a4567cbc010ff2ffeb97a536ed93b0219))
	// ROM_LOAD( "m_pc50-ii_54e3_pvgadk_even.bin", 0x00001, 0x8000, CRC(01f6b964) SHA1(799a84ddde8a7672a6df9439bad6198ec3ff98ec))
ROM_END

// Schneider 386SX VGA System 40 (the number indicates the size of the harddisk, there were System 70 as well) - uses the same case as the Schneider Tower AT
// Schneider Tower VGA I/O: Chipset: WD37C65BJM, BIGJIM 50773 1108-0056, two other bigger chips can't be read on the photos
// 104 pin CPU card connector (ISA without the key), 4xISA16, 1xISA8 - on board: IDE, parallel, serial, bus mouse (Atari compatible), VGA, internal floppy (26pin), external floppy (DB25)
// On board graphics: ATI VGA Wonder-16 (256KB), ATI18800-1 1138-0069
// CPU card: CPU: Intel NG680386SX-16 (C-Step), FPU socket provided - Chipset: DDA14-075E, Chips P82C812, P82C811, P82C206, P82C215-12 (16MHz) - RAM: 8xSIMM30
// OSC: 20.000, 14.31818, 24.000000MHz, 32.000000MHz, - keyboard
// beeps 1-2-4
ROM_START( tower386sx )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "v103", "V1.03") // from a 386SX System 70
	ROMX_LOAD("t386s103.bin", 0x10000, 0x10000, CRC(d4e177e6) SHA1(fa11d49d629cdcac4467a9deedd25171ae499346), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v100", "V1.00") // from a 386SX System 40
	ROMX_LOAD("schneider_ag_386sx_bios_1_version_1.00a_id.nr.52504.u16", 0x10000, 0x8000, CRC(2fec2d3a) SHA1(4227da07f6652b89b9d02d7570ad0476672fd80d), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("schneider_ag_386sx_bios_0_version_1.00a_id.nr.52504.u15", 0x10001, 0x8000, CRC(b3331429) SHA1(b214bccfb62add9caea3d734885bc945b868967a), ROM_SKIP(1) |ROM_BIOS(1))

	// models upgraded to 512KB video memory were sold as "CEG" models as the memory upgrade enabled some sort of antialiasing ("continuous edge graphics")
	// in a 256 color mode with a choice from 792.096 colors.
	// according to https://archive.org/stream/byte-magazine-1991-01/1991_01_BYTE_16-01_1990_BYTE_Award_of_Excellence#page/n197/mode/2up this needs an EDSUN D/A chip, it is unknown
	// if it's contained on the platter or on the graphics upgrade piggyback card
	// The 12.5MHz version of the towerat2xx (VGA Tower System 40 or 70) used the same I/O backplane and were also offered with the CEG upgrade.
	ROM_REGION16_LE(0x10000, "vga", 0)
	ROM_LOAD16_BYTE("schneider_ag_vga_bios_low_v1.00_id.nr_51368.u13", 0x0000, 0x8000, CRC(ec4ef170) SHA1(0049ae5eab1a21838e674cf77e88994b954b1da3))
	ROM_LOAD16_BYTE("schneider_ag_vga_bios_high_v1.00_id.nr_51368.u14", 0x0001, 0x8000, CRC(5354962a) SHA1(11a503473e2011f323cc81c0b63d24f231c54c31))
ROM_END

// Datavan Book-Size LAN Station - CPU: Am386SX/SXL-25 - Chipset: Headland HT18/C, RAM: 4xSIMM30
// BIOS: AMI 386SX BIOS PLUS S/NO. 232659 - Keyboard-BIOS: AMI KEYBOARD BIOS PLUS S/NO. 232659
// BIOS-String: 30-0100-ZZ1612-00101111-070791-HT18-F - ISA16: 1, used for LAN board - on board: 2xser, par, VGA
// Video: Realtek RTG3106 VGA, 1024K, MUSIC TR9C1710-66PCA - VGA BIOS: REALTEK / QUADTEL
// Mass storage: NEC FD1138H,  Quantum ProDrive ELS 977/10/17 - unknown DP8390DN based 8bit ISA LAN card
ROM_START( dvbslan ) // 3 short beeps =>  base memory read/write test error in the first 64 KB block of memory
	ROM_REGION16_LE( 0x20000, "bios", 0)
	ROM_LOAD( "ami_bios_386sx_bios_plus.bin", 0x10000, 0x10000, CRC(760afd89) SHA1(452f3e1b8b3b3d00df90c3501c8796de447a2184))

	ROM_REGION( 0x8000, "vga", 0) // REALTEK VGA BIOS
	ROM_LOAD( "realtek_vga.bin", 0x0000, 0x8000, CRC(4e416975) SHA1(826b7ec5494dc83cfcf25922185777bf6db46949))

	ROM_REGION( 0x2000, "lan", 0) // boot ROM from DP8390DN based LAN adapter
	ROM_LOAD( "net_isa_boot.bin", 0x0000, 0x2000, CRC(5bb527fd) SHA1(1eb7c82ef99d64eedffc3ac9c145d33b06cd8cb6))
ROM_END

// Epson PC AX3 - see epsax
ROM_START( epsax3 )
	ROM_REGION16_LE( 0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "evax3.bin", 0x10000, 0x8000, CRC(fec03cdc) SHA1(549027e6b786fa98a14912c1fb6d44c607206253))
	ROM_LOAD16_BYTE( "odax3.bin", 0x10001, 0x8000, CRC(0162d959) SHA1(f17741282e078a66e38d82ca48455b17c54e832f))
ROM_END

// Sanyo MBC-28
// Links: http://www.cc-computerarchiv.de/CC-Archiv/bc-alt/gb-san/gb-san-12_91.html
// Form factor: Desktop - CPU: 80386sx-20 - RAM: 1MB - 8MB on board - Mass storage: 1.44MB Floppy disk drive and 80MB IDE hard disk
// On board: 2xserial, parallel, bus mouse, keyboard - To-Do: Complains about missing mouse hardware (Bus Mouse), hangs in POST
ROM_START( mbc28 )
	ROM_REGION16_LE(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "mbc-28_sl-dt_ver.1620_low_din_checksum,454f00,27c256-15.bin", 0x10000, 0x8000, CRC(423b4693) SHA1(08e877baa59ebd9a1817dcdd27138c638edcbb84) )
	ROM_LOAD16_BYTE( "mbc-28_sl-dt_ver.1620_high_din_checksum,45ae00,27c256-15.bin", 0x10001, 0x8000, CRC(557b7346) SHA1(c0dca88627f8451211172441fefb4020839fb87f) )
ROM_END

/**************************************************************************
  80386 SX Laptop/Notebook
**************************************************************************/

// Commodore Laptop C386SX-LT -  screen remains blank
ROM_START( c386sxlt )
	ROM_REGION16_LE(0x20000, "bios", 0) // BIOS contains Cirrus Logic VGA firmware, rebadged Sanyo MBC-18NB, but different versions exist
	ROM_SYSTEM_BIOS(0, "c386sxlt_b400", "C386SX-LT V1.2 B400")
	ROMX_LOAD( "cbm-386lt-bios-v1.2-390981-03-b400.bin", 0x00000, 0x20000, CRC(b84f6883) SHA1(3f31060726c7c49a891b35ab024524a4239eb4d0), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "c386sxlt_cf00", "C386SX-LT V1.2 CF00")
	ROMX_LOAD( "cbm-386lt-bios-v1.2-390982-03-cf00.bin", 0x00000, 0x20000, CRC(c8cd2641) SHA1(18e55bff494c42389dfb445f2bc11e78db30e5f7), ROM_BIOS(1))
ROM_END

// Toshiba T3200SXC - CPU: 80386sx-20 - Floppy - 120 MB Hard disk - 1 MB RAM on board, 4 MB extra memory already installed. Can be upgraded to a total of 13 MB.
// TFT Display, 640x480, 256 colors - WD 90C21 video chip, 256 KB RAM - ISA8: 1, ISA16: 1
ROM_START( tot3200sxc ) // KBC ERROR
	ROM_REGION16_LE( 0x20000, "bios", 0)
	ROM_LOAD( "012c_t3200sxc_tc57h1024d-85.bin", 0x00000, 0x20000, CRC(5804a3da) SHA1(922dfb35b134a91a4c39e443597dad6798ce69d9))
ROM_END

// Sanyo MBC-18NB notebook - no display
ROM_START( mbc18nb )
	ROM_REGION16_LE( 0x20000, "bios", 0)
	ROM_LOAD( "sanyo_18nb.bin", 0x00000, 0x20000, CRC(64e283cf) SHA1(85ce4074c23b388d66e53cc83a8535bf7a2daf1f))
ROM_END

// Siemens-Nixdorf PCD-3Nsx notebook
// CPU: Intel NG680386SX-16 or -20 - 1MB RAM,upgradeable to 5MB, 40MB harddisk (-16 model)
// 2MB, upgradeable to 8MB, 80MB harddisk (-20 model), 9" mono VGA LCD, 1.44MB floppy drive
// Chipset: Headland HT21/E, , DP8473V, CHIPS F82C601, DS??87, unknown QFP100, ADC0833BCN (on PCU sub), CL-GD610-320C-C+CL-GD620-C
// Microcontrollers: N8042AH (KBC),  N80C51RH  (KBE)
// OSC: 14.318, 32.00024.0090
ROM_START( pcd3nsx )
	ROM_REGION16_LE( 0x20000, "bios", 0 )
	ROM_LOAD( "337-7_2400_3f_10-15-91.bin", 0x00000, 0x20000, CRC(99befce7) SHA1(150cd6a1476ca0ea970a1103b2a2c668c984433a) )

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD("33b_pcu_rev_3r.bin", 0x0000, 0x1000, CRC(d99308ec) SHA1(26621db2db37ab2bd1da972abb7398f9514329b2) )

	ROM_REGION( 0x800, "kbc", 0 )
	ROM_LOAD("kbc_c3f.bin", 0x000, 0x800, NO_DUMP)

	ROM_REGION( 0x1000, "kbe", 0 )
	ROM_LOAD("kbe_e3d.bin", 0x0000, 0x1000, NO_DUMP)
ROM_END

// Siemens-Nixdorf PCD-3Nsl - CPU: 80386SL@25MHz - 2MB RAM, upgradeable to 8MB - 85MB harddisk - 10" mono LCD VGA
// 1.44MB floppy drive
ROM_START( pcd3nsl )
	ROM_REGION16_LE( 0x20000, "bios", 0 )
	// Phoenix 80386 ROM BIOS PLUS Version 1.10.00 - failure at 100000-10FFFF - Resume memory backup failure
	ROM_SYSTEM_BIOS(0, "05-13-93", "05/13/93")
	ROMX_LOAD( "3n102l30.bin", 0x00000, 0x20000, CRC(02384c19) SHA1(552dc41b40272027e2b031187f8ab1e1513751b9), ROM_BIOS(0) )
	// Phoenix 80386 ROM BIOS PLUS Version 1.10.00 - Memory high address failure at 100000-10FFFF - Resume memory backup failure
	ROM_SYSTEM_BIOS(1, "01-26-94", "01/26/94")
	ROMX_LOAD( "3n120l40.bin", 0x00000, 0x20000, CRC(1336dd75) SHA1(80306d85f417c51a5235ac2f02ceb58bdb51205f), ROM_BIOS(1) )
ROM_END

// Toshiba T2000SX
// 1MB RAM on board, up to 9MB with 2MB, 4MB or 8MB expansion cards - 16 level grayscale VGA 640x480 display, PVGA1F display controller, 256KB VRAM
// Super integration (SI), components: DMAC 82C37Ax2, PIC 82C59Ax2, PIT 82C54, FDC TC8565, SIO TC8570 - 80C42 and 80C50 for keyboard - RTC 146818AF
// 128KB ROM, 32KB Backup RAM - GA-SYS CNT System control gate array - GA-IO CNT I/O gate array
ROM_START( t2000sx )
	ROM_REGION16_LE( 0x20000, "bios", 0 )
	ROM_LOAD( "014d.ic9", 0x00000, 0x20000, CRC(e9010b02) SHA1(75688fc8e222640fa22bcc90343c6966fe0da87f))
ROM_END

// Triumph-Adler Walkstation 386 SX - German version of the Olivetti S20
// VLSI VL82C320 + VL82C331; DP8473V
ROM_START( walk386sx )
	ROM_REGION16_LE( 0x20000, "bios", 0 ) // contains Cirrus Logic VGA BIOS
	ROM_LOAD( "cthj01_1014.bin", 0x00000, 0x20000, CRC(805084b9) SHA1(a92d78050844ccbcce53109c42603639aedd2335) )

	ROM_REGION( 0x2000, "mcu", 0 ) // MC68HC805B6FN
	ROM_LOAD( "cthj02_08_76.bin", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x1000, "cop888", 0 ) // COPCL888-RDT/V
	ROM_LOAD( "s9124ab_c4_e904-34162_00.bin", 0x0000, 0x1000, NO_DUMP )
ROM_END


/***************************************************************************
  Game driver(s)
***************************************************************************/

COMP( 199?, 386sxvhcom,ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "<unknown>",   "80386SX-VH-COM", MACHINE_NOT_WORKING )
COMP( 199?, 3siud,     ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "<unknown>",   "3SIUD-1.1", MACHINE_NOT_WORKING )
COMP( 199?, alaleolx,  ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "Alaris RYC", "LEOPARD LX", MACHINE_NOT_WORKING )
COMP( 199?, alim1217,  ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "<unknown>",   "386sx motherboards using the ALi M1217 chipset", MACHINE_NOT_WORKING )
COMP( 199?, anch386s,  ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "ANIX",        "CH-386S-16/20/25G", MACHINE_NOT_WORKING )
COMP( 199?, asc486slc, ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "Computechnik", "ASC486SLC", MACHINE_NOT_WORKING )
COMP( 1988, at386sx,   ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "<generic>",   "PC/AT 386SX (16 MHz, MF2 Keyboard)", MACHINE_NOT_WORKING )
COMP( 1990, c386sx16,  ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "Commodore Business Machines", "386SX-16", MACHINE_NOT_WORKING )
COMP( 1990, c386sx25,  ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "Commodore Business Machines", "386SX-25", MACHINE_NOT_WORKING )
COMP( 1991, c386sxlt,  ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "Commodore Business Machines",  "Laptop C386SX-LT", MACHINE_NOT_WORKING )
COMP( 1988, ct386sx,   ibm5170, 0,       ct386sx,   0,     at386sx_state,     init_at386sx,        "<generic>",   "NEAT 386SX (16 MHz, MF2 Keyboard)", MACHINE_NOT_WORKING )
COMP( 1993, cxsxd,     ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "CX Technology", "CX SXD", MACHINE_NOT_WORKING )
COMP( 199?, dfi386sx,  ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "Diamond Flower International", "386SX-16/20CN Rev 1.0", MACHINE_NOT_WORKING )
COMP( 199?, dvbslan,   ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "Datavan", "Book-Size LAN station", MACHINE_NOT_WORKING )
COMP( 199?, ecs8517,   ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "Elitegroup", "ECS 8517 v3.3", MACHINE_NOT_WORKING )
COMP( 199?, elt386sx,  ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "Elitegroup",  "ELT-386SX-160BE", MACHINE_NOT_WORKING )
COMP( 198?, eltp9,     ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "Chaintech", "ELT-P9 / Most likely ELT-386SX-160D", MACHINE_NOT_WORKING )
COMP( 198?, epsax3,    ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "Epson",       "PC AX3", MACHINE_NOT_WORKING )
COMP( 19??, ht18c,     ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "<unknown>",   "unknown 386sx AT clones (HT18/C chipset)", MACHINE_NOT_WORKING )
COMP( 199?, ibm2121,   ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "International Business Machines",  "PS/1 2121", MACHINE_NOT_WORKING )
COMP( 199?, ibm2121rd, ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "International Business Machines",  "PS/1 2121 (international models with ROM DOS)", MACHINE_NOT_WORKING )
COMP( 199?, ibm2123,   ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "International Business Machines",  "PS/1 2123", MACHINE_NOT_WORKING )
COMP( 199?, ilm396b,   ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "ILON USA, INC.", "M-396B", MACHINE_NOT_WORKING )
COMP( 198?, ktx20t02,  ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "Quadtel",     "QTC-SXM KT X20T02/HI Rev.3", MACHINE_NOT_WORKING )
COMP( 199?, mbc18nb,   ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "Sanyo",       "MBC-18NB", MACHINE_NOT_WORKING )
COMP( 1992, mbc28,     ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "Sanyo",       "MBC-28", MACHINE_NOT_WORKING ) // Complains about missing mouse hardware
COMP( 199?, mmbo4088,  ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "<unknown>",   "486MMBO4088 (TI TX486SLC/E", MACHINE_NOT_WORKING )
COMP( 199?, mokp386sx, ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "Morse",       "KP 386SX V2.21", MACHINE_NOT_WORKING )
COMP( 199?, ocpanii,   ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "Octek",       "Panther II", MACHINE_NOT_WORKING )
COMP( 199?, op82c283,  ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "<unknown>",   "386sx motherboards using the OPTi 82C283 chipset", MACHINE_NOT_WORKING )
COMP( 199?, op82c291,  ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "<unknown>",   "386sx motherboards using the OPTi 82C291 chipset", MACHINE_NOT_WORKING )
COMP( 199?, p386sx25pw,ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "Prolink",  "P386SX-25PW VER:2.00", MACHINE_NOT_WORKING )
COMP( 198?, pc50ii,    ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "Commodore Business Machines",  "PC 50-II", MACHINE_NOT_WORKING )
COMP( 198?, pcb303,    ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "Packard Bell", "PCB-303 Rev.01", MACHINE_NOT_WORKING )
COMP( 199?, pccm396f,  ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "PC-Chips",    "M396F", MACHINE_NOT_WORKING )
COMP( 199?, pcd3nsl,   ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "Siemens-Nixdorf", "PCD-3Nsl Notebook Computer", MACHINE_NOT_WORKING )
COMP( 199?, pcd3nsx,   ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "Siemens-Nixdorf", "PCD-3Nsx Notebook Computer", MACHINE_NOT_WORKING )
COMP( 199?, php3239,   ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "Philips",     "P3239", MACHINE_NOT_WORKING )
COMP( 199?, ppm3333p,  ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "DTK Computer", "PPM-3333P", MACHINE_NOT_WORKING )
COMP( 199?, pt319a,    ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "Pine",        "PT-319A", MACHINE_NOT_WORKING )
COMP( 199?, scamp386sx,ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "<unknown>",   "386sx motherboards using the SCAMPSX chipset", MACHINE_NOT_WORKING )
COMP( 199?, scsxaio,   ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "Peacock",     "386sx Ver. 2.0 motherboard SCsxAIO", MACHINE_NOT_WORKING )
COMP( 199?, sh386sx20, ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "Shuttle", "386SX REV 2.0A", MACHINE_NOT_WORKING )
COMP( 1991, t2000sx,   ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "Toshiba",     "T2000SX", MACHINE_NOT_WORKING )
COMP( 199?, td70a,     ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "<unknown>",   "TD70A and TD70AN", MACHINE_NOT_WORKING )
COMP( 199?, td70n,     ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "<unknown>",   "TD70N", MACHINE_NOT_WORKING )
COMP( 199?, tot3200sxc,ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "Toshiba",     "T3200SXC", MACHINE_NOT_WORKING )
COMP( 198?, tower386sx,ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "Schneider Rundfunkwerke AG", "386SX System 40 (VGA)", MACHINE_NOT_WORKING )
COMP( 1992, walk386sx, ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "Triumph-Adler", "Walkstation 386 SX", MACHINE_NOT_WORKING ) // screen remains blank
COMP( 198?, zeos386sx, ibm5170, 0,       at386sx,   0,     at386sx_state,     init_at386sx,        "ZEOS", "386 SX-16", MACHINE_NOT_WORKING )
