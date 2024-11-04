// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

 Bailey International discrete hardware games

 Fighter Squadron (1976)
 Fun Four (1976)
 Missile Command (1976)
 Sebring (1976)

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


class bailey_state : public driver_device
{
public:
	bailey_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_video(*this, "fixfreq")
	{
	}

	// devices
	required_device<netlist_mame_device> m_maincpu;
	required_device<fixedfreq_device> m_video;

	void bailey(machine_config &config);
protected:

	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void video_start() override ATTR_COLD;

private:

};


static NETLIST_START(bailey)
{
	SOLVER(Solver, 48000)
//  PARAM(Solver.FREQ, 48000)
	PARAM(Solver.ACCURACY, 1e-4) // works and is sufficient

	// schematics
	//...

//  NETDEV_ANALOG_CALLBACK(sound_cb, sound, exidyttl_state, sound_cb, "")
//  NETDEV_ANALOG_CALLBACK(video_cb, videomix, fixedfreq_device, update_vid, "fixfreq")
}



void bailey_state::machine_start()
{
}

void bailey_state::machine_reset()
{
}


void bailey_state::video_start()
{
}

void bailey_state::bailey(machine_config &config)
{
	/* basic machine hardware */
	NETLIST_CPU(config, m_maincpu, netlist::config::DEFAULT_CLOCK()).set_source(netlist_bailey);

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


ROM_START( fun4 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0400, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "bailey.bh3",    0x0000, 0x0200, CRC(ca44974f) SHA1(5ac7b764fa89ba93ad7c2a7db649f34ce16d526b) )
	ROM_LOAD( "bailey.d3",     0x0000, 0x0200, CRC(419aa79b) SHA1(8b7a5da33e24f9e13212721757165c53340828ce) )
	ROM_LOAD( "bailey.f4",     0x0000, 0x0200, CRC(3558736b) SHA1(a32b76a8691abf06972f6a9ac784c31dbb42f3f1) )
	ROM_LOAD( "bailey.g2",     0x0000, 0x0200, CRC(426290ba) SHA1(d40c9632ca64ac1445c2e05f9b62798a3e96cd81) )
	ROM_LOAD( "bailey.g3",     0x0000, 0x0200, CRC(5dfc36ac) SHA1(c13209aea3038928bae3610b6071efe205bc6273) )
	ROM_LOAD( "bailey.g5",     0x0000, 0x0200, CRC(477f6fca) SHA1(0b522b6a952e2fb17d657af74ac5759db81ba7b4) )
	ROM_LOAD( "bailey.h2",     0x0000, 0x0200, CRC(a2b6e2fa) SHA1(c92c1e45c55103de1ce9069f368eb2af5ba23906) )
	ROM_LOAD( "bailey.h4",     0x0000, 0x0200, CRC(47d757fc) SHA1(24f4459ed174308c69f48d43fb8cdf88c494b954) )
	ROM_LOAD( "bailey.h5",     0x0000, 0x0200, CRC(26fa6907) SHA1(c2a3d648983d27d94f414412d6eda005c923013c) )
	ROM_LOAD( "bailey.th3",    0x0000, 0x0200, CRC(9d66f473) SHA1(eeef0f10bdd1ac01db2870ff4b0084c97c268bd6) )

	ROM_LOAD( "bailey.e3",     0x0000, 0x0020, CRC(82e5e448) SHA1(549925b8e9280ed102396dca1acaa8c999838993) )
	ROM_LOAD( "bailey.f3",     0x0000, 0x0020, CRC(c3a5b496) SHA1(662bd8b66659a02c0b6a5291ac1ac134ddc212b7) )
	ROM_LOAD( "bailey.g4",     0x0000, 0x0020, CRC(99f918d5) SHA1(74fb3a0f0f86b2900a1bd0dac77ae7e129885a1d) )
ROM_END


ROM_START( fun4a )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0400, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "bailey1.bh3",    0x0000, 0x0200, CRC(58df5484) SHA1(3cbc2fadde7c482efd1282ecacc254411401e3d6) )
	ROM_LOAD( "bailey1.d3",     0x0000, 0x0200, CRC(c3e586ba) SHA1(39f4ab41a62d52ba8ccf722c1f190afc439a607c) )
	ROM_LOAD( "bailey1.f4",     0x0000, 0x0200, CRC(b727524a) SHA1(e24df1bdc9a4469602f2e9e59be9882e3c1a1988) )
	ROM_LOAD( "bailey1.g2",     0x0000, 0x0200, CRC(c01db19b) SHA1(f38c05518b8eef34f953471e63d0245c17a756de) )
	ROM_LOAD( "bailey1.g3",     0x0000, 0x0200, CRC(df83178d) SHA1(3a7ce78bc354e23d3dbc106095e49122d0e29607) )
	ROM_LOAD( "bailey1.g5",     0x0000, 0x0200, CRC(c5004eeb) SHA1(b948c8d00c51ceb6622f504a6dc01074e19da17d) )
	ROM_LOAD( "bailey1.h2",     0x0000, 0x0200, CRC(20c9c3db) SHA1(3b6a135353b9f4e919dc5c271c255552a086c1a6) )
	ROM_LOAD( "bailey1.h4",     0x0000, 0x0200, CRC(c5a876dd) SHA1(47a6ecf8b59c23f616a6ec0559a4225195752f0e) )
	ROM_LOAD( "bailey1.h5",     0x0000, 0x0200, CRC(a4854826) SHA1(0c6cd51acf80c8f3691c5d5025b3df44064cc37a) )
	ROM_LOAD( "bailey1.th3",    0x0000, 0x0200, CRC(0ffd37b8) SHA1(5937704be2f745939e6271f1a69474337c4d721f) )

	ROM_LOAD( "bailey1.e3",     0x0000, 0x0020, CRC(82e5e448) SHA1(549925b8e9280ed102396dca1acaa8c999838993) )
	ROM_LOAD( "bailey1.f3",     0x0000, 0x0020, CRC(c3a5b496) SHA1(662bd8b66659a02c0b6a5291ac1ac134ddc212b7) )
	ROM_LOAD( "bailey1.g4",     0x0000, 0x0020, CRC(99f918d5) SHA1(74fb3a0f0f86b2900a1bd0dac77ae7e129885a1d) )
ROM_END

} // anonymous namespace


GAME( 1976, fun4,  0,    bailey, 0, bailey_state, empty_init, ROT0, "Bailey International", "Fun Four (set 1)", MACHINE_IS_SKELETON )
GAME( 1976, fun4a, fun4, bailey, 0, bailey_state, empty_init, ROT0, "Bailey International", "Fun Four (set 2)", MACHINE_IS_SKELETON )
