// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

clown roll down z8 = 2732a
clown roll down z9 = 2732a

can't find any info on this?

*/

#include "emu.h"


class clowndwn_state : public driver_device
{
public:
	clowndwn_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag)
	//  ,m_maincpu(*this, "maincpu")
	{ }

	virtual void machine_start() override;
	virtual void machine_reset() override;

//  required_device<mcs51_cpu_device> m_maincpu;
};

static INPUT_PORTS_START( clowndwn )
INPUT_PORTS_END



void clowndwn_state::machine_start()
{
}

void clowndwn_state::machine_reset()
{
}


static MACHINE_CONFIG_START( clowndwn, clowndwn_state )

	/* basic machine hardware */
//  MCFG_CPU_ADD("maincpu", ??, 8000000) // unknown (vectors at end? 6xxx ?)
//  MCFG_CPU_PROGRAM_MAP(clowndwn_map)
//  MCFG_CPU_IO_MAP(clowndwn_io)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END


// has
// COPYRIGHT 1982, 1983, 1984, 1985, and 1987 by ELWOOD ELECTRONICS CO., INC
// in Z9

ROM_START( clowndwn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "CLWNROLL.Z8", 0x0000, 0x1000, CRC(ec655745) SHA1(e38de904f30530f8971eb4a9d7796da345bf81ad) )
	ROM_LOAD( "CLWNROLL.Z9", 0x1000, 0x1000, CRC(aeef885e) SHA1(bc6805b638625a347e1288a927ce30e030afe9e3) )
ROM_END

GAME( 1987, clowndwn,  0,    clowndwn, clowndwn, driver_device,  0, ROT0, "Elwood Electronics", "Clown Roll Down (Elwood)", MACHINE_IS_SKELETON_MECHANICAL )
