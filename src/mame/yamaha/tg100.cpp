// license:BSD-3-Clause
// copyright-holders:superctr
/*
 Yamaha TG100 AWM Tone Generator
 Skeleton written by superctr

 Roms dumped by vampirefrog
 Service manual scanned by bmos

 CPU: Hitachi HD6435208A00P (H8/520)
  - 20MHz clock
  - Mode bits 0,1 high, 2 low
  - 32kb RAM (1x HM65256 PSRAM)
 Sound generator: Yamaha YMW258-F (GEW8, identical to MultiPCM?)
  - 9.4MHz clock
  - 28 voices polyphony
  - 2MB sample ROM containing 140 12-bit PCM samples
 Effect DSP: Yamaha YM3413
  - clocked by sound generator
  - Effect memory: 64kb (2x HM65256 PSRAM)
 LCD:
  - 1x16 characters

 Other ICs:
  HG62E11R54FS (XK462A00) Gate array (LCD control, glue logic)

*/

#include "emu.h"

#include "cpu/h8500/h8520.h"
#include "sound/multipcm.h"

#include "screen.h"
#include "speaker.h"


namespace {

class tg100_state : public driver_device
{
public:
	tg100_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ymw258(*this, "ymw258")
	{ }

	void tg100(machine_config &config);

private:
	required_device<h8520_device> m_maincpu;
	required_device<multipcm_device> m_ymw258;
	void tg100_map(address_map &map);
	void ymw258_map(address_map &map);
};

/* all memory accesses are decoded by the gate array... */
void tg100_state::tg100_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).ram(); /* gate array stuff */
	map(0x00000000, 0x000000ff).rom().region("prgrom", 0x00000);
	map(0x00080000, 0x0009ffff).rom().region("prgrom", 0x00000);
}

static INPUT_PORTS_START( tg100 )
INPUT_PORTS_END

void tg100_state::ymw258_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
}

void tg100_state::tg100(machine_config &config)
{
	/* basic machine hardware */
	HD6435208(config, m_maincpu, XTAL(20'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &tg100_state::tg100_map);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	MULTIPCM(config, m_ymw258, 9400000);
	m_ymw258->set_addrmap(0, &tg100_state::ymw258_map);
	m_ymw258->add_route(0, "lspeaker", 1.0);
	m_ymw258->add_route(1, "rspeaker", 1.0);
}

ROM_START( tg100 )
	ROM_REGION(0x20000, "prgrom", 0)
	ROM_LOAD( "xk731c0.ic4", 0x00000, 0x20000, CRC(8fb6139c) SHA1(483103a2ffc63a90a2086c597baa2b2745c3a1c2) )

	ROM_REGION(0x4000, "maincpu", 0)
	ROM_COPY( "prgrom", 0x0000, 0x0000, 0x4000 )

	ROM_REGION(0x200000, "ymw258", 0)
	ROM_LOAD( "xk992a0.ic6", 0x000000, 0x200000, CRC(01dc6954) SHA1(32ec77a46f4d005538c735f56ad48fa7243c63be) )
ROM_END

} // anonymous namespace


//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY   FULLNAME      FLAGS
CONS( 1991, tg100, 0,      0,      tg100,   tg100, tg100_state, empty_init, "Yamaha", "TG100",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
