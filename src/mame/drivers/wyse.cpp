// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-10-27 Skeleton

Wyse terminals.

WY-30+: P8031AH, OKI M76V020, TC5565APL-12, HY6116AP-15, Beeper, 31.2795, 7.3728 (for CPU)

WY-60: SC67336P, P8051AN-40196, SCN2661B, 7219-0629, 4.9152, 11.000, 39.710, 26.580, Beeper

WY-150: Philips P80C32SBPN, Toshiba 211009-02, Winbond ??, unknown crystal

        CPU             EPROM               VIDEO               RAM     Row Buffer      Font RAM    NVRAM       Serial      Dot Clk         CPU Clk Ser Clk
WY-30+  8031            27128(250971-02)    211019-02           5565                    6116        CAT59C11                31.2795         7.3728
WY-55   8032            (251352-12)         211019-05           8k      (8kx2)                      battery     16C452      49.423          14.7456
WY-60   8051(202008-03) 27512(193003-01)    211003-02/205001-02 2064    ( 2064/2016/2016/2064)      X24C04      2661        39.710/26.580   11.000  4.9152
WY-120  8032            27256(250412-01)    SLA7490(211009-01)  6264/5564               6264        battery     -           48.000          11.000
WY-150  8032            27512(251167-06)    211009-02           6264    (2x6264)                    battery     -           48.000          11.000


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

private:
	void io_map(address_map &map);
	void mem_map(address_map &map);

	//  required_device<cpu_device> m_maincpu;
};

void wyse_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
}

void wyse_state::io_map(address_map &map)
{
}

static INPUT_PORTS_START( wyse )
INPUT_PORTS_END

void wyse_state::wyse(machine_config &config)
{
	i8031_device &maincpu(I8031(config, "maincpu", 11'000'000));
	maincpu.set_addrmap(AS_PROGRAM, &wyse_state::mem_map);
	maincpu.set_addrmap(AS_IO, &wyse_state::io_map);
}

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

ROM_START( wy120 ) // b&w
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wy120_ver1.4.bin", 0x000000, 0x010000, CRC(6de23624) SHA1(ad90087237347662b5ae4fcc8a05d66d76c46a26) )
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

COMP( 1986, wy60,  0,     0, wyse, wyse, wyse_state, empty_init, "Wyse Technology", "WY-60",  MACHINE_IS_SKELETON )
COMP( 1987, wy120, 0,     0, wyse, wyse, wyse_state, empty_init, "Wyse Technology", "WY-120", MACHINE_IS_SKELETON )
COMP( 1988, wy150, 0,     0, wyse, wyse, wyse_state, empty_init, "Wyse Technology", "WY-150", MACHINE_IS_SKELETON )
COMP( 1990, wy160, 0,     0, wyse, wyse, wyse_state, empty_init, "Wyse Technology", "WY-160", MACHINE_IS_SKELETON )
COMP( 1991, wy30p, 0,     0, wyse, wyse, wyse_state, empty_init, "Wyse Technology", "WY-30+", MACHINE_IS_SKELETON )
COMP( 1993, wy55,  0,     0, wyse, wyse, wyse_state, empty_init, "Wyse Technology", "WY-55",  MACHINE_IS_SKELETON )
