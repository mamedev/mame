// license:BSD-3-Clause
// copyright-holders:

/*
Crazy Balls by E.G.S. (Electronic Games Systems)

http://www.tilt.it/deb/egs-en.html

Entirely TTL. Believed to be the first arcade game to have high score initials entry.
This was achieved via a keyboard on the control panel: http://www.citylan.it/wiki/images/c/c3/1698_control_panel_%2B_ingame.jpg

main PCB is marked: "EGS 113 [S]" on component side
sub PCB is marked: "EGS 114 [S]" on component side
27.025 OSC on main PCB

A PCB set is available for tracing.
*/


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

	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

private:
	// devices
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
	/* basic machine hardware */
	NETLIST_CPU(config, m_maincpu, netlist::config::DEFAULT_CLOCK()).set_source(netlist_crazybal);

	/* video hardware */
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);
	FIXFREQ(config, m_video).set_screen("screen");
	m_video->set_monitor_clock(MASTER_CLOCK);
	m_video->set_horz_params(H_TOTAL-67,H_TOTAL-40,H_TOTAL-8,H_TOTAL);
	m_video->set_vert_params(V_TOTAL-22,V_TOTAL-19,V_TOTAL-12,V_TOTAL);
	m_video->set_fieldcount(1);
	m_video->set_threshold(0.30);
}


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


GAME( 1978, crazybal, 0, crazybal, 0, crazybal_state, empty_init, ROT0, "Electronic Games Systems", "Crazy Balls [TTL]", MACHINE_IS_SKELETON )
