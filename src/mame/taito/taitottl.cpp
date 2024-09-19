// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Scott Stone
/***************************************************************************

 Taito Discrete Hardware Games


 Game Name(s)                                                        Part #'s       Data      PROM/ROM Chip Numbers
 ------------------------------------------------------------------+--------------+---------+---------------------------------
 Anti-Aircraft (1975) (clone of Atari's Anti-Aircraft)                              YES
 Astro Race (11/1973) (clone of Atari's Space Race)                                 UNKNOWN
 Attack (09/1976)                                                                   UNKNOWN
 Attack UFO (08/1974)                                                               UNKNOWN
 Avenger (??/1976) (clone of Electra's Avenger)                                     YES
 Ball Park (??/1974) (clone of Midway's Ball Park)                                  YES
 Basketball (04/1974) (clone of Midway's TV Basketball)                             NO
 Bombs Away (??/1977) (clone of Meadows' Bombs Away)                                YES
 Cisco/Fisco 400 (04/1977)                                                          YES        24 Chips of various sizes
 Clean Sweep (??/1976) (clone of Ramtek's Clean Sweep)                              YES
 Clean Sweep II (??/1976) (clone of Ramtek's Clean Sweep)                           YES
 Crashing Race (06/1976) (clone of Atari's Crash'n Score?)                          UNKNOWN
 Cross Fire (08/1977) (clone of PSE's Bazooka)                                      YES
 Davis Cup (12/1973) (clone of Atari's Coupe Davis / Pong Doubles?)                 NO
 Dead Heat (??/1975)                                                                UNKNOWN
 Elepong (07/1973)                                                                  UNKNOWN
 Flying Fortress (??/1977) (clone of Electra's Flying Fortress?)                    YES
 Flying Fortress II (06/1977)                                                       UNKNOWN
 Gunman (10/1977)                                                                   YES        8 x 32bytes (or 11? )
 Interceptor (03/1976)                                                              UNKNOWN
 Missile-X (??/1977)                                                                YES        10 - (5 x 512bytes, 5x32bytes)
 Pro Hockey (11/1973)                                                               UNKNOWN
 Road Champion (04/1977) (clone of Williams' Road Champion?)                        YES
 Road Champion S (06/1977)                                                          UNKNOWN
 Safari (??/1977) (clone of Gremlin's Safari?)                                      UNKNOWN
 Soccer (11/1973)                                                                   UNKNOWN
 Soccer Deluxe (??/1977)                                                            UNKNOWN
 Speed Race (11/1974)                                                               UNKNOWN
 Speed Race CL-5 (10/1978)                                                          UNKNOWN
 Speed Race Deluxe (08/1975)                                                        UNKNOWN
 Speed Race Twin (04/1976)                                                          UNKNOWN
 Speed Race GP-5 (03/1980)                                                          UNKNOWN
 Super Block (02/1978)                                                              UNKNOWN
 Super Speed Race (12/1977)                                                         UNKNOWN
 Super Speed Race V (07/1978)                                                       UNKNOWN
 Super Speed Race GP-V (??/1980)                                                    UNKNOWN
 Table Football (??/1977) (clone of Exidy's Table Football?)                        UNKNOWN
 Tahitian (??/1975)                                                                 UNKNOWN
 Tennis (??/1977)                                                                   UNKNOWN
 Top Bowler (06/1978)                                                               UNKNOWN
 T. T. Ball Park (??/1974) (clone of Midway's Ball Park)                            YES
 T. T. Block (08/1977)                                                              YES        1 x 2048bytes
 T. T. Block C (05/1978)                                                            UNKNOWN
 T. T. Block CU (08/1978)                                                           UNKNOWN
 T. T. Speed Race (06/1978)                                                         UNKNOWN
 T. T. Speed Race CL (10/1978)                                                      YES
 T. T. Top Bowler (??/1978)                                                         UNKNOWN
 T. T. Zun Zun Block (??/1979)                                                      YES        3 - (2 x 512bytes, 1 x 32bytes)
 Wall Block (08/1978)                                                               UNKNOWN
 Wall Break (01/1977)                                                               UNKNOWN
 Western Gun (09/1975)                                                              UNKNOWN
 Zun Zun Block (04/1979)                                                            YES        3 - (2 x 512bytes, 1 x 32bytes)

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


class taitottl_state : public driver_device
{
public:
	taitottl_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_video(*this, "fixfreq")
	{
	}

	void taitottl(machine_config &config);

private:
	// devices
	required_device<netlist_mame_device> m_maincpu;
	required_device<fixedfreq_device> m_video;

	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void video_start() override ATTR_COLD;
};


static NETLIST_START(taitottl)
{
	SOLVER(Solver, 48000)
//  PARAM(Solver.FREQ, 48000)
	PARAM(Solver.ACCURACY, 1e-4) // works and is sufficient

	// schematics
	//...

	//  NETDEV_ANALOG_CALLBACK(sound_cb, sound, exidyttl_state, sound_cb, "")
	//  NETDEV_ANALOG_CALLBACK(video_cb, videomix, fixedfreq_device, update_vid, "fixfreq")
}



void taitottl_state::machine_start()
{
}

void taitottl_state::machine_reset()
{
}


void taitottl_state::video_start()
{
}

void taitottl_state::taitottl(machine_config &config)
{
	/* basic machine hardware */
	NETLIST_CPU(config, m_maincpu, netlist::config::DEFAULT_CLOCK()).set_source(netlist_taitottl);

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

/*

 Fisco 400

 label  loc. Part #
 ==========================================================
 TOP PCB : [FS070001 FSN00001A]
 CR11   L9  MMI 6301
 FS01   J10 Texas Instruments 74S288 (read as AMD 74S288)
 FS02   D10 MMI 6301
 FS03   E10 MMI 6301
 FS04   A7  Intersil IM5624
 FS05   B7  Intersil IM5624
 FS06   C7  Intersil IM5624
 FS07   D7  Intersil IM5624
 FS08   E7  Intersil IM5624
 FS09   A6  Intersil IM5624
 FS10   B6  Intersil IM5624
 FS11   C6  Intersil IM5624
 FS12   D6  Intersil IM5624

 Bottom PCB : [FS070002A FSN00002A]
 FS13   H8  Texas Inst. 74S287 (read as national 74S287)
 FS14   J8  Texas Inst. 74S287 (read as national 74S287)
 FS15   K8  Texas Inst. 74S287 (read as national 74S287)
 FS16   L8  Texas Inst. 74S287 (read as national 74S287)
 FS17   M8  Texas Inst. 74S287 (read as national 74S287)
 FS18   A8  MMI 6306 (read as Harris 7621)
 FS19   B8  MMI 6306 (read as Harris 7621)
 FS20   C8  MMI 6306 (read as Harris 7621)
 FS21   D8  MMI 6306 (read as Harris 7621)
 FS22   E8  MMI 6306 (read as Harris 7621)
 FS23   F8  MMI 6306 (read as Harris 7621)

*/

ROM_START( fisco400 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0800, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "cr11.l9",      0x0000, 0x0100, CRC(b3250118) SHA1(ab8bfa11b112b03b8ff00b09b3e6d9b221051ad2) ) // Score
	ROM_LOAD( "fs01.j10",     0x0000, 0x0020, CRC(0a11a36a) SHA1(4064cc76f8565514d692ae9e00a9c7533f0c067c) ) // Game Control
	ROM_LOAD( "fs02.d10",     0x0000, 0x0100, CRC(e95f6e38) SHA1(f78cebe214d712bfe08a93fedbf8a394036dc434) ) // Checkered Flag
	ROM_LOAD( "fs03.e10",     0x0000, 0x0100, CRC(1e7f6bec) SHA1(ed897c599542194be654650c1627879ba4db4d16) ) // Insert Coin Graphic
	ROM_LOAD( "fs04.a7",      0x0000, 0x0200, CRC(c9be1200) SHA1(6471e91a09d7644777bb594e3cf4357afb389a5d) ) // Track Graphics
	ROM_LOAD( "fs05.b7",      0x0000, 0x0200, CRC(2d9786da) SHA1(869df761c9ad9f90f56f558880ec405dfc56b6d9) ) // Track Graphics
	ROM_LOAD( "fs06.c7",      0x0000, 0x0200, CRC(cbf412fb) SHA1(3fb00127ed19758db35a4ab148ba531c6f2e520f) ) // Track Graphics
	ROM_LOAD( "fs07.d7",      0x0000, 0x0200, CRC(f6014074) SHA1(522b98f62a3de2980a615723d88542cbb23a7fa3) ) // Track Graphics
	ROM_LOAD( "fs08.e7",      0x0000, 0x0200, CRC(9b0d955b) SHA1(9d676bee180c57cfce9c8a6929a16d00eca2a468) ) // Track Graphics
	ROM_LOAD( "fs09.a6",      0x0000, 0x0200, CRC(f278ac29) SHA1(6287ee2f922c01f2f027103f04684ca952ece479) ) // Track Graphics
	ROM_LOAD( "fs10.b6",      0x0000, 0x0200, CRC(7fe466d2) SHA1(ad07f2fb7fe0ad35dbd41f53dc53415894b8de2f) ) // Track Graphics
	ROM_LOAD( "fs11.c6",      0x0000, 0x0200, CRC(13747458) SHA1(65611ea3011b65869bf5482f343476b6a50404a5) ) // Track Graphics
	ROM_LOAD( "fs12.d6",      0x0000, 0x0200, CRC(91a8b55b) SHA1(7d01ef51ec75af3d05f1a5a903d4cc112de433ba) ) // Track Graphics
	ROM_LOAD( "fs13.8h",      0x0000, 0x0100, CRC(2717181d) SHA1(2bee55d004fbbb991dfc95aba1d16be6fd65e7b3) ) // Speed Data Storage
	ROM_LOAD( "fs14.8j",      0x0000, 0x0100, CRC(fb2dc57d) SHA1(cb08fce0988acc920158433e5b54666e6edb78ea) ) // Speed Data Storage
	ROM_LOAD( "fs15.8k",      0x0000, 0x0100, CRC(76414291) SHA1(d90d0eb639887ed58dfee9256b808d59a02925ec) ) // Speed Data Storage
	ROM_LOAD( "fs16.8l",      0x0000, 0x0100, CRC(32cf6200) SHA1(648d6a1bfa561641fa5dd368a7a8c7c73bd339b4) ) // Speed Data Storage
	ROM_LOAD( "fs17.8m",      0x0000, 0x0100, CRC(aff8ac17) SHA1(4a35dcab142238b7247ea14a8b1c0eeeded792fc) ) // Speed Data Storage
	ROM_LOAD( "fs18.8a",      0x0000, 0x0200, CRC(4e354f7d) SHA1(8e2270c3eddefdd62f807d37a581b6fb1dc44e5e) ) // Car Form
	ROM_LOAD( "fs19.8b",      0x0000, 0x0200, CRC(9813b6ea) SHA1(3a2dca63bbae3f894061f1a4726686149e12b08e) ) // Car Form
	ROM_LOAD( "fs20.8c",      0x0000, 0x0200, CRC(77acbc5c) SHA1(bc371002d8fd4f38fc15a26cb82e0405308c8cf9) ) // Car Form
	ROM_LOAD( "fs21.8d",      0x0000, 0x0200, CRC(b24e6610) SHA1(d5e87aa63b8d7d0f71ad85b8670cba786bef29c7) ) // Car Form
	ROM_LOAD( "fs22.8e",      0x0000, 0x0200, CRC(b71eacb0) SHA1(c26b260f5cdbf72a2b069f17ca73757dfe92218f) ) // Car Form
	ROM_LOAD( "fs23.8f",      0x0000, 0x0200, CRC(48171b93) SHA1(b6a87710a8132cf179e65411c45fbcb7a41eb11e) ) // Car Form
ROM_END

/*

 Gunman

 label  loc. Part #
 ==========================================================
 CR11   N5  74s287      yes, it says CRxx not GNxx
 GN10   H5  5623
 GN09   N6  soldered in
 GN08   J7  IM5200CJG
 GN07   L9  soldered in
 GN06   H6  7621
 GN05   G9  74s287
 GN04   F9  7643
 GN03   D9  7621
 GN02   C9  74s287
 GN01   B9  7643

*/

ROM_START( gunman )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0800, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "cr11.n5",      0x0000, 0x0200, CRC(8e2ec052) SHA1(b849080c9ed325a478739fbdf315c8a6c2bb7bd7) )
	ROM_LOAD( "gn10.h5",      0x0000, 0x0200, CRC(a976c4c7) SHA1(f2fb56ccec99ceb19d607bee3b4cca31d6db58fd) )
	ROM_LOAD( "gn06.h6",      0x0000, 0x0400, CRC(17376de6) SHA1(4ef660da9d37024a13925af12a7e102a49c67ff6) )
	ROM_LOAD( "gn05.g9",      0x0000, 0x0200, CRC(f3d7f6cb) SHA1(7abf445cc814dc242ebf2393e9c1335ba4aedb90) )
	ROM_LOAD( "gn04.f9",      0x0000, 0x0800, CRC(dec942db) SHA1(165d8594e07837fd4a87895f0a350d5ed8112ead) )
	ROM_LOAD( "gn03.d9",      0x0000, 0x0400, CRC(b0e6e473) SHA1(9665608b7ec1dfb39e4e1df74adbfc773e9a6aa7) )
	ROM_LOAD( "gn02.c9",      0x0000, 0x0200, CRC(897ae7b8) SHA1(2e91bc8336f8620faedd5329ce7d9734f19a2cfa) )
	ROM_LOAD( "gn01.b9",      0x0000, 0x0800, CRC(69474a3a) SHA1(7396f1be991a0e6207eaa79e0206a5286a8d615d) )
ROM_END

/*

 Missile-X
 14.314 mhz XTAL

 label  loc. Part #     Use?
 ==========================================================
 CR11   L4  74s287      numerics (yes, it says CRxx not GNxx)
 MS09   F6  74s287      ground explosion
 MS08   C2  74s287      400pt jeep graphics
 MS07   N8  74s287      player missile
 MS06   M8  74s287      missile animated graphics
 MS05   11F IM5610      200pt tank R->L graphic
 MS05   11E IM5610      200pt tank L->R graphic
 MS04   N7  IM5610      player missile trajectory pattern
 MS03   F3  IM5610      anti-missile ack-ack graphics
 MS02   L12 IM5610      missile left/right position
 MS01   D8  IM5610      100pt tanks graphics

*/

ROM_START( missilex )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0800, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "cr11.4l",      0x0000, 0x0200, CRC(3d10a407) SHA1(1a85581c34d7e6766eaebbcc9895ed0ab2f94387) )
	ROM_LOAD( "ms09.6f",      0x0000, 0x0200, CRC(80787642) SHA1(9de2419f0ba16f9c2b06e417c1ebba441fdae053) )
	ROM_LOAD( "ms08.2c",      0x0000, 0x0200, CRC(dd785590) SHA1(7ba6b6f6091595852d6feaef5a029b2aca684440) )
	ROM_LOAD( "ms07.8n",      0x0000, 0x0200, CRC(e278d03a) SHA1(c40975e5807936fed40cda4a6881f6aef0e7f350) )
	ROM_LOAD( "ms06.8m",      0x0000, 0x0200, CRC(fe6c9192) SHA1(d110e010cf685ee18479ca7f890fa9da2fa71603) )
	ROM_LOAD( "ms05.11f",     0x0000, 0x0020, CRC(845fe0cc) SHA1(ce8db615c1f7be242fc2ee25c1ef75e8608a771a) )
	ROM_LOAD( "ms05.11e",     0x0000, 0x0020, CRC(845fe0cc) SHA1(ce8db615c1f7be242fc2ee25c1ef75e8608a771a) )
	ROM_LOAD( "ms04.7n",      0x0000, 0x0020, CRC(34d0bee4) SHA1(41848def6aeb128ec985c158f3ed01c5b20bdcf6) )
	ROM_LOAD( "ms03.3f",      0x0000, 0x0020, CRC(d139f5fa) SHA1(29c05143d05553c2cb2831f9624f307f59436850) )
	ROM_LOAD( "ms02.12l",     0x0000, 0x0020, CRC(157b7e68) SHA1(d1a98267af1562e6126faaf0850906224f8a608d) )
	ROM_LOAD( "ms01.8d",      0x0000, 0x0020, CRC(e89e76c3) SHA1(1149b5d1f93baa8aecd54a618083cc13b63a894d) )
ROM_END

ROM_START( ttblock )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0800, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "tl01.4a",      0x0000, 0x0800, CRC(65b730f7) SHA1(f82931c9a128021c97d1d41b5eac05df55dd5994) ) // MMI 6353
ROM_END

/*

TT Speed Race Color
14.314 mhz XTAL

 label  loc. Part #        Use?
 ==========================================================
TOP PCB: [CEN00008 CE070013 CE-44 B]
 CE01   7J   Harris 7643
 SD06   4D   Harris 7643
 SD07   4C   Harris 7643
 SD05   5B   MMI 6301

MIDDLE PCB: [CEN00007 CE070012 CE-44]
 CR11   4A   MMI 6301
 CR11   4J   MMI 6301
 TE-01  2B   Harris 7643

BOTTOM PCB: [CEN00006 CE070011 CE-44 B]
 SD08   5E   MMI 6331
 SD09   5D   MMI 6331
 SD10   6M   MMI 6331
 RED    2H   MMI 6331

SOUND PCB: [CEN00004A CE070004A]

*/

ROM_START( ttsracec )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0800, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "ce01.7j",      0x0000, 0x0800, CRC(277cd3ca) SHA1(9493f3b4575a10d0b243446d92fdc56a540bc0e4) )
	ROM_LOAD( "sd06.4d",      0x0000, 0x0800, CRC(2ff92235) SHA1(7be446300937362e365b85181d90006e901534aa) )
	ROM_LOAD( "sd07.4c",      0x0000, 0x0800, CRC(38b69397) SHA1(6a072e74d4537c8128bf4a3b91f5636640f2472f) )
	ROM_LOAD( "sd05.5b",      0x0000, 0x0100, CRC(d10920ee) SHA1(39bcff62a028373193875f873a76a42b9105a647) )

	ROM_LOAD( "cr11.4a",      0x0000, 0x0100, CRC(b3250118) SHA1(ab8bfa11b112b03b8ff00b09b3e6d9b221051ad2) )
	ROM_LOAD( "cr11.4j",      0x0000, 0x0100, CRC(b3250118) SHA1(ab8bfa11b112b03b8ff00b09b3e6d9b221051ad2) )
	ROM_LOAD( "te-01.2b",     0x0000, 0x0800, CRC(1bafaae6) SHA1(5b6dcdf1e2f3842b0a7313cbd308ae312a675298) )

	ROM_LOAD( "sd08.5e",      0x0000, 0x0020, CRC(a0fc7c02) SHA1(ea2cf44ab64bfa8d5f9105069a8f38dc75505c30) )
	ROM_LOAD( "sd09.5d",      0x0000, 0x0020, CRC(6e4fa64e) SHA1(04368bcdb2e91a17cd7815ee98ad846bb9732d9b) )
	ROM_LOAD( "sd10.6m",      0x0000, 0x0020, CRC(402e0c59) SHA1(a8e43fe7c2f194ae49977e8ef753049f6b378937) )
	ROM_LOAD( "red.2h",       0x0000, 0x0020, CRC(43a3b7ff) SHA1(a1ff087f7aba211a021a8ead688a3a8780d34174) )
ROM_END


ROM_START( zzblock )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0400, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "tz01.1e",      0x0000, 0x0200, CRC(bd1e59e0) SHA1(038ad0b1b629c6c04f4d885fce6fe1c1f72b33e5) )
	ROM_LOAD( "tz02.1d",      0x0000, 0x0200, CRC(befc8ad3) SHA1(80ce2a9978d22d45ccbe055c7dafba5c0ad6cf3c) )
	ROM_LOAD( "tz03.6f",      0x0000, 0x0020, CRC(1ba385b1) SHA1(b70c356174a1748723c0116ae8dd74b25e1fe6b4) )
ROM_END

//****************************************************************
//******** GAMES AND CLONES NOT FROM TAITO ***********************

/*
Super Road Champions (1978), from Model Racing

7 PCBs:

CS219A (road PCB):
  1 x 22x2 edge connector
  6 x trimmer (VR1-VR6)
  2 x 1 DIP switch bank (SW1-SW2)
  ROMs:
    1 x 6301
    1 x 6331

CS220:
  1 x 22x2 edge connector
  3 x trimmer (VR1-VR3)
  1 x F32582DC 64x7x5 Character Generator (not dumped)
  ROMs:
    3 x 6331

CS221 (sound):
  1 x 22x2 edge connector
  17 x trimmer (VR1-VR17)
  1 x 1 DIP switch bank (SW)
  1 x LM324 Quad Operational Amplifier (sound)
  1 x MM5837N Digital Noise Source (sound)
  1 x N50241 13-Note Top Octave Generator (sound)
  2 x TBA810 Audio Amplifier (sound)
  ROMs:
    1 x 6301

CS222 (main):
  2 x 22x2 edge connector
  1 x LM324 Quad Operational Amplifier (sound)
  ROMs:
    8 x 6301

CS223 (base)

CS224 (power supply):
  2x 22x2 edge connector
  3x trimmer (VR1-VR3)

CS225 (high score):
  1x 11x2 edge connector
*/
ROM_START( srdchamp )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0400, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "cs219_mr31.5d",     0x000, 0x020, CRC(332efd4f) SHA1(dc858b02d92d2a6f466fd4b0a8a2e415050286a5) )
	ROM_LOAD( "cs219_mr32.4j",     0x000, 0x100, CRC(08a808e3) SHA1(11e279e5f80d702155850612b1fe299af5f62167) )

	ROM_LOAD( "cs220_f32582dc.3f", 0x000, 0x400, NO_DUMP ) // CGROM size unknown
	ROM_LOAD( "cs220_mr39.3e",     0x000, 0x020, CRC(46bb17ff) SHA1(14d3b675613e9e13943d5fd38dbaab827f4d7a48) )
	ROM_LOAD( "cs220_mr40.3g",     0x000, 0x020, CRC(65fc450c) SHA1(d75f59775a3d6c82f942245b550be9cec2183848) )
	ROM_LOAD( "cs220_mr41.3h",     0x000, 0x020, CRC(b46a4389) SHA1(260a965b0219feae29bef0891bc96858995894a1) )

	ROM_LOAD( "cs221_mr42.5a",     0x000, 0x100, CRC(9d24a1a8) SHA1(f98b55316dfc00c43830aff77c257265b0b5403d) )

	ROM_LOAD( "cs222_mr33.3c",     0x000, 0x100, CRC(cc2ba2ea) SHA1(80fabb1b0b828f24012285525493f241e1b51a9f) )
	ROM_LOAD( "cs222_mr33.3g",     0x000, 0x100, CRC(cc2ba2ea) SHA1(80fabb1b0b828f24012285525493f241e1b51a9f) )
	ROM_LOAD( "cs222_mr34.3b",     0x000, 0x100, CRC(ce321649) SHA1(2ec5b25e03807ca49ab72dab13173e3d0445b1ac) )
	ROM_LOAD( "cs222_mr34.3h",     0x000, 0x100, CRC(ce321649) SHA1(2ec5b25e03807ca49ab72dab13173e3d0445b1ac) )

	ROM_LOAD( "cs222_mr35.9g",     0x000, 0x100, CRC(f434cc57) SHA1(f2cf604d4d5cd55de31032abea5c35f3179121b0) )
	ROM_LOAD( "cs222_mr36.11f",    0x000, 0x100, CRC(8edc7993) SHA1(bc21ef568d6ee5ba33cefe015269ac2853dc0594) )
	ROM_LOAD( "cs222_mr37.12f",    0x000, 0x100, CRC(85573385) SHA1(db1feef3525247926316743f54e626e0c3da3f2b) )
	ROM_LOAD( "cs222_mr38.11j",    0x000, 0x100, CRC(fb775d23) SHA1(8dd3fca4d08c235c0999bd3623127331eed004d9) )
ROM_END

} // anonymous namespace


GAME( 1977, fisco400, 0, taitottl, 0, taitottl_state, empty_init, ROT0, "Taito", "Cisco/Fisco 400",    MACHINE_IS_SKELETON )
GAME( 1977, gunman,   0, taitottl, 0, taitottl_state, empty_init, ROT0, "Taito", "Gunman",             MACHINE_IS_SKELETON )
GAME( 1977, missilex, 0, taitottl, 0, taitottl_state, empty_init, ROT0, "Taito", "Missile-X",          MACHINE_IS_SKELETON )
GAME( 1977, ttblock,  0, taitottl, 0, taitottl_state, empty_init, ROT0, "Taito", "T.T Block",          MACHINE_IS_SKELETON )
GAME( 1978, ttsracec, 0, taitottl, 0, taitottl_state, empty_init, ROT0, "Taito", "T.T. Speed Race CL", MACHINE_IS_SKELETON )
GAME( 1979, zzblock,  0, taitottl, 0, taitottl_state, empty_init, ROT0, "Taito", "Zun Zun Block",      MACHINE_IS_SKELETON )

// Not from Taito
GAME( 1978, srdchamp, 0, taitottl, 0, taitottl_state, empty_init, ROT0, "Model Racing", "Super Road Champions", MACHINE_IS_SKELETON )
