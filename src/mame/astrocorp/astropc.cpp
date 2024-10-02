// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Astro Russian Fruit Machines on SVGA "Pallas" hardware (PC based).

Some of the games have HDD/Flash images.
We need to convert them to CHDs (I think.. unless they're flash)
For images in WinHex format, see https://www.x-ways.net/winhex/WHX_Format.html

BIOS is Award v4.51PG, 03/05/2004-GXm-Cx5530-2A434U39C-00
Contains video BIOS for Cyrix GX86 (PCI ID: 1078:0104)

Games run on Red Hat Linux 32-bit, Linux kernel 2.4.21-rc6

There is a dongle (internally called "AstroCard") attached to parallel port 1,
which uses I/O ports 0x300/0x304/0x308 (rw) and 0x378/0x379/0x37a (ro).
It appears to contain 508 bytes of XOR-encrypted code which is decrypted by
the game's executable. See the "ProtectCode" function, which writes the
decrypted "PutJtx" function into the program in memory.

TODO:
- (with basic VGA core hooked up) hangs at Astro logo, attempts to write in the $cxxxx range (UMA?)

Known games

Title            Dumped  Notes
Fairy Tales          NO  https://www.astrocorp.com.tw/eng/game_1_1.php?gid=30
Arabian Night        NO  https://www.astrocorp.com.tw/eng/game_1_1.php?gid=31
Black Beard         YES  https://www.astrocorp.com.tw/eng/game_1_1.php?gid=32
Dragon Slayer       YES  https://www.astrocorp.com.tw/eng/game_1_1.php?gid=33
Flying Age           NO  https://www.astrocorp.com.tw/eng/game_1_1.php?gid=34
Halloween Party     YES  https://www.astrocorp.com.tw/eng/game_1_1.php?gid=35
Olympian Games      YES  https://www.astrocorp.com.tw/eng/game_1_1.php?gid=36
The Circus           NO  https://www.astrocorp.com.tw/eng/game_1_1.php?gid=37
Treasure Hunting     NO  https://www.astrocorp.com.tw/eng/game_1_1.php?gid=38
World War II         NO  https://www.astrocorp.com.tw/eng/game_1_1.php?gid=39
Ra's Scepter        YES
Hawaii              YES

Notes:
* Is "Hawaii" the same game as "Treasure Hunting"?
* The dslayrr sets appear to be pre-setup, have passwords set(?)
* Game information in the rom entries comes from `AstroGame/data/setup2.txt` or `AstroGame/INF.ini`

*/


#include "emu.h"

#include "bus/isa/isa_cards.h"
#include "cpu/i386/i386.h"
#include "machine/8042kbdc.h"
#include "machine/mc146818.h"
#include "machine/mediagx_cs5530_bridge.h"
#include "machine/mediagx_cs5530_ide.h"
#include "machine/mediagx_cs5530_video.h"
#include "machine/mediagx_host.h"
#include "machine/pci.h"
#include "machine/zfmicro_usb.h"

namespace {

class astropc_state : public driver_device
{
public:
	astropc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rtc(*this, "rtc")
		, m_kbdc(*this, "kbdc")
	{ }

	void astropc(machine_config &config);

	void init_astropc();

private:
	void main_map(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<ds1287_device> m_rtc;
	required_device<kbdc8042_device> m_kbdc;
};

void astropc_state::main_map(address_map &map)
{
}

void astropc_state::main_io(address_map &map)
{
}


static INPUT_PORTS_START( astropc )
INPUT_PORTS_END


void astropc_state::astropc(machine_config &config)
{
	MEDIAGX(config, m_maincpu, 233'000'000); // Cyrix MediaGX GXm-266GP
	m_maincpu->set_addrmap(AS_PROGRAM, &astropc_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &astropc_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("pci:12.0:pic8259_master", FUNC(pic8259_device::inta_cb));

	// TODO: copied from misc/matrix.cpp, verify if this has a working super I/O
	DS1287(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->set_binary(true);
	m_rtc->set_epoch(1980);
	m_rtc->irq().set("pci:12.0", FUNC(mediagx_cs5530_bridge_device::pc_irq8n_w));

	KBDC8042(config, m_kbdc, 0);
	// TODO: PS/2 mouse
	m_kbdc->set_keyboard_type(kbdc8042_device::KBDC8042_STANDARD);
	m_kbdc->system_reset_callback().set_inputline(":maincpu", INPUT_LINE_RESET);
	m_kbdc->gate_a20_callback().set_inputline(":maincpu", INPUT_LINE_A20);
	m_kbdc->input_buffer_full_callback().set(":pci:12.0", FUNC(mediagx_cs5530_bridge_device::pc_irq1_w));
	m_kbdc->set_keyboard_tag("at_keyboard");

	at_keyboard_device &at_keyb(AT_KEYB(config, "at_keyboard", pc_keyboard_device::KEYBOARD_TYPE::AT, 1));
	at_keyb.keypress().set(m_kbdc, FUNC(kbdc8042_device::keyboard_w));

	PCI_ROOT(config, "pci", 0);
	MEDIAGX_HOST(config, "pci:00.0", 0, "maincpu", 128*1024*1024);
	// TODO: again copied from misc/matrix.cpp, verify usage here
	PCI_BRIDGE(config, "pci:01.0", 0, 0x10780000, 0);

	// "pci:12.0" or "pci:10.0" depending on pin H26 (readable in bridge thru PCI index $44)
	mediagx_cs5530_bridge_device &isa(MEDIAGX_CS5530_BRIDGE(config, "pci:12.0", 0, "maincpu", "pci:12.2"));
	isa.set_kbdc_tag("kbdc");
	isa.boot_state_hook().set([](u8 data) { /* printf("%02x\n", data); */ });
	//isa.smi().set_inputline("maincpu", INPUT_LINE_SMI);
	isa.rtcale().set([this](u8 data) { m_rtc->address_w(data); });
	isa.rtccs_read().set([this]() { return m_rtc->data_r(); });
	isa.rtccs_write().set([this](u8 data) { m_rtc->data_w(data); });

	// "pci:12.1" SMI & ACPI

	mediagx_cs5530_ide_device &ide(MEDIAGX_CS5530_IDE(config, "pci:12.2", 0, "maincpu"));
	ide.irq_pri().set("pci:12.0", FUNC(mediagx_cs5530_bridge_device::pc_irq14_w));
	ide.irq_sec().set("pci:12.0", FUNC(mediagx_cs5530_bridge_device::pc_irq15_w));

	// "pci:12.3" XpressAUDIO
	MEDIAGX_CS5530_VIDEO(config, "pci:12.4", 0);

	ZFMICRO_USB(config, "pci:13.0", 0);

	// 2 PCI slots, 2 ISA slots
	ISA16_SLOT(config, "isa1", 0, "pci:12.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa2", 0, "pci:12.0:isabus", pc_isa16_cards, nullptr, false);
}


ROM_START( blackbd )
	// Название игры : BLACK BEARD GAMES
	// Тип игры      : Многолинейный видео-слот
	// Разработчик   : ASTRO CORP.
	ROM_REGION32_LE(0x40000, "pci:12.0", 0) /* motherboard bios */
	ROM_LOAD( "bios.002", 0x0000, 0x040000, CRC(45333544) SHA1(bcc03b4f77b2e447192a1e5ed9f4cc09d0289714) )

	ROM_REGION(0x20000, "rom", 0)
	ROM_LOAD16_BYTE( "b43-chip1.512", 0x0000, 0x010000, CRC(f17e4a65) SHA1(a375715c3d2c1bee01e022eb7b39f9b08207de13) )
	ROM_LOAD16_BYTE( "b43-chip2.512", 0x0001, 0x010000, CRC(612fffe0) SHA1(c762700bdb87c777d8ece9c11addf93850aae6db) )

	ROM_REGION(0x8000000, "drive", 0) // Raw sector image, last modified date of 2005-09-22
	ROM_LOAD( "black beard ru.04.43.a.dd", 0x0000, 0x3e20000, CRC(14430270) SHA1(1df178bf3b00a60448b82953696ff205adf3dc66) )

	ROM_REGION(0x8000000, "others", 0)
	ROM_LOAD( "93c46-2.046", 0x0000, 0x000080, CRC(08c9dea5) SHA1(647eda3f6ca8b8863417e9a64b87a99843ce3820) )
	ROM_LOAD( "93c46.046", 0x0000, 0x000080, CRC(52428b49) SHA1(e170e83193c97dd0016551e8d0ce56cc48d3afc4) )
ROM_END


ROM_START( blackbda )
	ROM_REGION32_LE(0x40000, "pci:12.0", 0) /* motherboard bios */  // wasn't in this set!
	ROM_LOAD( "bios.002", 0x0000, 0x040000, CRC(45333544) SHA1(bcc03b4f77b2e447192a1e5ed9f4cc09d0289714) )

	ROM_REGION(0x20000, "rom", 0)
	ROM_LOAD16_BYTE( "bbird1.512", 0x00000, 0x010000, CRC(0782cd0b) SHA1(976273880dc0357b4c9e432e44c9f82bac55f5e5) )
	ROM_LOAD16_BYTE( "bbird2.512", 0x00001, 0x010000, CRC(9cf9f0a7) SHA1(f8a9c851ca7ea859f90f9ad6afd4cf1178eae039) )

	ROM_REGION(0x8000000, "drive", 0) // "Paragon Backup File" format
	ROM_LOAD( "img_d1.pbf", 0x0000, 0x004c45, CRC(a0dba309) SHA1(4d9dd245cc973fb70aff90cac5a94701b6b6ccd3) )
	ROM_LOAD( "img_d100.p00", 0x0000, 0x36a50ac, CRC(e0353f4f) SHA1(a930ac9e272d5474264490dc3a3223670d00b61d) )
	ROM_LOAD( "img_d100.pfm", 0x0000, 0x000618, CRC(b3738001) SHA1(8266fd53357779d934cd1cb1b5a27b8e9e0dcce2) )
ROM_END

ROM_START( blackbdb )
	// Название игры : BLACK BEARD GAMES
	// Тип игры      : Многолинейный видео-слот
	// Разработчик   : ASTRO CORP.
	ROM_REGION32_LE(0x40000, "pci:12.0", 0) /* motherboard bios */  // wasn't in this set!
	ROM_LOAD( "bios.002", 0x0000, 0x040000, CRC(45333544) SHA1(bcc03b4f77b2e447192a1e5ed9f4cc09d0289714) )

	ROM_REGION(0x20000, "rom", 0)
	ROM_LOAD16_BYTE( "u1_bbru44.bin", 0x00000, 0x010000, CRC(bd973bc1) SHA1(1f1997a3c1c70ccec01c1cb44c127356b6412457) )
	ROM_LOAD16_BYTE( "u2_bbru44.bin", 0x00001, 0x010000, CRC(4a6a6a18) SHA1(959ad5a369b27e73e9e879471784ccab1001d114) )

	ROM_REGION(0x8000000, "drive", 0) // Raw sector image, last modified date of 2005-12-22
	ROM_LOAD( "blackbeard  ru_04b.img", 0x0000, 0x3e20000, CRC(cadbaa2b) SHA1(15033bffedd173622d50ac0adf99e257c207748c) )
ROM_END

ROM_START( dslayrr )
	// TITLE = DRAGON_SLAYER
	// TYPE = MULTI-LINER
	// PRODUCER = ASTRO_CORP.
	// PUBDATE = 2005/08/10
	// VERLOC = LOC_RU
	// VERMAJOR = 15
	// VERMINOR = B
	// RELEASE = 1
	ROM_REGION32_LE(0x40000, "pci:12.0", 0) /* motherboard bios */  // wasn't in this set!
	ROM_LOAD( "bios.002", 0x0000, 0x040000, CRC(45333544) SHA1(bcc03b4f77b2e447192a1e5ed9f4cc09d0289714) )

	ROM_REGION(0x20000, "rom", 0)
	ROM_LOAD16_BYTE( "ds ru 15.21.b-1.bin", 0x00000, 0x010000, CRC(d33fdb7b) SHA1(1bf16716dd534ca8e89063184cd7ccaed732c43b) )
	ROM_LOAD16_BYTE( "ds ru 15.21.b-2.bin", 0x00001, 0x010000, CRC(50065dd5) SHA1(44f60467e90d0bdb82227ce25261d22f921b9903) )

	ROM_REGION(0x8000000, "drive", 0) // WinHex format, last modified date of 2005-08-22
	ROM_LOAD( "dragon slayer ru 15.21.b.whx", 0x0000, 0x7c80181, CRC(7af9ed2e) SHA1(082463eb44e8ca144e2e934ba5820ab248599033) )
ROM_END

ROM_START( dslayrra )
	// TITLE = DRAGON_SLAYER
	// TYPE = MULTI-LINER
	// PRODUCER = ASTRO_CORP.
	// PUBDATE = 2005/11/10
	// VERLOC = LOC_RU
	// VERMAJOR = 16
	// VERMINOR = B
	// RELEASE = 1
	ROM_REGION32_LE(0x40000, "pci:12.0", 0) /* motherboard bios */  // wasn't in this set!
	ROM_LOAD( "bios.002", 0x0000, 0x040000, CRC(45333544) SHA1(bcc03b4f77b2e447192a1e5ed9f4cc09d0289714) )

	ROM_REGION(0x20000, "rom", 0)
	ROM_LOAD16_BYTE( "u1-0f0b.rf", 0x00000, 0x010000, CRC(d33fdb7b) SHA1(1bf16716dd534ca8e89063184cd7ccaed732c43b) )
	ROM_LOAD16_BYTE( "u2-f500.rf", 0x00001, 0x010000, CRC(50065dd5) SHA1(44f60467e90d0bdb82227ce25261d22f921b9903) )

	ROM_REGION(0x8000000, "drive", 0) // Raw sector image, last modified date of 2006-06-30
	ROM_LOAD( "ds.16b", 0x0000, 0x7a80000, CRC(19b229c6) SHA1(eb419dbfdec0ad03c422fdc54e77a5df37442026) )
ROM_END

ROM_START( hawaii )
	// GAME NAME    : HAWAII GAMES
	// GAME TYPE    : MULTI-LINER
	// DEVELOPER    : ASTRO CORP.
	ROM_REGION32_LE(0x40000, "pci:12.0", 0) /* motherboard bios */  // wasn't in this set!
	ROM_LOAD( "bios.002", 0x0000, 0x040000, CRC(45333544) SHA1(bcc03b4f77b2e447192a1e5ed9f4cc09d0289714) )

	ROM_REGION(0x20000, "rom", 0)
	ROM_LOAD16_BYTE( "rom1.512", 0x00000, 0x010000, CRC(b963d590) SHA1(c6d38cf1865efd8619a9eec07410db1e16e7276d) )
	ROM_LOAD16_BYTE( "rom2.512", 0x00001, 0x010000, CRC(db670705) SHA1(6a76d5114847f54cf98e4c016bb5d47a4b7e1ef8) )

	ROM_REGION(0x8000000, "drive", 0) // WinHex format, last modified date of 2004-07-19
	ROM_LOAD( "hawaii   v1.01a .whx", 0x0000, 0x3c0017b, CRC(f033f963) SHA1(5f50aaf3ddbde176388612ea1a4c0040533f2109) )
ROM_END

ROM_START( oligam )
	// GAME NAME    : OLYMPIAN GAMES
	// GAME TYPE    : MULTI-LINER
	// DEVELOPER    : ASTRO CORP.
	ROM_REGION32_LE(0x40000, "pci:12.0", 0) /* motherboard bios */  // wasn't in this set!
	ROM_LOAD( "bios.002", 0x0000, 0x040000, CRC(45333544) SHA1(bcc03b4f77b2e447192a1e5ed9f4cc09d0289714) )

	ROM_REGION(0x20000, "rom", 0)
	ROM_LOAD16_BYTE( "rom1.512", 0x00000, 0x010000, CRC(41cefe2a) SHA1(5f03f93e92555a76284f97366a9761106901506f) )
	ROM_LOAD16_BYTE( "rom2.512", 0x00001, 0x010000, CRC(caa4fe49) SHA1(6b92af831a210b0c5264dde30fb13611bf2e366c) )

	ROM_REGION(0x8000000, "drive", 0) // Raw sector image, last modified date of 2005-11-29
	ROM_LOAD( "olympian games ru.04.39.a.dd", 0x0000, 0x3e90000, CRC(ba452de5) SHA1(371e6157bcd5a1ed48b4a75f4962244157610912) )
ROM_END

ROM_START( rasce )
	// GAME NAME    : RA'S SCEPTER GAMES
	// GAME TYPE    : MULTI-LINER
	// DEVELOPER    : ASTRO CORP.
	ROM_REGION32_LE(0x40000, "pci:12.0", 0) /* motherboard bios */  // wasn't in this set!
	ROM_LOAD( "bios.002", 0x0000, 0x040000, CRC(45333544) SHA1(bcc03b4f77b2e447192a1e5ed9f4cc09d0289714) )

	ROM_REGION(0x20000, "rom", 0)
	ROM_LOAD16_BYTE( "u1-49c1.rf", 0x00000, 0x010000, CRC(2da68b6a) SHA1(3ad5833841d06495bae3fcca561f23124602864e) )
	ROM_LOAD16_BYTE( "u2-3f00.rf", 0x00001, 0x010000, CRC(b1dabdc9) SHA1(9d88be2a9851497c03143232d7c22da0ff297d05) )

	ROM_REGION(0x8000000, "drive", 0) // Raw sector image, last modified date of 2005-11-29
	ROM_LOAD( "rs.06.03r", 0x0000, 0x7a80000, CRC(66132c3d) SHA1(4a73bab9518548950e11aebc6edf67f64d0d7798) )
ROM_END

 // Pallas GX1 REV:B - AMD Geode CS5530A-UCE, SMC FDC37C932APM. Sub board with ROMs, ASTRO M and ASTRO V102PX-003 customs
ROM_START( hwparty )
	// GAME NAME    : HALLOWEEN PARTY
	// GAME TYPE    : MULTI-LINER
	// DEVELOPER    : ASTRO CORP
	ROM_REGION32_LE(0x40000, "pci:12.0", 0) /* motherboard bios */
	ROM_LOAD( "phoenixbios_e586 bios.u16", 0x0000, 0x040000, CRC(885e3cde) SHA1(ff90cda4383a119c7f54545c11fa432505e66c1b) )

	ROM_REGION(0x20000, "rom", 0) // on subboard
	ROM_LOAD16_BYTE( "1_hwus-112.u22", 0x00000, 0x010000, CRC(5cbe992c) SHA1(e785e5863ccc52d4ca5485569244d89e226398a0) )
	ROM_LOAD16_BYTE( "2_hwus-112.u11", 0x00001, 0x010000, CRC(7b0187ed) SHA1(7d0791990aec06d929bcb40a6d9888b3bb350071) )

	ROM_REGION(0x8000000, "drive", 0)
	ROM_LOAD( "halloween_us.23.a.img", 0x0000, 0x7a80000, CRC(65d3877f) SHA1(076035bd55189a186368ae42463ab7471be1583c) )
ROM_END


// The following games use an Artemis II sub board and may need to be moved to a separate driver
// The Artemis II sub board has 2x M68K ROMs and 2x LY621024SL RAMs on the upper side,
// while on the back side there are the following components: ASTRO V102PX encrypted M68K (002 for carnivac, 001 for santacl),
// Astro ROHS (GFX chip?) and 2x LY621024SL RAMs.


// main PCB: unmarked with AMD Geode CS5536AD, W83627HG-AW IOC, RTL8100C ethernet controller, big heat-sinked chip,
// 4x HY5DU121622CTP SRAM (mounted on SO-DIMM), Realtek ALC203 + Yamaha YDA138-E (mounted on Yamaha_Audio_A riser board)
// with 24.576 MHz XTAL, Trascend Compact Flash
ROM_START( carnivac )
	ROM_REGION32_LE(0x80000, "pci:12.0", 0)
	ROM_LOAD( "phoenixbios.u8", 0x00000, 0x80000, CRC(aeb6cac5) SHA1(1b12b0d20d6451ac36ab5976d1e977034212e3a0) )

	ROM_REGION(0x20000, "rom", 0) // on subboard
	ROM_LOAD16_BYTE( "1_caus-014.tu1", 0x00000, 0x010000, CRC(131947d1) SHA1(521751c5a8a0e6031de7998ed64a8be3d6f0a290) )
	ROM_LOAD16_BYTE( "2_caus-014.tu3", 0x00001, 0x010000, CRC(22d6209d) SHA1(621786d49e7fd38ce1b7665664f7b97604fedbd5) )

	ROM_REGION(0x10000000, "drive", 0)
	ROM_LOAD( "carnival_us.004.d.img", 0x0000, 0xf618000, CRC(566b9fbf) SHA1(14364b05151db60b1882c2230d896ac8c632395b) )
ROM_END

// main PCB: Astro AS-LX800 A5 with AMD Geode CS5536AD, big heat-sinked chip, 4x HY5DU121622CTP SRAM (directly on PCB),
// Realtek ALC203 + Yamaha YDA138-E with 24.576 MHz XTAL (directly on PCB), Trascend Compact Flash
ROM_START( santacl )
	ROM_REGION32_LE(0x80000, "pci:12.0", 0)
	ROM_LOAD( "phoenixbios.u30", 0x00000, 0x80000, CRC(4e24ffdb) SHA1(8b4bba02183da5eae27755b86eca830d06fe05ae) )

	ROM_REGION(0x20000, "rom", 0) // on subboard
	ROM_LOAD16_BYTE( "1_scin-020.tu1", 0x00000, 0x010000, CRC(b05e9c50) SHA1(2e67d144497f0b531d94e97107e8c2b238e46363) )
	ROM_LOAD16_BYTE( "2_scin-020.tu3", 0x00001, 0x010000, CRC(97821e0d) SHA1(ec2a44916799c2f92b9c6db75cffe22cd2dd61ef) )

	ROM_REGION(0x20000000, "drive", 0)
	ROM_LOAD( "santa_claus_in.001.07.a.img", 0x0000, 0x1e6c6000, CRC(c89732de) SHA1(499f49b97baeef9681a0f9e4538e74676d4379c4) )
ROM_END


void astropc_state::init_astropc()
{
}

} // anonymous namespace


// Pallas games

GAME( 2005,  blackbd,  0,       astropc, astropc, astropc_state, init_astropc, ROT0, "Astro Corp.", "Black Beard (Russia, set 1)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2005?, blackbda, blackbd, astropc, astropc, astropc_state, init_astropc, ROT0, "Astro Corp.", "Black Beard (Russia, set 2)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2005,  blackbdb, blackbd, astropc, astropc, astropc_state, init_astropc, ROT0, "Astro Corp.", "Black Beard (Russia, set 3)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

GAME( 2005,  dslayrr,  0,       astropc, astropc, astropc_state, init_astropc, ROT0, "Astro Corp.", "Dragon Slayer (Russia, v15.B, 2005/08/10)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2006,  dslayrra, dslayrr, astropc, astropc, astropc_state, init_astropc, ROT0, "Astro Corp.", "Dragon Slayer (Russia, v16.B, 2005/11/10)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

GAME( 2005,  hwparty,  0,       astropc, astropc, astropc_state, init_astropc, ROT0, "Astro Corp.", "Halloween Party (US.23.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

GAME( 2004,  hawaii,   0,       astropc, astropc, astropc_state, init_astropc, ROT0, "Astro Corp.", "Hawaii (Russia)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

GAME( 2005,  oligam,   0,       astropc, astropc, astropc_state, init_astropc, ROT0, "Astro Corp.", "Olympian Games (Russia)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

GAME( 2005,  rasce,    0,       astropc, astropc, astropc_state, init_astropc, ROT0, "Astro Corp.", "Ra's Scepter (Russia)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )


// Artemis II games
GAME( 2009,  carnivac, 0,       astropc, astropc, astropc_state, init_astropc, ROT0, "Astro Corp.", "Carnival (Astro Corp., US.004.D)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2009,  santacl,  0,       astropc, astropc, astropc_state, init_astropc, ROT0, "Astro Corp.", "Santa Claus (IN.001.07.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
