// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Dave Widel, gregf
/***************************************************************************

 Project Support Engineering Games

 Game Name                       DATA
 -------------------------------------
 1-2-4 Cocktail Table (197?)     UNKNOWN
 Bazooka (1977)                  YES
 Desert Patrol (1977)            YES
 Espana (cabinet) (197?)         NO
 Frenzy (1975)                   UNKNOWN
 Game Tree (1978)                YES
 Hodge Podge (197?)              UNKNOWN
 Knights in Armor (1976)         YES
 Maneater (1975)                 YES
 Play Five (1975?)               UNKNOWN
 Scandia (cabinet) (1975)        NO
 Two Game (1974)                 UNKNOWN
 U.N. Command (1977)             UNKNOWN

 ***************************************************************************/


#include "emu.h"

#include "machine/netlist.h"
#include "netlist/devices/net_lib.h"
#include "video/fixfreq.h"

// copied by Pong, not accurate for this driver!
// start
#define MASTER_CLOCK    7159000
#define V_TOTAL         (0x105+1)       // 262
#define H_TOTAL         (0x1C6+1)       // 454

#define HBSTART                 (H_TOTAL)
#define HBEND                   (80)
#define VBSTART                 (V_TOTAL)
#define VBEND                   (16)

#define HRES_MULT                   (1)
// end


class pse_state : public driver_device
{
public:
	pse_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_video(*this, "fixfreq")
	{
	}

	// devices
	required_device<netlist_mame_device_t> m_maincpu;
	required_device<fixedfreq_device> m_video;

protected:

	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();

private:

};


static NETLIST_START(pse)
	SOLVER(Solver, 48000)
//  PARAM(Solver.FREQ, 48000)
	PARAM(Solver.ACCURACY, 1e-4) // works and is sufficient

	// schematics
	//...

//  NETDEV_ANALOG_CALLBACK(sound_cb, sound, psettl_state, sound_cb, "")
//  NETDEV_ANALOG_CALLBACK(video_cb, videomix, fixedfreq_device, update_vid, "fixfreq")
NETLIST_END()



void pse_state::machine_start()
{
}

void pse_state::machine_reset()
{
}


void pse_state::video_start()
{
}

static MACHINE_CONFIG_START( pse, pse_state )

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", NETLIST_CPU, NETLIST_CLOCK)
	MCFG_NETLIST_SETUP(pse)

	/* video hardware */
	MCFG_FIXFREQ_ADD("fixfreq", "screen")
	MCFG_FIXFREQ_MONITOR_CLOCK(MASTER_CLOCK)
	MCFG_FIXFREQ_HORZ_PARAMS(H_TOTAL-67,H_TOTAL-40,H_TOTAL-8,H_TOTAL)
	MCFG_FIXFREQ_VERT_PARAMS(V_TOTAL-22,V_TOTAL-19,V_TOTAL-12,V_TOTAL)
	MCFG_FIXFREQ_FIELDCOUNT(1)
	MCFG_FIXFREQ_SYNC_THRESHOLD(0.30)
MACHINE_CONFIG_END


/***************************************************************************

 Game driver(s)

 ***************************************************************************/


ROM_START( dpatrol )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0800, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "bd1.d2", 0x0000, 0x0400, CRC(e4c8e4ab) SHA1(0b989ca9369139f212dcea1d1461998f20057db8)) // computer program game code. 6341-1 or 82S181 according to Desert Patrol schematics
	ROM_LOAD( "bd1.e2", 0x0000, 0x0400, CRC(256b3320) SHA1(712573e3d9625a84c54bbe2e3edafb8879a14b2e)) // computer program game code. 6341-1 or 82S181 according to Desert Patrol schematics

	ROM_LOAD( "bd2.l4", 0x0000, 0x0200, CRC(bc87c648) SHA1(c4709d155aa50cc87146abd152a11de618cfd64c)) // prom 1 contains aircraft target images and explosion image. pcb has 82S141; schematics show 6341-1
	ROM_LOAD( "bd2.l1", 0x0000, 0x0800, CRC(f1e8ba9e) SHA1(605db3fdbaff4ba13729371ad0c4fbab3889378e)) // prom 2 contains parachute and man, falling man. pcb has 82S141; schematics show 6341-1

	ROM_LOAD( "bd2.h7", 0x0000, 0x0020, NO_DUMP) // contains prom address codes and image speeds. Each image has its own speed and address block in the image PROM. chip is 82S123

	ROM_LOAD( "bd3.d1", 0x0000, 0x0020, NO_DUMP ) // data in prom is organized to produce the waveform of a human scream. Chip type is 8574 or MM6301-0J
ROM_END

/*
ROM_START( knightar )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

    ROM_REGION( 0x0020, "roms", ROMREGION_ERASE00 )
    ROM_LOAD( "1.m1" ) // Man rom stores image characters of knights. 82S115P or 8205R according to KIA schematics
    ROM_LOAD( "2.m2" ) // Horse rom stores image characters of horses. 82S115P or 8205R according to KIA schematics
ROM_END
*/

/*
ROM_START( gametree )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

    ROM_REGION( 0x0020, "roms", ROMREGION_ERASE00 )
    ROM_LOAD( "bd1.d2" ) // computer program game code. 6341-1 or 82S181 according to Game Tree schematics
    ROM_LOAD( "bd1.e2" ) // computer program game code. 6341-1 or 82S181 according to Game Tree schematics

    ROM_LOAD( "bd2.f12" ) // prom 1 contains squirrel and squirrel point value. 82S141 or 6341-1 according to Game Tree layout
    ROM_LOAD( "bd2.f14" ) // prom 1 contains squirrel and squirrel point value. 82S141 or 6341-1 according to Game Tree layout


    ROM_LOAD( "bd2.e12" ) // prom 2 contains other targets (rabbit and turkey) and point values
    ROM_LOAD( "bd2.e14" ) // prom 2 82S141 or 6341-1 according to Game Tree layout.


    ROM_LOAD( "bd2.a2" ) // contains prom address codes and image speeds. Each has its own speed and address block in the image prom.

    ROM_LOAD( "bd2.a3" ) // contains prom address codes and image speeds. 82S123 or 6331-1 according to Game Tree layout.
ROM_END
*/


GAME( 1977, dpatrol, 0, pse, 0, driver_device, 0, ROT0, "Project Support Engineering", "Desert Patrol [TTL]", MACHINE_IS_SKELETON )
//GAME( 1976, knightar, 0, pse, 0, driver_device, 0, ROT0, "Project Support Engineering", "Knights in Armor [TTL]", MACHINE_IS_SKELETON )
//GAME( 1978, gametree, 0, pse, 0, driver_device, 0, ROT0, "Project Support Engineering", "Game Tree [TTL]", MACHINE_IS_SKELETON )
