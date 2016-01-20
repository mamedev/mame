// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

After Shock by Laser Tron

redemption game

devices are 27c512

--------------------------

file Aftrshk.upd is the updated eprom for an update version.
The update version uses a small (appx 2" x 4" ) pcb to turn on
the playfield motor.  If you don't have this small pcb, then don't use
this .upd version software.  The small pcb is numbered

"pcb100067"
"Lazer Tron driver pcb V.02"

This "kit" will correct aproblem with the Allegro UCN5801A chip
on the main pcb.
full instructions available from Lazer-Tron aka Arcade Planet


a video of this in action can be seen at
https://www.youtube.com/watch?v=9DIhuOEVwf4


*/

#include "emu.h"
#include "sound/okim6295.h"


class aftrshok_state : public driver_device
{
public:
	aftrshok_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag)
	//  ,m_maincpu(*this, "maincpu")
	{ }

	virtual void machine_start() override;
	virtual void machine_reset() override;

//  required_device<mcs51_cpu_device> m_maincpu;
};

static INPUT_PORTS_START( aftrshok )
INPUT_PORTS_END



void aftrshok_state::machine_start()
{
}

void aftrshok_state::machine_reset()
{
}


static MACHINE_CONFIG_START( aftrshok, aftrshok_state )

	/* basic machine hardware */
//  MCFG_CPU_ADD("maincpu", ??, 8000000) // unknown
//  MCFG_CPU_PROGRAM_MAP(aftrshok_map)
//  MCFG_CPU_IO_MAP(aftrshok_io)
//  MCFG_CPU_VBLANK_INT_DRIVER("screen", aftrshok_state,  irq0_line_hold)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH) // maybe
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



ROM_START( aftrshok )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "AFTRSHOK.U3.UPD", 0x00000, 0x10000, CRC(779fad60) SHA1(6be3b99cea95b5320c6d500616a703cdab126d9c) ) // see note at top of driver about this update

	ROM_REGION( 0xc0000, "oki", 0 )
	ROM_LOAD( "AFTRSHOK.U27", 0x00000, 0x10000, CRC(2d0061ef) SHA1(cc674ea020ef9e6b3baecfdb72f9766fef89bed8) )
	ROM_LOAD( "AFTRSHOK.U26", 0x10000, 0x10000, CRC(d2b55dc1) SHA1(2684bfc65628a550fcbaa6726b5dab488e7ede5a) )
	ROM_LOAD( "AFTRSHOK.U25", 0x20000, 0x10000, CRC(d5d1c606) SHA1(ad72a00c211ee7f5bc0772d6f469d59047131095) )
ROM_END

ROM_START( aftrshoka )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "AFTRSHOK.U3", 0x00000, 0x10000, CRC(841828b9) SHA1(51a88ce7466fcbc6d5192d738425ae1d37c1b88e) )

	ROM_REGION( 0xc0000, "oki", 0 )
	ROM_LOAD( "AFTRSHOK.U27", 0x00000, 0x10000, CRC(2d0061ef) SHA1(cc674ea020ef9e6b3baecfdb72f9766fef89bed8) )
	ROM_LOAD( "AFTRSHOK.U26", 0x10000, 0x10000, CRC(d2b55dc1) SHA1(2684bfc65628a550fcbaa6726b5dab488e7ede5a) )
	ROM_LOAD( "AFTRSHOK.U25", 0x20000, 0x10000, CRC(d5d1c606) SHA1(ad72a00c211ee7f5bc0772d6f469d59047131095) )
ROM_END


GAME( 19??, aftrshok,  0,           aftrshok, aftrshok, driver_device,  0, ROT0, "Lazer-tron", "After Shock (Lazer-tron, set 1)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 19??, aftrshoka, aftrshok,    aftrshok, aftrshok, driver_device,  0, ROT0, "Lazer-tron", "After Shock (Lazer-tron, set 2)", MACHINE_IS_SKELETON_MECHANICAL )
