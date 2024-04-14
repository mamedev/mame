// license:BSD-3-Clause
// copyright-holders:

/*
This driver contains early non-CPU Midway dumps, until they are emulated and moved as needed
*/

#include "emu.h"

#include "machine/netlist.h"
#include "netlist/devices/net_lib.h"
#include "video/fixfreq.h"


namespace {

// TODO: just a placeholder, everything needs to be updated

#define MASTER_CLOCK    7159000
#define V_TOTAL         (0x105+1)       // 262
#define H_TOTAL         (0x1C6+1)       // 454

#define HBSTART                 (H_TOTAL)
#define HBEND                   (80)
#define VBSTART                 (V_TOTAL)
#define VBEND                   (16)

#define HRES_MULT                   (1)


class midwayttl_state : public driver_device
{
public:
	midwayttl_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_video(*this, "fixfreq")
	{
	}

	void midwayttl(machine_config &config);

private:
	required_device<netlist_mame_device> m_maincpu;
	required_device<fixedfreq_device> m_video;

};


static NETLIST_START(midwayttl) // TODO: just a placeholder, everything needs to be updated
{
	SOLVER(Solver, 48000)
//  PARAM(Solver.FREQ, 48000)
	PARAM(Solver.ACCURACY, 1e-4)

	// schematics
	//...
}


void midwayttl_state::midwayttl(machine_config &config)
{
	// basic machine hardware
	NETLIST_CPU(config, m_maincpu, netlist::config::DEFAULT_CLOCK()).set_source(netlist_midwayttl);

	// video hardware
	SCREEN(config, "screen", SCREEN_TYPE_RASTER); // TODO: just a placeholder, everything needs to be updated
	FIXFREQ(config, m_video).set_screen("screen");
	m_video->set_monitor_clock(MASTER_CLOCK);
	m_video->set_horz_params(H_TOTAL-67,H_TOTAL-40,H_TOTAL-8,H_TOTAL);
	m_video->set_vert_params(V_TOTAL-22,V_TOTAL-19,V_TOTAL-12,V_TOTAL);
	m_video->set_fieldcount(1);
	m_video->set_threshold(0.30);
}


ROM_START( wheelsii )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x1a0, "roms", 0 ) // All PROMs TI SN74188N. Read as National 74S188
	ROM_LOAD( "257.1k",  0x000, 0x020, CRC(00b8fef8) SHA1(e3f465b9df78cfbd8f4547b8b4d45281e30a2a89) )
	ROM_LOAD( "258.5k",  0x020, 0x020, CRC(eab44d13) SHA1(daed044e5759de22055c4dcaeeb57b12d572ca2b) )
	ROM_LOAD( "259.5l",  0x040, 0x020, CRC(ba31afc0) SHA1(f583dc898e6ecaa1dd6ac78fe9bea6619858d83c) )
	ROM_LOAD( "260.1p",  0x060, 0x020, CRC(2cf891d6) SHA1(625f904b97ed5b714b9174d70dbc247b2cb8cb22) )
	ROM_LOAD( "261.1j",  0x080, 0x020, CRC(8aa08f16) SHA1(7fc6a6a3b75640beddffa482012e724f0af332b3) )
	ROM_LOAD( "262.15j", 0x0a0, 0x020, CRC(814d0588) SHA1(6680b5dcd9af9ac1cef310b462a0f82c63616967) )
	ROM_LOAD( "263.15p", 0x0c0, 0x020, CRC(ada34863) SHA1(4491b38c5e17242459f38ac7c683371f7fdfe05e) )
	ROM_LOAD( "264.15r", 0x0e0, 0x020, CRC(a7114570) SHA1(1f91d1cc08cc3c4943bd6a6ac231f43935853cbe) )
	ROM_LOAD( "265.7p",  0x100, 0x020, CRC(501953ba) SHA1(f1016676c90def8a6aff1c8999c2a6ab2cb10398) )
	ROM_LOAD( "266.7r",  0x120, 0x020, CRC(a0eeb385) SHA1(7044c1ab239e3ec60a63b841c09cdf2b1d25a39b) )
	ROM_LOAD( "267.9p",  0x140, 0x020, CRC(a6983222) SHA1(265501cda78d85bdd82b0b6fa4731b519e40a9b7) )
	ROM_LOAD( "268.9r",  0x160, 0x020, CRC(715cb8da) SHA1(b72c8b549220b36d179b03ab5dfafda20d66fd56) )
	ROM_LOAD( "269.10k", 0x180, 0x020, CRC(eeb6a479) SHA1(e4f0540c1c5d0a5c9037ce9e9dcc1150eeeea3f9) )
ROM_END

} // anonymous namespace


GAME( 1975, wheelsii, 0, midwayttl, 0, midwayttl_state, empty_init, ROT0, "Midway", "Wheels II", MACHINE_IS_SKELETON )
