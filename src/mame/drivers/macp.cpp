// license:BSD-3-Clause
// copyright-holders:Ivan Vangelista
// PINBALL
// Skeleton driver for MAC S.A. and CICPlay pinballs. ROM definitions taken from PinMAME.

#include "emu.h"
#include "machine/genpin.h"
#include "cpu/z80/z80.h"
#include "machine/i8279.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"

class macp_state : public genpin_class
{
public:
	macp_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu") { }

		void macp(machine_config &config);
		void macpmsm(machine_config &config);
		void macp0(machine_config &config);
		void macp0_map(address_map &map);
		void macp_io(address_map &map);
		void macp_map(address_map &map);
private:
	required_device<cpu_device> m_maincpu;
};

void macp_state::macp_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xc7ff).ram();
}

void macp_state::macp0_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
}

void macp_state::macp_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
}

static INPUT_PORTS_START( macp )
INPUT_PORTS_END

static INPUT_PORTS_START( cicplay )
INPUT_PORTS_END

void macp_state::macp(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 2500000);
	m_maincpu->set_addrmap(AS_PROGRAM, &macp_state::macp_map);
	m_maincpu->set_addrmap(AS_IO, &macp_state::macp_io);

	/* video hardware */
	//config.set_default_layout();

	//I8279

	/* sound hardware */
	//2x AY8910
	genpin_audio(config);
}

void macp_state::macp0(machine_config &config)
{
	macp(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &macp_state::macp0_map);
}

void macp_state::macpmsm(machine_config &config)
{
	macp(config);
	// MSM5205
}

ROM_START(macgalxy)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("galaxy1.bin", 0x0000, 0x2000, CRC(00c71e67) SHA1(c1ad1dacae2b90f516c732bfdf8244908f67e15a))
	ROM_LOAD("galaxy2.bin", 0x2000, 0x2000, CRC(f0efb723) SHA1(697b3c9f3ebedca1087354eda5dfe9719d497045))
ROM_END

ROM_START(macjungl)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("jungle1.bin", 0x0000, 0x2000, CRC(461a3e1b) SHA1(96981b4d8db0412c474169eaf5e5386be5006ffe))
	ROM_LOAD("jungle2.bin", 0x2000, 0x2000, CRC(26b53e6e) SHA1(e588787b2381c0e6a42590f0e7d18d2a74ebf5f0))
ROM_END

ROM_START(spctrain)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("mbm27128.25", 0x0000, 0x4000, CRC(d65c5c36) SHA1(6f350b48daaecd36b3086e682ec6ee174f297a34))
ROM_END

ROM_START(spcpnthr)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("sp_game.bin", 0x0000, 0x8000, CRC(0428563c) SHA1(45b9daf12f8384101450f1e529491812f73d88bd))

	ROM_REGION(0x8000, "msm5205", 0)
	ROM_LOAD("mac_snd.bin", 0x0000, 0x8000, CRC(d7aedbac) SHA1(4b59028e08b2d7ff8f19596022ba6e830cf736e2))
ROM_END

ROM_START(mac_1808)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("mac_1808.cpu", 0x0000, 0x8000, CRC(29126585) SHA1(b24c4f0f17f3ef7de5348cb06ec3b305c6ca7373))

	ROM_REGION(0x8000, "msm5205", 0)
	ROM_LOAD("mac_snd.bin", 0x0000, 0x8000, CRC(d7aedbac) SHA1(4b59028e08b2d7ff8f19596022ba6e830cf736e2))
ROM_END

ROM_START(macjungn)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("juego1.bin", 0x0000, 0x8000, CRC(0b2d9b64) SHA1(22602b79c8b178793b447783bca59dcb49e4525f))

	ROM_REGION(0x8000, "msm5205", 0)
	ROM_LOAD("mac_snd.bin", 0x0000, 0x8000, CRC(d7aedbac) SHA1(4b59028e08b2d7ff8f19596022ba6e830cf736e2))
ROM_END

ROM_START(nbamac)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("nba_mac.cpu", 0x0000, 0x8000, CRC(0c430988) SHA1(71126d9caf10ac27056b8bf28d300775062dc693))

	ROM_REGION(0x8000, "msm5205", 0)
	ROM_LOAD("mac_snd.bin", 0x0000, 0x8000, CRC(d7aedbac) SHA1(4b59028e08b2d7ff8f19596022ba6e830cf736e2))
ROM_END

ROM_START(glxplay)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("1083-1.cpu", 0x0000, 0x2000, CRC(3df33169) SHA1(657720aab4cccf3364f013acb3f5dbc46fe0e05c))
	ROM_LOAD("1083-2.cpu", 0x2000, 0x2000, CRC(47b4f49e) SHA1(59853ac56bb9e2dc7b848dc46ebd27c21b9d2e82))
ROM_END

ROM_START(kidnap)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("kidnap_1.bin", 0x0000, 0x2000, CRC(4b8f9bb1) SHA1(16672c1a5e55ba5963fbd8834443dbead9bdff10) BAD_DUMP)
	ROM_LOAD("kidnap_2.bin", 0x2000, 0x2000, CRC(4333d9ba) SHA1(362bcc9caaf37ad7efc116c3bee9b99cbbfa0563))
ROM_END

ROM_START(glxplay2)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("1382-1.cpu", 0x0000, 0x2000, CRC(da43b0b9) SHA1(b13b260c61b3bd0b7632aabcdbcf4cdd5cbe4b22))
	ROM_LOAD("1382-2.cpu", 0x2000, 0x2000, CRC(945c90fd) SHA1(8367992f8db8b402d82e4a3f02a35b796756ce0f))
ROM_END

// MAC S.A. pinballs
GAME( 1986, macgalxy, 0, macp0,   macp,    macp_state, empty_init, ROT0, "MAC S.A.", "MAC's Galaxy",             MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, macjungl, 0, macp0,   macp,    macp_state, empty_init, ROT0, "MAC S.A.", "MAC Jungle",               MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, spctrain, 0, macp,    macp,    macp_state, empty_init, ROT0, "MAC S.A.", "Space Train (Pinball)",    MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1988, spcpnthr, 0, macpmsm, macp,    macp_state, empty_init, ROT0, "MAC S.A.", "Space Panther",            MACHINE_IS_SKELETON_MECHANICAL )
GAME( 19??, mac_1808, 0, macpmsm, macp,    macp_state, empty_init, ROT0, "MAC S.A.", "unknown game (MAC #1808)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1995, macjungn, 0, macpmsm, macp,    macp_state, empty_init, ROT0, "MAC S.A.", "MAC Jungle (New version)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1996, nbamac,   0, macpmsm, macp,    macp_state, empty_init, ROT0, "MAC S.A.", "NBA MAC",                  MACHINE_IS_SKELETON_MECHANICAL )

// CICPlay pinballs
GAME( 1985, glxplay,  0, macp0,   cicplay, macp_state, empty_init, ROT0, "CICPlay", "Galaxy Play",   MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1986, kidnap,   0, macp0,   cicplay, macp_state, empty_init, ROT0, "CICPlay", "Kidnap",        MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, glxplay2, 0, macp0,   cicplay, macp_state, empty_init, ROT0, "CICPlay", "Galaxy Play 2", MACHINE_IS_SKELETON_MECHANICAL )
