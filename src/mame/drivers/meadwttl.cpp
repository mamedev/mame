// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

 Meadows Discrete Game List

 Bombs Away (1976)
 Ckidzo (1976)
 Cobra Gunship (1976)
 Drop Zone 4 (1975)
 Flim Flam (1974)
 Flim Flam II (1975)
 Gridiron (1977)
 Meadows 4 in 1 (197?)
 Star Shooter (1975)

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


class meadwttl_state : public driver_device
{
public:
	meadwttl_state(const machine_config &mconfig, device_type type, const char *tag)
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


static NETLIST_START(meadows)
	SOLVER(Solver, 48000)
//  PARAM(Solver.FREQ, 48000)
	PARAM(Solver.ACCURACY, 1e-4) // works and is sufficient

	// schematics
	//...

//  NETDEV_ANALOG_CALLBACK(sound_cb, sound, exidyttl_state, sound_cb, "")
//  NETDEV_ANALOG_CALLBACK(video_cb, videomix, fixedfreq_device, update_vid, "fixfreq")
NETLIST_END()



void meadwttl_state::machine_start()
{
}

void meadwttl_state::machine_reset()
{
}


void meadwttl_state::video_start()
{
}

static MACHINE_CONFIG_START( meadows, meadwttl_state )

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", NETLIST_CPU, NETLIST_CLOCK)
	MCFG_NETLIST_SETUP(meadows)

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


ROM_START( bombaway )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0400, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "b.3j",      0x0000, 0x0200, CRC(d1e3ddfd) SHA1(268915eba79179b7329076c875172f910cf77930) ) // bottom row (4 point) ship graphic
	ROM_LOAD( "c.4j",      0x0000, 0x0200, CRC(95108ae8) SHA1(872596a666bfc03fcc40b1f8c532d41951b0b506) ) // middle row (2 point) ship graphic
	ROM_LOAD( "a.5j",      0x0000, 0x0200, CRC(3804bc84) SHA1(ba943bdb3fa1ab8210da0d4613a641fd2578eca2) ) // top row (1 point) ship graphic
ROM_END

ROM_START( ckidzo )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0400, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "ckidzo.5a",     0x0000, 0x0200, CRC(431dc30d) SHA1(1f00136a7688acf8097d58b2e737fd13902db5b5) )
ROM_END

ROM_START( cgunship )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0400, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "cobra.a9",      0x0000, 0x0200, CRC(7e0c767c) SHA1(7dd62186c99cfeec2c31f9366f0961abd4479147) )
	ROM_LOAD( "cobra.a10",     0x0000, 0x0200, CRC(b1ec7340) SHA1(1cd69ed56d77014b957efe0426d769a5ad4899de) )
	ROM_LOAD( "cobra.f9",      0x0000, 0x0020, CRC(851868b9) SHA1(25a33683594829c1a362adefcba770d2369cdcdc) )
	ROM_LOAD( "cobra.f10",     0x0000, 0x0020, CRC(54f7a696) SHA1(c426a530bdf8a7bdc9ef5ef3efdfcdb87ff63164) )
	ROM_LOAD( "cobra.h7",      0x0000, 0x0200, CRC(cd490692) SHA1(50fc0e6d45d20d0f3604936443d3b85da9e8d0ef) )
	ROM_LOAD( "cobra.m6",      0x0000, 0x0200, CRC(54d0e415) SHA1(3a9bacc5c90983f68ed7476323153e787e8c3d2c) )
	ROM_LOAD( "cobra.m11",     0x0000, 0x0200, CRC(3840ac7b) SHA1(4d47cf00968070ad248aa8ad4f72a6a5fc61c82a) )
	ROM_LOAD( "cobra.m13",     0x0000, 0x0200, CRC(b564078e) SHA1(4f49e94b586ba62b28edb8c2eb90e303aa141a62) )

	ROM_LOAD( "cobra.h9",      0x0000, 0x0020, CRC(f31e283c) SHA1(6aecad1ce0a45560edc89f2b3d16f697aa4a822e) )
	ROM_LOAD( "cobra.k9",      0x0000, 0x0020, CRC(8e1cf316) SHA1(85d37e580c34a9ac25de988e9db209d934ee7333) )
	ROM_LOAD( "cobra.k12",     0x0000, 0x0020, CRC(2cd65371) SHA1(40db29163064ebaf2e4ef241c1a06361bce2de60) )
ROM_END

ROM_START( mead4in1 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0400, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "hockey.a6",     0x0000, 0x0200, CRC(32db6d8f) SHA1(841474d3ed5f63dfbe4e537f750c7d42be36b1a9) )
	ROM_LOAD( "hockey.a7",     0x0000, 0x0200, CRC(1faf0755) SHA1(1b550cbf6550301b81e6d4233f2d7e5778db877e) )
	ROM_LOAD( "hockey.a8",     0x0000, 0x0200, CRC(8cc699a3) SHA1(eeb39aa7dc67a855ff774780f56958a3e36da9c8) )
	ROM_LOAD( "hockey.a9",     0x0000, 0x0200, CRC(0485cbce) SHA1(85231fae51a782acea4d3cff896ef8df1c213b74) )
	ROM_LOAD( "hockey.k2",     0x0000, 0x0200, CRC(32645320) SHA1(e4b2d10c0fd8d7e6b2013617fef94394966460e3) )

	ROM_LOAD( "hockey.b12",    0x0000, 0x0020, CRC(cc65da1c) SHA1(ad154032b524c302682a834c814e8676f47eb892) )
	ROM_LOAD( "hockey.b2",     0x0000, 0x0020, CRC(2dab2259) SHA1(1d20d6a3e3ee1719b7e9e92765aea3109be4e375) )
ROM_END



GAME( 1976, bombaway,  0,        meadows,  0,  driver_device,  0, ROT0, "Meadows",  "Bombs Away [TTL]", MACHINE_IS_SKELETON )
GAME( 1976, ckidzo,    0,        meadows,  0,  driver_device,  0, ROT0, "Meadows",  "Ckidzo [TTL]", MACHINE_IS_SKELETON )
GAME( 1976, cgunship,  0,        meadows,  0,  driver_device,  0, ROT0, "Meadows",  "Cobra Gunship [TTL]", MACHINE_IS_SKELETON )
GAME( 197?, mead4in1,  0,        meadows,  0,  driver_device,  0, ROT0, "Meadows",  "Meadows 4 in 1 [TTL]", MACHINE_IS_SKELETON )
