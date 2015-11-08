// license:BSD-3-Clause
// copyright-holders:Robbbert
/*****************************************************************************************

  PINBALL
  Mephisto

******************************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "cpu/mcs51/mcs51.h"

// mephisto_state was also defined in mess/drivers/mephisto.c
class mephisto_pinball_state : public driver_device
{
public:
	mephisto_pinball_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
public:
	DECLARE_DRIVER_INIT(mephisto);
};


static ADDRESS_MAP_START( mephisto_map, AS_PROGRAM, 8, mephisto_pinball_state )
	AM_RANGE(0x00000, 0x0ffff) AM_ROM
	AM_RANGE(0x10000, 0x1ffff) AM_RAM
	//AM_RANGE(0x12000, 0x1201f) io (switches & lamps)
	AM_RANGE(0xf8000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mephisto_cpu2, AS_PROGRAM, 8, mephisto_pinball_state )
	AM_RANGE(0x00000, 0x07fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mephisto_cpu2_io, AS_IO, 8, mephisto_pinball_state )
	//AM_RANGE(0x0000, 0x07ff) AM_WRITE
ADDRESS_MAP_END

static INPUT_PORTS_START( mephisto )
INPUT_PORTS_END

void mephisto_pinball_state::machine_reset()
{
}

DRIVER_INIT_MEMBER(mephisto_pinball_state,mephisto)
{
}

static MACHINE_CONFIG_START( mephisto, mephisto_pinball_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8088, 8000000)
	MCFG_CPU_PROGRAM_MAP(mephisto_map)
	MCFG_CPU_ADD("cpu2", I8051, 12000000)
	MCFG_CPU_PROGRAM_MAP(mephisto_cpu2)
	MCFG_CPU_IO_MAP(mephisto_cpu2_io)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Mephisto
/-------------------------------------------------------------------*/
ROM_START(mephistp)
	ROM_REGION(0x1000000, "maincpu", 0)
	ROM_LOAD("cpu_ver1.2", 0x00000, 0x8000, CRC(845c8eb4) SHA1(2a705629990950d4e2d3a66a95e9516cf112cc88))
	ROM_RELOAD(0x08000, 0x8000)
	ROM_RELOAD(0xf8000, 0x8000)

	ROM_REGION(0x20000, "cpu2", 0)
	ROM_LOAD("ic15_02", 0x00000, 0x8000, CRC(2accd446) SHA1(7297e4825c33e7cf23f86fe39a0242e74874b1e2))

	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("ic14_s0", 0x00000, 0x8000, CRC(7cea3018) SHA1(724fe7a4456cbf2ac01466d946668ee86f4410ae))
	ROM_LOAD("ic13_s1", 0x08000, 0x8000, CRC(5a9e0f1d) SHA1(dbfd307706c51f8809f4867a199b4b62beb64379))
	ROM_LOAD("ic12_s2", 0x10000, 0x8000, CRC(b3cc962a) SHA1(521376cab7e917a5d5f5f183bccb21bd13327c48))
	ROM_LOAD("ic11_s3", 0x18000, 0x8000, CRC(8aaa21ec) SHA1(29f17249cac62128fd8b0eee415ce399ee2ec672))
	ROM_LOAD("ic16_c", 0x20000, 0x8000, CRC(5f12b4f4) SHA1(73fbdb57fca0dbc918e6665a6cb949e741f2720a))
	ROM_LOAD("ic17_d", 0x28000, 0x8000, CRC(d17e18a8) SHA1(372eaf209ea5d26f3c096aadd7d028ef68bfb68e))
	ROM_LOAD("ic18_e", 0x30000, 0x8000, CRC(eac6dbba) SHA1(f4971c8b0aa3a72c396b943a0ee3094afb902ec1))
	ROM_LOAD("ic19_f", 0x38000, 0x8000, CRC(cc4bb629) SHA1(db46be2a8034bbd106b7dd80f50988c339684b5e))
ROM_END

ROM_START(mephistp1)
	ROM_REGION(0x1000000, "maincpu", 0)
	ROM_LOAD("cpu_ver1.1", 0x00000, 0x8000, CRC(ce584902) SHA1(dd05d008bbd9b6588cb204e8d901537ffe7ddd43))
	ROM_RELOAD(0x08000, 0x8000)
	ROM_RELOAD(0xf8000, 0x8000)

	ROM_REGION(0x20000, "cpu2", 0)
	ROM_LOAD("ic15_02", 0x00000, 0x8000, CRC(2accd446) SHA1(7297e4825c33e7cf23f86fe39a0242e74874b1e2))

	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("ic14_s0", 0x00000, 0x8000, CRC(7cea3018) SHA1(724fe7a4456cbf2ac01466d946668ee86f4410ae))
	ROM_LOAD("ic13_s1", 0x08000, 0x8000, CRC(5a9e0f1d) SHA1(dbfd307706c51f8809f4867a199b4b62beb64379))
	ROM_LOAD("ic12_s2", 0x10000, 0x8000, CRC(b3cc962a) SHA1(521376cab7e917a5d5f5f183bccb21bd13327c48))
	ROM_LOAD("ic11_s3", 0x18000, 0x8000, CRC(8aaa21ec) SHA1(29f17249cac62128fd8b0eee415ce399ee2ec672))
	ROM_LOAD("ic16_c", 0x20000, 0x8000, CRC(5f12b4f4) SHA1(73fbdb57fca0dbc918e6665a6cb949e741f2720a))
	ROM_LOAD("ic17_d", 0x28000, 0x8000, CRC(d17e18a8) SHA1(372eaf209ea5d26f3c096aadd7d028ef68bfb68e))
	ROM_LOAD("ic18_e", 0x30000, 0x8000, CRC(eac6dbba) SHA1(f4971c8b0aa3a72c396b943a0ee3094afb902ec1))
	ROM_LOAD("ic19_f", 0x38000, 0x8000, CRC(cc4bb629) SHA1(db46be2a8034bbd106b7dd80f50988c339684b5e))
ROM_END


GAME(1986,  mephistp,   0,         mephisto,  mephisto, mephisto_pinball_state,  mephisto,  ROT0,  "Stargame",    "Mephisto (rev. 1.2)",     MACHINE_IS_SKELETON_MECHANICAL)
GAME(1986,  mephistp1,  mephistp,  mephisto,  mephisto, mephisto_pinball_state,  mephisto,  ROT0,  "Stargame",    "Mephisto (rev. 1.1)",     MACHINE_IS_SKELETON_MECHANICAL)
