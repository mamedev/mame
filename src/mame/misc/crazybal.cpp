// license:BSD-3-Clause
// copyright-holders:

/*
Crazy Balls by E.G.S. (Electronic Games Systems)

http://www.tilt.it/deb/egs-en.html

Entirely TTL. Believed to be the first arcade game to have high score initials entry.
This was achieved via a keyboard on the control panel: http://www.citylan.it/wiki/images/c/c3/1698_control_panel_%2B_ingame.jpg

crazybal:
main PCB is marked: "EGS 113 [S]" on component side
sub PCB is marked: "EGS 114 [S]" on component side
27.025 OSC on main PCB

crazybala:
main PCB is marked: "EGS 106" on component side
sub PCB is marked: "EGS 107" on component side

PCB set 113 + 114 is available for tracing.
Schematics are available for PCB set 106 + 107.
*/


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


class crazybal_state : public driver_device
{
public:
	crazybal_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_video(*this, "fixfreq")
	{
	}

	void crazybal(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void video_start() override ATTR_COLD;

private:
	required_device<netlist_mame_device> m_maincpu;
	required_device<fixedfreq_device> m_video;
};


static NETLIST_START(crazybal)
{
	SOLVER(Solver, 48000)
//  PARAM(Solver.FREQ, 48000)
	PARAM(Solver.ACCURACY, 1e-4)

	// schematics
	//...
}



void crazybal_state::machine_start()
{
}

void crazybal_state::machine_reset()
{
}


void crazybal_state::video_start()
{
}

void crazybal_state::crazybal(machine_config &config)
{
	// basic machine hardware
	NETLIST_CPU(config, m_maincpu, netlist::config::DEFAULT_CLOCK()).set_source(netlist_crazybal);

	// video hardware
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);
	FIXFREQ(config, m_video).set_screen("screen");
	m_video->set_monitor_clock(MASTER_CLOCK);
	m_video->set_horz_params(H_TOTAL-67,H_TOTAL-40,H_TOTAL-8,H_TOTAL);
	m_video->set_vert_params(V_TOTAL-22,V_TOTAL-19,V_TOTAL-12,V_TOTAL);
	m_video->set_fieldcount(1);
	m_video->set_threshold(0.30);
}


// EGS 113 + EGS 114 PCBs
ROM_START( crazybal )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0600, "mainpcb_proms", ROMREGION_ERASE00 ) // all Sn74S287N
	ROM_LOAD( "1.6c",      0x000, 0x100, CRC(d5700b52) SHA1(372b011de9eb2dab80df589103a860bf63dd7e7d) )
	ROM_LOAD( "2.1a",      0x100, 0x100, CRC(09cfacf9) SHA1(7f9f8424d92d1d5a6ccf8c041ad6547ca4984622) )
	ROM_LOAD( "4.8e",      0x200, 0x100, CRC(f1c02c73) SHA1(6d1c61a77514497f95457f7056e0efe342483d46) )
	ROM_LOAD( "5.11g",     0x300, 0x100, CRC(65c6cfc9) SHA1(a327d0ee9cc101f5bdc970108e4378140ebbd765) )
	ROM_LOAD( "5.4h",      0x400, 0x100, CRC(65c6cfc9) SHA1(a327d0ee9cc101f5bdc970108e4378140ebbd765) )
	ROM_LOAD( "6.11a",     0x500, 0x100, CRC(4c83ffa3) SHA1(2a9f9e88b6fc3334a5e7cba52d39ae567043d8be) )

	ROM_REGION( 0x100, "subpcb_proms", ROMREGION_ERASE00 )
	ROM_LOAD( "3.12l",     0x000, 0x100, CRC(e2ca8670) SHA1(60bc4be4185c50a9afd3a28d1fb9e8f46c93764a) ) // Sn74S287N
ROM_END

// EGS 106 + EGS 107 PCBs (found in a cabinet distributed by Bontempi)
ROM_START( crazybala )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0340, "mainpcb_proms", ROMREGION_ERASE00 )
	ROM_LOAD( "74s287.12c", 0x000, 0x100, CRC(1f05c2df) SHA1(189e90ca29ef043ed1d4640aed3fa472a4e26da8) )
	ROM_LOAD( "74s287.12d", 0x100, 0x100, CRC(f1d7a030) SHA1(697cf26dbfbd6207cb1dd2e098d6dba9b0bdbaf3) )
	ROM_LOAD( "74s287.13e", 0x200, 0x100, CRC(57280959) SHA1(8162d9868e8367ba4c21712dc275ce4888cfbc70) )
	ROM_LOAD( "6331.4g",    0x300, 0x020, CRC(21ee11cc) SHA1(4cdf16665015ee984a300e59c73eb2aa12c13e4e) )
	ROM_LOAD( "74s188.6h",  0x320, 0x020, CRC(7b4b2f9f) SHA1(96b4a90e3c51582434e0eca1c3701cc2183dd372) )
ROM_END

// EGS 106 + EGS 107 PCBs. Only one PROM (at 4G) differs from the other set on this hw version.
// It affects the shape of the 6 bumpers in the middle of the screen.
ROM_START( crazybalb )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0340, "mainpcb_proms", ROMREGION_ERASE00 )
	ROM_LOAD( "mmi6331.12c", 0x000, 0x100, CRC(1f05c2df) SHA1(189e90ca29ef043ed1d4640aed3fa472a4e26da8) )
	ROM_LOAD( "mmi6331.12d", 0x100, 0x100, CRC(f1d7a030) SHA1(697cf26dbfbd6207cb1dd2e098d6dba9b0bdbaf3) )
	ROM_LOAD( "mmi6331.13e", 0x200, 0x100, CRC(57280959) SHA1(8162d9868e8367ba4c21712dc275ce4888cfbc70) )
	ROM_LOAD( "74s188.4g",   0x300, 0x020, CRC(fd10b3e7) SHA1(b5464d198f08d770a781ee284ba56987d1bcff12) )
	ROM_LOAD( "74s188.6h",   0x320, 0x020, CRC(7b4b2f9f) SHA1(96b4a90e3c51582434e0eca1c3701cc2183dd372) )
ROM_END

} // anonymous namespace


GAME( 1978, crazybal,  0,        crazybal, 0, crazybal_state, empty_init, ROT0, "Electronic Games Systems / NAT",      "Crazy Balls (NAT)",             MACHINE_IS_SKELETON )
GAME( 1978, crazybala, crazybal, crazybal, 0, crazybal_state, empty_init, ROT0, "Electronic Games Systems / Bontempi", "Crazy Balls (Bontempi, set 1)", MACHINE_IS_SKELETON )
GAME( 1978, crazybalb, crazybal, crazybal, 0, crazybal_state, empty_init, ROT0, "Electronic Games Systems / Bontempi", "Crazy Balls (Bontempi, set 2)", MACHINE_IS_SKELETON )
