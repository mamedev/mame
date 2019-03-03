// license:BSD-3-Clause
// copyright-holders:Ivan Vangelista
// PINBALL
// Skeleton driver for Barni pinballs.
// Known pinballs to be dumped: Shield (1985)
// Hardware listing and ROM definitions from PinMAME.

/*
    Hardware:
    CPU: 2 x 6809E, optional MC6802 which may replace second 6809E
    INT: IRQ on CPU 0, FIRQ on CPU 1
    IO: 2x PIA 6821
        1x VIA 6522
    DISPLAY: 5x6 digit 7 or 16 segment display
    SOUND: basically the same as Bally's Squalk & Talk -61 board but missing AY8912 synth chip
*/

#include "emu.h"
#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "machine/6522via.h"
#include "machine/6821pia.h"
#include "sound/tms5220.h"

class barni_state : public genpin_class
{
public:
	barni_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu") { }

		void barni(machine_config &config);
		void audiocpu_map(address_map &map);
		void maincpu_map(address_map &map);
		void subcpu_map(address_map &map);
private:
	required_device<cpu_device> m_maincpu;
};

void barni_state::maincpu_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).ram();
	map(0xa100, 0xa7ff).ram();
	map(0xc000, 0xffff).rom();
}

void barni_state::subcpu_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x03ff).ram();
	map(0xe000, 0xffff).rom();
}

void barni_state::audiocpu_map(address_map &map)
{
	map(0x0000, 0x007f).ram();
	map(0xc000, 0xffff).rom();
}

static INPUT_PORTS_START( barni )
INPUT_PORTS_END

void barni_state::barni(machine_config &config)
{
	/* basic machine hardware */
	MC6809E(config, m_maincpu, XTAL(4'000'000) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &barni_state::maincpu_map);

	mc6809e_device &subcpu(MC6809E(config, "subcpu", XTAL(4'000'000) / 4));
	subcpu.set_addrmap(AS_PROGRAM, &barni_state::subcpu_map);

	m6802_cpu_device &audiocpu(M6802(config, "audiocpu", 4000000)); // uses own XTAL, but what is the value?
	audiocpu.set_addrmap(AS_PROGRAM, &barni_state::audiocpu_map);

	/* video hardware */
	//config.set_default_layout();

	//6522via
	//6821pia

	/* sound hardware */
	//tmms5220
	//dac
	genpin_audio(config);
}


/*--------------------------------
/ Champion 85
/-------------------------------*/
ROM_START(champion)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("che.bin", 0xe000, 0x2000, CRC(c5dc9228) SHA1(5306980a9c73118cfb843dbce0d56f516d054220))
	ROM_LOAD("chc.bin", 0xc000, 0x2000, CRC(6ab0f232) SHA1(0638d33f86c62ee93dff924a16a5b9309392d9e8))

	ROM_REGION(0x10000, "subcpu", 0)
	ROM_LOAD("chan.bin", 0xe000, 0x2000, CRC(3f148587) SHA1(e44dc9cce15830f522dc781aaa13c659a43371f3))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("voz1.bin", 0xf000, 0x1000, CRC(48665778) SHA1(c295dfe7f4a98756f508391eb326f37a5aac37ff))
	ROM_LOAD("voz2.bin", 0xe000, 0x1000, CRC(30e7da5e) SHA1(3054cf9b09e0f89c242e1ad35bb31d9bd77248e4))
	ROM_LOAD("voz3.bin", 0xd000, 0x1000, CRC(3cd8058e) SHA1(fa4fd0cf4124263d4021c5a86033af9e5aa66eed))
	ROM_LOAD("voz4.bin", 0xc000, 0x1000, CRC(0d00d8cc) SHA1(10f64d2fc3fc3e276bbd0e108815a3b395dcf0c9))
ROM_END


/*--------------------------------
/ Red Baron
/-------------------------------*/
ROM_START(redbarnp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("redbaron.r1", 0xe000, 0x2000, CRC(fd401d3f) SHA1(33c0f178c798e16a9b4489a0e469f0a227882e79))
	ROM_LOAD("redbaron.r2", 0xc000, 0x2000, CRC(0506e53e) SHA1(a1eaa39181cb3e5a1c281d217d680a42e39c856a))

	ROM_REGION(0x10000, "subcpu", 0)
	ROM_LOAD("redbaron.r3", 0xe000, 0x2000, CRC(45bca0b8) SHA1(77e2d6b04ea8d6fa7e30b59232696b9aa5307286))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("rbsnd1.732", 0xf000, 0x1000, CRC(674389ce) SHA1(595bbfe51dc3af266f4344e3865c0e48dd96acea))
	ROM_LOAD("rbsnd2.732", 0xe000, 0x1000, CRC(30e7da5e) SHA1(3054cf9b09e0f89c242e1ad35bb31d9bd77248e4))
	ROM_LOAD("rbsnd3.732", 0xd000, 0x1000, CRC(a4ba0f72) SHA1(e46148a2f5125914944973f37e73a62001c76aaa))
	ROM_LOAD("rbsnd4.732", 0xc000, 0x1000, CRC(fd8db899) SHA1(0978213f14f73ccc4a24eb42a39db00d9299c5d0))
ROM_END


GAME( 1985, champion, 0, barni, barni, barni_state, empty_init, ROT0, "Barni", "Champion 85",         MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1985, redbarnp, 0, barni, barni, barni_state, empty_init, ROT0, "Barni", "Red Baron (Pinball)", MACHINE_IS_SKELETON_MECHANICAL )
