// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

 Allied Leisure discrete hardware games

 Chase (aka Chase1) (1976)
 Deluxe Soccer (1973)
 Fire Power (1975)                EG-1020-2
 F??tsball (1975)
 Galaxy Raider (1974)
 Hesitation (1974)                AL-6500?
 Hockey, Soccer, Tennis (1974)
 Improbable (1974)
 Knock Ball (1974)
 Paddle Battle (1973)
 Ric-O-Chet (1973)
 Robot (1975)                     AL-7500
 Ski (1975) (electromechanical)
 Street Burners (1975)            URL-8300
 Super Soccer (1973)
 Tennis Tourney (1973)
 Zap (1974)                       AL-6500

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


class sburners_state : public driver_device
{
public:
	sburners_state(const machine_config &mconfig, device_type type, std::string tag)
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


static NETLIST_START(sburners)
	SOLVER(Solver, 48000)
//  PARAM(Solver.FREQ, 48000)
	PARAM(Solver.ACCURACY, 1e-4) // works and is sufficient

	// schematics
	//...

//  NETDEV_ANALOG_CALLBACK(sound_cb, sound, exidyttl_state, sound_cb, "")
//  NETDEV_ANALOG_CALLBACK(video_cb, videomix, fixedfreq_device, update_vid, "fixfreq")
NETLIST_END()



void sburners_state::machine_start()
{
}

void sburners_state::machine_reset()
{
}


void sburners_state::video_start()
{
}

static MACHINE_CONFIG_START( sburners, sburners_state )

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", NETLIST_CPU, NETLIST_CLOCK)
	MCFG_NETLIST_SETUP(sburners)

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


ROM_START( sburners )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0400, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "url1.c15",     0x0000, 0x0200, CRC(e7941edb) SHA1(e3661a4b883e827fa8f0e5191007b948159cb3f4) )
ROM_END


GAME( 1975, sburners,  0, sburners, 0, driver_device,  0, ROT0, "Allied Leisure", "Street Burners [TTL]", MACHINE_IS_SKELETON )
