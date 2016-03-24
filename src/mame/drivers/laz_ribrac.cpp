// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

ribbit racing -- prog rom 27c512  @ u7
sound roms -----u7-u10 and u11-u14  = 27c512

IC positions look similar to Awesome Toss 'em
sound rom dumps weren't present even tho mentioned??

this appears to be the operators manual
http://ohwow-arcade.com/Assets/Game_Manuals/RIBBIT%20RACIN.PDF

*/

#include "emu.h"
#include "sound/okim6295.h"


class laz_ribrac_state : public driver_device
{
public:
	laz_ribrac_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	//  ,m_maincpu(*this, "maincpu")
	{ }

	virtual void machine_start() override;
	virtual void machine_reset() override;

//  required_device<mcs51_cpu_device> m_maincpu;
};

static INPUT_PORTS_START( laz_ribrac )
INPUT_PORTS_END



void laz_ribrac_state::machine_start()
{
}

void laz_ribrac_state::machine_reset()
{
}


static MACHINE_CONFIG_START( laz_ribrac, laz_ribrac_state )

	/* basic machine hardware */
//  MCFG_CPU_ADD("maincpu", ??, 8000000) // unknown
//  MCFG_CPU_PROGRAM_MAP(laz_ribrac_map)
//  MCFG_CPU_IO_MAP(laz_ribrac_io)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH) // maybe
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



ROM_START( ribrac )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "RIBBITR.U7", 0x00000, 0x10000, CRC(9eb78ca3) SHA1(4fede7bdd30449602a01489dc72dbbd5452d6b5a) )

	ROM_REGION( 0xc0000, "oki", 0 )
	ROM_LOAD( "RIBBITR_SND.U10", 0x00000, 0x10000, NO_DUMP )
	ROM_LOAD( "RIBBITR_SND.U9", 0x10000, 0x10000, NO_DUMP )
	ROM_LOAD( "RIBBITR_SND.U8", 0x20000, 0x10000, NO_DUMP )
	ROM_LOAD( "RIBBITR_SND.U7", 0x30000, 0x10000, NO_DUMP )

	ROM_REGION( 0xc0000, "oki2", 0 )
	ROM_LOAD( "RIBBITR_SND.U14", 0x00000, 0x10000, NO_DUMP )
	ROM_LOAD( "RIBBITR_SND.U13", 0x10000, 0x10000, NO_DUMP )
	ROM_LOAD( "RIBBITR_SND.U12", 0x20000, 0x10000, NO_DUMP )
	ROM_LOAD( "RIBBITR_SND.U11", 0x30000, 0x10000, NO_DUMP )
ROM_END

GAME( 1993, ribrac,  0,    laz_ribrac, laz_ribrac, driver_device,  0, ROT0, "Lazer-tron", "Ribbit Racing (Lazer-tron)", MACHINE_IS_SKELETON_MECHANICAL )
