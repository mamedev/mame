// license:BSD-3-Clause
// copyright-holders:

/*
Namco/Mitsubishi DS-5000 Driving Simulator

This is an absolute monster. It drives six displays, with a DSP, OBJ and PGN PCB dedicated to each.
That's 30 TMS320C25 DSPs in total.

The 'CPU020' board is the same type as used in Galaxian 3. The OBJ board appears to be slightly different
to the other System 21 titles and the ROM and sound / CPU68K boards are unique.

The metal rack contains a total of 21 PCBs (020, 68K, ROM, 6x DSP, 6x OBJ and 6x PGN).


8623961202 CPU020
(8623963202)

MC68020RP25E CPU
49.1520 MHz XTAL
Namco C165
38.7692 MHz XTAL
8x HM6708AP-20 RAM
Namco C139 SCI
MSM5179P-45 RAM
MC68681P DUART
3.6864 MHz XTAL
HN58C65P-25 RAM
Namco C197
Namco C137 clock generator IC
bank of 4 switches
2x banks of 8 switches
reset push button


CPU68K
8623961304
(8623963304)

HD68HC000P12 CPU
49.1520 MHz XTAL
HN58C65P-25 RAM
2x HM65256BLSP-12 RAM
4x HM628128ALP-8 RAM
2x 84256A-10L RAM
HD68B09P CPU
Namco C137 clock generator IC
Namco C165
38.7692 MHz XTAL
HM628128ALP-8 RAM
MB8464A-15L RAM
MB8422-12LP dual port RAM
Namco C140 sound chip
4x HM628128ALP-8 RAM
2x CY7C128A-35PC RAM
Namco C148 interrupt controller
Namco C197
Namco C327
2x Namco C139 SCI
2x M5M5179P-45 RAM
LC7881 DAC
M51568FP audio amplifier
bank of 8 switches
reset (?) button


8623961401 ROM
(8623963401)

MB89352AP SCSI protocol controller
empty socket marked keycus
bank of 8 switches
bank of 4 switches


8623961703 DSP
(8623963703) TSK-A

4x Namco C342
5x TMS320C25 (marked 67, 1 master and 4 slaves)
5x Namco C197 (1 for each DSP)
4x M5M5189BP-20 (for the master DSP)
16x HM6268P-25 RAM (4x for each slave DSP)
Namco C317
6x HM628128LP-7
Namco C195
Namco C327
Namco C197
2x HM62832HP-25
40.000 MHz XTAL
bank of 4 switches
empty socket marked keycus
empty sockets for 2 ROMs for each DSP


8623961600 PGN
(8623963600) TSK-A

Namco C167
4x 84256A-10L RAM
CY7C128A-35PC
16x Namco C157
8x 84256A-10L RAM
2x L7A0564
4x CY7C128A-35PC
2x M5M5178P-33 RAM
MB8422-12LP dual port RAM
2x 84256A-10L RAM
L7A0564
Namco C150
2x M5M5178P-33
20.000 MHz XTAL


8623961505 OBJ
(8623963505)

Namco C165
38.76922 MHz XTAL
Namco C138
3x LH52B256-70LL RAM
Namco C187
32x 65256BLFP-10T RAM
Namco C355
2x M5M5256BP-70L
4x HM628128LP-10
empty sockets for 4 OBJ ROMs
*/


#include "emu.h"

#include "namco_c139.h"
#include "namco_c148.h"
#include "namco_c355spr.h"
#include "namcos21_3d.h"
#include "namcos21_dsp_c67.h"

#include "cpu/m68000/m68000.h"
#include "cpu/m68000/m68020.h"
#include "cpu/m6809/m6809.h"
#include "sound/c140.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class ds5000_state : public driver_device
{
public:
	ds5000_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void ds5000(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_program_map(address_map &map) ATTR_COLD;
	void sound_program_map(address_map &map) ATTR_COLD;
};


uint32_t ds5000_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void ds5000_state::main_program_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
}

void ds5000_state::sound_program_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
}


static INPUT_PORTS_START( ds5000 )
INPUT_PORTS_END


// TODO: all dividers not verified
void ds5000_state::ds5000(machine_config &config)
{
	M68020(config, m_maincpu, 49.152_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &ds5000_state::main_program_map);
	m_maincpu->set_vblank_int("screen", FUNC(ds5000_state::irq1_line_hold));

	m68000_device &audiocpu(M68000(config, "audiocpu", 49.152_MHz_XTAL / 4));
	audiocpu.set_addrmap(AS_PROGRAM, &ds5000_state::sound_program_map);

	// TODO: HD68B09P

	// TODO: DSPs

	// TODO: Namco customs

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0*8, 512-1, 0*8, 512-1);
	screen.set_screen_update(FUNC(ds5000_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(0x8000);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	C140(config, "c140", 49.152_MHz_XTAL / 384 / 6).add_route(ALL_OUTPUTS, "mono", 0.75); // 21.333kHz, copied from other Namco drivers
}


ROM_START( ds5000 )
	ROM_REGION( 0x40000, "maincpu", 0 ) // DS1 91/07/10 17:11:50 string. On CPU020 board
	ROM_LOAD16_BYTE( "dsk1_prog3.18b", 0x00000, 0x20000, CRC(9d55673b) SHA1(23fa73d473a9e02f674426ebd474e5cc0b4f2a44) ) // 1111xxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "dsk1_prog2.14b", 0x00001, 0x20000, CRC(0245f06f) SHA1(dfd2476aeeba1bd7cd8499762abce00e97c7cca6) ) // 1111xxxxxxxxxxxxx = 0xFF
	// prog 0-1 and data 0-3 sockets not populated

	ROM_REGION( 0x40000, "audiocpu", 0 ) // DS2 93/11/04 20:39:16 string. On CPU68K board
	ROM_LOAD16_BYTE( "dsk1_prgub.9c", 0x00000, 0x20000, CRC(bd205b6a) SHA1(0f8f3cea06f50b6ecdaf0648b275c657bd133c6b) ) // 1111xxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "dsk1_prglb.9a", 0x00001, 0x20000, CRC(263fc8ac) SHA1(cbb93f3eecbe4f11edf8a03994c391917d965d2c) ) // 1111xxxxxxxxxxxxx = 0xFF

	ROM_REGION16_BE( 0x400000, "c140", ROMREGION_ERASE00 ) // on CPU68K board. TODO: verify ROM loading
	ROM_LOAD16_BYTE( "dsk1-voi0.4t", 0x000000, 0x80000, CRC(273d019e) SHA1(149c2000d27d238ed6de729f162f7b2c1739d9da) )
	ROM_LOAD16_BYTE( "dsk1-voi1.6t", 0x100000, 0x80000, CRC(c57b8382) SHA1(7d2187fdb410002630fdd60468abf1083825617f) )
	ROM_LOAD16_BYTE( "dsk1-voi2.7t", 0x200000, 0x80000, CRC(4477e975) SHA1(a78367932309f87952f129ffb407109142d0e722) )
	ROM_LOAD16_BYTE( "dsk1-voi3.9t", 0x300000, 0x80000, CRC(ca8aeb04) SHA1(340f63cfcf96f2a7a979619fc18132df8d81a8c2) )
	// voice 4-7 sockets are populated with RAM instead of ROM

	ROM_REGION16_BE( 0x200000, "dsp1", 0 ) // on ROM board. TODO: verify ROM loading
	ROM_LOAD16_BYTE( "dss2_a0u.12a", 0x000000, 0x20000, CRC(55755eb3) SHA1(d75fd9139d0783dc75ca2107cccb2612d427e8ba) )
	ROM_LOAD16_BYTE( "dss2_a0l.8a",  0x000001, 0x20000, CRC(8b53cbe5) SHA1(0c9dba0949399b7d427a618fa1cd6fa890d2862b) )
	ROM_LOAD16_BYTE( "dss2_a1u.13a", 0x040000, 0x20000, CRC(a10811ff) SHA1(9ae20aa409d04c2c36a13c1b1f2c2570b891cd06) ) // 1xxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "dss2_a1l.9a",  0x040001, 0x20000, CRC(c771e9bf) SHA1(acce88e01142f647ee059b196c5eec7832457ff3) ) // 1xxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "dss2_a2u.14a", 0x080000, 0x40000, CRC(30c11b0f) SHA1(2c5ecc52e3373106c2cf6c9ddbe1af07e861a827) )
	ROM_LOAD16_BYTE( "dss2_a2l.10a", 0x080001, 0x40000, CRC(0e787b6b) SHA1(5ecec393e1629f00ab828f810feb79fb14b80563) )
	ROM_LOAD16_BYTE( "dss2_a3u.15a", 0x100000, 0x80000, CRC(067e4890) SHA1(799ec54cf68a6f5cbe1adefb76de8cccb31aa030) )
	ROM_LOAD16_BYTE( "dss2_a3l.11a", 0x100001, 0x80000, CRC(01d461f5) SHA1(5858e6ec60e339b39e1875853a91776ea5229214) )
	// a4-7u and a4-7l sockets not populated

	ROM_REGION16_BE( 0x200000, "dsp2", 0 ) // on ROM board. TODO: verify ROM loading
	ROM_LOAD16_BYTE( "dsw2_b4u.12f", 0x000000, 0x20000, CRC(51951824) SHA1(43a5702b7c2b6aa6a12d9adc4823cc3033db370b) )
	ROM_LOAD16_BYTE( "dsw2_b4l.8f",  0x000001, 0x20000, CRC(9f889865) SHA1(9326d6981c489d65519ec498560717cf86032dba) )
	ROM_LOAD16_BYTE( "dsw2_b5u.13f", 0x040000, 0x20000, CRC(b69d9d58) SHA1(146e2e881becef384ceed49033ca30dfeca87f19) )
	ROM_LOAD16_BYTE( "dsw2_b5l.9f",  0x040001, 0x20000, CRC(11b112dc) SHA1(0ef54786df7d54514619c13ad7eea0ce7815fe42) )
	ROM_LOAD16_BYTE( "dsw2_b6u.14f", 0x080000, 0x40000, CRC(90609bee) SHA1(dc2cf98f630a86458ee33867401619def2c9bf8f) )
	ROM_LOAD16_BYTE( "dsw2_b6l.10f", 0x080001, 0x40000, CRC(f2c7200a) SHA1(38232be89cc64b47738b9803f56b88fe2ca0348a) )
	ROM_LOAD16_BYTE( "dsw2_b7u.15f", 0x100000, 0x80000, CRC(db369aef) SHA1(5d1c90a20de02d65904d21bffc68624c08aa433c) )
	ROM_LOAD16_BYTE( "dsw2_b7l.11f", 0x100001, 0x80000, CRC(3466bea4) SHA1(7f21b7dec83ce3be8a86696d5a97b61438cde2ec) )
	// b0-3u and b0-3l sockets not populated

	ROM_REGION( 0x200, "cpu020_plds", ROMREGION_ERASE00 )
	ROM_LOAD( "3p0201.12r", 0x000, 0x149, NO_DUMP ) // AMPAL18P8BPC

	ROM_REGION( 0x400, "cpu68k_plds", ROMREGION_ERASE00 )
	ROM_LOAD( "3p68k1.9k",  0x000, 0x0cc, NO_DUMP ) // PAL20L10ACNS
	ROM_LOAD( "3p68k2.11r", 0x100, 0x117, NO_DUMP ) // PALCE16V8H-15
	ROM_LOAD( "3p68k5.9e",  0x300, 0x0cc, NO_DUMP ) // PAL20L10ACNS

	ROM_REGION( 0xb00, "rom_plds", ROMREGION_ERASE00 )
	ROM_LOAD( "3prom1.2e",  0x000, 0x0cc, NO_DUMP ) // chip type unreadable
	ROM_LOAD( "3prom2.5c",  0x100, 0x149, CRC(34d260e8) SHA1(b3d55a77a851316886da04a12b1c28c8d3acaf12) ) // PLHS18P8A
	ROM_LOAD( "3prom3.5f",  0x300, 0x149, CRC(38836bd3) SHA1(e49578fc606b4fcb0d5e96cb338e217ed32ce0ad) ) // PLHS18P8A
	ROM_LOAD( "3prom4.6c",  0x500, 0x117, NO_DUMP ) // PAL16L8BCN
	ROM_LOAD( "3prom5.6f",  0x700, 0x117, NO_DUMP ) // PAL16L8BCN
	ROM_LOAD( "3prom6.9r",  0x900, 0x149, CRC(582a081c) SHA1(03d8935fed94f9811eb220e1942a047af4118162) ) // PLHS18P8A

	ROM_REGION( 0x500, "dsp_plds", ROMREGION_ERASE00 )
	ROM_LOAD( "3pdsp1.17c",  0x000, 0x0cc, NO_DUMP ) // PAL.. exact chip type not readable
	ROM_LOAD( "3pdsp2.2e",   0x100, 0x117, NO_DUMP ) // chip type not readable
	ROM_LOAD( "3pdsp5.17d",  0x300, 0x117, NO_DUMP ) // GAL16V8A

	ROM_REGION( 0x200, "pgn_plds", ROMREGION_ERASE00 )
	ROM_LOAD( "3ppgn1.4c", 0x000, 0x149, NO_DUMP ) // PAL18.. exact chip type not readable

	ROM_REGION( 0xa00, "obj_plds", ROMREGION_ERASE00 )
	ROM_LOAD( "3pobj1.6c",  0x000, 0x149, NO_DUMP ) // chip type unreadable
	ROM_LOAD( "3pobj2.6b",  0x200, 0x149, NO_DUMP ) // chip type unreadable
	ROM_LOAD( "3pobj3.21n", 0x400, 0x149, NO_DUMP ) // PLHS18P8A
	ROM_LOAD( "3pobj4.22j", 0x600, 0x149, NO_DUMP ) // PLHS18P8A
	ROM_LOAD( "3pobj5.22k", 0x800, 0x149, NO_DUMP ) // chip type unreadable
ROM_END

} // anonymous namespace


GAME( 1991, ds5000, 0, ds5000, ds5000, ds5000_state, empty_init, ROT0, "Namco / Mitsubishi", "DS-5000 Driving Simulator", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
