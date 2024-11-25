// license:BSD-3-Clause
// copyright-holders: Fabio Priuli, Dave Widel, gregf

/***************************************************************************

 Project Support Engineering Games

 Game Name                       DATA
 -------------------------------------
 1-2-4 Cocktail Table (197?)     UNKNOWN
 Bazooka (1976/11)               YES
 Desert Patrol (1977/11)         YES
 Espana (cabinet) (1975/10)      NO
 Frenzy (1975/08)                UNKNOWN
 Game Tree (1978/02)             YES
 Hodge Podge (1975?)             NO
 Knights in Armor (1976/06)      YES
 Maneater (1975/11)              YES
 Play Five (1975?)               UNKNOWN
 Scandia (cabinet) (1975/08)     NO
 Space Out (late 1970s)          UNKNOWN (most likely)
 Two Game (1974)                 UNKNOWN
 U.N. Command (1977)             YES?

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


class pse_state : public driver_device
{
public:
	pse_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_video(*this, "fixfreq")
	{
	}

	void pse(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<netlist_mame_device> m_maincpu;
	required_device<fixedfreq_device> m_video;

};


static NETLIST_START(pse)
{
	SOLVER(Solver, 48000)
//  PARAM(Solver.FREQ, 48000)
	PARAM(Solver.ACCURACY, 1e-4) // works and is sufficient

	// schematics
	//...

		//  NETDEV_ANALOG_CALLBACK(sound_cb, sound, psettl_state, sound_cb, "")
		//  NETDEV_ANALOG_CALLBACK(video_cb, videomix, fixedfreq_device, update_vid, "fixfreq")
}



void pse_state::machine_start()
{
}

void pse_state::machine_reset()
{
}


void pse_state::video_start()
{
}

void pse_state::pse(machine_config &config)
{
	// basic machine hardware
	NETLIST_CPU(config, m_maincpu, netlist::config::DEFAULT_CLOCK()).set_source(netlist_pse);

	// video hardware
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

ROM_START( bazooka )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0840, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "bd2.k1",  0x0000, 0x0200, CRC(c9e9ed15) SHA1(624bbc10942a386040aef161b96d64021a842c9f) ) // 6341-1 - gfx: tank, truck, jeep motorcycle
	ROM_LOAD( "bd2.k4",  0x0200, 0x0200, CRC(c5a74df9) SHA1(2846a039e9bf372f3aa0b88ed89f9029eb7f797c) ) // 6341-1 - gfx: ambulance, stretcher, explosion
	ROM_LOAD( "bd1.d2",  0x0400, 0x0200, CRC(4fc10886) SHA1(b1c6f890994ba2182a4e7fc17582d6797dbd6ce9) ) // 6341-1 or 82s115
	ROM_LOAD( "bd1.e2",  0x0600, 0x0200, CRC(00179936) SHA1(e5417b8d3814dafe1278179b307a1b563a378cbe) ) // 6341-1 or 82s115
	ROM_LOAD( "bd2.e6",  0x0800, 0x0020, CRC(14b84564) SHA1(69cdd14e23094678c4b280f60cec963609181b00) ) // 82123
	ROM_LOAD( "bd2.e7",  0x0820, 0x0020, CRC(1bfb073f) SHA1(f6b26dcece71b2cf2ed4a537434edbe31cb10399) ) // 82123
ROM_END

ROM_START( bazookabr )  // 4 identical PROMs were found on a Model Racing "CS 18" PCB, labeled Cross Fir. Unfortunately PROMs at 1l and 6c were missing.
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0840, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "bk01.1l", 0x0000, 0x0200, CRC(edc34cb0) SHA1(f76a81833b015784e55b33189e9058cd24922f9b) ) // 74S473
	ROM_LOAD( "bk02.4l", 0x0200, 0x0200, CRC(3e78e4c2) SHA1(814509eb773bfa87f1df933214f079e7dd2a8fa2) ) // 74S473
	ROM_LOAD( "bk03.8j", 0x0400, 0x0200, CRC(4fc10886) SHA1(b1c6f890994ba2182a4e7fc17582d6797dbd6ce9) ) // 74S473
	ROM_LOAD( "bk04.8h", 0x0600, 0x0200, CRC(00179936) SHA1(e5417b8d3814dafe1278179b307a1b563a378cbe) ) // 74S473
	ROM_LOAD( "bk05.6c", 0x0800, 0x0020, CRC(4193d32e) SHA1(d9e3392a8681198e110cfcd68ef20ae3dc366527) ) // 82S123 or 6331-1J
	ROM_LOAD( "bk06.6d", 0x0820, 0x0020, CRC(1bfb073f) SHA1(f6b26dcece71b2cf2ed4a537434edbe31cb10399) ) // 82S123 or 6331-1J
ROM_END

ROM_START( dpatrol )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0ca0, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "bd1.d2", 0x0000, 0x0400, CRC(e4c8e4ab) SHA1(0b989ca9369139f212dcea1d1461998f20057db8)) // Computer program game code. 6341-1 or 82S181 according to Desert Patrol schematics - sldh w/dpatrola
	ROM_LOAD( "bd1.e2", 0x0400, 0x0400, CRC(256b3320) SHA1(712573e3d9625a84c54bbe2e3edafb8879a14b2e)) // Computer program game code. 6341-1 or 82S181 according to Desert Patrol schematics - sldh w/dpatrola

	ROM_LOAD( "bd2.l4", 0x0800, 0x0200, CRC(bc87c648) SHA1(c4709d155aa50cc87146abd152a11de618cfd64c)) // PROM 1 contains aircraft target images and explosion image. PCB has 82S141; schematics show 6341-1
	ROM_LOAD( "bd2.l1", 0x0a00, 0x0200, CRC(4ddcc237) SHA1(6bfad6a8bf8387e93c0bb1a04b647690b3701d54)) // PROM 2 contains parachute and man, falling man. PCB has 82S141; schematics show 6341-1 (from dpatrola, but expected to match)

	ROM_LOAD( "bd2.h7", 0x0c00, 0x0020, NO_DUMP) // Contains PROM address codes and image speeds. Each image has its own speed and address block in the image PROM. Chip is 82S123

	ROM_LOAD( "bd3.d1", 0x0c20, 0x0080, NO_DUMP ) // Data in PROM is organized to produce the waveform of a human scream. Chip type is 8574 or MM6301-0J
ROM_END

ROM_START( dpatrola )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0900, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "bd1.d2", 0x0000, 0x0200, CRC(dd30f565) SHA1(04676adf9fe172c5332bdc9d235a899c7dbe90b5)) // Computer program game code. PCB has SN74S474; schematics show 6341-1 or 82S181 - sldh w/dpatrol
	ROM_LOAD( "bd1.e2", 0x0200, 0x0200, CRC(e1f0941b) SHA1(57f51e9a74838708c3017c5a00e8ec33c6445e47)) // Computer program game code. PCB has SN74S474; schematics show 6341-1 or 82S181 - sldh w/dpatrol

	ROM_LOAD( "bd2.l4", 0x0400, 0x0200, CRC(bc87c648) SHA1(c4709d155aa50cc87146abd152a11de618cfd64c)) // PROM 1 contains aircraft target images and explosion image. PCB has SN74S474; schematics show 6341-1
	ROM_LOAD( "bd2.l1", 0x0600, 0x0200, CRC(4ddcc237) SHA1(6bfad6a8bf8387e93c0bb1a04b647690b3701d54)) // PROM 2 contains parachute and man, falling man. PCB has SN74S474; schematics show 6341-1

	ROM_LOAD( "bd2.h7", 0x0800, 0x0020, CRC(7e8f20a2) SHA1(1fd16e10b7913bc6d949c7fc117ee80a87371777)) // Contains PROM address codes and image speeds. Each image has its own speed and address block in the image PROM. PCB had SN74S288; schematics show 82S123

	ROM_LOAD( "bd3.d1", 0x0820, 0x0080, NO_DUMP ) // Data in PROM is organized to produce the waveform of a human scream. Chip type is 8574 or MM6301-0J. Was not present on dumped PCB.
ROM_END

ROM_START( knightar )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0400, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "1.m1", 0x0000, 0x0200, CRC(7aa9c36c) SHA1(e06c4bf3311cd818d57c96d937e315d433dce457) ) // Man ROM stores image characters of knights. 82S115P or 8205R according to Knights in Armor schematics, but MMI 6341 on the dumped PCB
	ROM_LOAD( "2.m2", 0x0200, 0x0200, CRC(a6705909) SHA1(0cbf033bff33b1b0a45e9190f527b7c00507250d) ) // Horse ROM stores image characters of horses. 82S115P or 8205R according to Knights in Armor schematics, but MMI 6341 on the dumped PCB
ROM_END



ROM_START( gametree )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x1040, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "bd1gtlo.d2", 0x0000, 0x0400, CRC(d44cf6a3) SHA1(66e9a8c0184ac53bf300fc5d5d6c8091f829485c)) // Computer program game code. PCB has Signetics 82s2708; schematics show 6341-1 or 82S181
	ROM_LOAD( "bd1gthi.f2", 0x0400, 0x0400, CRC(3c5a04ac) SHA1(427bae562c019257bcd050458d64670874d903fb)) // Computer program game code. PCB has Signetics 82s2708; schematics show 6341-1 or 82S181

	ROM_LOAD( "bd2gt1a.f12", 0x0800, 0x0200, CRC(820cec79) SHA1(b7142d75ba1cd4ebb0b69dd1184c6e1ea0611ba9)) // PROM 1 contains squirrel and squirrel point value. PCB has NEC B425; schematics show 82S141 or 6341-1
	ROM_LOAD( "bd2gt1b.f14", 0x0a00, 0x0200, CRC(52abe627) SHA1(960f19bef52fb5cf9fb74ab928a8dcb09922049d)) // PROM 1 contains squirrel and squirrel point value. PCB has NEC B425; schematics show 82S141 or 6341-1

	ROM_LOAD( "bd2gt2a.e12", 0x0c00, 0x0200, CRC(5d8ef022) SHA1(8e6ccfded85b0611670e6a7fb99c0b279f79445e)) // PROM 2 contains other targets (rabbit and turkey) and point values. PCB has NEC B425
	ROM_LOAD( "bd2gt2b.e14", 0x0e00, 0x0200, CRC(9ca95a82) SHA1(c5057cbae18d71e6a04dd4ec87b83a5690a23888)) // PROM 2 - PCB has NEC B425; schematics show 82S141 or 6341-1

	ROM_LOAD( "bd2a2.a2", 0x1000, 0x0020, CRC(1df96293) SHA1(7c8d19e34803efbe648b1db2d8c0c9a637df57d2)) // Contains PROM address codes and image speeds. Each has its own speed and address block in the image PROM. PCB has MMI 6331

	ROM_LOAD( "bd2a3.a3", 0x1020, 0x0020, CRC(63dc8c9b) SHA1(0956180a3d8877aed181513887f5a8b15cd81b93)) // Contains PROM address codes and image speeds. PCB has MMI 6331; schematics show 82S123 or 6331-1
ROM_END

} // anonymous namespace


GAME( 1976, bazooka,   0,       pse, 0, pse_state, empty_init, ROT0, "Project Support Engineering",                     "Bazooka",               MACHINE_IS_SKELETON )
GAME( 1976, knightar,  0,       pse, 0, pse_state, empty_init, ROT0, "Project Support Engineering",                     "Knights in Armor",      MACHINE_IS_SKELETON )
GAME( 1977, bazookabr, bazooka, pse, 0, pse_state, empty_init, ROT0, "Taito do Brasil",                                 "Bazooka (Brazil)",      MACHINE_IS_SKELETON )
GAME( 1977, dpatrol,   0,       pse, 0, pse_state, empty_init, ROT0, "Project Support Engineering",                     "Desert Patrol",         MACHINE_IS_SKELETON )
GAME( 1977, dpatrola,  dpatrol, pse, 0, pse_state, empty_init, ROT0, "Project Support Engineering (Telegames license)", "Desert Patrol (set 2)", MACHINE_IS_SKELETON )
GAME( 1978, gametree,  0,       pse, 0, pse_state, empty_init, ROT0, "Project Support Engineering",                     "Game Tree",             MACHINE_IS_SKELETON )
