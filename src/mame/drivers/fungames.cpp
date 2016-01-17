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
 Tankers (1975)

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


class fungames_state : public driver_device
{
public:
	fungames_state(const machine_config &mconfig, device_type type, std::string tag)
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
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

private:

};


static NETLIST_START(fungames)
	SOLVER(Solver, 48000)
//  PARAM(Solver.FREQ, 48000)
	PARAM(Solver.ACCURACY, 1e-4) // works and is sufficient

	// schematics
	//...

//  NETDEV_ANALOG_CALLBACK(sound_cb, sound, exidyttl_state, sound_cb, "")
//  NETDEV_ANALOG_CALLBACK(video_cb, videomix, fixedfreq_device, update_vid, "fixfreq")
NETLIST_END()



void fungames_state::machine_start()
{
}

void fungames_state::machine_reset()
{
}


void fungames_state::video_start()
{
}

static MACHINE_CONFIG_START( fungames, fungames_state )

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", NETLIST_CPU, NETLIST_CLOCK)
	MCFG_NETLIST_SETUP(fungames)

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


ROM_START( biplane4 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0400, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "biplane.p",     0x0000, 0x0200, CRC(0b3af146) SHA1(de7e4bffd4ca3baf3fe6017609d1b11fa9fc356a) )
	ROM_LOAD( "biplane.r",     0x0000, 0x0200, CRC(121eee0c) SHA1(fbdbc4da94dd9dba5903a6df321a9c2319f86dbd) )
	ROM_LOAD( "biplane.s",     0x0000, 0x0200, CRC(a57a1c43) SHA1(64d8e609415bc0fe51581bfea2e777106505c761) )
	ROM_LOAD( "biplane.t",     0x0000, 0x0200, CRC(40415f2a) SHA1(8a5d4cfb7713fc42ab12e3af61230255c5dffbf2) )
ROM_END


ROM_START( take5 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0200, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "take5.d2",     0x0000, 0x0200, CRC(fa6afc19) SHA1(9735e13b20e698d1b72cbff5783a06f260ff2bba) )
	ROM_LOAD( "take5.e2",     0x0000, 0x0200, CRC(1a41d642) SHA1(925010aa953cea2ab5bf0e9148f25b13d0a0e52e) )
	ROM_LOAD( "take5.i2",     0x0000, 0x0200, CRC(25ff4f82) SHA1(6b3b9071591ca5126cce556007c12e4bf2cb3d51) )
ROM_END


GAME( 1976, biplane4,  0, fungames, 0, driver_device,  0, ROT0, "Fun Games", "Biplane 4 [TTL]", MACHINE_IS_SKELETON )
GAME( 1975, take5,     0, fungames, 0, driver_device,  0, ROT0, "Fun Games", "Take 5 [TTL]", MACHINE_IS_SKELETON )
