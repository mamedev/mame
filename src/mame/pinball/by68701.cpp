// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/********************************************************************************************************************
PINBALL
Bally prototyping system with MC68701 CPU.
It eventually emerged 5 years later as BY6803.


Status:
- Skeletons

ToDo:
- Everything

*********************************************************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6801.h"


namespace {

class by68701_state : public driver_device
{
public:
	by68701_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	void by68701(machine_config &config);
	void by68701_map(address_map &map) ATTR_COLD;
protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset() override ATTR_COLD;
public:
	void init_by68701();
};


void by68701_state::by68701_map(address_map &map)
{
	map(0x0000, 0xffff).noprw();
	map(0x0400, 0x04ff).ram();
	map(0x0500, 0x07ff).ram();
	map(0x7000, 0xffff).rom();
}

static INPUT_PORTS_START( by68701 )
INPUT_PORTS_END

void by68701_state::machine_reset()
{
}

void by68701_state::init_by68701()
{
}

void by68701_state::by68701(machine_config &config)
{
	/* basic machine hardware */
	M6803(config, m_maincpu, 3579545/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &by68701_state::by68701_map);
}

/*------------------
/ Flash Gordon
/------------------*/
ROM_START(flashgdnp1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "fg68701.bin", 0xf800, 0x0800, CRC(e52da294) SHA1(0191ae821fbeae40192d858ca7f2dccda84de73f))
	ROM_LOAD( "xxx-xx.u10", 0xc000, 0x1000, CRC(3e9fb30f) SHA1(173cd9e55e9c954944aa504308564e4842646e55))
	ROM_LOAD( "xxx-xx.u11", 0xa000, 0x1000, CRC(8b0ae6d8) SHA1(2380bd6d354c204153fd44534d617f7be000e46f))
	ROM_LOAD( "xxx-xx.u12", 0x8000, 0x1000, CRC(57406a1f) SHA1(01986e8d879071374d6f94ae6fce5832eb89f160))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("834-20_2.532", 0xc000, 0x1000, CRC(2f8ced3e) SHA1(ecdeb07c31c22ec313b55774f4358a9923c5e9e7))
	ROM_LOAD("834-18_5.532", 0xf000, 0x1000, CRC(8799e80e) SHA1(f255b4e7964967c82cfc2de20ebe4b8d501e3cb0))
ROM_END

ROM_START(flashgdnp2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "fg6801.bin", 0xf800, 0x0800, CRC(8af7bf77) SHA1(fd65578b2340eb207b2e197765e6721473340565) BAD_DUMP)
	ROM_LOAD( "xxx-xx2.u10", 0xc000, 0x1000, CRC(0fff825c) SHA1(3c567aa8ec04a8ff9a09b530b6d324fdbe363ab6))
	ROM_LOAD( "xxx-xx2.u11", 0xa000, 0x1000, CRC(e34b113a) SHA1(b2860d284995db0ec59b22b434ccb9f6721e7d9d))
	ROM_LOAD( "xxx-xx2.u12", 0x8000, 0x1000, CRC(12bf4e19) SHA1(b0acf069d86f7472728610834ef3ab6da83b4b67))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("834-20_2.532", 0xc000, 0x1000, CRC(2f8ced3e) SHA1(ecdeb07c31c22ec313b55774f4358a9923c5e9e7))
	ROM_LOAD("834-18_5.532", 0xf000, 0x1000, CRC(8799e80e) SHA1(f255b4e7964967c82cfc2de20ebe4b8d501e3cb0))
ROM_END

/*------------------
/ Eight Ball Deluxe
/------------------*/
ROM_START(eballdlxp1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "ebd68701.1", 0xf800, 0x0800, CRC(2c693091) SHA1(93ae424d6a43424e8ea023ef555f6a4fcd06b32f))
	ROM_LOAD( "720-61.u10", 0xd000, 0x1000, CRC(ac646e58) SHA1(85694264a739118ed249d97c04fe8e9f6edfdd33))
	ROM_LOAD( "720-62.u14", 0xc000, 0x1000, CRC(b6476a9b) SHA1(1dc92125422908e829ce17aaed5ad49b0dbda0e5))
	ROM_LOAD( "720-63.u13", 0x7000, 0x1000, CRC(f5d751fd) SHA1(4ab5975d52cdde0e05f2bbea7dcd732882fb1dd5))
	ROM_LOAD( "838-19.u12", 0xa000, 0x1000, CRC(dfba8976) SHA1(70c19237842ee73cfc8e9607df79a59424d90d99))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("838-08_3.532", 0xd000, 0x1000, CRC(c39478d7) SHA1(8148aca7c4113921ab882da32d6d88e66abb22cc))
	ROM_LOAD("838-09_4.716", 0xe000, 0x0800, CRC(518ea89e) SHA1(a387274ef530bb57f31819733b35615a39260126))
	ROM_RELOAD(0xe800, 0x0800)
	ROM_LOAD("838-16_5.532", 0xf000, 0x1000, CRC(63d92025) SHA1(2f8e8435326a39064b99b9971b0d8944586571fb))
ROM_END
ROM_START(eballdlxp2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "ebd68701.1", 0xf800, 0x0800, CRC(2c693091) SHA1(93ae424d6a43424e8ea023ef555f6a4fcd06b32f))
	ROM_LOAD( "720-56.u10", 0xd000, 0x1000, CRC(65a3a02b) SHA1(6fa8667509d314f521dce63d9f1b7fc132d85a1f))
	ROM_LOAD( "720-57.u14", 0xc000, 0x1000, CRC(a7d96074) SHA1(04726af863a2c7589308725f3183112b5e1f84ac))
	ROM_LOAD( "720-58.u13", 0x7000, 0x1000, CRC(c9585f1f) SHA1(a38b059bb7ef15fccb54bec58d88dd15182b66a6))
	ROM_LOAD( "838-18.u12", 0xa000, 0x1000, CRC(20fa35e5) SHA1(d8808aa357d2a20fc235da7c80f78c8e5d805ac3))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("838-08_3.532", 0xd000, 0x1000, CRC(c39478d7) SHA1(8148aca7c4113921ab882da32d6d88e66abb22cc))
	ROM_LOAD("838-09_4.716", 0xe000, 0x0800, CRC(518ea89e) SHA1(a387274ef530bb57f31819733b35615a39260126))
	ROM_RELOAD(0xe800, 0x0800)
	ROM_LOAD("838-16_5.532", 0xf000, 0x1000, CRC(63d92025) SHA1(2f8e8435326a39064b99b9971b0d8944586571fb))
ROM_END
ROM_START(eballdlxp3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "ebd68701.1", 0xf800, 0x0800, CRC(2c693091) SHA1(93ae424d6a43424e8ea023ef555f6a4fcd06b32f))
	ROM_LOAD( "720-xx.u10", 0xd000, 0x1000, CRC(6da34581) SHA1(6e005ceda9a4a23603d5243dfca85ccd3f0e425a))
	ROM_LOAD( "720-xx.u14", 0xc000, 0x1000, CRC(7079648a) SHA1(9d91cd18fb68f165498de8ac51c1bc2a35bd9468))
	ROM_LOAD( "xxx-xx.u13", 0x7000, 0x1000, CRC(bda2c78b) SHA1(d5e7d0dd3d44d63b9d4b43bf5f63917b80a7ce23))
	ROM_LOAD( "838-18.u12", 0xa000, 0x1000, CRC(20fa35e5) SHA1(d8808aa357d2a20fc235da7c80f78c8e5d805ac3))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("838-08_3.532", 0xd000, 0x1000, CRC(c39478d7) SHA1(8148aca7c4113921ab882da32d6d88e66abb22cc))
	ROM_LOAD("838-09_4.716", 0xe000, 0x0800, CRC(518ea89e) SHA1(a387274ef530bb57f31819733b35615a39260126))
	ROM_RELOAD(0xe800, 0x0800)
	ROM_LOAD("838-16_5.532", 0xf000, 0x1000, CRC(63d92025) SHA1(2f8e8435326a39064b99b9971b0d8944586571fb))
ROM_END
ROM_START(eballdlxp4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "ebd68701.1", 0xf800, 0x0800, CRC(2c693091) SHA1(93ae424d6a43424e8ea023ef555f6a4fcd06b32f))
	ROM_LOAD( "720-54.u10", 0xd000, 0x1000, CRC(9facc547) SHA1(a10d7747918b3a1d87bd9caa19e87739631a7566))
	ROM_LOAD( "720-55.u14", 0xc000, 0x1000, CRC(99080832) SHA1(e1d416b4910ed31b40bde0860e698f0cbe46cc57))
	ROM_LOAD( "838-17.u12", 0xa000, 0x1000, CRC(43ebffc6) SHA1(bffd41c68430889b3926db9b05c5991185c28053))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("838-08_3.532", 0xd000, 0x1000, CRC(c39478d7) SHA1(8148aca7c4113921ab882da32d6d88e66abb22cc))
	ROM_LOAD("838-09_4.716", 0xe000, 0x0800, CRC(518ea89e) SHA1(a387274ef530bb57f31819733b35615a39260126))
	ROM_RELOAD(0xe800, 0x0800)
	ROM_LOAD("838-16_5.532", 0xf000, 0x1000, CRC(63d92025) SHA1(2f8e8435326a39064b99b9971b0d8944586571fb))
ROM_END

} // anonymous namespace


GAME( 1981, flashgdnp1, flashgdn, by68701, by68701, by68701_state, init_by68701, ROT0, "Bally", "Flash Gordon (prototype rev. 1)",       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME( 1981, flashgdnp2, flashgdn, by68701, by68701, by68701_state, init_by68701, ROT0, "Bally", "Flash Gordon (prototype rev. 2)",       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME( 1981, eballdlxp1, eballdlx, by68701, by68701, by68701_state, init_by68701, ROT0, "Bally", "Eight Ball Deluxe (prototype rev. 1)",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME( 1981, eballdlxp2, eballdlx, by68701, by68701, by68701_state, init_by68701, ROT0, "Bally", "Eight Ball Deluxe (prototype rev. 2)",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME( 1981, eballdlxp3, eballdlx, by68701, by68701, by68701_state, init_by68701, ROT0, "Bally", "Eight Ball Deluxe (prototype rev. 3)",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME( 1981, eballdlxp4, eballdlx, by68701, by68701, by68701_state, init_by68701, ROT0, "Bally", "Eight Ball Deluxe (prototype rev. 4)",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
