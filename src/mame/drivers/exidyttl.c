// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

Exidy discrete hardware games

 Alley Rally (1975) (AR-1A)
 Attack (1977)
 Death Race (1976)
 Destruction Derby (1975)
 Hockey / Tennis (Thumper Bumper?) (1974)
 Score (1977)
 Spiders From Space (1976)
 Sting (1974)
 Super Death Chase (1977)
 Table Foosballer / Table Football (1975)
 Table Pinball (1974)
 TV Pinball (1974) (PB-4)

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


class exidyttl_state : public driver_device
{
public:
	exidyttl_state(const machine_config &mconfig, device_type type, const char *tag)
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


static NETLIST_START(attack)
	SOLVER(Solver, 48000)
//  PARAM(Solver.FREQ, 48000)
	PARAM(Solver.ACCURACY, 1e-4) // works and is sufficient

	// schematics
	//...

//  NETDEV_ANALOG_CALLBACK(sound_cb, sound, exidyttl_state, sound_cb, "")
//  NETDEV_ANALOG_CALLBACK(video_cb, videomix, fixedfreq_device, update_vid, "fixfreq")
NETLIST_END()



void exidyttl_state::machine_start()
{
}

void exidyttl_state::machine_reset()
{
}


void exidyttl_state::video_start()
{
}

static MACHINE_CONFIG_START( attack, exidyttl_state )

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", NETLIST_CPU, NETLIST_CLOCK)
	MCFG_NETLIST_SETUP(attack)

	/* video hardware */
	MCFG_FIXFREQ_ADD("fixfreq", "screen")
	MCFG_FIXFREQ_MONITOR_CLOCK(MASTER_CLOCK)
	MCFG_FIXFREQ_HORZ_PARAMS(H_TOTAL-67,H_TOTAL-40,H_TOTAL-8,H_TOTAL)
	MCFG_FIXFREQ_VERT_PARAMS(V_TOTAL-22,V_TOTAL-19,V_TOTAL-12,V_TOTAL)
	MCFG_FIXFREQ_FIELDCOUNT(1)
	MCFG_FIXFREQ_SYNC_THRESHOLD(0.30)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( deathrac, exidyttl_state )

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", NETLIST_CPU, NETLIST_CLOCK)
	MCFG_NETLIST_SETUP(attack)

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


ROM_START( attckexd )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0400, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "attack.a1",     0x0000, 0x0100, CRC(d9b116b8) SHA1(eb988d4f6a46ed7e2657d87343cd925aba678a21) )
	ROM_LOAD( "attack.b1",     0x0000, 0x0100, CRC(2317197f) SHA1(03cfc56bd7166e1af071b8fd0700350f51940ca5) )
	ROM_LOAD( "attack.c1",     0x0000, 0x0100, CRC(7391e44c) SHA1(0a2c9b8f4738c1a49a3169e42817bcd624a4364c) )
	ROM_LOAD( "attack.d1",     0x0000, 0x0100, CRC(d4a06439) SHA1(3da394196810bcdcfd76b2f5c3b06dbc636f875d) )
	ROM_LOAD( "attack.j6",     0x0000, 0x0100, CRC(1ce2921c) SHA1(ba2af281af2770a623de2c82a79be350b030c59f) )
	ROM_LOAD( "attack.k6",     0x0000, 0x0100, CRC(e120839f) SHA1(74dc19a732238d35e467d814ead581a60463aaa2) )
ROM_END

ROM_START( attckexd2 )  //  These are likely an overdump, but we are waiting for confirmation before removing the files
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0400, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "attack__set2.a1",     0x0000, 0x0200, CRC(5afd5aff) SHA1(051915c43b33f476597b2adddda7ae2f5d4fe214) )
	ROM_LOAD( "attack__set2.b1",     0x0000, 0x0200, CRC(92d0fbf4) SHA1(31f7e04c4cd1fb36404b22a26d7e62939b43d941) )
	ROM_LOAD( "attack__set2.c1",     0x0000, 0x0200, CRC(25625d6e) SHA1(d61ff867a226a01781e689c124b93159a92dc057) )
	ROM_LOAD( "attack__set2.d1",     0x0000, 0x0200, CRC(2ff8dd6b) SHA1(21921faa26f95414070df12d30a9bcdc0b674c00) )
	ROM_LOAD( "attack__set2.j6",     0x0000, 0x0200, CRC(21f87c1a) SHA1(1881142ecceee1175f1837e63b7258a08ed293d6) )
	ROM_LOAD( "attack__set2.k6",     0x0000, 0x0200, CRC(ba5115b3) SHA1(1679b07e8a9376789c9cf15fe16e97003e9267be) )
ROM_END


/***********

 Exidy Death Race 1976

 Drawing Name
 ------------
 6331-36.E7 32x8    Right Gremlin
 6301-91.J10    256x4

 6331-36.R7 32x8    Left Gremlin
 6301-91.V10    256x4

 6301-92.V5     P1 (left car)
 6331-35.T7

 6301-92.J5     P2 (right car)
 6331-35.G7

 6301-97.M11        Image Generation
 6301-98.L11
 6301-99.K11
 6301-100.J11

 6331-33.P14        Score & Timer

 6331-31.A11        Timing / Sync
 6331-32.C12

***********/


ROM_START( deathrac )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0400, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "6301-100.j11",   0x0000, 0x0100, CRC(d751bd57) SHA1(a6208af40661bf3cd50363d2ece38cd3b9f6a7a0) )
	ROM_LOAD( "6301-91.j10",    0x0000, 0x0100, CRC(c3823f0b) SHA1(42fe8c1e0f54b3f968a630dd564a8941410c5d86) )
	ROM_LOAD( "6301-91.v10",    0x0000, 0x0100, CRC(c3823f0b) SHA1(42fe8c1e0f54b3f968a630dd564a8941410c5d86) )
	ROM_LOAD( "6301-92.j5",     0x0000, 0x0100, CRC(82d7d25f) SHA1(d4b3a6655f91647545d493c2ff996daa66df0395) )
	ROM_LOAD( "6301-92.v5",     0x0000, 0x0100, CRC(82d7d25f) SHA1(d4b3a6655f91647545d493c2ff996daa66df0395) )
	ROM_LOAD( "6301-97.m11",    0x0000, 0x0100, CRC(2b02444f) SHA1(e1fc01f7271109515438542a223efc0042f794a5) )
	ROM_LOAD( "6301-98.l11",    0x0000, 0x0100, CRC(0bdaf1eb) SHA1(67976e73bfdc4d42a520212d020dd52d51667674) )
	ROM_LOAD( "6301-99.k11",    0x0000, 0x0100, CRC(34763c8f) SHA1(2012ace666e8b82a89a0c15511ee80173d9700bc) )

	ROM_LOAD( "6331-31.a11",    0x0000, 0x0020, CRC(f304a1fb) SHA1(0f029274bb99723ebcc271d761e1500ca50b2738) )
	ROM_LOAD( "6331-32.c12",    0x0000, 0x0020, CRC(f8dbd779) SHA1(55bdaf9eb1ba6185e20512c4874ebb625861508e) )
	ROM_LOAD( "6331-33.p14",    0x0000, 0x0020, CRC(2e83bf80) SHA1(02fcc1e879c06759a21ef4f004fe7aa790814112) )
	ROM_LOAD( "6331-36.e7",     0x0000, 0x0020, CRC(1358c8d5) SHA1(2fa1041f30f3a6775393714a65c416738b06b330) )
	ROM_LOAD( "6331-36.g7",     0x0000, 0x0020, CRC(15e00a2a) SHA1(cd43d227a34e5444ed9d8a4acf5497df9c789c73) )
	ROM_LOAD( "6331-36.r7",     0x0000, 0x0020, CRC(1358c8d5) SHA1(2fa1041f30f3a6775393714a65c416738b06b330) )
	ROM_LOAD( "6331-36.t7",     0x0000, 0x0020, CRC(15e00a2a) SHA1(cd43d227a34e5444ed9d8a4acf5497df9c789c73) )
ROM_END



GAME( 1977, attckexd,  0,        attack,   0, driver_device,  0, ROT0, "Exidy", "Attack (Set 1) [TTL]", MACHINE_IS_SKELETON )
GAME( 1977, attckexd2, attckexd, attack,   0, driver_device,  0, ROT0, "Exidy", "Attack (Set 2) [TTL]", MACHINE_IS_SKELETON )
GAME( 1976, deathrac,  0,        deathrac, 0, driver_device,  0, ROT0, "Exidy", "Death Race [TTL]", MACHINE_IS_SKELETON )
