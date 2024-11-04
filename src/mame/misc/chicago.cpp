// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

 Chicago Coin discrete hardware games

 Game Name                        Model #
 -----------------------------------------
 TV Ping Pong (1973)              #424
 TV Tennis (1973)                 #427
 Olympic TV Hockey (1973)         #429
 Olympic TV Football (1973)       #429-A
 TV Goalee (1974)                 #434
 TV Pingame (1975)                #451
 Super Flipper (1975)             #458
 Demolition Derby (1976)          #466

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


class chicago_state : public driver_device
{
public:
	chicago_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_video(*this, "fixfreq")
	{
	}

	// devices
	required_device<netlist_mame_device> m_maincpu;
	required_device<fixedfreq_device> m_video;

	void chicago(machine_config &config);
protected:

	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void video_start() override ATTR_COLD;

private:

};


static NETLIST_START(chicago)
{
	SOLVER(Solver, 48000)
//  PARAM(Solver.FREQ, 48000)
	PARAM(Solver.ACCURACY, 1e-4) // works and is sufficient

	// schematics
	//...

//  NETDEV_ANALOG_CALLBACK(sound_cb, sound, exidyttl_state, sound_cb, "")
//  NETDEV_ANALOG_CALLBACK(video_cb, videomix, fixedfreq_device, update_vid, "fixfreq")
}



void chicago_state::machine_start()
{
}

void chicago_state::machine_reset()
{
}


void chicago_state::video_start()
{
}

void chicago_state::chicago(machine_config &config)
{
	/* basic machine hardware */
	NETLIST_CPU(config, m_maincpu, netlist::config::DEFAULT_CLOCK()).set_source(netlist_chicago);

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

// Demolition Derby is licensed from Exidy, PROMs are identical to Exidy Destruction Derby
// However, the PCBs and layouts are different, so there is a separate driver here

ROM_START( dmodrbcc )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0400, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "cdi.b2",     0x0000, 0x0100, CRC(c3823f0b) SHA1(42fe8c1e0f54b3f968a630dd564a8941410c5d86) )
	ROM_LOAD( "cdi.b8",     0x0000, 0x0100, CRC(9f65c1df) SHA1(e367cc418b34005198f9b958592573986d37274a) )
	ROM_LOAD( "cdi.c8",     0x0000, 0x0100, CRC(5e27fc7c) SHA1(bae31e7f0ff7a5ebfe4a12fc537249ee24a4cf4b) )
	ROM_LOAD( "cdi.d8",     0x0000, 0x0100, CRC(93dc096c) SHA1(b89a478ef731024eb24a79f9e82a5ef779e8e3d0) )
	ROM_LOAD( "cdi.e8",     0x0000, 0x0100, CRC(ddbb0cc7) SHA1(6f169566cb09c78090cec2e375013c0eb2656890) )
	ROM_LOAD( "cdi.h3",     0x0000, 0x0100, CRC(82d7d25f) SHA1(d4b3a6655f91647545d493c2ff996daa66df0395) )
	ROM_LOAD( "cdi.h6",     0x0000, 0x0100, CRC(c3823f0b) SHA1(42fe8c1e0f54b3f968a630dd564a8941410c5d86) )
	ROM_LOAD( "cdi.n1",     0x0000, 0x0100, CRC(82d7d25f) SHA1(d4b3a6655f91647545d493c2ff996daa66df0395) )

	ROM_LOAD( "cdi.c13",    0x0000, 0x0020, CRC(f304a1fb) SHA1(0f029274bb99723ebcc271d761e1500ca50b2738) )
	ROM_LOAD( "cdi.c14",    0x0000, 0x0020, CRC(f8dbd779) SHA1(55bdaf9eb1ba6185e20512c4874ebb625861508e) )
	ROM_LOAD( "cdi.d5",     0x0000, 0x0020, CRC(e9af4217) SHA1(813e126b1263e13f1684d700c5c4bec34d063b38) )
	ROM_LOAD( "cdi.n13",    0x0000, 0x0020, CRC(2e83bf80) SHA1(02fcc1e879c06759a21ef4f004fe7aa790814112) )
	ROM_LOAD( "cdi.g2",     0x0000, 0x0020, CRC(5ed8cdd2) SHA1(d193d819ad634c43d648ce49073799b4df6dfd2f) )
	ROM_LOAD( "cdi.j10",    0x0000, 0x0020, CRC(e9af4217) SHA1(813e126b1263e13f1684d700c5c4bec34d063b38) )
	ROM_LOAD( "cdi.r1",     0x0000, 0x0020, CRC(5ed8cdd2) SHA1(d193d819ad634c43d648ce49073799b4df6dfd2f) )
ROM_END

} // anonymous namespace


GAME( 1976, dmodrbcc, 0, chicago, 0, chicago_state, empty_init, ROT0, "Chicago Coin", "Demolition Derby (Chicago Coin)", MACHINE_IS_SKELETON )
