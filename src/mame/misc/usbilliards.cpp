// license:BSD-3-Clause
// copyright-holders:Stiletto
/***************************************************************************

 US Billiards TTL Games

 Game Name                       DATA
 -------------------------------------
 TV Tennis (1973/03,11?)         NO
 TV Hockey? (1973)               UNKNOWN
 Survival (1975/02-03?)          UNKNOWN
 Shark (1975/11)                 YES
 Space Battle (1977/10-12?)      UNKNOWN
 Variety TV (1978/09)            UNKNOWN

 ***************************************************************************/


#include "emu.h"

#include "machine/netlist.h"
#include "netlist/devices/net_lib.h"
#include "video/fixfreq.h"


namespace {

// copied from Pong, not accurate for this driver!
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


class usbilliards_state : public driver_device
{
public:
	usbilliards_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_video(*this, "fixfreq")
	{
	}

	void usbilliards(machine_config &config);

private:
	// devices
	required_device<netlist_mame_device> m_maincpu;
	required_device<fixedfreq_device> m_video;

	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void video_start() override ATTR_COLD;
};


static NETLIST_START(usbilliards)
{
	SOLVER(Solver, 48000)
//  PARAM(Solver.FREQ, 48000)
	PARAM(Solver.ACCURACY, 1e-4) // works and is sufficient

	// schematics
	//...

		//  NETDEV_ANALOG_CALLBACK(sound_cb, sound, usbilliards_state, sound_cb, "")
		//  NETDEV_ANALOG_CALLBACK(video_cb, videomix, fixedfreq_device, update_vid, "fixfreq")
}



void usbilliards_state::machine_start()
{
}

void usbilliards_state::machine_reset()
{
}


void usbilliards_state::video_start()
{
}

void usbilliards_state::usbilliards(machine_config &config)
{
	/* basic machine hardware */
	NETLIST_CPU(config, m_maincpu, netlist::config::DEFAULT_CLOCK()).set_source(netlist_usbilliards);

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

/*
Shark by U.S. Billiards

Etched in copper on Top     (C) 1975
                010
                1SCOOP J6 2SCOOP

Handwritten on top      J0037
                124

empty socket at 5M  C etched in copper next to socket

*/

ROM_START( sharkusb )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0200, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "82s123_b.5n",  0x0000, 0x0100, CRC(91b977b3) SHA1(37929f6049dea0ebed2c01ae20354e41c867b8f9) ) // 82s123 - handwritten B - B also etched in copper next to socket
	ROM_LOAD( "82s123_a.6n",  0x0100, 0x0100, CRC(63f621cb) SHA1(6c6e6f22313db33afd069dae1b0180b5ccddfa56) ) // 82s123 - handwritten A - A also etched in copper next to socket
ROM_END

} // anonymous namespace


GAME( 1975, sharkusb,    0,       usbilliards, 0, usbilliards_state, empty_init, ROT0, "US Billiards Inc.", "Shark (US Billiards)", MACHINE_IS_SKELETON )
