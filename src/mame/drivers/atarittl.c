// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Scott Stone
/***************************************************************************

 Atari / Kee Games Driver - Discrete Games made in the 1970's


 Atari / Kee Games List and Data based, in part from:

 - Andy's collection of Bronzeage Atari Video Arcade PCB's"
 http://www.andysarcade.net/personal/bronzeage/index.htm

 - "Atari's Technical Manual Log"
 http://www.atarigames.com/manuals.txt

 Suspected "same games" are grouped together.  These are usually the exact same game but different cabinet/name.


 Technical Manual #s  Game Name(s)                                              Atari Part #'s                     Data      PROM/ROM Chip Numbers
 -------------------+---------------------------------------------------------+----------------------------------+---------+---------------------------------------
 TM-025               Anti-Aircraft (1975)                                      A000951                            YES       003127
 TM-058               Breakout/Breakout Cocktail (1976)                         A004533                            NO
 TM-015               Cocktail Pong/Coup Franc (1974)                           ???????                            NO
 TM-048               Crash 'N Score/Stock Car (1975)                           A004256                            YES       003186(x2), 003187(x2), 004248, 004247
 TM-030               Crossfire (1975)                                          ???????                            NO?
 TM-003,005,011,020   Gran Trak 10/Trak 10/Formula K (1974)                     A000872,A000872 K3RT               YES       74186 Racetrack Prom (K5)
 TM-004,021           Gran Trak 20/Trak 20/Twin Racer (1974)                    A001791(RT20),A001793(A20-K4DRTA)  YES       74186 Racetrack prom (K5)
 TM-006,035           Goal 4/World Cup/Coupe De Monde (1975)                    A000823                            NO
 TM-016               Gotcha/Gotcha Color? (1973)                               A000816                            NO
 TM-028               Highway/Hi-Way (1975)                                     A003211                            NO
 TM-055               Indy 4 (1976)                                             A003000,A006268,A006270            YES       003186, 003187, 005502-01, 05503-01
 TM-026               Indy 800 (1975)                                           A003000,A003170,A003182            YES       003186-003189 (4)
                                                                                A003184,A003191,A003198,A003199
 TM-027               Jet Fighter/Jet Fighter Cocktail/Launch Aircraft (1975)   A004254,A004255                    YES       004250-004252, 004253-01 to 03 (3)
 TM-077               LeMans (1976)                                             A005844,A005845                    YES       005837-01, 005838-01, 005839-01
 TM-040               Outlaw (1976)                                             A003213                            YES       003323 - ROM (8205 @ J4)
 TM-007               Pin Pong (1974)                                           A001660                            NO
 TM-013               Pong/Super Pong (1972)                                    A001433,A000423                    NO
 TM-014               Pong Doubles/Coupe Davis (1974)                           A000785                            NO
 TM-018               Pursuit (1975)                                            K8P-B 90128                        NO
 TM-012,022,034       Quadrapong/Elimination (1974)                             A000845                            NO
 TM-009               Qwak!/Quack (1974)                                        A000937,A000953                    YES       72074/37-2530N (K9)
 TM-001,032           Rebound/Volleyball (1974)                                 A000517,A000846                    NO
 TM-047               Shark Jaws (1975)                                         A003806                            YES       004182, 004183
 TM-008               Space Race (1974)                                         A000803                            NO
 TM-023               Spike  (1974)                                             SPIKE-(A or B)                     NO
 TM-046               Steeplechase/Astroturf (1975)                             A003750                            YES       003774 ROM Bugle (C8), 003773-01 "A" Horse (C4), 003773-02 "B" Horse (D4)
 TM-057               Stunt Cycle (1976)                                        A004128                            YES       004275 ROM Motorcycle/Bus (1F), 004811 ROM Score Translator (D7)
 TM-010,036,049       Tank/Tank Cocktail/Tank II (1974/1975)                    A003111 (K5T-F 90124)              YES       90-2006
 TM-002               Touch Me (1974)                                           ???????                            NO

 - Not Known to be released or produced, but at least announced.

 TM-0??               Arcade Driver/Driver First Person (Not Produced/Released) (197?)
 TM-0??               Dodgeball/Dodgem (Not Produced/Released) (1975)
 TM-024               Qwakers (Not Produced/Released) (1974?)
 TM-017               World Cup Football (Not Produced/Released) (1974)


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



class atarikee_state : public driver_device
{
public:
	atarikee_state(const machine_config &mconfig, device_type type, const char *tag)
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



static NETLIST_START(atarikee)
	SOLVER(Solver, 48000)
//  PARAM(Solver.FREQ, 48000)
	PARAM(Solver.ACCURACY, 1e-4) // works and is sufficient

	// schematics
	//...

//  NETDEV_ANALOG_CALLBACK(sound_cb, sound, atarikee_state, sound_cb, "")
//  NETDEV_ANALOG_CALLBACK(video_cb, videomix, fixedfreq_device, update_vid, "fixfreq")
NETLIST_END()


void atarikee_state::machine_start()
{
}

void atarikee_state::machine_reset()
{
}

void atarikee_state::video_start()
{
}



static MACHINE_CONFIG_START( atarikee, atarikee_state )

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", NETLIST_CPU, NETLIST_CLOCK)
	MCFG_NETLIST_SETUP(atarikee)

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


ROM_START( antiairc )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x20, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "003127.k1",     0x0000, 0x0020, CRC(9de772d5) SHA1(2855ba908d8e14a5aca43d4e0594d19f23fe9aae) ) // Anti-Aircraft Target
ROM_END


ROM_START( crashnsc )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0400, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "003186.f6",     0x0000, 0x0200, CRC(b3443354) SHA1(f43b82fd5d02dad2f597f890f5845701e73476a5) ) // Car Video #1
	ROM_LOAD( "003186.p6",     0x0200, 0x0200, CRC(b3443354) SHA1(f43b82fd5d02dad2f597f890f5845701e73476a5) ) // Car Video #2

	ROM_REGION( 0x0040, "motion", ROMREGION_ERASE00 )
	ROM_LOAD( "003187.f7",     0x0000, 0x0020, CRC(01dca5b9) SHA1(0e3fbefc5df993b5a6a724aee258653897954255) ) // Car Motion #1
	ROM_LOAD( "003187.p7",     0x0020, 0x0020, CRC(01dca5b9) SHA1(0e3fbefc5df993b5a6a724aee258653897954255) ) // Car Motion #2

	ROM_REGION( 0x0200, "location", ROMREGION_ERASE00 )
	ROM_LOAD( "004248.d2",     0x0000, 0x0200, CRC(683b203b) SHA1(97202da5dd4a6cb66714d8e58ecee5c6efa65c1c) ) // Car Location Code

	ROM_REGION( 0x0200, "shape", ROMREGION_ERASE00 )
	ROM_LOAD( "004247.e2",     0x0000, 0x0200, CRC(478afac2) SHA1(fb15af0d2fc9d9ed0e92a3e7610c22dadf91d012) ) // Car Shape Code
ROM_END


ROM_START( indy4 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0200, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "003186.p6",     0x0000, 0x0200, CRC(b3443354) SHA1(f43b82fd5d02dad2f597f890f5845701e73476a5) ) // Car Video

	ROM_REGION( 0x0020, "motion", ROMREGION_ERASE00 )
	ROM_LOAD( "003187.f7",     0x0000, 0x0020, CRC(01dca5b9) SHA1(0e3fbefc5df993b5a6a724aee258653897954255) ) // Car Motion

	ROM_REGION( 0x0020, "checkpoint", ROMREGION_ERASE00 )
	ROM_LOAD( "005502.e5",     0x0000, 0x0020, CRC(e30ea877) SHA1(86f1f2c2e6e8472f7019f17bac723cb36faf098a) ) // Check Points

	ROM_REGION( 0x0200, "racetrack", ROMREGION_ERASE00 )
	ROM_LOAD( "005503.f4",     0x0000, 0x0200, CRC(1aafbe72) SHA1(c59829eccfe5a6014acad9682c401ca3f32fdfc9) ) // Race Track
ROM_END


ROM_START( indy800 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0200, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "003186.p6",     0x0000, 0x0200, CRC(b3443354) SHA1(f43b82fd5d02dad2f597f890f5845701e73476a5) ) // Car Video

	ROM_REGION( 0x0020, "motion", ROMREGION_ERASE00 )
	ROM_LOAD( "003187.f7",     0x0000, 0x0020, CRC(01dca5b9) SHA1(0e3fbefc5df993b5a6a724aee258653897954255) ) // Car Motion

	ROM_REGION( 0x0020, "checkpoint", ROMREGION_ERASE00 )
	ROM_LOAD( "003188.e5",     0x0000, 0x0020, NO_DUMP ) // Check Points - Might be same as indy4?

	ROM_REGION( 0x0200, "racetrack", ROMREGION_ERASE00 )
	ROM_LOAD( "003189.f4",     0x0000, 0x0200, NO_DUMP ) // Race Track - Might be same as indy4?
ROM_END


ROM_START( jetfight )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0040, "shell", ROMREGION_ERASE00 )
	ROM_LOAD( "004250.m1",     0x0000, 0x0020, CRC(bee62d20) SHA1(2ea5fd7b087004c37901d2a56da2d6f6dcce9e29) ) // Shell Rom
	ROM_LOAD( "004250.j1",     0x0020, 0x0020, CRC(bee62d20) SHA1(2ea5fd7b087004c37901d2a56da2d6f6dcce9e29) ) // Shell Rom

	ROM_REGION( 0x0020, "singleplayer", ROMREGION_ERASE00 )
	ROM_LOAD( "004251.r5",     0x0000, 0x0020, CRC(bd95f87e) SHA1(4bd863104f1a7260b95f3fb2c13f40b7337d3dd9) ) // Single Player Rom

	ROM_REGION( 0x0100, "score", ROMREGION_ERASE00 )
	ROM_LOAD( "004252.a4",     0x0000, 0x0100, CRC(08a0b011) SHA1(71998728604a152006550869afe60d405643ccf1) ) // Score Rom

	ROM_REGION( 0x0400, "gfx", ROMREGION_ERASE00 )
	/* Note:  Use 004253-01 and 004253-02 or use 004253-03 ONLY, not both together.  Presumably, -03 = data from -02 and -01 */
	ROM_LOAD( "004253-02.j5",  0x0000, 0x0200, CRC(c58ee65d) SHA1(785f842897a2ce92ce2f009e9b6d8e96950deb1f) ) // Picture & S.C. Rom A
	ROM_LOAD( "004253-01.k5",  0x0200, 0x0200, CRC(0d5648a9) SHA1(7a79ca587376678d9735f025d59088e6686fd783) ) // Picture & S.C. Rom B
//  ROM_LOAD( "004253-03.f5",  0x0000, 0x0400, NO_DUMP ) // Picture & S.C. Rom C
ROM_END


ROM_START( jetfighta )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0040, "shell", ROMREGION_ERASE00 )
	ROM_LOAD( "004250.m1",     0x0000, 0x0020, CRC(bee62d20) SHA1(2ea5fd7b087004c37901d2a56da2d6f6dcce9e29) ) // Shell Rom
	ROM_LOAD( "004250.j1",     0x0020, 0x0020, CRC(bee62d20) SHA1(2ea5fd7b087004c37901d2a56da2d6f6dcce9e29) ) // Shell Rom

	ROM_REGION( 0x0020, "singleplayer", ROMREGION_ERASE00 )
	ROM_LOAD( "004251.r5",     0x0000, 0x0020, CRC(bd95f87e) SHA1(4bd863104f1a7260b95f3fb2c13f40b7337d3dd9) ) // Single Player Rom

	ROM_REGION( 0x0200, "score", ROMREGION_ERASE00 )
	ROM_LOAD( "jet.a4",        0x0000, 0x0200, CRC(9e267e44) SHA1(b1c74ab275e30ed41c60e8490eaaf5211ec14ec5) ) // Score Rom

	ROM_REGION( 0x0800, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "jet.j5",        0x0000, 0x0400, CRC(853d61b3) SHA1(c5e1b09153b813b7b4042246e5634cc83de9654c) ) // Picture & S.C. Rom A
	ROM_LOAD( "jet.k5",        0x0400, 0x0400, CRC(a3fada62) SHA1(2efed600683e35ffa10acc5a301e736989c9f236) ) // Picture & S.C. Rom B
ROM_END


ROM_START( outlaw )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0200, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "0003323.j4",  0x0000, 0x0200, CRC(3166dad9) SHA1(4fca88b4256d8fb3e0deca54a15ffaafb830831e) ) // Rom (8205)
ROM_END


ROM_START( sharkjaw )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0200, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "004182.da1",  0x0000, 0x0100, CRC(05242912) SHA1(d3925cde795f04ac04151165bbbff74b15dce5ca) ) // Shark & Fish P-Rom
	ROM_LOAD( "004183.db1",  0x0100, 0x0100, CRC(b161b889) SHA1(009c6fc93174df15fb6a7993a73cfda56c8edfa2) )// Diver P-Rom
ROM_END


ROM_START( steeplec )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0200, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "003773-a.4c",  0x0000, 0x0100, CRC(5ddc49b6) SHA1(58eba996703cbb7b3f66ff97357e191c9a3ab340) ) // Bugle
	ROM_LOAD( "003773-b.4d",  0x0100, 0x0100, CRC(e6994cde) SHA1(504f92dba0c8640d55c7412697868582043f3817) ) // Graphics
ROM_END


ROM_START( stuntcyc )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0200, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "004275.f1",  0x0000, 0x0200, CRC(4ed5a99d) SHA1(1e5f439bce72e78dfff76fd8f61187c6ef484a64) ) // Motorcycle & Bus

	ROM_REGION( 0x0020, "score", ROMREGION_ERASE00 )
	ROM_LOAD( "004811.d7",  0x0000, 0x0020, CRC(31a09efb) SHA1(fd5d538c9ec1234acf7c74ca0704113d220abbf6) ) // Score Translator
ROM_END


ROM_START( tank )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	/* The "custom" 24-pin ROM used in Atari/Kee Games "Tank" is known as a MOSTEK MK28000P. */
	ROM_REGION( 0x0801, "gfx", ROMREGION_ERASE00 ) // 2049 Byte Size?
	ROM_LOAD( "90-2006.k10" ,0x0000, 0x0801, CRC(c25f6014) SHA1(7bd3fca5f64c928a645ca27c643b736667cef216) )
ROM_END


/*  // NO DUMPED ROMS

// Astroturf (1975)
ROM_START( astrotrf )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

    ROM_REGION( 0x0400, "gfx", ROMREGION_ERASE00 ) // Region Size unknown, dump size unknown
    ROM_LOAD( "003774.c8",     0x0000, 0x0100, NO_DUMP ) // Bugle
    ROM_LOAD( "003773-02.c4",  0x0100, 0x0100, NO_DUMP ) // Graphics (Astroturf - Rev.A)
ROM_END

// Gran Trak 10 / Trak 10 / Formula K / Race Circuit (1974)
ROM_START( gtrak10 )  // Unknown size, assumed 2K Bytes
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

    ROM_REGION( 0x0800, "racetrack", ROMREGION_ERASE00 )
    ROM_LOAD( "74168.k5",     0x0000, 0x0800, NO_DUMP) // Racetrack
ROM_END

// Gran Trak 20 / Trak 20 / Twin Racer (1974)
ROM_START( gtrak20 )  // Unknown size, assumed 2K Bytes
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

    ROM_REGION( 0x0800, "racetrack", ROMREGION_ERASE00 )
    ROM_LOAD( "74168.k5",     0x0000, 0x0800, NO_DUMP) // Racetrack
ROM_END

// LeMans (1976)
ROM_START( lemans )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

    ROM_REGION( 0x0400, "gfx", ROMREGION_ERASE00 ) // Region Size unknown, dump size unknown
    ROM_LOAD( "005837-01.n5",  0x0000, 0x0100, NO_DUMP ) // Rom 1
    ROM_LOAD( "005838-01.n4",  0x0100, 0x0100, NO_DUMP ) // Rom 2
    ROM_LOAD( "005839-01.n6",  0x0200, 0x0100, NO_DUMP ) // Rom 3
ROM_END

// Qwak! / Quack (1974)
ROM_START( qwak )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

    ROM_REGION( 0x0200, "gfx", ROMREGION_ERASE00 ) // Region Size unknown, dump size unknown
    ROM_LOAD( "37-2530n.k9",  0x0000, 0x0200, NO_DUMP ) // Custom Rom (2530 N)
ROM_END

*/


/*  // 100% TTL - NO ROMS

// Crossfire (1975)
// Unclear if this is 100% TTL or if it uses a ROM:
// IC description in manual says a rom is used (74186 ROM)
// but the parts list in the same manual mentions no IC 74186!
// Simulated in DICE without ROMs from schematics, so unlikely
// it uses any, and is in fact 100% TTL..
ROM_START ( crossfir )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( goal4 )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( gotcha )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( gotchac )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( highway )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( pinpong )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( pongdbl )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( pursuit )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( quadpong )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( rebound )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( spacrace )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( touchme )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

*/



GAME(1975,  antiairc,  0,         atarikee,   0,  driver_device, 0,  ROT0,  "Atari",      "Anti-Aircraft [TTL]",    MACHINE_IS_SKELETON)
GAME(1975,  crashnsc,  0,         atarikee,   0,  driver_device, 0,  ROT0,  "Atari",      "Crash 'n Score/Stock Car [TTL]",   MACHINE_IS_SKELETON)
GAME(1976,  indy4,     0,         atarikee,   0,  driver_device, 0,  ROT0,  "Atari",      "Indy 4 [TTL]",           MACHINE_IS_SKELETON)
GAME(1975,  indy800,   0,         atarikee,   0,  driver_device, 0,  ROT90, "Atari/Kee",  "Indy 800 [TTL]",         MACHINE_IS_SKELETON)
GAME(1975,  jetfight,  0,         atarikee,   0,  driver_device, 0,  ROT0,  "Atari",      "Jet Fighter/Jet Fighter Cocktail/Launch Aircraft (set 1) [TTL]",      MACHINE_IS_SKELETON)
GAME(1975,  jetfighta, jetfight,  atarikee,   0,  driver_device, 0,  ROT0,  "Atari",      "Jet Fighter/Jet Fighter Cocktail/Launch Aircraft (set 2) [TTL]",      MACHINE_IS_SKELETON)
GAME(1976,  outlaw,    0,         atarikee,   0,  driver_device, 0,  ROT0,  "Atari",      "Outlaw [TTL]",           MACHINE_IS_SKELETON)
GAME(1975,  sharkjaw,  0,         atarikee,   0,  driver_device, 0,  ROT0,  "Atari/Horror Games",  "Shark JAWS [TTL]",MACHINE_IS_SKELETON)
GAME(1975,  steeplec,  0,         atarikee,   0,  driver_device, 0,  ROT0,  "Atari",      "Steeplechase [TTL]",     MACHINE_IS_SKELETON)
GAME(1976,  stuntcyc,  0,         atarikee,   0,  driver_device, 0,  ROT0,  "Atari",      "Stunt Cycle [TTL]",      MACHINE_IS_SKELETON)
GAME(1974,  tank,      0,         atarikee,   0,  driver_device, 0,  ROT0,  "Atari/Kee",  "Tank/Tank Cocktail/Tank II [TTL]",     MACHINE_IS_SKELETON)

// MISSING ROM DUMPS
//GAME(1975,  astrotrf,  steeplec,  atarikee,   0,  driver_device, 0,  ROT0,  "Atari",      "Astroturf [TTL]",        MACHINE_IS_SKELETON)
//GAME(1974,  gtrak10,   0,         atarikee,   0,  driver_device, 0,  ROT0,  "Atari/Kee",  "Gran Trak 10/Trak 10/Formula K [TTL]",     MACHINE_IS_SKELETON) //?
//GAME(1974,  gtrak20,   0,         atarikee,   0,  driver_device, 0,  ROT0,  "Atari/Kee",  "Gran Trak 20/Trak 20/Twin Racer [TTL]",     MACHINE_IS_SKELETON) //?
//GAME(1976,  lemans,    0,         atarikee,   0,  driver_device, 0,  ROT0,  "Atari",      "LeMans [TTL]",           MACHINE_IS_SKELETON)
//GAME(1974,  quack,     0,         atarikee,   0,  driver_device, 0,  ROT0,  "Atari",      "Qwak!/Quack [TTL]",      MACHINE_IS_SKELETON)

// 100% TTL
//GAME(1974,  coupedem,  0,         atarikee,   0,  driver_device, 0,  ROT0,  "Atari",      "Coupe De Monde [TTL]",   MACHINE_IS_SKELETON)
//GAME(1975,  crossfir,  0,         atarikee,   0,  driver_device, 0,  ROT0,  "Atari",      "Crossfire [TTL]",        MACHINE_IS_SKELETON)
//GAME(1975,  goal4,     0,         atarikee,   0,  driver_device, 0,  ROT0,  "Atari",      "Goal 4/World Cup/Coupe De Monde [TTL]",     MACHINE_IS_SKELETON)
//GAME(1973,  gotchaat,  0,         atarikee,   0,  driver_device, 0,  ROT0,  "Atari",      "Gotcha [TTL]",           MACHINE_IS_SKELETON) //?
//GAME(1973,  gotchaatc, 0,         atarikee,   0,  driver_device, 0,  ROT0,  "Atari",      "Gotcha Color [TTL]",     MACHINE_IS_SKELETON) //?
//GAME(1975,  highway,   0,         atarikee,   0,  driver_device, 0,  ROT0,  "Atari",      "Highway/Hiway [TTL]",    MACHINE_IS_SKELETON)
//GAME(1974,  pinpong,   0,         atarikee,   0,  driver_device, 0,  ROT0,  "Atari",      "Pin Pong [TTL]",         MACHINE_IS_SKELETON)
//GAME(1975,  pursuit,   0,         atarikee,   0,  driver_device, 0,  ROT0,  "Atari",      "Pursuit [TTL]",          MACHINE_IS_SKELETON)
//GAME(1973,  quadpong,  0,         atarikee,   0,  driver_device, 0,  ROT0,  "Atari/Kee",  "Quadrapong/Elimination [TTL]",     MACHINE_IS_SKELETON)
//GAME(1974,  rebound,   0,         atarikee,   0,  driver_device, 0,  ROT0,  "Atari/Kee",  "Rebound/Spike/Volleyball [TTL]",     MACHINE_IS_SKELETON)
//GAME(1974,  spacrace,  0,         atarikee,   0,  driver_device, 0,  ROT0,  "Atari",      "Space Race [TTL]",       MACHINE_IS_SKELETON)
//GAME(1974,  touchme,   0,         atarikee,   0,  driver_device, 0,  ROT0,  "Atari",      "Touch Me [TTL]",         MACHINE_IS_SKELETON) //?
