// license:BSD-3-Clause
// copyright-holders:

/*
    Skeleton driver for Namco Sweet Land screenless crane machine. Supposed to be the 1986 version.
    PCB silkscreened I602961100 (I602971100), with a T0544 sticker

    Main components:
    1x Z80A CPU
    2x 8255
    1x YM2203C
    1x 6 dips bank
*/

#include "emu.h"
#include "speaker.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/2203intf.h"

class sweetland_state : public driver_device
{
public:
	sweetland_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		{ }

	void sweetland(machine_config &config);

private:
	void program_map(address_map &map);
};

void sweetland_state::program_map(address_map &map) // TODO: check everything
{
	map(0x0000, 0x3fff).rom().region("maincpu", 0);
	map(0x4000, 0x47ff).ram();
	map(0x8000, 0x8001).w("ym", FUNC(ym2203_device::write));
	map(0x9000, 0x9003).rw("8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xa000, 0xa003).rw("8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	//map(0xc000, 0xc000)
}


static INPUT_PORTS_START( sweetland )
INPUT_PORTS_END

void sweetland_state::sweetland(machine_config &config)
{
	z80_device &maincpu(Z80(config, "maincpu", 4000000)); // unknown clock
	maincpu.set_addrmap(AS_PROGRAM, &sweetland_state::program_map);

	I8255(config, "8255_0");

	I8255(config, "8255_1");

	SPEAKER(config, "mono").front_center();

	YM2203(config, "ym", 4000000).add_route(ALL_OUTPUTS, "mono", 0.50); // unknown clock
}


ROM_START(sweetlnd)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD( "sw1.5h", 0x0000, 0x4000, CRC(6e361ae5) SHA1(b12ce39aa8235993d0acdc623ff8dbce9feb146e) )
ROM_END


GAME( 1986?, sweetlnd, 0, sweetland, sweetland, sweetland_state, empty_init, ROT0, "Namco", "Sweet Land", MACHINE_IS_SKELETON_MECHANICAL )
