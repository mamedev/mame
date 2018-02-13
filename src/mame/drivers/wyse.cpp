// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-10-27 Skeleton

Wyse terminals.

WY-30+: P8031AH, OKI M76V020, TC5565APL-12, HY6116AP-15, Beeper, 31.2795, 7.3728 (for CPU)

WY-50: SAB8031P, SCN2672A (CRTC), SCN2661B (UART), 2x MSM2128-15RS, SY2158A, 80-435-00 (WYSE proprietory gate array),
       Beeper, 4.9152 (for UART), 11.000 (for CPU), 68.850 (for video).

WY-60: SC67336P, P8051AN-40196, SCN2661B, 7219-0629, 4.9152, 11.000, 39.710, 26.580, Beeper

WY-85: 5x HM6116P-3, AM9265EPC, SCN2681A, SCN2672B, SCN8032H, 48.5568, 11.000, 3.6854, Beeper

WY-150: Philips P80C32SBPN, Toshiba 211009-02, Winbond ??, unknown crystal

        CPU             EPROM               VIDEO               RAM     Row Buffer      Font RAM    Font ROM        NVRAM       Serial      Dot Clk         CPU Clk Ser Clk
WY-30+  8031            27128(250971-02)    211019-02           5565                    6116                        CAT59C11                31.2795         7.3728
WY-50   8031            2764                2672/80-435-00      2x2Kx8  1Kx8                        2716            ER-1400     2661        68.850          11.000  4.9152
WY-55   8032            (251352-12)         211019-05           8k      (8kx2)                                      battery     16C452      49.423          14.7456
WY-60   8051(202008-03) 27512(193003-01)    211003-02/205001-02 2064    ( 2064/2016/2016/2064)                      X24C04      2661        39.710/26.580   11.000  4.9152
WY-85   8032            27128(250151-04)    2672                4x6116  6116                        9265            M5G1400     2681        48.5568         11.000  3.6864
WY-150  8032            27512(251167-06)    211009-02           6264    (2x6264)                                    battery     -           48.000          11.000


************************************************************************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"

class wyse_state : public driver_device
{
public:
	wyse_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
//      , m_maincpu(*this, "maincpu")
	{ }

void wyse(machine_config &config);
void io_map(address_map &map);
void mem_map(address_map &map);
private:
//  required_device<cpu_device> m_maincpu;
};

ADDRESS_MAP_START(wyse_state::mem_map)
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0xc000, 0xffff) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END

ADDRESS_MAP_START(wyse_state::io_map)
ADDRESS_MAP_END

static INPUT_PORTS_START( wyse )
INPUT_PORTS_END

MACHINE_CONFIG_START(wyse_state::wyse)
	MCFG_CPU_ADD("maincpu", I8031, 11'000'000) // confirmed for WY-50
	MCFG_CPU_PROGRAM_MAP(mem_map)
	MCFG_CPU_IO_MAP(io_map)
MACHINE_CONFIG_END

ROM_START( wy50 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2301_E.u6",         0x0000, 0x2000, CRC(2a62ea25) SHA1(f69c596aab307ef1872df29d353b5a61ff77bb74) )
	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "2201_B.u16",        0x0000, 0x1000, CRC(ee318814) SHA1(0ac64b60ff978e607a087e9e6f4d547811c015c5) )
ROM_END

ROM_START( wy30p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "250971-02.u4",      0x0000, 0x4000, CRC(3666549c) SHA1(23c432da2083df4b355daf566dd6514d1f9a7690) )
ROM_END

ROM_START( wy55 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "251352-12.bin",     0x0000, 0x20000, CRC(efe41862) SHA1(52ee76d636b166fa10a37356aef81011a9b079cc) ) // v2.1
ROM_END

ROM_START( wy60 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "193003-01.u9",      0x0000, 0x10000, CRC(26de0ea4) SHA1(91409f98a3990b514fbcb7de2eb45944bf5b95bc) )
	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "wy-60_4k.u6",       0x0000, 0x10000, CRC(6daf2824) SHA1(23cd039ec7ae71b0742e8eebf75be8cd5992e3fd) )
ROM_END

ROM_START( wy85 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "250151-04_revA.5e", 0x0000, 0x4000, CRC(8fcb9f43) SHA1(6c7e1d27fa6014870c29ab2b8b856ae412bfc411) )
	ROM_REGION( 0x2000, "user1", 0 )
	ROM_LOAD( "am9265.1h",         0x0000, 0x2000, CRC(5ee65b55) SHA1(a0b38a38838f262aaea22d212351e7441e4b07e8) )
ROM_END

ROM_START( wy150 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "251167-01.bin",           0x00000, 0x10000, CRC(4f425b11) SHA1(e44f54aa98d9f9c668a6ad674ec07e47879fc2a0) )
	ROM_REGION( 0x20000, "link", 0 )
	ROM_LOAD( "link_mc3.bin",            0x00000, 0x10000, CRC(9e1d37d9) SHA1(d74c0faf6cf1eb06243607931967cf35a633ac8e) )
	ROM_LOAD( "link_mc5_xerox-wy30.bin", 0x10000, 0x10000, CRC(1aa00cb4) SHA1(6a7267132fe35c8e07deccd67c0fb4fe5a240c99) )
ROM_END

ROM_START( wy160 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "251167-06.bin",           0x00000, 0x10000, CRC(36e920df) SHA1(8fb7f51b4f47ef63b21d421227d6fef98001e4e9) )
ROM_END

COMP( 1984, wy50,  0,    0, wyse, wyse, wyse_state, 0, "Wyse Technology", "WY-50",  MACHINE_IS_SKELETON )
COMP( 1986, wy60,  wy50, 0, wyse, wyse, wyse_state, 0, "Wyse Technology", "WY-60",  MACHINE_IS_SKELETON )
COMP( 1985, wy85,  wy50, 0, wyse, wyse, wyse_state, 0, "Wyse Technology", "WY-85",  MACHINE_IS_SKELETON )
COMP( 1988, wy150, wy50, 0, wyse, wyse, wyse_state, 0, "Wyse Technology", "WY-150", MACHINE_IS_SKELETON )
COMP( 1990, wy160, wy50, 0, wyse, wyse, wyse_state, 0, "Wyse Technology", "WY-160", MACHINE_IS_SKELETON )
COMP( 1991, wy30p, wy50, 0, wyse, wyse, wyse_state, 0, "Wyse Technology", "WY-30+", MACHINE_IS_SKELETON )
COMP( 1993, wy55,  wy50, 0, wyse, wyse, wyse_state, 0, "Wyse Technology", "WY-55",  MACHINE_IS_SKELETON )
