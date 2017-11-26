// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-11-17 Skeleton

A selection of terminals from Bitsavers. A placeholder for now.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"

class terminals_state : public driver_device
{
public:
	terminals_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		//, m_p_chargen(*this, "chargen")
	{ }

private:
	required_device<cpu_device> m_maincpu;
	//required_region_ptr<u8> m_p_chargen;
};

static ADDRESS_MAP_START( mem_map, AS_PROGRAM, 8, terminals_state )
ADDRESS_MAP_END

static INPUT_PORTS_START( terminals )
INPUT_PORTS_END

static MACHINE_CONFIG_START( terminals )
	MCFG_CPU_ADD("maincpu", Z80, 2'000'000)
	MCFG_CPU_PROGRAM_MAP(mem_map)
MACHINE_CONFIG_END

/**************************************************************************************************************

Ann Arbor Ambassador.
Chips: Z80A, M58725P (16k RAM), 2x SCN2651C, nvram, button-battery
Crystals: 18.414, 6.0688

***************************************************************************************************************/

ROM_START( aaa )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "459_1.bin",    0x0000, 0x1000, CRC(55fb3e3b) SHA1(349cd257b1468827e1b389be7c989d0e4a13a5f1) )
	ROM_LOAD( "459_3.bin",    0x1000, 0x1000, CRC(e1e84ca4) SHA1(42dc5f4211beee79178f0c03bb45c66833119eae) )
	ROM_LOAD( "459_4.bin",    0x8000, 0x2000, CRC(4038aa89) SHA1(caf33c1f87aa396860324b9c73b35e4221f03d2e) )

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "202510b.bin",  0x0000, 0x1000, CRC(deda4aa4) SHA1(0bce5a8dc260ba51f3e431d8da408eac1f41acf7) )
ROM_END

COMP( 1981, aaa, 0, 0, terminals, terminals, terminals_state, 0, "Ann Arbor", "Ambassador", MACHINE_IS_SKELETON )



/**************************************************************************************************************

Hewlett-Packard HP-700/92.
Chips: TC5564APL-15, proprietory square chip, D70108C (V20), SCN2681, Beeper
Crystals: 29.4912

***************************************************************************************************************/

ROM_START( hp700_92 )
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD( "5181-8672.u803", 0x00000, 0x20000, CRC(21440d2f) SHA1(69a3de064ae2b18adc46c2fdd0bf69620375efe7) )
ROM_END

COMP( 1987, hp700_92, 0, 0, terminals, terminals, terminals_state, 0, "HP", "HP-700/92", MACHINE_IS_SKELETON )



/**************************************************************************************************************

Hewlett-Packard HP-2622A.
Chips: National 8367 CRTC (labeled B8250), SY6551 (labeled 8251), Z8400A (Z80)
Crystal: 25.7715

***************************************************************************************************************/

ROM_START( hp2622a )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "1818-1685.xu63", 0x0000, 0x2000, CRC(a57ffe5e) SHA1(4d7844320deba916d9ec289927af987fea025b02) )
	ROM_LOAD( "1818-1686.xu64", 0x2000, 0x2000, CRC(bee9274c) SHA1(20796c559031a91cb2666776fcf7ffdb52a0a318) )
	ROM_LOAD( "1818-1687.xu65", 0x4000, 0x2000, CRC(e9ecd489) SHA1(9b249b8d066d256069ccdb8809bb808c414f106a) )

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD( "1818-1489.xu311", 0x0000, 0x2000, CRC(9879b153) SHA1(fc1705d6de38eb6d3a67f1ae439e359e5124d028) )
ROM_END

COMP( 1982, hp2622a, 0, 0, terminals, terminals, terminals_state, 0, "HP", "HP-2622A", MACHINE_IS_SKELETON )



/**************************************************************************************************************

Qume QVT-70.
Chips: Z80, Z80 DART, 5x CXK5864CM-70LL/W242575-70LL, 801000-02, 303489-01, DS1231, Button battery, Beeper
Crystals: unreadable

***************************************************************************************************************/

ROM_START( qvt70 )
	ROM_REGION(0x30000, "maincpu", 0)
	ROM_LOAD( "251513-04_revj.u12", 0x00000, 0x10000, CRC(3960bbd5) SHA1(9db306cef09be21ff43c081ebe11e9b46f617861) )
	ROM_LOAD( "251513-03_revj.u11", 0x10000, 0x20000, CRC(c56796fe) SHA1(afe024ff93d5e75dc18041219d61e1a22fc6d883) )
ROM_END

COMP( 1992, qvt70, 0, 0, terminals, terminals, terminals_state, 0, "Qume", "QVT-70", MACHINE_IS_SKELETON )



/**************************************************************************************************************

Qume QVT-102.
Chips: HD46800DP (6800), HD46505SP (6845), HD46850P (6850), M58725P-15 (16k RAM), LH0082 (Z80CTC), Button battery
Crystals: 16.6698
Keyboard: D8748D, 6.000, Beeper

***************************************************************************************************************/

ROM_START( qvt102 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "t205m.u8",   0x0000, 0x2000, CRC(59cc04f6) SHA1(ee2e3a3ea7b57a231483fcc74266f0f3f51204af) )

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "c3205m.u32", 0x0000, 0x1000, CRC(f6d86e87) SHA1(c0885e4a35095a730d760bf91a1cf4e8edd6a2bb) )

	ROM_REGION(0x10000, "keyboard", 0)
	ROM_LOAD( "k301.u302",  0x0000, 0x0400, CRC(67564b20) SHA1(5897ff920f8fae4aa498d3a4dfd45b58183c041d) )
ROM_END

COMP( 1983, qvt102, 0, 0, terminals, terminals, terminals_state, 0, "Qume", "QVT-102", MACHINE_IS_SKELETON )



/**************************************************************************************************************

Qume QVT-103.
Chips: Z80A, Z80A DART, Z80A CTC, 2x CRT9212, 5x HM6116P-2, TC5516APL, D8741AD, CRT9007, 1x 10-sw dip, Button battery.
Crystals: (all hard to read) 29.876, 6.000
Keyboard CPU, Crystal, ROM are on the main board.

***************************************************************************************************************/

ROM_START( qvt103 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "t103e1.u28", 0x0000, 0x2000, CRC(eace3cbe) SHA1(1e7f395c5233d8656df5305163d050275f0a8033) )
	ROM_LOAD( "t103e2.u27", 0x2000, 0x4000, CRC(100cf542) SHA1(4b2569d509790a0f94b4447fb9d3d42582fcaf66) )

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "c103b.u40",  0x0000, 0x1000, CRC(3419760d) SHA1(3455c70ed48c7f7769d73a84f152beddf508094f) )

	ROM_REGION(0x10000, "keyboard", 0)
	ROM_LOAD( "k304a.u24",  0x0000, 0x0400, CRC(e4b1f0da) SHA1(e9f8c48c34105464b3db206b34f67e7603484fea) )
ROM_END

COMP( 1983, qvt103, 0, 0, terminals, terminals, terminals_state, 0, "Qume", "QVT-103", MACHINE_IS_SKELETON )



/**************************************************************************************************************

Qume QVT-190.
Chips: MC68B00P, 2x MC68B50P, MC68B45P, V61C16P55L, M5M5165P-70L, ABHGA101006, button battery, 7-DIL-jumper
Crystal: unreadable

***************************************************************************************************************/

ROM_START( qvt190 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "95987-267.u19", 0x0000, 0x8000, CRC(78894d8e) SHA1(0a0f6883dd18872bddeb3ed18ebe496080e6591b) )

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD( "95864-304.u17", 0x0000, 0x2000, CRC(2792e99b) SHA1(4a84d029d0e63975fc95dc7056d2523193dff986) )
ROM_END

COMP( 1987, qvt190, 0, 0, terminals, terminals, terminals_state, 0, "Qume", "QVT-190", MACHINE_IS_SKELETON )



/**************************************************************************************************************

Qume QVT-201.
Chips: Z80A, SCN2681A, SCN2672B, 4x HM6116P-2, D446C-2, button battery
Crystals: (from schematics, unreadable on photo) 48.654 MHz (Y1), 3.6864 MHz (Y2)
Board is marked QVT-202.

***************************************************************************************************************/

ROM_START( qvt201 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "390410-002.u11", 0x0000, 0x4000, CRC(69337561) SHA1(022e49bf5e8d76a3c2cc5af65630d3f77cc32bc1) )
	ROM_LOAD( "390410-001.u10", 0x4000, 0x4000, CRC(977cc138) SHA1(a019980ea6da2dce53617bced420014ab4e03ec8) )

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "301847-01.u42",  0x0000, 0x1000, CRC(546ed236) SHA1(312d57a7012f50327310bd11bda000149f13342e) )
ROM_END

COMP( 1986, qvt201, 0, 0, terminals, terminals, terminals_state, 0, "Qume", "QVT-201", MACHINE_IS_SKELETON )



/**************************************************************************************************************

Relisys TR175 II.
Chips: MC68000P12, HM82C11C, SCN2681, 3x W24257-70L, KDA0476BCN-66 (RAMDAC), 4 undumped proms, Beeper, Button battery
Crystals: 28.322, 46.448, 11.0592, unknown.
Colour screen.

***************************************************************************************************************/

ROM_START( tr175 )
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD16_BYTE( "v6.05.u50", 0x00000, 0x10000, CRC(5a33b6b3) SHA1(d673f50dd88f8a154ddaabe34cfcc9ab91435a4c) )
	ROM_LOAD16_BYTE( "v6.05.u45", 0x00001, 0x10000, CRC(e220befe) SHA1(8402280577e6de4b85843222bbd6b06a3f625b3b) )
ROM_END

COMP( 1982, tr175, 0, 0, terminals, terminals, terminals_state, 0, "Relisys", "TR175II", MACHINE_IS_SKELETON )



/**************************************************************************************************************

Televideo TVI-912C.
Chips: i8035, TMS9927NL, AY5-1013A (COM2502)
Crystals: 23.814 (divide by 4 for CPU clock)
Other: 1x 8-sw DIP, 1x 10-sw DIP (internal), 2x 10-sw DIP (available to user at the back)

***************************************************************************************************************/

ROM_START( tv912c )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "a49c1.bin",    0x0000, 0x1000, CRC(d21851bf) SHA1(28fe77a218a5eee11de376f5d16e9380b616b3ca) ) // last half is all FF

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD( "a3-2.bin",     0x0000, 0x0800, CRC(bb9a7fbd) SHA1(5f1c4d41b25bd3ca4dbc336873362935daf283da) )
ROM_END

COMP( 1978, tv912c, 0, 0, terminals, terminals, terminals_state, 0, "TeleVideo", "TVI-912C", MACHINE_IS_SKELETON )



/**************************************************************************************************************

Televideo TVI-955
Chips: G65SC02P-3, 3x S6551AP, SCN2674B, AMI 131406-00 (unknown 40-pin DIL), odd round silver thing, might be a battery
Crystals: 19.3396, 31.684, 3.6864
Keyboard: M5L8049-230P-6, 5.7143, Beeper

***************************************************************************************************************/

ROM_START( tv955 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "t180002-88d_955.u4",  0x0000, 0x4000, CRC(5767fbe7) SHA1(49a2241612af5c3af09778ffa541ac0bc186e05a) )
	ROM_LOAD( "t180002-91a_calc.u5", 0x4000, 0x2000, CRC(f86c103a) SHA1(fa3ada3a5d8913e519e2ea4817e96166c1fedd32) ) // first half is all FF

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "t180002-26b.u45",     0x0000, 0x1000, CRC(69c9ebc7) SHA1(32282c816ec597a7c45e939acb7a4155d35ea584) )

	ROM_REGION(0x10000, "keyboard", 0)
	ROM_LOAD( "8049.kbd",            0x0000, 0x0800, CRC(bc86e349) SHA1(0b62003ab7931822f1bcac8370517c685849f62c) )
ROM_END

COMP( 1985, tv955, 0, 0, terminals, terminals, terminals_state, 0, "TeleVideo", "TVI-955", MACHINE_IS_SKELETON )



/**************************************************************************************************************

Televideo TVI-965
Chips: G65SC816P-5, SCN2672TC5N40, 271582-00 (unknown square chip), 2x UM6551A, Beeper
Crystals: 44.4528, 26.9892, 3.6864

***************************************************************************************************************/

ROM_START( tv965 )
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD( "180003-30h.u8", 0x00000, 0x010000, CRC(c7b9ca39) SHA1(1d95a8b0a4ea5caf3fb628c44c7a3567700a0b59) )
	ROM_LOAD( "180003-38h.u9", 0x10000, 0x008000, CRC(30fae408) SHA1(f05bb2a9ce2df60b046733f746d8d8a1eb3ac8bc) )
ROM_END

COMP( 1989, tv965, 0, 0, terminals, terminals, terminals_state, 0, "TeleVideo", "TVI-965", MACHINE_IS_SKELETON )



/**************************************************************************************************************

Visual 100. (VT-100 clone)
Chips: D780C-1 (Z80), CRT5037, D8255AC-5, uPB8214C, COM8116T-020, D8251AC, ER1400, 8-sw dip
Crystal: 47.736

***************************************************************************************************************/

ROM_START( v100 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "262-047.u108",  0x0000, 0x1000, CRC(e82f708c) SHA1(20ed83a41fd0703d72a20e170af971181cfbd575) )
	ROM_LOAD( "262-048.u110",  0x1000, 0x1000, CRC(830923d3) SHA1(108590234ff84b5856cc2784d738a2a625305953) )

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD( "241-001.u29",   0x0000, 0x0800, CRC(ef807141) SHA1(cbf3fed001811c5840b9a131d2d3133843cb3b6a) )
ROM_END

COMP( 1982, v100, 0, 0, terminals, terminals, terminals_state, 0, "Visual Technology", "Visual 100", MACHINE_IS_SKELETON )



/**************************************************************************************************************

Visual 102. (VT-102 clone plus graphics)
Chips: D780C-1 (Z80), CRT9021B-018, COM8251A, D8255AC-5, 2x CRT9006-135, CRT9007, M5L8253P-5, X2804AP-35, D7201C
Crystals: 18.575000
Keyboard: TMP8039P-6

***************************************************************************************************************/

ROM_START( v102 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "245-001.u1",  0x0000, 0x4000, CRC(c36cc525) SHA1(a45e75ded10979c8e3ad262e2cf5818e08db762c) )

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "260-001.u50", 0x0000, 0x1000, CRC(732f5b99) SHA1(d105bf9f3ed41109d7181bcf0223bb280afe3f0a) )

	ROM_REGION(0x10000, "keyboard", 0)
	ROM_LOAD( "150.kbd",     0x0000, 0x0800, CRC(afe55cff) SHA1(b26ebdde63ec0e94c08780285def39a282e128b3) )
ROM_END

COMP( 1984, v102, 0, 0, terminals, terminals, terminals_state, 0, "Visual Technology", "Visual 102", MACHINE_IS_SKELETON )



/**************************************************************************************************************

DEC VT420.
Chips: SCN8031HFCN40, SCN2681TC1N40, M5M4464AP, HM62256LFP-10T, TC23SC070AT, TC531001CP-1815
Crystals: 43.320

***************************************************************************************************************/

ROM_START( vt420 )
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD( "23-068e9-00.e2", 0x00000, 0x20000, CRC(22c3f93b) SHA1(b212911c41e4dba2e09d91fdd1f72d6c7536b0af) )
ROM_END

COMP( 1990, vt420, 0, 0, terminals, terminals, terminals_state, 0, "DEC", "VT-240", MACHINE_IS_SKELETON )



/**************************************************************************************************************

Zentec Zephyr (Model 00-441-01).
Chips: COM2017, i8085A, 2x unreadable (40-pin AMI DIP), Beeper
Crystal: 45.582000

***************************************************************************************************************/

ROM_START( zephyr )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "23-006-32c.bin",  0x0000, 0x0800, CRC(0a3a5447) SHA1(a8c25730a1d7e5b9c86e0d504afc923e931f9025) )
	ROM_LOAD( "23-067-004b.bin", 0x0800, 0x0800, CRC(37741104) SHA1(52b9998e0a8d4949e0dc7c3349b3681e13345061) )
	ROM_LOAD( "23-067-03b.bin",  0x1000, 0x0800, CRC(29cfa003) SHA1(9de7a8402173a2c448e54ee433ba3050db7b70bb) ) // this doesn't seem to fit

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD( "23-066-02a.bin",  0x0000, 0x0800, CRC(d5650b6c) SHA1(e6333e59018d9904f12abb270db4ba28aeff1995) )
ROM_END

COMP( 1979, zephyr, 0, 0, terminals, terminals, terminals_state, 0, "Zentec", "Zephyr", MACHINE_IS_SKELETON )
