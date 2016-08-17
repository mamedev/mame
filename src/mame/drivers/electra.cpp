// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

 Electra discrete hardware games

 Game Name
 Avenger (1975)                               EG-1020
 Combo 3 (Tennis, Soccer, Hockey) (1975)
 Eliminator IV (1976)
 Flying Fortress (1976)                       EG-1060  (Taito same name?)
 Knockout (1975)
 Pace Car Pro (1975)                          EG-1000
 Pace Race (1974?)
 RTH (1976)
 UFO Chase (1975)                             EG-1010
 Wings / Wings Cocktail (1976)

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


class electra_state : public driver_device
{
public:
	electra_state(const machine_config &mconfig, device_type type, const char *tag)
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


static NETLIST_START(electra)
	SOLVER(Solve, 48000)
//  PARAM(Solver.FREQ, 48000)
	PARAM(Solver.ACCURACY, 1e-4) // works and is sufficient

	// schematics
	//...

//  NETDEV_ANALOG_CALLBACK(sound_cb, sound, exidyttl_state, sound_cb, "")
//  NETDEV_ANALOG_CALLBACK(video_cb, videomix, fixedfreq_device, update_vid, "fixfreq")
NETLIST_END()



void electra_state::machine_start()
{
}

void electra_state::machine_reset()
{
}


void electra_state::video_start()
{
}

static MACHINE_CONFIG_START( electra, electra_state )

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", NETLIST_CPU, NETLIST_CLOCK)
	MCFG_NETLIST_SETUP(electra)

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


ROM_START( avenger )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0200, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "106069-a.l10",     0x0000, 0x0200, CRC(12052a01) SHA1(0674254f73be14b871870c52d7f731209411bcea) )
	ROM_LOAD( "106072-1.d10",     0x0000, 0x0020, CRC(3c10773b) SHA1(84b6d10d372978e80f358e66713571a26e129eed) )
	ROM_LOAD( "106072-2.g10",     0x0000, 0x0020, CRC(b2dba75e) SHA1(dc4e205aeb62ebd5617e571d9e7b467da377fff5) )
	ROM_LOAD( "106072-3.h10",     0x0000, 0x0020, CRC(816a8136) SHA1(2eca1ce7b53dd314ad0b2fdf71b843aaca774721) )
ROM_END


GAME( 1975, avenger,  0, electra, 0, driver_device,  0, ROT0, "Electra", "Avenger [TTL]", MACHINE_IS_SKELETON )
