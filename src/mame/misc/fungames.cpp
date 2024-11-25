// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

 Fun Games discrete hardware games

 Biplane (1975)
 Biplane 4 (1976)
 King (1976) - unreleased
 Race! (1976)
 Take 5 (1975)
 Take 7 (1975)
 Tankers (1975) - there is a bootleg of this released by Alca called "Battleground"

***************************************************************************/


#include "emu.h"

#include "machine/netlist.h"
#include "netlist/devices/net_lib.h"
#include "video/fixfreq.h"


namespace {

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


class fungames_state : public driver_device
{
public:
	fungames_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_video(*this, "fixfreq")
	{
	}

	void fungames(machine_config &config);

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<netlist_mame_device> m_maincpu;
	required_device<fixedfreq_device> m_video;
};


static NETLIST_START(fungames)
{
	SOLVER(Solver, 48000)
//  PARAM(Solver.FREQ, 48000)
	PARAM(Solver.ACCURACY, 1e-4) // works and is sufficient

	// schematics
	//...

//  NETDEV_ANALOG_CALLBACK(sound_cb, sound, exidyttl_state, sound_cb, "")
//  NETDEV_ANALOG_CALLBACK(video_cb, videomix, fixedfreq_device, update_vid, "fixfreq")
}



void fungames_state::machine_start()
{
}

void fungames_state::machine_reset()
{
}


void fungames_state::video_start()
{
}

void fungames_state::fungames(machine_config &config)
{
	/* basic machine hardware */
	NETLIST_CPU(config, m_maincpu, netlist::config::DEFAULT_CLOCK()).set_source(netlist_fungames);

	/* video hardware */
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);
	FIXFREQ(config, m_video).set_screen("screen");
	m_video->set_monitor_clock(MASTER_CLOCK);
	m_video->set_horz_params(H_TOTAL-67,H_TOTAL-40,H_TOTAL-8,H_TOTAL);
	m_video->set_vert_params(V_TOTAL-22,V_TOTAL-19,V_TOTAL-12,V_TOTAL);
	m_video->set_fieldcount(1);
	m_video->set_threshold(0.30);
}


/***************************************************************************

 Game driver(s)

 ***************************************************************************/


ROM_START( biplane )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0400, "roms", ROMREGION_ERASE00 ) // 82s115 PROMs
	ROM_LOAD( "12355.h6",     0x0000, 0x0200, CRC(15e9067a) SHA1(d9a5eb558bcf9c1846b2090247bd7940aa6e2d5a) )
	ROM_LOAD( "12356.h7",     0x0000, 0x0200, CRC(c02c66cb) SHA1(8f7f7b8e5f48e49cb1fd01b456a2bf659ba68047) )
	ROM_LOAD( "12357.h8",     0x0000, 0x0200, CRC(a57a1c43) SHA1(64d8e609415bc0fe51581bfea2e777106505c761) )
	ROM_LOAD( "12358.h9",     0x0000, 0x0200, CRC(40415f2a) SHA1(8a5d4cfb7713fc42ab12e3af61230255c5dffbf2) )
ROM_END


/*

Both Fun Games BiPlane and Mirco Sky War have similar PCB design set up with four PROMs located on the
'memory' PCB. Mirco Sky War 'memory' PCB part number is 3701. Mirco Sky War schematics show PROM id being
the same, but the PROMs are located on a different row on the 'memory' PCB. Their listed locations are 6J,
7J, 8J and 9J. Fun Games starts with letter first. For continuity, staying with Fun Games method instead of
Mirco since Mirco's game is probably a licensed clone. Besides the PROM id number, the chip type is probably
the same.


ROM_START( skywar )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

    ROM_REGION( 0x0400, "roms", ROMREGION_ERASE00 ) // most likely 82s115 PROMs as well
    ROM_LOAD( "12355.j6", 0x0000, 0x0200, CRC(15e9067a) SHA1(d9a5eb558bcf9c1846b2090247bd7940aa6e2d5a) )
    ROM_LOAD( "12356.j7", 0x0000, 0x0200, CRC(c02c66cb) SHA1(8f7f7b8e5f48e49cb1fd01b456a2bf659ba68047) )
    ROM_LOAD( "12357.j8", 0x0000, 0x0200, CRC(a57a1c43) SHA1(64d8e609415bc0fe51581bfea2e777106505c761) )
    ROM_LOAD( "12358.j9", 0x0000, 0x0200, CRC(40415f2a) SHA1(8a5d4cfb7713fc42ab12e3af61230255c5dffbf2) )
ROM_END

*/


ROM_START( biplane4 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0400, "roms", ROMREGION_ERASE00 ) // 82s115 PROMs
	ROM_LOAD( "12365.p12",     0x0000, 0x0200, CRC(0b3af146) SHA1(de7e4bffd4ca3baf3fe6017609d1b11fa9fc356a) )
	ROM_LOAD( "12366.r12",     0x0000, 0x0200, CRC(121eee0c) SHA1(fbdbc4da94dd9dba5903a6df321a9c2319f86dbd) )
	ROM_LOAD( "12357.s12",     0x0000, 0x0200, CRC(a57a1c43) SHA1(64d8e609415bc0fe51581bfea2e777106505c761) )
	ROM_LOAD( "12358.t12",     0x0000, 0x0200, CRC(40415f2a) SHA1(8a5d4cfb7713fc42ab12e3af61230255c5dffbf2) )
ROM_END


ROM_START( take5 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0200, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "take5.d2",     0x0000, 0x0200, CRC(fa6afc19) SHA1(9735e13b20e698d1b72cbff5783a06f260ff2bba) )
	ROM_LOAD( "take5.e2",     0x0000, 0x0200, CRC(1a41d642) SHA1(925010aa953cea2ab5bf0e9148f25b13d0a0e52e) )
	ROM_LOAD( "take5.i2",     0x0000, 0x0200, CRC(25ff4f82) SHA1(6b3b9071591ca5126cce556007c12e4bf2cb3d51) )
ROM_END


/*

Fun Games Tankers has a similar PCB design set up with four PROMs located only on the 'memory' PCB just like
Fun Games BiPlane. The chip type is probably the same type used with Fun Games BiPlane and
Fun Games BiPlane 4.

ROM_START( tankers )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

    ROM_REGION( 0x0400, "roms", ROMREGION_ERASE00 ) // most likely 82s115 PROMs as well
    ROM_LOAD( "12345.h6", 0x0000, 0x0200, NO_DUMP )
    ROM_LOAD( "12346.h7", 0x0000, 0x0200, NO_DUMP )
    ROM_LOAD( "12347.h8", 0x0000, 0x0200, NO_DUMP )
    ROM_LOAD( "12348.h9", 0x0000, 0x0200, NO_DUMP )
ROM_END

*/

} // Anonymous namespace


GAME( 1975, biplane, 0, fungames, 0, fungames_state, empty_init, ROT0, "Fun Games", "Biplane", MACHINE_IS_SKELETON )
//GAME( 1975, skywar, biplane, 0, fungames, 0, fungames_state, empty_init, ROT0, "Mirco Games", "Sky War", MACHINE_IS_SKELETON )
GAME( 1976, biplane4, 0, fungames, 0, fungames_state, empty_init, ROT0, "Fun Games", "Biplane 4", MACHINE_IS_SKELETON )
GAME( 1975, take5, 0, fungames, 0, fungames_state, empty_init, ROT0, "Fun Games", "Take 5", MACHINE_IS_SKELETON )
//GAME( 1975, tankers, 0, fungames, 0, fungames_state, empty_init, ROT0, "Fun Games", "Tankers", MACHINE_IS_SKELETON )
