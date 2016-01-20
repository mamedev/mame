// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/**********************************************************************************

    Pinball
    Playmatic MPU 3

***********************************************************************************/


#include "emu.h"
#include "cpu/cosmac/cosmac.h"

class play_3_state : public driver_device
{
public:
	play_3_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cosmac_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset() override;
public:
	DECLARE_DRIVER_INIT(play_3);
};


static ADDRESS_MAP_START( play_3_map, AS_PROGRAM, 8, play_3_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( play_3 )
INPUT_PORTS_END

void play_3_state::machine_reset()
{
}

DRIVER_INIT_MEMBER(play_3_state,play_3)
{
}

static MACHINE_CONFIG_START( play_3, play_3_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", CDP1802, 2950000)
	MCFG_CPU_PROGRAM_MAP(play_3_map)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Meg Aaton (1983)
/-------------------------------------------------------------------*/

ROM_START(megaaton)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpumegat.bin", 0x0000, 0x2000, CRC(7e7a4ede) SHA1(3194b367cbbf6e0cb2629cd5d82ddee6fe36985a))
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x8000, 0x2000)
	ROM_RELOAD(0xc000, 0x2000)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("smogot.bin", 0x0000, 0x2000, CRC(fefc3ab2) SHA1(e748d9b443a69fcdd587f22c87d41818b6c0e436))
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x8000, 0x2000)
	ROM_RELOAD(0xc000, 0x2000)
	ROM_LOAD("smegat.bin", 0x2000, 0x1000, CRC(910ab7fe) SHA1(0ddfd15c9c25f43b8fcfc4e11bc8ea180f6bd761))
	ROM_RELOAD(0x6000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
ROM_END

ROM_START(megaatona)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mega_u12.bin", 0x0000, 0x1000, CRC(65761b02) SHA1(dd9586eaf70698ef7a80ce1be293322f64829aea))
	ROM_RELOAD(0x4000, 0x1000)
	ROM_RELOAD(0x8000, 0x1000)
	ROM_RELOAD(0xc000, 0x1000)
	ROM_LOAD("mega_u11.bin", 0x1000, 0x1000, CRC(513f3683) SHA1(0f080a33426df1ffdb14e9b2e6382304e201e335))
	ROM_RELOAD(0x5000, 0x1000)
	ROM_RELOAD(0x9000, 0x1000)
	ROM_RELOAD(0xd000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("smogot.bin", 0x0000, 0x2000, CRC(fefc3ab2) SHA1(e748d9b443a69fcdd587f22c87d41818b6c0e436))
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x8000, 0x2000)
	ROM_RELOAD(0xc000, 0x2000)
	ROM_LOAD("smegat.bin", 0x2000, 0x1000, CRC(910ab7fe) SHA1(0ddfd15c9c25f43b8fcfc4e11bc8ea180f6bd761))
	ROM_RELOAD(0x6000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
ROM_END

GAME(1983,  megaaton,  0,  play_3,  play_3, play_3_state,  play_3,  ROT0,  "Playmatic",    "Meg-Aaton",     MACHINE_IS_SKELETON_MECHANICAL)
GAME(1983,  megaatona, megaaton,  play_3,  play_3, play_3_state,  play_3,  ROT0,  "Playmatic",    "Meg-Aaton (alternate set)",     MACHINE_IS_SKELETON_MECHANICAL)
