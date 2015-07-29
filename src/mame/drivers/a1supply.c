// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

 A-1 Supply discrete hardware games

 TV 21 (197?)
 TV 21 III (197?)
 TV Poker (197?)

 These actually seem to use Intel 4040 as a CPU + a lot of discrete circuitry...
 to be checked!

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


class a1supply_state : public driver_device
{
public:
	a1supply_state(const machine_config &mconfig, device_type type, const char *tag)
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


static NETLIST_START(a1supply)
	SOLVER(Solver, 48000)
//  PARAM(Solver.FREQ, 48000)
	PARAM(Solver.ACCURACY, 1e-4) // works and is sufficient

	// schematics
	//...

//  NETDEV_ANALOG_CALLBACK(sound_cb, sound, exidyttl_state, sound_cb, "")
//  NETDEV_ANALOG_CALLBACK(video_cb, videomix, fixedfreq_device, update_vid, "fixfreq")
NETLIST_END()



void a1supply_state::machine_start()
{
}

void a1supply_state::machine_reset()
{
}


void a1supply_state::video_start()
{
}

static MACHINE_CONFIG_START( a1supply, a1supply_state )

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", NETLIST_CPU, NETLIST_CLOCK)
	MCFG_NETLIST_SETUP(a1supply)

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


ROM_START( tv21 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0800, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "aw.1c",     0x0000, 0x0800, CRC(7a1d2705) SHA1(622fbccbbf9fc80d62a5dd6d143b24852385914b) )
	ROM_LOAD( "aw.3c",     0x0000, 0x0800, CRC(f1e8ba9e) SHA1(605db3fdbaff4ba13729371ad0c4fbab3889378e) )

	ROM_LOAD( "aw.43",     0x0000, 0x0200, CRC(b23759c7) SHA1(6903b8cc9fa711b985afd52582237e66d97d3262) )
	ROM_LOAD( "aw.45",     0x0000, 0x0200, CRC(6acefe3e) SHA1(6cf751df41c26eb0375770742d3bfc318c084b11) )
	ROM_LOAD( "aw.63",     0x0000, 0x0200, CRC(a022fbe7) SHA1(625283f1cd7fbd21bcd17912cbd455404282bef8) )
	ROM_LOAD( "aw.73",     0x0000, 0x0200, CRC(34e3082d) SHA1(4daf28cfee41c2fd9711a5b5365bf322cf2fe8cd) )

	ROM_LOAD( "aw.12",     0x0000, 0x0020, CRC(490c782a) SHA1(6c5455ece13f200079924e5d3af3f6b6ee8ab3ef) )
	ROM_LOAD( "aw.22",     0x0000, 0x0020, CRC(80d03096) SHA1(39e60a7acaf019c0738e2048efbef6dd566426bc) )
	ROM_LOAD( "aw.41",     0x0000, 0x0020, CRC(8b2e1b4d) SHA1(efc374c8919496211b8587a9f6da15d13c801213) )
	ROM_LOAD( "aw.65",     0x0000, 0x0020, CRC(a54ace38) SHA1(05d8ec79566310b18d14c04a5216288e15575908) )
ROM_END


ROM_START( tv21_3 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0800, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "tv21.9",      0x0000, 0x0800, CRC(c821464c) SHA1(5334e6011ff8cd76b6215af05e697e4538921260) )
	ROM_LOAD( "tv21.42",     0x0000, 0x0200, CRC(d8595357) SHA1(44805f2b3dad8e764dda246ed19d328927679062) )
	ROM_LOAD( "tv21.47",     0x0000, 0x0200, CRC(165f590a) SHA1(d4d001ac710d28b983f8f5ce4a2e9364c2e73179) )
ROM_END


ROM_START( tvpoker )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0400, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "tvpoker.47",   0x0000, 0x0200, CRC(92bafcb3) SHA1(53598477c12e725c2aaaba1065e8a51f52e821ca) )
	ROM_LOAD( "tvpoker.67",   0x0000, 0x0200, CRC(cf7d7d7f) SHA1(d6a892cd9f1b817ac189c50c94081c948ea9e3e0) )

	ROM_LOAD( "tvpoker.26",   0x0000, 0x0100, CRC(4b301446) SHA1(5020d03678b8a193a06d658ea6088cdcc55ebf35) )
	ROM_LOAD( "tvpoker.36",   0x0000, 0x0100, CRC(40ac3596) SHA1(1c1a4b5278b9fdbe467a6abbd9d5ed4edbc7b49b) )
	ROM_LOAD( "tvpoker.38",   0x0000, 0x0100, CRC(95945f9f) SHA1(b83bcee3df787577a3b0651c554e075b28246e31) )
	ROM_LOAD( "tvpoker.39",   0x0000, 0x0100, CRC(40ac3596) SHA1(1c1a4b5278b9fdbe467a6abbd9d5ed4edbc7b49b) )
	ROM_LOAD( "tvpoker.68",   0x0000, 0x0100, CRC(d3e64864) SHA1(89bf6a2f3a8840331bf14bd4345f88c463efcc29) )

	ROM_LOAD( "tvpoker.17",   0x0000, 0x0020, CRC(8b2e1b4d) SHA1(efc374c8919496211b8587a9f6da15d13c801213) )
	ROM_LOAD( "tvpoker.20",   0x0000, 0x0020, CRC(a4a7d564) SHA1(fd625d431ca00fec129b85526839cd8e4f7d7091) )
	ROM_LOAD( "tvpoker.21",   0x0000, 0x0020, CRC(80d03096) SHA1(39e60a7acaf019c0738e2048efbef6dd566426bc) )
	ROM_LOAD( "tvpoker.22",   0x0000, 0x0020, CRC(490c782a) SHA1(6c5455ece13f200079924e5d3af3f6b6ee8ab3ef) )
	ROM_LOAD( "tvpoker.62",   0x0000, 0x0020, CRC(d72e5be0) SHA1(91daaf62dc17f8c1b837a9fa991c57b471450a1a) )
	ROM_LOAD( "tvpoker.69",   0x0000, 0x0020, CRC(d8c22608) SHA1(170e6f552fc013fec6903e45e2c7ec07e44d725c) )
	ROM_LOAD( "tvpoker.71",   0x0000, 0x0020, CRC(fea65356) SHA1(4f336dfa33a3920aef3f3eb68239c64e0fc0fed5) )
ROM_END


GAME( 197?, tv21,     0, a1supply, 0, driver_device,  0, ROT0, "A-1 Supply", "T.V. 21", MACHINE_IS_SKELETON )
GAME( 197?, tv21_3,   0, a1supply, 0, driver_device,  0, ROT0, "A-1 Supply", "T.V. 21 III", MACHINE_IS_SKELETON )
GAME( 197?, tvpoker,  0, a1supply, 0, driver_device,  0, ROT0, "A-1 Supply", "T.V. Poker", MACHINE_IS_SKELETON )
