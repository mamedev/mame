// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

 Electra discrete hardware games


Game Name                                 Board part number  DATA

Avenger (1975)                            EG-1020            YES
Combo 3 (Tennis, Soccer, Hockey) (1975)                      UNKNOWN
Eliminator IV (1976)                                         UNKNOWN
Flying Fortress (1976) (Taito same name?) EG-1060A + EG1060B YES
Knockout (1975)                                              UNKNOWN
Pace Car Pro (1975)                       EG-1000            NO
Pace Race (1975)                          EG-1000            NO
RTH (1976)                                                   UNKNOWN
UFO Chase (1975)                          EG-1010            UNKNOWN
Wings / Wings Cocktail (1976)             EG-1040            YES

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


class electra_state : public driver_device
{
public:
	electra_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_video(*this, "fixfreq")
	{
	}

	void electra(machine_config &config);

private:
	// devices
	required_device<netlist_mame_device> m_maincpu;
	required_device<fixedfreq_device> m_video;

	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void video_start() override ATTR_COLD;
};


static NETLIST_START(electra)
{
	SOLVER(Solver, 48000)
//  PARAM(Solver.FREQ, 48000)
	PARAM(Solver.ACCURACY, 1e-4) // works and is sufficient

	// schematics
	//...

//  NETDEV_ANALOG_CALLBACK(sound_cb, sound, exidyttl_state, sound_cb, "")
//  NETDEV_ANALOG_CALLBACK(video_cb, videomix, fixedfreq_device, update_vid, "fixfreq")
}



void electra_state::machine_start()
{
}

void electra_state::machine_reset()
{
}


void electra_state::video_start()
{
}

void electra_state::electra(machine_config &config)
{
	/* basic machine hardware */
	NETLIST_CPU(config, m_maincpu, netlist::config::DEFAULT_CLOCK()).set_source(netlist_electra);

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


ROM_START( avenger )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0260, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "106069-a.l10", 0x0000, 0x0200, CRC(12052a01) SHA1(0674254f73be14b871870c52d7f731209411bcea) )
	ROM_LOAD( "106072-1.d10", 0x0200, 0x0020, CRC(3c10773b) SHA1(84b6d10d372978e80f358e66713571a26e129eed) )
	ROM_LOAD( "106072-2.g10", 0x0220, 0x0020, CRC(b2dba75e) SHA1(dc4e205aeb62ebd5617e571d9e7b467da377fff5) )
	ROM_LOAD( "106072-3.h10", 0x0240, 0x0020, CRC(816a8136) SHA1(2eca1ce7b53dd314ad0b2fdf71b843aaca774721) )
ROM_END


ROM_START( flyingf )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0320, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "b-1.2b",  0x0000, 0x0100, CRC(c88a3dff) SHA1(9b5e568206263087f8f1dd7b94b7ae82aa3bdbaf) )
	ROM_LOAD( "b-2.1a",  0x0100, 0x0100, CRC(7f6e4af5) SHA1(1a436713ae1639b75e4567de040109714b4ff52b) )
	ROM_LOAD( "b-3.1i",  0x0200, 0x0100, CRC(5687270b) SHA1(481055801f0ba3c036e42e2254962028c5855bbe) )
	ROM_LOAD( "prom.1d", 0x0300, 0x0020, CRC(4fabe931) SHA1(ac3c2a59dce080460b4a9230f5d36d2b2627f729) )
ROM_END

} // anonymous namespace


GAME( 1975, avenger, 0, electra, 0, electra_state, empty_init, ROT0, "Electra", "Avenger",         MACHINE_IS_SKELETON )
GAME( 1976, flyingf, 0, electra, 0, electra_state, empty_init, ROT0, "Electra", "Flying Fortress", MACHINE_IS_SKELETON )
