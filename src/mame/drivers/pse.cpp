// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Dave Widel, gregf
/***************************************************************************

 Project Support Engineering Games

 Game Name                       DATA
 -------------------------------------
 1-2-4 Cocktail Table (197?)     UNKNOWN
 Bazooka (1976/11)               YES
 Desert Patrol (1977/11)         YES
 Espana (cabinet) (1975/10)      NO
 Frenzy (1975/08)                UNKNOWN
 Game Tree (1978/02)             YES
 Hodge Podge (1975?)             NO
 Knights in Armor (1976/06)      YES
 Maneater (1975/11)              YES
 Play Five (1975?)               UNKNOWN
 Scandia (cabinet) (1975/08)     NO
 Two Game (1974)                 UNKNOWN
 U.N. Command (1977)             YES?

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

ROM_START( bazooka )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0840, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "bd2.k1",      0x0000, 0x0200, CRC(c9e9ed15) SHA1(624bbc10942a386040aef161b96d64021a842c9f) ) // 6341-1 - gfx: tank, truck, jeep motorcycle
	ROM_LOAD( "bd2.k4",      0x0200, 0x0200, CRC(c5a74df9) SHA1(2846a039e9bf372f3aa0b88ed89f9029eb7f797c) ) // 6341-1 - gfx: ambulance, stretcher, explosion
	ROM_LOAD( "bd1.d2",      0x0400, 0x0200, CRC(4fc10886) SHA1(b1c6f890994ba2182a4e7fc17582d6797dbd6ce9) ) // 6341-1 or 82s115
	ROM_LOAD( "bd1.e2",      0x0600, 0x0200, CRC(00179936) SHA1(e5417b8d3814dafe1278179b307a1b563a378cbe) ) // 6341-1 or 82s115
	ROM_LOAD( "bd2.e6",      0x0800, 0x0020, CRC(14b84564) SHA1(69cdd14e23094678c4b280f60cec963609181b00) ) // 82123
	ROM_LOAD( "bd2.e7",      0x0820, 0x0020, CRC(1bfb073f) SHA1(f6b26dcece71b2cf2ed4a537434edbe31cb10399) ) // 82123
ROM_END

ROM_START( bazookabr )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0840, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "1",           0x0000, 0x0200, CRC(edc34cb0) SHA1(f76a81833b015784e55b33189e9058cd24922f9b) )
	ROM_LOAD( "2",           0x0200, 0x0200, CRC(3e78e4c2) SHA1(814509eb773bfa87f1df933214f079e7dd2a8fa2) )
	ROM_LOAD( "3",           0x0400, 0x0200, CRC(4fc10886) SHA1(b1c6f890994ba2182a4e7fc17582d6797dbd6ce9) )
	ROM_LOAD( "4",           0x0600, 0x0200, CRC(00179936) SHA1(e5417b8d3814dafe1278179b307a1b563a378cbe) )
	ROM_LOAD( "bd2.e6",      0x0800, 0x0020, BAD_DUMP CRC(14b84564) SHA1(69cdd14e23094678c4b280f60cec963609181b00) ) // not dumped, taken from PSE set
	ROM_LOAD( "bd2.e7",      0x0820, 0x0020, BAD_DUMP CRC(1bfb073f) SHA1(f6b26dcece71b2cf2ed4a537434edbe31cb10399) ) // not dumped, taken from PSE set
ROM_END

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

GAME( 1976, bazooka,    0,       pse, 0, driver_device,  0, ROT0, "Project Support Engineering", "Bazooka [TTL]", MACHINE_IS_SKELETON )
GAME( 1977, bazookabr,  bazooka, pse, 0, driver_device,  0, ROT0, "Taito do Brasil", "Bazooka (Brazil) [TTL]", MACHINE_IS_SKELETON )
GAME( 1977, dpatrol,    0,       pse, 0, driver_device,  0, ROT0, "Project Support Engineering", "Desert Patrol [TTL]", MACHINE_IS_SKELETON )
//GAME( 1976, knightar, 0,       pse, 0, driver_device,  0, ROT0, "Project Support Engineering", "Knights in Armor [TTL]", MACHINE_IS_SKELETON )
//GAME( 1978, gametree, 0,       pse, 0, driver_device,  0, ROT0, "Project Support Engineering", "Game Tree [TTL]", MACHINE_IS_SKELETON )
