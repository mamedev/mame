// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-10-27 Skeleton

Wyse terminals.

WY-60: SC67336P, P8051AN-40196, SCN2661B, 7219-0629, 4.9152, 11.000, 39.710, 26.580, Beeper

        CPU             EPROM               VIDEO               RAM     Row Buffer      Font RAM    NVRAM       Serial      Dot Clk         CPU Clk Ser Clk
WY-55   8032            (251352-12)         211019-05           8k      (8kx2)                      battery     16C452      49.423          14.7456
WY-60   8051(202008-03) 27512(193003-01)    211003-02/205001-02 2064    ( 2064/2016/2016/2064)      X24C04      2661        39.710/26.580   11.000  4.9152


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
	map(0x0000, 0xffff).rom();
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

COMP( 1986, wy60,  0,     0, wyse, wyse, wyse_state, empty_init, "Wyse Technology", "WY-60",  MACHINE_IS_SKELETON )
COMP( 1993, wy55,  0,     0, wyse, wyse, wyse_state, empty_init, "Wyse Technology", "WY-55",  MACHINE_IS_SKELETON )
