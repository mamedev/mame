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
	required_device<netlist_mame_device_t> m_maincpu;
	required_device<fixedfreq_device> m_video;

protected:

	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();

private:

};


static NETLIST_START(chicago)
	SOLVER(Solver, 48000)
//  PARAM(Solver.FREQ, 48000)
	PARAM(Solver.ACCURACY, 1e-4) // works and is sufficient

	// schematics
	//...

//  NETDEV_ANALOG_CALLBACK(sound_cb, sound, exidyttl_state, sound_cb, "")
//  NETDEV_ANALOG_CALLBACK(video_cb, videomix, fixedfreq_device, update_vid, "fixfreq")
NETLIST_END()



void chicago_state::machine_start()
{
}

void chicago_state::machine_reset()
{
}


void chicago_state::video_start()
{
}

static MACHINE_CONFIG_START( chicago, chicago_state )

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", NETLIST_CPU, NETLIST_CLOCK)
	MCFG_NETLIST_SETUP(chicago)

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


ROM_START( dmodrbcc )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0400, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "cdi.b2",     0x0000, 0x0200, CRC(9c727712) SHA1(0d5da25fda57da873da0c066d3472c076c18b853) )
	ROM_LOAD( "cdi.b8",     0x0000, 0x0200, CRC(840653fa) SHA1(b71d35af4bb4a6f98acfa9161c0f3e8165440d16) )
	ROM_LOAD( "cdi.c8",     0x0000, 0x0200, CRC(79ebcfa1) SHA1(5267a38b76f58a9add928d1241803792b35f65c0) )
	ROM_LOAD( "cdi.d8",     0x0000, 0x0200, CRC(afcfe339) SHA1(34912eb50fe162743fa2a7433a1e8634050fb514) )
	ROM_LOAD( "cdi.e8",     0x0000, 0x0200, CRC(7b7bf492) SHA1(6d48ae1963c6f867a461d15e657df8196fdeaf20) )
	ROM_LOAD( "cdi.h3",     0x0000, 0x0200, CRC(66f25de0) SHA1(1984c3f9a4f5c01b1db784cec7279307aa9a851b) )
	ROM_LOAD( "cdi.h6",     0x0000, 0x0200, CRC(9c727712) SHA1(0d5da25fda57da873da0c066d3472c076c18b853) )
	ROM_LOAD( "cdi.n1",     0x0000, 0x0200, CRC(66f25de0) SHA1(1984c3f9a4f5c01b1db784cec7279307aa9a851b) )

	ROM_LOAD( "cdi.c13",    0x0000, 0x0020, CRC(f304a1fb) SHA1(0f029274bb99723ebcc271d761e1500ca50b2738) )
	ROM_LOAD( "cdi.c14",    0x0000, 0x0020, CRC(f8dbd779) SHA1(55bdaf9eb1ba6185e20512c4874ebb625861508e) )
	ROM_LOAD( "cdi.d5",     0x0000, 0x0020, CRC(e9af4217) SHA1(813e126b1263e13f1684d700c5c4bec34d063b38) )
	ROM_LOAD( "cdi.n13",    0x0000, 0x0020, CRC(2e83bf80) SHA1(02fcc1e879c06759a21ef4f004fe7aa790814112) )
	ROM_LOAD( "cdi.g2",     0x0000, 0x0020, CRC(5ed8cdd2) SHA1(d193d819ad634c43d648ce49073799b4df6dfd2f) )
	ROM_LOAD( "cdi.j10",    0x0000, 0x0020, CRC(e9af4217) SHA1(813e126b1263e13f1684d700c5c4bec34d063b38) )
	ROM_LOAD( "cdi.r1",     0x0000, 0x0020, CRC(5ed8cdd2) SHA1(d193d819ad634c43d648ce49073799b4df6dfd2f) )
ROM_END


GAME( 1976, dmodrbcc,  0, chicago, 0, driver_device,  0, ROT0, "Chicago Coin", "Demolition Derby [TTL]", MACHINE_IS_SKELETON )
