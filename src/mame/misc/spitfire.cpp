// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"

#include "cpu/f8/f8.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class spitfire_state : public driver_device
{
public:
	spitfire_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void spitfire(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void prg_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};

void spitfire_state::video_start()
{
}

uint32_t spitfire_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void spitfire_state::prg_map(address_map &map)
{
	map(0x0000, 0x07ff).rom();
}

void spitfire_state::io_map(address_map &map)
{
}

static INPUT_PORTS_START( spitfire )
INPUT_PORTS_END

void spitfire_state::machine_start()
{
}

void spitfire_state::machine_reset()
{
}

void spitfire_state::spitfire(machine_config &config)
{
	F8(config, m_maincpu, 2000000); // 2 chip CPU setup, one is a MK3?60P-1, other unreadable.  Unknown frequency
	m_maincpu->set_addrmap(AS_PROGRAM, &spitfire_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &spitfire_state::io_map);

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 16, 256-16-1);
	screen.set_screen_update(FUNC(spitfire_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xRGB_444, 0x100).set_endianness(ENDIANNESS_BIG);

	SPEAKER(config, "mono").front_center();
}

ROM_START( spitfire )
	ROM_REGION( 0x800, "maincpu", 0 )
	// near the CPU
	ROM_LOAD( "sf_p1a.h1", 0x000, 0x200, CRC(75e294f1) SHA1(f0cb60bd7273ca36fb0f97f1b1b69a4493437c97) )
	//ROM_LOAD( "sf_p2.h2",  0x200, 0x200, CRC(3f86e112) SHA1(01bba52db889fef70ea09fb2bb3de9142f40f49a) ) // FIXED BITS (xxxx1xxx)
	ROM_LOAD( "sf_p2.h2",  0x200, 0x200, CRC(242dc618) SHA1(b5fe5ce084b88de2d656f749a932396defc46313) ) // taken from set below, same data without the fixed bits problem
	ROM_LOAD( "sf_p3.j1",  0x400, 0x200, CRC(88caf987) SHA1(c6cf88c95fc78292678a893b4685659b27c322f5) )
	ROM_LOAD( "sf_p4.j2",  0x600, 0x200, CRC(09afd5ee) SHA1(b3e7403e10ae4764890635324f016f81e291f10a) )

	ROM_REGION( 0x400, "data", 0 ) // or GFX?
	// on the opposite side of the PCB to the CPU
	ROM_LOAD( "sf_f1.a3",  0x000, 0x200, CRC(4ed940b5) SHA1(09da6c1df5f9264c226a204fb895e866f5b85138) )
	ROM_LOAD( "sf_f2.a4",  0x200, 0x200, CRC(99a1230b) SHA1(f88aa9e71031fe504cf444df65a02283c45d3f31) )
ROM_END

ROM_START( spitfirea )
	ROM_REGION( 0x800, "maincpu", 0 )
	// near the CPU
	//ROM_LOAD( "sf_p1a.h1", 0x000, 0x200, CRC(25588f41) SHA1(783daf04f357122e909b4dbc351a3effe491a4dc) ) // BADADDR xxxxx-xxx
	ROM_LOAD( "sf_p1a.h1", 0x000, 0x200, CRC(75e294f1) SHA1(f0cb60bd7273ca36fb0f97f1b1b69a4493437c97) ) // taken from set above, same data without the badaddr problem
	ROM_LOAD( "sf_p2.h2",  0x200, 0x200, CRC(242dc618) SHA1(b5fe5ce084b88de2d656f749a932396defc46313) )
	ROM_LOAD( "sf_p3.j1",  0x400, 0x200, CRC(88caf987) SHA1(c6cf88c95fc78292678a893b4685659b27c322f5) )
	ROM_LOAD( "sf_p4a.j2", 0x600, 0x200, CRC(13920a83) SHA1(2ce3884898c1851e83025eaedef846f591263a64) ) // different label, unique ROM

	ROM_REGION( 0x400, "data", 0 ) // or GFX?
	// on the opposite side of the PCB to the CPU
	ROM_LOAD( "sf_f1.a3",  0x000, 0x200, CRC(4ed940b5) SHA1(09da6c1df5f9264c226a204fb895e866f5b85138) )
	ROM_LOAD( "sf_f2.a4",  0x200, 0x200, CRC(99a1230b) SHA1(f88aa9e71031fe504cf444df65a02283c45d3f31) )
ROM_END

} // anonymous namespace

GAME( 1976, spitfire,  0,        spitfire, spitfire,  spitfire_state, empty_init, ROT0, "Innovative Coin Company", "Spitfire (set 1)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1976, spitfirea, spitfire, spitfire, spitfire,  spitfire_state, empty_init, ROT0, "Innovative Coin Company", "Spitfire (set 2)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
