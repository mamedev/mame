// license:BSD-3-Clause
// copyright-holders:David Haywood
/* JPM System 80 Hardware

  TMS9995 CPU

  +

  ???

  AY8910?

  TMS9902
  (is there a 9901 as well?)

 ---

 There are also older platforms also using the TMS CPU, we load some of those roms here too, but many should go into the SRU folder.


System80 is based on the SRU platform, but with more outputs and finally a separate CPU and sound board setup to permit easy repair.

*/


#include "emu.h"
#include "cpu/tms9900/tms9995.h"
#include "sound/ay8910.h"
#include "machine/tms9902.h"
#include "jpms80.lh"

class jpms80_state : public driver_device
{
public:
	jpms80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }
	virtual void machine_reset() override;

protected:

	// devices
	required_device<cpu_device> m_maincpu;
public:
	DECLARE_DRIVER_INIT(jpms80);
};

static ADDRESS_MAP_START( jpms80_map, AS_PROGRAM, 8, jpms80_state )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x3000, 0x3fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( jpms80_io_map, AS_IO, 8, jpms80_state )
	ADDRESS_MAP_GLOBAL_MASK(0x1ff)
//  AM_RANGE(0x0000, 0x000f) // I/O & Optic (in) / Reels (out)
//  AM_RANGE(0x0050, 0x0050) // INT1 enable (lv3)
//  AM_RANGE(0x0051, 0x0051) // INT2 enable (lv4)
//  AM_RANGE(0x0052, 0x0052) // Watchdog
//  AM_RANGE(0x0053, 0x0053) // I/O Enable
//  AM_RANGE(0x0140, 0x015f) // AY
	AM_RANGE(0x01e0, 0x01ff) AM_DEVREADWRITE("tms9902duart", tms9902_device, cruread, cruwrite)
//  Lamps, Meters etc. can move around
ADDRESS_MAP_END


static INPUT_PORTS_START( jpms80 )
INPUT_PORTS_END

// these are wrong
#define MAIN_CLOCK 2000000
#define SOUND_CLOCK 2000000
#define DUART_CLOCK 2000000

void jpms80_state::machine_reset()
{
	// Disable auto wait state generation by raising the READY line on reset
	static_cast<tms9995_device*>(machine().device("maincpu"))->ready_line(ASSERT_LINE);
}

static MACHINE_CONFIG_START( jpms80, jpms80_state )
	// CPU TMS9995, standard variant; no line connections
	MCFG_TMS99xx_ADD("maincpu", TMS9995, MAIN_CLOCK, jpms80_map, jpms80_io_map)
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DEVICE_ADD("tms9902duart", TMS9902, DUART_CLOCK)

	MCFG_SOUND_ADD("aysnd", AY8910, 2000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

DRIVER_INIT_MEMBER(jpms80_state,jpms80)
{
}



ROM_START( j80bac )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bankacoinp1.bin", 0x0000, 0x1000, CRC(7b82025b) SHA1(f698688c55f8c5dc891e470de8df2eb12f6b1ec5) )
	ROM_LOAD( "bankacoinp2.bin", 0x1000, 0x1000, CRC(91d71fbe) SHA1(d0c45218b7568d5293f015334d7d1045bcb2fe03) )
	ROM_LOAD( "bankacoinp3.bin", 0x2000, 0x1000, CRC(0c3b2954) SHA1(4342a2a047496caf8569d4519dd8daad47e634e3) )
ROM_END

ROM_START( j80blbnk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "blankity-bank-p1.bin", 0x0000, 0x1000, CRC(8b2aeca6) SHA1(207db63d5130cb7a8eb9be41d116432f7b7728b0) )
	ROM_LOAD( "blankity-bank-p2.bin", 0x1000, 0x1000, CRC(561262da) SHA1(d1f3e7815c5ea3ba7c26ed3b95a16c802f6af50f) )
	ROM_LOAD( "blankity-bank-p3.bin", 0x2000, 0x1000, CRC(39925035) SHA1(0af8c8f8074c4873581250474d59d715178cded4) )
ROM_END


ROM_START( j80bounc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bouncer.p1", 0x0000, 0x1000, CRC(81de115b) SHA1(0890de1492859c58411fd130ecf721df7611247a) )
	ROM_LOAD( "bouncer.p2", 0x1000, 0x1000, CRC(8507ea42) SHA1(e4838fe737c8a9964e0067be460e8bfc18b0a406) )
ROM_END



ROM_START( j80frogh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "froghop1.bin", 0x0000, 0x1000, CRC(606846f8) SHA1(8796fb647a41dad087b9eb3e24fa7071c933d1ec) )
	ROM_LOAD( "froghop2.bin", 0x1000, 0x1000, CRC(b64ed5ad) SHA1(5697b0a16191ee3845f0f4077cf7b597f0b20024) )
	ROM_LOAD( "froghop3.bin", 0x2000, 0x1000, CRC(f5b55c0e) SHA1(9fdef9f634f9b832a1bf6e3e3890a7fa328d20e3) )
ROM_END



ROM_START( j80fruit )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fruit_snappa_1-1.bin", 0x0000, 0x1000, CRC(f6eea72d) SHA1(ae994f9eb68aa6ea127586afb448cc8fbff0c314) )
	ROM_LOAD( "fruit_snappa_1-2.bin", 0x1000, 0x1000, CRC(10eccac5) SHA1(3c9cc57a3b51fdae713c11a33677555be3f669bc) )
	ROM_LOAD( "fruit_snappa_1-3.bin", 0x2000, 0x1000, CRC(6f938a9a) SHA1(edbf44ae7cb060420b6f952652f08271c4af35bd) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	// from 'fruit chaser' set, other roms matched.
	ROM_LOAD( "fruit31.p1", 0x0000, 0x001000, CRC(406ff4c1) SHA1(10ef59e66debb15b22d25b66ed19f45a242e30ac) )
ROM_END



ROM_START( j80golds )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "goldstep2-1.p1", 0x0000, 0x1000, CRC(bc1e0788) SHA1(5e01881bda22fc00b2d2ac2b80acc67caddea682) )
	ROM_LOAD( "goldstep2-2.p2", 0x1000, 0x1000, CRC(6ea82bd9) SHA1(289c9a076b9e5039f09283d64ceb77dfd7ea79ea) )
ROM_END



ROM_START( j80hotln )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lines2_1.rom", 0x0000, 0x1000, CRC(f0ce5d7f) SHA1(be3f8ff3f83737a004d6a78cc61c3385307df1c3) )
	ROM_LOAD( "lines2_2.rom", 0x1000, 0x1000, CRC(d5e69b49) SHA1(fcaa527875f81e03c5a5866d6d8b017450c50d9c) )
ROM_END



ROM_START( j80myspn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ms1.bin", 0x0000, 0x1000, CRC(b247374e) SHA1(33399f39bba68eff13e05529174d17f5b1ca0f70) )
	ROM_LOAD( "ms2.bin", 0x1000, 0x1000, CRC(721c35df) SHA1(05ea0cdc83823f268becc7b9dd99db61949ad229) )
ROM_END



ROM_START( j80nudg2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ndu.p1", 0x0000, 0x1000, CRC(4cfd3c6f) SHA1(06ad825343178a694585ee3b4ff8400caf15dd21) ) // aka ndudx5-1-5p.p1 (was in j2nuddud)
ROM_END



ROM_START( j80rr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jpmroadrunnerp1.bin", 0x0000, 0x1000, CRC(86f50997) SHA1(8bb266274d3ebeee942e5f878f7faae012712382) )
	ROM_LOAD( "jpmroadrunnerp2.bin", 0x1000, 0x1000, CRC(aea12b9e) SHA1(6f6eb286c43a9bc04bfcab71713ce59da61cc063) )
	ROM_LOAD( "jpmroadrunnerp3.bin", 0x2000, 0x1000, CRC(9b0b6fb9) SHA1(0282189e2945e4aa3a338930666d1eb34022894c) )
ROM_END


ROM_START( j80rra )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rr.p1", 0x0000, 0x1000, CRC(38cd5043) SHA1(f4b828ad2e761bba91336714357a18f10d79c22b) )
	ROM_LOAD( "rr.p2", 0x1000, 0x1000, CRC(81dc46ec) SHA1(17c60590cf5628df6bd109213a3f671b1a6df14b) )
	ROM_LOAD( "rr.p3", 0x2000, 0x1000, CRC(5e617600) SHA1(1a2a25f81818fc3abeceb74608b2ffd53fac2c6d) )
ROM_END






ROM_START( j80supst )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "supasteppa2-1.p1", 0x0000, 0x1000, CRC(aac5b165) SHA1(5bf4acb85be227e1f4979fea4552fa5f64e9b7b2) )
	ROM_LOAD( "supasteppa2-2.p2", 0x1000, 0x1000, CRC(3a93ea9e) SHA1(24e711a398d7f071fb904993ff0a974b4ac8b1d6) )
ROM_END



ROM_START( j80supbk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sbank-4.1.bin", 0x0000, 0x1000, CRC(effd29fa) SHA1(1e20bc6130f5d49db3856c56c64746f3fa49bd9c) )
	ROM_LOAD( "sbank-4.2.bin", 0x1000, 0x1000, CRC(6ca5cc1d) SHA1(77d9bb44e6837027b61286f30bcb2c1b0e6a53fb) )
	ROM_LOAD( "sbank-4.3.bin", 0x2000, 0x1000, CRC(af08594d) SHA1(ebff60e63e99af102874f4b3f070d9bfd229ab89) )
ROM_END



ROM_START( j80topsp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "topsprint4-1.p1", 0x0000, 0x1000, CRC(91c4f494) SHA1(e4fd688a1fd23694c4fe8529d07ac248f262ad70) )
	ROM_LOAD( "topsprint4-2.p2", 0x1000, 0x1000, CRC(e9ad3706) SHA1(bb6cb1a8ea740be017055e4fa621fabc8df77086) )
	ROM_LOAD( "topsprint4-3.p3", 0x2000, 0x1000, CRC(d1abfb54) SHA1(33b11563c6e1ddfaa5527ad7a384fecd03c7de0a) )
ROM_END



ROM_START( j80topup )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "topup3-1.bin", 0x0000, 0x1000, CRC(2feb37e8) SHA1(098671f81fa94b851a8fa41ee7bd3d1b762eb824) )
	ROM_LOAD( "topup3-2.bin", 0x1000, 0x1000, CRC(1937e7c9) SHA1(a9ae5163e560642598ec9878276d8785c28eb035) )
	ROM_LOAD( "topup3-3.bin", 0x2000, 0x1000, CRC(283d7dd2) SHA1(8246c80c85956a0a3b59d68700319a59b35a5326) )
ROM_END



ROM_START( j80tumbl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tumble3-1.bin", 0x0000, 0x1000, CRC(2feb37e8) SHA1(098671f81fa94b851a8fa41ee7bd3d1b762eb824) )
	ROM_LOAD( "tumble3-2.bin", 0x1000, 0x1000, CRC(1937e7c9) SHA1(a9ae5163e560642598ec9878276d8785c28eb035) )
	ROM_LOAD( "tumble3-3.bin", 0x2000, 0x1000, CRC(23789c80) SHA1(6b6ac4e1dc66d5eb399437e87a9e7ee461bee086) )
ROM_END


ROM_START( j80wsprt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "winsprint4-1.bin", 0x0000, 0x1000, CRC(57259716) SHA1(76ac953f3edefd0fe540e931bd73f564ea49d0c5) )
	ROM_LOAD( "winsprint4-2.bin", 0x1000, 0x1000, CRC(f097bde3) SHA1(22bccd74564fbc2df63ffed3972199d25059b881) )
	ROM_LOAD( "winsprint4-3.bin", 0x2000, 0x1000, CRC(c7318871) SHA1(6534f06be122ed0c9665437485fb860547dd1d7f) )
ROM_END

ROM_START( j80wsprt3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "winsprint3-1.bin", 0x0000, 0x1000, CRC(1af581c2) SHA1(5e04bf37551fc8fc7f477770635f8a573d659a2f) )
	ROM_LOAD( "winsprint3-2.bin", 0x1000, 0x1000, CRC(f097bde3) SHA1(22bccd74564fbc2df63ffed3972199d25059b881) )
	ROM_LOAD( "winsprint3-3.bin", 0x2000, 0x1000, CRC(73098120) SHA1(e353714350681c84f4d3d093dbf07f7e8c70e109) )
ROM_END

ROM_START( j80wsprt2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "winsprint.p1", 0x0000, 0x1000, CRC(e440c7bb) SHA1(5ef85a93a6170115c750257ac6c755b18b3114a9) )
	ROM_LOAD( "winsprint.p2", 0x1000, 0x1000, CRC(225674bf) SHA1(d8a15226ff4f7b16f7f1a8dff969585a6b4536fe) )
	ROM_LOAD( "winsprint.p3", 0x2000, 0x1000, CRC(51d11f59) SHA1(756ba5f02c0733d082767cbdaa93105a7d3f31f3) )
ROM_END

ROM_START( j80alad )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "al_p1.bin", 0x0000, 0x001000, CRC(41134b85) SHA1(8d48af9e3eae8dcc5888a3fa8ae9681ff6047dbb) )
	ROM_LOAD( "al_p2.bin", 0x1000, 0x001000, CRC(934248eb) SHA1(2abb23907acf5036b63185afb164117f1d1bab0c) )
	ROM_LOAD( "al_p3.bin", 0x2000, 0x001000, CRC(da2a56a3) SHA1(1b125be1bd4f0d63c68f370a2a7202f68ff11577) )
ROM_END

ROM_START( j80fortr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fortunetrail1.1", 0x0000, 0x001000, CRC(1691b72f) SHA1(be0966c9560249f2529dd76421fe6646adaeeadb) )
	ROM_LOAD( "fortunetrail1.2", 0x1000, 0x001000, CRC(02dfcfc4) SHA1(ae97b670a4d5b341ee150cabf3d264a02cd7a32c) )
ROM_END

ROM_START( j80mster )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "masterspyp1.bin", 0x0000, 0x001000, CRC(7264e304) SHA1(cd50fd116226de2980f31728faa6723dca0ac061) )
	ROM_LOAD( "masterspyp2.bin", 0x1000, 0x001000, CRC(e6349ee9) SHA1(207a4089e7128143eaeebe1dc082b742b5dfab96) )
	ROM_LOAD( "masterspyp3.bin", 0x2000, 0x001000, CRC(83868a58) SHA1(3264dfa7994c62615da2418d1667cb3bd2e00435) )
ROM_END

ROM_START( j80plsnd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "plus_p1.bin", 0x0000, 0x000400, CRC(02721d4f) SHA1(ea5da3f08098a9d12c71d41d70f09aca6660d6c5) )
	ROM_LOAD( "plus_p2.bin", 0x0400, 0x000400, CRC(f58b492f) SHA1(569805044fa64c1d0c3620f380b4a09152ce2964) )
	ROM_LOAD( "plus_p3.bin", 0x0800, 0x000400, CRC(e9584323) SHA1(7b2101626920bed533b392d1064fde305c8c18e8) )
	ROM_LOAD( "plus_p4.bin", 0x0c00, 0x000400, CRC(67f9d05f) SHA1(1c441c775f2126861858c65c7634773a86f4fcc5) )
	ROM_LOAD( "plus_p5.bin", 0x1000, 0x000400, CRC(d111b2c6) SHA1(c0182a4b163e4dbb67f1c98251b93fa878bff2e2) )
ROM_END



GAME(198?, j80bac   ,0          ,jpms80,jpms80, jpms80_state,jpms80,ROT0,   "JPM","Bank A Coin (JPM) (SYSTEM80)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j80bounc ,0          ,jpms80,jpms80, jpms80_state,jpms80,ROT0,   "JPM","Bouncer (JPM) (SYSTEM80)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j80frogh ,0          ,jpms80,jpms80, jpms80_state,jpms80,ROT0,   "JPM","Frog Hop (JPM) (SYSTEM80)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j80fruit ,0          ,jpms80,jpms80, jpms80_state,jpms80,ROT0,   "JPM","Fruit Snappa (JPM) (SYSTEM80)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j80golds ,0          ,jpms80,jpms80, jpms80_state,jpms80,ROT0,   "JPM","Golden Steppa (JPM) (SYSTEM80)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j80hotln ,0          ,jpms80,jpms80, jpms80_state,jpms80,ROT0,   "JPM","Hot Lines (JPM) (SYSTEM80)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j80myspn ,0          ,jpms80,jpms80, jpms80_state,jpms80,ROT0,   "JPM","Mystery Spin (JPM) (SYSTEM80)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j80nudg2 ,0          ,jpms80,jpms80, jpms80_state,jpms80,ROT0,   "JPM","Nudge Double Up MkII (JPM) (SYSTEM80)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j80rr    ,0          ,jpms80,jpms80, jpms80_state,jpms80,ROT0,   "JPM","Road Runner (JPM) (SYSTEM80, set 1)",                        MACHINE_IS_SKELETON_MECHANICAL ) // was also in a set named 'Route 66' with identical roms, but text in ROM indicates name is Road Runner, maybe a reskin?
GAME(198?, j80rra   ,j80rr      ,jpms80,jpms80, jpms80_state,jpms80,ROT0,   "JPM","Road Runner (JPM) (SYSTEM80, set 2)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j80supst ,0          ,jpms80,jpms80, jpms80_state,jpms80,ROT0,   "JPM","Supa Steppa (JPM) (SYSTEM80)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j80supbk ,0          ,jpms80,jpms80, jpms80_state,jpms80,ROT0,   "JPM","Superbank (JPM) (SYSTEM80)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j80topsp ,0          ,jpms80,jpms80, jpms80_state,jpms80,ROT0,   "JPM","Top Sprint (JPM) (SYSTEM80)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j80topup ,0          ,jpms80,jpms80, jpms80_state,jpms80,ROT0,   "JPM","Top Up (JPM) (SYSTEM80)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j80tumbl ,0          ,jpms80,jpms80, jpms80_state,jpms80,ROT0,   "JPM","Tumble (JPM) (SYSTEM80)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j80wsprt, 0          ,jpms80,jpms80, jpms80_state,jpms80,ROT0,   "JPM","Winsprint (JPM) (V4, 5x20p) (SYSTEM80)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j80wsprt3, j80wsprt  ,jpms80,jpms80, jpms80_state,jpms80,ROT0,   "JPM","Winsprint (JPM) (V3, 50p, 5 credits) (SYSTEM80)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j80wsprt2,j80wsprt   ,jpms80,jpms80, jpms80_state,jpms80,ROT0,   "JPM","Winsprint (JPM) (V2, 10x10p) (SYSTEM80)",                     MACHINE_IS_SKELETON_MECHANICAL )

GAME(198?, j80blbnk ,0          ,jpms80,jpms80, jpms80_state,jpms80,ROT0,   "PCP","Blankity Bank (PCP) (SYSTEM80)",                     MACHINE_IS_SKELETON_MECHANICAL )

// these look like they're probably SYSTEM80, not 100% sure tho
GAME(198?, j80alad  ,0          ,jpms80,jpms80, jpms80_state,jpms80,ROT0,   "JPM","Aladdin's Cave (PCP)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j80fortr ,0          ,jpms80,jpms80, jpms80_state,jpms80,ROT0,   "JPM","Fortune Trail (JPM)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j80mster ,0          ,jpms80,jpms80, jpms80_state,jpms80,ROT0,   "JPM","Masterspy (Pcp)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j80plsnd ,0          ,jpms80,jpms80, jpms80_state,jpms80,ROT0,   "JPM","Plus Nudge (JPM)",                       MACHINE_IS_SKELETON_MECHANICAL )
