// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Yamaha MU-5 Tone Generator
    Preliminary driver by R. Belmont

    CPU: H8/3002 (HD6413002F10)
    Sound: YMW-258-F (aka "MultiPCM")
*/

#include "emu.h"

#include "cpu/h8/h83002.h"
#include "sound/multipcm.h"

#include "screen.h"
#include "speaker.h"

class mu5_state : public driver_device
{
public:
	mu5_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ymw258(*this, "ymw258")
	{ }

	void mu5(machine_config &config);

private:
	required_device<h83002_device> m_maincpu;
	required_device<multipcm_device> m_ymw258;
	void mu5_map(address_map &map);
	void ymw258_map(address_map &map);
};

void mu5_state::mu5_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom().region("maincpu", 0);
	map(0x200000, 0x21ffff).ram();
	map(0x400000, 0x400007).rw(m_ymw258, FUNC(multipcm_device::read), FUNC(multipcm_device::write)).umask16(0xff00);
}

void mu5_state::ymw258_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
}

static INPUT_PORTS_START( mu5 )
INPUT_PORTS_END

void mu5_state::mu5(machine_config &config)
{
	/* basic machine hardware */
	H83002(config, m_maincpu, 10_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mu5_state::mu5_map);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	MULTIPCM(config, m_ymw258, 9400000);
	m_ymw258->set_addrmap(0, &mu5_state::ymw258_map);
	m_ymw258->add_route(0, "lspeaker", 1.0);
	m_ymw258->add_route(1, "rspeaker", 1.0);
}

ROM_START( mu5 )
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("yamaha_mu5_program_xq201a0.bin", 0x000000, 0x020000, CRC(e9b3ec26) SHA1(cfee69699de78e2792dac24d428a120021ba147b))

	ROM_REGION(0x200000, "ymw258", 0)
	ROM_LOAD("yamaha_mu5_waverom_xp50280-801.bin", 0x000000, 0x200000, CRC(e0913030) SHA1(369f8df4942b6717c142ca8c4913e556dafae187))
ROM_END

CONS(1994, mu5, 0, 0, mu5, mu5, mu5_state, empty_init, "Yamaha", "MU-5", MACHINE_NOT_WORKING )
