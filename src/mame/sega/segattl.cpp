// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Scott Stone
/***************************************************************************

 known Sega Discrete Hardware Games


 Game Name(s)                                                        Part #'s       Data      PROM/ROM Chip Numbers
 ------------------------------------------------------------------+--------------+---------+---------------------------------

    3 Way Block
    Balloon Gun
    Bomber
    Break Open
    Bullet Mark
    Cartoon Gun
    Castling
    Crash Course
    Double Block
    Duck Shoot
    Erase
    Fonz                                                             94789-P                   IC86.86 IC87.87 PR-09.49 PR-08.50
    Galaxy War
    Goal Kick
    Heavyweight Champ
    Hockey TV
    Last Inning
    Man TT
    Mini Hockey
    Monaco GP (see monacogp.cpp)
    Moto-Cross
    Pong-Tron
    Pong-Tron II
    Pro Monaco GP
    Pro Racer
    Road Race                                                        94540Y-P
    Rock'n Bark
    Secret Base
    Secret War
    Senkan Yamato
    Space Fighter
    Sparkling Corner
    Squadron
    Super Break Open
    Table Baseball
    Table Hockey
    Top Runner
    Tracer
    Twin Course T.T.
    Upset Block
    Wild Wood
    World Cup
    Zigzag Block


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


class segattl_state : public driver_device
{
public:
	segattl_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_video(*this, "fixfreq")
	{
	}

	void segattl(machine_config &config);

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


static NETLIST_START(segattl)
{
	SOLVER(Solver, 48000)
//  PARAM(Solver.FREQ, 48000)
	PARAM(Solver.ACCURACY, 1e-4) // works and is sufficient

	// schematics
	//...

	//  NETDEV_ANALOG_CALLBACK(sound_cb, sound, exidyttl_state, sound_cb, "")
	//  NETDEV_ANALOG_CALLBACK(video_cb, videomix, fixedfreq_device, update_vid, "fixfreq")
}



void segattl_state::machine_start()
{
}

void segattl_state::machine_reset()
{
}


void segattl_state::video_start()
{
}

void segattl_state::segattl(machine_config &config)
{
	/* basic machine hardware */
	NETLIST_CPU(config, m_maincpu, netlist::config::DEFAULT_CLOCK()).set_source(netlist_segattl);

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
IC86.86     MMI 6306 (dumped as AMD 27S13)  0390
IC87.87     MMI 6306 (dumped as AMD 27S13)  050D

PR-09.49    TI 74S287
PR-08.50    TI 74S287
*/

ROM_START( fonz )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0600, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "ic86.86",      0x000, 0x200, CRC(627a7795) SHA1(88304451ffbd7231d397ada632c7bf6f8cbe7598) )
	ROM_LOAD( "ic87.87",      0x200, 0x200, CRC(d7dd14c7) SHA1(2f112197d7caafb4cb7e2658ea8b298e15aba6ac) )
	ROM_LOAD( "pr-09.49",     0x400, 0x100, CRC(2e293727) SHA1(a03eb4aa726a2e2c872f40dfd1628b00d4edfa33) )
	ROM_LOAD( "pr-08.50",     0x500, 0x100, CRC(6c763af7) SHA1(fdfa310f54b88610c1f59345f5fc72b3b90641ad) )
ROM_END

} // anonymous namespace


GAME( 1976, fonz, 0, segattl, 0, segattl_state, empty_init, ROT0, "Sega", "Fonz", MACHINE_IS_SKELETON )
