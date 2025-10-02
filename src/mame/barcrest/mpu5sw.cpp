// license:BSD-3-Clause
// copyright-holders:David Haywood
/* for MPU5 hardware emulation see mpu5hw.c, this just contains the set listing and per machine configs */

/* Example MPU5 game cart layout:

Barcrest Gold Strike V.1.0 game cart for MPU5 (distributed on Spain by Bilso / Servimatic)
 _________________________________________________
|                                    ___          |
|                                    |  |    ____ |
|                         CPUBUS     |IC|    |SW ||
|                           __       |1 |    |2__||
|   __________  __________ | |       |__|     ___ |
|  | P7      | | P8 EMPTY| | |                |SW||
|  |_________| |_________| | |                |1 ||
|   __________ __________  | |                |__||
|  | P5 EMPTY| | P6 EMPTY| | |  ________     ____ |
|  |_________| |_________| | |  |__IC3__|    |CON||
|   __________  __________ | |               | 1 ||
|  | P3      | | P4      | | |               |___||
|  |_________| |_________| | |   _______     ____ |
|   __________ __________  | |  |__IC2__|    |CON||
|  | P1      | | P2      | | |               | 2 ||
|  |_________| |_________| | |               |___||
|   HIGH BYTE   LOW BYTE   |_|                    |
|                          CBA                    |
|_________________________________________________|

P7 = ST M48T02-150PC1 NVRAM labeled as BILSO S.A. B-14 GOLD STRIKE V 1.0 CVB-0200A / 11-1562 [may be RAM (P7B) or PROM (P7A)]
P1 = 27C4001 labeled as BILSO S.A. B-14 P1 GOLD STRIKE V 1.0 CVB-0200A / 11-1562
P2 = 27C4001 labeled as BILSO S.A. B-14 P2 GOLD STRIKE V 1.0 CVB-0200A / 11-1562
P3 = 27C4001 labeled as BILSO S.A. B-14 P3 GOLD STRIKE V 1.0 CVB-0200A / 11-1562
P4 = 27C4001 labeled as BILSO S.A. B-14 P4 GOLD STRIKE V 1.0 CVB-0200A / 11-1562
IC1 = PIC16C54C labeled as 105RGSG (may be PIC or Z8)
IC2 = GAL16V8B labeled as 105ICIIE
IC3 = GAL16V8B labeled as 105ICI2A
SW1 = 8 dipswitches for options
SW2 = Test switch
CON1 = Female DB9 for percentage
CON2 = Male DB9 for stake/jackpot


       CPUBUS
        __
  DREC2 | | DONE2
   IRQ5 | | SER_INT
   IRQ7 | | TIN1
 CPUCLK | | INTX
   BERR | | GND
  TOUT2 | | 5V
 DSACK1 | | GND
  TOUT1 | | GND
    FC3 | | GND
    BDS | | GND
   SIZ1 | | GND
 EXT_IO | | 5V
     A1 | | GND
     A3 | | GND
     A5 | | GND
     A7 | | GND
     A9 | | GND
    A11 | | 5V
    A13 | | GND
    A15 | | 24V
    A17 | | GND
    A19 | | -12V
    A21 | | GND
    A23 | | 5V
     D1 | | GND
     D3 | | GND
     D5 | | 12V
     D7 | | 12V
     D9 | | 12V
    D11 | | 12V
    D13 | | 12V
    D15 |_| 12V
*/
#include "emu.h"
#include "mpu5.h"

// MFME2MAME layouts:
#include "m5addams.lh"
#include "m5all41d.lh"
#include "m5arab.lh"
#include "m5austin11.lh"
#include "m5barkng.lh"
#include "m5barmy.lh"
#include "m5baxe04.lh"
#include "m5bbro.lh"
#include "m5bbrocl.lh"
#include "m5beansa.lh"
#include "m5bigchs.lh"
#include "m5biggam.lh"
#include "m5bling.lh"
#include "m5blkwht11.lh"
#include "m5bnzclb.lh"
#include "m5btlbnk.lh"
#include "m5bttf.lh"
#include "m5bwaves.lh"
#include "m5carou.lh"
#include "m5cashat.lh"
#include "m5cashrn.lh"
#include "m5cbw.lh"
#include "m5centcl.lh"
#include "m5circlb33.lh"
#include "m5circus0a.lh"
#include "m5clifhn.lh"
#include "m5clown11.lh"
#include "m5codft.lh"
#include "m5cosclb.lh"
#include "m5crzkni.lh"
#include "m5cshkcb.lh"
#include "m5cshstx.lh"
#include "m5dblqtsb.lh"
#include "m5devil.lh"
#include "m5dick10.lh"
#include "m5doshpk05.lh"
#include "m5egr.lh"
#include "m5elband.lh"
#include "m5elim.lh"
#include "m5evgrhr.lh"
#include "m5ewn.lh"
#include "m5extrm.lh"
#include "m5fiddle.lh"
#include "m5fire.lh"
#include "m5firebl.lh"
#include "m5flipcr.lh"
#include "m5fortby.lh"
#include "m5frnzy.lh"
#include "m5funsun.lh"
#include "m5gdrag.lh"
#include "m5ggems20.lh"
#include "m5gimmie.lh"
#include "m5grush.lh"
#include "m5grush5.lh"
#include "m5gsstrk07.lh"
#include "m5gstrik.lh"
#include "m5hellrz.lh"
#include "m5hgl14.lh"
#include "m5hiclau.lh"
#include "m5hifly.lh"
#include "m5hilok.lh"
#include "m5hisprt.lh"
#include "m5hlsumo.lh"
#include "m5holy.lh"
#include "m5hopidl.lh"
#include "m5hotslt.lh"
#include "m5hotstf.lh"
#include "m5hypvip.lh"
#include "m5jackbx.lh"
#include "m5jackp2.lh"
#include "m5jackpt.lh"
#include "m5jlyjwl.lh"
#include "m5jmpgem01.lh"
#include "m5kingqc06.lh"
#include "m5kkebab.lh"
#include "m5korma.lh"
#include "m5loony.lh"
#include "m5loot.lh"
#include "m5lotta.lh"
#include "m5martns07.lh"
#include "m5mega.lh"
#include "m5mmak06.lh"
#include "m5monmst.lh"
#include "m5mpfc.lh"
#include "m5mprio.lh"
#include "m5neptun.lh"
#include "m5nnww.lh"
#include "m5oohaah.lh"
#include "m5oohrio.lh"
#include "m5openbx05.lh"
#include "m5overld.lh"
#include "m5peepsh.lh"
#include "m5piefac.lh"
#include "m5piefcr.lh"
#include "m5ppussy.lh"
#include "m5psyccl01.lh"
#include "m5psycho.lh"
#include "m5ptyani.lh"
#include "m5qdrawb.lh"
#include "m5qshot04.lh"
#include "m5ratpka.lh"
#include "m5razdz10.lh"
#include "m5redbal.lh"
#include "m5redrcka.lh"
#include "m5resfrg.lh"
#include "m5revo13.lh"
#include "m5rfymc.lh"
#include "m5rgclb12.lh"
#include "m5rhrgt02.lh"
#include "m5ritj.lh"
#include "m5rollup.lh"
#include "m5rollx.lh"
#include "m5rthh.lh"
#include "m5rub.lh"
#include "m5rwb.lh"
#include "m5scharg.lh"
#include "m5seven.lh"
#include "m5shark.lh"
#include "m5sheik.lh"
#include "m5skulcl20.lh"
#include "m5sondra.lh"
#include "m5speccl.lh"
#include "m5spiker.lh"
#include "m5spins.lh"
#include "m5squids06.lh"
#include "m5sstrk.lh"
#include "m5starcl.lh"
#include "m5stars26.lh"
#include "m5stax.lh"
#include "m5supnov.lh"
#include "m5supro.lh"
#include "m5tbird.lh"
#include "m5tempcl.lh"
#include "m5tempp.lh"
#include "m5tempt2.lh"
#include "m5tictacbwb.lh"
#include "m5trail.lh"
#include "m5ultimo04.lh"
#include "m5upover.lh"
#include "m5vampup.lh"
#include "m5vertgo.lh"
#include "m5wking05.lh"
#include "m5wonga.lh"
#include "m5wthing20.lh"
#include "m5xchn.lh"
#include "m5xfact11.lh"


/* Empire
   Ace Of Clubs
   the clones all differ by only 1 ROM
*/

ROM_START( m5aceclb )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "acec0_3.p1", 0x000000, 0x80000, CRC(da4f7950) SHA1(d40db8c27b4a6bbfe4fe553684766a83ee29d7c3) )
	ROM_LOAD16_BYTE( "acec0_3.p2", 0x000001, 0x80000, CRC(7fbeaece) SHA1(db462aa647c29e41b9f72f53c7e89e57bcc4a446) )
	ROM_LOAD16_BYTE( "acec0_3.p3", 0x100000, 0x80000, CRC(7d57dc0e) SHA1(6a63c5226ee04d7a4c884e34f382c2c124b8d867) )
	ROM_LOAD16_BYTE( "acec0_3.p4", 0x100001, 0x80000, CRC(34010608) SHA1(f649cc719b68482ee744a097c8047c17316b66f1) )
ROM_END

ROM_START( m5aceclba )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "acec0_6.p1", 0x000000, 0x80000, CRC(a7fbeb10) SHA1(5f7bd5325e9db4561fb8a6e1ea02765e172ee68d) )
	ROM_LOAD16_BYTE( "acec0_6.p2", 0x000001, 0x80000, CRC(d4d4543f) SHA1(6d5c907e37eb804ba21f0666d92b2f062a18739c) )
	ROM_LOAD16_BYTE( "acec0_3.p3", 0x100000, 0x80000, CRC(7d57dc0e) SHA1(6a63c5226ee04d7a4c884e34f382c2c124b8d867) )
	ROM_LOAD16_BYTE( "acec0_3.p4", 0x100001, 0x80000, CRC(34010608) SHA1(f649cc719b68482ee744a097c8047c17316b66f1) )
ROM_END

ROM_START( m5aceclbb )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "acec0_6d.p1",0x000000, 0x80000, CRC(c4baad89) SHA1(46b4d0dd8cdf0d020089d7cb064daad5e2881728) ) // only changed ROM from m5aceclba
	ROM_LOAD16_BYTE( "acec0_6.p2", 0x000001, 0x80000, CRC(d4d4543f) SHA1(6d5c907e37eb804ba21f0666d92b2f062a18739c) )
	ROM_LOAD16_BYTE( "acec0_3.p3", 0x100000, 0x80000, CRC(7d57dc0e) SHA1(6a63c5226ee04d7a4c884e34f382c2c124b8d867) )
	ROM_LOAD16_BYTE( "acec0_3.p4", 0x100001, 0x80000, CRC(34010608) SHA1(f649cc719b68482ee744a097c8047c17316b66f1) )
ROM_END

/* Vivid
   Magnificent 7's
   the clones all differ by only 1 ROM
*/

ROM_START( m5mag7s )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "msesjb_1.3_1", 0x000000, 0x80000, CRC(4f97c046) SHA1(c1260e9d8984c62298a30a22085860de8e25d0db) )
	ROM_LOAD16_BYTE( "msesjb_1.3_2", 0x000001, 0x80000, CRC(cf9eaaf0) SHA1(83d04335dd3d9bc7e74a21f75f4c882ec7b7048d) )
	ROM_LOAD16_BYTE( "msesjb_1.3_3", 0x100000, 0x80000, CRC(c67a758a) SHA1(0cd3de1480d8c9e87737d064affdb5aa5d25fb51) )
	ROM_LOAD16_BYTE( "msesjb_1.3_4", 0x100001, 0x80000, CRC(5c88688a) SHA1(7b03ee580f50d6c1f6ca72e75f85c6b7567a08a0) )
ROM_END

ROM_START( m5mag7sa )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "msesja_1.3_1", 0x000000, 0x80000, CRC(a036d8fb) SHA1(e948fefda577403cd52020cdea8a40129f34e580) ) // only changed ROM from m_mag7s
	ROM_LOAD16_BYTE( "msesjb_1.3_2", 0x000001, 0x80000, CRC(cf9eaaf0) SHA1(83d04335dd3d9bc7e74a21f75f4c882ec7b7048d) )
	ROM_LOAD16_BYTE( "msesjb_1.3_3", 0x100000, 0x80000, CRC(c67a758a) SHA1(0cd3de1480d8c9e87737d064affdb5aa5d25fb51) )
	ROM_LOAD16_BYTE( "msesjb_1.3_4", 0x100001, 0x80000, CRC(5c88688a) SHA1(7b03ee580f50d6c1f6ca72e75f85c6b7567a08a0) )
ROM_END

ROM_START( m5mag7sb )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "msesjad1.3_1", 0x000000, 0x80000, CRC(8be417e6) SHA1(bde19e8a49ae1993d9f0166dd593f2dd0b739386) ) // only changed ROM from m_mag7s
	ROM_LOAD16_BYTE( "msesjb_1.3_2", 0x000001, 0x80000, CRC(cf9eaaf0) SHA1(83d04335dd3d9bc7e74a21f75f4c882ec7b7048d) )
	ROM_LOAD16_BYTE( "msesjb_1.3_3", 0x100000, 0x80000, CRC(c67a758a) SHA1(0cd3de1480d8c9e87737d064affdb5aa5d25fb51) )
	ROM_LOAD16_BYTE( "msesjb_1.3_4", 0x100001, 0x80000, CRC(5c88688a) SHA1(7b03ee580f50d6c1f6ca72e75f85c6b7567a08a0) )
ROM_END

ROM_START( m5mag7sc )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "msesjbd1.3_1", 0x000000, 0x80000, CRC(64450f5b) SHA1(07c91d6685ce0cd7cea8a04cb40446058161111d) ) // only changed ROM from m_mag7s
	ROM_LOAD16_BYTE( "msesjb_1.3_2", 0x000001, 0x80000, CRC(cf9eaaf0) SHA1(83d04335dd3d9bc7e74a21f75f4c882ec7b7048d) )
	ROM_LOAD16_BYTE( "msesjb_1.3_3", 0x100000, 0x80000, CRC(c67a758a) SHA1(0cd3de1480d8c9e87737d064affdb5aa5d25fb51) )
	ROM_LOAD16_BYTE( "msesjb_1.3_4", 0x100001, 0x80000, CRC(5c88688a) SHA1(7b03ee580f50d6c1f6ca72e75f85c6b7567a08a0) )
ROM_END

ROM_START( m5mag7sd )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "msesjbg1.3_1", 0x000000, 0x80000, CRC(c1a2cbcf) SHA1(dc9178df1600889aa6e204a6a29082d1867eb007) ) // only changed ROM from m_mag7s
	ROM_LOAD16_BYTE( "msesjb_1.3_2", 0x000001, 0x80000, CRC(cf9eaaf0) SHA1(83d04335dd3d9bc7e74a21f75f4c882ec7b7048d) )
	ROM_LOAD16_BYTE( "msesjb_1.3_3", 0x100000, 0x80000, CRC(c67a758a) SHA1(0cd3de1480d8c9e87737d064affdb5aa5d25fb51) )
	ROM_LOAD16_BYTE( "msesjb_1.3_4", 0x100001, 0x80000, CRC(5c88688a) SHA1(7b03ee580f50d6c1f6ca72e75f85c6b7567a08a0) )
ROM_END

ROM_START( m5mag7se )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "msesjbt1.3_1", 0x000000, 0x80000, CRC(863962b1) SHA1(d0d3d9c0e7214ed785df1d5b496d79200fa10d06) ) // only changed ROM from m_mag7s
	ROM_LOAD16_BYTE( "msesjb_1.3_2", 0x000001, 0x80000, CRC(cf9eaaf0) SHA1(83d04335dd3d9bc7e74a21f75f4c882ec7b7048d) )
	ROM_LOAD16_BYTE( "msesjb_1.3_3", 0x100000, 0x80000, CRC(c67a758a) SHA1(0cd3de1480d8c9e87737d064affdb5aa5d25fb51) )
	ROM_LOAD16_BYTE( "msesjb_1.3_4", 0x100001, 0x80000, CRC(5c88688a) SHA1(7b03ee580f50d6c1f6ca72e75f85c6b7567a08a0) )
ROM_END

/* Vivid
   Manic Streak Features
*/

ROM_START( m5msf )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "man_sjs1.3_1", 0x000000, 0x80000, CRC(622f285f) SHA1(90638a9dcb26c3ff92ff4e7da847ea92951efa67) )
	ROM_LOAD16_BYTE( "man_sjs1.3_2", 0x000001, 0x80000, CRC(e6a59fea) SHA1(b44ffcab4f85e07c022fdf5a280ad95ee596d145) )
	ROM_LOAD16_BYTE( "man_sjs1.3_3", 0x100000, 0x80000, CRC(558db803) SHA1(c378520b2ef54b7a96721cbaa4abc127a0eec04a) )
	ROM_LOAD16_BYTE( "man_sjs1.3_4", 0x100001, 0x80000, CRC(f2a7895f) SHA1(a2378072a47857ad38eda9c309d77b45ef694b21) )
ROM_END

ROM_START( m5msfa )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "man_sjs1.3d1", 0x000000, 0x80000, CRC(8efb1a0a) SHA1(d217cb1d941dd89b31d5b6d444950ceb0da1666d) )
	ROM_LOAD16_BYTE( "man_sjs1.3d2", 0x000001, 0x80000, CRC(e6a59fea) SHA1(b44ffcab4f85e07c022fdf5a280ad95ee596d145) )
	ROM_LOAD16_BYTE( "man_sjs1.3_3", 0x100000, 0x80000, CRC(558db803) SHA1(c378520b2ef54b7a96721cbaa4abc127a0eec04a) )
	ROM_LOAD16_BYTE( "man_sjs1.3_4", 0x100001, 0x80000, CRC(f2a7895f) SHA1(a2378072a47857ad38eda9c309d77b45ef694b21) )
ROM_END

/* Emipre
   Money Monster
   not sure about the 2nd set, do 1.7 and 1.6 really mix like that?
*/

ROM_START( m5monmst )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mm_sj__1.4_1", 0x000000, 0x80000, CRC(9659bc3f) SHA1(499fdd4aa643afc8689be426fa11d6ead6c953bc) )
	ROM_LOAD16_BYTE( "mm_sj__1.4_2", 0x000001, 0x80000, CRC(19374068) SHA1(f6a112d7a9c61bda39c046a91a576208e37f494a) )
	ROM_LOAD16_BYTE( "mm_sj__1.4_3", 0x100000, 0x80000, CRC(2a35f8cb) SHA1(fda935737b1c3d4ad72ad197183dacc9d32aecc0) )
	ROM_LOAD16_BYTE( "mm_sj__1.4_4", 0x100001, 0x80000, CRC(2ea1203a) SHA1(735d35c1c6c80f3b3c319e225e711d87fc648f4c) )
ROM_END

ROM_START( m5monmsta )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "momo1_7.p1", 0x000000, 0x80000, CRC(0f11b40b) SHA1(1cf37e4171792c659566c363af840c913f36ab4b) )
	ROM_LOAD16_BYTE( "momo1_7.p2", 0x000001, 0x80000, CRC(cd8f5d54) SHA1(1416ca80d21d47d8ea6df211e087e7e42df7478a) )
	ROM_LOAD16_BYTE( "momo16.p3",  0x100000, 0x80000, CRC(2a35f8cb) SHA1(fda935737b1c3d4ad72ad197183dacc9d32aecc0) )
	ROM_LOAD16_BYTE( "momo16.p4",  0x100001, 0x80000, CRC(2ea1203a) SHA1(735d35c1c6c80f3b3c319e225e711d87fc648f4c) )
ROM_END


/* Vivid
   Honey Money
*/

ROM_START( m5honmon )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hmo_23s.p1", 0x000000, 0x80000, CRC(b3a44b47) SHA1(f54399bb1cc01fd4d615bd2c1a539c132b99d811) )
	ROM_LOAD16_BYTE( "hmo_23l.p2", 0x000001, 0x80000, CRC(09e116f7) SHA1(0d94b957f4bb3ef6aa10511c69e51d5400698622) )
	ROM_LOAD16_BYTE( "hmo_23l.p3", 0x100000, 0x80000, CRC(072b4af0) SHA1(799280cd27e53e167f28c5ad71868e8cd29b0200) )
	ROM_LOAD16_BYTE( "hmo_23l.p4", 0x100001, 0x80000, CRC(8443e257) SHA1(55c9dda40368914481b40b7224081c67758c5717) )
	ROM_LOAD16_BYTE( "hmo_23l.p5", 0x200000, 0x80000, CRC(e1cdf0f4) SHA1(0a1d09fefced246ce0c7692171c56f45050c49fd) )
	ROM_LOAD16_BYTE( "hmo_23l.p6", 0x200001, 0x80000, CRC(32496a48) SHA1(7c29020c1b35dd078e0807b86f67ec41432d84a1) )
ROM_END

ROM_START( m5honmona )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "honey.p1", 0x00000, 0x080000, CRC(22ade9cd) SHA1(29600f23fbb55ff5624b97fb4d503adda4bdd8dc) )
	ROM_LOAD16_BYTE( "honey.p2", 0x00001, 0x080000, CRC(065cba42) SHA1(46755e5ef4dbbba89fc57cdb09bb92c4a2213640) )
	/* 3,4,5,6 */
ROM_END

/* Sets below haven't been split into parent / clones yet! */


ROM_START( m5sixsht )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ssh_sjs1.1_1", 0x000000, 0x080000, CRC(77955a9a) SHA1(18bb2740bebcf5ce4cb48bdc4896a85d1d1ec75e) )
	ROM_LOAD16_BYTE( "ssh_sjs1.1_2", 0x000001, 0x080000, CRC(0f61b0b7) SHA1(185971aeefab6cb002a41929cb3d38ef51591303) )
	ROM_LOAD16_BYTE( "ssh_sjs1.1_3", 0x100000, 0x080000, CRC(d1ba8953) SHA1(40c0bdab8aa719db796591e03f2befcb117efbb4) )
	ROM_LOAD16_BYTE( "ssh_sjs1.1_4", 0x100001, 0x080000, CRC(45e81620) SHA1(84ff802b880eef5aedaad09b28caafa4e6219e7b) )
ROM_END

ROM_START( m5sixshta )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ssh_sjs1.1d1", 0x000000, 0x080000, CRC(422fa3e7) SHA1(79471610ddd8dd8f00b4be0d2e4c8f4882ce03b6) )
	ROM_LOAD16_BYTE( "ssh_sjs1.1_2", 0x000001, 0x080000, CRC(0f61b0b7) SHA1(185971aeefab6cb002a41929cb3d38ef51591303) )
	ROM_LOAD16_BYTE( "ssh_sjs1.1_3", 0x100000, 0x080000, CRC(d1ba8953) SHA1(40c0bdab8aa719db796591e03f2befcb117efbb4) )
	ROM_LOAD16_BYTE( "ssh_sjs1.1_4", 0x100001, 0x080000, CRC(45e81620) SHA1(84ff802b880eef5aedaad09b28caafa4e6219e7b) )
ROM_END

/* should these be using 3+4 from above? */
ROM_START( m5sixshtb )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ssh_20h2.0d1", 0x000000, 0x080000, CRC(19644b40) SHA1(5068e9f9ade5c5a46e54f7b4b18d4922c02f2add) )
	ROM_LOAD16_BYTE( "ssh_sj2.0_2",  0x000001, 0x080000, CRC(455b5618) SHA1(162ea4b4c50bb5fdf97a27dd0ee19686da66967d) )
ROM_END

ROM_START( m5sixshtc )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ssh_sja2.0_1", 0x000000, 0x080000, CRC(ed0bfbc9) SHA1(728f117a1aee6dd4acc88d0044d1b8a50c97d163) )
	ROM_LOAD16_BYTE( "ssh_sj2.0_2",  0x000001, 0x080000, CRC(455b5618) SHA1(162ea4b4c50bb5fdf97a27dd0ee19686da66967d) )
ROM_END

ROM_START( m5sixshtd )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ssh_sjh2.0d1", 0x000000, 0x080000, CRC(76de5395) SHA1(14955b51040f8f99b05c1c638b6611d3a1b6bd15) )
	ROM_LOAD16_BYTE( "ssh_sj2.0_2",  0x000001, 0x080000, CRC(455b5618) SHA1(162ea4b4c50bb5fdf97a27dd0ee19686da66967d) )
ROM_END

ROM_START( m5sixshte )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ssh_sjk2.0_1", 0x000000, 0x080000, CRC(33b7fbda) SHA1(c0bc9aa07511a19d407494f379025977f5a17cd7) )
	ROM_LOAD16_BYTE( "ssh_sj2.0_2",  0x000001, 0x080000, CRC(455b5618) SHA1(162ea4b4c50bb5fdf97a27dd0ee19686da66967d) )
ROM_END

ROM_START( m5sixshtf )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ssh_sjl2.0d1", 0x000000, 0x080000, CRC(bfc749c1) SHA1(586c83b463eaae71db3b69df5d100cb2f03bee87) )
	ROM_LOAD16_BYTE( "ssh_sj2.0_2",  0x000001, 0x080000, CRC(455b5618) SHA1(162ea4b4c50bb5fdf97a27dd0ee19686da66967d) )
ROM_END

ROM_START( m5sixshtg )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ssh_sjs2.0_1", 0x000000, 0x080000, CRC(f1d45c8c) SHA1(9828749be008548599a9bc229bb7918cc08ef114) )
	ROM_LOAD16_BYTE( "ssh_sj2.0_2",  0x000001, 0x080000, CRC(455b5618) SHA1(162ea4b4c50bb5fdf97a27dd0ee19686da66967d) )
ROM_END

ROM_START( m5sixshth )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ssh_sjs2.0d1", 0x000000, 0x080000, CRC(b4bdf4c3) SHA1(13d4e1499a689b1d80ac6dfb3834fa8639940efc) )
	ROM_LOAD16_BYTE( "ssh_sj2.0_2",  0x000001, 0x080000, CRC(455b5618) SHA1(162ea4b4c50bb5fdf97a27dd0ee19686da66967d) )
ROM_END

ROM_START( m5sixshti )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ssh_sjs2.1_1", 0x000000, 0x080000, CRC(5b15856a) SHA1(fb697d332ce3ce6aeeebd09beb0d745a94c30ab6) )
	ROM_LOAD16_BYTE( "ssh_sj2.1_2",  0x000001, 0x080000, CRC(ff614ab4) SHA1(a49601295d8eb518cf39567a7b5116e4367273c5) )
ROM_END

ROM_START( m5sixshtj )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ssh_20h2.1d1", 0x000000, 0x080000, CRC(10b80756) SHA1(6caa557cf9ad866189443c6b9ba59c73f7ed634f) )
	ROM_LOAD16_BYTE( "ssh_sj2.1_2",  0x000001, 0x080000, CRC(ff614ab4) SHA1(a49601295d8eb518cf39567a7b5116e4367273c5) )
ROM_END

ROM_START( m5sixshtk )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ssh_sjh2.1d1", 0x000000, 0x080000, CRC(230973bd) SHA1(489255ad731966e8c6e54fa5b1b2165b0503c428) )
	ROM_LOAD16_BYTE( "ssh_sj2.1_2",  0x000001, 0x080000, CRC(ff614ab4) SHA1(a49601295d8eb518cf39567a7b5116e4367273c5) )
ROM_END

ROM_START( m5sixshtl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ssh_sjk2.1_1", 0x000000, 0x080000, CRC(ccbd35fb) SHA1(56179564be5af8948879477599ffac09aeb783fb) )
	ROM_LOAD16_BYTE( "ssh_sj2.1_2",  0x000001, 0x080000, CRC(ff614ab4) SHA1(a49601295d8eb518cf39567a7b5116e4367273c5) )
ROM_END

ROM_START( m5sixshtm )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ssh_sjl2.1d1", 0x000000, 0x080000, CRC(cce0a2a1) SHA1(366042d9551d8ee864d67c577c9c1050953ad85a) )
	ROM_LOAD16_BYTE( "ssh_sj2.1_2",  0x000001, 0x080000, CRC(ff614ab4) SHA1(a49601295d8eb518cf39567a7b5116e4367273c5) )
ROM_END

ROM_START( m5sixshtn )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ssh_sjs2.1d1", 0x000000, 0x080000, CRC(b4a1c32c) SHA1(803d7fa2ae47440a7ea2318b93cb585a626800e4) )
	ROM_LOAD16_BYTE( "ssh_sj2.1_2",  0x000001, 0x080000, CRC(ff614ab4) SHA1(a49601295d8eb518cf39567a7b5116e4367273c5) )
ROM_END


ROM_START( m5fewmor )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fewd0_2.p1", 0x000000, 0x080000, CRC(422c0992) SHA1(71f72e7790526e4723068c9b3204a014d33c778d) )
	ROM_LOAD16_BYTE( "fewd0_2.p2", 0x000001, 0x080000, CRC(aebba280) SHA1(c087b704587746a7f7c155c0ffd6ba1418302bed) )
ROM_END

ROM_START( m5fewmora )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fewd0_2d.p1", 0x000000, 0x080000, CRC(a52715af) SHA1(5146d95e64e0f78d19f9166547960fda10f2610b) )
	ROM_LOAD16_BYTE( "fewd0_2.p2",  0x000001, 0x080000, CRC(aebba280) SHA1(c087b704587746a7f7c155c0ffd6ba1418302bed) )
ROM_END

ROM_START( m5fewmorb )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fewd0_3.p1", 0x000000, 0x080000, CRC(738305f8) SHA1(57de9bfbfdaf33bb1af6fda17bf97219baa39596) )
	ROM_LOAD16_BYTE( "fewd0_3.p2", 0x000001, 0x080000, CRC(6ac63e0b) SHA1(94cda7755b996e0a0f7b9e646438b8736c78f648) )
ROM_END

ROM_START( m5fewmorc )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fewd0_3d.p1", 0x000000, 0x080000, CRC(4b02fc98) SHA1(a9324378032881e83237948a6001d5cb393606d7) )
	ROM_LOAD16_BYTE( "fewd0_3.p2",  0x000001, 0x080000, CRC(6ac63e0b) SHA1(94cda7755b996e0a0f7b9e646438b8736c78f648) )
ROM_END


ROM_START( m5wonga )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fish0_1.p1", 0x000000, 0x080000, CRC(4d59ad1d) SHA1(9f49204ee237b3d1750e4dc448fad1e6816b2038) )
	ROM_LOAD16_BYTE( "fish0_1.p2", 0x000001, 0x080000, CRC(1fb1df8d) SHA1(9bcf4814a07cc2e3132c435d706c94ddf6888391) )
	ROM_LOAD16_BYTE( "fish0_1.p3", 0x100000, 0x080000, CRC(fc990d69) SHA1(8f01202aff1c67dbd210cb57c4f31caa1c1d7326) )
	ROM_LOAD16_BYTE( "fish0_1.p4", 0x100001, 0x080000, CRC(3cf608c0) SHA1(972f802cff2dc08a1205822918cb0e4d19049013) )
ROM_END

ROM_START( m5addams )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "admf05k.p1", 0x000000, 0x080000, CRC(44905d92) SHA1(298f86db7575f4bb969f464f8608ae8e44991078) )
	ROM_LOAD16_BYTE( "admf05.p2", 0x000001, 0x080000, CRC(70f4b107) SHA1(0e34a4930eaf898b8e16bbf739191890fedb04b9) )
	ROM_LOAD16_BYTE( "admf02.p3", 0x100000, 0x080000, CRC(20a4e22f) SHA1(c73278fd90e2f44d9d83216c29c8e52faed238ae) ) // 05 == 02
	ROM_LOAD16_BYTE( "admf02.p4", 0x100001, 0x080000, CRC(f42102b9) SHA1(2daa37a7e0c176909ba5a7a421612db3ea38ebef) ) // 05 == 02
ROM_END

ROM_START( m5addamsa )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "admf05ad.p1", 0x000000, 0x080000, CRC(1441eee4) SHA1(dd7254f9d7ea199d42efb06f956985e5217ab009) )
	ROM_LOAD16_BYTE( "admf05.p2", 0x000001, 0x080000, CRC(70f4b107) SHA1(0e34a4930eaf898b8e16bbf739191890fedb04b9) )
	ROM_LOAD16_BYTE( "admf02.p3", 0x100000, 0x080000, CRC(20a4e22f) SHA1(c73278fd90e2f44d9d83216c29c8e52faed238ae) )
	ROM_LOAD16_BYTE( "admf02.p4", 0x100001, 0x080000, CRC(f42102b9) SHA1(2daa37a7e0c176909ba5a7a421612db3ea38ebef) )
ROM_END

ROM_START( m5addamsb )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "admf05b.p1", 0x000000, 0x080000, CRC(5f5bcd26) SHA1(1641fbcfcad946c28a25ecac064e90ad3434da98) )
	ROM_LOAD16_BYTE( "admf05.p2", 0x000001, 0x080000, CRC(70f4b107) SHA1(0e34a4930eaf898b8e16bbf739191890fedb04b9) )
	ROM_LOAD16_BYTE( "admf02.p3", 0x100000, 0x080000, CRC(20a4e22f) SHA1(c73278fd90e2f44d9d83216c29c8e52faed238ae) )
	ROM_LOAD16_BYTE( "admf02.p4", 0x100001, 0x080000, CRC(f42102b9) SHA1(2daa37a7e0c176909ba5a7a421612db3ea38ebef) )
ROM_END

ROM_START( m5addamsc )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "admf05dy.p1", 0x000000, 0x080000, CRC(972a0584) SHA1(1e1cef6be4bbbefdecb025bf0a0273447bc17b0a) )
	ROM_LOAD16_BYTE( "admf05.p2", 0x000001, 0x080000, CRC(70f4b107) SHA1(0e34a4930eaf898b8e16bbf739191890fedb04b9) )
	ROM_LOAD16_BYTE( "admf02.p3", 0x100000, 0x080000, CRC(20a4e22f) SHA1(c73278fd90e2f44d9d83216c29c8e52faed238ae) )
	ROM_LOAD16_BYTE( "admf02.p4", 0x100001, 0x080000, CRC(f42102b9) SHA1(2daa37a7e0c176909ba5a7a421612db3ea38ebef) )
ROM_END

ROM_START( m5addamsd )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "admf05h.p1", 0x000000, 0x080000, CRC(0787e74a) SHA1(7afa100bf497b649a817b2cc5f46a8297fb7f2ab) )
	ROM_LOAD16_BYTE( "admf05.p2", 0x000001, 0x080000, CRC(70f4b107) SHA1(0e34a4930eaf898b8e16bbf739191890fedb04b9) )
	ROM_LOAD16_BYTE( "admf02.p3", 0x100000, 0x080000, CRC(20a4e22f) SHA1(c73278fd90e2f44d9d83216c29c8e52faed238ae) )
	ROM_LOAD16_BYTE( "admf02.p4", 0x100001, 0x080000, CRC(f42102b9) SHA1(2daa37a7e0c176909ba5a7a421612db3ea38ebef) )
ROM_END

ROM_START( m5addamse )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "admf05r.p1", 0x000000, 0x080000, CRC(44a3ac58) SHA1(b1721ab24cddc2436fc0caf9e3e4a0861b126484) )
	ROM_LOAD16_BYTE( "admf05.p2", 0x000001, 0x080000, CRC(70f4b107) SHA1(0e34a4930eaf898b8e16bbf739191890fedb04b9) )
	ROM_LOAD16_BYTE( "admf02.p3", 0x100000, 0x080000, CRC(20a4e22f) SHA1(c73278fd90e2f44d9d83216c29c8e52faed238ae) )
	ROM_LOAD16_BYTE( "admf02.p4", 0x100001, 0x080000, CRC(f42102b9) SHA1(2daa37a7e0c176909ba5a7a421612db3ea38ebef) )
ROM_END

ROM_START( m5addamsf )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "admf05s.p1", 0x000000, 0x080000, CRC(002a6e41) SHA1(608067b0bd2755c6d330622f472908330c7c4993) )
	ROM_LOAD16_BYTE( "admf05.p2", 0x000001, 0x080000, CRC(70f4b107) SHA1(0e34a4930eaf898b8e16bbf739191890fedb04b9) )
	ROM_LOAD16_BYTE( "admf02.p3", 0x100000, 0x080000, CRC(20a4e22f) SHA1(c73278fd90e2f44d9d83216c29c8e52faed238ae) )
	ROM_LOAD16_BYTE( "admf02.p4", 0x100001, 0x080000, CRC(f42102b9) SHA1(2daa37a7e0c176909ba5a7a421612db3ea38ebef) )
ROM_END

ROM_START( m5addamsg )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "admf05y.p1", 0x000000, 0x080000, CRC(352b5417) SHA1(391aabe1a21b53eb4199f8e985e1410e266a253d) )
	ROM_LOAD16_BYTE( "admf05.p2", 0x000001, 0x080000, CRC(70f4b107) SHA1(0e34a4930eaf898b8e16bbf739191890fedb04b9) )
	ROM_LOAD16_BYTE( "admf02.p3", 0x100000, 0x080000, CRC(20a4e22f) SHA1(c73278fd90e2f44d9d83216c29c8e52faed238ae) )
	ROM_LOAD16_BYTE( "admf02.p4", 0x100001, 0x080000, CRC(f42102b9) SHA1(2daa37a7e0c176909ba5a7a421612db3ea38ebef) )
ROM_END

ROM_START( m5addamsh )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "adtr02s.p1", 0x000000, 0x080000, CRC(62f7b34a) SHA1(751fc47aaab01dd4565403e47413e68f612c3554) )
	ROM_LOAD16_BYTE( "adtr02.p2",  0x000001, 0x080000, CRC(d33a8a9f) SHA1(18457ca60f78204ee67fb993f63342e01666d950) )
	ROM_LOAD16_BYTE( "admf02.p3", 0x100000, 0x080000, CRC(20a4e22f) SHA1(c73278fd90e2f44d9d83216c29c8e52faed238ae) )
	ROM_LOAD16_BYTE( "admf02.p4", 0x100001, 0x080000, CRC(f42102b9) SHA1(2daa37a7e0c176909ba5a7a421612db3ea38ebef) )
ROM_END

ROM_START( m5addamsi )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "adtr02dy.p1", 0x000000, 0x080000, CRC(21f4bcd4) SHA1(40aa0cb2c494a3582284d9caadc259c9249353b5) )
	ROM_LOAD16_BYTE( "adtr02.p2",  0x000001, 0x080000, CRC(d33a8a9f) SHA1(18457ca60f78204ee67fb993f63342e01666d950) )
	ROM_LOAD16_BYTE( "admf02.p3", 0x100000, 0x080000, CRC(20a4e22f) SHA1(c73278fd90e2f44d9d83216c29c8e52faed238ae) )
	ROM_LOAD16_BYTE( "admf02.p4", 0x100001, 0x080000, CRC(f42102b9) SHA1(2daa37a7e0c176909ba5a7a421612db3ea38ebef) )
ROM_END

ROM_START( m5addamsj )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "adtr02y.p1", 0x000000, 0x080000, CRC(70e38117) SHA1(3b686f707d16921bb70bc7be2a6639caa09de341) )
	ROM_LOAD16_BYTE( "adtr02.p2",  0x000001, 0x080000, CRC(d33a8a9f) SHA1(18457ca60f78204ee67fb993f63342e01666d950) )
	ROM_LOAD16_BYTE( "admf02.p3", 0x100000, 0x080000, CRC(20a4e22f) SHA1(c73278fd90e2f44d9d83216c29c8e52faed238ae) )
	ROM_LOAD16_BYTE( "admf02.p4", 0x100001, 0x080000, CRC(f42102b9) SHA1(2daa37a7e0c176909ba5a7a421612db3ea38ebef) )
ROM_END

ROM_START( m5addamsk )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "adtr03s.p1", 0x000000, 0x080000, CRC(af664a9a) SHA1(a6abdf179018fff3fa97507eabbc85fd1db790cf) )
	ROM_LOAD16_BYTE( "adtr03.p2", 0x000000, 0x080000, CRC(a932cb2d) SHA1(a2b2cb0d171480e1c15f021580a684e77403630b) )
	ROM_LOAD16_BYTE( "admf02.p3", 0x100000, 0x080000, CRC(20a4e22f) SHA1(c73278fd90e2f44d9d83216c29c8e52faed238ae) )
	ROM_LOAD16_BYTE( "admf02.p4", 0x100001, 0x080000, CRC(f42102b9) SHA1(2daa37a7e0c176909ba5a7a421612db3ea38ebef) )
ROM_END

ROM_START( m5addamsl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "adtr03ad.p1", 0x000000, 0x080000, CRC(657592e2) SHA1(d22d1bd80975feaebe24aac56892fb998ea27a1a) )
	ROM_LOAD16_BYTE( "adtr03.p2", 0x000000, 0x080000, CRC(a932cb2d) SHA1(a2b2cb0d171480e1c15f021580a684e77403630b) )
	ROM_LOAD16_BYTE( "admf02.p3", 0x100000, 0x080000, CRC(20a4e22f) SHA1(c73278fd90e2f44d9d83216c29c8e52faed238ae) )
	ROM_LOAD16_BYTE( "admf02.p4", 0x100001, 0x080000, CRC(f42102b9) SHA1(2daa37a7e0c176909ba5a7a421612db3ea38ebef) )
ROM_END

ROM_START( m5addamsm )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "adtr03b.p1", 0x000000, 0x080000, CRC(adda9dc9) SHA1(ca34f0a0fff272ae29dec2ca3135a3454b852216) )
	ROM_LOAD16_BYTE( "adtr03.p2", 0x000000, 0x080000, CRC(a932cb2d) SHA1(a2b2cb0d171480e1c15f021580a684e77403630b) )
	ROM_LOAD16_BYTE( "admf02.p3", 0x100000, 0x080000, CRC(20a4e22f) SHA1(c73278fd90e2f44d9d83216c29c8e52faed238ae) )
	ROM_LOAD16_BYTE( "admf02.p4", 0x100001, 0x080000, CRC(f42102b9) SHA1(2daa37a7e0c176909ba5a7a421612db3ea38ebef) )
ROM_END

ROM_START( m5addamsn )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "adtr03bd.p1", 0x000000, 0x080000, CRC(25b6c611) SHA1(638e1eb9791ab913e51f1ce4443b77dbe8bfb7fc) )
	ROM_LOAD16_BYTE( "adtr03.p2", 0x000000, 0x080000, CRC(a932cb2d) SHA1(a2b2cb0d171480e1c15f021580a684e77403630b) )
	ROM_LOAD16_BYTE( "admf02.p3", 0x100000, 0x080000, CRC(20a4e22f) SHA1(c73278fd90e2f44d9d83216c29c8e52faed238ae) )
	ROM_LOAD16_BYTE( "admf02.p4", 0x100001, 0x080000, CRC(f42102b9) SHA1(2daa37a7e0c176909ba5a7a421612db3ea38ebef) )
ROM_END

ROM_START( m5addamso )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "adtr03d.p1", 0x000000, 0x080000, CRC(270a1142) SHA1(68015422622077358828492b5dc14f965fe348f2) )
	ROM_LOAD16_BYTE( "adtr03.p2", 0x000000, 0x080000, CRC(a932cb2d) SHA1(a2b2cb0d171480e1c15f021580a684e77403630b) )
	ROM_LOAD16_BYTE( "admf02.p3", 0x100000, 0x080000, CRC(20a4e22f) SHA1(c73278fd90e2f44d9d83216c29c8e52faed238ae) )
	ROM_LOAD16_BYTE( "admf02.p4", 0x100001, 0x080000, CRC(f42102b9) SHA1(2daa37a7e0c176909ba5a7a421612db3ea38ebef) )
ROM_END

ROM_START( m5addamsp )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "adtr03dy.p1", 0x000000, 0x080000, CRC(a224cb0b) SHA1(e8c15e4a180033ada0d001ef502e04a925fa79a0) )
	ROM_LOAD16_BYTE( "adtr03.p2", 0x000000, 0x080000, CRC(a932cb2d) SHA1(a2b2cb0d171480e1c15f021580a684e77403630b) )
	ROM_LOAD16_BYTE( "admf02.p3", 0x100000, 0x080000, CRC(20a4e22f) SHA1(c73278fd90e2f44d9d83216c29c8e52faed238ae) )
	ROM_LOAD16_BYTE( "admf02.p4", 0x100001, 0x080000, CRC(f42102b9) SHA1(2daa37a7e0c176909ba5a7a421612db3ea38ebef) )
ROM_END

ROM_START( m5addamsq )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "adtr03k.p1", 0x000000, 0x080000, CRC(8657158d) SHA1(9a5a6dd9776d925a63fec40d56e6f704fb949661) )
	ROM_LOAD16_BYTE( "adtr03.p2", 0x000000, 0x080000, CRC(a932cb2d) SHA1(a2b2cb0d171480e1c15f021580a684e77403630b) )
	ROM_LOAD16_BYTE( "admf02.p3", 0x100000, 0x080000, CRC(20a4e22f) SHA1(c73278fd90e2f44d9d83216c29c8e52faed238ae) )
	ROM_LOAD16_BYTE( "admf02.p4", 0x100001, 0x080000, CRC(f42102b9) SHA1(2daa37a7e0c176909ba5a7a421612db3ea38ebef) )
ROM_END

ROM_START( m5addamsr )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "adtr03r.p1", 0x000000, 0x080000, CRC(531a2e97) SHA1(187c0523a2032d6ff6be1300879a83fa95eb9f7b) )
	ROM_LOAD16_BYTE( "adtr03.p2", 0x000000, 0x080000, CRC(a932cb2d) SHA1(a2b2cb0d171480e1c15f021580a684e77403630b) )
	ROM_LOAD16_BYTE( "admf02.p3", 0x100000, 0x080000, CRC(20a4e22f) SHA1(c73278fd90e2f44d9d83216c29c8e52faed238ae) )
	ROM_LOAD16_BYTE( "admf02.p4", 0x100001, 0x080000, CRC(f42102b9) SHA1(2daa37a7e0c176909ba5a7a421612db3ea38ebef) )
ROM_END

ROM_START( m5addamss )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "adtr03y.p1", 0x000000, 0x080000, CRC(e39ed644) SHA1(b4b0cbca2240d0f42ff671abec7eb88a7f884862) )
	ROM_LOAD16_BYTE( "adtr03.p2", 0x000000, 0x080000, CRC(a932cb2d) SHA1(a2b2cb0d171480e1c15f021580a684e77403630b) )
	ROM_LOAD16_BYTE( "admf02.p3", 0x100000, 0x080000, CRC(20a4e22f) SHA1(c73278fd90e2f44d9d83216c29c8e52faed238ae) )
	ROM_LOAD16_BYTE( "admf02.p4", 0x100001, 0x080000, CRC(f42102b9) SHA1(2daa37a7e0c176909ba5a7a421612db3ea38ebef) )
ROM_END



ROM_START( m5addlad )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "addr06ad.p1", 0x000000, 0x080000, CRC(104db505) SHA1(d1afaabce5cb0899208758b64d4efca16250ebf9) )
	ROM_LOAD16_BYTE( "addr06.p2", 0x000001, 0x080000, CRC(20c6d5be) SHA1(6d3c6cec0043072e4cd67f2c25a813eadf8eee6a) )
	ROM_LOAD16_BYTE( "addr06.p3", 0x100000, 0x080000, CRC(2761decd) SHA1(0c1ac294d48dba1bb991e7df3974ee2357231dd8) )
	ROM_LOAD16_BYTE( "addr06.p4", 0x100001, 0x080000, CRC(449ee8a8) SHA1(f9702b87d284b8e8d69c72db92869b87fe47c0ad) )
ROM_END

ROM_START( m5addlada )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "addr06b.p1", 0x000000, 0x080000, CRC(e4cde9b1) SHA1(6b1196de5a0c5435aba670c81ee00bf7e480571b) )
	ROM_LOAD16_BYTE( "addr06.p2", 0x000001, 0x080000, CRC(20c6d5be) SHA1(6d3c6cec0043072e4cd67f2c25a813eadf8eee6a) )
	ROM_LOAD16_BYTE( "addr06.p3", 0x100000, 0x080000, CRC(2761decd) SHA1(0c1ac294d48dba1bb991e7df3974ee2357231dd8) )
	ROM_LOAD16_BYTE( "addr06.p4", 0x100001, 0x080000, CRC(449ee8a8) SHA1(f9702b87d284b8e8d69c72db92869b87fe47c0ad) )
ROM_END

ROM_START( m5addladb )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "addr06bd.p1", 0x000000, 0x080000, CRC(97650266) SHA1(2e88abe9807814ebf64f9471f9e4f9a65f7c4c1b) )
	ROM_LOAD16_BYTE( "addr06.p2", 0x000001, 0x080000, CRC(20c6d5be) SHA1(6d3c6cec0043072e4cd67f2c25a813eadf8eee6a) )
	ROM_LOAD16_BYTE( "addr06.p3", 0x100000, 0x080000, CRC(2761decd) SHA1(0c1ac294d48dba1bb991e7df3974ee2357231dd8) )
	ROM_LOAD16_BYTE( "addr06.p4", 0x100001, 0x080000, CRC(449ee8a8) SHA1(f9702b87d284b8e8d69c72db92869b87fe47c0ad) )
ROM_END

ROM_START( m5addladc )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "addr06d.p1", 0x000000, 0x080000, CRC(fa9415d4) SHA1(1dd43f200ef24eb82cb94a604ba28a9247252e8c) )
	ROM_LOAD16_BYTE( "addr06.p2", 0x000001, 0x080000, CRC(20c6d5be) SHA1(6d3c6cec0043072e4cd67f2c25a813eadf8eee6a) )
	ROM_LOAD16_BYTE( "addr06.p3", 0x100000, 0x080000, CRC(2761decd) SHA1(0c1ac294d48dba1bb991e7df3974ee2357231dd8) )
	ROM_LOAD16_BYTE( "addr06.p4", 0x100001, 0x080000, CRC(449ee8a8) SHA1(f9702b87d284b8e8d69c72db92869b87fe47c0ad) )
ROM_END

ROM_START( m5addladd )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "addr06dy.p1", 0x000000, 0x080000, CRC(18483fe9) SHA1(ff8551746c465096d3d4c2c4c6ae91ff9a1de94b) )
	ROM_LOAD16_BYTE( "addr06.p2", 0x000001, 0x080000, CRC(20c6d5be) SHA1(6d3c6cec0043072e4cd67f2c25a813eadf8eee6a) )
	ROM_LOAD16_BYTE( "addr06.p3", 0x100000, 0x080000, CRC(2761decd) SHA1(0c1ac294d48dba1bb991e7df3974ee2357231dd8) )
	ROM_LOAD16_BYTE( "addr06.p4", 0x100001, 0x080000, CRC(449ee8a8) SHA1(f9702b87d284b8e8d69c72db92869b87fe47c0ad) )
ROM_END

ROM_START( m5addlade )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "addr06h.p1", 0x000000, 0x080000, CRC(613f3a1c) SHA1(da74b1ab86b1d5d9513ee7e860fa9f78d52979d7) )
	ROM_LOAD16_BYTE( "addr06.p2", 0x000001, 0x080000, CRC(20c6d5be) SHA1(6d3c6cec0043072e4cd67f2c25a813eadf8eee6a) )
	ROM_LOAD16_BYTE( "addr06.p3", 0x100000, 0x080000, CRC(2761decd) SHA1(0c1ac294d48dba1bb991e7df3974ee2357231dd8) )
	ROM_LOAD16_BYTE( "addr06.p4", 0x100001, 0x080000, CRC(449ee8a8) SHA1(f9702b87d284b8e8d69c72db92869b87fe47c0ad) )
ROM_END

ROM_START( m5addladf )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "addr06k.p1", 0x000000, 0x080000, CRC(fcf4adac) SHA1(a3d91af384e84f83c48aa5657fd652572ba46799) )
	ROM_LOAD16_BYTE( "addr06.p2", 0x000001, 0x080000, CRC(20c6d5be) SHA1(6d3c6cec0043072e4cd67f2c25a813eadf8eee6a) )
	ROM_LOAD16_BYTE( "addr06.p3", 0x100000, 0x080000, CRC(2761decd) SHA1(0c1ac294d48dba1bb991e7df3974ee2357231dd8) )
	ROM_LOAD16_BYTE( "addr06.p4", 0x100001, 0x080000, CRC(449ee8a8) SHA1(f9702b87d284b8e8d69c72db92869b87fe47c0ad) )
ROM_END

ROM_START( m5addladg )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "addr06r.p1", 0x000000, 0x080000, CRC(5f39651f) SHA1(c747c87d689c5e494e5637fccb4285f4d7d5fdb1) )
	ROM_LOAD16_BYTE( "addr06.p2", 0x000001, 0x080000, CRC(20c6d5be) SHA1(6d3c6cec0043072e4cd67f2c25a813eadf8eee6a) )
	ROM_LOAD16_BYTE( "addr06.p3", 0x100000, 0x080000, CRC(2761decd) SHA1(0c1ac294d48dba1bb991e7df3974ee2357231dd8) )
	ROM_LOAD16_BYTE( "addr06.p4", 0x100001, 0x080000, CRC(449ee8a8) SHA1(f9702b87d284b8e8d69c72db92869b87fe47c0ad) )
ROM_END

ROM_START( m5addladh )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "addr06s.p1", 0x000000, 0x080000, CRC(893cfe03) SHA1(92b72d0ea1b39c6d8de11cfaf1a10eeaca8703d4) )
	ROM_LOAD16_BYTE( "addr06.p2", 0x000001, 0x080000, CRC(20c6d5be) SHA1(6d3c6cec0043072e4cd67f2c25a813eadf8eee6a) )
	ROM_LOAD16_BYTE( "addr06.p3", 0x100000, 0x080000, CRC(2761decd) SHA1(0c1ac294d48dba1bb991e7df3974ee2357231dd8) )
	ROM_LOAD16_BYTE( "addr06.p4", 0x100001, 0x080000, CRC(449ee8a8) SHA1(f9702b87d284b8e8d69c72db92869b87fe47c0ad) )
ROM_END

ROM_START( m5addladi )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "addr06y.p1", 0x000000, 0x080000, CRC(6be0d43e) SHA1(abdc1f4a7be8fc6d4a77e764126f6b45a865049c) )
	ROM_LOAD16_BYTE( "addr06.p2", 0x000001, 0x080000, CRC(20c6d5be) SHA1(6d3c6cec0043072e4cd67f2c25a813eadf8eee6a) )
	ROM_LOAD16_BYTE( "addr06.p3", 0x100000, 0x080000, CRC(2761decd) SHA1(0c1ac294d48dba1bb991e7df3974ee2357231dd8) )
	ROM_LOAD16_BYTE( "addr06.p4", 0x100001, 0x080000, CRC(449ee8a8) SHA1(f9702b87d284b8e8d69c72db92869b87fe47c0ad) )
ROM_END

ROM_START( m5addladj )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "artr01ad.p1", 0x000000, 0x080000, CRC(2c36d12c) SHA1(7362234394d130f9d85166598ea7d60abf94977c) )
	ROM_LOAD16_BYTE( "artr01.p2", 0x000001, 0x080000, CRC(f164c8fc) SHA1(70cc0fa816bd9ef6ce941a38cb146ca7e390f238) )
	ROM_LOAD16_BYTE( "artr01.p3", 0x100000, 0x080000, CRC(8ba9377e) SHA1(65e78c8b571cdd021a00c6bf981d0344d9c37ec0) )
	ROM_LOAD16_BYTE( "artr01.p4", 0x100001, 0x080000, CRC(ba1ed9e0) SHA1(29df221030571052fe6577391a60fb3ff0ab7730) )
ROM_END

ROM_START( m5addladk )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "artr01b.p1", 0x000000, 0x080000, CRC(22a8b341) SHA1(b6268ffe4f03e0695d29fbfb1b9de95d81eb7700) )
	ROM_LOAD16_BYTE( "artr01.p2", 0x000001, 0x080000, CRC(f164c8fc) SHA1(70cc0fa816bd9ef6ce941a38cb146ca7e390f238) )
	ROM_LOAD16_BYTE( "artr01.p3", 0x100000, 0x080000, CRC(8ba9377e) SHA1(65e78c8b571cdd021a00c6bf981d0344d9c37ec0) )
	ROM_LOAD16_BYTE( "artr01.p4", 0x100001, 0x080000, CRC(ba1ed9e0) SHA1(29df221030571052fe6577391a60fb3ff0ab7730) )
ROM_END

ROM_START( m5addladl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "artr01d.p1", 0x000000, 0x080000, CRC(2e58112f) SHA1(cc374bbb4137e90f84d2ae4733aefe4fa770cbc1) )
	ROM_LOAD16_BYTE( "artr01.p2", 0x000001, 0x080000, CRC(f164c8fc) SHA1(70cc0fa816bd9ef6ce941a38cb146ca7e390f238) )
	ROM_LOAD16_BYTE( "artr01.p3", 0x100000, 0x080000, CRC(8ba9377e) SHA1(65e78c8b571cdd021a00c6bf981d0344d9c37ec0) )
	ROM_LOAD16_BYTE( "artr01.p4", 0x100001, 0x080000, CRC(ba1ed9e0) SHA1(29df221030571052fe6577391a60fb3ff0ab7730) )
ROM_END

ROM_START( m5addladm )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "artr01dy.p1", 0x000000, 0x080000, CRC(730c8ecf) SHA1(9967eb33bc2564394a1dacca5badcadc368f7a04) )
	ROM_LOAD16_BYTE( "artr01.p2", 0x000001, 0x080000, CRC(f164c8fc) SHA1(70cc0fa816bd9ef6ce941a38cb146ca7e390f238) )
	ROM_LOAD16_BYTE( "artr01.p3", 0x100000, 0x080000, CRC(8ba9377e) SHA1(65e78c8b571cdd021a00c6bf981d0344d9c37ec0) )
	ROM_LOAD16_BYTE( "artr01.p4", 0x100001, 0x080000, CRC(ba1ed9e0) SHA1(29df221030571052fe6577391a60fb3ff0ab7730) )
ROM_END

ROM_START( m5addladn )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "artr01k.p1", 0x000000, 0x080000, CRC(e094b466) SHA1(7adf44d2d6eec86a7d6838e4e262ae8f34cf405d) )
	ROM_LOAD16_BYTE( "artr01.p2", 0x000001, 0x080000, CRC(f164c8fc) SHA1(70cc0fa816bd9ef6ce941a38cb146ca7e390f238) )
	ROM_LOAD16_BYTE( "artr01.p3", 0x100000, 0x080000, CRC(8ba9377e) SHA1(65e78c8b571cdd021a00c6bf981d0344d9c37ec0) )
	ROM_LOAD16_BYTE( "artr01.p4", 0x100001, 0x080000, CRC(ba1ed9e0) SHA1(29df221030571052fe6577391a60fb3ff0ab7730) )
ROM_END

ROM_START( m5addlado )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "artr01s.p1", 0x000000, 0x080000, CRC(cdc0256e) SHA1(e31ff30228d6b5ab31302976cec83ba8cd377d73) )
	ROM_LOAD16_BYTE( "artr01.p2", 0x000001, 0x080000, CRC(f164c8fc) SHA1(70cc0fa816bd9ef6ce941a38cb146ca7e390f238) )
	ROM_LOAD16_BYTE( "artr01.p3", 0x100000, 0x080000, CRC(8ba9377e) SHA1(65e78c8b571cdd021a00c6bf981d0344d9c37ec0) )
	ROM_LOAD16_BYTE( "artr01.p4", 0x100001, 0x080000, CRC(ba1ed9e0) SHA1(29df221030571052fe6577391a60fb3ff0ab7730) )
ROM_END

ROM_START( m5addladp )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "artr01y.p1", 0x000000, 0x080000, CRC(9094ba8e) SHA1(9a53c0552f74a219a23715a88fde5082e2d34e50) )
	ROM_LOAD16_BYTE( "artr01.p2", 0x000001, 0x080000, CRC(f164c8fc) SHA1(70cc0fa816bd9ef6ce941a38cb146ca7e390f238) )
	ROM_LOAD16_BYTE( "artr01.p3", 0x100000, 0x080000, CRC(8ba9377e) SHA1(65e78c8b571cdd021a00c6bf981d0344d9c37ec0) )
	ROM_LOAD16_BYTE( "artr01.p4", 0x100001, 0x080000, CRC(ba1ed9e0) SHA1(29df221030571052fe6577391a60fb3ff0ab7730) )
ROM_END

ROM_START( m5addladq )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "addsjs0.4_1", 0x000000, 0x080000, CRC(53d3e902) SHA1(67271f1a87861b95c43869199ad095dc2ccb111c) )
	ROM_LOAD16_BYTE( "addsjs0.4_2", 0x000001, 0x080000, CRC(314fbbc1) SHA1(1a1b6bb4ffb00cac970eff67ddcca11ee4a84876) )
	/* which 3+4 pair, or missing? */
ROM_END

ROM_START( m5addladr )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "addsjs0.4d1", 0x000000, 0x080000, CRC(83deb932) SHA1(e8a94221ab249223be22a059ec96fae3ae60aa12) )
	ROM_LOAD16_BYTE( "addsjs0.4d2", 0x000000, 0x080000, CRC(314fbbc1) SHA1(1a1b6bb4ffb00cac970eff67ddcca11ee4a84876) )
	/* which 3+4 pair, or missing? */
ROM_END

ROM_START( m5addlads )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "c adders.p1", 0x000000, 0x080000, CRC(0058d970) SHA1(6353fba189dc17a8b7f582a182e3aeccc6b5f7bf) )
	ROM_LOAD16_BYTE( "c adders.p2", 0x000001, 0x080000, CRC(8b235678) SHA1(c7644db37132c4cce861889a4d45ad66495134e5) )
	/* which 3+4 pair, or missing? */
ROM_END






ROM_START( m5ashock )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ash12s.p1", 0x000000, 0x080000, CRC(9365be33) SHA1(6c3814c255925c2b6010fb9b81ea7cd9bfb1f483) )
	ROM_LOAD16_BYTE( "ash12s.p2", 0x000001, 0x080000, CRC(bbfad58b) SHA1(f1a1d2fae52fa4fd2b057105a1338909e9c8ec7d) )
	ROM_LOAD16_BYTE( "ash12s.p3", 0x100000, 0x080000, CRC(d8161daa) SHA1(0b0fb0debcfa3be6f6c37c7013078b87593f0a95) )
	ROM_LOAD16_BYTE( "ash12s.p4", 0x100001, 0x080000, CRC(a6276057) SHA1(4b3b5cadf082cc6ba5a87c8b5df3934d1c7752b4) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "ash12d.p3", 0x000000, 0x080000, CRC(1ce434b1) SHA1(97af9714bfa0dbab34303c446b9101abdea7ab4a) )
	ROM_LOAD16_BYTE( "ash12e.p3", 0x000000, 0x080000, CRC(16256c61) SHA1(ff001374dac1cbef4a6301d04cf7989d44c5c3fe) )
	ROM_LOAD16_BYTE( "ash12k.p3", 0x000000, 0x080000, CRC(9f01f87d) SHA1(83eee3d48c9ef71c1bfd0e7167d38dff1a3d6ab1) )
ROM_END

ROM_START( m5ashocka )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ash13s.p1", 0x000000, 0x080000, CRC(99c935eb) SHA1(2ddad31c08d82dc40748fc2e8585ee63356fceef) )
	ROM_LOAD16_BYTE( "ash13s.p2", 0x000001, 0x080000, CRC(6269f21f) SHA1(77eb65a47b869d48fb4b8163afb23bd07485faf6) )
	ROM_LOAD16_BYTE( "ash13s.p3", 0x100000, 0x080000, CRC(c943494e) SHA1(e557e934a2e593879e0b5781134efb8b7ef1597c) )
	ROM_LOAD16_BYTE( "ash13s.p4", 0x100001, 0x080000, CRC(6f91996b) SHA1(ded42f280bd607808875748a75a266ec551ce619) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "ash13d.p1", 0x000000, 0x080000, CRC(1f2165e7) SHA1(45e0966052e890138d08081db9b747ebe9c7ed2e) )
	ROM_LOAD16_BYTE( "ash13k.p1", 0x000000, 0x080000, CRC(bd0ba407) SHA1(d2fa42cab351517430c1f0d648d18a369e9ac079) )
ROM_END

ROM_START( m5all41 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "a4__sjk1.2_1", 0x000000, 0x080000, CRC(cab3de23) SHA1(4a50665fff238f8a2c3240dbc0da13674094ae69) )
	ROM_LOAD16_BYTE( "a4__sjk1.2_2", 0x000001, 0x080000, CRC(e49d240a) SHA1(e0d0873dd73559378b62ce936f0898679d3732a1) )
	ROM_LOAD16_BYTE( "a4__sj1.1_3",  0x100000, 0x080000, CRC(3e03f73d) SHA1(97d15790edf5f08e37b25fc21640047bbf7a5da7) ) // 1.2 ==  1.1
	ROM_LOAD16_BYTE( "a4__sj1.1_4",  0x100001, 0x080000, CRC(79bfdc16) SHA1(38547276e8da9f60a7f2e143b2311df0e10b22e1) ) // 1.2 ==  1.1
ROM_END

ROM_START( m5all41a )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "a4__sjs1.3d1", 0x000000, 0x080000, CRC(9feff7df) SHA1(e3c6199da52442fd3e42b36b3c8d1241e346742a) )
	ROM_LOAD16_BYTE( "a4__sjs1.3d2", 0x000001, 0x080000, CRC(20c7a288) SHA1(957d421a191d018863ae4ea7399e132ecf29acb4) )
	ROM_LOAD16_BYTE( "a4__sj1.1_3",  0x100000, 0x080000, CRC(3e03f73d) SHA1(97d15790edf5f08e37b25fc21640047bbf7a5da7) ) // 1.3 ==  1.1
	ROM_LOAD16_BYTE( "a4__sj1.1_4",  0x100001, 0x080000, CRC(79bfdc16) SHA1(38547276e8da9f60a7f2e143b2311df0e10b22e1) ) // 1.3 ==  1.1
ROM_END

ROM_START( m5all41b )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "a4_sjs1.3_1",  0x000000, 0x080000, CRC(2577b58c) SHA1(358b533d9c456db762abc80ce365d711a7c02c4c) )
	ROM_LOAD16_BYTE( "a4_sjs1.3_2",  0x000000, 0x080000, CRC(9f5942ed) SHA1(0d510cd69232722307abf3e3abe5aad8937bbe77) )
	ROM_LOAD16_BYTE( "a4__sj1.1_3",  0x100000, 0x080000, CRC(3e03f73d) SHA1(97d15790edf5f08e37b25fc21640047bbf7a5da7) ) // 1.3 ==  1.1
	ROM_LOAD16_BYTE( "a4__sj1.1_4",  0x100001, 0x080000, CRC(79bfdc16) SHA1(38547276e8da9f60a7f2e143b2311df0e10b22e1) ) // 1.3 ==  1.1
ROM_END

ROM_START( m5all41c )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ao__sjh1.1d1", 0x000000, 0x080000, CRC(7fcc1e6b) SHA1(d825e0442249c9f6e3bf2b20300afbdf58f63083) )
	ROM_LOAD16_BYTE( "ao__sjh1.1d2", 0x000001, 0x080000, CRC(34934857) SHA1(a77f4ea425bc045fe785e4b03ff2aff7717755bf) )
	ROM_LOAD16_BYTE( "a4__sj1.1_3",  0x100000, 0x080000, CRC(3e03f73d) SHA1(97d15790edf5f08e37b25fc21640047bbf7a5da7) )
	ROM_LOAD16_BYTE( "a4__sj1.1_4",  0x100001, 0x080000, CRC(79bfdc16) SHA1(38547276e8da9f60a7f2e143b2311df0e10b22e1) )
ROM_END

ROM_START( m5all41d )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ao__sjk1.1_1", 0x000000, 0x080000, CRC(10b5668d) SHA1(cc1878d5e1c15e22bb298b6008a33a93df1ab53a) )
	ROM_LOAD16_BYTE( "ao__sjk1.1_2", 0x000000, 0x080000, CRC(34934857) SHA1(a77f4ea425bc045fe785e4b03ff2aff7717755bf) )
	ROM_LOAD16_BYTE( "a4__sj1.1_3",  0x100000, 0x080000, CRC(3e03f73d) SHA1(97d15790edf5f08e37b25fc21640047bbf7a5da7) )
	ROM_LOAD16_BYTE( "a4__sj1.1_4",  0x100001, 0x080000, CRC(79bfdc16) SHA1(38547276e8da9f60a7f2e143b2311df0e10b22e1) )
ROM_END

ROM_START( m5all41e )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ao__sjs1.4_1", 0x000000, 0x080000, CRC(9a4c3f50) SHA1(1790552fd7bad11d12fd72757b435d9c3387400a) )
	ROM_LOAD16_BYTE( "ao__sjs1.4_2", 0x000001, 0x080000, CRC(f6eec85f) SHA1(7618a936e8c564f6b63e8fcb0404422d95d7bc27) )
	ROM_LOAD16_BYTE( "a4__sj1.1_3",  0x100000, 0x080000, CRC(3e03f73d) SHA1(97d15790edf5f08e37b25fc21640047bbf7a5da7) ) // 1.4 ==  1.1
	ROM_LOAD16_BYTE( "a4__sj1.1_4",  0x100001, 0x080000, CRC(79bfdc16) SHA1(38547276e8da9f60a7f2e143b2311df0e10b22e1) ) // 1.4 ==  1.1
ROM_END

ROM_START( m5all41f )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ao_sjs1.6_1",  0x000000, 0x080000, CRC(cd181bb5) SHA1(58d9b13c194e9962af9cb7a71d48707cb63c650f) )
	ROM_LOAD16_BYTE( "ao_sjs1.6_2",  0x000001, 0x080000, CRC(2ad7933e) SHA1(6d4e3b6749817eb171901ec5b6917992446111d1) )
	/* 3+4 unsure */
ROM_END

ROM_START( m5all41g )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ao_sjs1.6d1", 0x000000, 0x080000, CRC(15d23f47) SHA1(0af6dd916491d2501596195cc6fb1bd88b2de3ae) )
	ROM_LOAD16_BYTE( "ao_sjs1.6d2", 0x000000, 0x080000, CRC(2ad7933e) SHA1(6d4e3b6749817eb171901ec5b6917992446111d1) )
	/* 3+4 unsure */
ROM_END

ROM_START( m5all41h )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "all 4 1.p2", 0x0000, 0x080000, CRC(7c48076f) SHA1(b6b812aa91a5e86b0db8a1e6ca24834208852582) )
	ROM_LOAD16_BYTE( "all 4 1.p1", 0x0000, 0x080000, CRC(227ec737) SHA1(5b19a65677755b0e279d30a3889de06bca862fc7) )
	/* 3+4 unsure */
ROM_END

ROM_START( m5all41i )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ao__20h1.8d1", 0x000000, 0x080000, CRC(be4e0f0d) SHA1(f0f0bc4d2f629c46a743408f4b9c73e2ed71dfbd) )
	ROM_LOAD16_BYTE( "ao__20h1.8d2", 0x000001, 0x080000, CRC(7b48f566) SHA1(7a94063605dde5ad0098e4c8fdc9e2d3a5c1f451) )
	/* 3+4 unsure */
ROM_END

ROM_START( m5all41j )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ao__sjh1.8d1", 0x000000, 0x080000, CRC(3bcdb15a) SHA1(aab3ddd367ddd776e2d0007d9933aa5873554d38) )
	ROM_LOAD16_BYTE( "ao__sjh1.8d2", 0x000001, 0x080000, CRC(7b48f566) SHA1(7a94063605dde5ad0098e4c8fdc9e2d3a5c1f451) )
	/* 3+4 unsure */
ROM_END

ROM_START( m5all41k )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ao__sjk1.8_1", 0x000000, 0x080000, CRC(3fedb808) SHA1(3da29acc59f4d1c82a38a40897804874ea2a8f92) )
	ROM_LOAD16_BYTE( "ao__sjk1.8_2", 0x000001, 0x080000, CRC(7b48f566) SHA1(7a94063605dde5ad0098e4c8fdc9e2d3a5c1f451) )
	/* 3+4 unsure */
ROM_END

ROM_START( m5all41l )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ao__sjs1.8_1", 0x000000, 0x080000, CRC(e19c59aa) SHA1(f3a53ebc1637358618739a3866b5d40dda7d8d7c) )
	ROM_LOAD16_BYTE( "ao__sjs1.8_2", 0x000001, 0x080000, CRC(7b48f566) SHA1(7a94063605dde5ad0098e4c8fdc9e2d3a5c1f451) )
	/* 3+4 unsure */
ROM_END

ROM_START( m5all41m )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ao__sjs1.8d1", 0x000000, 0x080000, CRC(e5bc50f8) SHA1(dc5c565f26f7220e0a30e658aec0aeeb3f3779a9) )
	ROM_LOAD16_BYTE( "ao__sjs1.8d2", 0x000001, 0x080000, CRC(7b48f566) SHA1(7a94063605dde5ad0098e4c8fdc9e2d3a5c1f451) )
	/* 3+4 unsure */
ROM_END

ROM_START( m5all41low )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "aod_2_2.p1", 0x000000, 0x080000, CRC(83c60b4d) SHA1(ae2bec2505f7c6b204d3f9d72957f98f0758f349) )
	ROM_LOAD16_BYTE( "aod_2_2.p2", 0x000001, 0x080000, CRC(9490fbd7) SHA1(10682470b08090a7b5fdb4734cdc3e12d53e48b6) )
	ROM_LOAD16_BYTE( "aod_2_2.p3", 0x100000, 0x080000, CRC(6ece83dd) SHA1(693f69c29238305a2ab15f364df804cd9118d9a1) )
	ROM_LOAD16_BYTE( "aod_2_2.p4", 0x100001, 0x080000, CRC(1011712c) SHA1(f775ce0ff792cfbbdac0a33969acab24c81a6906) )
ROM_END


ROM_START( m5arab )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "arab09ad.p1", 0x000000, 0x080000, CRC(1f68a20a) SHA1(0055428f07377725361eb8c25b01b58613336ba9) )
	ROM_LOAD16_BYTE( "arab09.p2", 0x000001, 0x080000, CRC(af8ee71e) SHA1(167fa55fbdb5d5377c18c696d279d4018128f790) )
	ROM_LOAD16_BYTE( "arab09.p3", 0x100000, 0x080000, CRC(e6bbd54e) SHA1(3e217406aca645a6ebff139b2b02b28120a10657) )
	ROM_LOAD16_BYTE( "arab09.p4", 0x100001, 0x080000, CRC(0893b767) SHA1(b6d56a58163462425507c077ef8a5bcdd4dc5180) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "arab09b.p1", 0x000000, 0x080000, CRC(c0db14dc) SHA1(68403912edd003a22af85f0289aee01834bc412f) )
	ROM_LOAD16_BYTE( "arab09bd.p1", 0x000000, 0x080000, CRC(5820d7e3) SHA1(cc675c7e5a76a74f74c8ff46d9f1b584d9e246cd) )
	ROM_LOAD16_BYTE( "arab09d.p1", 0x000000, 0x080000, CRC(7d5f8028) SHA1(9faa7355ad791f62e4651a2141506a715a9fb80b) )
	ROM_LOAD16_BYTE( "arab09dy.p1", 0x000000, 0x080000, CRC(1b408a4f) SHA1(7400173679b76ea63c525b653862ff4a28668978) )
	ROM_LOAD16_BYTE( "arab09h.p1", 0x000000, 0x080000, CRC(e9964bc8) SHA1(c70b8ab78dda2ed6ed6286445df9700067b0b506) )
	ROM_LOAD16_BYTE( "arab09r.p1", 0x000000, 0x080000, CRC(a6e31e5c) SHA1(9afc5c2deda057a51b53a9c80b4f0e2a752bf92a) )
	ROM_LOAD16_BYTE( "arab09s.p1", 0x000000, 0x080000, CRC(e5a44317) SHA1(1c11c1a6348fa29980afc1d8521056529002feae) )
	ROM_LOAD16_BYTE( "arab09y.p1", 0x000000, 0x080000, CRC(83bb4970) SHA1(82d611e594583c25a62ab54f65c86e193744bf1c) )
ROM_END

ROM_START( m5arab03 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "arab03f.p1", 0x000000, 0x080000, CRC(76766338) SHA1(b2db342426f0d184f20b2f352c452445ae4abefc) )
	ROM_LOAD16_BYTE( "arab03.p2", 0x000001, 0x080000, NO_DUMP )
	/* 3+4 */
ROM_END


ROM_START( m5atlan )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "atlsjh1.4d1", 0x000000, 0x080000, CRC(ba888aac) SHA1(e1d92214dd78abdc8e55cd7376a81f9b914619bd) )
	ROM_LOAD16_BYTE( "atlsja1.4_2", 0x000001, 0x080000, CRC(7f974373) SHA1(068e50f1c5bab121dfe16d4851c6a8ea30edcb0c) )
	ROM_LOAD16_BYTE( "atlsja1.4_3", 0x100000, 0x080000, CRC(bd4054bb) SHA1(83bd71e26c6e8e34a9e97eaa2b67c8a38d55159f) )
	ROM_LOAD16_BYTE( "atlsja1.4_4", 0x100001, 0x080000, CRC(95615a4f) SHA1(db5945e360ca65b8bbd3a17905ea9f195071b02e) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "atlsjk1.4_1", 0x000000, 0x080000, CRC(95a6181e) SHA1(6de28e8bbbe45bd6bbee5516be306c9bf00086c7) )
	ROM_LOAD16_BYTE( "atlsjs1.4_1", 0x000000, 0x080000, CRC(26696133) SHA1(63f870e2b77a0fc372e5c8f7df65d3a7ef08398f) )
	ROM_LOAD16_BYTE( "atlsjs1.4d1", 0x000000, 0x080000, CRC(0947f381) SHA1(c1eeafe1fa08a2b72f0968acac57fdfc31abb764) )
ROM_END

ROM_START( m5atlana )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "atl_sjs1.2_1", 0x000000, 0x080000, CRC(eec370e0) SHA1(160c07d6ce1cab68809a8c17d82a73b421384c98) )
	ROM_LOAD16_BYTE( "atl_sja1.2_2", 0x000001, 0x080000, CRC(98e82423) SHA1(1c8db8ffe6afbb3074cfbd68713c303e095f77b2) )
	ROM_LOAD16_BYTE( "atl_sja1.2_3", 0x100000, 0x080000, CRC(bd4054bb) SHA1(83bd71e26c6e8e34a9e97eaa2b67c8a38d55159f) )
	ROM_LOAD16_BYTE( "atl_sja1.2_4", 0x100001, 0x080000, CRC(95615a4f) SHA1(db5945e360ca65b8bbd3a17905ea9f195071b02e) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "atl_sjh1.2d1", 0x000000, 0x080000, CRC(7eb816e4) SHA1(18ce9463643fde976c16c4e9c1f178343fbf14a0) )
	ROM_LOAD16_BYTE( "atl_sjk1.2_1", 0x000000, 0x080000, CRC(3bc99546) SHA1(f9349006f364ca668f28a586ddf2200bfefa994a) )
	ROM_LOAD16_BYTE( "atl_sjs1.2d1", 0x000000, 0x080000, CRC(abb2f342) SHA1(4ade9f4d8702f64f93fac019d1211d9abff7512a) )
ROM_END

ROM_START( m5austin )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "apow02ad.p1", 0x000000, 0x080000, CRC(c0206bb5) SHA1(d80e7b942485f335542260acf3b62ec7a814f663) )
	ROM_LOAD16_BYTE( "apow02.p2", 0x000001, 0x080000, CRC(f19744c2) SHA1(80b34f730edd60314609f6e8b2a869831054ffd3) )
	ROM_LOAD16_BYTE( "apow02.p3", 0x100000, 0x080000, CRC(3fa7ef72) SHA1(2625cca97e7d5a81f3d7e28136ec0b54c2f01b1e) )
	ROM_LOAD16_BYTE( "apow02.p4", 0x100001, 0x080000, CRC(f18232d0) SHA1(cd5f843a227dd7fef95ecc47619e34e8ea829fc1) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "apow02b.p1", 0x000000, 0x080000, CRC(ff2cdde4) SHA1(98fe6a1674da9067d0bf2eb8417fd5dc024f0993) )
	ROM_LOAD16_BYTE( "apow02d.p1", 0x000000, 0x080000, CRC(61006045) SHA1(a45c7607c4be8d999117f139adbf5b2c21c481cf) )
	ROM_LOAD16_BYTE( "apow02dy.p1", 0x000000, 0x080000, CRC(9ac73b39) SHA1(99880d633f3b2272f802c8765ef5caafee796fe6) )
	ROM_LOAD16_BYTE( "apow02h.p1", 0x000000, 0x080000, CRC(2c2ce3d5) SHA1(97a3dd569f7b278447858a2d8cab3d6558dc607f) )
	ROM_LOAD16_BYTE( "apow02k.p1", 0x000000, 0x080000, CRC(e2d2379d) SHA1(f20c602da8a0ad19065d0d7ef1f112ce008318df) )
	ROM_LOAD16_BYTE( "apow02r.p1", 0x000000, 0x080000, CRC(686de837) SHA1(5fa20eb90e900ddd1119368b27316bfd52ca3dcb) )
	ROM_LOAD16_BYTE( "apow02s.p1", 0x000000, 0x080000, CRC(d833bf08) SHA1(c83475f7e5d7c98f2230fdfe63c03c5054ffe6e7) )
	ROM_LOAD16_BYTE( "apow02y.p1", 0x000000, 0x080000, CRC(23f4e474) SHA1(dc4f0d879a4afed9a4f821ea67dda25be637a8a3) )
ROM_END

ROM_START( m5austin10 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "aptr10s.p1", 0x000000, 0x080000, CRC(67ffc878) SHA1(2a4264f4ed0c9287b713c8538142addd0c7ac204) )
	ROM_LOAD16_BYTE( "aptr10.p2",  0x000001, 0x080000, CRC(17be7fea) SHA1(32166f27f7bb080a21ead236a77660404293e411) )
	ROM_LOAD16_BYTE( "aptr10.p3",  0x100000, 0x080000, CRC(63a03a48) SHA1(511ae14cf9e4801c73c1e1044b6d44e9e16cd9cf) )
	ROM_LOAD16_BYTE( "aptr10.p4",  0x100001, 0x080000, CRC(725118c1) SHA1(ef92f39d973b97c13c222d7ee1e3d33500b504c4) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "aptr10ad.p1", 0x000000, 0x080000, CRC(973c8882) SHA1(2a8d69be338a1dd9aceac84c32544a690be49ac6) )
	ROM_LOAD16_BYTE( "aptr10b.p1", 0x000000, 0x080000, CRC(c69e9090) SHA1(52f9c768c9715846777a2d1520bb26217a046f09) )
	ROM_LOAD16_BYTE( "aptr10bd.p1", 0x000000, 0x080000, CRC(cee9fc6f) SHA1(b7fae924200457b9b8f241534581510b40792d3a) )
	ROM_LOAD16_BYTE( "aptr10d.p1", 0x000000, 0x080000, CRC(a221368e) SHA1(30921d5ceeee2be1ba54b097e8c4a4e584840d5c) )
	ROM_LOAD16_BYTE( "aptr10dy.p1", 0x000000, 0x080000, CRC(9183336b) SHA1(e2465efa90ec7375f46873a4bbd1f3419356bfe1) )
	ROM_LOAD16_BYTE( "aptr10k.p1", 0x000000, 0x080000, CRC(f9332320) SHA1(38a88601a0f902475dcecc59a362d52752c83c49) )
	ROM_LOAD16_BYTE( "aptr10r.p1", 0x000000, 0x080000, CRC(c976e2f8) SHA1(190eb27cf12966a090f4fad1aca5227a753f3488) )
	ROM_LOAD16_BYTE( "aptr10y.p1", 0x000000, 0x080000, CRC(545dcd9d) SHA1(a3006ee5f1bc0c75773d9c511cfaf268969af79b) )
ROM_END

ROM_START( m5austin11 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "aptr11s.p1", 0x000000, 0x080000, CRC(9f56e817) SHA1(9ff30c0b9633c277965fe3b11db05b535fa50a64) )
	ROM_LOAD16_BYTE( "aptr11.p2",  0x000001, 0x080000, CRC(f5efd4f8) SHA1(aaedc8bc087a4eb10981da32b0a158b9b995e59b) )
	/* 3+4? */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "aptr11ad.p1", 0x000000, 0x080000, CRC(949ed53f) SHA1(08d09d3aac777ac5933a87fdc0d6b9cf2f440b07) )
	ROM_LOAD16_BYTE( "aptr11b.p1", 0x000000, 0x080000, CRC(15da74bc) SHA1(8a38dc44b7cafa1c3319a4e230d46fd7610ade28) )
	ROM_LOAD16_BYTE( "aptr11bd.p1", 0x000000, 0x080000, CRC(c85c839f) SHA1(e810370cfc25a3704550d2040fe1249f577c8e77) )
	ROM_LOAD16_BYTE( "aptr11d.p1", 0x000000, 0x080000, CRC(4f03e87d) SHA1(6a44cfb050d8124da2ba545b0b88fe1d273b41a3) )
	ROM_LOAD16_BYTE( "aptr11dy.p1", 0x000000, 0x080000, CRC(aa4ebdf0) SHA1(cab8fd43bd211cb3bd344ca8786772e9a1018820) )
	ROM_LOAD16_BYTE( "aptr11k.p1", 0x000000, 0x080000, CRC(dfa67a55) SHA1(7e7acc0cc3a0733abf28928adc7fdb20ef1edbfa) )
	ROM_LOAD16_BYTE( "aptr11r.p1", 0x000000, 0x080000, CRC(63f5f938) SHA1(6c0911d12c20d45ba163538dfa5c15b791861302) )
	ROM_LOAD16_BYTE( "aptr11y.p1", 0x000000, 0x080000, CRC(7a1bbd9a) SHA1(bb5b9716c4a81f2bbd07621bea46a67ca320d1ec) )
ROM_END


ROM_START( m5bttf )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bfe_12s.p1", 0x000000, 0x080000, CRC(05191f0d) SHA1(bdec5a9042465d3b8874ff6f07b13b133a4022ec) )
	ROM_LOAD16_BYTE( "bfe_12l.p2", 0x000001, 0x080000, CRC(c020b40f) SHA1(4f2c6773fbf76aa71772ffeb2dfc52462f71d02c) )
	ROM_LOAD16_BYTE( "bfe_12l.p3", 0x100000, 0x080000, CRC(7362219a) SHA1(de06b10d0e76376a50c3943be96ed13a19c051c0) )
	ROM_LOAD16_BYTE( "bfe_12l.p4", 0x100001, 0x080000, CRC(ff5b3bd0) SHA1(3f02f4c8b440d7ad1ec10170c9b3b2de87aa98f5) )
ROM_END

ROM_START( m5bttfa )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "b_tofeat.p1", 0x00000, 0x080000, CRC(c203dd09) SHA1(141c65e3b00e2ac8188fffb8899b61384a3b4786) )
	ROM_LOAD16_BYTE( "b_tofeat.p2", 0x00001, 0x080000, CRC(989fa928) SHA1(daee2fd9bed569de385b8beee51225e45610db52) )
	/* 3+4 */
ROM_END


ROM_START( m5barkng )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bark06ad.p1", 0x000000, 0x080000, CRC(bff3bc1c) SHA1(1ea30f909d3bff312966820158f5595d26d76d86) )
	ROM_LOAD16_BYTE( "bark06.p2", 0x000001, 0x080000, CRC(a90b94ec) SHA1(ac8ff9cf0cebeab9592aee932fbe60e5f4b83598) )
	ROM_LOAD16_BYTE( "bark06.p3", 0x100000, 0x080000, CRC(28e2bfb4) SHA1(c91e10fcdeed5cb1aad45b7b0e51c1bf89cb2e89) )
	ROM_LOAD16_BYTE( "bark06.p4", 0x100001, 0x080000, CRC(6fc91491) SHA1(38ce47a6539fff9587d99adf75a8a374ffbe0cdd) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "bark06b.p1", 0x000000, 0x080000, CRC(74650db0) SHA1(e2d966337068f9b3767e937f0968bde31a3451ef) )
	ROM_LOAD16_BYTE( "bark06d.p1", 0x000000, 0x080000, CRC(6b18462a) SHA1(d45a4ff43a125f6f943c1eb2d291f4dd4af9ccad) )
	ROM_LOAD16_BYTE( "bark06dy.p1", 0x000000, 0x080000, CRC(7c779f47) SHA1(cd3ad79e76cecb666ad09382106a5d81da2b2874) )
	ROM_LOAD16_BYTE( "bark06r.p1", 0x000000, 0x080000, CRC(324097b4) SHA1(126a69c92ab767cc34a36ef7fce29cff4e706c1c) )
	ROM_LOAD16_BYTE( "bark06y.p1", 0x000000, 0x080000, CRC(20f81596) SHA1(6165e28e3c9bd9418ecc0a245cbe42245495021a) )
ROM_END

ROM_START( m5baxe )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "batr01d.p1", 0x000000, 0x080000, CRC(a4ee3c3d) SHA1(b8122ba9a6815dc20dc34e0d82605938b5785ca4) )
	ROM_LOAD16_BYTE( "batr01.p2", 0x000001, 0x080000, CRC(582da32d) SHA1(0b323c9c1dd3b58aa2e9a99287b9f395cd104901) )
	ROM_LOAD16_BYTE( "batr01.p3", 0x100000, 0x080000, CRC(c92aeb52) SHA1(cbf8d2b80c28c6f535fc94eafa7a579db664e259) )
	ROM_LOAD16_BYTE( "batr01.p4", 0x100001, 0x080000, CRC(3c53f077) SHA1(20ac6ff9277f66103c735e64b6d805bda622e524) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "batr01dy.p1", 0x000000, 0x080000, CRC(2ec9249b) SHA1(2acb44ec90a06c606fa4eeb677207efd8f49ecfc) )
	ROM_LOAD16_BYTE( "batr01s.p1", 0x000000, 0x080000, CRC(54e041b8) SHA1(d50f2ff641bbb2d7d9d8540b35aa51f6d996762e) )
	ROM_LOAD16_BYTE( "batr01y.p1", 0x000000, 0x080000, CRC(dec7591e) SHA1(fb346672ae9e7bad35f42e2a519f2933d46b0755) )
ROM_END

ROM_START( m5baxe04 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "baxe04s.p1", 0x000000, 0x080000, CRC(5968eb24) SHA1(47ce83070cc3c182913a1c96ecb09dd4a771e9a2) )
	ROM_LOAD16_BYTE( "baxe04.p2", 0x000001, 0x080000, CRC(3928c2ff) SHA1(b5cf4cb8e579f91492e2eec6adb86ada0ceb9091) )
	ROM_LOAD16_BYTE( "baxe04.p3", 0x100000, 0x080000, CRC(c92aeb52) SHA1(cbf8d2b80c28c6f535fc94eafa7a579db664e259) ) // == 01
	ROM_LOAD16_BYTE( "baxe04.p4", 0x100001, 0x080000, CRC(3c53f077) SHA1(20ac6ff9277f66103c735e64b6d805bda622e524) ) // == 01

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "baxe04ad.p1", 0x000000, 0x080000, CRC(fb065844) SHA1(5487da7bfc436cab1080ab7554913c8c6a9ea58f) )
	ROM_LOAD16_BYTE( "baxe04b.p1", 0x000000, 0x080000, CRC(f543ecae) SHA1(bacbb635df90ac950d97f5b76990c300e0b92230) )
	ROM_LOAD16_BYTE( "baxe04bd.p1", 0x000000, 0x080000, CRC(3435d26e) SHA1(143896c2060fef6577aa502347205413436b9426) )
	ROM_LOAD16_BYTE( "baxe04d.p1", 0x000000, 0x080000, CRC(981ed5e4) SHA1(616ef751b1a29020e3627ceceac92d74dd9bdfbc) )
	ROM_LOAD16_BYTE( "baxe04dy.p1", 0x000000, 0x080000, CRC(39f2bf48) SHA1(9d52f08704e0024170772f6abf2cf0410b45090f) )
	ROM_LOAD16_BYTE( "baxe04h.p1", 0x000000, 0x080000, CRC(3bd18eba) SHA1(568ec19ca59a307ecb013b1d8a909424524d96bc) )
	ROM_LOAD16_BYTE( "baxe04k.p1", 0x000000, 0x080000, CRC(6c837386) SHA1(820c0fee3d89b7e554d99ef82eff2e589cac6e42) )
	ROM_LOAD16_BYTE( "baxe04r.p1", 0x000000, 0x080000, CRC(e9f3189f) SHA1(bdc7a9f3673e4fa48c927d59e0e2e0eee4e391a8) )
	ROM_LOAD16_BYTE( "baxe04y.p1", 0x000000, 0x080000, CRC(f8848188) SHA1(571ca96688afe4cf02e17e1fdb48c6b11bec5eef) )
ROM_END


ROM_START( m5bbro )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bbro10ad.p1", 0x000000, 0x080000, CRC(e042f003) SHA1(7ae9c3632d15932a4a53e493cddb29e0c1f98397) )
	ROM_LOAD16_BYTE( "bbro10.p2", 0x000001, 0x080000, CRC(468f8325) SHA1(0d7461b88865854b9775fbdaa3cb0c97908dae4b) )
	ROM_LOAD16_BYTE( "bbro10.p3", 0x100000, 0x080000, CRC(6c2e9086) SHA1(856ed523b09356dd25249ad93d2433c7565fac7c) )
	ROM_LOAD16_BYTE( "bbro10.p4", 0x100001, 0x080000, CRC(a0f9ff38) SHA1(e9f38c546e6a194e9c72ab5089bdf9b9c6e0c5e8) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "bbro10b.p1", 0x000000, 0x080000, CRC(3b151856) SHA1(c6c994ccd901a685bfc0bdea274d77c782b94641) )
	ROM_LOAD16_BYTE( "bbro10bd.p1", 0x000000, 0x080000, CRC(24f0885c) SHA1(d686c172f854f756f6119b06e28bb4d9a922ba15) )
	ROM_LOAD16_BYTE( "bbro10d.p1", 0x000000, 0x080000, CRC(ce4cb7d0) SHA1(1dcce21ada6748ee6e0efdca11c92e79b3379751) )
	ROM_LOAD16_BYTE( "bbro10dy.p1", 0x000000, 0x080000, CRC(5d373321) SHA1(5c6806c49638d87c0c0f16919a62c0af2c5be885) )
	ROM_LOAD16_BYTE( "bbro10h.p1", 0x000000, 0x080000, CRC(c4dff431) SHA1(a9ddf8e2afa361840dd5504f80ae4eb1103d6926) )
	ROM_LOAD16_BYTE( "bbro10k.p1", 0x000000, 0x080000, CRC(23c063ef) SHA1(4994dc3529a7925088c9cb6fe84118be386048af) )
	ROM_LOAD16_BYTE( "bbro10r.p1", 0x000000, 0x080000, CRC(3ad22855) SHA1(d43fc543dd1da4d5ec1b9c666a8ac8d6a110c6b3) )
	ROM_LOAD16_BYTE( "bbro10s.p1", 0x000000, 0x080000, CRC(d1a927da) SHA1(8dd917856006453b8c633ac98b003754b733c44d) )
	ROM_LOAD16_BYTE( "bbro10y.p1", 0x000000, 0x080000, CRC(42d2a32b) SHA1(8c94b98701a8bcdb633f2b70c189c1c4e4a53e4e) )
ROM_END


ROM_START( m5bbro02 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bbtr02.p1", 0x000000, 0x080000, CRC(829b5cf0) SHA1(4df802e54e2c0685a63335e9429156a7c81824a1) )
	ROM_LOAD16_BYTE( "bbtr02.p2", 0x000001, 0x080000, CRC(1fc52d87) SHA1(12147409de0420323c998c58eec3c3c3d326063e) )
	ROM_LOAD16_BYTE( "bbtr02.p3", 0x100000, 0x080000, CRC(6c2e9086) SHA1(856ed523b09356dd25249ad93d2433c7565fac7c) ) // == bbro10.p3
	ROM_LOAD16_BYTE( "bbtr02.p4", 0x100001, 0x080000, CRC(a0f9ff38) SHA1(e9f38c546e6a194e9c72ab5089bdf9b9c6e0c5e8) ) // == bbro10.p4

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "bbtr02ad.p1", 0x000000, 0x080000, CRC(27a37ec6) SHA1(5122603bc3c9ce4cd2627bc5229dd8becefcaadf) )
	ROM_LOAD16_BYTE( "bbtr02b.p1", 0x000000, 0x080000, CRC(6d173df4) SHA1(8fe471472b2fb01ec4d2ce7ea2984328a43d59fb) )
	ROM_LOAD16_BYTE( "bbtr02bd.p1", 0x000000, 0x080000, CRC(ea11d4db) SHA1(0c7bc7bcf22f1040e9c03060c30e7513d88e5b49) )
	ROM_LOAD16_BYTE( "bbtr02d.p1", 0x000000, 0x080000, CRC(059db5df) SHA1(51422cb1de471a0c085215bf871792a986ef3b1f) )
	ROM_LOAD16_BYTE( "bbtr02dy.p1", 0x000000, 0x080000, CRC(e26bf5ae) SHA1(4caa6b2339f1732eb28c713b5e957c5f519d99db) )
	ROM_LOAD16_BYTE( "bbtr02k.p1", 0x000000, 0x080000, CRC(d76a7323) SHA1(0d72bee24484df41ccf234debe077aa3da2540e0) )
	ROM_LOAD16_BYTE( "bbtr02r.p1", 0x000000, 0x080000, CRC(4a2b53cf) SHA1(2b41a6350f4a784655ca6233d7275f61b36a1cc5) )
	ROM_LOAD16_BYTE( "bbtr02s.p1", 0x000000, 0x080000, CRC(829b5cf0) SHA1(4df802e54e2c0685a63335e9429156a7c81824a1) )
	ROM_LOAD16_BYTE( "bbtr02y.p1", 0x000000, 0x080000, CRC(656d1c81) SHA1(964a15a8e68ad85737239b4aad7a67ab2a9ab950) )
ROM_END


ROM_START( m5bbrocl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bbrc02.p1", 0x000000, 0x080000, CRC(239f6008) SHA1(0c54e36fa861181cd6c1bda1c5e5e4d5e9569fa8) )
	ROM_LOAD16_BYTE( "bbrc02.p2", 0x000001, 0x080000, CRC(5c809548) SHA1(76ff6267dfd8b43f390ba961547469709747e12b) )
	ROM_LOAD16_BYTE( "bbrc02.p3", 0x100000, 0x080000, CRC(d98c7a3e) SHA1(be90c3ba614e472e75470052135082b5e1d4b7a4) )
	ROM_LOAD16_BYTE( "bbrc02.p4", 0x100001, 0x080000, CRC(107250a5) SHA1(b15d288aeee78a208af411f53c2a1e8d9dc8539b) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "bbrc02d.p1", 0x000000, 0x080000, CRC(079e667c) SHA1(9becdbd8835bb2fc12944b31c31551a4ff21d19f) )
	ROM_LOAD16_BYTE( "bbrc02f.p1", 0x000000, 0x080000, CRC(e963a531) SHA1(9be5ebcc2682c0af3d861b5f3649a18278e8ca91) )
	//ROM_LOAD16_BYTE( "bbrc02s.p1", 0x000000, 0x080000, CRC(239f6008) SHA1(0c54e36fa861181cd6c1bda1c5e5e4d5e9569fa8) )
ROM_END


ROM_START( m5bigchs )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bigc04ad.p1", 0x000000, 0x080000, CRC(35649d13) SHA1(7ddd795d84f552a7ba16d1665e266ea6a495ed4c) )
	ROM_LOAD16_BYTE( "bigc04.p2", 0x000001, 0x080000, CRC(221807c3) SHA1(0d3c26facdb0a95b34843834a052505f944f3036) )
	ROM_LOAD16_BYTE( "bigc04.p3", 0x100000, 0x080000, CRC(fc932b1a) SHA1(4050c4e6abd8e7105592c8042a2f7a37ca7093c5) )
	ROM_LOAD16_BYTE( "bigc04.p4", 0x100001, 0x080000, CRC(4dbfe06c) SHA1(4b5cd1a94ce69b6ffb7132991e0fc6c439242a11) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "bigc04b.p1", 0x000000, 0x080000, CRC(a919116a) SHA1(283eabdbbd911b1b4398f4d14cf7a0c1c65e855c) )
	ROM_LOAD16_BYTE( "bigc04bd.p1", 0x000000, 0x080000, CRC(601c3751) SHA1(6d33dd7ed8ea81527d1543d1f8dc649b9f12e919) )
	ROM_LOAD16_BYTE( "bigc04d.p1", 0x000000, 0x080000, CRC(9f7760f6) SHA1(6dd4cd94f359e3da879b4ee68935cb4397e8d768) )
	ROM_LOAD16_BYTE( "bigc04dy.p1", 0x000000, 0x080000, CRC(ae9d48b1) SHA1(68fe5867b113d569b83abeacac6cdf644a655d7e) )
	ROM_LOAD16_BYTE( "bigc04k.p1", 0x000000, 0x080000, CRC(08156ee9) SHA1(93f6994a5d12a2a415a5e1fd0a2fa4e109161854) )
	ROM_LOAD16_BYTE( "bigc04r.p1", 0x000000, 0x080000, CRC(a2bd3459) SHA1(00807aef865ea48bcd962ec9cfede42d240faabb) )
	ROM_LOAD16_BYTE( "bigc04s.p1", 0x000000, 0x080000, CRC(8245e0fa) SHA1(ba0f8c6a74e2ace35c491887166ddba7684fb772) )
	ROM_LOAD16_BYTE( "bigc04y.p1", 0x000000, 0x080000, CRC(b3afc8bd) SHA1(9f491bcce024a3924c5d81ee1e677c581d1cf1b9) )
ROM_END

ROM_START( m5bigchs05 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bigc05s.p1", 0x000000, 0x080000, CRC(aa3f4da7) SHA1(562ed3fa08f2e9c7ac3db684312f16711f1faa36) )
	ROM_LOAD16_BYTE( "bigc05.p2", 0x000001, 0x080000, CRC(f31083d7) SHA1(c0e5af819450d797077c88a346621898c4775992) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "bigc05ad.p1", 0x000000, 0x080000, CRC(4fc927b9) SHA1(1f8542a325fd93c432e2f663992a45489a59bd58) )
	ROM_LOAD16_BYTE( "bigc05b.p1", 0x000000, 0x080000, CRC(f26355be) SHA1(bdfda45a1b91db1409c25250601038e87c11d842) )
	ROM_LOAD16_BYTE( "bigc05bd.p1", 0x000000, 0x080000, CRC(f132489d) SHA1(71c7d584e6a33cb6177fd7e0b242aa24a91c1c11) )
	ROM_LOAD16_BYTE( "bigc05d.p1", 0x000000, 0x080000, CRC(a943668a) SHA1(05440bf5dca60ed233447adad3ffcdf55b9f6056) )
	ROM_LOAD16_BYTE( "bigc05dy.p1", 0x000000, 0x080000, CRC(7edeadf2) SHA1(97acc070daa863584ea192cb8b8fca662225545a) )
	ROM_LOAD16_BYTE( "bigc05k.p1", 0x000000, 0x080000, CRC(e61041fe) SHA1(528473466ac17ec0d23d11fe8b76ccc274fb3329) )
	ROM_LOAD16_BYTE( "bigc05r.p1", 0x000000, 0x080000, CRC(fdd8b979) SHA1(bb18f6d371a1bdc36a20db29bf13dd26c86ba4f5) )
	ROM_LOAD16_BYTE( "bigc05y.p1", 0x000000, 0x080000, CRC(7da286df) SHA1(eefe95f0dcea45b0769c04408f901ed181bc5a71) )
ROM_END


ROM_START( m5biggam )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bigg08ad.p1", 0x000000, 0x080000, CRC(6506bc44) SHA1(288fb897b20d75cd294e6c6cc5f5d9f3e2a26e7a) )
	ROM_LOAD16_BYTE( "bigg08.p2", 0x000001, 0x080000, CRC(dfa872b2) SHA1(6e1ecd3c912c31369542891e5d5ac1ef37055e45) )
	ROM_LOAD16_BYTE( "bigg08.p3", 0x100000, 0x080000, CRC(0c0e1e99) SHA1(158820e40fa23447c850f41fe56a83f95dbce80f) )
	ROM_LOAD16_BYTE( "bigg08.p4", 0x100001, 0x080000, CRC(237b94c9) SHA1(28603c73f3ecb8606527691ff9df66e6c446f64c) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "bigg08b.p1", 0x000000, 0x080000, CRC(cb23fba1) SHA1(a2a20a48c89aa6ac37ed78feab4674a30e6b7162) )
	ROM_LOAD16_BYTE( "bigg08bd.p1", 0x000000, 0x080000, CRC(b4c5c265) SHA1(7641442888294f6b227e78ee4dc8b168f6851e4e) )
	ROM_LOAD16_BYTE( "bigg08d.p1", 0x000000, 0x080000, CRC(575e8211) SHA1(a656b783b795836241a890033c0fba7d76efc5da) )
	ROM_LOAD16_BYTE( "bigg08dy.p1", 0x000000, 0x080000, CRC(00af1c36) SHA1(3379aba6ba23c4675ac69e75a1df0b78f0a5cc0e) )
	ROM_LOAD16_BYTE( "bigg08h.p1", 0x000000, 0x080000, CRC(9fcae653) SHA1(c5e2c4329221373ae8a2e151664ff93b048361b4) )
	ROM_LOAD16_BYTE( "bigg08r.p1", 0x000000, 0x080000, CRC(40f5229d) SHA1(40408d272b2a39f3c52db0cad3e209655760e105) )
	ROM_LOAD16_BYTE( "bigg08s.p1", 0x000000, 0x080000, CRC(d1868ce2) SHA1(9b25f817e71c8e08342849f17d19e503985eb88b) )
	ROM_LOAD16_BYTE( "bigg08y.p1", 0x000000, 0x080000, CRC(867712c5) SHA1(20a6938dc61c14d064013960e9f5ab6612feb90b) )
ROM_END

ROM_START( m5biggam11 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bigg11s.p1", 0x000000, 0x080000, CRC(11d470c7) SHA1(50b4d826ab278cc1fa2d4a7b362d8cfe7e50281a) )
	ROM_LOAD16_BYTE( "bigg11.p2", 0x000000, 0x080000, CRC(377668bd) SHA1(55c4cf61170be112b6e49c21ffe4f3aacf0d1618) )
	ROM_LOAD16_BYTE( "bigg11.p3", 0x000000, 0x080000, CRC(0c0e1e99) SHA1(158820e40fa23447c850f41fe56a83f95dbce80f) ) // == 08
	ROM_LOAD16_BYTE( "bigg11.p4", 0x000000, 0x080000, CRC(237b94c9) SHA1(28603c73f3ecb8606527691ff9df66e6c446f64c) ) // == 08

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "bigg11ad.p1", 0x000000, 0x080000, CRC(6746440d) SHA1(2e1c06dcdaf6179019a11ea76eb29a5c6b27e09e) )
	ROM_LOAD16_BYTE( "bigg11b.p1", 0x000000, 0x080000, CRC(ea5d86c9) SHA1(deb8b265fd8e30b2700963a4a4fbdeb53e39747e) )
	ROM_LOAD16_BYTE( "bigg11bd.p1", 0x000000, 0x080000, CRC(7c126080) SHA1(920c2ebe1450622e918f4d510d5b919b5751a91a) )
	ROM_LOAD16_BYTE( "bigg11d.p1", 0x000000, 0x080000, CRC(4dff9580) SHA1(4b49639734390777fc1aaaea67e42c8dbc7052bd) )
	ROM_LOAD16_BYTE( "bigg11dy.p1", 0x000000, 0x080000, CRC(2125bec1) SHA1(9fa60d215a8dac4c25f1bcc11ad5f58421c59c20) )
	ROM_LOAD16_BYTE( "bigg11h.p1", 0x000000, 0x080000, CRC(46ee059c) SHA1(9b4073c0a1eda999375348b419197e6fb0369e3e) )
	ROM_LOAD16_BYTE( "bigg11r.p1", 0x000000, 0x080000, CRC(c58fe72c) SHA1(604c9984299643465b8571d4ad3f655a27838911) )
	ROM_LOAD16_BYTE( "bigg11y.p1", 0x000000, 0x080000, CRC(7d0e5b86) SHA1(bb8fd44f72dbb07461baef6c9bedcdd690e83909) )
ROM_END


ROM_START( m5bigsht )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bsa10s.p1", 0x000000, 0x080000, CRC(71f10226) SHA1(98028cd28250c78b72afeb833cde4161410ce15e) )
	ROM_LOAD16_BYTE( "bsa10s.p2", 0x000001, 0x080000, CRC(113dc4d8) SHA1(37cf67973e50d481fd0d885842ac983d430d7eb4) )
	ROM_LOAD16_BYTE( "bsa10s.p3", 0x100000, 0x080000, CRC(bbb9538a) SHA1(18feaf2d2bf4be9c3632ee210fdd2b8f1be676b1) )
	ROM_LOAD16_BYTE( "bsa10s.p4", 0x100001, 0x080000, CRC(18c1956b) SHA1(8c496f83609e281605377a191c35f379db53983a) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "bsa10d.p1", 0x000000, 0x080000, CRC(f719522a) SHA1(74e3d0b86edc22c3b931c73d4edd78a65dcaa516) )
	ROM_LOAD16_BYTE( "bsa10k.p1", 0x000000, 0x080000, CRC(553393ca) SHA1(5c0c1fec3ff23ceff24c3c27d168640fe1bb755b) )
ROM_END

ROM_START( m5bigsht04 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bsa04s.p1", 0x000000, 0x080000, CRC(b0d53f31) SHA1(f51db7c1341cd1d3e26f5409571c48a4ea91b918) )
	ROM_LOAD16_BYTE( "bsa04s.p2", 0x000001, 0x080000, CRC(a635b46f) SHA1(e750f75366b1a3153f8872f3a9b4549b29a7d671) )
	ROM_LOAD16_BYTE( "bsa04s.p3", 0x100000, 0x080000, CRC(bbb9538a) SHA1(18feaf2d2bf4be9c3632ee210fdd2b8f1be676b1) ) // == 10
	ROM_LOAD16_BYTE( "bsa04s.p4", 0x100001, 0x080000, CRC(18c1956b) SHA1(8c496f83609e281605377a191c35f379db53983a) ) // == 10

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "bsa04d.p1", 0x000000, 0x080000, CRC(363d6f3d) SHA1(33c1aea12f450c4489b0b1e7347f52ba724fee01) )
	ROM_LOAD16_BYTE( "bsa04k.p1", 0x000000, 0x080000, CRC(9417aedd) SHA1(902bec2307f26c98e186236589998217dfe50f73) )
ROM_END

ROM_START( m5bigsht11 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bsa11s.p1", 0x000000, 0x080000, CRC(9e42a5eb) SHA1(292a6dddd046504e4f6ecccdf53472d95298663b) )
	ROM_LOAD16_BYTE( "bsa11s.p2", 0x000001, 0x080000, CRC(72a36915) SHA1(3635e42ca3ac8913a306bd86e3d3d3dbdd32450e) )
	ROM_LOAD16_BYTE( "bsa11s.p3", 0x100000, 0x080000, CRC(bbb9538a) SHA1(18feaf2d2bf4be9c3632ee210fdd2b8f1be676b1) ) // == 10
	ROM_LOAD16_BYTE( "bsa11s.p4", 0x100001, 0x080000, CRC(18c1956b) SHA1(8c496f83609e281605377a191c35f379db53983a) ) // == 10

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "bsa11d.p1", 0x000000, 0x080000, CRC(18aaf5e7) SHA1(eb963f9cac3b19633062a42d673f1595765cfa76) )
	ROM_LOAD16_BYTE( "bsa11k.p1", 0x000000, 0x080000, CRC(ba803407) SHA1(966ba90ee6e1b7bc512142ca26d11094e879d5f2) )
ROM_END

ROM_START( m5bigsht13 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bsh13s.p1", 0x000000, 0x080000, CRC(9ffcd5e4) SHA1(58a4ffe87cac83be2ae78e2b4bc02b5b43274acc) )
	ROM_LOAD16_BYTE( "bsh13s.p2", 0x000001, 0x080000, CRC(f5ce2d0a) SHA1(979f17d3faedf25943c197625219c877ff51ab64) )
	ROM_LOAD16_BYTE( "bsh13s.p3", 0x100000, 0x080000, CRC(78776cec) SHA1(7e0e8cfc650043fd84949df89ecc7a422c2a765e) )
	ROM_LOAD16_BYTE( "bsh13s.p4", 0x100001, 0x080000, CRC(14bdc563) SHA1(391db5fa27fec4b8916aa2387c2c71fb7e09ae10) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "bsh13d.p1", 0x000000, 0x080000, CRC(191485e8) SHA1(a8528f89a2aabb65f9543b15b0332bbfdb0b1bb7) )
	ROM_LOAD16_BYTE( "bsh13f.p1", 0x000000, 0x080000, CRC(bb3e4408) SHA1(572b6f22502f21f7483f6db09f0df3dd0016bd48) )
	ROM_LOAD16_BYTE( "bsh13l.p1", 0x000000, 0x080000, CRC(be46c1e7) SHA1(9d2c268ca306ad741bc941b0b544e627f85d3bf6) )
	ROM_LOAD16_BYTE( "bsh13m.p1", 0x000000, 0x080000, CRC(d679f63c) SHA1(e3fb7bcd25cb936443b039c51e9af8677a9ff1dd) )
	ROM_LOAD16_BYTE( "bsh13o.p1", 0x000000, 0x080000, CRC(62995cc5) SHA1(0de8096c247e3550c8147d339fb64cef9a875d19) )
ROM_END

ROM_START( m5bigshta )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "b shot d.p1", 0x00000, 0x080000, CRC(c82e29f5) SHA1(91ce304240a4cda0284950372e3e91430ed52883) )
	ROM_LOAD16_BYTE( "b shot d.p2", 0x00001, 0x080000, CRC(ffd1a5ea) SHA1(54fb030a323ba8eaf9782a679819f47047df4ebb) )
	/* 3+4 */
ROM_END

ROM_START( m5blkwht )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "blac16s.p1", 0x000000, 0x080000, CRC(f00436e5) SHA1(8bc994b68ba527b0b217b7f7e4ecd12de8b375e0) )
	ROM_LOAD16_BYTE( "blac16.p2", 0x000001, 0x080000, CRC(56e18eee) SHA1(b6b799f25f308f89259060c0bfa73a9754eb4431) )
	ROM_LOAD16_BYTE( "blac16.p3", 0x100000, 0x080000, CRC(af9daa46) SHA1(bd84bd4f9719b1e4400f50c19f89675c92805a4d) )
	ROM_LOAD16_BYTE( "blac16.p4", 0x100001, 0x080000, CRC(88b3555c) SHA1(72edd22dc011c2e22b2dc0d204aeea053ae20a29) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "blac16b.p1", 0x000000, 0x080000, CRC(88d453a7) SHA1(18714a95dd3abd439effa45589788f3bd3bdc4c9) )
	ROM_LOAD16_BYTE( "blac16d.p1", 0x000000, 0x080000, CRC(43728990) SHA1(9b45516d15c3da8be7b878d5c4dd1d5a019ef3d5) )
	ROM_LOAD16_BYTE( "blac16dy.p1", 0x000000, 0x080000, CRC(c0daae91) SHA1(b6daf50568208821126f202437a19d6f7610d78a) )
	ROM_LOAD16_BYTE( "blac16r.p1", 0x000000, 0x080000, CRC(8f35c063) SHA1(1f108a39032ce1d55b519a71ad420e0586a769b0) )
	ROM_LOAD16_BYTE( "blac16y.p1", 0x000000, 0x080000, CRC(73ac11e4) SHA1(53e5254020ca1f39426606d916625c575753fc14) )
ROM_END

ROM_START( m5blkwht11 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "blac11s.p1", 0x000000, 0x080000, CRC(1608179d) SHA1(bb6c06e2b4a024a5aad192c94c7041af669cd7e5) )
	ROM_LOAD16_BYTE( "blac11.p2", 0x000001, 0x080000, CRC(0ab3dff7) SHA1(f2cc56800ad3773aca10c237af35b1ecab29d501) )
	/* 3+4? */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "blac11b.p1", 0x000000, 0x080000, CRC(17f7151b) SHA1(567929615b3db99c58e16db4cfec8c88490b9aea) )
	ROM_LOAD16_BYTE( "blac11bd.p1", 0x000000, 0x080000, CRC(6eda3f86) SHA1(5e0504a32d1df573975b8c6f1f1991e86f002c14) )
	ROM_LOAD16_BYTE( "blac11d.p1", 0x000000, 0x080000, CRC(075b147f) SHA1(5e51da6e6aa681e64389ebc01401200632a7fc65) )
	ROM_LOAD16_BYTE( "blac11dy.p1", 0x000000, 0x080000, CRC(897e3b09) SHA1(6d9da0c82d939bea97bac75f3a4c954ccdb8afa3) )
	ROM_LOAD16_BYTE( "blac11r.p1", 0x000000, 0x080000, CRC(8dd19956) SHA1(6e5d84810c782a63f9ed239669005032bdad0989) )
	ROM_LOAD16_BYTE( "blac11y.p1", 0x000000, 0x080000, CRC(bce066ff) SHA1(963e7b053b95d546a79260a9595ddb474c3da57d) )
ROM_END

ROM_START( m5blkwht01 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "blte01s.p1", 0x000000, 0x080000, CRC(62ea3b88) SHA1(d2c6079c6e59716f8c011b18c592afb57e8e354f) )
	ROM_LOAD16_BYTE( "blte01.p2", 0x000001, 0x080000, CRC(fa4d98a1) SHA1(208f7415575ed30bd5e31829e4068abceb9ef042) )
	/* 3+4? */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "blte01ad.p1", 0x000000, 0x080000, CRC(2a099b13) SHA1(e400a102f2f00dc9fd04c3838892bcc2e9fd91bf) )
	ROM_LOAD16_BYTE( "blte01b.p1", 0x000000, 0x080000, CRC(2d06509c) SHA1(7aa82739603c0d3cb33c65a43abb0978bb48fbec) )
	ROM_LOAD16_BYTE( "blte01bd.p1", 0x000000, 0x080000, CRC(cd116e14) SHA1(95c3b18f7e7b653b50908e27f1c9d7891e181ef2) )
	ROM_LOAD16_BYTE( "blte01d.p1", 0x000000, 0x080000, CRC(cc33b744) SHA1(2709009508355302abc0a5e16f9c227696280d01) )
	ROM_LOAD16_BYTE( "blte01dy.p1", 0x000000, 0x080000, CRC(44d9db03) SHA1(ec6826de06d591b3f0afb45340a6c8fe7281e3e8) )
	ROM_LOAD16_BYTE( "blte01r.p1", 0x000000, 0x080000, CRC(5864b896) SHA1(bf560234c3db517f1024fccfd002f79affb8163a) )
	ROM_LOAD16_BYTE( "blte01y.p1", 0x000000, 0x080000, CRC(ea0057cf) SHA1(f2301c46dfbbba5737554b20d3368344501c4bf6) )
ROM_END


ROM_START( m5bnzclb )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cbon1_0.p1", 0x000000, 0x080000, CRC(e82e50cf) SHA1(cc83ecb0a798d0ecb627379dea9ae97fac69037f) )
	ROM_LOAD16_BYTE( "cbon1_0.p2", 0x000001, 0x080000, CRC(07a3ed9b) SHA1(bfc04422b479a1a1bfdcf41b8f3431fcb07d4321) )
	ROM_LOAD16_BYTE( "cbon1_0.p3", 0x100000, 0x080000, CRC(0d340237) SHA1(b5c34905cb2698b9752326de6082954a91480cdc) )
	ROM_LOAD16_BYTE( "cbon1_0.p4", 0x100001, 0x080000, CRC(0be7ed51) SHA1(108944005cb1f75d45a5ce2e8e8139a3f2f067d7) )
ROM_END

ROM_START( m5bnzclb11 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cbon1_1.p1", 0x000000, 0x080000, CRC(4bc79ea8) SHA1(64ff6b7e5504c6d8cf216603dd721b9a09ca5eaa) )
	ROM_LOAD16_BYTE( "cbon1_1.p2", 0x000001, 0x080000, CRC(7a9787f7) SHA1(715771d4411ed36efde5dfddf1d1171af9db0902) )
	/* 3+4 */
ROM_END


ROM_START( m5bnkrs )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bon10s.p1", 0x000000, 0x080000, CRC(75511be9) SHA1(8d36dff96f255639270d6165a2c8c38ff7e591ac) )
	ROM_LOAD16_BYTE( "bon10s.p2", 0x000001, 0x080000, CRC(f33faef4) SHA1(dc1ddddf649bfa807e12c6cc4760c2b41c3e5454) )
	ROM_LOAD16_BYTE( "bon10s.p3", 0x100000, 0x080000, CRC(7a120a48) SHA1(1c1785a20d138966ed681e22f68ecb320656f944) )
	ROM_LOAD16_BYTE( "bon10s.p4", 0x100001, 0x080000, CRC(509e4400) SHA1(010dddffdac1740a97334c879a631971ce0fe180) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "bon10d.p1", 0x000000, 0x080000, CRC(f3b94be5) SHA1(56cbdc86399ed516a4c8181c6549d957892aa548) )
	ROM_LOAD16_BYTE( "bon10k.p1", 0x000000, 0x080000, CRC(51938a05) SHA1(2c4b4e6ae891850929f6a6c0281f3e9226a29845) )
ROM_END


ROM_START( m5btlbnk )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bba_20s.p1", 0x000000, 0x080000, CRC(7253dda6) SHA1(483aee9e3a99c7039dfd69d6b6da888dfa7fbab1) )
	ROM_LOAD16_BYTE( "bba_20l.p2", 0x000001, 0x080000, CRC(334308a6) SHA1(adaa0d8fa9877802cdc382d4a5575707a189a15e) )
	ROM_LOAD16_BYTE( "bba_20l.p3", 0x100000, 0x080000, CRC(c8178132) SHA1(025c7aacc61d7922a78ed69040001f3d920e4e2e) )
	ROM_LOAD16_BYTE( "bba_20l.p4", 0x100001, 0x080000, CRC(5dbdf9ed) SHA1(f24e4d31896f75e3eba6770e3e43a931420f15d1) )
ROM_END


ROM_START( m5bwaves )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "brwv09ad.p1", 0x000000, 0x080000, CRC(764d5d30) SHA1(a9d424dc5c4abfe8dbab90c5df327ceb2b7b830b) )
	ROM_LOAD16_BYTE( "brwv09.p2", 0x000001, 0x080000, CRC(8ff84321) SHA1(dac103411e1291d84b65c9cfc3dc5694c5e62aa5) )
	ROM_LOAD16_BYTE( "brwv09.p3", 0x100000, 0x080000, CRC(1572fe7e) SHA1(47b9584288c90fb4d1466a81d0ff04d6d2410ead) )
	ROM_LOAD16_BYTE( "brwv09.p4", 0x100001, 0x080000, CRC(f1ab42d9) SHA1(3ca5bccfcbff4ef321eac877a0792b030b9f6c7f) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "brwv09b.p1", 0x000000, 0x080000, CRC(58cb45f9) SHA1(cce19e43b56c30984994bbde2f8f8c8a928f2696) )
	ROM_LOAD16_BYTE( "brwv09bd.p1", 0x000000, 0x080000, CRC(ec9f7ec8) SHA1(1cb05fc9e25519bceac74a9f2a7fc5dd783df426) )
	ROM_LOAD16_BYTE( "brwv09d.p1", 0x000000, 0x080000, CRC(5f15aaaa) SHA1(32bbe252480738d69c82913be6e85d4f5da5f542) )
	ROM_LOAD16_BYTE( "brwv09dy.p1", 0x000000, 0x080000, CRC(e9298749) SHA1(bc23b2e0cf3e0d10b1e8e54ea7a5557aa8dc2c49) )
	ROM_LOAD16_BYTE( "brwv09h.p1", 0x000000, 0x080000, CRC(1ca4484f) SHA1(bc2deb93001f9555ce18f82229fbaff279a7334d) )
	ROM_LOAD16_BYTE( "brwv09r.p1", 0x000000, 0x080000, CRC(7dc8644a) SHA1(997e6322a187f94e650dd7c222d7e1a31fcffada) )
	ROM_LOAD16_BYTE( "brwv09s.p1", 0x000000, 0x080000, CRC(eb41919b) SHA1(5ca0e59969404c4d3f9d5f64bca8466a3ddab7e3) )
	ROM_LOAD16_BYTE( "brwv09y.p1", 0x000000, 0x080000, CRC(5d7dbc78) SHA1(03e26c888ad2c45d2a28f58f1b0e3b48a4585645) )
ROM_END

ROM_START( m5bwaves07 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "brwv07f",   0x000000, 0x080000, CRC(3e01280c) SHA1(37ec9872580c4a15c68cc5ca7b0d59e06752dea7) )
	ROM_LOAD16_BYTE( "brwv07.p2", 0x000001, 0x080000, CRC(256fd369) SHA1(68164c41f91f94c9e6ac4ec56d1f44811b324aaa) )
	ROM_LOAD16_BYTE( "brwv07.p3", 0x100000, 0x080000, CRC(265ca09e) SHA1(bd4a11964bc0db67cba594095c32ad701d75e33f) )
	ROM_LOAD16_BYTE( "brwv07.p4", 0x100001, 0x080000, CRC(1f7170f6) SHA1(94987f79c58fd938ddb5d17dbfb1d8cd414922ab) )
ROM_END



ROM_START( m5bbank )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "btb12s.p1", 0x000000, 0x080000, CRC(3bc2fa49) SHA1(786c505b41fa45615de33fe69a9d9cec95320750) )
	ROM_LOAD16_BYTE( "btb12s.p2", 0x000001, 0x080000, CRC(fcf64e04) SHA1(f415709e8b572ded428daa0905b23dc070dcf9c0) )
	ROM_LOAD16_BYTE( "btb12s.p3", 0x100000, 0x080000, CRC(c5420655) SHA1(f74cf3b80a74da35e1fc27f23dad87d8625e6fbe) )
	ROM_LOAD16_BYTE( "btb12s.p4", 0x100001, 0x080000, CRC(5f03b803) SHA1(a2ab914faa7227c9ce9f11b3a88feb0aacb74580) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "btb12d.p3", 0x000000, 0x080000, CRC(01b02f4e) SHA1(ca4ad7b2e244dbe91130373b8597d708ea0336d1) )
	ROM_LOAD16_BYTE( "btb12e.p3", 0x000000, 0x080000, CRC(0b71779e) SHA1(eb0730b7ccf7d3afd7626cad0fc96453b288fb02) )
	ROM_LOAD16_BYTE( "btb12k.p3", 0x000000, 0x080000, CRC(8255e382) SHA1(161bdeb22526c1baddb77adfca5cd62cabe20378) )
ROM_END

ROM_START( m5bbank13 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "btb13s.p1", 0x000000, 0x080000, CRC(77eb1f78) SHA1(cb41930662c0d234bcb2b162416e78134be8cc68) )
	ROM_LOAD16_BYTE( "btb13s.p2", 0x000001, 0x080000, CRC(805bbd24) SHA1(0fcaa8cbc3048e1bb46db1427880d720d44dd093) )
	ROM_LOAD16_BYTE( "btb13s.p3", 0x100000, 0x080000, CRC(f003c9d2) SHA1(f91052010aecaa18ab501ff9b9783d9afbb41df5) )
	ROM_LOAD16_BYTE( "btb13s.p4", 0x100001, 0x080000, CRC(8f92c534) SHA1(4a28c6f687c017e8109867c5476860c3706d0aeb) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "btb13d.p1", 0x000000, 0x080000, CRC(f1034f74) SHA1(2f81d62d902ebcbc1304989919c198be9155669d) )
	ROM_LOAD16_BYTE( "btb13k.p1", 0x000000, 0x080000, CRC(53298e94) SHA1(5d718c79c533c3ea06d0383caabc0bff8f3c0e59) )
ROM_END


ROM_START( m5cbw )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cbwa02ad.p1", 0x000000, 0x080000, CRC(c768e364) SHA1(85145f569d9b537e3dc1cf17f64593952a34dd08) )
	ROM_LOAD16_BYTE( "cbwa02.p2", 0x000001, 0x080000, CRC(af82f5f0) SHA1(8a7c10b831e9adcf8494df1e4e5fbf34bb953538) )
	ROM_LOAD16_BYTE( "cbwa02.p3", 0x100000, 0x080000, CRC(827f6ea8) SHA1(12906f157d2199f96d4f7ee35c79cfc5047fa6c9) )
	ROM_LOAD16_BYTE( "cbwa02.p4", 0x100001, 0x080000, CRC(4d57417e) SHA1(95d820641e9b2eee658eba3dfe6e3584384d2ad9) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "cbwa02b.p1", 0x000000, 0x080000, CRC(d210f59a) SHA1(c7529c47abd080a6a39be54d89ee07dda3c2fe0a) )
	ROM_LOAD16_BYTE( "cbwa02bd.p1", 0x000000, 0x080000, CRC(aebe0487) SHA1(7781c6a11fc04bdf156ca588bc0be712d70185c4) )
	ROM_LOAD16_BYTE( "cbwa02d.p1", 0x000000, 0x080000, CRC(c58dcf1c) SHA1(a288826726cecc6ee5730baaa5877a12af4b1f34) )
	ROM_LOAD16_BYTE( "cbwa02dy.p1", 0x000000, 0x080000, CRC(a268adbe) SHA1(8e627807810e3a076583ca18243736d41e6f36d6) )
	ROM_LOAD16_BYTE( "cbwa02k.p1", 0x000000, 0x080000, CRC(d8e290ae) SHA1(9a96d5f6677315d6415fc8a211ad9f74b07059ad) )
	ROM_LOAD16_BYTE( "cbwa02r.p1", 0x000000, 0x080000, CRC(542e5441) SHA1(86196119638c93e40b9e18e74fcd872992c529ff) )
	ROM_LOAD16_BYTE( "cbwa02s.p1", 0x000000, 0x080000, CRC(2a1e3ef9) SHA1(da96cc24b12fe326b6ea9887a0ea37d5d35f6a08) )
	ROM_LOAD16_BYTE( "cbwa02y.p1", 0x000000, 0x080000, CRC(4dfb5c5b) SHA1(b6de6ed5d00b7ca493a1cd76ae4d246f2c13c8d7) )
ROM_END

ROM_START( m5cbwa )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "c_b_wall.p1", 0x00000, 0x080000, CRC(6dcc8428) SHA1(f25b6b1bb41dcd125a6ced2350e515ef83487948) )
	ROM_LOAD16_BYTE( "c_b_wall.p2", 0x00001, 0x080000, CRC(71e2d919) SHA1(37338e062232f8bb1fac0d69cdcdcadf5fa79534) )
	/* 3+4? */
ROM_END


ROM_START( m5cbrun )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cbal0_1.p1", 0x000000, 0x080000, CRC(1df9477c) SHA1(5b45bc9bf6e7b30715d7b876a31937df782e0e83) )
	ROM_LOAD16_BYTE( "cbal0_1.p2", 0x000001, 0x080000, CRC(4749aecf) SHA1(551f94c04a5dcf2e6c07908c50f9c20acde64e1a) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "cbal0_1d.p1", 0x000000, 0x080000, CRC(67ffdac1) SHA1(6617555c5945d074dd629e153a5d84ae1eebaec8) )
ROM_END


ROM_START( m5cpcash )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "capt03ad.p1", 0x000000, 0x080000, CRC(2a5e6b7d) SHA1(a1e01ed10d873c0f3cc83667496b90a2712e9b40) )
	ROM_LOAD16_BYTE( "capt03.p2", 0x000001, 0x080000, CRC(17dd42d1) SHA1(d38bd6d76afad07a85f3810476676e64ec50f0d3) )
	ROM_LOAD16_BYTE( "capt03.p3", 0x100000, 0x080000, CRC(047fd5b0) SHA1(1642d23f8c6437b14d89a0e65c06426dfbb0371f) )
	ROM_LOAD16_BYTE( "capt03.p4", 0x100001, 0x080000, CRC(2dad791d) SHA1(721ecdc83a79bbe1a6427731d7cb6169019d39fa) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "capt03b.p1", 0x000000, 0x080000, CRC(379bedb1) SHA1(5ff25ef06da48fd13701c5b424af1862b6c84a6c) )
	ROM_LOAD16_BYTE( "capt03bd.p1", 0x000000, 0x080000, CRC(884f8396) SHA1(97be787a9460829f85cfb66d92edab87db0ae749) )
	ROM_LOAD16_BYTE( "capt03d.p1", 0x000000, 0x080000, CRC(dd1379ba) SHA1(605eb129413326e78397b46c2370360ed1f723a4) )
	ROM_LOAD16_BYTE( "capt03dy.p1", 0x000000, 0x080000, CRC(1c878fb5) SHA1(86f7db1b42c45b7e93816a8edcf7a305a2cae121) )
	ROM_LOAD16_BYTE( "capt03h.p1", 0x000000, 0x080000, CRC(8384a233) SHA1(79856f70dec5c422c969650acc3417d7524aac7e) )
	ROM_LOAD16_BYTE( "capt03r.p1", 0x000000, 0x080000, CRC(1f1b82bb) SHA1(82a32768547720ae7e34ea83ab7f05f7527d87c9) )
	ROM_LOAD16_BYTE( "capt03s.p1", 0x000000, 0x080000, CRC(fea5d89b) SHA1(59ed5650c5833c790f7dc3dc65dd96fbfb878c71) )
	ROM_LOAD16_BYTE( "capt03y.p1", 0x000000, 0x080000, CRC(3f312e94) SHA1(3e9e508ce64ffe3fb3307b1d47e362cc6d125d7f) )
ROM_END

ROM_START( m5cshkcb )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "csc____1.1_1", 0x000000, 0x080000, CRC(75e746e6) SHA1(3776b221acbad5360fa344498c941db396ae5b17) )
	ROM_LOAD16_BYTE( "csc____1.1_2", 0x000001, 0x080000, CRC(2d330f46) SHA1(fc960de70bb85c5e85c78e5c109acdb12974daf7) )
	ROM_LOAD16_BYTE( "csc____1.1_3", 0x100000, 0x080000, CRC(dcbe62c6) SHA1(b98ada7c2745c04581666ae64248183d2a4d01ba) )
	ROM_LOAD16_BYTE( "csc____1.1_4", 0x100001, 0x080000, CRC(ca2fdd38) SHA1(962d9d44b33c06470916984f1873542bb8d1b790) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "csc___d1.1_1", 0x000000, 0x080000, CRC(9a4dbc41) SHA1(d49db41f5ac743be2190d9fd0fe073edb73946be) )
ROM_END

ROM_START( m5cshkcb12 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "csc____1.2_1", 0x000000, 0x080000, CRC(bd137211) SHA1(bde355c613b757a7241c2b9e44e25fe345b4c8f9) )
	ROM_LOAD16_BYTE( "csc____1.2_2", 0x000001, 0x080000, CRC(5a2a342c) SHA1(4678a6a47b815729cd82921a5a6533a021dcdd6a) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "csc___d1.2_1", 0x000000, 0x080000, CRC(4214ee0b) SHA1(2dadf830fdf2e0648e68e4a91835b9d613ee8c03) )
ROM_END


ROM_START( m5cshkcb13 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "csc____1.3_1", 0x000000, 0x080000, CRC(5c70939b) SHA1(051981d3ca01265f27cbc0d8c467748f4fa84b78) )
	ROM_LOAD16_BYTE( "csc____1.3_2", 0x000001, 0x080000, CRC(bc0acb8d) SHA1(e3f89490b6a2a39a8c4707e40c110e9fa2c06d3d) )
	ROM_LOAD16_BYTE( "csc____1.3_3", 0x100000, 0x080000, CRC(1751905a) SHA1(5a06d4528c823bd4f6f3d68ae100d414008fbe02) )
	ROM_LOAD16_BYTE( "csc____1.3_4", 0x100001, 0x080000, CRC(384cdc37) SHA1(d37ef78aa38de8a010f0bbf369a5b10c1fb6b6bb) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "csc___d1.3_1", 0x000000, 0x080000, CRC(5e0a3a44) SHA1(608d4d0372f83e8ec5d3944ecea4d42c005c5bb1) )
ROM_END

ROM_START( m5carclb )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "crbc02s.p1", 0x000000, 0x080000, CRC(949ac336) SHA1(2da3f1f2c180ef2d9d2125f34cb2b02c0ad6cfa0) )
	ROM_LOAD16_BYTE( "crbc02s.p2", 0x000001, 0x080000, CRC(b7de3ab8) SHA1(baa695c65b87bc9543dd39a75e9fef7632c88645) )
	ROM_LOAD16_BYTE( "crbc02s.p3", 0x100000, 0x080000, CRC(3e14ac22) SHA1(86ab231a5334a4a77517300b595c253499b9f92b) )
	ROM_LOAD16_BYTE( "crbc02s.p4", 0x100001, 0x080000, CRC(f775dde0) SHA1(bad521459eae5e3f15877ba02e3c5d4a9326cf86) )
	ROM_LOAD16_BYTE( "crbc02s.p5", 0x200000, 0x080000, CRC(4e265feb) SHA1(4702c8f10262f170f8f66d24cf88d77c5eb2b8ae) )
	ROM_LOAD16_BYTE( "crbc02s.p6", 0x200001, 0x080000, CRC(a58b7781) SHA1(5ea6647c0726fdd8a1cabcaad00c56c5d5508dda) )
ROM_END


ROM_START( m5carou )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "csel0_2.p1", 0x000000, 0x080000, CRC(4d56795e) SHA1(705be381cc3aa0f9faa43ea8159299a309198ae7) )
	ROM_LOAD16_BYTE( "csel0_2.p2", 0x000001, 0x080000, CRC(e0619e7e) SHA1(4540f36a9b6cb5a4647e318f2fda268f71d212ea) )
	ROM_LOAD16_BYTE( "csel0_2.p3", 0x100000, 0x080000, CRC(f1e898fe) SHA1(69286c7f39ee495ad30468a83b86859c64cf2960) )
	ROM_LOAD16_BYTE( "csel0_2.p4", 0x100001, 0x080000, CRC(d93bea10) SHA1(bbab34780688f341d9baa2ebed40c5689af71227) )
ROM_END



ROM_START( m5cashar )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "carn03ad.p1", 0x000000, 0x080000, CRC(52204ff3) SHA1(1b96099f65243da5b3f2d99fc53810fbe95ce318) )
	ROM_LOAD16_BYTE( "carn03.p2", 0x000001, 0x080000, CRC(475423cc) SHA1(3afbe72ec11aa5db7539f071fe965ee62cf14789) )
	ROM_LOAD16_BYTE( "carn03.p3", 0x100000, 0x080000, CRC(b4e632d4) SHA1(4bc3d04aa1884d50c5052b9e8b0069a243d3875b) )
	ROM_LOAD16_BYTE( "carn03.p4", 0x100001, 0x080000, CRC(691b53e5) SHA1(580f9ea8cc2ca5cb80da8aecd7d7a6b3ce5e158c) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "carn03b.p1", 0x000000, 0x080000, CRC(4a0a0fef) SHA1(7948ddc1ea95a4ad7ca92244a9cb9e8d919fc1b4) )
	ROM_LOAD16_BYTE( "carn03bd.p1", 0x000000, 0x080000, CRC(33badadf) SHA1(516c1a11b57a20783b9500be9b82176c62854b48) )
	ROM_LOAD16_BYTE( "carn03d.p1", 0x000000, 0x080000, CRC(6d7a0e63) SHA1(033cc87d6c380296b5f236d8016321d2313147f0) )
	ROM_LOAD16_BYTE( "carn03dy.p1", 0x000000, 0x080000, CRC(6b69ddea) SHA1(d6eea2e5a613d63f77f9daf3fe91ebf87fe640ca) )
	ROM_LOAD16_BYTE( "carn03h.p1", 0x000000, 0x080000, CRC(6b97d5d0) SHA1(66455da92b54b2187417e2944a9401379b5effeb) )
	ROM_LOAD16_BYTE( "carn03r.p1", 0x000000, 0x080000, CRC(a272fa87) SHA1(a320a9074beadcd3267ee29ea075b1feefcc7c9f) )
	ROM_LOAD16_BYTE( "carn03s.p1", 0x000000, 0x080000, CRC(3bbce703) SHA1(9399b34f52079ded70d16572a292ba082de12dea) )
	ROM_LOAD16_BYTE( "carn03y.p1", 0x000000, 0x080000, CRC(3daf348a) SHA1(e6dff93e2f821f8cc7608185cbd3b034307fcc23) )
ROM_END

ROM_START( m5cashar04 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "carn04s.p1", 0x000000, 0x080000, CRC(45e49281) SHA1(683f9af9fbf96f5ff6dbc4a00c07044611a623bb) )
	ROM_LOAD16_BYTE( "carn04.p2", 0x000001, 0x080000, CRC(99aa52e2) SHA1(3b41d928b519001497e1dae17bde5fdc71f971a7) )
	ROM_LOAD16_BYTE( "carn04.p3", 0x100000, 0x080000, CRC(b4e632d4) SHA1(4bc3d04aa1884d50c5052b9e8b0069a243d3875b) ) // == 03
	ROM_LOAD16_BYTE( "carn04.p4", 0x100001, 0x080000, CRC(691b53e5) SHA1(580f9ea8cc2ca5cb80da8aecd7d7a6b3ce5e158c) ) // == 03

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "carn04ad.p1", 0x000000, 0x080000, CRC(aa4a14e5) SHA1(250941c658dce7077a0b5a65be464ff6307aabdf) )
	ROM_LOAD16_BYTE( "carn04b.p1", 0x000000, 0x080000, CRC(c9ced662) SHA1(2a0da3e37ad6d6d9c2401182c368256102c23800) )
	ROM_LOAD16_BYTE( "carn04bd.p1", 0x000000, 0x080000, CRC(6a8b7193) SHA1(95b38e8936c74a7223958cc155ad73d0f2b29c5e) )
	ROM_LOAD16_BYTE( "carn04d.p1", 0x000000, 0x080000, CRC(df152300) SHA1(2c1562b70aae62621e09e97661eb8da8e5e2a038) )
	ROM_LOAD16_BYTE( "carn04dy.p1", 0x000000, 0x080000, CRC(8261498f) SHA1(913d2997ac63755ec3d9f589dfccefa7bd9a5e61) )
	ROM_LOAD16_BYTE( "carn04h.p1", 0x000000, 0x080000, CRC(be167ce1) SHA1(a86aa6ce7ec3850ea6e59760eb9bffe4b276be76) )
	ROM_LOAD16_BYTE( "carn04r.p1", 0x000000, 0x080000, CRC(a26f1980) SHA1(a3c9547d84d552ee1e8a667e649f35337db044f1) )
	ROM_LOAD16_BYTE( "carn04y.p1", 0x000000, 0x080000, CRC(1890f80e) SHA1(1f33c9908a20c2848f898e5bb55bd0d7d441cd48) )
ROM_END

ROM_START( m5cashat )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cash03ad.p1", 0x000000, 0x080000, CRC(66196308) SHA1(d230606ccd0e520fd57b3b8b17ae488ac13d9a20) )
	ROM_LOAD16_BYTE( "cash03.p2", 0x000001, 0x080000, CRC(57ad9e82) SHA1(ceacd61dac05d1acd91e99c5bdd09121cc8a8d6d) )
	ROM_LOAD16_BYTE( "cash03.p3", 0x100000, 0x080000, CRC(52a2b946) SHA1(edf2974809e94fea8433b536c75fdb751265c1ad) )
	ROM_LOAD16_BYTE( "cash03.p4", 0x100001, 0x080000, CRC(93b76f13) SHA1(89744013b6bc69408d3add0b963da1ea5b87867e) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "cash03b.p1", 0x000000, 0x080000, CRC(aad00e23) SHA1(814f398a18260a2dbd7dd738d61b6311be522b1d) )
	ROM_LOAD16_BYTE( "cash03bd.p1", 0x000000, 0x080000, CRC(36b36cc9) SHA1(86cbac649c169a00f492238f8cff2c9063cce652) )
	ROM_LOAD16_BYTE( "cash03d.p1", 0x000000, 0x080000, CRC(c13027f0) SHA1(282925a680dbced5beef38ad903b508ebce607ff) )
	ROM_LOAD16_BYTE( "cash03dy.p1", 0x000000, 0x080000, CRC(ea5b983b) SHA1(dcab7a224c88c5fae961e566f54fcda7e1d302d6) )
	ROM_LOAD16_BYTE( "cash03k.p1", 0x000000, 0x080000, CRC(015cb76f) SHA1(2953f9c1312e208e6aaf36b4a9adfcbb90faf179) )
	ROM_LOAD16_BYTE( "cash03r.p1", 0x000000, 0x080000, CRC(7be38e8b) SHA1(44bf9eee9c05aa7a1e25e280fa7e4310134eb1fe) )
	ROM_LOAD16_BYTE( "cash03s.p1", 0x000000, 0x080000, CRC(7debb2cd) SHA1(2cece6a4c5a3abb3ace5781ff47a9590db9e134e) )
	ROM_LOAD16_BYTE( "cash03y.p1", 0x000000, 0x080000, CRC(56800d06) SHA1(db0ff553eb954aa9590a53c9ef42bc8ccf3974b1) )
ROM_END


ROM_START( m5cashln )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cshl03ad.p1", 0x000000, 0x080000, CRC(08413893) SHA1(5885a3439e63cdabeadadc4660e9d66f77d74d0a) )
	ROM_LOAD16_BYTE( "cshl03.p2", 0x000001, 0x080000, CRC(345b1b86) SHA1(62e26562dc5f306c6f308d7f0151e521381fac3b) )
	ROM_LOAD16_BYTE( "cshl03.p3", 0x100000, 0x080000, CRC(eb9d6674) SHA1(53463d2f3edbfd7b0c4a22e45ace539ab263c63b) )
	ROM_LOAD16_BYTE( "cshl03.p4", 0x100001, 0x080000, CRC(d199af69) SHA1(2da10944dfd3fd519cc50ed10ff6261165d02fd1) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "cshl03b.p1", 0x000000, 0x080000, CRC(3603c3ab) SHA1(0e9ad95ecc788e18e8ac8cfeb0dede149e89d049) )
	ROM_LOAD16_BYTE( "cshl03d.p1", 0x000000, 0x080000, CRC(d21cf7d0) SHA1(28c054c9f229ab35e95fb2d21906041ac63b6adb) )
	ROM_LOAD16_BYTE( "cshl03dy.p1", 0x000000, 0x080000, CRC(176bcdfa) SHA1(02a3bd33580e1c9f5b685ce148583aedadc4fd53) )
	ROM_LOAD16_BYTE( "cshl03r.p1", 0x000000, 0x080000, CRC(27a48645) SHA1(8d59f312127b16f38af8c7841b18680aee86dfb5) )
	ROM_LOAD16_BYTE( "cshl03s.p1", 0x000000, 0x080000, CRC(fb02165c) SHA1(91c0ed6825bb7893853b283761ef36045d572e84) )
	ROM_LOAD16_BYTE( "cshl03y.p1", 0x000000, 0x080000, CRC(3e752c76) SHA1(dde56f51349c059df23b70a46a071216ca320c0c) )
ROM_END

ROM_START( m5cashrn )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "c2tr01d.p1", 0x000000, 0x080000, CRC(a9e26d09) SHA1(97a7e205b9d9d81de9c58f49f847679387ecd2b0) )
	ROM_LOAD16_BYTE( "c2tr01.p2", 0x000001, 0x080000, CRC(02174b55) SHA1(9a89ecc24a83a20d6722255083a5c2155afbd71b) )
	ROM_LOAD16_BYTE( "c2tr01.p3", 0x100000, 0x080000, CRC(c51a8eed) SHA1(ce629c8e503551d236709d9f5c05766db03dfa2f) )
	ROM_LOAD16_BYTE( "c2tr01.p4", 0x100001, 0x080000, CRC(23b59b4d) SHA1(7c19d14abebbbcbe05e3097fd926bbf941ce10ca) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "c2tr01dy.p1", 0x000000, 0x080000, CRC(6e4ddfd4) SHA1(0875d5ece7b2b24945e83de72ecec9ee9b9aa84e) )
	ROM_LOAD16_BYTE( "c2tr01k.p1", 0x000000, 0x080000, CRC(4e7bd7e7) SHA1(e6980cc3f4b3a7fceffe24fee519ba49a767bba4) )
	ROM_LOAD16_BYTE( "c2tr01s.p1", 0x000000, 0x080000, CRC(1239ddd9) SHA1(a70cc410df2e093b2bb60fc5119297f8976cd802) )
	ROM_LOAD16_BYTE( "c2tr01y.p1", 0x000000, 0x080000, CRC(405220ee) SHA1(bcaeba5c08acfe6c0f9ffeb9eafa25d631d9d104) )
ROM_END

ROM_START( m5cashrn01 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "catr01s.p1", 0x000000, 0x080000, CRC(044db3c6) SHA1(d81c20e72b8bf07ef45abbadb1116bdddb18ecaa) )
	ROM_LOAD16_BYTE( "catr01.p2", 0x000001, 0x080000, CRC(831be8b2) SHA1(02d85e9c65f327101b3f8435e154519568709852) )
	ROM_LOAD16_BYTE( "catr01.p3", 0x100000, 0x080000, CRC(c51a8eed) SHA1(ce629c8e503551d236709d9f5c05766db03dfa2f) ) // == 01
	ROM_LOAD16_BYTE( "catr01.p4", 0x100001, 0x080000, CRC(23b59b4d) SHA1(7c19d14abebbbcbe05e3097fd926bbf941ce10ca) ) // == 01

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "catr01d.p1", 0x000000, 0x080000, CRC(1d37c429) SHA1(9cdc41bb3bf04ff3639b23e1aae77b0898cbe048) )
	ROM_LOAD16_BYTE( "catr01dy.p1", 0x000000, 0x080000, CRC(c34a2763) SHA1(1e25136f8514ddb8f79615621d1edae4575ada59) )
	ROM_LOAD16_BYTE( "catr01k.p1", 0x000000, 0x080000, CRC(68cdc50c) SHA1(c02fa612a7fd6a8186091797901d123adbf85c9b) )
	ROM_LOAD16_BYTE( "catr01y.p1", 0x000000, 0x080000, CRC(72259374) SHA1(5a082f0454046d31d26a595a682d9800d181fa4a) )
ROM_END

ROM_START( m5cashrn02 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "car202.p1", 0x000000, 0x080000, CRC(a3a81ea5) SHA1(2259c5d034e4e5e955894731d9b9eee522d6ea15) )
	ROM_LOAD16_BYTE( "car202.p2", 0x000001, 0x080000, CRC(eebaa515) SHA1(3afdce9cfebb1cb0ab66ae21c2f7d7c74066aa58) )
	ROM_LOAD16_BYTE( "car202.p3", 0x100000, 0x080000, CRC(c51a8eed) SHA1(ce629c8e503551d236709d9f5c05766db03dfa2f) ) // == 01
	ROM_LOAD16_BYTE( "car202.p4", 0x100001, 0x080000, CRC(23b59b4d) SHA1(7c19d14abebbbcbe05e3097fd926bbf941ce10ca) ) // == 01

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "car202b.p1", 0x000000, 0x080000, CRC(ed59fdc9) SHA1(2705d6175e55768966c0206be70fd4bf84e7259a) )
	ROM_LOAD16_BYTE( "car202bd.p1", 0x000000, 0x080000, CRC(94ca6abe) SHA1(11745c55af73a1ff4f1886667fb520192d45601e) )
	ROM_LOAD16_BYTE( "car202d.p1", 0x000000, 0x080000, CRC(da3b89d2) SHA1(8f59d9b913ad4a2b50afaf87e86f184142a6bb26) )
	ROM_LOAD16_BYTE( "car202dy.p1", 0x000000, 0x080000, CRC(e5592e09) SHA1(848de1a6fae170f7936eac74ce444e188309eaae) )
	ROM_LOAD16_BYTE( "car202h.p1", 0x000000, 0x080000, CRC(465dcf86) SHA1(83fab2dc14b0b787b1d1be8e4178da88f54cd06e) )
	ROM_LOAD16_BYTE( "car202r.p1", 0x000000, 0x080000, CRC(1647926f) SHA1(1f856d5339f6e8f137332a8c0358adc7564a50b6) )
	ROM_LOAD16_BYTE( "car202s.p1", 0x000000, 0x080000, CRC(a3a81ea5) SHA1(2259c5d034e4e5e955894731d9b9eee522d6ea15) )
	ROM_LOAD16_BYTE( "car202y.p1", 0x000000, 0x080000, CRC(9ccab97e) SHA1(d280709ee78287a23eda48d3215296c611b732ac) )
ROM_END

ROM_START( m5cashrn04 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "caru04s.p1", 0x000000, 0x080000, CRC(dc1eecfb) SHA1(65e12f69b520edae7618db6730458e4998905198) )
	ROM_LOAD16_BYTE( "caru04.p2", 0x000001, 0x080000, CRC(c47ae830) SHA1(05d11acb3467b725111eba4b4058dcfecb7082e2) )
	ROM_LOAD16_BYTE( "caru04.p3", 0x100000, 0x080000, CRC(c51a8eed) SHA1(ce629c8e503551d236709d9f5c05766db03dfa2f) ) // == 01
	ROM_LOAD16_BYTE( "caru04.p4", 0x100001, 0x080000, CRC(23b59b4d) SHA1(7c19d14abebbbcbe05e3097fd926bbf941ce10ca) ) // == 01

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "caru04ad.p1", 0x000000, 0x080000, CRC(82cd5ac7) SHA1(eed24c58708e81d077c257290991f4da572ad476) )
	ROM_LOAD16_BYTE( "caru04b.p1", 0x000000, 0x080000, CRC(48ac1e8f) SHA1(41332abad02ead6a0a7622f079fad4406d9bce93) )
	ROM_LOAD16_BYTE( "caru04bd.p1", 0x000000, 0x080000, CRC(fb5805f0) SHA1(4bc0ff617b125e962da6bc6d67d6462dc25f7019) )
	ROM_LOAD16_BYTE( "caru04d.p1", 0x000000, 0x080000, CRC(6feaf784) SHA1(48ff5f14f658e53a644bcc48e858ff3a18afae7e) )
	ROM_LOAD16_BYTE( "caru04dy.p1", 0x000000, 0x080000, CRC(c6d4b1f1) SHA1(79739025149002705283136744208c1b22d94435) )
	ROM_LOAD16_BYTE( "caru04h.p1", 0x000000, 0x080000, CRC(249426ec) SHA1(320966bd3bfeb0e60cd2da8363e0f9acf250f265) )
	ROM_LOAD16_BYTE( "caru04r.p1", 0x000000, 0x080000, CRC(518bb2e2) SHA1(8c45e8953739e1a4abcee2eee9f380a4691605da) )
	ROM_LOAD16_BYTE( "caru04y.p1", 0x000000, 0x080000, CRC(7520aa8e) SHA1(79896fa6229def197906444ae1ab3f1f0b76a881) )
ROM_END


ROM_START( m5casroc )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "casr0_3.p1", 0x000000, 0x080000, CRC(79936fae) SHA1(825d8d7137e100dc4d7e8dc2ca7787f9588d71a9) )
	ROM_LOAD16_BYTE( "casr0_3.p2", 0x000001, 0x080000, CRC(61cdd56f) SHA1(06c7b85746bca1ddbcf254934cf04f8e2c7eeb92) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "casr0_3d.p1", 0x000000, 0x080000, CRC(1b0d1355) SHA1(1c2df191307e4a51c18abe7199b29525406ceeff) )
ROM_END

ROM_START( m5centcl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cent1_0.p1", 0x000000, 0x080000, CRC(8b94fa57) SHA1(e9694423e519f4768e19eaf6519e6a01da8c39ac) )
	ROM_LOAD16_BYTE( "cent1_0.p2", 0x000001, 0x080000, CRC(ef9b8283) SHA1(6b1171ae3ee63a488424b8906ad8089bdcd6a381) )
	ROM_LOAD16_BYTE( "cent1_0.p3", 0x100000, 0x080000, CRC(e573923b) SHA1(9d4f1e078f665fc2a58c3cf2075620632434f75d) )
	ROM_LOAD16_BYTE( "cent1_0.p4", 0x100001, 0x080000, CRC(a9f01838) SHA1(265992f6a26b6cfd0b50c36e5ecc807415b979dd) )
ROM_END

ROM_START( m5centcl20 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cent2_0.p1", 0x000000, 0x080000, CRC(7fe522b4) SHA1(0741b4a718f096162c1f9e4ba781e173497eebec) )
	ROM_LOAD16_BYTE( "cent2_0.p2", 0x000001, 0x080000, CRC(58814498) SHA1(608e2c2aa9aaa4e57eaae771d6745fe519f8caef) )
	/* 3+4 */
ROM_END

ROM_START( m5centcl21 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cent2_1.p1", 0x000000, 0x080000, CRC(bac9d8da) SHA1(663f11a95805c702a3781f64eef1619f982b0384) )
	ROM_LOAD16_BYTE( "cent2_1.p2", 0x000001, 0x080000, CRC(e1a89664) SHA1(e4237cd94ac3b2292bcadeb25d392deab601055e) )
	/* 3+4 */
ROM_END

ROM_START( m5centcl21a )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cent2_1d.p1", 0x000000, 0x080000, CRC(23880893) SHA1(6895d297242d0db2744b36ad82797e4032165909) )
	ROM_LOAD16_BYTE( "cent2_1d.p2", 0x000001, 0x080000, CRC(50c1865b) SHA1(e04a51fa587e707254716624c09a07a887663d0e) )
	/* 3+4 */
ROM_END


ROM_START( m5centcla )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cent_clu.p1", 0x00000, 0x080000, CRC(29f3fd66) SHA1(357dbe35c3221b0b400a1a9ab40ca17532f9fd51) )
	ROM_LOAD16_BYTE( "cent_clu.p2", 0x00001, 0x080000, CRC(c4b29c39) SHA1(0ee39207b4b9e3d06571404f7eabcf513dd1488c) )
	/* 3+4 */
ROM_END


ROM_START( m5clifhn )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "chasjk0.4_1", 0x000000, 0x080000, CRC(4f1922ac) SHA1(d0eb20da8a6e4552cc13f4f3db4eb0e8086cb467) )
	ROM_LOAD16_BYTE( "chasjk0.4_2", 0x000001, 0x080000, CRC(e884b66b) SHA1(26f8f42b868a2a6d2fda6f8d598a50ef8627fc74) )
	ROM_LOAD16_BYTE( "chasjk0.4_3", 0x100000, 0x080000, CRC(08988ee0) SHA1(152b2ffde609dd0f2ebcebd751742437e765aaa4) )
	ROM_LOAD16_BYTE( "chasjk0.4_4", 0x100001, 0x080000, CRC(54586b5c) SHA1(59757c24db94ecc6fbd818e143e7f002c9870f63) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "chasjs0.4_1", 0x000000, 0x080000, CRC(c694e721) SHA1(55695c5869dd11970449f6127d6feb06edf0aa32) )
	ROM_LOAD16_BYTE( "chasjs0.4d1", 0x000000, 0x080000, CRC(20f4b45a) SHA1(a80511580cc544bbc2e0523d22415b354db31d7f) )
ROM_END

ROM_START( m5cworan )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cloc0_1.p1", 0x000000, 0x080000, CRC(3686a8c5) SHA1(5162574fda90ec9665ba47a1f19a3be25396dac4) )
	ROM_LOAD16_BYTE( "cloc0_1.p2", 0x000001, 0x080000, CRC(1eb44cf4) SHA1(774d6ceb118d516cc2faba11578f58a510360064) )
	ROM_LOAD16_BYTE( "cloc0_1.p3", 0x100000, 0x080000, CRC(2f854915) SHA1(4bc30ccfa4dd5117b9e2f085140b8adaf94363e8) )
	ROM_LOAD16_BYTE( "cloc0_1.p4", 0x100001, 0x080000, CRC(97bc80eb) SHA1(f641cc60c49e72c6604bd1cea555e9658f3f8f84) )
ROM_END

ROM_START( m5cworan12 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cloc1_2.p1", 0x000000, 0x080000, CRC(9762c4d9) SHA1(2c764484ee0b3097d2b98e78d5be0c05f4322a30) )
	ROM_LOAD16_BYTE( "cloc1_2.p2", 0x000001, 0x080000, CRC(d8f87137) SHA1(1e4199eced9ec2488abd71d062cf7a93ea86977a) )
	/* 3+4 */
	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "cloc1_2d.p1", 0x000000, 0x080000, CRC(809dd7ca) SHA1(e95380f4cb9c6829006c695adf481e0ccd0c3631) )
ROM_END

ROM_START( m5clbtro )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "trop1_0.p1", 0x000000, 0x080000, CRC(d5dea6b1) SHA1(279ebef8ac5705ac0bce0d0ce9a8573e47b08d36) )
	ROM_LOAD16_BYTE( "trop1_0.p2", 0x000001, 0x080000, CRC(478045db) SHA1(25da8014902e5023fd7e6a7b4e92c106c0559e48) )
ROM_END

// 2_4 and 2_5 are either VERY similar, or there is a mistake here
ROM_START( m5clbtro24 ) // or is it 25?
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "trop2_4.p1", 0x000000, 0x080000, CRC(731f2cfe) SHA1(c0cecda77f6726631982c5342ec2740c86432594) )
	ROM_LOAD16_BYTE( "trop2_4.p2", 0x000001, 0x080000, CRC(c5f151f1) SHA1(4a6578e0b4afb46a4054f544fd996b5f3937bd24) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "trop2_4d.p1", 0x000000, 0x080000, CRC(a5eac7a4) SHA1(77fa99ae3a2ee7d4856a3d9918086a471e161a91) )
ROM_END

ROM_START( m5clbtro25 ) // or is it 24?
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "trop2_5.p1", 0x000000, 0x080000, CRC(731f2cfe) SHA1(c0cecda77f6726631982c5342ec2740c86432594) ) // == "trop2_4.p1 ??
	ROM_LOAD16_BYTE( "trop2_5.p2", 0x000001, 0x080000, CRC(665c2043) SHA1(32146abb066d50b901a110e8260c94ea4b9db4e9) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "trop2_5d.p1", 0x000000, 0x080000, CRC(a5eac7a4) SHA1(77fa99ae3a2ee7d4856a3d9918086a471e161a91) ) // == trop2_4d.p1 ??
ROM_END




ROM_START( m5cockdd )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cadd0_1.p1", 0x000000, 0x080000, CRC(d3ed1886) SHA1(4fd9fb263881ed1efb99fa75e9e35dc376ce7f32) )
	ROM_LOAD16_BYTE( "cadd0_1.p2", 0x000001, 0x080000, CRC(f471001c) SHA1(c6b38ccc7d77a104d3107a6861c21cb168598b77) )
	ROM_LOAD16_BYTE( "cadd0_1.p3", 0x100000, 0x080000, CRC(22092c41) SHA1(80dfc356d68811ba9f44926cb917568ca3f87cae) )
	ROM_LOAD16_BYTE( "cadd0_1.p4", 0x100001, 0x080000, CRC(36b6d2ca) SHA1(3b7618f383e3d866a5040d74d921aca3f9708b6d) )
ROM_END

ROM_START( m5cockdd05 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cadd0_5.p1", 0x000000, 0x080000, CRC(2da0d538) SHA1(37d664ed6fc84e7a7dfdd36e11792ecfe1f28570) )
	ROM_LOAD16_BYTE( "cadd0_5.p2", 0x000000, 0x080000, CRC(bed0c1c8) SHA1(5461cc0786ee84e3e20658a840f9c0322893224e) )
	/* 3+4 */
ROM_END


ROM_START( m5codft )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "codf06ad.p1", 0x000000, 0x080000, CRC(c41fb45a) SHA1(6bc91b4bd2e9cb372a2faafac574530620070601) )
	ROM_LOAD16_BYTE( "codf06.p2", 0x000001, 0x080000, CRC(58f41457) SHA1(66611d9d05168d40e753467e3b94a5332f8986e8) )
	ROM_LOAD16_BYTE( "codf06.p3", 0x100000, 0x080000, CRC(2fc0ed76) SHA1(6548767bc1c0d6135de3a60a0f6ce48098f7d137) )
	ROM_LOAD16_BYTE( "codf06.p4", 0x100001, 0x080000, CRC(fb5c5c16) SHA1(7c6346bb93933bd4b5a0cf73ba4eeb74d2b47812) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "codf06b.p1", 0x000000, 0x080000, CRC(01759a9c) SHA1(02078ab42d3a8255e87e1a0f915de93d61344855) )
	ROM_LOAD16_BYTE( "codf06d.p1", 0x000000, 0x080000, CRC(8069823b) SHA1(871896e0fe1cd0aa27237a9cc08b9af68580db7b) )
	ROM_LOAD16_BYTE( "codf06db.p1", 0x000000, 0x080000, CRC(b91fe9df) SHA1(b63ad599fee440e484799bf4248031f440b8cc63) )
	ROM_LOAD16_BYTE( "codf06dy.p1", 0x000000, 0x080000, CRC(ea10b6cd) SHA1(60738858e71aa45b0d4f3fb5235bfb51edeb42e7) )
	ROM_LOAD16_BYTE( "codf06h.p1", 0x000000, 0x080000, CRC(9f3ed508) SHA1(e4e13e814361508eb1f38d0ee99e4fc2c51ff889) )
	ROM_LOAD16_BYTE( "codf06k.p1", 0x000000, 0x080000, CRC(52259125) SHA1(a4ea69a7a38f864745909cc568a85defa65ff6b2) )
	ROM_LOAD16_BYTE( "codf06r.p1", 0x000000, 0x080000, CRC(f30d6a3f) SHA1(4404fe8c90f3efeeda4642a0f15eea1e5ffd6628) )
	ROM_LOAD16_BYTE( "codf06s.p1", 0x000000, 0x080000, CRC(b1e4e9ba) SHA1(a13fe3b12112df84030398dddee2f2fe95075497) )
	ROM_LOAD16_BYTE( "codf06y.p1", 0x000000, 0x080000, CRC(db9ddd4c) SHA1(18c0df196f0076895bbf1ab1ffaa77d43ee8b2ac) )
ROM_END

ROM_START( m5codft02 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cotr02s.p1", 0x000000, 0x080000, CRC(db2520d5) SHA1(53335c6506f2aaeed7ef29ef25259abf5c93a82a) )
	ROM_LOAD16_BYTE( "cotr02.p2", 0x000001, 0x080000, CRC(69cdb5b3) SHA1(399f8c6049de30aded5766514a40cedea043df3a) )
	ROM_LOAD16_BYTE( "cotr02.p3", 0x100000, 0x080000, CRC(2fc0ed76) SHA1(6548767bc1c0d6135de3a60a0f6ce48098f7d137) ) // == 06
	ROM_LOAD16_BYTE( "cotr02.p4", 0x100001, 0x080000, CRC(fb5c5c16) SHA1(7c6346bb93933bd4b5a0cf73ba4eeb74d2b47812) ) // == 06

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "cotr02d.p1", 0x000000, 0x080000, CRC(0a08db68) SHA1(dc31265d659acb53e1ce2b823501387c7d629ffe) )
	ROM_LOAD16_BYTE( "cotr02dy.p1", 0x000000, 0x080000, CRC(abcf7e6f) SHA1(cab844c757dd4ed4bed44fe371a3ffa9b54a42b3) )
	ROM_LOAD16_BYTE( "cotr02y.p1", 0x000000, 0x080000, CRC(7ae285d2) SHA1(e97cbf303dfee2890a797734e4eb6c5f0a3bb650) )
ROM_END


ROM_START( m5cnct4 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "con_sjs2.1_1", 0x000000, 0x080000, CRC(e92305d4) SHA1(2750d55d01c26c180c81869a3a83f5d8f1eedf47) )
	ROM_LOAD16_BYTE( "con_sjs2.1_2", 0x000001, 0x080000, CRC(a33722ee) SHA1(4eed78e616e9d364b62f4033f5e381b19eaba72b) )
	ROM_LOAD16_BYTE( "con_sjs2.1_3", 0x100000, 0x080000, CRC(e5f5a7ba) SHA1(bae96ab67e202a145678271dd7b0c9213e01a053) )
	ROM_LOAD16_BYTE( "con_sjs2.1_4", 0x100001, 0x080000, CRC(d5bf39ba) SHA1(fab5c25c62588e856fb82eb3bb2ce5d6d54d2b66) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	// all of these were paired with above, rv 2.1
	ROM_LOAD16_BYTE( "con_sja2.1_1", 0x000000, 0x080000, CRC(aec7d5ea) SHA1(8edac7e4020fd445bdaee0702fb66022dd5352c1) )
	ROM_LOAD16_BYTE( "con_sjk2.1_1", 0x000000, 0x080000, CRC(6f36f7f1) SHA1(3675fc169b2a38752bae49ef0810c6c415be007d) )
	ROM_LOAD16_BYTE( "con_sjh2.1d1", 0x000000, 0x080000, CRC(1a577436) SHA1(2bf85eed468c29be5e5ca63696cc60576d9c31c3) )
	ROM_LOAD16_BYTE( "con_sjl2.1d1", 0x000000, 0x080000, CRC(4c55a4ac) SHA1(5937752f900ea4cf8beb044b4774d0cb368729db) )
	ROM_LOAD16_BYTE( "con_sjs2.1d1", 0x000000, 0x080000, CRC(9c428613) SHA1(702494a1b5fe8787d52812dfc7adcce806dad73d) )
	ROM_LOAD16_BYTE( "con_20h2.1d1", 0x000000, 0x080000, CRC(2c897ecf) SHA1(e9829a332241eee4ee977a2dcb5451b365b0a4fe) )
ROM_END


ROM_START( m5cnct415 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cfrsjs1.5_1", 0x000000, 0x080000, CRC(e080fa69) SHA1(47910af0e3577f442d41fe84674fecd4c9e09db8) )
	ROM_LOAD16_BYTE( "cfrsjs1.5_2", 0x000001, 0x080000, CRC(0e0ea177) SHA1(3f530923b2b392d6b6e96415db75ed6ddefaebbd) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "cfrsjs1.5d1", 0x000000, 0x080000, CRC(9cb4c64b) SHA1(0b5e03d74fa63847e6a29a6a1c3d1bc042aef9ea) )
ROM_END

ROM_START( m5cnct420 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "con_sjs2.0_1", 0x000000, 0x080000, CRC(a7057e51) SHA1(5b9cec2a48c78ddc900dae907b2dd02169b1e811) )
	ROM_LOAD16_BYTE( "con_sjs2.0_2", 0x000001, 0x080000, CRC(3423d8db) SHA1(b6a3a3c42f653a177d8a19a545e0444918d4881b) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	// all of these were paired with above, rv 2.0
	ROM_LOAD16_BYTE( "con_sjs2.0d1", 0x000000, 0x080000, CRC(cbc8555f) SHA1(3365543762a5e266aec2332e59d59f7d324039e2) )
	ROM_LOAD16_BYTE( "con_sjl2.0d1", 0x000000, 0x080000, CRC(6f0b3e41) SHA1(69eab2ce970b8ac1c6afd3413e0701fddc7e8071) )
	ROM_LOAD16_BYTE( "con_sjk2.0_1", 0x000000, 0x080000, CRC(b55d4972) SHA1(b369df0f3027c7bedb9522ef376bade68c7807a3) )
	ROM_LOAD16_BYTE( "con_sjh2.0d1", 0x000000, 0x080000, CRC(d990627c) SHA1(24f03c70a6bfbc694e1afd74e298f21613ba0e45) )
	ROM_LOAD16_BYTE( "con_20h2.0d1", 0x000000, 0x080000, CRC(805e66f6) SHA1(a13083417f8eb75d0c1b07eb5aae6f54a79d0721) )
	ROM_LOAD16_BYTE( "con_sja2.0_1", 0x000000, 0x080000, CRC(ec739c62) SHA1(285d21faf3ec17475602e195ce8cf978c5971639) )
ROM_END


ROM_START( m5cos )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "codz03d.p1", 0x000000, 0x080000, CRC(5a227552) SHA1(d8bb4fdc80bbac97b41f4e6ae0f2baa1eb359664) )
	ROM_LOAD16_BYTE( "codz03.p2", 0x000001, 0x080000, CRC(44ab6d02) SHA1(8cf98f4c7531565579edd8872144a9fde54ba994) )
	ROM_LOAD16_BYTE( "codz03.p3", 0x100000, 0x080000, CRC(f5a4b3b7) SHA1(6a7c6773eb3b1bbf6c88e119d68827ba54262d1b) )
	ROM_LOAD16_BYTE( "codz03.p4", 0x100001, 0x080000, CRC(ffbb3cc0) SHA1(12dfd6a0ef59712ee4c23212d5491c1a55792b13) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "codz03f.p1", 0x000000, 0x080000, CRC(55eb0f23) SHA1(089e380aca9c97412aa2f37f5488ca5d79769707) )
	ROM_LOAD16_BYTE( "codz03s.p1", 0x000000, 0x080000, CRC(8caf42a9) SHA1(af5a7c028676ed9500a58be7853826ed6113c500) )
ROM_END

ROM_START( m5cosclb )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "codc06d.p1", 0x000000, 0x080000, CRC(7eafc55c) SHA1(aa03b6a1ac12b35dfa62d166818bd02d730a166a) )
	ROM_LOAD16_BYTE( "codc06.p2", 0x000001, 0x080000, CRC(b089ffe9) SHA1(36c28e83365cc688de959fdcf08459b367fafd53) )
	ROM_LOAD16_BYTE( "codc06.p3", 0x100000, 0x080000, CRC(f5a4b3b7) SHA1(6a7c6773eb3b1bbf6c88e119d68827ba54262d1b) )
	ROM_LOAD16_BYTE( "codc06.p4", 0x100001, 0x080000, CRC(ffbb3cc0) SHA1(12dfd6a0ef59712ee4c23212d5491c1a55792b13) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "codc06f.p1", 0x000000, 0x080000, CRC(e76ba5ee) SHA1(b0d419461391f3d21d18dba2f13fd72f50785cbf) )
	ROM_LOAD16_BYTE( "codc06s.p1", 0x000000, 0x080000, CRC(9a19e12f) SHA1(ed0c9e4fd2f170b8986e5a094e0ebadfcfee2d34) )
ROM_END



ROM_START( m5crzkni )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cckn02ad.p1", 0x000000, 0x080000, CRC(ed00ffce) SHA1(e5a239c11a052154b807d277e6eee2aa57646db1) )
	ROM_LOAD16_BYTE( "cckn02.p2", 0x000001, 0x080000, CRC(a70249d0) SHA1(0c78ba0b26348d5e96908f4e9c4f6ea08b78940c) )
	ROM_LOAD16_BYTE( "cckn02.p3", 0x100000, 0x080000, CRC(76dd2055) SHA1(9ad4787df4fe28f995d9f166326ea2d8eece456d) )
	ROM_LOAD16_BYTE( "cckn02.p4", 0x100001, 0x080000, CRC(7212f7e7) SHA1(ff4630aa2b3b57fb2d585fbd7deb6c4307b93e30) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "cckn02b.p1", 0x000000, 0x080000, CRC(a64fd2ca) SHA1(c818e75af752187969efab045cdf235bf724f8a4) )
	ROM_LOAD16_BYTE( "cckn02bd.p1", 0x000000, 0x080000, CRC(0723b63c) SHA1(c6ed74393a39d9014b7e3077cf9fa299cc95f69e) )
	ROM_LOAD16_BYTE( "cckn02d.p1", 0x000000, 0x080000, CRC(591fa65c) SHA1(711d7a2f4f0e0a8cc6839c14a63545383788243e) )
	ROM_LOAD16_BYTE( "cckn02dy.p1", 0x000000, 0x080000, CRC(8670af68) SHA1(0abeebf5ea4a479936dd80b2e55454a3dd99857f) )
	ROM_LOAD16_BYTE( "cckn02h.p1", 0x000000, 0x080000, CRC(85ffbc40) SHA1(a94472dbc4c3839851b4c37cb3621cf1a8d4f5cf) )
	ROM_LOAD16_BYTE( "cckn02r.p1", 0x000000, 0x080000, CRC(b5fb3b3a) SHA1(163c1efcdfe028622f618c0d8c1a39624fe8f212) )
	ROM_LOAD16_BYTE( "cckn02s.p1", 0x000000, 0x080000, CRC(03da44a1) SHA1(dad3cb900f9bf329841493a1bd22df407da6ae8f) )
	ROM_LOAD16_BYTE( "cckn02y.p1", 0x000000, 0x080000, CRC(dcb54d95) SHA1(33d3cdaeebec1d5e7d6925ac2563a0f0f6fc4eae) )
ROM_END

ROM_START( m5crzkni03 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cckn03s.p1", 0x000000, 0x080000, CRC(c1a23590) SHA1(5a5537dcd349674de1930f28ff4c42c384777b25) )
	ROM_LOAD16_BYTE( "cckn03.p2", 0x000001, 0x080000, CRC(8cb9b49e) SHA1(fa7fdcc75eaf8e19c51b4cb541099a9c1d6b8c6e) )
	ROM_LOAD16_BYTE( "cckn03.p3", 0x100000, 0x080000, CRC(76dd2055) SHA1(9ad4787df4fe28f995d9f166326ea2d8eece456d) ) // = 02
	ROM_LOAD16_BYTE( "cckn03.p4", 0x100001, 0x080000, CRC(7212f7e7) SHA1(ff4630aa2b3b57fb2d585fbd7deb6c4307b93e30) ) // = 02

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "cckn03ad.p1", 0x000000, 0x080000, CRC(a034c91c) SHA1(132602fe4b390eb6fd620ffcf3a6a7c8e9edda32) )
	ROM_LOAD16_BYTE( "cckn03b.p1", 0x000000, 0x080000, CRC(f200e4cc) SHA1(085a98e9d7c135aacad944da1cf56f34b7ea7b59) )
	ROM_LOAD16_BYTE( "cckn03bd.p1", 0x000000, 0x080000, CRC(b3d7f7c9) SHA1(3eeb8ba61432e6fe9398d6085997e4e8a8dd8f03) )
	ROM_LOAD16_BYTE( "cckn03d.p1", 0x000000, 0x080000, CRC(4bd0e3af) SHA1(29430f15171dd5e17f056aab6844c7395866c22d) )
	ROM_LOAD16_BYTE( "cckn03dy.p1", 0x000000, 0x080000, CRC(cab645f3) SHA1(e46b9bb76f035cef598582975e4a2e8ef7c4721b) )
	ROM_LOAD16_BYTE( "cckn03h.p1", 0x000000, 0x080000, CRC(33c5bd3b) SHA1(83b0f7bb23ccefdb89717068a81458f3b1f5915c) )
	ROM_LOAD16_BYTE( "cckn03r.p1", 0x000000, 0x080000, CRC(3a40a946) SHA1(a033fdca983b6ad943b410711b524f4cbe1dc296) )
	ROM_LOAD16_BYTE( "cckn03y.p1", 0x000000, 0x080000, CRC(c01c4118) SHA1(66a89c25e53c1d03c59f9b72865e103d206511ad) )
ROM_END

ROM_START( m5crocrk )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "croc0_3.p1", 0x000000, 0x080000, CRC(80c8c406) SHA1(225b7ec43824770aabae9e1ec780189bfe4886c2) )
	ROM_LOAD16_BYTE( "croc0_3.p2", 0x000001, 0x080000, CRC(7ca2f33c) SHA1(f9b59991594e91b30c2c8cb4f8f607e4c2dbc71a) )
	ROM_LOAD16_BYTE( "croc0_2.p3", 0x100000, 0x080000, CRC(b04c704f) SHA1(1af3b57b9c13ee71577b6c6ad706981207af011f) )
	ROM_LOAD16_BYTE( "croc0_2.p4", 0x100001, 0x080000, CRC(107565ee) SHA1(7fb6512c260358362b6b35651f8f77ceeef25e5c) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "croc0_3d.p1", 0x000000, 0x080000, CRC(fef5d3eb) SHA1(2ab5ae313d1bf4847350d146369540ab1a93a4ac) )
ROM_END

ROM_START( m5crocrk10 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "croc1_0.p1", 0x000000, 0x080000, CRC(e86e6f20) SHA1(4ec858b4217cec1c50e05df921d01435b3860f3b) )
	ROM_LOAD16_BYTE( "croc1_0.p2", 0x000001, 0x080000, CRC(bb4c66be) SHA1(398a51dcc8fba725b0a035366262e85191692b57) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "croc1_0d.p1", 0x000000, 0x080000, CRC(2eeb8615) SHA1(97be222ddacf59dce0f9d597a2c486f3c9b20f5b) )
ROM_END


ROM_START( m5croclb )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ccro0_4.p1", 0x000000, 0x080000, CRC(ee490a58) SHA1(125b29495a65c05b24b2436a3336ff40e736ae4f) )
	ROM_LOAD16_BYTE( "ccro0_4.p2", 0x000001, 0x080000, CRC(fd51e532) SHA1(a6d74eb7f75505a206e5446b770c71dd037acb71) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "ccro0_4d.p1", 0x000000, 0x080000, CRC(fc2b5e33) SHA1(255a88845797bc2ec74a743235cf2faa3a4dfca7) )
ROM_END


ROM_START( m5crsfir )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cros1_1.p1", 0x000000, 0x080000, CRC(46b1ef3e) SHA1(c501dcdc1d8fd76e6883c9eaf11d853683ed1fcc) )
	ROM_LOAD16_BYTE( "cros1_1.p2", 0x000001, 0x080000, CRC(6919f8f4) SHA1(785b5cc2a335f88f0b06a0c61074498f6b1998b7) )
	ROM_LOAD16_BYTE( "cros0_6.p3", 0x100000, 0x080000, CRC(90e12d4d) SHA1(ba6cd55fa3932d3383998c6ffc473661b1961d1e) )
	ROM_LOAD16_BYTE( "cros0_6.p4", 0x100001, 0x080000, CRC(4084ecca) SHA1(51420945907b8c34b620178b536c65b989dce778) )
ROM_END


ROM_START( m5dmnstr )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "dst11s.p1", 0x000000, 0x080000, CRC(6b724807) SHA1(35e6dab9fdba4097ef111503af89dcc9d5128677) )
	ROM_LOAD16_BYTE( "dst11s.p2", 0x000001, 0x080000, CRC(30b48165) SHA1(fad2ffb6044d010a33c0831c9bcf322ccf6d7f72) )
	ROM_LOAD16_BYTE( "dst11s.p3", 0x100000, 0x080000, CRC(a475ddd6) SHA1(97e26e23a9b030ff2eb242feaabc9b437ffaf299) )
	ROM_LOAD16_BYTE( "dst11s.p4", 0x100001, 0x080000, CRC(4693aae3) SHA1(2e201950670ed2b4373b23158f2689d4d9f8e8c4) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "dst11a.p1", 0x000000, 0x080000, CRC(10ff912a) SHA1(6e1bc7f474b90d312ba5b0f36ffb1297ad169dbc) )
	ROM_LOAD16_BYTE( "dst11d.p1", 0x000000, 0x080000, CRC(ed9a180b) SHA1(370fd26cc461015af8959361ebff2eb82a59bf09) )
	ROM_LOAD16_BYTE( "dst11k.p1", 0x000000, 0x080000, CRC(4fb0d9eb) SHA1(f11bba2ab9d6fe32ddb853cecc9ce776d87dd6c3) )
ROM_END

ROM_START( m5dmnstra )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "dst13s.p1", 0x000000, 0x080000, CRC(228af479) SHA1(a74ce5cbe9ac76d0681efbcff46cb883c94fed5c) )
	ROM_LOAD16_BYTE( "dst13s.p2", 0x000001, 0x080000, CRC(11fc9375) SHA1(f6ef09285266671df1b72892590ea51a80ab33eb) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "dst13a.p1", 0x000000, 0x080000, CRC(59072d54) SHA1(c8c0f1c39a579105018eb40fe608bd634b36e663) )
	ROM_LOAD16_BYTE( "dst13d.p1", 0x000000, 0x080000, CRC(a462a475) SHA1(5cc054fee775535b336bf572dcb4c611904eeec2) )
	ROM_LOAD16_BYTE( "dst13k.p1", 0x000000, 0x080000, CRC(06486595) SHA1(b63645d1b16cef05952f4f5b8e8fd00f19341b36) )
ROM_END


ROM_START( m5dmnf )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "dafr0_3.p1", 0x000000, 0x080000, CRC(0bb7f48f) SHA1(4e76280f8090fbca96f5eb66b99e57e5aec07b17) )
	ROM_LOAD16_BYTE( "dafr0_3.p2", 0x000001, 0x080000, CRC(010bef71) SHA1(3a6fc034bc11b6c745781376be96fc75756e4d8c) )
	ROM_LOAD16_BYTE( "dafr0_3.p3", 0x100000, 0x080000, CRC(af57eaee) SHA1(bfd69bd917a4abf5a43398a7530ed164befd8331) )
	ROM_LOAD16_BYTE( "dafr0_3.p4", 0x100001, 0x080000, CRC(882c2fca) SHA1(67098d7919b96ac67d2929388de614d3cd04c144) )
ROM_END

ROM_START( m5dmnf10 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "dafr1_0.p1", 0x000000, 0x080000, CRC(97cd1946) SHA1(a31687b8cb4a38729b8bcc4c7dfaecc96539153b) )
	ROM_LOAD16_BYTE( "dafr1_0.p2", 0x000001, 0x080000, CRC(effb87e0) SHA1(d7194d173fc5b66d2005a7e35bf5668029f64605) )
	/* 3+4 */
ROM_END


ROM_START( m5dmnfcl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cdaf0_3.p1", 0x000000, 0x080000, CRC(5d16e0a7) SHA1(476b719130f02e614a32740cac3a46c9da3551d5) )
	ROM_LOAD16_BYTE( "cdaf0_3.p2", 0x000001, 0x080000, CRC(77cf66c0) SHA1(5688ea0a4932e8e711cab4f2d0de5a317d3579f9) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "cdaf0_3d.p1", 0x000000, 0x080000, CRC(f696a608) SHA1(232b73a1c10642d67889e11f2feb663105ff352a) )
ROM_END

ROM_START( m5dmnfcl04 ) // very similar to 0_3 or an error
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cdaf0_4.p1", 0x000000, 0x080000, CRC(5d16e0a7) SHA1(476b719130f02e614a32740cac3a46c9da3551d5) ) // == 03
	ROM_LOAD16_BYTE( "cdaf0_4.p2", 0x000001, 0x080000, CRC(88ff8af1) SHA1(d691b23d0972c2794e6c844d027ce02b6819888f) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "cdaf0_4d.p1", 0x000000, 0x080000, CRC(f696a608) SHA1(232b73a1c10642d67889e11f2feb663105ff352a) ) // == 03
ROM_END


ROM_START( m5doshpk )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "dosh03ad.p1", 0x000000, 0x080000, CRC(0b24ae7f) SHA1(f0eb43f29970fe2da3f24540a9f2df963d8989a6) )
	ROM_LOAD16_BYTE( "dosh03.p2", 0x000001, 0x080000, CRC(35b3d12d) SHA1(9cff1ce47003e358713f647764eb7224c6109584) )
	ROM_LOAD16_BYTE( "dosh03.p3", 0x100000, 0x080000, CRC(2b2c95ca) SHA1(4242ecf226d886fbe9c6e3b9feaa6736c7574832) )
	ROM_LOAD16_BYTE( "dosh03.p4", 0x100001, 0x080000, CRC(e046b13a) SHA1(b1bcd2838bcbab822030f25a358e7fe0c7f80d4e) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "dosh03b.p1", 0x000000, 0x080000, CRC(c40b0b0f) SHA1(f171014d6721211b230414b54882a5a5d23c2330) )
	ROM_LOAD16_BYTE( "dosh03bd.p1", 0x000000, 0x080000, CRC(387ab7f0) SHA1(62f58478bbd9ea7bbbfa94c061c6bb34ccb5d2b4) )
	ROM_LOAD16_BYTE( "dosh03d.p1", 0x000000, 0x080000, CRC(42d6cb45) SHA1(a0f91d46421668ee9ac06aaf0e7b0b66a5f89b0b) )
	ROM_LOAD16_BYTE( "dosh03dy.p1", 0x000000, 0x080000, CRC(7d634b6c) SHA1(0b0308de3ebf6612069fc00b4a7823161ded8612) )
	ROM_LOAD16_BYTE( "dosh03k.p1", 0x000000, 0x080000, CRC(f69e32d3) SHA1(c9175b14ed2f6a7bbd166bcd8836b03a933170dd) )
	ROM_LOAD16_BYTE( "dosh03r.p1", 0x000000, 0x080000, CRC(63a9cda9) SHA1(6a89eef7010e556cd46bba70114991c2c5ff7d81) )
	ROM_LOAD16_BYTE( "dosh03s.p1", 0x000000, 0x080000, CRC(392d2f38) SHA1(d88cb02aba00e653665c90219033139224dba142) )
	ROM_LOAD16_BYTE( "dosh03y.p1", 0x000000, 0x080000, CRC(0698af11) SHA1(1e89c174eb78341325e8f4622fab0eb703208a8d) )
ROM_END

ROM_START( m5doshpk05 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "dosh0_5.p1", 0x000000, 0x080000, CRC(03e06273) SHA1(f7bfcdf161fdbe4abdf466f3eb9566b252fd2622) )
	ROM_LOAD16_BYTE( "dosh0_5.p2", 0x000001, 0x080000, CRC(be0fde9b) SHA1(dbeed91ab51f405f2167349dd8c84c31a6c42862) )
	ROM_LOAD16_BYTE( "dosh0_5.p3", 0x100000, 0x080000, CRC(2b2c95ca) SHA1(4242ecf226d886fbe9c6e3b9feaa6736c7574832) ) // == 03
	ROM_LOAD16_BYTE( "dosh0_5.p4", 0x100001, 0x080000, CRC(e046b13a) SHA1(b1bcd2838bcbab822030f25a358e7fe0c7f80d4e) ) // == 03
ROM_END



ROM_START( m5draclb )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "drcc06d.p1", 0x000000, 0x080000, CRC(740c8351) SHA1(2e5afe8422cc5986fdf6acec2b60e82bb66f6338) )
	ROM_LOAD16_BYTE( "drcc06.p2", 0x000001, 0x080000, CRC(33582bca) SHA1(e70d9734950a91b944b75ac1659d241106da267c) )
	ROM_LOAD16_BYTE( "drcc06.p3", 0x100000, 0x080000, CRC(5054ec32) SHA1(2f4134456f502583ce93af4ccc55eb89dbfe48ca) )
	ROM_LOAD16_BYTE( "drcc06.p4", 0x100001, 0x080000, CRC(0483d31f) SHA1(81a299f05cfd6d1446d44509da86c3a70fb00ba0) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "drcc06dz.p1", 0x000000, 0x080000, CRC(04fa636f) SHA1(a411b9a7281b90700c489b837159489b7dbe9620) )
	ROM_LOAD16_BYTE( "drcc06f.p1", 0x000000, 0x080000, CRC(64d070ad) SHA1(3d123a1dc6f0c7e1b0b59b45133140b67cfde2b2) )
	ROM_LOAD16_BYTE( "drcc06s.p1", 0x000000, 0x080000, CRC(cb74c9fa) SHA1(72d23fe679566518742a754855215d546e02c556) )
	ROM_LOAD16_BYTE( "drcc06z.p1", 0x000000, 0x080000, CRC(07f4b6fe) SHA1(25fb368d32ecb306ae47feab80c09b8d6bbbaf86) )
ROM_END

ROM_START( m5draclb07 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "drct07s.p1", 0x000000, 0x080000, CRC(d8a1df97) SHA1(ac29755dafe150f4d3cfbbd23a71c4a55e9bf8de) )
	ROM_LOAD16_BYTE( "drct07.p2", 0x000001, 0x080000, CRC(4ce428df) SHA1(1144bc21269af10e7094b1bd2271fc6ee63a8d25) )
	ROM_LOAD16_BYTE( "drct07.p3", 0x100000, 0x080000, CRC(5054ec32) SHA1(2f4134456f502583ce93af4ccc55eb89dbfe48ca) ) // == 06
	ROM_LOAD16_BYTE( "drct07.p4", 0x100001, 0x080000, CRC(0483d31f) SHA1(81a299f05cfd6d1446d44509da86c3a70fb00ba0) ) // == 06

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "drct07d.p1", 0x000000, 0x080000, CRC(c467ca89) SHA1(06f13448860ca7360dd48e29fc780554bc42ab8b) )
	ROM_LOAD16_BYTE( "drct07dz.p1", 0x000000, 0x080000, CRC(907d9e63) SHA1(a1a8f74e79043cecb42cfd28c14cef98d54b4785) )
	ROM_LOAD16_BYTE( "drct07f.p1", 0x000000, 0x080000, CRC(73aefaaf) SHA1(a064856c16571ba536c390e10e90e080ac212e58) )
	ROM_LOAD16_BYTE( "drct07z.p1", 0x000000, 0x080000, CRC(8cbb8b7d) SHA1(0c7831a3798fcbb365dacc86045cc742e445b27f) )
ROM_END

ROM_START( m5draclb01 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "drcu01.p1", 0x000000, 0x080000, CRC(47a133ac) SHA1(6835e1cea39d0da144b85c3b66afe850a54eb5c5) )
	ROM_LOAD16_BYTE( "drcu01.p2", 0x000001, 0x080000, CRC(78a55628) SHA1(5bada6f5749fb4079b058e1798e16dc30496db7c) )
	/* 3+4? */
ROM_END



ROM_START( m5ewn )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ewyn03d.p1", 0x000000, 0x080000, CRC(bd038fd2) SHA1(196478e5f2f6d176df51dc008f60643d0d7bf55f) )
	ROM_LOAD16_BYTE( "ewyn03.p2", 0x000001, 0x080000, CRC(e60e6ee8) SHA1(aad20254013d8ccb2f353c6e17f5cc168a49838d) )
	ROM_LOAD16_BYTE( "ewyn03.p3", 0x100000, 0x080000, CRC(aa525011) SHA1(4902ead245ac2621e409718e30963ae83c2935a9) )
	ROM_LOAD16_BYTE( "ewyn03.p4", 0x100001, 0x080000, CRC(d866609e) SHA1(c3bbeab852966751b24996d20899cb428f64d33e) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "ewyn03dy.p1", 0x000000, 0x080000, CRC(64469790) SHA1(58f7cb899c57af367fa688e07239c3c41bd5ee11) )
	ROM_LOAD16_BYTE( "ewyn03h.p1", 0x000000, 0x080000, CRC(1d56fb1a) SHA1(68a8d88b9b9e141e8945251fa7712eae97f92455) )
	ROM_LOAD16_BYTE( "ewyn03s.p1", 0x000000, 0x080000, CRC(fd38a3b6) SHA1(e3f524abf0e8977bf893aaacadefcdb9fa735442) )
	ROM_LOAD16_BYTE( "ewyn03y.p1", 0x000000, 0x080000, CRC(0272423f) SHA1(dc560366c5497b93d33b80a622c3296cfae9a157) )
ROM_END

ROM_START( m5ewn08 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ewyn08s.p1", 0x000000, 0x080000, CRC(03b5c72a) SHA1(0c8f148140fb691aca7fcc1520eecdf81601bc10) )
	ROM_LOAD16_BYTE( "ewyn08.p2", 0x000001, 0x080000, CRC(f92b9097) SHA1(93ee983527aa4605e33fac6812c3fe1a49225224) )
	ROM_LOAD16_BYTE( "ewyn08.p3", 0x100000, 0x080000, CRC(079e1138) SHA1(c9bb5ece68cec07d4eb497e9486eeb3bdcaeb940) )
	ROM_LOAD16_BYTE( "ewyn08.p4", 0x100001, 0x080000, CRC(6aa11324) SHA1(6e62fec68eca4db43e5feaca3d79907ae662f1a8) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "ewyn08ad.p1", 0x000000, 0x080000, CRC(f10430a7) SHA1(5e9444214543e986821e8a0ec4d9ee4ffb251964) )
	ROM_LOAD16_BYTE( "ewyn08b.p1", 0x000000, 0x080000, CRC(91ff4334) SHA1(2d5cded32a4aac6e4589980f9ef3a9811bf2f258) )
	ROM_LOAD16_BYTE( "ewyn08bd.p1", 0x000000, 0x080000, CRC(da571a74) SHA1(eb4427f66d5bcf11036c5a2e4d340639ed77fa01) )
	ROM_LOAD16_BYTE( "ewyn08d.p1", 0x000000, 0x080000, CRC(1aa51e3a) SHA1(a3cc0511a5e8824ef69eb17ee9341430bcfbebde) )
	ROM_LOAD16_BYTE( "ewyn08dy.p1", 0x000000, 0x080000, CRC(b421e1fb) SHA1(93463bab7b7ccfe6585a9cea6c4ca408fa9f7fbd) )
	ROM_LOAD16_BYTE( "ewyn08h.p1", 0x000000, 0x080000, CRC(d71a86d7) SHA1(d86c5768455958eeb885cfd38fb62f9e63d087ba) )
	ROM_LOAD16_BYTE( "ewyn08r.p1", 0x000000, 0x080000, CRC(f6ebc1e3) SHA1(23f6befdf9a138f79b9348c1cbba03b0e7644b73) )
	ROM_LOAD16_BYTE( "ewyn08y.p1", 0x000000, 0x080000, CRC(a538b5fa) SHA1(3597cf36627f5c57362e5c9664b58fe4a2abeff2) )
ROM_END

ROM_START( m5elband )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "eba_14s.p1", 0x000000, 0x080000, CRC(087880ac) SHA1(34a83b282af945fd1f42498ff019a3ca1ff84948) )
	ROM_LOAD16_BYTE( "eba_14.p2", 0x000001, 0x080000, CRC(504b22cb) SHA1(fd4eb8b4f5983cb5da4eb950ae3d656655c55f7e) )
	ROM_LOAD16_BYTE( "eba_14.p3", 0x100000, 0x080000, CRC(270268a6) SHA1(0ef3f5d798d8219e364f9a45892a76ee764eaa2f) )
	ROM_LOAD16_BYTE( "eba_14.p4", 0x100001, 0x080000, CRC(8b1b3852) SHA1(58cb06a24548c1700d904f357f36fc2cb7c43b63) )
ROM_END

ROM_START( m5elim )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "elim02ad.p1", 0x000000, 0x080000, CRC(07ee20df) SHA1(0a947dc6d03e90a6c2cebcc1dd70a197cfc1f9c7) )
	ROM_LOAD16_BYTE( "elim02.p2", 0x000001, 0x080000, CRC(7a5d4a13) SHA1(4160466b815b5a3c59d1007a5e0c30e6a0dc9ca5) )
	ROM_LOAD16_BYTE( "elim02.p3", 0x100000, 0x080000, CRC(d93cc124) SHA1(01e6752e0fbda9879daecabc35602a2f49a0aaab) )
	ROM_LOAD16_BYTE( "elim02.p4", 0x100001, 0x080000, CRC(2bcdfa0a) SHA1(5e6cf3f716527a38b89d9f6bc22ca4876fde404b) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "elim02b.p1", 0x000000, 0x080000, CRC(90e16c8d) SHA1(784fefb7d4b10de4a604fe0c7373b7544839cf45) )
	ROM_LOAD16_BYTE( "elim02d.p1", 0x000000, 0x080000, CRC(be305d93) SHA1(ec3b32664d9491ccb0865a6791886853c63fe1d6) )
	ROM_LOAD16_BYTE( "elim02dy.p1", 0x000000, 0x080000, CRC(c230b71c) SHA1(43982898c1bad88d27ed93ccc48d6405a309a82e) )
	ROM_LOAD16_BYTE( "elim02h.p1", 0x000000, 0x080000, CRC(7f02f0b6) SHA1(e65b56f4c7a8b849da88eab9c2dde32307c94977) )
	ROM_LOAD16_BYTE( "elim02r.p1", 0x000000, 0x080000, CRC(caa11fe5) SHA1(8f720dc7c1c9bf417e90cd92be9df31861e105e6) )
	ROM_LOAD16_BYTE( "elim02s.p1", 0x000000, 0x080000, CRC(3f626dee) SHA1(3c482e90f8fdbed1e3e91db980073e99ada9e6bb) )
	ROM_LOAD16_BYTE( "elim02y.p1", 0x000000, 0x080000, CRC(f16a9961) SHA1(35e18bb70e8e5b096526242ef9400630d675e704) )
ROM_END

ROM_START( m5elim03 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "elim03s", 0x00000, 0x080000, CRC(6903d0d0) SHA1(01af2abc7f88e7e9ebfbe668727f5ff52658a5e1) )
	ROM_LOAD16_BYTE( "elim03.p2", 0x00001, 0x080000, CRC(3262eb5a) SHA1(62bbfebb32e16fb7f5945a6b523c619401117a1f) )
	/* 3+4? */
ROM_END

ROM_START( m5elim04 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "elim04s.p1", 0x000000, 0x080000, CRC(56b13411) SHA1(5fa183c45ee5b4527650171ab7bbb4afa95eb577) )
	ROM_LOAD16_BYTE( "elim04.p2", 0x000001, 0x080000, CRC(4f17729a) SHA1(8854a8fd571a8de6b49fec6c172ec0a5f5f1622f) )
	ROM_LOAD16_BYTE( "elim04.p3", 0x100000, 0x080000, CRC(1387646a) SHA1(00c6157c07398c6e5b4e6ead6a8cca88e38f298d) )
	ROM_LOAD16_BYTE( "elim04.p4", 0x100001, 0x080000, CRC(99f609d4) SHA1(53803bc0f4712b7772f525f2b65ad3ca8b1e6122) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "elim04ad.p1", 0x000000, 0x080000, CRC(f23ed4db) SHA1(990dc8b10931745a7a9ab24d4502d70bc7f28d1d) )
	ROM_LOAD16_BYTE( "elim04b.p1", 0x000000, 0x080000, CRC(3e18df20) SHA1(7a68900770bdf624e14ae8e2ec05fbb7c2dee91a) )
	ROM_LOAD16_BYTE( "elim04bd.p1", 0x000000, 0x080000, CRC(9594c18e) SHA1(e17c3cda1d8e3812a1dbd2126a006ce4eed6acb7) )
	ROM_LOAD16_BYTE( "elim04d.p1", 0x000000, 0x080000, CRC(d3e5e0e0) SHA1(eef5b68e210ee7dca92ab44e561d6cc9c70a2b3d) )
	ROM_LOAD16_BYTE( "elim04dy.p1", 0x000000, 0x080000, CRC(e252526a) SHA1(cb871e437dd7d165a2e4845d3e5e7e34e0821a05) )
	ROM_LOAD16_BYTE( "elim04h.p1", 0x000000, 0x080000, CRC(a7e8dfd2) SHA1(86beb9501833effb94687571af5e66d27333593b) )
	ROM_LOAD16_BYTE( "elim04r.p1", 0x000000, 0x080000, CRC(ed73666d) SHA1(4063e483a252c2092ad70fdb50e25eae72c585ad) )
	ROM_LOAD16_BYTE( "elim04y.p1", 0x000000, 0x080000, CRC(6706869b) SHA1(13fa227fcc01db7f3143ec6d9aa20054dd112884) )
ROM_END


ROM_START( m5extrm )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "extr0_2.p1", 0x000000, 0x080000, CRC(e4c23cd4) SHA1(dfcbad797ef96091760dda7c158893fdff5bd6fc) )
	ROM_LOAD16_BYTE( "extr0_2.p2", 0x000001, 0x080000, CRC(afcb3c17) SHA1(13a8aa67a6762e8af4558dfb051663608520cd95) )
	ROM_LOAD16_BYTE( "extr0_3.p3", 0x100000, 0x080000, CRC(400dfd6b) SHA1(6491b0aa2f70b9843c6d8f7852105d5c99984c1d) )
	ROM_LOAD16_BYTE( "extr0_3.p4", 0x100001, 0x080000, CRC(77ffb5a6) SHA1(5bafe0163127c7682da4695497d2af971a3200d4) )
ROM_END


ROM_START( m5extrmm )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "emad0_4m.p1", 0x000000, 0x080000, CRC(afbcce7c) SHA1(c34d2224a32cbd6900232675211fa854ff8da344) )
	ROM_LOAD16_BYTE( "emad0_4m.p2", 0x000001, 0x080000, CRC(712e36be) SHA1(00dfefe980948f2834d47714482790aba02a62f3) )
ROM_END

ROM_START( m5extrmm04a )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "emad0_4q.p1", 0x000000, 0x080000, CRC(4031b2c1) SHA1(55b802252c697d4e7b9c0d69b0b75480e6891e3c) )
	ROM_LOAD16_BYTE( "emad0_4q.p2", 0x000001, 0x080000, CRC(27357b33) SHA1(461b481987b0992e117e8c8125b4806d39066722) )
ROM_END

ROM_START( m5extrmm04b )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "emad0_4x.p1", 0x000000, 0x080000, CRC(be72f2a9) SHA1(0448aced7049a19153452e7ee432498f22392a0e) )
	ROM_LOAD16_BYTE( "emad0_4x.p2", 0x000001, 0x080000, CRC(27357b33) SHA1(461b481987b0992e117e8c8125b4806d39066722) )
ROM_END

ROM_START( m5extrmm10 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "emad1_0.p1", 0x000000, 0x080000, CRC(575b8cab) SHA1(78df3e4f1ee8b7cd9973eb0d79580b897c3c0044) )
	ROM_LOAD16_BYTE( "emad1_0.p2", 0x000001, 0x080000, CRC(769125b8) SHA1(d45f3ad1a2ea1cd1e0b2fcc0eed325e879e460d4) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "emad1_0d.p1", 0x000000, 0x080000, CRC(86c999ed) SHA1(12944216f6d827ec97eb9dedd4cda00c9179b3b3) )
ROM_END


ROM_START( m5firebl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fire03dy.p1", 0x000000, 0x080000, CRC(b381f455) SHA1(15800dcc9109a749936ed7d82d2ad8278bd6d333) )
	ROM_LOAD16_BYTE( "fire0_3.p2", 0x000001, 0x080000, CRC(d43f3c8f) SHA1(05adf13f54a67318b4ed5df33b68480e63bc55e3) )
	ROM_LOAD16_BYTE( "fire0_3.p3", 0x100000, 0x080000, CRC(a1dca412) SHA1(4d73c86b2b224956c13750c327adecd16c5281da) )
	ROM_LOAD16_BYTE( "fire0_3.p4", 0x100001, 0x080000, CRC(b35ccfec) SHA1(bb9c375001e36c1104e575e7183fb5966e2496ac) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "fire0_3b.p1", 0x000000, 0x080000, CRC(0af4e8a2) SHA1(a7dceddafebe33175028abcf705d152a36a5df71) )
	ROM_LOAD16_BYTE( "fire0_3d.p1", 0x000000, 0x080000, CRC(b3f3b039) SHA1(a3684f627537b9da28fbe56d14d560e5c935f3e4) )
	ROM_LOAD16_BYTE( "fire0_3k.p1", 0x000000, 0x080000, CRC(0747b7f9) SHA1(d8374130401d755a105f5354f9b0744fe151d3fb) )
	ROM_LOAD16_BYTE( "fire0_3r.p1", 0x000000, 0x080000, CRC(6d5304fc) SHA1(96e4ad0a3ff2f2950f3b6588358cdf002af29997) )
	ROM_LOAD16_BYTE( "fire0_3s.p1", 0x000000, 0x080000, CRC(f1511132) SHA1(4deb2567b7903ccfc2dfc16dcdbc902905b8c52c) )
	ROM_LOAD16_BYTE( "fire0_3y.p1", 0x000000, 0x080000, CRC(f123555e) SHA1(014a4f4dc366835e6297ba88cd531ce7a454a82b) )
ROM_END


ROM_START( m5fishdl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fful0_2.p1", 0x000000, 0x080000, CRC(2611f107) SHA1(8c6b16d6ef3bb51e3cf59c3424671df2feabc3be) )
	ROM_LOAD16_BYTE( "fful0_2.p2", 0x000001, 0x080000, CRC(b1d7cde0) SHA1(a5891dad1fdb2a6c4e0c3efc137cd1c433d803b7) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "fful0_2d.p1", 0x000000, 0x080000, CRC(74efacea) SHA1(a2e55760b551d082b9df047b09529b8fbf5a0193) )
ROM_END

ROM_START( m5fishdl10 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fful1_0.p1", 0x000000, 0x080000, CRC(04c0282a) SHA1(c229132de4789d00e097ff01922f6416bfed56dc) )
	ROM_LOAD16_BYTE( "fful1_0.p2", 0x000001, 0x080000, CRC(3b314d3d) SHA1(b0897c8cf1c704877d24009083d32c7cf788820d) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "fful1_0d.p1", 0x000000, 0x080000, CRC(7f423604) SHA1(880f8916916e13d6fb757060bd6c6fb518265e27) )
ROM_END

ROM_START( m5fishcl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cffu0_2.p1", 0x000000, 0x080000, CRC(0f75f459) SHA1(769db2ad0175f3ffa964d038c3393c623546392e) )
	ROM_LOAD16_BYTE( "cffu0_2.p2", 0x000001, 0x080000, CRC(c7bbe7a5) SHA1(0def965967346a32300d0038843b59bf070f60fb) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "cffu0_2d.p1", 0x000000, 0x080000, CRC(b9e29a7f) SHA1(b6f9ab25c2606cd96fefd98f7d01520bb058b490) )
ROM_END


ROM_START( m5flipcr )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "flip04.p1", 0x000000, 0x080000, CRC(631ee8a8) SHA1(86724a699013bba769516860e6f303b69b8fbe6c) )
	ROM_LOAD16_BYTE( "flip04.p2", 0x000001, 0x080000, CRC(7e9182b9) SHA1(376d7209036d11a6cf509915dc16912183c92b91) )
	ROM_LOAD16_BYTE( "flip04.p3", 0x100000, 0x080000, CRC(4c9eaec6) SHA1(bc013c2feac6574d76169f2516844e7eae77d377) )
	ROM_LOAD16_BYTE( "flip04.p4", 0x100001, 0x080000, CRC(9209150c) SHA1(cd37f82a2b652efabeea8164461e2055ae93cbaf) )
ROM_END


ROM_START( m5fortby )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fboy03ad.p1", 0x000000, 0x080000, CRC(4c9ed8c7) SHA1(8557eb80b875236e5bb660d8dc96fc43cee7922d) )
	ROM_LOAD16_BYTE( "fboy03.p2", 0x000001, 0x080000, CRC(1cf29757) SHA1(e4bb9482ddaea30a162f197abfaa01eebabe9d27) )
	ROM_LOAD16_BYTE( "fboy03.p3", 0x100000, 0x080000, CRC(5c7eb266) SHA1(42c8ed84b274906daa06a773ff5bbb48a98a495d) )
	ROM_LOAD16_BYTE( "fboy03.p4", 0x100001, 0x080000, CRC(a9e4bcc6) SHA1(0ebad4a51756029980e20a5fbc648e2d799b5583) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "fboy03b.p1", 0x000000, 0x080000, CRC(bc63ab15) SHA1(e71c180c92fa5d46209778ae0d77a34dc9b85a56) )
	ROM_LOAD16_BYTE( "fboy03bd.p1", 0x000000, 0x080000, CRC(3bada29b) SHA1(622986b62e752706e54978247ea7770e8d1ef894) )
	ROM_LOAD16_BYTE( "fboy03d.p1", 0x000000, 0x080000, CRC(ed0b440a) SHA1(8bf8c681b2da8b294f91679e7380af68836945fe) )
	ROM_LOAD16_BYTE( "fboy03dy.p1", 0x000000, 0x080000, CRC(267434f7) SHA1(14c9a8b5721aeffc4a4bb4a470dd893d6405c17b) )
	ROM_LOAD16_BYTE( "fboy03h.p1", 0x000000, 0x080000, CRC(d2198bae) SHA1(5a91a405b7c21348ddf706a38b4388418d55789a) )
	ROM_LOAD16_BYTE( "fboy03k.p1", 0x000000, 0x080000, CRC(e066842a) SHA1(7bca7dd6c319fdb134b3ab0507631def5ee3397d) )
	ROM_LOAD16_BYTE( "fboy03r.p1", 0x000000, 0x080000, CRC(cd560670) SHA1(c4ceac1a2b466188b661ea7b7439dfbfbef1d777) )
	ROM_LOAD16_BYTE( "fboy03s.p1", 0x000000, 0x080000, CRC(28723ee6) SHA1(427f10b3c40c8a7656e3478685a94cfe47eade0d) )
	ROM_LOAD16_BYTE( "fboy03y.p1", 0x000000, 0x080000, CRC(e30d4e1b) SHA1(f6353b525807b3e797dad3e4a1ef5a47eb699aa7) )
ROM_END

ROM_START( m5fortby01 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fbtr01s.p1", 0x000000, 0x080000, CRC(ca6c0f4c) SHA1(b15a86f7540e454062074f655b5fad1d7ef09347) )
	ROM_LOAD16_BYTE( "fbtr01.p2", 0x000001, 0x080000, CRC(2eb06307) SHA1(be31e555b0f392b0124365e26c66b9e18cadc53f) )
	ROM_LOAD16_BYTE( "fbtr01.p3", 0x100000, 0x080000, CRC(5c7eb266) SHA1(42c8ed84b274906daa06a773ff5bbb48a98a495d) ) // == 03
	ROM_LOAD16_BYTE( "fbtr01.p4", 0x100001, 0x080000, CRC(a9e4bcc6) SHA1(0ebad4a51756029980e20a5fbc648e2d799b5583) ) // == 03

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "fbtr01d.p1", 0x000000, 0x080000, CRC(f09961ef) SHA1(fc273fd38f321f537c7b405f971a7e8d8ff95c4f) )
	ROM_LOAD16_BYTE( "fbtr01dy.p1", 0x000000, 0x080000, CRC(a30fed6e) SHA1(25cfa3c36fe37d86f74a853f033fbd632fd11380) )
	ROM_LOAD16_BYTE( "fbtr01y.p1", 0x000000, 0x080000, CRC(99fa83cd) SHA1(7adc35e8c8aaf02e1af34c6034cf08c4cc91b85a) )
ROM_END

ROM_START( m5frnzy )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "frnz10ad.p1", 0x000000, 0x080000, CRC(c33be811) SHA1(ddc6a92fec7df71c232dfb94a38be3fc2c7d1ade) )
	ROM_LOAD16_BYTE( "frnz10.p2", 0x000001, 0x080000, CRC(fa9b0f41) SHA1(e08a313a1535687cbd63a22eed9ec951f1ea0c6d) )
	ROM_LOAD16_BYTE( "frnz10.p3", 0x100000, 0x080000, CRC(af8ca3c8) SHA1(71d097915dc32aecfa9a7a4b696854273b7d105e) )
	ROM_LOAD16_BYTE( "frnz10.p4", 0x100001, 0x080000, CRC(639e377c) SHA1(dc33979b6374f277d6cf178470a32f5871ab3483) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "frnz10b.p1", 0x000000, 0x080000, CRC(6ad52a63) SHA1(2a11e243b2281026e20ff5718c963100ea724889) )
	ROM_LOAD16_BYTE( "frnz10bd.p1", 0x000000, 0x080000, CRC(245c589c) SHA1(43075360eda8ad2ce1af180ea7501f799764de85) )
	ROM_LOAD16_BYTE( "frnz10d.p1", 0x000000, 0x080000, CRC(90c7886a) SHA1(f80bf1ed032ab0fca764b31b2d605865a39c42f2) )
	ROM_LOAD16_BYTE( "frnz10dy.p1", 0x000000, 0x080000, CRC(81e658a8) SHA1(bfc1de77bd5802c511a1a9e66c56849e93976904) )
	ROM_LOAD16_BYTE( "frnz10h.p1", 0x000000, 0x080000, CRC(9165efbf) SHA1(71a59f3fe37543b80e35128d4687338c0c7a9a93) )
	ROM_LOAD16_BYTE( "frnz10k.p1", 0x000000, 0x080000, CRC(b9598910) SHA1(9aa77587862e328be0d550fd1136ffdf24c4f15b) )
	ROM_LOAD16_BYTE( "frnz10r.p1", 0x000000, 0x080000, CRC(bc1ad6a2) SHA1(1018187f8811c52043edc2303e8290d72201cc3e) )
	ROM_LOAD16_BYTE( "frnz10s.p1", 0x000000, 0x080000, CRC(de4efa95) SHA1(ff1ebacd48ae9a4ab46bd4568f61bca8af63e811) )
	ROM_LOAD16_BYTE( "frnz10y.p1", 0x000000, 0x080000, CRC(cf6f2a57) SHA1(069e8c935fa455bc973d439d51ecf655d9717f4e) )
ROM_END

ROM_START( m5frnzya )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "frenzy.p1",  0x000000, 0x080000, NO_DUMP )
	ROM_LOAD16_BYTE( "frenzy.p2",  0x000001, 0x080000, CRC(ede2528d) SHA1(5e536648807e734d9a5dc4599cf0abf7c98c4163) )
	ROM_LOAD16_BYTE( "frenzy.p3",  0x100000, 0x080000, CRC(371069c4) SHA1(dbd192a7f20473ca52d85188ef5b964e33c5f271) )
	ROM_LOAD16_BYTE( "frenzy.p4",  0x100001, 0x080000, CRC(b8be736d) SHA1(7ca620ace6bb872239d9cf33189921caee64a75a) )
ROM_END

ROM_START( m5fnfair )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "funfair.p1", 0x000000, 0x080000, CRC(c4d609ca) SHA1(9095347962bf0d55e25f7bfb64e2c80ad9e482bc) )
	ROM_LOAD16_BYTE( "funfair.p2", 0x000001, 0x080000, CRC(2d58f56b) SHA1(a4415810dbea0e572751f17aacedaeb435eb7af7) )
	ROM_LOAD16_BYTE( "funfair.p3", 0x100000, 0x080000, CRC(6112054c) SHA1(48a9f4fa48d06ef848b2dbb75286002d4da330b9) )
	ROM_LOAD16_BYTE( "funfair.p4", 0x100001, 0x080000, CRC(60b56e14) SHA1(6a8a356e127178ce5abf105d47c970cb2c100561) )
ROM_END

ROM_START( m5fnfaird )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "f_fair.p1d", 0x000000, 0x080000, CRC(f0097df8) SHA1(3d17638b4ab812c29d366570732beeb67315be89) )
	ROM_LOAD16_BYTE( "f_fair.p2d", 0x000001, 0x080000, CRC(20d55e77) SHA1(c1158d7e95e7575debd341f8bdecaae44405e0ed) )
	ROM_LOAD16_BYTE( "funfair.p3", 0x100000, 0x080000, CRC(6112054c) SHA1(48a9f4fa48d06ef848b2dbb75286002d4da330b9) )
	ROM_LOAD16_BYTE( "funfair.p4", 0x100001, 0x080000, CRC(60b56e14) SHA1(6a8a356e127178ce5abf105d47c970cb2c100561) )
ROM_END

ROM_START( m5fusir )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fus10s.p1", 0x000000, 0x080000, CRC(3b793fb0) SHA1(0e203808c2017ff5a169b8c66569245fa6dc9651) )
	ROM_LOAD16_BYTE( "fus10s.p2", 0x000001, 0x080000, CRC(e4bb31de) SHA1(a7297fadbedd430e14d69fda6821bb50f3505d9f) )
	ROM_LOAD16_BYTE( "fus10s.p3", 0x100000, 0x080000, CRC(f75eba19) SHA1(8ead5bdd3c180acddd305dd46a5508141341c975) )
	ROM_LOAD16_BYTE( "fus10s.p4", 0x100001, 0x080000, CRC(cd85ac33) SHA1(be315f5a5163ac7190111555b7b00afe4c5bf6f8) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "fus10d.p1", 0x000000, 0x080000, CRC(bd916fbc) SHA1(0d437fd734fbf0fd0fe2b911950460257119375a) )
	ROM_LOAD16_BYTE( "fus10k.p1", 0x000000, 0x080000, CRC(1fbbae5c) SHA1(d42d107bc34deeacbdcf8580cc1757611fe7a39c) )
ROM_END


ROM_START( m5fusir11 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fus11s.p1", 0x000000, 0x080000, CRC(b6cbe454) SHA1(4c635dd384d7dc271d3730ab28f5d7ef304e3185) )
	ROM_LOAD16_BYTE( "fus11s.p2", 0x000001, 0x080000, CRC(13af10d5) SHA1(8f50de1003068070aef1e3d565a67b686d72eae9) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "fus11d.p1", 0x000000, 0x080000, CRC(3023b458) SHA1(d61b301fc2974a0219d8acde48b6768a1645cdcc) )
	ROM_LOAD16_BYTE( "fus11k.p1", 0x000000, 0x080000, CRC(920975b8) SHA1(826a28f3daa05f6d25c0639bb6048740cb7bad0c) )
ROM_END


ROM_START( m5fusir12 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fus12s.p1", 0x000000, 0x080000, CRC(75c0172e) SHA1(89f61431c595bad0434863bc1fdf4312b7a24e33) )
	ROM_LOAD16_BYTE( "fus12s.p2", 0x000001, 0x080000, CRC(eff4cfba) SHA1(7266e1c5a345ed6437f60eb2c364a4a0616526d8) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "fus12d.p1", 0x000000, 0x080000, CRC(f3284722) SHA1(fa110781727b3c8d80a1d3b105222d7ef33c8567) )
	ROM_LOAD16_BYTE( "fus12k.p1", 0x000000, 0x080000, CRC(510286c2) SHA1(68d3c5c1f3e73a3d3c24b06d9d34380a340e8af9) )
ROM_END

ROM_START( m5fmonty )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "full0_4.p1", 0x000000, 0x080000, CRC(d776135b) SHA1(def98d4a5bc1326bbf3215ed1b99878fd918acd3) )
	ROM_LOAD16_BYTE( "full0_4.p2", 0x000001, 0x080000, CRC(c8733632) SHA1(77be639cd3dff6f651818b7c1767e641d4e0408c) )
ROM_END

ROM_START( m5fmonty04a )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "full0_4m.p1", 0x000000, 0x080000, CRC(b00811be) SHA1(e29227fe856c6bf6f1ad342b7a0f5cce509ea906) )
	ROM_LOAD16_BYTE( "full0_4m.p2", 0x000000, 0x080000, CRC(c8733632) SHA1(77be639cd3dff6f651818b7c1767e641d4e0408c) )
ROM_END

ROM_START( m5fmonty04b )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "full0_4q.p1", 0x000000, 0x080000, CRC(f4d0b17e) SHA1(98bb1855298f6fe808a0e697f664fc0ec35e8f7c) )
	ROM_LOAD16_BYTE( "full0_4q.p2", 0x000000, 0x080000, CRC(718d454c) SHA1(9b9ee6bc464c6b20d083e2d3dfa2d63577bdfb46) )
ROM_END

ROM_START( m5fmonty04c )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "full0_4x.p1", 0x000000, 0x080000, CRC(ddffff04) SHA1(44410134011aaf672a9bbd2457aa9917136574aa) )
	ROM_LOAD16_BYTE( "full0_4x.p2", 0x000000, 0x080000, CRC(718d454c) SHA1(9b9ee6bc464c6b20d083e2d3dfa2d63577bdfb46) )
ROM_END



ROM_START( m5fmount )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fmou0_3.p1", 0x000000, 0x080000, CRC(c468aa38) SHA1(5f6b7178c7812afad87b8c89a0be94f9b68cb7ea) )
	ROM_LOAD16_BYTE( "fmou0_3.p2", 0x000001, 0x080000, CRC(a6d595c6) SHA1(6a1b0b158a6f1835f16b2f68ead69af6f5718b03) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "fmou0_3d.p1", 0x000000, 0x080000, CRC(beaf8805) SHA1(71004edef4366054ec69c67fa56662ccc6a91e8b) )
ROM_END


ROM_START( m5funsun )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fitr01d.p1", 0x000000, 0x080000, CRC(55e3f00d) SHA1(88948d92da93529cbc821377802a813f711b0379) )
	ROM_LOAD16_BYTE( "fitr01.p2", 0x000001, 0x080000, CRC(e0fb48ef) SHA1(1a57fb76e8a6684dc5e1b6a0e26c3186cee47fb4) )
	ROM_LOAD16_BYTE( "fitr01.p3", 0x100000, 0x080000, CRC(ba3ecd13) SHA1(aaf03ef6e7230a1ba88201540da019ad7ae573e6) )
	ROM_LOAD16_BYTE( "fitr01.p4", 0x100001, 0x080000, CRC(3aa10886) SHA1(0b69434e0d139e48c48da0a086b8728db27a01e7) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "fitr01dy.p1", 0x000000, 0x080000, CRC(8bfbb42f) SHA1(6c00554ef181d40e23cd50e511cee9c786416251) )
	ROM_LOAD16_BYTE( "fitr01s.p1", 0x000000, 0x080000, CRC(9554834b) SHA1(c63ee3fea33b3ed215bcc713526393a9d598f57f) )
	ROM_LOAD16_BYTE( "fitr01y.p1", 0x000000, 0x080000, CRC(4b4cc769) SHA1(b5d18d5fc7db269d56dde31260dbf10a4d9c4ae8) )
ROM_END

ROM_START( m5funsun03 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fits03s.p1", 0x000000, 0x080000, CRC(2057779f) SHA1(663147880ba1ddc28f57e4469d13eccdd5e959fc) )
	ROM_LOAD16_BYTE( "fits03.p2", 0x000001, 0x080000, CRC(5096b567) SHA1(154e26895bcbf9000cc9f4fb20f5796832aa4c7c) )
	ROM_LOAD16_BYTE( "fits03.p3", 0x100000, 0x080000, CRC(ba3ecd13) SHA1(aaf03ef6e7230a1ba88201540da019ad7ae573e6) ) // == 01
	ROM_LOAD16_BYTE( "fits03.p4", 0x100001, 0x080000, CRC(3aa10886) SHA1(0b69434e0d139e48c48da0a086b8728db27a01e7) ) // == 01

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "fits03ad.p1", 0x000000, 0x080000, CRC(01ce15f8) SHA1(8af226ce837dee4d470892c0a1c8b10769be4246) )
	ROM_LOAD16_BYTE( "fits03b.p1", 0x000000, 0x080000, CRC(fac8a7bf) SHA1(1e81c51bb318dc8f6ac8f1321a9be3a584edc61a) )
	ROM_LOAD16_BYTE( "fits03d.p1", 0x000000, 0x080000, CRC(23d82301) SHA1(7657a882e7e6df4db400fe0227b36f105e1be4ba) )
	ROM_LOAD16_BYTE( "fits03dy.p1", 0x000000, 0x080000, CRC(4a2dcad9) SHA1(11c47eb91f675d42f7122a6e120971977dd0c7c4) )
	ROM_LOAD16_BYTE( "fits03h.p1", 0x000000, 0x080000, CRC(43c3ee4e) SHA1(ba24ecd1bc0acc8230062063496ab82f7a268878) )
	ROM_LOAD16_BYTE( "fits03k.p1", 0x000000, 0x080000, CRC(a40d7159) SHA1(3fc076127caaba43fb28210ef226783daa456f1a) )
	ROM_LOAD16_BYTE( "fits03r.p1", 0x000000, 0x080000, CRC(4a10afaa) SHA1(733af64415b3bd9da140ed0776ca84e64bdeea01) )
	ROM_LOAD16_BYTE( "fits03y.p1", 0x000000, 0x080000, CRC(49a29e47) SHA1(144ef11dd2bc89a625a8ff4d3eeed62cae401784) )
ROM_END


ROM_START( m5ggems )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gge_20h2.2d1", 0x000000, 0x080000, CRC(7e161e06) SHA1(791e96d858038c5509f2513ad1f0bdebe7bd5be7) )
	ROM_LOAD16_BYTE( "gge_20h2.2d2", 0x000001, 0x080000, CRC(1803d961) SHA1(c9cf4c4004e42b2fdc9664756c54673fb72cd1c7) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "gge_sjs2.2_1", 0x000000, 0x080000, CRC(2589d1ec) SHA1(c44963790e0518a6dfe70e3190226620f7198168) )
	ROM_LOAD16_BYTE( "gge_sjh2.2d1", 0x000000, 0x080000, CRC(8a2b8ebe) SHA1(b89bf6a8778421cf891cce8274d01b168e434e68) )
	ROM_LOAD16_BYTE( "gge_sjk2.2_1", 0x000000, 0x080000, CRC(30e8d9aa) SHA1(cf70d3fb91d4f2f42af38293f218e14435f773f1) )
	ROM_LOAD16_BYTE( "gge_sjl2.2d1", 0x000000, 0x080000, CRC(f755ffda) SHA1(b53ba6d4c6dda468997d9d36e2896a9a52bf8c26) )

	// these just seem to be bad (sound) roms from something else
	//ROM_LOAD16_BYTE( "gia_gems.p1",  0x000000, 0x080000, CRC(c841d2f7) SHA1(480397141c917bce26cfdc8ba7fa8d21ab741770) )
	//ROM_LOAD16_BYTE( "gia_gems.p2",  0x000000, 0x080000, CRC(7824adf5) SHA1(99b59d28adb80d5cf3d70691cc134eba72288f20) )
	//ROM_LOAD16_BYTE( "gia_gems.p3",  0x000000, 0x008000, CRC(b5c915e1) SHA1(c716fa7cb6f4e01dc063cad9c42548f855d8440e) )
ROM_END

ROM_START( m5ggems20 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gge_sjs2.0_1", 0x000000, 0x080000, CRC(8687720c) SHA1(daeeac77d453d35ad0c5189930536b8399b00589) )
	ROM_LOAD16_BYTE( "gge_sjs2.0_2", 0x000001, 0x080000, CRC(6365f2fa) SHA1(d725a097469f82645cdee661a2d7a2705fbffe51) )
	ROM_LOAD16_BYTE( "gge_sjs2.0_3", 0x100000, 0x080000, CRC(9e98350b) SHA1(dc0b8fc77a4218b0d45b3d295cf1207556e20854) )
	ROM_LOAD16_BYTE( "gge_sjs2.0_4", 0x100001, 0x080000, CRC(8d7028c3) SHA1(9ffe1bc08ba09063fbd4d09c930d9b6e6cd9894e) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "gge_sjs2.0d1", 0x000000, 0x080000, CRC(a869fef6) SHA1(c0533674754e3712561a62dbb9a0777713883215) )
ROM_END

ROM_START( m5gimmie )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gidr01.p1", 0x000000, 0x080000, CRC(f4d07787) SHA1(63def64dfe22300d83202572b5772ba0f4f408a3) )
	ROM_LOAD16_BYTE( "gidr01.p2", 0x000001, 0x080000, CRC(841c7141) SHA1(a433c00bb473aa57aca7a5dc2f6c5661e98c2f99) )
	ROM_LOAD16_BYTE( "gidr01.p3", 0x100000, 0x080000, CRC(e1eb4113) SHA1(83dcf872a539c8e66b598f53262e45376cb5801c) )
	ROM_LOAD16_BYTE( "gidr01.p4", 0x100001, 0x080000, CRC(5d62e95f) SHA1(91327228238f3061e0bd93ecd5c564fc069aee6d) )
ROM_END


ROM_START( m5grush )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gldr02ad.p1", 0x000000, 0x080000, CRC(4b6b6fcf) SHA1(6bb2d278cb64ab384d4c09ddd8e99382e51b6f56) )
	ROM_LOAD16_BYTE( "gldr02.p2", 0x000001, 0x080000, CRC(80ad5e4b) SHA1(ff02457f24abdc710b1276a2fbd9d0a659007405) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "gldr02b.p1", 0x000000, 0x080000, CRC(02112427) SHA1(dd0376568179bac18c37c63124cd4c37bc73dca0) )
	ROM_LOAD16_BYTE( "gldr02bd.p1", 0x000000, 0x080000, CRC(32242750) SHA1(dd400b3261b0ab2ec026f28f86c45b97fea77e5d) )
	ROM_LOAD16_BYTE( "gldr02d.p1", 0x000000, 0x080000, CRC(46fe8e34) SHA1(d46838d930cec67c6f16fb9d1aee68872d499ae7) )
	ROM_LOAD16_BYTE( "gldr02dy.p1", 0x000000, 0x080000, CRC(24d3c2fe) SHA1(39fdc623fdf4109c1a5ab8f57bd28935ef9ac806) )
	ROM_LOAD16_BYTE( "gldr02r.p1", 0x000000, 0x080000, CRC(cdc86903) SHA1(0fa139fa83b9be893826fa7680bf60feab68c5fb) )
	ROM_LOAD16_BYTE( "gldr02s.p1", 0x000000, 0x080000, CRC(3bc119be) SHA1(436e5723402de85e2dc34ac3f75ffaeaed9ee18b) )
	ROM_LOAD16_BYTE( "gldr02y.p1", 0x000000, 0x080000, CRC(59ec5574) SHA1(e85f5dc2f2aa6abed9b57199e8ac9f63ab25500c) )
ROM_END

ROM_START( m5grush10 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gldr10s.p1", 0x000000, 0x080000, CRC(f1702b48) SHA1(50c9c88edd12c38506a7dfbcf0fac25cdb1c5979) )
	ROM_LOAD16_BYTE( "gldr10.p2", 0x000001, 0x080000, CRC(6688a3ee) SHA1(51bac67cc39ed55c4ca1e5f7904eacc76ce186ba) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "gldr10ad.p1", 0x000000, 0x080000, CRC(76bda2d8) SHA1(4295078c8143127e58d08da3490ee92572c67477) )
	ROM_LOAD16_BYTE( "gldr10b.p1", 0x000000, 0x080000, CRC(5eac62a0) SHA1(ca588ebd67e8d5bd0765c21e96c1c0ca26e83b24) )
	ROM_LOAD16_BYTE( "gldr10bd.p1", 0x000000, 0x080000, CRC(d2c160ff) SHA1(b23badc310d4484d5850c2b6434a1057b74c1c2e) )
	ROM_LOAD16_BYTE( "gldr10d.p1", 0x000000, 0x080000, CRC(30fee695) SHA1(e6af80e875b04cc3e3d57cb4c7b094027997beb9) )
	ROM_LOAD16_BYTE( "gldr10dy.p1", 0x000000, 0x080000, CRC(584abd42) SHA1(56a2ace6f2e2a825b5232b2a3e68276362bdcd4b) )
	ROM_LOAD16_BYTE( "gldr10r.p1", 0x000000, 0x080000, CRC(e563cd7f) SHA1(d0facc5904f55129ae328ece4775573af92b71d1) )
	ROM_LOAD16_BYTE( "gldr10y.p1", 0x000000, 0x080000, CRC(99c4709f) SHA1(45e5969873f797bcc52b725f1de7a5073f45ed0c) )
ROM_END

ROM_START( m5grush04 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "grsd04s.p1", 0x000000, 0x080000, CRC(feab336f) SHA1(aa5c0e5156428f10171d5cb18320608f02299713) )
	ROM_LOAD16_BYTE( "grsd04.p2", 0x000001, 0x080000, CRC(76539325) SHA1(9dd5ed167ed8aa5e69a6bcd036f1fdc51465e32b) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "grsd04ad.p1", 0x000000, 0x080000, CRC(048534ea) SHA1(80cf04c66b3bb7287f5fd73517d22e220e0eed14) )
	ROM_LOAD16_BYTE( "grsd04b.p1", 0x000000, 0x080000, CRC(1c799582) SHA1(753189da17aba0fb65fdca8c0fe3e6c3fc94b57b) )
	ROM_LOAD16_BYTE( "grsd04bd.p1", 0x000000, 0x080000, CRC(f4aaffd5) SHA1(09e21297cfd21c0eaab214fcebd97a4f2318cd0a) )
	ROM_LOAD16_BYTE( "grsd04d.p1", 0x000000, 0x080000, CRC(0bd25272) SHA1(81bd2c924acb38cfe60a06ebb667913e7b347f77) )
	ROM_LOAD16_BYTE( "grsd04dy.p1", 0x000000, 0x080000, CRC(96708ccb) SHA1(d784530d9369094abe909d1f5ac5a2efb874e5e9) )
	ROM_LOAD16_BYTE( "grsd04r.p1", 0x000000, 0x080000, CRC(f5d40d04) SHA1(47e28f392b7c6584057de92d206a9e50997a5705) )
	ROM_LOAD16_BYTE( "grsd04y.p1", 0x000000, 0x080000, CRC(6309edd6) SHA1(b804d30106d824f697ebeccb5858286846d054c5) )
ROM_END

ROM_START( m5grush03 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gldr03s.p1", 0x000000, 0x080000, CRC(1030dc21) SHA1(d824494fb378504225077bd4bac4dde8349f9d0b) )
	ROM_LOAD16_BYTE( "gldr03.p2", 0x000001, 0x080000, NO_DUMP )
ROM_END

ROM_START( m5grush02 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "grtr02.p1", 0x000000, 0x080000, CRC(95be0322) SHA1(688022cbbaf241dccc3b74951b5f03390a9e075c) )
	ROM_LOAD16_BYTE( "grtr02.p2", 0x000001, 0x080000, CRC(38898d47) SHA1(a5b171d2653d31c3e92aa1ab7093e4721e7621a7) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "grtr02ad.p1", 0x000000, 0x080000, CRC(8bd2d74d) SHA1(3f111cfd7f933d3222a706bc192cc18b7f23151f) )
	ROM_LOAD16_BYTE( "grtr02b.p1", 0x000000, 0x080000, CRC(c7f629b8) SHA1(839758636de9a9c23ecaad87d99845773d60bc61) )
	ROM_LOAD16_BYTE( "grtr02bd.p1", 0x000000, 0x080000, CRC(39fa0d64) SHA1(ff5aee25d026501967932297ac042be1ed9731f3) )
	ROM_LOAD16_BYTE( "grtr02d.p1", 0x000000, 0x080000, CRC(d260f6ba) SHA1(362a60cbcb470369cf0fe16ca755e2f01cafaaed) )
	ROM_LOAD16_BYTE( "grtr02dy.p1", 0x000000, 0x080000, CRC(b92628d4) SHA1(2ea29dde38849a96308192f2dc8e99e1c3e4b7b3) )
	ROM_LOAD16_BYTE( "grtr02k.p1", 0x000000, 0x080000, CRC(dc671ca8) SHA1(caddb6b3f5251f42a71255c0d9370db5974e1d60) )
	ROM_LOAD16_BYTE( "grtr02r.p1", 0x000000, 0x080000, CRC(a7af36f0) SHA1(c92e06e04e2fa2df721113884a9cc097bbde1d68) )
	ROM_LOAD16_BYTE( "grtr02s.p1", 0x000000, 0x080000, CRC(95be0322) SHA1(688022cbbaf241dccc3b74951b5f03390a9e075c) )
ROM_END

ROM_START( m5grush01 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "grtr01y.p1", 0x000000, 0x080000, CRC(5b03d1ea) SHA1(7ac6c24aa421f024edb70e4a1a14b9f56c9f96f0) )
	ROM_LOAD16_BYTE( "grtr01.p2", 0x000001, 0x080000, NO_DUMP )
ROM_END




ROM_START( m5grush5 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "g5tr01s.p1", 0x000000, 0x080000, CRC(efb9c1ea) SHA1(9c89af90bcc4fd4fe140e0ab92c6190a047efdf7) )
	ROM_LOAD16_BYTE( "g5tr01.p2", 0x000001, 0x080000, CRC(fd371a85) SHA1(ea209ab3083db78e1018c884d02fd717a8661eb7) )
	ROM_LOAD16_BYTE( "g5tr01.p3", 0x100000, 0x080000, CRC(529b3d07) SHA1(527c35ee1170c94a95fcf69f15453fb9b2593615) )
	ROM_LOAD16_BYTE( "g5tr01.p4", 0x100001, 0x080000, CRC(af4c3e53) SHA1(6639e612052482a74a3661397d2f1bd23968c23e) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "g5tr01ad.p1", 0x000000, 0x080000, CRC(f0ff5dc5) SHA1(43005efa498d59de41ea89d29974f3b452566571) )
	ROM_LOAD16_BYTE( "g5tr01b.p1", 0x000000, 0x080000, CRC(bbd745fd) SHA1(fc61cdf178234340e1db4ef654f6731c5936c9f8) )
	ROM_LOAD16_BYTE( "g5tr01bd.p1", 0x000000, 0x080000, CRC(d6a1ca6b) SHA1(b95ff28595e6395cca52f885788a5ba12e189a79) )
	ROM_LOAD16_BYTE( "g5tr01d.p1", 0x000000, 0x080000, CRC(c377f374) SHA1(139c68bb162fba3fe0b26da68fb4ba79bbb8824c) )
	ROM_LOAD16_BYTE( "g5tr01dy.p1", 0x000000, 0x080000, CRC(00a13991) SHA1(90614a3d2182f98bc3522a28e78a7a9f694f15d8) )
	ROM_LOAD16_BYTE( "g5tr01k.p1", 0x000000, 0x080000, CRC(109ac66c) SHA1(b80f17626a620e10be52d367d4bf4aeea8fb1ad4) )
	ROM_LOAD16_BYTE( "g5tr01y.p1", 0x000000, 0x080000, CRC(2c6f0b0f) SHA1(e49fe2f57fc33552d706b8629253e0955923cb1e) )
ROM_END

ROM_START( m5grush504 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "grfl04s.p1", 0x000000, 0x080000, CRC(9150b119) SHA1(4ca0228f65dea9dac9525ceab34c0951a8bf0322) )
	ROM_LOAD16_BYTE( "grfl04.p2", 0x000001, 0x080000, CRC(d27fae8b) SHA1(4ccf6f7e99aef7ac1fe39254ea0a6202a7d30e56) )
	ROM_LOAD16_BYTE( "grfl04.p3", 0x100000, 0x080000, CRC(c21685d5) SHA1(a7bcc3024960f4db51d14b57dcd1073ed602b107) )
	ROM_LOAD16_BYTE( "grfl04.p4", 0x100001, 0x080000, CRC(3daa1bf3) SHA1(fafe2b5b6632a54f438e552f15ebcaf483c0607b) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "grfl04ad.p1", 0x000000, 0x080000, CRC(79dec127) SHA1(e8c97c35d458fe3aed62a8fd5f88e08d55315fe7) )
	ROM_LOAD16_BYTE( "grfl04b.p1", 0x000000, 0x080000, CRC(0fabe21b) SHA1(d5d5a6c93d009ffea7aa5b3e2da8da22df7e7959) )
	ROM_LOAD16_BYTE( "grfl04bd.p1", 0x000000, 0x080000, CRC(d7d523d6) SHA1(668bc8bc0649041d4db6dd421ca072f1fd1370cf) )
	ROM_LOAD16_BYTE( "grfl04d.p1", 0x000000, 0x080000, CRC(ba12b0fd) SHA1(043c27be7d5004f0c9932f4f3342339f830cd8bf) )
	ROM_LOAD16_BYTE( "grfl04dy.p1", 0x000000, 0x080000, CRC(ae039b83) SHA1(e58c902a8b9f2503c1ba50ad7e74826b5bfc0273) )
	ROM_LOAD16_BYTE( "grfl04r.p1", 0x000000, 0x080000, CRC(bca46ee5) SHA1(f365d9e2bd1fea4013e9d0a89b058caacb079442) )
	ROM_LOAD16_BYTE( "grfl04y.p1", 0x000000, 0x080000, CRC(85419a67) SHA1(de985d51436e09ddfd603385c255b1f15095e866) )
ROM_END


ROM_START( m5gruss )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rstr01ad.p1", 0x000000, 0x080000, CRC(a2ffa9ef) SHA1(61bd3575799f6257d8d963a9196efc8696540fda) )
	ROM_LOAD16_BYTE( "rstr01.p2", 0x000001, 0x080000, CRC(1eaee2a7) SHA1(4b36918f512f184a13cca9dcdf613f2f38847dbd) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "rstr01b.p1", 0x000000, 0x080000, CRC(b1e2d945) SHA1(2c1574d3775c25e1052f025d4ab33d51548cf9b9) )
	ROM_LOAD16_BYTE( "rstr01bd.p1", 0x000000, 0x080000, CRC(45b91148) SHA1(92a6e7f763ae7b7113f7f3f3144ae0c48cfaa9d5) )
	ROM_LOAD16_BYTE( "rstr01d.p1", 0x000000, 0x080000, CRC(9396228a) SHA1(33076698afc6999d405a2fb4d9422e58a57cdc21) )
	ROM_LOAD16_BYTE( "rstr01dy.p1", 0x000000, 0x080000, CRC(6430a0a2) SHA1(f770aab88ba9e418dc3b9ad02b1d9da2b6ee6e07) )
	ROM_LOAD16_BYTE( "rstr01k.p1", 0x000000, 0x080000, CRC(7712e457) SHA1(390e133b2e9f97eb3328778a51888d3b8d767e9a) )
	ROM_LOAD16_BYTE( "rstr01s.p1", 0x000000, 0x080000, CRC(2bb58fe4) SHA1(57f6e3767bd306341fb05893711f15a622fb7ad1) )
	ROM_LOAD16_BYTE( "rstr01y.p1", 0x000000, 0x080000, CRC(dc130dcc) SHA1(89c8f9053417dd71df4063429e504e081b3dd702) )
ROM_END


ROM_START( m5grusst )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "grst05.p1", 0x000000, 0x080000, CRC(67e3a3a9) SHA1(35ee60697bb24eac77a0c90490e5155bc3e9aec3) )
	ROM_LOAD16_BYTE( "grst05.p2", 0x000001, 0x080000, CRC(ce2e7167) SHA1(999d16d947ecef74a4a7a655031a344918ec8a8b) )
ROM_END

ROM_START( m5grusst04 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "grtb04.p1", 0x000000, 0x080000, CRC(06bcea34) SHA1(2a53c35581c27c13584141ac37f3368ec5b45cc9) )
	ROM_LOAD16_BYTE( "grtb04.p2", 0x000001, 0x080000, CRC(640c300a) SHA1(b1973fd785e272dde56cd33bdf755dbe38079ec7) )
ROM_END

ROM_START( m5grusst03 ) // Gold Rush Stampede Three Player Stand Up (Barcrest) [Rom].zip....
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gr3b03.p1", 0x00000, 0x080000, CRC(4c8e3b99) SHA1(5e5b41433c9a1a0b1dbddbdebf1504cf54e0bb2f) )
	ROM_LOAD16_BYTE( "gr3b03.p2", 0x00001, 0x080000, CRC(4ddb0a6b) SHA1(62b9b828583e34fe993945d3c554085e3cf292b6) )
ROM_END

ROM_START( m5gstrik )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gost03ad.p1", 0x000000, 0x080000, CRC(e8716e1e) SHA1(240f3aba5eaf9f8b90787cd3f28665ad91fc1f44) )
	ROM_LOAD16_BYTE( "gost03.p2", 0x000001, 0x080000, CRC(74266727) SHA1(9fcadc768882fdbd5dc7208eae9b5ac13e5708b0) )
	ROM_LOAD16_BYTE( "gost03.p3", 0x100000, 0x080000, CRC(befbf489) SHA1(d198c27bafaddfd9c2c522be84b2a7aae52f712a) )
	ROM_LOAD16_BYTE( "gost03.p4", 0x100001, 0x080000, CRC(50e63933) SHA1(501caf831c11d603079807f3fd0570f0991a5f9c) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "gost03b.p1", 0x000000, 0x080000, CRC(1c4d7fa0) SHA1(25f38ffe450caa5b3005ae215dc745beb253c519) )
	ROM_LOAD16_BYTE( "gost03bd.p1", 0x000000, 0x080000, CRC(45e31179) SHA1(1c204f6ffaa95f42c79e6ff10231ee4bd6b2c40c) )
	ROM_LOAD16_BYTE( "gost03d.p1", 0x000000, 0x080000, CRC(9c00b5af) SHA1(20d16404bf4c4cdf0fac76b7a0ef10df360a4001) )
	ROM_LOAD16_BYTE( "gost03dy.p1", 0x000000, 0x080000, CRC(597da84f) SHA1(00942d779af48efb1bfe0e3501d8995275f47055) )
	ROM_LOAD16_BYTE( "gost03h.p1", 0x000000, 0x080000, CRC(7efddc09) SHA1(e98cfd11d6d8be0b414440dd876a8b9b2eefe676) )
	ROM_LOAD16_BYTE( "gost03r.p1", 0x000000, 0x080000, CRC(92cb81aa) SHA1(8ebb1654bcac8c971e6d73829c139696911bbe5c) )
	ROM_LOAD16_BYTE( "gost03s.p1", 0x000000, 0x080000, CRC(c5aedb76) SHA1(b4ee6ebc54262b0081b1a118cd1e7eae4223affd) )
	ROM_LOAD16_BYTE( "gost03y.p1", 0x000000, 0x080000, CRC(00d3c696) SHA1(f350a56df5c4f9c937d3b815428e8332fdf855f9) )
ROM_END

/* Dipswitches for Spanish Gold Strike:
_________________________________________________________________________________
|Bank 1 (on MPU5)|OFF                |ON                 |                      |
|----------------|-------------------|-------------------|----------------------|
|Switch 1        |                   |                   |Unused                |
|----------------|-------------------|-------------------|----------------------|
|Switch 2        |Disable > 10€ note |Accept 5€/10€ note |                      |
|----------------|-------------------|-------------------|----------------------|
|Switch 3        |Disable 20€ note   |Accept 20€ note    |Switch 3 has to be ON |
|----------------|-------------------|-------------------|----------------------|
|Switch 4        |Escrow enabled     |Escrow disabled    |                      |
|----------------|-------------------|-------------------|----------------------|
|Switch 5        |                                                              |
|----------------|                                                              |
|Switch 6        |                                                              |
|----------------|           See table below for Hopper Configurations          |
|Switch 7        |                                                              |
|----------------|                                                              |
|Switch 8        |                                                              |
|________________|______________________________________________________________|
___________________________________________________________________________________
|Hoppers  | 50+10 | 50+20 |100+10 |100+20 |200+10 |200+20 |200+100|100+100|200+200|
|---------|-------|-------|-------|-------|-------|-------|-------|-------|-------|
|Switch 5 |  OFF  |   ON  |  OFF  |   ON  |  OFF  |   ON  |  OFF  |   ON  |  OFF  |
|---------|-------|-------|-------|-------|-------|-------|-------|-------|-------|
|Switch 6 |  OFF  |  OFF  |   ON  |   ON  |  OFF  |  OFF  |   ON  |   ON  |  OFF  |
|---------|-------|-------|-------|-------|-------|-------|-------|-------|-------|
|Switch 7 |  OFF  |  OFF  |  OFF  |  OFF  |   ON  |   ON  |   ON  |   ON  |  OFF  |
|---------|-------|-------|-------|-------|-------|-------|-------|-------|-------|
|Switch 8 |  OFF  |  OFF  |  OFF  |  OFF  |  OFF  |  OFF  |  OFF  |  OFF  |   ON  |
|_________|_______|_______|_______|_______|_______|_______|_______|_______|_______|

_________________________________________________________________________________
|Bank 2 (on card)|OFF                |ON                 |                      |
|----------------|-------------------|-------------------|----------------------|
|Switch 1        |                                                              |
|----------------|                                                              |
|Switch 2        |                  See table below for Percentage              |
|----------------|                                                              |
|Switch 3        |                                                              |
|----------------|-------------------|-------------------|----------------------|
|Switch 4        |Disable show mode  |Enable show mode   |Show mode             |
|----------------|-------------------|-------------------|----------------------|
|Switch 5        |Enable             |Disable            |Bank transfer         |
|----------------|-------------------|-------------------|----------------------|
|Switch 6        |                   |                   |Reel anim             |
|----------------|-------------------|-------------------|----------------------|
|Switch 7        |Disable            |Enable             |Direct payout         |
|----------------|-------------------|-------------------|----------------------|
|Switch 8        |Disable            |Enable             |Auto hold             |
|________________|___________________|___________________|______________________|
____________________________________________________________
|Percentage| 70% | 73% | 76% | 78% | 80% | 82% | 84% | 86% |
|----------|-----|-----|-----|-----|-----|-----|-----|-----|
|Switch 1  | OFF |  ON | OFF |  ON | OFF |  ON | OFF |  ON |
|----------|-----|-----|-----|-----|-----|-----|-----|-----|
|Switch 2  | OFF | OFF |  ON |  ON | OFF | OFF |  ON |  ON |
|----------|-----|-----|-----|-----|-----|-----|-----|-----|
|Switch 3  | OFF | OFF | OFF | OFF |  ON |  ON |  ON |  ON |
|__________|_____|_____|_____|_____|_____|_____|_____|_____|

*/
ROM_START( m5gstriks )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bilso_b-14_gold_strike_v1.00_cvb_0200a_11-1562.p1", 0x000000, 0x080000, CRC(3ff69415) SHA1(ff01a2e688f66cc0e6b52e070cd2c558e54b69dc) ) // 27C4001 labeled as BILSO S.A. B-14 P1 GOLD STRIKE V 1.0 CVB-0200A / 11-1562
	ROM_LOAD16_BYTE( "bilso_b-14_gold_strike_v1.00_cvb_0200a_11-1562.p2", 0x000001, 0x080000, CRC(dd72a7a0) SHA1(d1d2a20ac9fea8bb46b0925ce941df2b58d37677) ) // 27C4001 labeled as BILSO S.A. B-14 P2 GOLD STRIKE V 1.0 CVB-0200A / 11-1562
	ROM_LOAD16_BYTE( "bilso_b-14_gold_strike_v1.00_cvb_0200a_11-1562.p3", 0x100000, 0x080000, CRC(d918a4dc) SHA1(501a9ee07bd5f7acfea42bb2d6559582a8dcba7e) ) // 27C4001 labeled as BILSO S.A. B-14 P3 GOLD STRIKE V 1.0 CVB-0200A / 11-1562
	ROM_LOAD16_BYTE( "bilso_b-14_gold_strike_v1.00_cvb_0200a_11-1562.p4", 0x100001, 0x080000, CRC(8bfa7a54) SHA1(18a7d88903696202a3981cd96381eb82703ade06) ) // 27C4001 labeled as BILSO S.A. B-14 P4 GOLD STRIKE V 1.0 CVB-0200A / 11-1562

	ROM_REGION( 0x00022e, "plds", 0 ) // All unprotected
	ROM_LOAD( "105_iciie_gal16v8b.ic2", 0x000000, 0x000117, CRC(c068d560) SHA1(76182850567adf92fc45767439d87184d8b09dac) )
	ROM_LOAD( "105_ici2a_gal16v8b.ic3", 0x000117, 0x000117, CRC(6e0c3f98) SHA1(634fb1894b217bfe620cd38bae2f17d4df475bad) )

	ROM_REGION( 0x000800, "nvram", 0 ) // RTC
	ROM_LOAD( "bilso_b-14_gold_strike_v1.00_cvb_0200a_11-1562.p7a", 0x000000, 0x000800, CRC(8942fba8) SHA1(e9264a386bd2f3fbb3c53434b94314d29c32b6f9) ) // ST M48T02-150PC1 dumped as DS1642

	ROM_REGION( 0x407, "pic", 0 ) // Decapped
	ROM_LOAD( "105_rgsg_pic16c54c.ic1", 0x000000, 0x407, CRC(2f910e58) SHA1(1d4857f25ec7db7da5ab29bc8be2f45aaacfca45))
ROM_END

ROM_START( m5gstrik11 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gost11s.p1", 0x000000, 0x080000, CRC(a44b737f) SHA1(b5ae06c8bcd053b46ca5b19d206b3fb6f146ddd8) )
	ROM_LOAD16_BYTE( "gost11.p2", 0x000001, 0x080000, CRC(8b778220) SHA1(136b95d43e521da8698a94e9bf827e120744c7c1) )
	ROM_LOAD16_BYTE( "gost11.p3", 0x100000, 0x080000, CRC(befbf489) SHA1(d198c27bafaddfd9c2c522be84b2a7aae52f712a) ) // == 03
	ROM_LOAD16_BYTE( "gost11.p4", 0x100001, 0x080000, CRC(50e63933) SHA1(501caf831c11d603079807f3fd0570f0991a5f9c) ) // == 03

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "gost11ad.p1", 0x000000, 0x080000, CRC(be12b3fb) SHA1(2f5846f8afa12e31404e1adad1b7241d65add54e) )
	ROM_LOAD16_BYTE( "gost11b.p1", 0x000000, 0x080000, CRC(4766cff8) SHA1(701b1ce621240fcf769f5769117cd5aea9d7587d) )
	ROM_LOAD16_BYTE( "gost11bd.p1", 0x000000, 0x080000, CRC(8defecfd) SHA1(77c4c72d5faf7da797df238bb8940ee6762701db) )
	ROM_LOAD16_BYTE( "gost11d.p1", 0x000000, 0x080000, CRC(6ec2507a) SHA1(13f8ff61369787165acf542fb77037d19fe83023) )
	ROM_LOAD16_BYTE( "gost11dy.p1", 0x000000, 0x080000, CRC(2bdc65e5) SHA1(872af59e86eb7812b35ddedd0c5bcede60009668) )
	ROM_LOAD16_BYTE( "gost11h.p1", 0x000000, 0x080000, CRC(fb2ef23a) SHA1(0bf2478cd18df7f9053c43552d0a72499514616d) )
	ROM_LOAD16_BYTE( "gost11k.p1", 0x000000, 0x080000, CRC(8101b8c3) SHA1(da194096b8d662bdc95b238b06e21b53be469131) )
	ROM_LOAD16_BYTE( "gost11r.p1", 0x000000, 0x080000, CRC(059e0329) SHA1(372382cbc6ab64f2a455b4a5ebced5b5ebed9243) )
	ROM_LOAD16_BYTE( "gost11y.p1", 0x000000, 0x080000, CRC(e15546e0) SHA1(8024c85cca4c09e1e526660d6ef1760c1bd6943c) )
ROM_END

ROM_START( m5gstrik02 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "g1tr02s.p1", 0x000000, 0x080000, CRC(3881b4f7) SHA1(dc3cdc56ca9df0a4dec83976f79c042368403beb) )
	ROM_LOAD16_BYTE( "g1tr02.p2", 0x000001, 0x080000, CRC(7d3118e9) SHA1(a4ccf46d15125c67e6e17b117bc3a880b46b47e7) )
	ROM_LOAD16_BYTE( "g1tr02.p3", 0x100000, 0x080000, CRC(befbf489) SHA1(d198c27bafaddfd9c2c522be84b2a7aae52f712a) ) // == 03
	ROM_LOAD16_BYTE( "g1tr02.p4", 0x100001, 0x080000, CRC(50e63933) SHA1(501caf831c11d603079807f3fd0570f0991a5f9c) ) // == 03

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "g1tr02dy.p1", 0x000000, 0x080000, CRC(e9c967ea) SHA1(1a2806e5b95e11aa5a80d7e434ebb40eba057aac) )
	ROM_LOAD16_BYTE( "g1tr02k.p1", 0x000000, 0x080000, CRC(d892e461) SHA1(72185d4c8e6670a0ec87f3807d4c465b1ea2ea89) )
	ROM_LOAD16_BYTE( "g1tr02y.p1", 0x000000, 0x080000, CRC(9d3f2d28) SHA1(fa0cc57f0e58489c2b8408c479525b99a2ca6ebd) )
ROM_END

ROM_START( m5gstrik01 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gotr01s.p1", 0x000000, 0x080000, CRC(b7b74ea3) SHA1(aededf597114db939f5b2ead31f04018383ad4bc) )
	ROM_LOAD16_BYTE( "gotr01.p2", 0x000001, 0x080000, CRC(000f6663) SHA1(f79f52c5d4af6cf576db9c529b868b34cd203026) )
	ROM_LOAD16_BYTE( "gotr01.p3", 0x100000, 0x080000, CRC(befbf489) SHA1(d198c27bafaddfd9c2c522be84b2a7aae52f712a) ) // == 03
	ROM_LOAD16_BYTE( "gotr01.p4", 0x100001, 0x080000, CRC(50e63933) SHA1(501caf831c11d603079807f3fd0570f0991a5f9c) ) // == 03

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "gotr01d.p1", 0x000000, 0x080000, CRC(171a5db8) SHA1(4b10705efafcc456c2a26f9a80bbb9f7c67cea9b) )
	ROM_LOAD16_BYTE( "gotr01dy.p1", 0x000000, 0x080000, CRC(715102e2) SHA1(af067bb2a80793391cb350e15f7f364f9ed3ee22) )
	ROM_LOAD16_BYTE( "gotr01k.p1", 0x000000, 0x080000, CRC(c57930cb) SHA1(3d24482169dc207b5d8ff9819fff8661df19bc79) )
	ROM_LOAD16_BYTE( "gotr01y.p1", 0x000000, 0x080000, CRC(d1fc11f9) SHA1(b62e59b3130a4a737c186f507e1a762c83740789) )
ROM_END

ROM_START( m5gstrik01a )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "g1tr01d.p1", 0x000000, 0x080000, CRC(cdfe3366) SHA1(22ef7acb1827d26562cb4c4371c42c9441208d9b) )
	ROM_LOAD16_BYTE( "g1tr01.p2", 0x000001, 0x080000, NO_DUMP )
	/* 3+4 */
ROM_END

ROM_START( m5gstrika )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "gold_st.p1", 0x00000, 0x080000, CRC(3bb4032f) SHA1(30d9b2160e9fc55b9691abbdfe32be3a548e94fa) )
	ROM_LOAD( "gold_st.p2", 0x00001, 0x080000, CRC(b890145f) SHA1(3351fe5926a30bf5b665ebd8bbcaaf5c74b4a218) )
	/* 3+4 */
ROM_END

ROM_START( m5gsstrk )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ss2501ad.p1", 0x000000, 0x080000, CRC(50ae38d5) SHA1(d20f3f3b8e6852aa83fb3c737b55635f0d550797) )
	ROM_LOAD16_BYTE( "ss2501.p2", 0x000001, 0x080000, CRC(e06cfa79) SHA1(a2d3361657bda57365eac3fd48479b3fdb9773b8) )
	ROM_LOAD16_BYTE( "ss2501.p3", 0x100000, 0x080000, CRC(9871de25) SHA1(53d243407422d9d45d8d1b749b9c098eba7d7ba0) )
	ROM_LOAD16_BYTE( "ss2501.p4", 0x100001, 0x080000, CRC(e782daee) SHA1(cfef53a8223dbb40ab4384d3796a53c06bbf6649) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "ss2501b.p1", 0x000000, 0x080000, CRC(78254183) SHA1(f3347bc4b414bc6b649d53780ca3157d97091d7c) )
	ROM_LOAD16_BYTE( "ss2501bd.p1", 0x000000, 0x080000, CRC(e7a23406) SHA1(c72f118f32ad9f2f2b19a6c8b455f44b44d9c404) )
	ROM_LOAD16_BYTE( "ss2501d.p1", 0x000000, 0x080000, CRC(904a509f) SHA1(fe6d63161d4cffd0adc835e10291d740f6f83021) )
	ROM_LOAD16_BYTE( "ss2501dy.p1", 0x000000, 0x080000, CRC(ed9773f8) SHA1(83a8cba23df19880ced9641496b1b8544be29207) )
	ROM_LOAD16_BYTE( "ss2501k.p1", 0x000000, 0x080000, CRC(bbb6b617) SHA1(ad2c7d08e531e5c7521fe6ec8f824f2d984b54f1) )
	ROM_LOAD16_BYTE( "ss2501r.p1", 0x000000, 0x080000, CRC(d22c63a3) SHA1(bf1bd5314ee1a5377c0028dd5c1cf248513b0885) )
	ROM_LOAD16_BYTE( "ss2501s.p1", 0x000000, 0x080000, CRC(b673ef82) SHA1(78061dcbc1552c26eb6cb357f05bf8b9f16b362a) )
	ROM_LOAD16_BYTE( "ss2501y.p1", 0x000000, 0x080000, CRC(cbaecce5) SHA1(478742d6850bbbbf83ce84471f952727b05db236) )
ROM_END

ROM_START( m5gsstrk07 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gstk07s.p1", 0x000000, 0x080000, CRC(f8a84259) SHA1(514bd4403ba464e732d9dded684c0a15c13811e4) )
	ROM_LOAD16_BYTE( "gstk07.p2", 0x000001, 0x080000, CRC(6a9baa5f) SHA1(c5ad8367ff449c2dc8a2a010917a95d80dabedfd) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "gstk07ad.p1", 0x000000, 0x080000, CRC(0e44d457) SHA1(35687eabb8141365454f9110a47662dd99de629e) )
	ROM_LOAD16_BYTE( "gstk07b.p1", 0x000000, 0x080000, CRC(7608df0d) SHA1(af75559a7d79fb38d50efa718b7674efb54affa6) )
	ROM_LOAD16_BYTE( "gstk07bd.p1", 0x000000, 0x080000, CRC(5eb69cac) SHA1(6fa06d50b8a1b6a16a4ece90001128d402dd5188) )
	ROM_LOAD16_BYTE( "gstk07d.p1", 0x000000, 0x080000, CRC(1c701e20) SHA1(1d74c0a48b6128969491df5ca7d081aa95de01fe) )
	ROM_LOAD16_BYTE( "gstk07dy.p1", 0x000000, 0x080000, CRC(2678a410) SHA1(4e27d63b872ee8ebd2e6c9801ad5a4f2474e4f41) )
	ROM_LOAD16_BYTE( "gstk07r.p1", 0x000000, 0x080000, CRC(3d5e5962) SHA1(642456567713f72f239ae12179abe2a93adb1f45) )
	ROM_LOAD16_BYTE( "gstk07y.p1", 0x000000, 0x080000, CRC(c2a0f869) SHA1(3105a6ff654bfdfed323312a9b6172f3c5da5e18) )
ROM_END


ROM_START( m5gdrag )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "drag04ad.p1", 0x000000, 0x080000, CRC(f0f03443) SHA1(a8f54f507b4aa429855edecbdfcecd457ec7c951) )
	ROM_LOAD16_BYTE( "drag04.p2", 0x000001, 0x080000, CRC(a84c22e4) SHA1(b1f2df1050bc17adef6df6b05f166107f615a534) )
	ROM_LOAD16_BYTE( "drag04.p3", 0x100000, 0x080000, CRC(72c3290b) SHA1(8ecd8950082b890744db22ed8e3f2bd51324e8a0) )
	ROM_LOAD16_BYTE( "drag04.p4", 0x100001, 0x080000, CRC(8882cc46) SHA1(d4ea11a4ea04f91b48b7038471b2c05ef6ed1136) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "drag04b.p1", 0x000000, 0x080000, CRC(c170f446) SHA1(ddbdb8c937f332573e602043ac1476f45f6e8d35) )
	ROM_LOAD16_BYTE( "drag04bd.p1", 0x000000, 0x080000, CRC(f7fbf665) SHA1(5d866c4c3ffd50e77a4c525a73eff0deff1ebd25) )
	ROM_LOAD16_BYTE( "drag04d.p1", 0x000000, 0x080000, CRC(0dfc8b2d) SHA1(8ed08ab228c35ff8a7f2c2ccb1944d1c5522ffac) )
	ROM_LOAD16_BYTE( "drag04dy.p1", 0x000000, 0x080000, CRC(b1e4a24c) SHA1(5563b0cc2f621bd78100860b224e0dbd321400c9) )
	ROM_LOAD16_BYTE( "drag04k.p1", 0x000000, 0x080000, CRC(b54308a8) SHA1(5c809d6a630d1fdad1c67bed652bb537dd6f7612) )
	ROM_LOAD16_BYTE( "drag04r.p1", 0x000000, 0x080000, CRC(417d823b) SHA1(27e3de5aebd51883c9e39ed6a1479f4af0346edb) )
	ROM_LOAD16_BYTE( "drag04s.p1", 0x000000, 0x080000, CRC(814046b3) SHA1(f124c7a38a160c94e6717300b986374d265dd1b5) )
	ROM_LOAD16_BYTE( "drag04y.p1", 0x000000, 0x080000, CRC(3d586fd2) SHA1(b77bcebca074173aeae27d1ce0f12d2eee4a2649) )
ROM_END


ROM_START( m5gdrgcl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gdgc04d.p1", 0x000000, 0x080000, CRC(513c1c5c) SHA1(470540549ddff631f6b9a1ce102d8d0b5d080927) )
	ROM_LOAD16_BYTE( "gdgc04.p2", 0x000001, 0x080000, CRC(0e8c60f7) SHA1(a3f2d56b1665d21fb69cf54704d5cae4dabb7f60) )
	ROM_LOAD16_BYTE( "gdgc04.p3", 0x100000, 0x080000, CRC(40e87b16) SHA1(0ecc3fb9ba87dac668b90bd02fc2cc92c68396ab) )
	ROM_LOAD16_BYTE( "gdgc04.p4", 0x100001, 0x080000, CRC(cc2d2806) SHA1(30fb319106c940998b576573308a01566e56bb23) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "gdgc04dz.p1", 0x000000, 0x080000, CRC(6f304873) SHA1(05021acc665e84d085d2544bab0c1ea90a42ff41) )
	ROM_LOAD16_BYTE( "gdgc04f.p1", 0x000000, 0x080000, CRC(ea80dd8d) SHA1(7323b6f904d47b840a3ccff634ac2f12f1b29728) )
	ROM_LOAD16_BYTE( "gdgc04s.p1", 0x000000, 0x080000, CRC(dfc898ed) SHA1(159fd9c14717eb3dd1bb25e4d44aff91c1c6aa1e) )
	ROM_LOAD16_BYTE( "gdgc04z.p1", 0x000000, 0x080000, CRC(91be4c46) SHA1(20bf32d99e7f3e8da04167649fe3a3e7f2993a77) )
ROM_END

ROM_START( m5gdrgcl05 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gdgc05s.p1", 0x000000, 0x080000, CRC(af4d242f) SHA1(35b8cbb6e8a299901f96231d1084dc94ec793139) )
	ROM_LOAD16_BYTE( "gdgc05.p2", 0x000001, 0x080000, CRC(68020883) SHA1(563b14c1c7bf5adac4c23eac10d01d17473a24f0) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "gdgc05d.p1", 0x000000, 0x080000, CRC(281b610b) SHA1(2660a246786418b04a975918abc12fd95b7ae0d4) )
	ROM_LOAD16_BYTE( "gdgc05dz.p1", 0x000000, 0x080000, CRC(891d128e) SHA1(7db770c90c7f13e490bf836372ae69d957c09840) )
	ROM_LOAD16_BYTE( "gdgc05f.p1", 0x000000, 0x080000, CRC(97fa9ce4) SHA1(4a7d0f7c90c72316337109b2aae8b77a226afa59) )
	ROM_LOAD16_BYTE( "gdgc05z.p1", 0x000000, 0x080000, CRC(7814760f) SHA1(05a8b3b332924356f5077bf64a5b23ab4226fa14) )
ROM_END

ROM_START( m5gkeys )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gok10_5s.p1", 0x000000, 0x080000, CRC(8a9caf39) SHA1(b19e03b94baff64dbc438fc13659bb4d864995cf) )
	ROM_LOAD16_BYTE( "gok10_5.p2", 0x000001, 0x080000, CRC(4aeba905) SHA1(f4886e061b339a7d2b4eb4bda9453497ab7f53a4) )
	ROM_LOAD16_BYTE( "gok10_5.p3", 0x100000, 0x080000, CRC(64de794b) SHA1(0d43081fe2bb03551086bb7a8274fd30e38114fd) )
	ROM_LOAD16_BYTE( "gok10_5.p4", 0x100001, 0x080000, CRC(435f897a) SHA1(96c1e75010ee3ae4816783951cc89c80c437a280) )
ROM_END


ROM_START( m5groll )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gro_sjs1.3_1", 0x000000, 0x080000, CRC(de34da42) SHA1(93078f08473e7452c24c59985099f97a3416ed2e) )
	ROM_LOAD16_BYTE( "gro_sjs1.3_2", 0x000001, 0x080000, CRC(e5c796ce) SHA1(31ae5b5212aaebc5d60231b1fa89c32a1cc79cb8) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "gro_86_1.3_1", 0x000000, 0x080000, CRC(8ca62449) SHA1(f299edc844bbe1a34333ecbdb83612d811ec5bfd) )
	ROM_LOAD16_BYTE( "gro_86_1.3d1", 0x000000, 0x080000, CRC(dbce6531) SHA1(7a429034676aa87e8a35044de2d9c90306dff8b1) )
	ROM_LOAD16_BYTE( "gro_ga1.3_1", 0x000000, 0x080000, CRC(994750eb) SHA1(597daafe0adbfefd4efe8b0c2125a5c4b0a71f95) )
	ROM_LOAD16_BYTE( "gro_ge1.3_1", 0x000000, 0x080000, CRC(a2fc7669) SHA1(7453882cea57295e7049cd4704ac7fbaaa5e1d0b) )
	ROM_LOAD16_BYTE( "gro_gg1.3_1", 0x000000, 0x080000, CRC(5e488721) SHA1(1ffb9aa8624ee54603313443129aefc76a7329e4) )
	ROM_LOAD16_BYTE( "gro_gj1.3_1", 0x000000, 0x080000, CRC(3dff7f4c) SHA1(96e96735e138314c0985a2a6a2a8d60af69e4cd5) )
	ROM_LOAD16_BYTE( "gro_sjs1.3d1", 0x000000, 0x080000, CRC(09cea678) SHA1(b32a86059cf05659c1e24eff6234423bc897af79) )
ROM_END


ROM_START( m5gophr )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "goph0_5.p1", 0x000000, 0x080000, CRC(172754b2) SHA1(f6f1113d53ceffda49f7abda5ebe64edb60f418b) )
	ROM_LOAD16_BYTE( "goph0_5.p2", 0x000001, 0x080000, CRC(bed741b6) SHA1(edb4f3fd7aec132220b4284a0860eeb12f12a2b4) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "goph0_5d.p1", 0x000000, 0x080000, CRC(4758a194) SHA1(f611cbb6c57b47f0ad916bb53cd9380d9972b702) )
ROM_END


ROM_START( m5gophcl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cgop0_6.p1", 0x000000, 0x080000, CRC(82873dc7) SHA1(1deaeae2dfeba6314dbe51ce69abb5d692e878f2) )
	ROM_LOAD16_BYTE( "cgop0_6.p2", 0x000001, 0x080000, CRC(1c447490) SHA1(8a5e309fa35f9aea1a946b6f97e88d2d01790674) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "cgop0_6d.p1", 0x000000, 0x080000, CRC(391c489a) SHA1(abeaf990a79d2952f721e3552375cc1c235359a9) )
ROM_END


ROM_START( m5hellrz )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hell04s.p1", 0x000000, 0x080000, CRC(a5bad966) SHA1(0daae4b8636820c65460df91a77816e19c96bb10) )
	ROM_LOAD16_BYTE( "hell0_4.p2", 0x000001, 0x080000, CRC(bbf49448) SHA1(5379be957ece17d302ca78ab72733a57f1033d97) )
	ROM_LOAD16_BYTE( "hell04.p3", 0x100000, 0x080000, CRC(5127afcf) SHA1(7c3141945bfb4491a7a32e940a3d392fed25e0a1) )
	ROM_LOAD16_BYTE( "hell04.p4", 0x100001, 0x080000, CRC(08ecd9b6) SHA1(85b4003cad75298fa844a3e6b1d7861426557ca5) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "hell0_4b.p1", 0x000000, 0x080000, CRC(e2530046) SHA1(a9f8d756157490e7059317622846987b43fc76c2) )
	ROM_LOAD16_BYTE( "hell0_4d.p1", 0x000000, 0x080000, CRC(ffd3abcf) SHA1(4ca0c85dd0461c3e658214643df8f482342402b3) )
	ROM_LOAD16_BYTE( "hell0_4k.p1", 0x000000, 0x080000, CRC(5d73c50e) SHA1(e15066be54d50f16226dca61aec73e0af3585bed) )
	ROM_LOAD16_BYTE( "hell0_4r.p1", 0x000000, 0x080000, CRC(8665c679) SHA1(659ef38ae82c18ad914b860cc607db2bd4e5a815) )
	ROM_LOAD16_BYTE( "hell0_4y.p1", 0x000000, 0x080000, CRC(b8cea2cb) SHA1(a99354ab11d2cc7fa3437d4ecf26cdd5cf81e993) )
ROM_END


ROM_START( m5hlsumo )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sumo03ad.p1", 0x000000, 0x080000, CRC(b4f5e848) SHA1(e0547e245397433b49991befa8dce23568f07915) )
	ROM_LOAD16_BYTE( "sumo03.p2", 0x000001, 0x080000, CRC(bdde4347) SHA1(433b50a2ac81d87efd0eb85362418097285b6954) )
	ROM_LOAD16_BYTE( "sumo03.p3", 0x100000, 0x080000, CRC(b0ab15ee) SHA1(11ce56497cd0f043c12db960e8e309fd0f7c9784) )
	ROM_LOAD16_BYTE( "sumo03.p4", 0x100001, 0x080000, CRC(18a61199) SHA1(08bca99b82580c8fa22436ef5964c1637b10983d) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "sumo03b.p1", 0x000000, 0x080000, CRC(6cf91bba) SHA1(ada9d15fe054c62641ebe0a42a376808e227d746) )
	ROM_LOAD16_BYTE( "sumo03bd.p1", 0x000000, 0x080000, CRC(8995f157) SHA1(202d30767a94951f98a28db97de934145c53f467) )
	ROM_LOAD16_BYTE( "sumo03d.p1", 0x000000, 0x080000, CRC(8eb14e6f) SHA1(d5291b70f43acf847caed8cf11f7a77d79834f77) )
	ROM_LOAD16_BYTE( "sumo03dy.p1", 0x000000, 0x080000, CRC(7bb4baec) SHA1(ddcc2756fe83524378a48da8056f875da0a3b00f) )
	ROM_LOAD16_BYTE( "sumo03h.p1", 0x000000, 0x080000, CRC(e672737c) SHA1(0a7738dc63c414f376cab27968c8db3f771ccc98) )
	ROM_LOAD16_BYTE( "sumo03r.p1", 0x000000, 0x080000, CRC(9424c9ed) SHA1(0f146023186df5da47269dc46b1c25ee0bcdd435) )
	ROM_LOAD16_BYTE( "sumo03s.p1", 0x000000, 0x080000, CRC(6bdda482) SHA1(5b068905253f7c58bea323154b4aee8ebc3003e0) )
	ROM_LOAD16_BYTE( "sumo03y.p1", 0x000000, 0x080000, CRC(9ed85001) SHA1(635005e4288cec6ab76044fce4e2156bdef09c0a) )
ROM_END


ROM_START( m5hiclau )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hclsja_1.0_1", 0x000000, 0x080000, CRC(554f26f6) SHA1(50cd8d220ca1b8ba7cb5924838703b99a2bca9cc) )
	ROM_LOAD16_BYTE( "hclsja_1.0_2", 0x000001, 0x080000, CRC(68295452) SHA1(9fced831cb85c4d8d6082da227865c3d0b79d49f) )
	ROM_LOAD16_BYTE( "hclsja_1.0_3", 0x100000, 0x080000, CRC(9a586df6) SHA1(f0d9f9290d7cf945a873f2c3f5e277df3861cbfc) )
	ROM_LOAD16_BYTE( "hclsja_1.0_4", 0x100001, 0x080000, CRC(53523af2) SHA1(9a73a991f5aeca2e5306cba7fff361d258df7893) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "hclsjad1.0_1", 0x000000, 0x080000, CRC(205a9e34) SHA1(734bf7c57400d52ac977f5a696d109879a9b1fd7) )
	ROM_LOAD16_BYTE( "hclsjb_1.0_1", 0x000000, 0x080000, CRC(fdd3d6c1) SHA1(85dd7b2cecfef904b97bfecf768ca9a44f8fa18f) )
	ROM_LOAD16_BYTE( "hclsjbd1.0_1", 0x000000, 0x080000, CRC(88c66e03) SHA1(11ae91a0076dd42dca3ebc50b567e876c142faf8) )
	ROM_LOAD16_BYTE( "hclsjbg1.0_1", 0x000000, 0x080000, CRC(780912c3) SHA1(004a45c422f73a32b07daaff77177db2477983c6) )
	ROM_LOAD16_BYTE( "hclsjbt1.0_1", 0x000000, 0x080000, CRC(ba181214) SHA1(d80ef01b1b54d8c31073fd52fa32c2f8f5d488f1) )
ROM_END


ROM_START( m5hifly )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hfly05ad.p1", 0x000000, 0x080000, CRC(b194b4d9) SHA1(64c8a61b2dfc68b4c9c68f787c2feff1d4545b17) )
	ROM_LOAD16_BYTE( "hfly05.p2", 0x000001, 0x080000, CRC(1e0eca95) SHA1(ec15b7d6aaa14823dcecf5fec7a3c55f81bbd6a3) )
	ROM_LOAD16_BYTE( "hfly05.p3", 0x100000, 0x080000, CRC(550b0228) SHA1(eb5ae4710b83aa1072fd3da68b12163fec5ebbea) )
	ROM_LOAD16_BYTE( "hfly05.p4", 0x100001, 0x080000, CRC(9a62c206) SHA1(d9f97ce751608bd51bc18525dbc4d40b2652a6fe) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "hfly05b.p1", 0x000000, 0x080000, CRC(0a51422b) SHA1(0b8f366ca403a2a2317185eb2db0e04be4acd665) )
	ROM_LOAD16_BYTE( "hfly05bd.p1", 0x000000, 0x080000, CRC(9afcfc65) SHA1(f85cf64db7e7f60076b9d0499f32a429297d8f9c) )
	ROM_LOAD16_BYTE( "hfly05d.p1", 0x000000, 0x080000, CRC(dfdfe8e0) SHA1(bb9b6d825b6f09d76a7a95de350e98f5ca0b1b73) )
	ROM_LOAD16_BYTE( "hfly05dy.p1", 0x000000, 0x080000, CRC(7706f480) SHA1(92a939902dc4a7ad3f26ccee43637bb7dcd82c35) )
	ROM_LOAD16_BYTE( "hfly05k.p1", 0x000000, 0x080000, CRC(62b90ec9) SHA1(201f7063cd7d4aacd229012d52030a1582b24806) )
	ROM_LOAD16_BYTE( "hfly05r.p1", 0x000000, 0x080000, CRC(8c1414f0) SHA1(b14ec649fd901dd8212f81481a1f872991b05357) )
	ROM_LOAD16_BYTE( "hfly05s.p1", 0x000000, 0x080000, CRC(fb6700e9) SHA1(874b7553e1e338d65d3a686c29e34d555bd0b68c) )
	ROM_LOAD16_BYTE( "hfly05y.p1", 0x000000, 0x080000, CRC(53be1c89) SHA1(699d6a30bdd332f29ca48f4eb4e8336039a873bb) )
ROM_END

ROM_START( m5hifly03 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hfly03s.p1", 0x000000, 0x080000, CRC(45fb2a8a) SHA1(e811ba052c004c3db2f7972c26d969e9180028c9) )
	ROM_LOAD16_BYTE( "hfly0_3.p2", 0x000001, 0x080000, CRC(eb2e557f) SHA1(a958db766de05884fdbc85611c539bfe78f19855) )
	ROM_LOAD16_BYTE( "hfly03.p3", 0x100000, 0x080000, CRC(550b0228) SHA1(eb5ae4710b83aa1072fd3da68b12163fec5ebbea) ) // == 05
	ROM_LOAD16_BYTE( "hfly03.p4", 0x100001, 0x080000, CRC(9a62c206) SHA1(d9f97ce751608bd51bc18525dbc4d40b2652a6fe) ) // == 05

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "hfly03r.p1", 0x000000, 0x080000, CRC(ccce8657) SHA1(b7b461efd78ee6d7c7fbff2c3586ec44dfec4427) )
	ROM_LOAD16_BYTE( "hfly0_3d.p1", 0x000000, 0x080000, CRC(0b6dbc3b) SHA1(3fb0861a2d97aa707957d94d99d938721037ad7e) )
	ROM_LOAD16_BYTE( "hfly0_3y.p1", 0x000000, 0x080000, CRC(662e8239) SHA1(6f2fc2cf89c9b3734154ae64875ade0cba4721ec) )
ROM_END

ROM_START( m5hifly04 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hfly04s.p1", 0x000000, 0x080000, CRC(e735d98c) SHA1(3dc314fd5a32b544acf19909a5f6d901111bf341) )
	ROM_LOAD16_BYTE( "hfly04.p2", 0x000001, 0x080000, CRC(08701661) SHA1(c947316204c547debc9f55cb9fd6d15c313d7670) )
	ROM_LOAD16_BYTE( "hfly04.p3", 0x100000, 0x080000, CRC(550b0228) SHA1(eb5ae4710b83aa1072fd3da68b12163fec5ebbea) ) // == 05
	ROM_LOAD16_BYTE( "hfly04.p4", 0x100001, 0x080000, CRC(9a62c206) SHA1(d9f97ce751608bd51bc18525dbc4d40b2652a6fe) ) // == 05

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "hfly04ad.p1", 0x000000, 0x080000, CRC(d9bd7c64) SHA1(408493cba5946854f82c819d3fbf55e702feea45) )
	ROM_LOAD16_BYTE( "hfly04b.p1", 0x000000, 0x080000, CRC(554df194) SHA1(56884e1f3371d6ee3b50f7f773dff47b097a6bc9) )
	ROM_LOAD16_BYTE( "hfly04bd.p1", 0x000000, 0x080000, CRC(8bb599b8) SHA1(0a978826c893a300ba8316dfc4d962af548b18ca) )
	ROM_LOAD16_BYTE( "hfly04d.p1", 0x000000, 0x080000, CRC(cca22a8d) SHA1(502c2279b5d6dbdc8b03e406668eb69e52ac6939) )
	ROM_LOAD16_BYTE( "hfly04dy.p1", 0x000000, 0x080000, CRC(42bad05f) SHA1(b96f9587e91915a8d0f1669fae2e4a4a1f537086) )
	ROM_LOAD16_BYTE( "hfly04k.p1", 0x000000, 0x080000, CRC(41a85902) SHA1(d3df84867ebb4781bc9720dfcdeee03d3ee854bd) )
	ROM_LOAD16_BYTE( "hfly04r.p1", 0x000000, 0x080000, CRC(c4de26f0) SHA1(b2ce57b3bfce33791f6df99a2a0711d5c7b65865) )
	ROM_LOAD16_BYTE( "hfly04y.p1", 0x000000, 0x080000, CRC(692d235e) SHA1(02573892a0db3bad78782192605f38d5463a4d4c) )
ROM_END



ROM_START( m5hisprt )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "high0_2.p1", 0x000000, 0x080000, CRC(c4cb5872) SHA1(be44695a36ac8aba6d75de144825e148c7d540c8) )
	ROM_LOAD16_BYTE( "high0_2.p2", 0x000001, 0x080000, CRC(6a237241) SHA1(143c87bfb29ce8a64e8a2c9cfb6829a57496ad22) )
	ROM_LOAD16_BYTE( "high0_2.p3", 0x100000, 0x080000, CRC(e3780d22) SHA1(9d0cff425aafa19b2bce45861ca192d5e0d80113) )
	ROM_LOAD16_BYTE( "high0_2.p4", 0x100001, 0x080000, CRC(c941a8ee) SHA1(7d64d3a9af23b13972d77cc038e2796dbbd9e016) )
ROM_END


ROM_START( m5hocus )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hopo0_4.p1", 0x000000, 0x080000, CRC(5c149373) SHA1(16dc4d27286d9dd20cb0a3a16fa09cdd99a93302) )
	ROM_LOAD16_BYTE( "hopo0_4.p2", 0x000000, 0x080000, CRC(b1621f06) SHA1(a7bcf92c54f417c0d333f75cc1fb7508249a137b) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "hopo0_4d.p1", 0x000000, 0x080000, CRC(6612e4a2) SHA1(a34d6d349b21ee782a409f4127022f7ddcb2dfff) )
ROM_END

ROM_START( m5hocus10 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hopo1_0.p1", 0x000000, 0x080000, CRC(3532dd8a) SHA1(c724f7b58bd3b393cfc6f8faf0bbb7c6a0bb2f75) )
	ROM_LOAD16_BYTE( "hopo1_0.p2", 0x000001, 0x080000, CRC(a1990fa1) SHA1(b8e12f279ef26cda5c685bee2f76047dfee79138) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "hopo1_0d.p1", 0x000000, 0x080000, CRC(fb913056) SHA1(78ff750a993b90a028baf0493ca948355ffc7e13) )
ROM_END


ROM_START( m5hocscl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "chop0_3.p1", 0x000000, 0x080000, CRC(c4168f88) SHA1(7b95320038e6243e6299f98f75bcb0ecabdedc5e) )
	ROM_LOAD16_BYTE( "chop0_3.p2", 0x000001, 0x080000, CRC(7d0eb1a4) SHA1(3fcf1b69122f0136974774c4cbe0d3a1916ae569) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "chop0_3d.p1", 0x000000, 0x080000, CRC(b8ed323f) SHA1(f32993686b6feca426bd3e6332da882b2a77d4fa) )
ROM_END


ROM_START( m5holy )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "holy05s.p1", 0x000000, 0x080000, CRC(aca48863) SHA1(69832c2d08a01cfd93ac23aaf82c51fdb6636b78) )
	ROM_LOAD16_BYTE( "holy05.p2", 0x000001, 0x080000, CRC(be3773ac) SHA1(31762b2b17d9c505184a760c8159f0d9558dc6b7) )
	ROM_LOAD16_BYTE( "holy05.p3", 0x100000, 0x080000, CRC(169f72ee) SHA1(ee8052d54fdc4d06d7cc2851c45af07eb4c080cd) )
	ROM_LOAD16_BYTE( "holy05.p4", 0x100001, 0x080000, CRC(15931f2b) SHA1(4692e58cc04375505c9238614896af070ea8d01a) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "holy05ad.p1", 0x000000, 0x080000, CRC(817c3cbe) SHA1(19eefa53c1d53f5aa2bac0286d57fbbc9ceabfbb) )
	ROM_LOAD16_BYTE( "holy05b.p1", 0x000000, 0x080000, CRC(29865d4c) SHA1(7cf5f5a73982a5c27fe146832582bf5d68420631) )
	ROM_LOAD16_BYTE( "holy05bd.p1", 0x000000, 0x080000, CRC(adb58904) SHA1(7639b3fcd123d782f3efed777aae40aa970e66e7) )
	ROM_LOAD16_BYTE( "holy05d.p1", 0x000000, 0x080000, CRC(641a84e0) SHA1(65c8ec073dfae4f1b2d5c95b5f66fe86296b7923) )
	ROM_LOAD16_BYTE( "holy05dy.p1", 0x000000, 0x080000, CRC(0be16c35) SHA1(57dc11440d83447c732261447f46b8f83fc862e0) )
	ROM_LOAD16_BYTE( "holy05k.p1", 0x000000, 0x080000, CRC(35aa76a4) SHA1(b0a1d500cd7112b6d2e5be1cdf386690f93a4c47) )
	ROM_LOAD16_BYTE( "holy05r.p1", 0x000000, 0x080000, CRC(3bf3359e) SHA1(b6f6d67ee4f00bba9c57c68317609a7bc857895f) )
	ROM_LOAD16_BYTE( "holy05y.p1", 0x000000, 0x080000, CRC(c35f60b6) SHA1(e7f4b5e83460bf4c0c5145d719a31a4af5b7216f) )
ROM_END

ROM_START( m5holy10 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hotr10s.p1", 0x000000, 0x080000, CRC(d08dbe97) SHA1(e17997d4f7460dcf62bda98a1d18d409a38f320a) )
	ROM_LOAD16_BYTE( "hotr10.p2", 0x000000, 0x080000, CRC(966bcfec) SHA1(d48e240b3cd566d188dba7adb660798cc39b39f8) )
	ROM_LOAD16_BYTE( "hotr10.p3", 0x000000, 0x080000, CRC(169f72ee) SHA1(ee8052d54fdc4d06d7cc2851c45af07eb4c080cd) ) // == 05
	ROM_LOAD16_BYTE( "hotr10.p4", 0x000000, 0x080000, CRC(15931f2b) SHA1(4692e58cc04375505c9238614896af070ea8d01a) ) // == 05

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "hotr10d.p1", 0x000000, 0x080000, CRC(1f279622) SHA1(0c56f0638f536618de31388a67cc57ceb75a85bd) )
	ROM_LOAD16_BYTE( "hotr10dy.p1", 0x000000, 0x080000, CRC(8b89a405) SHA1(e22c22696b246139c0c423dd909dc83ecf88c11a) )
	ROM_LOAD16_BYTE( "hotr10y.p1", 0x000000, 0x080000, CRC(44238cb0) SHA1(9f6bc48ba2ed100da14faa1513eab465f48eb81c) )
ROM_END

ROM_START( m5hopidl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hid_10s.p1", 0x000000, 0x080000, CRC(824fe7d2) SHA1(1599624dd1a6fc5009ac9ee1690df33e3b8e8dc7) )
	ROM_LOAD16_BYTE( "hid_10l.p2", 0x000001, 0x080000, CRC(b497154c) SHA1(c2132be453657a422bcc196f0da429bbcdd22276) )
	ROM_LOAD16_BYTE( "hid_10l.p3", 0x100000, 0x080000, CRC(64bf6387) SHA1(38cbaca79655c72fa6a3b0084a8133c9224235ac) )
	ROM_LOAD16_BYTE( "hid_10l.p4", 0x100001, 0x080000, CRC(29738e9f) SHA1(7407e5f08257c1db03f9ae6c9dc73ac8b462ad6f) )
ROM_END


ROM_START( m5hotsht )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hot07k.p1", 0x000000, 0x080000, CRC(bd5b10c7) SHA1(0343f5941e9676da4c1ef93f7549db2e4bcad404) )
	ROM_LOAD16_BYTE( "hot07k.p2", 0x000001, 0x080000, CRC(44be98ff) SHA1(6793d56e4b115c7c0027583a5461aeccb6abf89f) )
	ROM_LOAD16_BYTE( "hot05.p3", 0x100000, 0x080000, CRC(317fd043) SHA1(36e215813d2754d1e731dc443712f4cc520b0847) )
	ROM_LOAD16_BYTE( "hot05.p4", 0x100001, 0x080000, CRC(d430bbe8) SHA1(af560dada5265a8ae9143565b8d5c8aae54e1074) )
ROM_END

ROM_START( m5hotsht07a )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hot07s.p1", 0x000000, 0x080000, CRC(9999812b) SHA1(ffe25252fc0b5d7c018c70a3552f4e73f357f94b) )
	ROM_LOAD16_BYTE( "hot07s.p2", 0x000001, 0x080000, CRC(44be98ff) SHA1(6793d56e4b115c7c0027583a5461aeccb6abf89f) )
	/* 3+4 */
ROM_END

ROM_START( m5hotsht08 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hot08k.p1", 0x000000, 0x080000, CRC(c6fca638) SHA1(968e2b9296d9f6584fbe9d2d570a3ea25ad86f4d) )
	ROM_LOAD16_BYTE( "hot08k.p2", 0x000001, 0x080000, CRC(401b11a3) SHA1(06b5f940db3329b31a66a2940180e465a8ac3566) )
	/* 3+4 */
ROM_END

ROM_START( m5hotsht08a )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hot08s.p1", 0x000000, 0x080000, CRC(e23e37d4) SHA1(5f296a17878b5e67771b6269e97eeff2116f3ddd) )
	ROM_LOAD16_BYTE( "hot08s.p2", 0x000001, 0x080000, CRC(401b11a3) SHA1(06b5f940db3329b31a66a2940180e465a8ac3566) )
	/* 3+4 */
ROM_END

ROM_START( m5hotsht10 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hot10s.p1", 0x000000, 0x080000, CRC(d6361cae) SHA1(539104f7f7b0fb43bdb5c234f60b1926ac2ce6a9) )
	ROM_LOAD16_BYTE( "hot10s.p2", 0x000001, 0x080000, CRC(46285a1b) SHA1(15def8bdbd00e114792f8ce2ef4859ad0a5d0678) )
	/* 3+4 */
ROM_END

ROM_START( m5hotsht10a )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hot10k.p1", 0x000000, 0x080000, CRC(f2f48d42) SHA1(01ad7b95f2665cb2d59ff7a690034ed493f3fe44) )
	ROM_LOAD16_BYTE( "hot10s.p2", 0x000001, 0x080000, CRC(46285a1b) SHA1(15def8bdbd00e114792f8ce2ef4859ad0a5d0678) )
	/* 3+4 */
ROM_END



ROM_START( m5hotslt )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "htsl16ad.p1", 0x000000, 0x080000, CRC(279479e4) SHA1(82a9d602e97ea3ad38ad9dc25e859be5efd7757e) )
	ROM_LOAD16_BYTE( "htsl16.p2", 0x000001, 0x080000, CRC(e31bf855) SHA1(127150ffbd9ce0543a71a2503823b798a765c9ad) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "htsl13s", 0x000000, 0x080000, CRC(5ce85a9d) SHA1(c2e0ccf2903c046547e4b4757e9ddb524838d51e) )
	ROM_LOAD16_BYTE( "htsl16b.p1", 0x000000, 0x080000, CRC(6ff9aa90) SHA1(e06992d29774dbf99def7df9f351b4e136c5ebdb) )
	ROM_LOAD16_BYTE( "htsl16bd.p1", 0x000000, 0x080000, CRC(65e46a90) SHA1(9700979ab55d3d7523e21ea35eed6c313aa3087b) )
	ROM_LOAD16_BYTE( "htsl16d.p1", 0x000000, 0x080000, CRC(59066c05) SHA1(5a558983114d2a4feca707e0299983542a278582) )
	ROM_LOAD16_BYTE( "htsl16dy.p1", 0x000000, 0x080000, CRC(c448c558) SHA1(f705601facd97b6e050a2e8e16d82b16b92eec98) )
	ROM_LOAD16_BYTE( "htsl16r.p1", 0x000000, 0x080000, CRC(4698bb5d) SHA1(36672d8d933e99a1a1d65e072481a354d3e8affc) )
	ROM_LOAD16_BYTE( "htsl16s.p1", 0x000000, 0x080000, CRC(8bc2f797) SHA1(4e40539ef6a9390569c9158471adc0cc0cd5a48d) )
	ROM_LOAD16_BYTE( "htsl16y.p1", 0x000000, 0x080000, CRC(168c5eca) SHA1(f68d165925e7d2759c596955338118a895efe67a) )
ROM_END


ROM_START( m5hotstf )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hstf04ad.p1", 0x000000, 0x080000, CRC(eb8206fb) SHA1(5362e4c25d607c8a1a71023c274fea65fe6aa2d8) )
	ROM_LOAD16_BYTE( "hstf04.p2", 0x000001, 0x080000, CRC(aa01c1d2) SHA1(b8337b63cd63071cde9b8c4b4798a05312404882) )
	ROM_LOAD16_BYTE( "hstf04.p3", 0x100000, 0x080000, CRC(d79885ca) SHA1(1259d67367e36da98a7cd763179f8f114102e7ac) )
	ROM_LOAD16_BYTE( "hstf04.p4", 0x100001, 0x080000, CRC(1ac1df11) SHA1(d105cc09d23d1e106b1740fb9894cbdd6d070610) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "hstf04b.p1", 0x000000, 0x080000, CRC(1f38e2cc) SHA1(101f381b6da5b58f2ee6214e312068599f5d4633) )
	ROM_LOAD16_BYTE( "hstf04bd.p1", 0x000000, 0x080000, CRC(f3f5efaa) SHA1(12f95359ae93aca8d815785396cf43e5e85fc39a) )
	ROM_LOAD16_BYTE( "hstf04d.p1", 0x000000, 0x080000, CRC(5a3a0714) SHA1(476c3ada4e532546cef8dddfe925557d6abf64ca) )
	ROM_LOAD16_BYTE( "hstf04dy.p1", 0x000000, 0x080000, CRC(1c56d056) SHA1(1a25c2ec7e725875a5f88ff2f1c26475b39c046f) )
	ROM_LOAD16_BYTE( "hstf04k.p1", 0x000000, 0x080000, CRC(2d346858) SHA1(69f87a8ab09557335c09f36947aa8fff70fe8d29) )
	ROM_LOAD16_BYTE( "hstf04r.p1", 0x000000, 0x080000, CRC(c9859cc4) SHA1(77dadc6825abe8c593b15474de820ecfc7547740) )
	ROM_LOAD16_BYTE( "hstf04s.p1", 0x000000, 0x080000, CRC(43fc71c9) SHA1(c2acbd43355b3926a75a283f9a91c1f59fb6644c) )
	ROM_LOAD16_BYTE( "hstf04y.p1", 0x000000, 0x080000, CRC(0590a68b) SHA1(42fe07729eededb840eb466a4850e3f567907f40) )
ROM_END


ROM_START( m5hula )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hula0_2.p1", 0x000000, 0x080000, CRC(df4db504) SHA1(c2bc7e8b90282e782faa295c2cba7588b9f8b75f) )
	ROM_LOAD16_BYTE( "hula0_2.p2", 0x000001, 0x080000, CRC(56894430) SHA1(6fa830a4ffa581e64e6eda2ade5e42efea878ee3) )
	ROM_LOAD16_BYTE( "hula0_2.p3", 0x100000, 0x080000, CRC(b3ae2298) SHA1(7427ca3046311ec10e9bb7ce4520c615b3713a47) )
	ROM_LOAD16_BYTE( "hula0_2.p4", 0x100001, 0x080000, CRC(a3621910) SHA1(fe1cdb69cf92c072364c4159947c5e4585428f68) )
ROM_END

ROM_START( m5hula10 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hula1_0.p1", 0x000000, 0x080000, CRC(791c8f04) SHA1(bb93909915c66d0f01d55d6b5bb803ade13c5560) )
	ROM_LOAD16_BYTE( "hula1_0.p2", 0x000001, 0x080000, CRC(b2c69534) SHA1(bce8c18358bed68805cfe68397664d0cc221e384) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "hula1_0d.p1", 0x000000, 0x080000, CRC(e423761a) SHA1(12679fe5b9c9cbd35d453e0fef7cae610576c2d9) )
ROM_END

ROM_START( m5hulacl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "chul0_3.p1", 0x000000, 0x080000, CRC(d6b5d1be) SHA1(3b081246b352645e8348f92aee87e3110b4694e8) )
	ROM_LOAD16_BYTE( "chul0_3.p2", 0x000001, 0x080000, CRC(8cd125b8) SHA1(2bca710ca72f6bdd7fc919bab23d70ec19e015c8) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "chul0_3d.p1", 0x000000, 0x080000, CRC(11bac621) SHA1(68268bf4728f2809af015b3bcf0cb9a53c3cb695) )
ROM_END


ROM_START( m5hypalx )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hlx12s.p1", 0x000000, 0x080000, CRC(cb8982d8) SHA1(439ac3c2a77eecce6540c5549132afcbade57c92) )
	ROM_LOAD16_BYTE( "hlx12s.p2", 0x000001, 0x080000, CRC(b17ff37f) SHA1(6b65daf341cd73b1373e6275d24fbd44e3bf5372) )
	ROM_LOAD16_BYTE( "hlx12s.p3", 0x100000, 0x080000, CRC(6c80c6aa) SHA1(03861ec40c1a407c243d799eaf7ccd656aec1464) )
	ROM_LOAD16_BYTE( "hlx12s.p4", 0x100001, 0x080000, CRC(c0a4ef14) SHA1(443cd5ba96bb730f7fce2eabcacac209d7429e67) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "hlx12d.p1", 0x000000, 0x080000, CRC(4d61d2d4) SHA1(311d150b1ff7937d17a7417877f87724bb7b16d8) )
	ROM_LOAD16_BYTE( "hlx12k.p1", 0x000000, 0x080000, CRC(ef4b1334) SHA1(2b7ea4009c5dc9f247765139aed6000ebf80f3e7) )
ROM_END


ROM_START( m5hypvip )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hypv05.p1", 0x000000, 0x080000, CRC(d12ec607) SHA1(cd9e7ddcdfb94d389e2e0dbc7070d1381a4c9a70) )
	ROM_LOAD16_BYTE( "hypv05.p2", 0x000001, 0x080000, CRC(fe9fa73b) SHA1(c54de999bfe50a88f6b8996580b85db7a8880378) )
	ROM_LOAD16_BYTE( "hypv05.p3", 0x100000, 0x080000, CRC(be9cc186) SHA1(84ee431ce26ee82a53bef7c3495e623d5f2e7d53) )
	ROM_LOAD16_BYTE( "hypv05.p4", 0x100001, 0x080000, CRC(cc14fb81) SHA1(184bf4a5c95981155edef7fc576a9094d026213c) )
	ROM_LOAD16_BYTE( "hypv05.p5", 0x200000, 0x080000, CRC(cfa001c2) SHA1(c4607b66086d2407418f0b2c096d939e4419021a) )
	ROM_LOAD16_BYTE( "hypv05.p6", 0x200001, 0x080000, CRC(5a6358a7) SHA1(5ecd950e09a661cd485b0a6969b49cafbe616856) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "hypv05s.p1", 0x000000, 0x080000, CRC(d12ec607) SHA1(cd9e7ddcdfb94d389e2e0dbc7070d1381a4c9a70) )
ROM_END


ROM_START( m5hypno )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hyp_sjh1.3d1", 0x000000, 0x080000, CRC(20586f6a) SHA1(39f356ddb1e2b5097ec1de99aa57b997eefd1605) )
	ROM_LOAD16_BYTE( "hyp_20h1.3d2", 0x000001, 0x080000, CRC(faf04852) SHA1(99fa8a9c32687cd22bdcaab2e76540dfd6feb728) )
	ROM_LOAD16_BYTE( "hyp_20h1.3d3", 0x100000, 0x080000, CRC(089df79e) SHA1(97a1731cc2edc5dcd549c472930099df7a925d41) )
	ROM_LOAD16_BYTE( "hyp_20h1.3d4", 0x100001, 0x080000, CRC(77ee395e) SHA1(6b332ca2098146b846f19b8129444c46898064a8) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "hyp_sjk1.3_1", 0x000000, 0x080000, CRC(39758985) SHA1(39824d27175f044d81ab350e2fe7272e679d1267) )
	ROM_LOAD16_BYTE( "hyp_sjs1.3_1", 0x000000, 0x080000, CRC(3cb435ec) SHA1(b019b59819688002961346d70187d152eb2e8b3c) )
	ROM_LOAD16_BYTE( "hyp_sjs1.3d1", 0x000000, 0x080000, CRC(2599d303) SHA1(b0c27084308516ed5813426a45611cba780fe650) )
ROM_END


ROM_START( m5jackbx )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jack0_2.p1", 0x000000, 0x080000, CRC(d9e40c69) SHA1(0aace0228c590fbc29c34057dfaa86db4f554591) )
	ROM_LOAD16_BYTE( "jack0_2.p2", 0x000001, 0x080000, CRC(83ba85a9) SHA1(b79dc7fa0cb83b3495ade864520cf23b84f42cfc) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "jack0_2d.p1", 0x000000, 0x080000, CRC(e671d609) SHA1(ab2cfdb3c2596125e1ca9139425c4db4e871b1e7) )
ROM_END

ROM_START( m5jackbx03 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jack0_3.p1", 0x000000, 0x080000, CRC(b82cc7b3) SHA1(ad133b519fda3a6363731b16c0f14b3bd4d4887a) )
	ROM_LOAD16_BYTE( "jack0_3.p2", 0x000001, 0x080000, CRC(aa0b2d53) SHA1(25accf2255788755f424592d34b742308f30731f) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "jack0_3d.p1", 0x000000, 0x080000, CRC(c53eef26) SHA1(f7407a414e5a32e0dc52da22ae5700bf3b91f5be) )
	ROM_LOAD16_BYTE( "jack0_3m.p1", 0x000000, 0x080000, CRC(da51e273) SHA1(1002a0affc0321ccfd3f37a2e86642d3fe20b86c) )
ROM_END

ROM_START( m5jackpt )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jptr01d.p1", 0x000000, 0x080000, CRC(9e4fe90c) SHA1(a01fe44b4f6cef856ad1b463705e7d5a00ff4cb7) )
	ROM_LOAD16_BYTE( "jptr01.p2", 0x000001, 0x080000, CRC(c23e6171) SHA1(d922a811303c8e918712b85c9fcd4f142a872943) )
	ROM_LOAD16_BYTE( "jptr01.p3", 0x100000, 0x080000, CRC(28665fbd) SHA1(ee3963f6c77304251d6f721e982a8ff22ca449b6) )
	ROM_LOAD16_BYTE( "jptr01.p4", 0x100001, 0x080000, CRC(de478607) SHA1(0fcc0a910ce437aea4c41257d0eb7fa52084a180) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "j1tr01.p2", 0x000000, 0x080000, CRC(fefc764f) SHA1(996f5f19746049c6d7d643d719a1c4bdad30af5e) )
	//ROM_LOAD16_BYTE( "j1tr01.p3", 0x000000, 0x080000, CRC(28665fbd) SHA1(ee3963f6c77304251d6f721e982a8ff22ca449b6) )
	//ROM_LOAD16_BYTE( "j1tr01.p4", 0x000000, 0x080000, CRC(de478607) SHA1(0fcc0a910ce437aea4c41257d0eb7fa52084a180) )
	ROM_LOAD16_BYTE( "j1tr01d.p1", 0x000000, 0x080000, CRC(df118a21) SHA1(fcd3d2b25def90134521c9c4bbc0b2a6fdcf711e) )
	ROM_LOAD16_BYTE( "j1tr01dy.p1", 0x000000, 0x080000, CRC(500816c2) SHA1(e921841fffd84097a693eee2f0322adda5577464) )
	ROM_LOAD16_BYTE( "j1tr01k.p1", 0x000000, 0x080000, CRC(79da323a) SHA1(9e605065a6e53d941160e2581604ef24373d9c60) )
	ROM_LOAD16_BYTE( "j1tr01s.p1", 0x000000, 0x080000, CRC(683b44e6) SHA1(26623723aafef644bd3a2d6656b89d3fc299b60e) )
	ROM_LOAD16_BYTE( "j1tr01y.p1", 0x000000, 0x080000, CRC(e722d805) SHA1(ef2cab4464ca42fc90f78be421cb3de343f66cc9) )
	ROM_LOAD16_BYTE( "jptr01dy.p1", 0x000000, 0x080000, CRC(96770765) SHA1(7b4bb0e39df49f50d5b2d6a6b91e8a2be6610b1c) )
	ROM_LOAD16_BYTE( "jptr01s.p1", 0x000000, 0x080000, CRC(9dc42d56) SHA1(7c19edd5f300b2a8dd56a516459ab30c270613f5) )
	ROM_LOAD16_BYTE( "jptr01y.p1", 0x000000, 0x080000, CRC(95fcc33f) SHA1(8efad2cbadf9327d99a0bdde39ad919cdf6a3dc3) )
ROM_END

ROM_START( m5jackpt07 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "eers07s.p1", 0x000000, 0x080000, CRC(7de13003) SHA1(2e59f2d54f30c11315f2c74ff077e109e1bc0e2d) )
	ROM_LOAD16_BYTE( "eers07.p2", 0x000001, 0x080000, CRC(af0a522b) SHA1(04c8881931fa8165410cefbf1c7394057b42c004) )
	/* 3+4? */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "eers07ad.p1", 0x000000, 0x080000, CRC(7fc0aed5) SHA1(45fca729cc8b0c9f9d22a38200b1e76a996c5e50) )
	ROM_LOAD16_BYTE( "eers07b.p1", 0x000000, 0x080000, CRC(cf4ced92) SHA1(5d5d1da03f3c8b697fc251d955fd61730d9ca768) )
	ROM_LOAD16_BYTE( "eers07bd.p1", 0x000000, 0x080000, CRC(99b8b3f1) SHA1(80ec7c7a4fa0f7cbe0f4a390a556166a3855e5c8) )
	ROM_LOAD16_BYTE( "eers07dy.p1", 0x000000, 0x080000, CRC(3b9db5df) SHA1(8c73139db1824562cc495d70ac4c89697f16e4b5) )
	ROM_LOAD16_BYTE( "eers07h.p1", 0x000000, 0x080000, CRC(f7204ffc) SHA1(3d6946db64a4749628032ec361d91468ec15324b) )
	ROM_LOAD16_BYTE( "eers07r.p1", 0x000000, 0x080000, CRC(0d774597) SHA1(1292363c63aaf14f7707feaa8444646ff7a464f4) )
	ROM_LOAD16_BYTE( "eers07y.p1", 0x000000, 0x080000, CRC(6d69ebbc) SHA1(321fb10a1a6a88775474054c7a3cb9c69ee9eb20) )
ROM_END



ROM_START( m5jackp2 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "j2dr02s.p1", 0x000000, 0x080000, CRC(aa13dffe) SHA1(95f6c8b97bec9cf9f350d60f1c435de568f5763b) )
	ROM_LOAD16_BYTE( "j2dr02.p2", 0x000001, 0x080000, CRC(0272a760) SHA1(ec3079310a58c7b957de86c9cd3ae738d0ec1e8a) )
	ROM_LOAD16_BYTE( "j2dr02.p3", 0x100000, 0x080000, CRC(c64e1a8e) SHA1(e9d4bc7956722bb47fb33876c4f8eaf9e46dce63) )
	ROM_LOAD16_BYTE( "j2dr02.p4", 0x100001, 0x080000, CRC(706b2793) SHA1(2e371fa65a6f54c755c9cc83defd9ccf7d7f1793) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "jer202.p2", 0x000000, 0x080000, CRC(e61faecc) SHA1(6978697a3d4b3f3dc99e7b2aa0776e83544d216e) )
	//ROM_LOAD16_BYTE( "jer202.p3", 0x000000, 0x080000, CRC(c64e1a8e) SHA1(e9d4bc7956722bb47fb33876c4f8eaf9e46dce63) )
	//ROM_LOAD16_BYTE( "jer202.p4", 0x000000, 0x080000, CRC(706b2793) SHA1(2e371fa65a6f54c755c9cc83defd9ccf7d7f1793) )
	ROM_LOAD16_BYTE( "jer202ad.p1", 0x000000, 0x080000, CRC(74b2a1d7) SHA1(dbf31838543341d61c9a14d2cb79002ae4b8a516) )
	ROM_LOAD16_BYTE( "jer202b.p1", 0x000000, 0x080000, CRC(c2d7f7c7) SHA1(8a061f6dfdaaa7d09fcc1594ae74584e3071a9a7) )
	ROM_LOAD16_BYTE( "jer202bd.p1", 0x000000, 0x080000, CRC(52266c88) SHA1(5b5cf5cd2c21e7dbd6ed1ad5280c378ca0e42d15) )
	ROM_LOAD16_BYTE( "jer202d.p1", 0x000000, 0x080000, CRC(5d875582) SHA1(2a8cebd6b01e929a21da5307be7475f286ce9056) )
	ROM_LOAD16_BYTE( "jer202dy.p1", 0x000000, 0x080000, CRC(4116b153) SHA1(6dad595c78c97d6ec47237a48478128c9dbb0532) )
	ROM_LOAD16_BYTE( "jer202k.p1", 0x000000, 0x080000, CRC(64d5ed0f) SHA1(4b03dd75c8414c1c0315891196e0dfcd3e028374) )
	ROM_LOAD16_BYTE( "jer202r.p1", 0x000000, 0x080000, CRC(533d1258) SHA1(3b25dff9130c91a9dc66481687577e184284e4f2) )
	ROM_LOAD16_BYTE( "jer202s.p1", 0x000000, 0x080000, CRC(3c4f5471) SHA1(9b2da0f4031d6c184979f5b7a659f3f6de228232) )
	ROM_LOAD16_BYTE( "jer202y.p1", 0x000000, 0x080000, CRC(20deb0a0) SHA1(92f99e2e2788f7860f7058cc2e26d2422511d3be) )
ROM_END

ROM_START( m5jackp2a )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "j pot 2.p1", 0x00000, 0x080000, CRC(f2dddbee) SHA1(e6dbd0925998eb8574599b496996740c4af5b5c2) )
	ROM_LOAD16_BYTE( "j pot 2.p2", 0x00001, 0x080000, CRC(f37fa1c9) SHA1(03bfb641d04c98d375f548d074a1c44cb18f1a4b) )
	/* 3+4 */
ROM_END

ROM_START( m5jlyjwl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jjew08ad.p1", 0x000000, 0x080000, CRC(e9b86a4f) SHA1(f02351340b536671a553aadae132dbfab4375434) )
	ROM_LOAD16_BYTE( "jjew08.p2", 0x000001, 0x080000, CRC(6aa0a41c) SHA1(0cca3d51fb2daa3f2168ed7c846fd8f41ac90857) )
	ROM_LOAD16_BYTE( "jjew08.p3", 0x100000, 0x080000, CRC(5e5b19e0) SHA1(ba326b40ec4d8fd8f264938ec21e0c8666cb1085) )
	ROM_LOAD16_BYTE( "jjew08.p4", 0x100001, 0x080000, CRC(adffcdc2) SHA1(0d46cb6f35655e898d8b1bdc568cf926e2564725) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "jjew08b.p1", 0x000000, 0x080000, CRC(f65188f2) SHA1(b5b592db4283430d4fec4b0da47d935ae5856c2e) )
	ROM_LOAD16_BYTE( "jjew08bd.p1", 0x000000, 0x080000, CRC(cbbc1cf3) SHA1(7537b6e76cd59b4e2cb6b71d8dc1435aabd22b01) )
	ROM_LOAD16_BYTE( "jjew08d.p1", 0x000000, 0x080000, CRC(80b4825e) SHA1(449bd68d77f5c3d058d29a317363c1cf3939c33a) )
	ROM_LOAD16_BYTE( "jjew08dy.p1", 0x000000, 0x080000, CRC(ee30a000) SHA1(9a2e82ecf5dcc323a3d9393ccb19a7bdd543bb2e) )
	ROM_LOAD16_BYTE( "jjew08r.p1", 0x000000, 0x080000, CRC(d240f87e) SHA1(aeddc410409cadf294c58f2232a56ad9e44477d3) )
	ROM_LOAD16_BYTE( "jjew08s.p1", 0x000000, 0x080000, CRC(e60beff3) SHA1(9247be2107d67a369a9cb4ed2fa12b06c4bfddfb) )
	ROM_LOAD16_BYTE( "jjew08y.p1", 0x000000, 0x080000, CRC(888fcdad) SHA1(97959813cdf70e5edf51524fe8cbe23cedfba7fe) )
ROM_END

ROM_START( m5jlyjwl01 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jjtr01s.p1", 0x000000, 0x080000, CRC(2ef26a11) SHA1(9fd06ec5d2640acd9f181d80caf7887ea4d4a968) )
	ROM_LOAD16_BYTE( "jjtr0_1.p2", 0x000001, 0x080000, CRC(8bad641c) SHA1(2bebfdd2970ac2c0bd518a1bb66d6a55db271d21) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "jjtr01ad.p1", 0x000000, 0x080000, CRC(357bb3a6) SHA1(22153d92208946eb39e0b20231d6f4eb8e5ddc52) )
	ROM_LOAD16_BYTE( "jjtr01b.p1", 0x000000, 0x080000, CRC(611a03dc) SHA1(19adf916e0938efc5669a81fbe22d4b25f93378c) )
	ROM_LOAD16_BYTE( "jjtr01bd.p1", 0x000000, 0x080000, CRC(c1b2d846) SHA1(357fba36b94ee38b342a233f2b31fd6e60f5bb8b) )
	ROM_LOAD16_BYTE( "jjtr01d.p1", 0x000000, 0x080000, CRC(15615b1f) SHA1(8f63d37342d5209d9d490766250517504d96f1d0) )
	ROM_LOAD16_BYTE( "jjtr01dy.p1", 0x000000, 0x080000, CRC(12abcd67) SHA1(f616cb3beddf2447099f804bdfe94ec910699fb8) )
	ROM_LOAD16_BYTE( "jjtr01k.p1", 0x000000, 0x080000, CRC(ce336566) SHA1(089140d7be81cf79b496c3487bb558dd22ccd1e9) )
	ROM_LOAD16_BYTE( "jjtr01r.p1", 0x000000, 0x080000, CRC(d1ded7e9) SHA1(a207f7b1a6671615a8c9ff567405f69391cf071e) )
	ROM_LOAD16_BYTE( "jjtr01y.p1", 0x000000, 0x080000, CRC(2938fc69) SHA1(4437e4a1a7ff8e065b7edb7614e16762e8999faa) )
ROM_END

ROM_START( m5jlyjwl02 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jjtr0_2s.p1", 0x000000, 0x080000, CRC(ff853137) SHA1(4bdb2fa0e171e6fde371a7f40ab242fa8ab6cb3a) )
	ROM_LOAD16_BYTE( "jjtr0_2.p2", 0x000001, 0x080000, CRC(74cabc43) SHA1(86a8a5332a4dc32bb917335583fc2b99e1b5f4a1) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "jjtr0_2b.p1", 0x000000, 0x080000, CRC(5151f589) SHA1(672a5caaac2a1ee1af03464ec95f4779c094d902) )
	ROM_LOAD16_BYTE( "jjtr0_2d.p1", 0x000000, 0x080000, CRC(1eea4e2a) SHA1(eeb13db33831e339da61cb736ff45ec23a0ccbfd) )
	ROM_LOAD16_BYTE( "jjtr0_2k.p1", 0x000000, 0x080000, CRC(24ed4580) SHA1(3f0ece7c4264c9cf00710212c77e0033ebc15fcb) )
	ROM_LOAD16_BYTE( "jjtr0_2r.p1", 0x000000, 0x080000, CRC(481062e9) SHA1(11486b0499b191e6cbd4920b3e4e3c6a3e4b2f62) )
	ROM_LOAD16_BYTE( "jjtr0_2y.p1", 0x000000, 0x080000, CRC(23f8463e) SHA1(5b5fb325379cf0af1b946d630d032237a182adbf) )
ROM_END


ROM_START( m5jlyrog )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jolr0_1.p1", 0x000000, 0x080000, CRC(1001217e) SHA1(b6a4766426a88f8e1432c0af4e16344da8b4fa56) )
	ROM_LOAD16_BYTE( "jolr0_1.p2", 0x000001, 0x080000, CRC(dc501165) SHA1(0391f58392ca2c784ea17cf8cf23e0e48533b4a0) )
	ROM_LOAD16_BYTE( "jolr0_1.p3", 0x100000, 0x080000, CRC(640e9ee8) SHA1(b9fb8a84c076154cff1754fab8247fa140ab2f23) )
	ROM_LOAD16_BYTE( "jolr0_1.p4", 0x100001, 0x080000, CRC(eaac6094) SHA1(0ca89a956208caa8ab16e1bbfb267265ac167300) )
ROM_END

ROM_START( m5jlyroga )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jrtb0_1.p1", 0x000000, 0x080000, CRC(95980094) SHA1(04a3385c84267e56eb35b42c9cc5e26c80f68ff5) )
	ROM_LOAD16_BYTE( "jrtb0_1.p2", 0x000001, 0x080000, CRC(065cc5f1) SHA1(ddb5cca97405111988932fa25011f145cf9864ea) )
	/* 3+4? */
ROM_END

ROM_START( m5jmpjok )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jjosjs1.2_1", 0x000000, 0x080000, CRC(129ba7b5) SHA1(1debfae1e739776c8af3e234b6c5ae69a0261135) )
	ROM_LOAD16_BYTE( "jjosjs1.2_2", 0x000001, 0x080000, CRC(09cbc26b) SHA1(53a3b36f4916aba4181c4849fec9930a52c16c6c) )
	ROM_LOAD16_BYTE( "jjosjs1.2_3", 0x100000, 0x080000, CRC(8af362f5) SHA1(2c703c7568e8129dee7c36d09155688ea2405bb4) )
	ROM_LOAD16_BYTE( "jjosjs1.2_4", 0x100001, 0x080000, CRC(f102335a) SHA1(87b82475ad6c0d085082c08946330a63c9311da4) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "jjosjh1.2d1", 0x000000, 0x080000, CRC(0a4a302a) SHA1(f9aa8c03a967e2fc7a44f3dfe54b3d9b23dc2a98) )
	ROM_LOAD16_BYTE( "jjosjk1.2_1", 0x000000, 0x080000, CRC(aa88520b) SHA1(35501bee2155befcaededb212b330093aa083f55) )
	ROM_LOAD16_BYTE( "jjosjl1.2d1", 0x000000, 0x080000, CRC(c37a1ed1) SHA1(49702bae4b4db6dae88bbf62793e03244bf954fd) )
	ROM_LOAD16_BYTE( "jjosjs1.2d1", 0x000000, 0x080000, CRC(b259c594) SHA1(78afb8f6865d4e7c5ac5d975e7e2a4cf6c32be39) )
	ROM_LOAD16_BYTE( "jjo20h1.2d1", 0x000000, 0x080000, CRC(539e0ae9) SHA1(600d4843f88fcd4c8dc32ab1ec3afa96a621d844) )
ROM_END

ROM_START( m5jmpjok11 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jjo_20h1.1d1", 0x000000, 0x080000, CRC(a08eb689) SHA1(44c161b41e8a3b932195a0f3282a56fbde3298b8) )
	ROM_LOAD16_BYTE( "jjo___l1.1_2", 0x000001, 0x080000, CRC(57f1f30c) SHA1(fd290b25b693c18cbf5f923d86d59a1e585ab5a1) )
	ROM_LOAD16_BYTE( "jjo___l1.1_3", 0x100000, 0x080000, CRC(f7476095) SHA1(b7ba15a348337f5b19f7f37e6761a2b11c5bad66) )
	ROM_LOAD16_BYTE( "jjo___l1.1_4", 0x100001, 0x080000, CRC(e1038fb8) SHA1(4c75d039e40d91ca09dbb3851ce54b34b0f8b3d7) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "jjo_sjh1.1d1", 0x000000, 0x080000, CRC(6c572196) SHA1(c15d5b7ee89a4a37e7978dad71773428e854a857) )
	ROM_LOAD16_BYTE( "jjo_sjk1.1_1", 0x000000, 0x080000, CRC(ed07ce44) SHA1(cccc5d978325d5d1eeae14eca0501d992bfd8cd9) )
	ROM_LOAD16_BYTE( "jjo_sjl1.1d1", 0x000000, 0x080000, CRC(849c596b) SHA1(2a8e216316fdb48e400d703117c6dc88aa8b7c56) )
	ROM_LOAD16_BYTE( "jjo_sjs1.1_1", 0x000000, 0x080000, CRC(19223905) SHA1(27efa4d96b77c1ca5c39465a4ce34e26ff625468) )
	ROM_LOAD16_BYTE( "jjo_sjs1.1d1", 0x000000, 0x080000, CRC(9872d6d7) SHA1(50939a78084c6a0f94a7ffac6fe49bdb9646fff8) )
ROM_END

ROM_START( m5jmpjoka )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jj_dreg.p1", 0x00000, 0x080000, CRC(f6c8b395) SHA1(af815da23e8de9a91efc60134188827554991025) )//Deregulation update (new gaming rules)
	ROM_LOAD16_BYTE( "jj_dreg.p2", 0x00001, 0x080000, CRC(b4f79d90) SHA1(b59fa9b28c80d4ef23dc75d48e0be66100782595) )
	/* 3+4 */
ROM_END

ROM_START( m5jmpjokb )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "j_jokers.g1", 0x00000, 0x080000, CRC(e243c895) SHA1(9c114ea68ec4233da66d835fabe1f94534d4241f) )
	ROM_LOAD16_BYTE( "j_jokers.g2", 0x00001, 0x080000, CRC(b2762f2b) SHA1(d8e347fae336d1dd0daa5f908d61223f1cdbb675) )
	/* 3+4 */
ROM_END


ROM_START( m5jmpgem )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jgem0_4.p1", 0x000000, 0x080000, CRC(0f3b7687) SHA1(d8344e9737d6ff210d93a43545b9f6f19d40dae9) )
	ROM_LOAD16_BYTE( "jgem0_4.p2", 0x000001, 0x080000, CRC(9b56e0bc) SHA1(22d03e8fe741c6bb6125dfafff953f3fbfbf2f66) )
	ROM_LOAD16_BYTE( "jgem.p3", 0x100000, 0x080000, CRC(1074efce) SHA1(376912c0944f8053797dba6829ebdde6bae02a9a) )
	ROM_LOAD16_BYTE( "jgem.p4", 0x100001, 0x080000, CRC(6cc9a209) SHA1(ec2679fb78d63e7366e5f0098dfff28aa8070eb0) )
ROM_END

ROM_START( m5jmpgem01 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jgem0_1.p1", 0x000000, 0x080000, CRC(a046f6a7) SHA1(8f6bbbb1e12c8eaf51312ead677dc1de6c676cf9) )
	ROM_LOAD16_BYTE( "jgem0_1.p2", 0x000001, 0x080000, CRC(f933cc7d) SHA1(1b1f66365338c22e38e899e1660b6ea78aaa12bc) )
	/* 3+4 */
ROM_END

ROM_START( m5jmpgem03 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jgem0_3.p1", 0x000000, 0x080000, CRC(3de4286f) SHA1(dd40e5652db0130b05db7d3e4eecfaa83d13d8be) )
	ROM_LOAD16_BYTE( "jgem0_3.p2", 0x000001, 0x080000, CRC(2b048f0b) SHA1(a9a2156b1fcc3fe0e02c0c48babc9cf5cfde0f07) )
	/* 3+4 */
ROM_END



ROM_START( m5kaleid )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "kale0_1.p1", 0x000000, 0x080000, CRC(b1bf2bd0) SHA1(714a3551d62073ea3294b67513d1aee99d268e57) )
	ROM_LOAD16_BYTE( "kale0_1.p2", 0x000001, 0x080000, CRC(b6cfedec) SHA1(6dff4609f8ebdbf7a315e3d2750102e2a910623e) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "kale0_1d.p1", 0x000000, 0x080000, CRC(11f3b05b) SHA1(30c13821fdc082b9546b7786967caaec3a12a9c3) )
ROM_END


ROM_START( m5kcclb )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "kcob0_1.p1", 0x000000, 0x080000, CRC(79c488bc) SHA1(7868fdbe3d5070a0aa276e4e1f3253c868de98d7) )
	ROM_LOAD16_BYTE( "kcob0_1.p2", 0x000001, 0x080000, CRC(c4c0fe6d) SHA1(9946cdac0a5d338314ff0fbb6bb83fae81aac8fe) )
	ROM_LOAD16_BYTE( "kcob0_1.p3", 0x100000, 0x080000, CRC(7d9a453f) SHA1(6b8c3016f23149b7fe76b76e2cc7d4a9a7bc6e0c) )
	ROM_LOAD16_BYTE( "kcob0_1.p4", 0x100001, 0x080000, CRC(9ff61bef) SHA1(36ed1d3b6aa95f5bf1ead9c4c3c744cf49c739be) )
ROM_END

ROM_START( m5kcclb24 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "kcob2_4.p1", 0x000000, 0x080000, CRC(74c31371) SHA1(438411abd156ffa805787c24f46fe6680e998881) )
	ROM_LOAD16_BYTE( "kcob2_4.p2", 0x000001, 0x080000, CRC(d7341c1e) SHA1(a4871f40037c5a6b35a1831841f2993c94e82077) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "kcob2_4d.p1", 0x000000, 0x080000, CRC(85d2c34d) SHA1(ef2e9221183b5cfb326147dbc8c8dd05ce6b247d) )
ROM_END

ROM_START( m5kkebab )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "kbab11ad.p1", 0x000000, 0x080000, CRC(559c939f) SHA1(24bd747074d77c819846da67acca69921d156a56) )
	ROM_LOAD16_BYTE( "kbab11.p2", 0x000001, 0x080000, CRC(48d5e277) SHA1(4529c0c2d38cbef8e014f6272543ac70ee4e884d) )
	ROM_LOAD16_BYTE( "kbab11.p3", 0x100000, 0x080000, CRC(6da356d3) SHA1(03cfbda5a0c735e38bfa7761e9a9a1cf7d0d9611) )
	ROM_LOAD16_BYTE( "kbab11.p4", 0x100001, 0x080000, CRC(14f6db6e) SHA1(6540600ca92bdba575e8a7a37ef2a7db88a93a6d) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "kbab11b.p1", 0x000000, 0x080000, CRC(81248747) SHA1(95197cf5e67e8dda8b24339fbe6995ed0405ee39) )
	ROM_LOAD16_BYTE( "kbab11bd.p1", 0x000000, 0x080000, CRC(f94b0cbd) SHA1(989fb3db79793d0fa0cc4069288be8c810c144d2) )
	ROM_LOAD16_BYTE( "kbab11d.p1", 0x000000, 0x080000, CRC(b5e4f71d) SHA1(6f31ddf577942cf2dfdd7f17c52c0c3413f8292d) )
	ROM_LOAD16_BYTE( "kbab11dy.p1", 0x000000, 0x080000, CRC(2570d590) SHA1(c497c0583e7031b5c7a3673b7daf3470ab674852) )
	ROM_LOAD16_BYTE( "kbab11h.p1", 0x000000, 0x080000, CRC(7490ba85) SHA1(46df554e643c82ab1b8b212542a23dec6297cd84) )
	ROM_LOAD16_BYTE( "kbab11r.p1", 0x000000, 0x080000, CRC(1c196f38) SHA1(a45fbf1a2968ac46b856d1b9bcd5071d786cab93) )
	ROM_LOAD16_BYTE( "kbab11s.p1", 0x000000, 0x080000, CRC(cd8b7ce7) SHA1(45542e6e40554d065992dcce9b2feec7dea22660) )
	ROM_LOAD16_BYTE( "kbab11y.p1", 0x000000, 0x080000, CRC(5d1f5e6a) SHA1(75f641072c512d1bb7e4b22c384e2d6d09deac02) )
ROM_END

ROM_START( m5kkebab10 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "kktr10s.p1", 0x000000, 0x080000, CRC(8b09764b) SHA1(d846b5af8c4288fd821e2d29a50328fd50838e5b) )
	ROM_LOAD16_BYTE( "kktr10.p2", 0x000001, 0x080000, CRC(e08af965) SHA1(833d5151a5f2150d719dbee13daab6ab75402696) )
	ROM_LOAD16_BYTE( "kktr10.p3", 0x100000, 0x080000, CRC(6da356d3) SHA1(03cfbda5a0c735e38bfa7761e9a9a1cf7d0d9611) ) // == 11
	ROM_LOAD16_BYTE( "kktr10.p4", 0x100001, 0x080000, CRC(14f6db6e) SHA1(6540600ca92bdba575e8a7a37ef2a7db88a93a6d) ) // == 11

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "kktr10d.p1", 0x000000, 0x080000, CRC(5e2bd9a5) SHA1(3e5b3ea08018e45b511c784d9ffdc4619b7560f0) )
	ROM_LOAD16_BYTE( "kktr10dy.p1", 0x000000, 0x080000, CRC(bd759992) SHA1(cc72271b968d3530d6a4532d2c2fb66db055e142) )
	ROM_LOAD16_BYTE( "kktr10k.p1", 0x000000, 0x080000, CRC(bcb8ad4a) SHA1(8aebdede4092d2eb6135a9a0ea2ceb49a80887a4) )
	ROM_LOAD16_BYTE( "kktr10y.p1", 0x000000, 0x080000, CRC(6857367c) SHA1(f602b5ff4a6e716e76abe7fd462dcea07ccb1f92) )
ROM_END


ROM_START( m5kkebaba ) // doesn't appear to pair up with any of the above
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "kebab.p1",   0x000000, 0x080000, NO_DUMP )
	ROM_LOAD16_BYTE( "kebab.p2",   0x000001, 0x080000, CRC(660e8ecd) SHA1(a6c763643f3e3a2ffc9ff88675f71b3552337177) )//15GBP
ROM_END



ROM_START( m5kingko )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "kiko03ad.p1", 0x000000, 0x080000, CRC(b97b8c88) SHA1(437c8eaba56a78118956f9f9afca451e65ec8fb9) )
	ROM_LOAD16_BYTE( "kiko03.p2", 0x000001, 0x080000, CRC(6386dd86) SHA1(8fb58fd955b41c28534e581d725cdcfa685585e0) )
	ROM_LOAD16_BYTE( "kiko03.p3", 0x100000, 0x080000, CRC(de726f52) SHA1(5fedb40e7d4ce9b6df1e59044d132f7ccafd3d6b) )
	ROM_LOAD16_BYTE( "kiko03.p4", 0x100001, 0x080000, CRC(9fd99906) SHA1(d69357927781de46bc74f7c4643ff3fbf0e5baa3) )
	ROM_LOAD16_BYTE( "kiko03.p5", 0x200000, 0x080000, CRC(360268ea) SHA1(3c4ae25580ad4403c38185b0c73036e1c96d39ff) )
	ROM_LOAD16_BYTE( "kiko03.p6", 0x200001, 0x080000, CRC(e743872a) SHA1(068b5c6ad573a8fada7cc1761e42e6b729c45be4) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "kiko03b.p1", 0x000000, 0x080000, CRC(643d09a1) SHA1(0fd05c0c367a96343ccc6dc230c761f852e7abb7) )
	ROM_LOAD16_BYTE( "kiko03bd.p1", 0x000000, 0x080000, CRC(840aefe4) SHA1(b64a21d5a36a8080b7802f50271d9d4c0e40c64f) )
	ROM_LOAD16_BYTE( "kiko03d.p1", 0x000000, 0x080000, CRC(64b12707) SHA1(ae8c0b61d75b8494b388d01cc0d592921c480af2) )
	ROM_LOAD16_BYTE( "kiko03dy.p1", 0x000000, 0x080000, CRC(7b869ea9) SHA1(cacc797fe7a56fe33478abd8c4d0ed77247e68d8) )
	ROM_LOAD16_BYTE( "kiko03k.p1", 0x000000, 0x080000, CRC(afbe5c59) SHA1(f9291b13d0c9299c1e7f85b89e18e84f3de0011f) )
	ROM_LOAD16_BYTE( "kiko03s.p1", 0x000000, 0x080000, CRC(60f5de26) SHA1(5a9471d611b833fef55ec9c46711e1138655686d) )
	ROM_LOAD16_BYTE( "kiko03y.p1", 0x000000, 0x080000, CRC(7fc26788) SHA1(c228ec8c1f0e387c1e997b2bd9d3432b3f06d978) )

ROM_END


ROM_START( m5kingko04 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "kiko04s.p1", 0x000000, 0x080000, CRC(b89c10eb) SHA1(e163e078540cb74092bc02c6c0125598c88d7e1a) )
	ROM_LOAD16_BYTE( "kiko04.p2", 0x000001, 0x080000, CRC(ba4f3829) SHA1(103d01c7222a731180c32819c3d52e3cc0f79027) )
	ROM_LOAD16_BYTE( "kiko04.p3", 0x100000, 0x080000, CRC(de726f52) SHA1(5fedb40e7d4ce9b6df1e59044d132f7ccafd3d6b) ) // == 03
	ROM_LOAD16_BYTE( "kiko04.p4", 0x100001, 0x080000, CRC(9fd99906) SHA1(d69357927781de46bc74f7c4643ff3fbf0e5baa3) ) // == 03
	ROM_LOAD16_BYTE( "kiko04.p5", 0x200000, 0x080000, CRC(360268ea) SHA1(3c4ae25580ad4403c38185b0c73036e1c96d39ff) ) // == 03
	ROM_LOAD16_BYTE( "kiko04.p6", 0x200001, 0x080000, CRC(e743872a) SHA1(068b5c6ad573a8fada7cc1761e42e6b729c45be4) ) // == 03

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "kiko04ad.p1", 0x000000, 0x080000, CRC(b93367b9) SHA1(95348198c385abfed859acc3387c70bc65c88724) )
	ROM_LOAD16_BYTE( "kiko04b.p1", 0x000000, 0x080000, CRC(10babbb0) SHA1(f69db6cc600cc2ffa4f17fe95708ddc8013c800a) )
	ROM_LOAD16_BYTE( "kiko04bd.p1", 0x000000, 0x080000, CRC(0550cfb9) SHA1(8243ea9de21d85b25fb8ef8a74ea948e6eb94e32) )
	ROM_LOAD16_BYTE( "kiko04d.p1", 0x000000, 0x080000, CRC(c549b890) SHA1(7ad8a9fcc8a117529e1165838b6f0df959428089) )
	ROM_LOAD16_BYTE( "kiko04dy.p1", 0x000000, 0x080000, CRC(f4216299) SHA1(d8daed04dfc66865782ca6c928e53a3a9649fbf8) )
	ROM_LOAD16_BYTE( "kiko04k.p1", 0x000000, 0x080000, CRC(712730b4) SHA1(e8691e28faaf29d828202802708ac06398be45a9) )
	ROM_LOAD16_BYTE( "kiko04r.p1", 0x000000, 0x080000, CRC(6abb7c02) SHA1(4fc18731ba2a7e0e0fc98af6f9e04d3fa398d9e1) )
	ROM_LOAD16_BYTE( "kiko04y.p1", 0x000000, 0x080000, CRC(89f4cae2) SHA1(c1931c0df7a0ecf973f0cc4e26110d83a609e27a) )
ROM_END

ROM_START( m5kingko05 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "kiko05s.p1", 0x000000, 0x080000, CRC(343b7d78) SHA1(7ec367f88ad187dd01404c3c9b599f40ee426b99) )
	ROM_LOAD16_BYTE( "kiko05.p2", 0x000001, 0x080000, CRC(d4df0fd0) SHA1(23105c81aa13dd1273b0928b55d0fc1e1718e679) )
	/* 3,4,5,6 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "kiko05ad.p1", 0x000000, 0x080000, CRC(8e315607) SHA1(f6f251d7052153eb2c86591ee40c4c2ac74378f9) )
	ROM_LOAD16_BYTE( "kiko05b.p1", 0x000000, 0x080000, CRC(a02ee399) SHA1(4384b3ef46361ca2c4650387d7b22e427bb84671) )
	ROM_LOAD16_BYTE( "kiko05bd.p1", 0x000000, 0x080000, CRC(b7ce59c7) SHA1(24052cf89c4bb17b6ccbc5dcfa73429359fa9702) )
	ROM_LOAD16_BYTE( "kiko05d.p1", 0x000000, 0x080000, CRC(198bebf2) SHA1(ddaeb5720aca6690f1342be15486e7e1ce85c8b7) )
	ROM_LOAD16_BYTE( "kiko05dy.p1", 0x000000, 0x080000, CRC(35627217) SHA1(70f446572abc5990af22ad56693db8316d637e3e) )
	ROM_LOAD16_BYTE( "kiko05k.p1", 0x000000, 0x080000, CRC(d9f4b615) SHA1(a892cc7ee3fde63cf9d687636bec9eb6edc71d6d) )
	ROM_LOAD16_BYTE( "kiko05r.p1", 0x000000, 0x080000, CRC(8190c924) SHA1(d1dc59249387d78788d691bc1bbda367edf5c65d) )
	ROM_LOAD16_BYTE( "kiko05y.p1", 0x000000, 0x080000, CRC(18d2e49d) SHA1(4721c34403685ce7275c92f4b35dd6665ff1cc80) )
ROM_END


ROM_START( m5kingqc )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "king0_2.p1", 0x000000, 0x080000, CRC(b7a8155d) SHA1(efecd1be879a1cf433bdc18df93d304b105815e4) )
	ROM_LOAD16_BYTE( "king0_2.p2", 0x000001, 0x080000, CRC(81f4348f) SHA1(da05b0bda23a7e268c690469dee340c30be56f0f) )
	ROM_LOAD16_BYTE( "king0_2.p3", 0x100000, 0x080000, CRC(a6fb2bfa) SHA1(d7ed94fc7d7eb27d49861af7e2e79a7065db7f57) )
	ROM_LOAD16_BYTE( "king0_2.p4", 0x100001, 0x080000, CRC(da9c2148) SHA1(e178662775488d10830f0ac2af784f40c1f04ff0) )
ROM_END

ROM_START( m5kingqc06 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "king0_6.p1", 0x000000, 0x080000, CRC(eec01c60) SHA1(fae4e143932770afd64b6a1d6ec355daba503d2f) )
	ROM_LOAD16_BYTE( "king0_6.p2", 0x000001, 0x080000, CRC(38adc121) SHA1(164c3acdf9b47ca75d7ae30048fc6efa6991296e) )
	/* 3+4 */
ROM_END

ROM_START( m5kingqc07 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "king0_7.p1", 0x000000, 0x080000, CRC(8f30e512) SHA1(de8cc69c3c89169c19f9cd548938b685df8ddada) )
	ROM_LOAD16_BYTE( "king0_7.p2", 0x000001, 0x080000, CRC(c1a80819) SHA1(bc988be411c7b97a59ed0f0cdf83ec3cffa00490) )
	/* 3+4 */
ROM_END

ROM_START( m5kingqc08 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "king0_8.p1", 0x000000, 0x080000, CRC(a4ebbfc9) SHA1(0d23e2b0ad74a4002c294edaf61f52db8f79c5cb) )
	ROM_LOAD16_BYTE( "king0_8.p2", 0x000001, 0x080000, CRC(71b336f0) SHA1(7a114d404cd7a567f42ad926610c68049cdb7eb1) )
	/* 3+4 */
ROM_END


ROM_START( m5korma )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "korm0_3.p1", 0x000000, 0x080000, CRC(ec1e20e6) SHA1(0ea9aa5036a3dc3a1d2bd44f4b67b7afb35263d8) )
	ROM_LOAD16_BYTE( "korm0_3.p2", 0x000001, 0x080000, CRC(9635367f) SHA1(89cfbf6afd556fd0381b7cfd7e835130ead9031f) )
	ROM_LOAD16_BYTE( "korm0_3.p3", 0x100000, 0x080000, CRC(11049c22) SHA1(b535f7fc6b5d4f7ec31d80aba90b75d56f2abaae) )
	ROM_LOAD16_BYTE( "korm0_3.p4", 0x100001, 0x080000, CRC(d124e705) SHA1(959612f7fec32c09e7d46fb67e6dfcbcd7b7afe6) )
ROM_END

ROM_START( m5korma12 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "korm1_2.p1", 0x000000, 0x080000, CRC(8d08dc36) SHA1(79e499c33e9d34741b73cea556077e2a4cfaa632) )
	ROM_LOAD16_BYTE( "korm1_2.p2", 0x000001, 0x080000, CRC(0ff4f7c7) SHA1(69f1da01f6c00fbade573b9d6fa895e49f627fca) )
	/* 3+4 */
ROM_END



ROM_START( m5kormcl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ckor0_7.p1", 0x000000, 0x080000, CRC(29ddfb84) SHA1(a12dd1d063643b317c28d813e5e5f54f99387545) )
	ROM_LOAD16_BYTE( "ckor0_7.p2", 0x000001, 0x080000, CRC(7edeefe9) SHA1(bfb8fc8d0ddf65e39e6c9818777a2c0932828ff1) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "ckor0_7d.p1", 0x000000, 0x080000, CRC(34cbee06) SHA1(b9af18ad3ed99101d797254c1312821b9d743e12) )
ROM_END


ROM_START( m5lock )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lnl11s.p1", 0x000000, 0x080000, CRC(d6a537d2) SHA1(67590c973f31d682a525090c2dee794a863770e6) )
	ROM_LOAD16_BYTE( "lnl11s.p2", 0x000001, 0x080000, CRC(2985df6e) SHA1(9d969de995276be2aa7e00d46cbd10ee2a2e822d) )
	ROM_LOAD16_BYTE( "lnl11s.p3", 0x100000, 0x080000, CRC(8159eb7d) SHA1(8150b8d00b97564d3d6e77acb2b4e283143a7515) )
	ROM_LOAD16_BYTE( "lnl11s.p4", 0x100001, 0x080000, CRC(0dec842c) SHA1(92c18edc41059ce38e1c77abdce20896ff6de8fc) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "lnl11d.p1", 0x000000, 0x080000, CRC(504d67de) SHA1(c696305925256e6cbdd3758de137d9db7ef8e0d2) )
	ROM_LOAD16_BYTE( "lnl11k.p1", 0x000000, 0x080000, CRC(f267a63e) SHA1(564326f7a865fd936cce6ad4d3dfabc92132236a) )
ROM_END

ROM_START( m5lock12 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lnl12s.p1", 0x000000, 0x080000, CRC(3f0b5101) SHA1(2cbc9ebb361b928aad79c3f1f48881c368ab97b2) )
	ROM_LOAD16_BYTE( "lnl12s.p2", 0x000001, 0x080000, CRC(8a275bf5) SHA1(ca408e1083d34ea99f7da2d8bb7af7da67a5066d) )
	ROM_LOAD16_BYTE( "lnl12s.p3", 0x100000, 0x080000, CRC(8159eb7d) SHA1(8150b8d00b97564d3d6e77acb2b4e283143a7515) ) // == 11
	ROM_LOAD16_BYTE( "lnl12s.p4", 0x100001, 0x080000, CRC(0dec842c) SHA1(92c18edc41059ce38e1c77abdce20896ff6de8fc) ) // == 11

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "lnl12d.p1", 0x000000, 0x080000, CRC(b9e3010d) SHA1(33d42458a5aa8f2519f0af9221ddcef570c5d6e1) )
	ROM_LOAD16_BYTE( "lnl12k.p1", 0x000000, 0x080000, CRC(1bc9c0ed) SHA1(e9b87c93280e6a94b38b9b895caa35bd30f65dfe) )
ROM_END

ROM_START( m5lock13 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lnl13s.p1", 0x000000, 0x080000, CRC(355ebd1d) SHA1(306df0c40c909ed2f86f5a81c67e340e4416df1f) )
	ROM_LOAD16_BYTE( "lnl13s.p2", 0x000001, 0x080000, CRC(ef2e5c42) SHA1(09f281689364c384d9ddb03d2821095670eec977) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "lnl13d.p1", 0x000000, 0x080000, CRC(b3b6ed11) SHA1(48ef0ce6acc045e1da2400903533f5748ca4dfb7) )
	ROM_LOAD16_BYTE( "lnl13k.p1", 0x000000, 0x080000, CRC(119c2cf1) SHA1(e871530d704b9a395dca2f00e5915d8f259ea270) )
ROM_END




ROM_START( m5lockcl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lnc13s.p1", 0x000000, 0x080000, CRC(8fb923c1) SHA1(880dd681471f96a11fdd8366161e28af7e185439) )
	ROM_LOAD16_BYTE( "lnc13s.p2", 0x000001, 0x080000, CRC(dc228c7b) SHA1(43d9698617bfc16a749852f72c70e4e9af558727) )
	ROM_LOAD16_BYTE( "lnc13s.p3", 0x100000, 0x080000, CRC(36158197) SHA1(28f5a90026fc528eb82e8767cb4c503306e0220a) )
	ROM_LOAD16_BYTE( "lnc13s.p4", 0x100001, 0x080000, CRC(2becfc30) SHA1(5f302648071a6167f6bf06f8a96bd6a84aef9a27) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "lnc13d.p1", 0x000000, 0x080000, CRC(b36d2a56) SHA1(104c086116a7b83864da8ac208a70ee631bc7faf) )
	ROM_LOAD16_BYTE( "lnc13f.p1", 0x000000, 0x080000, CRC(ed976e38) SHA1(47c7b0919e9dc7fb47bf4970ba92f7169485d4cc) )
	ROM_LOAD16_BYTE( "lnc13l.p1", 0x000000, 0x080000, CRC(1be8e314) SHA1(8c8104c976ba57d06081450d0f50eadde0e8f286) )
	ROM_LOAD16_BYTE( "lnc13m.p1", 0x000000, 0x080000, CRC(4be5b833) SHA1(734fc50a439dc433ae9ec2d5eb83e4783c5fdd06) )
	ROM_LOAD16_BYTE( "lnc13o.p1", 0x000000, 0x080000, CRC(2829408b) SHA1(ef6afec6acab3a67739ee64c799cc4dd0aa22b9a) )
ROM_END

ROM_START( m5lockcl14 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lnc14s.p1", 0x000000, 0x080000, CRC(9271768d) SHA1(6caf9f6ea2da6a51c83da5897aabe1a76f0828bf) )
	ROM_LOAD16_BYTE( "lnc14s.p2", 0x000001, 0x080000, CRC(4951fe08) SHA1(8d4f9be119b8b1fb6d38f22daf0ddaef8049b910) )
	ROM_LOAD16_BYTE( "lnc14s.p3", 0x100000, 0x080000, CRC(36158197) SHA1(28f5a90026fc528eb82e8767cb4c503306e0220a) ) // == 13
	ROM_LOAD16_BYTE( "lnc14s.p4", 0x100001, 0x080000, CRC(2becfc30) SHA1(5f302648071a6167f6bf06f8a96bd6a84aef9a27) ) // == 13

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "lnc14d.p1", 0x000000, 0x080000, CRC(aea57f1a) SHA1(180b58a8fef5de25ebe772c4b03866bf84d8c52f) )
	ROM_LOAD16_BYTE( "lnc14f.p1", 0x000000, 0x080000, CRC(f05f3b74) SHA1(9bd1da5a8ffbc4fcaa1928a56bbedd528a576f5e) )
	ROM_LOAD16_BYTE( "lnc14l.p1", 0x000000, 0x080000, CRC(0620b658) SHA1(6049ec2d125be23ac60f83a49f62b38648927a89) )
	ROM_LOAD16_BYTE( "lnc14m.p1", 0x000000, 0x080000, CRC(562ded7f) SHA1(8081efa138a3b30ec867d0feb7f2a967b6ee8813) )
	ROM_LOAD16_BYTE( "lnc14o.p1", 0x000000, 0x080000, CRC(35e115c7) SHA1(ee7ced3b2be6a3217a5dfb3843127afe0c7f81a7) )
ROM_END

ROM_START( m5lockcl15 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lnc15s.p1", 0x000000, 0x080000, CRC(a78cfab4) SHA1(52c9567bd47c7dcf6616bf33b9026c51d09d17d4) )
	ROM_LOAD16_BYTE( "lnc15s.p2", 0x000001, 0x080000, CRC(7d408d93) SHA1(1b9806f5ac476eb40489bce25c8977d98b9c06f3) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "lnc15d.p1", 0x000000, 0x080000, CRC(9b58f323) SHA1(25b13c0d17a5d64af7e377f5b6bc918203b768e4) )
	ROM_LOAD16_BYTE( "lnc15f.p1", 0x000000, 0x080000, CRC(c5a2b74d) SHA1(f927a30a5ee3c307f384d6dccf1f091b63957e2e) )
	ROM_LOAD16_BYTE( "lnc15l.p1", 0x000000, 0x080000, CRC(33dd3a61) SHA1(d8bf5f169c453ab586aee2d2d5f25384f66affb4) )
	ROM_LOAD16_BYTE( "lnc15m.p1", 0x000000, 0x080000, CRC(63d06146) SHA1(61b7f6b189221856b1eacbe6e1ced11e9fa0fcfc) )
	ROM_LOAD16_BYTE( "lnc15o.p1", 0x000000, 0x080000, CRC(001c99fe) SHA1(3306a89cb2794a0a552f01b10376b538f48edafa) )
ROM_END


ROM_START( m5loony )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lju_sjs1.3_1", 0x000000, 0x080000, CRC(6e5cda47) SHA1(9285cdbc0c1fe494acc02f7c57d7204141814426) )
	ROM_LOAD16_BYTE( "lju_sjs1.3_2", 0x000001, 0x080000, CRC(c472acd8) SHA1(bb1af9e428aaebb3c0a7cbc0f2d4968c495b1bce) )
	ROM_LOAD16_BYTE( "lju_sjs1.3_3", 0x100000, 0x080000, CRC(2d308353) SHA1(e5ad545214c675f7047ee4ff45176072c8a4321b) )
	ROM_LOAD16_BYTE( "lju_sjs1.3_4", 0x100001, 0x080000, CRC(11762cfa) SHA1(bdce1cebb7f893149d7742432cad8729ccca0ebd) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "lju_sjs1.3d1", 0x000000, 0x080000, CRC(77d8646c) SHA1(1d1c73348ff5295e9d03d9a35f9d850edc0dcc20) )
ROM_END



ROM_START( m5loot ) // 3_in_1_(hybrid)_[pdx01_1024_25jp].hex
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lkhsjb_1.7_1", 0x000000, 0x080000, CRC(f1e72a7f) SHA1(093dd37be96c07213bf9df3f3d193b38b8d4a3c4) )
	ROM_LOAD16_BYTE( "lkhsjb_1.7_2", 0x000001, 0x080000, CRC(d7b40df8) SHA1(4704cbe1c34239b7060135208ed550870ba69483) )
	ROM_LOAD16_BYTE( "lkhsjb_1.7_3", 0x100000, 0x080000, CRC(f1a13457) SHA1(11b254b00ab4d1249bd706fb43f0a947b230054f) )
	ROM_LOAD16_BYTE( "lkhsjb_1.7_4", 0x100001, 0x080000, CRC(a8e6ab92) SHA1(5640c83a841fce17f22e6d3ab8747eb2de266047) )
ROM_END

ROM_START( m5loota )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lkhsjbd1.7_1", 0x000000, 0x080000, CRC(81ba5ccc) SHA1(26375e6f81b926a5033c71f1b53f5b489644525e) ) // only change from m5loot
	ROM_LOAD16_BYTE( "lkhsjb_1.7_2", 0x000001, 0x080000, CRC(d7b40df8) SHA1(4704cbe1c34239b7060135208ed550870ba69483) )
	ROM_LOAD16_BYTE( "lkhsjb_1.7_3", 0x100000, 0x080000, CRC(f1a13457) SHA1(11b254b00ab4d1249bd706fb43f0a947b230054f) )
	ROM_LOAD16_BYTE( "lkhsjb_1.7_4", 0x100001, 0x080000, CRC(a8e6ab92) SHA1(5640c83a841fce17f22e6d3ab8747eb2de266047) )
ROM_END


ROM_START( m5lotta )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "luck20ad.p1", 0x000000, 0x080000, CRC(a7e58c3d) SHA1(ecb07cd64c97d31112120c9c1a3e8fee797a6f85) )
	ROM_LOAD16_BYTE( "luck20.p2", 0x000001, 0x080000, CRC(79490ac5) SHA1(3a74a813a76c3d6c3f1e7a454611129c6d5d7187) )
	ROM_LOAD16_BYTE( "luck20.p3", 0x100000, 0x080000, CRC(ab2f363f) SHA1(4d5fca129c8b332c411186b4049d81f63395478d) )
	ROM_LOAD16_BYTE( "luck20.p4", 0x100001, 0x080000, CRC(bbc2b9c5) SHA1(ff6ed8c5892aea6f23bea262f30f4acb8eeea3ed) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "luck20b.p1", 0x000000, 0x080000, CRC(8723d1c6) SHA1(1c610be7c3d2de56b8e44279ba2475a8e51a1186) )
	ROM_LOAD16_BYTE( "luck20bd.p1", 0x000000, 0x080000, CRC(85586660) SHA1(f243f9ada6e36f408476990aefa70e89e5288d8f) )
	ROM_LOAD16_BYTE( "luck20d.p1", 0x000000, 0x080000, CRC(19cb4fd6) SHA1(6e339df5350abb4e023f50ff582d69c794b19c8e) )
	ROM_LOAD16_BYTE( "luck20dy.p1", 0x000000, 0x080000, CRC(276381d1) SHA1(8c49b0717ef5ed314dca7aaf2698ac7e8c3c6fd7) )
	ROM_LOAD16_BYTE( "luck20h.p1", 0x000000, 0x080000, CRC(35a33dd3) SHA1(c1494af0a13151bc5d36be4a1bf3fa59ce60b8fe) )
	ROM_LOAD16_BYTE( "luck20r.p1", 0x000000, 0x080000, CRC(9d7bfca2) SHA1(2922fdbe8a116bfa2b4aba2fcbdcc0f1e1fdcbcd) )
	ROM_LOAD16_BYTE( "luck20s.p1", 0x000000, 0x080000, CRC(4f5e0c79) SHA1(873d7654dbc511c147d373f09c9f2fa8257a6145) )
	ROM_LOAD16_BYTE( "luck20y.p1", 0x000000, 0x080000, CRC(73c3cf14) SHA1(82b141ce714cc8a4802c8a75002e9efa19a5f885) )
ROM_END


ROM_START( m5martns )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mmma06ad.p1", 0x000000, 0x080000, CRC(6bcda900) SHA1(f9dc50d00ce5bd54c272f4cb583b0f2de7172d01) )
	ROM_LOAD16_BYTE( "mmma06.p2", 0x000001, 0x080000, CRC(ec1ee181) SHA1(7a51fb16ca525797bd91481b4a7d89eecebe22b0) )
	ROM_LOAD16_BYTE( "mmma06.p3", 0x100000, 0x080000, CRC(fa21548f) SHA1(4e98873ef586cd033f3b3955e9ad3dc3f343fd1c) )
	ROM_LOAD16_BYTE( "mmma06.p4", 0x100001, 0x080000, CRC(db8d7b89) SHA1(7d06a6b592dcb1a0779f19194be390880329e8d1) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "mmma06b.p1", 0x000000, 0x080000, CRC(35bc6695) SHA1(ae9ac0a728d485a9d969d1af3878ff372a727f46) )
	ROM_LOAD16_BYTE( "mmma06bd.p1", 0x000000, 0x080000, CRC(54f313a6) SHA1(2133b48575cba8fdc8a316d1b614e334c90f49e0) )
	ROM_LOAD16_BYTE( "mmma06d.p1", 0x000000, 0x080000, CRC(b4aaedb8) SHA1(b4f6bd496dfa5d60483906b3b8a064fae0febb67) )
	ROM_LOAD16_BYTE( "mmma06dy.p1", 0x000000, 0x080000, CRC(abbff8da) SHA1(1f046e9823cccbd3ce9f89b150ba3d3b328937b8) )
	ROM_LOAD16_BYTE( "mmma06k.p1", 0x000000, 0x080000, CRC(a57e0f06) SHA1(885544c86bde2916368b57728080290172fc9724) )
	ROM_LOAD16_BYTE( "mmma06r.p1", 0x000000, 0x080000, CRC(febb826f) SHA1(caae46a9a3a53298e1a3b2d99b7cc6cc959c0995) )
	ROM_LOAD16_BYTE( "mmma06s.p1", 0x000000, 0x080000, CRC(2fbf3a22) SHA1(54d14a34e6b6c8e4ee0d1858871503a9e8e1f2d8) )
	ROM_LOAD16_BYTE( "mmma06y.p1", 0x000000, 0x080000, CRC(30aa2f40) SHA1(e2f1ecee7280247ae4f68ba5066011ddf66b511e) )
ROM_END

ROM_START( m5martns07 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mmma07s.p1", 0x000000, 0x080000, CRC(03198c78) SHA1(ffbcc782403a6b372b1aff6a48621ab9155816ba) )
	ROM_LOAD16_BYTE( "mmma07.p2", 0x000001, 0x080000, CRC(11bda2ba) SHA1(81d2476ae658f06ddecbeb6ec99ba3829d1888ee) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "mmma07ad.p1", 0x000000, 0x080000, CRC(33b841e2) SHA1(2140bae7596e8431778f24468b8b9ba6c0dcec3b) )
	ROM_LOAD16_BYTE( "mmma07b.p1", 0x000000, 0x080000, CRC(b773ee4c) SHA1(a71cb2c6d49856f5f02f573dfc2279aea79b5b1a) )
	ROM_LOAD16_BYTE( "mmma07bd.p1", 0x000000, 0x080000, CRC(c15af452) SHA1(24263c105190a138f772d9ff001617214cdc6282) )
	ROM_LOAD16_BYTE( "mmma07d.p1", 0x000000, 0x080000, CRC(591bfac8) SHA1(9858fbf8b6f4323b0ed300ab8b286f68de1234cb) )
	ROM_LOAD16_BYTE( "mmma07dy.p1", 0x000000, 0x080000, CRC(9c9679b4) SHA1(583014f26c982c1bdb97e1d68cd2c5702c711592) )
	ROM_LOAD16_BYTE( "mmma07k.p1", 0x000000, 0x080000, CRC(0bf61171) SHA1(3d34640b460121c3176c8dba427239b303448a11) )
	ROM_LOAD16_BYTE( "mmma07r.p1", 0x000000, 0x080000, CRC(10f463c0) SHA1(cb7935d6457fb6df57ccdaf1c42c1422bc14ad8e) )
	ROM_LOAD16_BYTE( "mmma07y.p1", 0x000000, 0x080000, CRC(c6940f04) SHA1(815b6f3b9d840e4c4e94c5adaf234da8dba4fe9c) )
ROM_END

ROM_START( m5mmak )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mmak05ad.p1", 0x000000, 0x080000, CRC(2c1c0cfc) SHA1(3b22e6167a87dc093212b4e7beeac82bce35f963) )
	ROM_LOAD16_BYTE( "mmak05.p2", 0x000001, 0x080000, CRC(875e223f) SHA1(429a282a7ea99fb15645fbc9870a34642ca6e4d1) )
	ROM_LOAD16_BYTE( "mmak05.p3", 0x100000, 0x080000, CRC(1048ceca) SHA1(f6a6790ed3c7b60cbe31db9eb1ed7da5ac81c0fd) )
	ROM_LOAD16_BYTE( "mmak05.p4", 0x100001, 0x080000, CRC(50f7100c) SHA1(b7e0199bf183b7954416eb091a5aa8ed5c44143b) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "mmak05b.p1", 0x000000, 0x080000, CRC(765a8e4e) SHA1(78400d4678d55a254de8871098bf336b17a2dbd2) )
	ROM_LOAD16_BYTE( "mmak05bd.p1", 0x000000, 0x080000, CRC(4537c61d) SHA1(91b12cd0777d8a52a6f575183df78fd0a67b1b71) )
	ROM_LOAD16_BYTE( "mmak05d.p1", 0x000000, 0x080000, CRC(e94076fb) SHA1(3202cf187976ac1eb5b86efc34b456b483984a62) )
	ROM_LOAD16_BYTE( "mmak05dy.p1", 0x000000, 0x080000, CRC(510bd2dd) SHA1(7c2789631d67a5829fd2dc0a54c87836a65a910e) )
	ROM_LOAD16_BYTE( "mmak05h.p1", 0x000000, 0x080000, CRC(541e3602) SHA1(ed5d89da97b511670b52383d1b3c4dc12651b894) )
	ROM_LOAD16_BYTE( "mmak05r.p1", 0x000000, 0x080000, CRC(b120b9ab) SHA1(31a1acb1c9420a8bf84b0e27a85e176f41024138) )
	ROM_LOAD16_BYTE( "mmak05s.p1", 0x000000, 0x080000, CRC(5a0524db) SHA1(3b52c2a4edaa2ba89cc6735e96ca2fec85500f6a) )
	ROM_LOAD16_BYTE( "mmak05y.p1", 0x000000, 0x080000, CRC(e24e80fd) SHA1(49ce174a3213208a0b8e79ce48c8e221e2e07ed7) )
ROM_END

ROM_START( m5mmak06 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mmak06s.p1", 0x000000, 0x080000, CRC(c8e43a9c) SHA1(fe667ffe7df132c1a625b69c82ff8e7defce1317) )
	ROM_LOAD16_BYTE( "mmak06.p2", 0x000001, 0x080000, CRC(24ad42b1) SHA1(5189199316cf5f896fbdc00f15cf9abcba2489c6) )
	ROM_LOAD16_BYTE( "mmak06.p3", 0x100000, 0x080000, CRC(1048ceca) SHA1(f6a6790ed3c7b60cbe31db9eb1ed7da5ac81c0fd) ) // == 05
	ROM_LOAD16_BYTE( "mmak06.p4", 0x100001, 0x080000, CRC(50f7100c) SHA1(b7e0199bf183b7954416eb091a5aa8ed5c44143b) ) // == 05

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "mmak06ad.p1", 0x000000, 0x080000, CRC(fe92d3b3) SHA1(70bade6ac8a524ff640923960d6b33720d10945e) )
	ROM_LOAD16_BYTE( "mmak06b.p1", 0x000000, 0x080000, CRC(4c283966) SHA1(97a6d2b7778998067f9556f412cc116999a37fae) )
	ROM_LOAD16_BYTE( "mmak06bd.p1", 0x000000, 0x080000, CRC(d9f39761) SHA1(51e501ced84149b66602905dc526b7af69a6ce48) )
	ROM_LOAD16_BYTE( "mmak06d.p1", 0x000000, 0x080000, CRC(17de903c) SHA1(271a24014060dd3512e43236bb69fe69b1b26a1c) )
	ROM_LOAD16_BYTE( "mmak06dy.p1", 0x000000, 0x080000, CRC(5faaea66) SHA1(d5d5fb1ce508a1e97262bf14ea106592e5544d39) )
	ROM_LOAD16_BYTE( "mmak06h.p1", 0x000000, 0x080000, CRC(fa912ebb) SHA1(e6117cf7ef2f0428927c7d45bc53e5102ac685fb) )
	ROM_LOAD16_BYTE( "mmak06r.p1", 0x000000, 0x080000, CRC(e52e7181) SHA1(edee4b59afc56a110ed9f3d10ed6e771016b51e5) )
	ROM_LOAD16_BYTE( "mmak06y.p1", 0x000000, 0x080000, CRC(ad45ec70) SHA1(c837d2d2474457d20d5985645551e361a481f903) )
ROM_END

ROM_START( m5monty )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mptr01d.p1", 0x000000, 0x080000, CRC(e756a3cc) SHA1(19f1c59f4c4849f56d5b9ea40e2506a984e42e52) )
	ROM_LOAD16_BYTE( "mptr01.p2", 0x000001, 0x080000, CRC(bf0fe9c1) SHA1(67c80f4f8b48f1fe88ab6ff8cda70b81fd350da4) )
	ROM_LOAD16_BYTE( "mptr01.p3", 0x100000, 0x080000, CRC(8ecfbc3c) SHA1(74d1b065ebd0c33813fb1d5114efc7b4be77fd82) )
	ROM_LOAD16_BYTE( "mptr01.p4", 0x100001, 0x080000, CRC(e7fdb09b) SHA1(8de3e5ad049a337cfe55979cd08fba5a38460960) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "mptr01dy.p1", 0x000000, 0x080000, CRC(90038d81) SHA1(724b78ed2cff56721440c5bb87c40f1ddaa0fa22) )
	ROM_LOAD16_BYTE( "mptr01s.p1", 0x000000, 0x080000, CRC(b05d9b16) SHA1(7e37414f693cf0cdac0cf0078a40415aec722a08) )
	ROM_LOAD16_BYTE( "mptr01y.p1", 0x000000, 0x080000, CRC(c708b55b) SHA1(cb97ff0646b811b9b80c6f2759eba6850093eeae) )
ROM_END


ROM_START( m5mpfc )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pyth04ad.p1", 0x000000, 0x080000, CRC(0275a21b) SHA1(3cb62ec2dc4aefb23fa7ae90e9c6b25d95eabe9c) )
	ROM_LOAD16_BYTE( "pyth04.p2", 0x000001, 0x080000, CRC(7622bebd) SHA1(95b47ec30c055cc64fda9665ab56b4d642ae6076) )
	ROM_LOAD16_BYTE( "pyth04.p3", 0x100000, 0x080000, CRC(8ecfbc3c) SHA1(74d1b065ebd0c33813fb1d5114efc7b4be77fd82) )
	ROM_LOAD16_BYTE( "pyth04.p4", 0x100001, 0x080000, CRC(e7fdb09b) SHA1(8de3e5ad049a337cfe55979cd08fba5a38460960) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "pyth04b.p1", 0x000000, 0x080000, CRC(5817e71a) SHA1(aa0e154be589102916963928c2ecb6fea920dfd8) )
	ROM_LOAD16_BYTE( "pyth04bd.p1", 0x000000, 0x080000, CRC(61951660) SHA1(8c605bb6a8200db9e8e0454aaab9b8cbfc003a57) )
	ROM_LOAD16_BYTE( "pyth04d.p1", 0x000000, 0x080000, CRC(afc5a211) SHA1(ed6e2619bbcb53d85aa04989bdc8587cc9f1e98e) )
	ROM_LOAD16_BYTE( "pyth04dy.p1", 0x000000, 0x080000, CRC(91a54da8) SHA1(c67937bc0b810f3c0440645cb384680d3c930083) )
	ROM_LOAD16_BYTE( "pyth04h.p1", 0x000000, 0x080000, CRC(e5caf28a) SHA1(5ac3fc20605adfe879c0116e445746ddc4f52f34) )
	ROM_LOAD16_BYTE( "pyth04r.p1", 0x000000, 0x080000, CRC(18a50d9c) SHA1(31b9c871009befe92bfa9c17edc4e0aaad207b63) )
	ROM_LOAD16_BYTE( "pyth04s.p1", 0x000000, 0x080000, CRC(9647536b) SHA1(d8c419f8e04a3b309cf967b0431791b1b17572ac) )
	ROM_LOAD16_BYTE( "pyth04y.p1", 0x000000, 0x080000, CRC(a827bcd2) SHA1(a40b99fe8711d3ca897025ec1b0be4430429563d) )
ROM_END


ROM_START( m5mpfccl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mpfc04d.p1", 0x000000, 0x080000, CRC(8e87d7e1) SHA1(a68045c79d05d46960677651ca8716cb39a8cf1f) )
	ROM_LOAD16_BYTE( "mpfc04.p2", 0x000001, 0x080000, CRC(e5e666be) SHA1(3031269de2cbc2717773a627b9ab2b9129e11708) )
	ROM_LOAD16_BYTE( "mpfc04.p3", 0x100000, 0x080000, CRC(52cdc512) SHA1(9de40eff75ef4f1a66ce744dbe50a1abce1ae592) )
	ROM_LOAD16_BYTE( "mpfc04.p4", 0x100001, 0x080000, CRC(3513db48) SHA1(8c3279cb38d813be0698e1eabc813048836f6a94) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "mpfc04dz.p1", 0x000000, 0x080000, CRC(c4332483) SHA1(be2f59c251e2a9c564d06fbf7d15e72b08951669) )
	ROM_LOAD16_BYTE( "mpfc04f.p1", 0x000000, 0x080000, CRC(6e35c994) SHA1(3761057b4cf6b7af5914c86d0fcef44d4b32b6fe) )
	ROM_LOAD16_BYTE( "mpfc04s.p1", 0x000000, 0x080000, CRC(2ee96357) SHA1(3848dbc0e32a91fbde8073e0bbebc541319676fd) )
	ROM_LOAD16_BYTE( "mpfc04z.p1", 0x000000, 0x080000, CRC(270faccc) SHA1(8abd2716fc1c1df04849dca10bccd9066205fe3f) )
ROM_END


ROM_START( m5neptun )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "nptn04.p1", 0x000000, 0x080000, CRC(88d3c6bd) SHA1(19d927292bca3eb27fe126b64d480c9b447226ca) )
	ROM_LOAD16_BYTE( "nptn04.p2", 0x000001, 0x080000, CRC(a4038367) SHA1(1598918db73a40ed9cad34fe1369c7c56bf7fccb) )
	ROM_LOAD16_BYTE( "nptn04.p3", 0x100000, 0x080000, CRC(fe1f683f) SHA1(313d4195895ee0093e7b19813abad74961c5acae) )
	ROM_LOAD16_BYTE( "nptn04.p4", 0x100001, 0x080000, CRC(e327195a) SHA1(6c263f11174c63a08a41a38ad08bebe26f7a8fcb) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "nptn04", 0x000000, 0x080000, CRC(932d255a) SHA1(55c8c722bc8d30c362f72bd726d307611a6d74b0) )
	ROM_LOAD16_BYTE( "nptn04ad.p1", 0x000000, 0x080000, CRC(e436a840) SHA1(241b55535e0fd720bab1e16d948648614e994161) )
	ROM_LOAD16_BYTE( "nptn04b.p1", 0x000000, 0x080000, CRC(c3d44d62) SHA1(db7a8cace76738fbcded385fe8338613f7249690) )
	ROM_LOAD16_BYTE( "nptn04bd.p1", 0x000000, 0x080000, CRC(fc3620b6) SHA1(840c9a6361fd8d5d0ee7e5875bcc2d3c7ef79983) )
	ROM_LOAD16_BYTE( "nptn04d.p1", 0x000000, 0x080000, CRC(b731ab69) SHA1(789421936a337a241e7d6b9824d97c835c85ff85) )
	ROM_LOAD16_BYTE( "nptn04dy.p1", 0x000000, 0x080000, CRC(b007dfa3) SHA1(3ad8566c61269b90574ce413676e7cf412f9eea6) )
	ROM_LOAD16_BYTE( "nptn04h.p1", 0x000000, 0x080000, CRC(e6831479) SHA1(d0005c30cc25b830552062f86f936916f20481f8) )
	ROM_LOAD16_BYTE( "nptn04r.p1", 0x000000, 0x080000, CRC(4008229d) SHA1(2ac6c6746e282e8ed6a426a658514bbafcae15ad) )
	//ROM_LOAD16_BYTE( "nptn04s.p1", 0x000000, 0x080000, CRC(88d3c6bd) SHA1(19d927292bca3eb27fe126b64d480c9b447226ca) )
	ROM_LOAD16_BYTE( "nptn04y.p1", 0x000000, 0x080000, CRC(8fe5b277) SHA1(be97cf9a4319132be90f007f8f882fc35b7cae13) )
	ROM_LOAD( "n_treas.p3", 0x0000, 0x0f7778, CRC(5d837f4e) SHA1(4fc602896c901c8f270baf30476aa09d6475c8f2) ) //?
ROM_END



ROM_START( m5nitro )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "nto10s.p1", 0x000000, 0x080000, CRC(c1518d34) SHA1(8523a4a6386f01eed7f80f31da130caec367029a) )
	ROM_LOAD16_BYTE( "nto10s.p2", 0x000001, 0x080000, CRC(d2d3bf89) SHA1(2736aa11cbd5582152aff2c5753e36d0b5cb20d6) )
	ROM_LOAD16_BYTE( "nto10s.p3", 0x100000, 0x080000, CRC(481f77f6) SHA1(9327407090a8c48b2f922d53b3239ce118521842) )
	ROM_LOAD16_BYTE( "nto10s.p4", 0x100001, 0x080000, CRC(93798412) SHA1(20c6a7ddafd077ce1ca69db3852ee2227ca487e8) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "nto10d.p1", 0x000000, 0x080000, CRC(47b9dd38) SHA1(a286d766cb11e1a5d90fdf03517ec2fe3646b745) )
	ROM_LOAD16_BYTE( "nto10k.p1", 0x000000, 0x080000, CRC(e5931cd8) SHA1(63359d92e465ed71e07bbe2e381df5e3a096fc8b) )
ROM_END



ROM_START( m5nnww )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "nn2501.p1", 0x000000, 0x080000, CRC(104acda4) SHA1(f87e345144dc21e8d545ae043bbbf97fdd18b422) )
	ROM_LOAD16_BYTE( "nn2501.p2", 0x000001, 0x080000, CRC(0b739c9e) SHA1(9b0731b52c3d051828fa9ebe02117410f97ce52e) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "nn2501ad.p1", 0x000000, 0x080000, CRC(57213623) SHA1(4998d5bf5a87ffb0a1065da5cf5210bcc7384dd9) )
	ROM_LOAD16_BYTE( "nn2501d.p1", 0x000000, 0x080000, CRC(83744167) SHA1(e2a85d32469a3a8e93af6b9ae0a3602ddb78c7c6) )
	//ROM_LOAD16_BYTE( "nn2501s.p1", 0x000000, 0x080000, CRC(104acda4) SHA1(f87e345144dc21e8d545ae043bbbf97fdd18b422) )
ROM_END



ROM_START( m5nnwwgl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "nwgo07ad.p1", 0x000000, 0x080000, CRC(24866f88) SHA1(2f23d423c15e716a54ee309d5232eb0033a4124d) )
	ROM_LOAD16_BYTE( "nwgo07.p2", 0x000001, 0x080000, CRC(435f8991) SHA1(81769ef4b86141ad854fd189b2c4cf41beb1b0c1) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "nwgo07b.p1", 0x000000, 0x080000, CRC(6edc00d0) SHA1(3c98029699415c30be0c7e30e5d07d3374aec88a) )
	ROM_LOAD16_BYTE( "nwgo07bd.p1", 0x000000, 0x080000, CRC(4dc754c7) SHA1(0be431de161d4d673c3db1a065a539f3cd42cc20) )
	ROM_LOAD16_BYTE( "nwgo07d.p1", 0x000000, 0x080000, CRC(0bbaafb7) SHA1(cd248b4ed47c02fb0c7fcd4b29645ce809e798b3) )
	ROM_LOAD16_BYTE( "nwgo07dy.p1", 0x000000, 0x080000, CRC(aff647b8) SHA1(03215a8982dc881609462bdeaa9a54231bdba18b) )
	ROM_LOAD16_BYTE( "nwgo07r.p1", 0x000000, 0x080000, CRC(bef67c2c) SHA1(0334c29f0aa6831a67f2ad9497edc5a1d56fc140) )
	ROM_LOAD16_BYTE( "nwgo07s.p1", 0x000000, 0x080000, CRC(526af5ab) SHA1(6e217a3049c4c82e829cf34ed069703a292f092f) )
	ROM_LOAD16_BYTE( "nwgo07y.p1", 0x000000, 0x080000, CRC(f6261da4) SHA1(b87e4dab732a5217ceb1156c9bbdfc796e1fdc95) )
ROM_END


ROM_START( m5fiddle )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fidl08ad.p1", 0x000000, 0x080000, CRC(0b757cd1) SHA1(267b6d98b483deb72e2be1a40eff50a6d2433909) )
	ROM_LOAD16_BYTE( "fidl08.p2", 0x000001, 0x080000, CRC(8d471c95) SHA1(c22f97d14bb2a2687ac09857a0ea2896bada281c) )
	ROM_LOAD16_BYTE( "fidl08.p3", 0x100000, 0x080000, CRC(4c7a4be2) SHA1(a724b05a7b19d074402650f6694039815e2e67a5) )
	ROM_LOAD16_BYTE( "fidl08.p4", 0x100001, 0x080000, CRC(dff021aa) SHA1(b248ba85f12e87fdb2d02ea502568f753b2b61ea) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "fidl08b.p1", 0x000000, 0x080000, CRC(06ae2e01) SHA1(8c310104180611f9384ee2ef0ffb7796d1e59a07) )
	ROM_LOAD16_BYTE( "fidl08bd.p1", 0x000000, 0x080000, CRC(a33788bc) SHA1(3feffc590621686fe92ef496f25f82454c623324) )
	ROM_LOAD16_BYTE( "fidl08d.p1", 0x000000, 0x080000, CRC(3dc20659) SHA1(18c24c8daf3ed5788e050f7d7ad47a94ea6c1345) )
	ROM_LOAD16_BYTE( "fidl08dy.p1", 0x000000, 0x080000, CRC(e2701e4f) SHA1(bffb0da2ec197bbcf07789c4868416649ab7f3e1) )
	ROM_LOAD16_BYTE( "fidl08h.p1", 0x000000, 0x080000, CRC(5d82ca36) SHA1(34061277df9be45c771b05173969e0e2d7000324) )
	ROM_LOAD16_BYTE( "fidl08k.p1", 0x000000, 0x080000, CRC(0061ad91) SHA1(5a0565f1dd3b26e58a3b7e1ad266fdb347ceb94f) )
	ROM_LOAD16_BYTE( "fidl08r.p1", 0x000000, 0x080000, CRC(4c7083b5) SHA1(8c38650439594203fe0ee86cddf03425d4d0aeb4) )
	ROM_LOAD16_BYTE( "fidl08s.p1", 0x000000, 0x080000, CRC(43db59b5) SHA1(95146315dee00fb6afbeed571d00e13a8f8cf974) )
	ROM_LOAD16_BYTE( "fidl08y.p1", 0x000000, 0x080000, CRC(d5567381) SHA1(1968d33170b1099fd4d48ec44d13abb0f095dba3) )
ROM_END

ROM_START( m5fiddle03 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "oftr03s.p1", 0x000000, 0x080000, CRC(7c3c174c) SHA1(19a1d2c89753aa70abcf44ab6997760910366ce9) )
	ROM_LOAD16_BYTE( "oftr03.p2", 0x000001, 0x080000, CRC(e01e1b54) SHA1(23903aceb618eafeefd52a4e9074de83262d587d) )
	ROM_LOAD16_BYTE( "oftr03.p3", 0x100000, 0x080000, CRC(f05df967) SHA1(3de235d0841cd17d5f817ba7a45e7e1552f3bf28) )
	ROM_LOAD16_BYTE( "oftr03.p4", 0x100001, 0x080000, CRC(0abf20e5) SHA1(028aa8377e01e5de81bfdb1afc0ebce8a3eee2a5) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "oftr03ad.p1", 0x000000, 0x080000, CRC(ad325e8c) SHA1(33452592e6813154d5a2f30e2b81b053dd28875e) )
	ROM_LOAD16_BYTE( "oftr03b.p1", 0x000000, 0x080000, CRC(24fea5f1) SHA1(bc938017daa27ad6a2a54578ba0d6c05c9fe15ee) )
	ROM_LOAD16_BYTE( "oftr03bd.p1", 0x000000, 0x080000, CRC(73cea690) SHA1(0bd3b99980d5b9936f9f8dd47946dd32923a78b5) )
	ROM_LOAD16_BYTE( "oftr03d.p1", 0x000000, 0x080000, CRC(51d3028b) SHA1(2abceef001838f172101b42f604c7a6fe9aa3c49) )
	ROM_LOAD16_BYTE( "oftr03dy.p1", 0x000000, 0x080000, CRC(bb027b0f) SHA1(d133b9db5766e8494b90d76ff2ca8661ae6cb34c) )
	ROM_LOAD16_BYTE( "oftr03k.p1", 0x000000, 0x080000, CRC(180e25fc) SHA1(4eba34872f6d60146b38d9302d0e153aec425d32) )
	ROM_LOAD16_BYTE( "oftr03r.p1", 0x000000, 0x080000, CRC(b49c58ac) SHA1(a11b9b981dfd71929b214a303df60f26ed026aae) )
	ROM_LOAD16_BYTE( "oftr03y.p1", 0x000000, 0x080000, CRC(96ed6ec8) SHA1(d9939c1243b1ae1f400392a08c92a61b047ce02b) )
ROM_END

ROM_START( m5oohaah )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "oadg11ad.p1", 0x000000, 0x080000, CRC(b283762b) SHA1(fc72d64c1a95d5c6080333e7dd8f8bf8f6ab520f) )
	ROM_LOAD16_BYTE( "oadg11.p2", 0x000001, 0x080000, CRC(ffc5e89e) SHA1(95268592953c604e2b22e90a017fd211c4425bd5) )
	ROM_LOAD16_BYTE( "oadg11.p3", 0x100000, 0x080000, CRC(de7dac75) SHA1(805f39c08dab6df77555066ab8aab2c60154c657) )
	ROM_LOAD16_BYTE( "oadg11.p4", 0x100001, 0x080000, CRC(ba3d740e) SHA1(1b035b057a844e19abe869adbea95b6cbed7e8a1) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "oadg11b.p1", 0x000000, 0x080000, CRC(2ff74577) SHA1(2525471274a30e2abbdd8a6882dddfad3d52ae30) )
	ROM_LOAD16_BYTE( "oadg11bd.p1", 0x000000, 0x080000, CRC(75ae17bd) SHA1(c27456f7f77c1b08f6c039baf5f41041fed840a8) )
	ROM_LOAD16_BYTE( "oadg11d.p1", 0x000000, 0x080000, CRC(416f52c9) SHA1(1c679c91acd07d71aeac92d1d78a8a491a11ea7a) )
	ROM_LOAD16_BYTE( "oadg11dy.p1", 0x000000, 0x080000, CRC(5c5871c3) SHA1(84701707aa9db701b578b3ccf0677fc07fbb4ee1) )
	ROM_LOAD16_BYTE( "oadg11h.p1", 0x000000, 0x080000, CRC(c59e4472) SHA1(7c214d25df050708199b7a59f5db52671de34eb9) )
	ROM_LOAD16_BYTE( "oadg11r.p1", 0x000000, 0x080000, CRC(d8438598) SHA1(20e0b79259bcfc8ae9cb9516ec3cd735ac5f80c6) )
	ROM_LOAD16_BYTE( "oadg11s.p1", 0x000000, 0x080000, CRC(0b06bc2e) SHA1(589b861ddb847757fa90e82fba169b19f61830db) )
	ROM_LOAD16_BYTE( "oadg11y.p1", 0x000000, 0x080000, CRC(16319f24) SHA1(1f14c6f5ab5a77ab5fc774871a8cca2143cf6a69) )
ROM_END

ROM_START( m5oohaah01 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "oadg01.p1", 0x000000, 0x080000, CRC(1a4076fd) SHA1(5d79524ac3e40d82dcf67f2d2e4e4300b1594d28) )
	ROM_LOAD16_BYTE( "oadg01.p2", 0x000001, 0x080000, CRC(fc34684f) SHA1(88a7276993e95356b10ae3f9a4efa770c9430420) )
	/* 3+4 */
ROM_END




ROM_START( m5oohrio )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ltdc08ad.p1", 0x000000, 0x080000, CRC(0df807a2) SHA1(93193e9497ce80b09ad0f9475209867fdd26f739) )
	ROM_LOAD16_BYTE( "ltdc08.p2", 0x000001, 0x080000, CRC(1e3b7b60) SHA1(169a64449fc89e550bca1826d2d8cd5e20230471) )
	ROM_LOAD16_BYTE( "ltdc08.p3", 0x100000, 0x080000, CRC(ff2cadda) SHA1(152296e4407bd01b180dc82c73074b66bd98928f) )
	ROM_LOAD16_BYTE( "ltdc08.p4", 0x100001, 0x080000, CRC(c4007d0d) SHA1(1da2a9484e46fac8f4fd609bd01f7b5ea01fe46e) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "ltdc08b.p1", 0x000000, 0x080000, CRC(8afda5f3) SHA1(b36a0df587f48984b78611ea7c54f46f67ac5218) )
	ROM_LOAD16_BYTE( "ltdc08bd.p1", 0x000000, 0x080000, CRC(956e197e) SHA1(39fcdf31bdd20b6d4524b4fd3b959669c3fd650d) )
	ROM_LOAD16_BYTE( "ltdc08d.p1", 0x000000, 0x080000, CRC(0927c2c2) SHA1(9ad49d7f7525ce4703bf31537249fdc815490aa8) )
	ROM_LOAD16_BYTE( "ltdc08r.p1", 0x000000, 0x080000, CRC(31debc0d) SHA1(164f44fdc4daaa9484b30ad554b67316b6bfe5d8) )
	ROM_LOAD16_BYTE( "ltdc08s.p1", 0x000000, 0x080000, CRC(7f5ccc03) SHA1(963ec2d60bef6fc83eb6458b8c5bb5414b8d7530) )
	ROM_LOAD16_BYTE( "ltdc08y.p1", 0x000000, 0x080000, CRC(6f7d1ed4) SHA1(48904d52d15167f0dd1dd22773b1afcdc784c8a6) )
ROM_END



ROM_START( m5openbx )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "obtr02ad.p1", 0x000000, 0x080000, CRC(75a876d1) SHA1(d05b4f2f2d17b0c253c928ee16c9de554f653012) )
	ROM_LOAD16_BYTE( "obtr02.p2", 0x000001, 0x080000, CRC(67bde106) SHA1(a18af1a28bf5f70f0fdd9deffd7c09b1348f9785) )
	ROM_LOAD16_BYTE( "obtr02.p3", 0x100000, 0x080000, CRC(ff088cd9) SHA1(c34b0f76d2a669bb65e794de01962d8ae79a9317) )
	ROM_LOAD16_BYTE( "obtr02.p4", 0x100001, 0x080000, CRC(e905381b) SHA1(dcc93723e1d9ad118810bde5282bfa16c718a5f7) )
	ROM_LOAD16_BYTE( "obtr02.p5", 0x200000, 0x080000, CRC(5c634269) SHA1(231659180e44a3161ab9b27bf3336b8165cd7c75) )
	ROM_LOAD16_BYTE( "obtr02.p6", 0x200001, 0x080000, CRC(b151d179) SHA1(0a3bfde2310478c341b5ddae142f5efc9469a51c) )
	ROM_LOAD16_BYTE( "obtr02.p7", 0x300000, 0x080000, CRC(163ba007) SHA1(94a8618143910b63f70cd0e7339b337d6e81ef33) )
	ROM_LOAD16_BYTE( "obtr02.p8", 0x300001, 0x080000, CRC(5de27262) SHA1(c941275ce21506ab322ac0917725409a14660011) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "obtr02s.p1", 0x000000, 0x080000, CRC(decd39f4) SHA1(3b50b2c66fdfa59c9f2c646d9ff9586154d3ba0f) )
	ROM_LOAD16_BYTE( "obtr02b.p1", 0x000000, 0x080000, CRC(ffccc472) SHA1(d3346a1fe51d16833f071b1780a7cef8086bc385) )
	ROM_LOAD16_BYTE( "obtr02bd.p1", 0x000000, 0x080000, CRC(2562d2bd) SHA1(bd23e7d4065b6ec7037524f32d7a2904ea8e5594) )
	ROM_LOAD16_BYTE( "obtr02d.p1", 0x000000, 0x080000, CRC(1431f9dc) SHA1(6c186bc6f70a7dd2fba9eefe97640dbef0e21e1b) )
	ROM_LOAD16_BYTE( "obtr02dy.p1", 0x000000, 0x080000, CRC(eaf1771b) SHA1(78257088087050e1bbd7b6157f0ba702db25eb23) )
	ROM_LOAD16_BYTE( "obtr02k.p1", 0x000000, 0x080000, CRC(0e6e90e0) SHA1(e3cde3e95120297acc868339f7d82bb91787361c) )
	ROM_LOAD16_BYTE( "obtr02r.p1", 0x000000, 0x080000, CRC(f5de32f1) SHA1(dd5873f7cf2f73ef8a6ab5d620650e534ec19d91) )
	ROM_LOAD16_BYTE( "obtr02y.p1", 0x000000, 0x080000, CRC(200db733) SHA1(3203a88357df7c75976d7525e2a2853695a72aa4) )


ROM_END

ROM_START( m5openbx06 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "oboj06s.p1", 0x000000, 0x080000, CRC(5a73185e) SHA1(13ea115d9dbb5d3eb3e509538598e38e64443135) )
	ROM_LOAD16_BYTE( "oboj06.p2", 0x000001, 0x080000, CRC(2c1821aa) SHA1(874b1b377dcf21cb65f33f1823a48906c2a90d1d) )
	ROM_LOAD16_BYTE( "oboj06.p3", 0x100000, 0x080000, CRC(e93ecb5a) SHA1(ca65f9304bbdd1bbbc42a1bb062e9b32051400e8) ) // == 02
	ROM_LOAD16_BYTE( "oboj06.p4", 0x100001, 0x080000, CRC(e905381b) SHA1(dcc93723e1d9ad118810bde5282bfa16c718a5f7) ) // == 02
	ROM_LOAD16_BYTE( "oboj06.p5", 0x200000, 0x080000, CRC(5c634269) SHA1(231659180e44a3161ab9b27bf3336b8165cd7c75) ) // == 02
	ROM_LOAD16_BYTE( "oboj06.p6", 0x200001, 0x080000, CRC(b151d179) SHA1(0a3bfde2310478c341b5ddae142f5efc9469a51c) ) // == 02
	ROM_LOAD16_BYTE( "obtr06.p7", 0x300000, 0x080000, CRC(163ba007) SHA1(94a8618143910b63f70cd0e7339b337d6e81ef33) ) // missing but assumed the same
	ROM_LOAD16_BYTE( "oboj06.p8", 0x300001, 0x080000, CRC(5de27262) SHA1(c941275ce21506ab322ac0917725409a14660011) ) // == 02

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "oboj06ad.p1", 0x000000, 0x080000, CRC(ebf9d0b2) SHA1(cf1d3378d8a6e42b267a8d3e5c8f1cd48cccf3c0) )
	ROM_LOAD16_BYTE( "oboj06b.p1", 0x000000, 0x080000, CRC(bc0aaac3) SHA1(d22e9696e05fb57be8e01e0771678dec3a0d0080) )
	ROM_LOAD16_BYTE( "oboj06bd.p1", 0x000000, 0x080000, CRC(8fabe267) SHA1(62f55953a557429b0592440d8c87943271647cfb) )
	ROM_LOAD16_BYTE( "oboj06d.p1", 0x000000, 0x080000, CRC(cb82b47c) SHA1(15dd75e1e17002d2d365506b279d39c0a8f4fe12) )
	ROM_LOAD16_BYTE( "oboj06dy.p1", 0x000000, 0x080000, CRC(5fe9fcd0) SHA1(b7f1aa37dd4197c684d75efb31d75d9149559a50) )
	ROM_LOAD16_BYTE( "oboj06r.p1", 0x000000, 0x080000, CRC(a98a8838) SHA1(62efe36cf89e2d388da9d5ae5d4b6f464d610c41) )
	ROM_LOAD16_BYTE( "oboj06y.p1", 0x000000, 0x080000, CRC(ce1850f2) SHA1(e5216a70d7030f57f3e6c3c31fad7d3c159deaad) )
ROM_END

ROM_START( m5openbx05 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "obox05s.p1", 0x000000, 0x080000, CRC(f2defe83) SHA1(213ca25fd4b1efa3da0955e844e9439ba58d2be2) )
	ROM_LOAD16_BYTE( "obox05.p2", 0x000001, 0x080000, CRC(973a8bc5) SHA1(c18f0b8c86f0c21dad1cd911eecc8311bbd6d0e2) )
	/* 3,4,5,6,7,8 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "obox05ad.p1", 0x000000, 0x080000, CRC(3c26cb93) SHA1(9dc212f71d2d60398b6f3c62935168823b32599e) )
	ROM_LOAD16_BYTE( "obox05b.p1", 0x000000, 0x080000, CRC(fdc11791) SHA1(5963f8e9f7bb3773a4ee40f6394aa87c96091205) )
	ROM_LOAD16_BYTE( "obox05bd.p1", 0x000000, 0x080000, CRC(dd8ca413) SHA1(d21b08b14d3595166fcd341e08341c4c79e4b98e) )
	ROM_LOAD16_BYTE( "obox05d.p1", 0x000000, 0x080000, CRC(399d0384) SHA1(bf5372baf55fbd69a5aab49a7bc265a487b19c94) )
	ROM_LOAD16_BYTE( "obox05dy.p1", 0x000000, 0x080000, CRC(637fa940) SHA1(bafd071df4da8f9c20efa13f49bf94e105d3a9fd) )
	ROM_LOAD16_BYTE( "obox05r.p1", 0x000000, 0x080000, CRC(9b02cc32) SHA1(dd80648a9f4c4a342730a63bf988ffdf5ea00a2b) )
	ROM_LOAD16_BYTE( "obox05y.p1", 0x000000, 0x080000, CRC(a83c5447) SHA1(3c925fb6e53cf8569fe8f6af62662a6f510d5a3c) )
ROM_END

ROM_START( m5openbx01 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "obtr01s.p1", 0x000000, 0x080000, CRC(7ac858fc) SHA1(900e6d9c4ba994372f56a27f320ac76a9f794ccf) )
	ROM_LOAD16_BYTE( "obtr01.p2", 0x000001, 0x080000, CRC(87a1b987) SHA1(3c7b55cf7eb231fb1100677190800eb73d900cad) )
	ROM_LOAD16_BYTE( "obtr01.p3", 0x100000, 0x080000, CRC(e93ecb5a) SHA1(ca65f9304bbdd1bbbc42a1bb062e9b32051400e8) ) // == 02
	ROM_LOAD16_BYTE( "obtr01.p4", 0x100001, 0x080000, CRC(e905381b) SHA1(dcc93723e1d9ad118810bde5282bfa16c718a5f7) ) // == 02
	ROM_LOAD16_BYTE( "oboj06.p5", 0x200000, 0x080000, CRC(5c634269) SHA1(231659180e44a3161ab9b27bf3336b8165cd7c75) ) // dump was bad, but assume to be the same
	ROM_LOAD16_BYTE( "obtr01.p6", 0x200001, 0x080000, CRC(b151d179) SHA1(0a3bfde2310478c341b5ddae142f5efc9469a51c) ) // == 02
	ROM_LOAD16_BYTE( "obtr01.p7", 0x300000, 0x080000, CRC(163ba007) SHA1(94a8618143910b63f70cd0e7339b337d6e81ef33) ) // == 02
	ROM_LOAD16_BYTE( "obtr01.p8", 0x300001, 0x080000, CRC(5de27262) SHA1(c941275ce21506ab322ac0917725409a14660011) ) // == 02

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "obtr01ad.p1", 0x000000, 0x080000, CRC(00ebc5e7) SHA1(9a6a0de8a34cddabdd7f0481925cf2563ecaec16) )
	ROM_LOAD16_BYTE( "obtr01b.p1", 0x000000, 0x080000, CRC(00ad35e9) SHA1(6ee3c78b3aac115a90dc0fccc1a016197675c466) )
	ROM_LOAD16_BYTE( "obtr01bd.p1", 0x000000, 0x080000, CRC(c65d260d) SHA1(c0ebf204e4d983b2a8a0f8d83126eb661860e0c5) )
	ROM_LOAD16_BYTE( "obtr01d.p1", 0x000000, 0x080000, CRC(7e950ce2) SHA1(b36852fcb39b04d0b72e09d3b1bcc8cfca463a2b) )
	ROM_LOAD16_BYTE( "obtr01dy.p1", 0x000000, 0x080000, CRC(10864d45) SHA1(07026c5bfaefa85f2ce1f8f33fec0ea1c4834f9b) )
	ROM_LOAD16_BYTE( "obtr01k.p1", 0x000000, 0x080000, CRC(0854dfec) SHA1(b905bb50f1005565c469e30fcde908fdc76d492f) )
	ROM_LOAD16_BYTE( "obtr01y.p1", 0x000000, 0x080000, CRC(14db195b) SHA1(fc36de23d6106ddb23d53f84943d725ad0005f7f) )
ROM_END



ROM_START( m5overld )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "over04ad.p1", 0x000000, 0x080000, CRC(9fb6d993) SHA1(096073dc64a093d2aaf476098d396dde40cfc2ba) )
	ROM_LOAD16_BYTE( "over04.p2", 0x000001, 0x080000, CRC(9c5c8b75) SHA1(98aafa95b5358397f93ad4d0de1d9c99c1c2a384) )
	ROM_LOAD16_BYTE( "over04.p3", 0x100000, 0x080000, CRC(8f3820d5) SHA1(6936cb9cbcf3ad8256b2117a9258264d3b9ae510) )
	ROM_LOAD16_BYTE( "over04.p4", 0x100001, 0x080000, CRC(a51dce10) SHA1(e9f992c0110df7d472262a2b42adae256021fb3b) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "over04b.p1", 0x000000, 0x080000, CRC(473e8872) SHA1(971d43102cc2f2328309a5cceae5f9f15d4f7666) )
	ROM_LOAD16_BYTE( "over04d.p1", 0x000000, 0x080000, CRC(f6cc6ca8) SHA1(2ce2f1b00232f64927b73ee609a38d89bbade57c) )
	ROM_LOAD16_BYTE( "over04dy.p1", 0x000000, 0x080000, CRC(848c9d77) SHA1(258e496414786a6435bce5e6ea5de47234065c4f) )
	ROM_LOAD16_BYTE( "over04h.p1", 0x000000, 0x080000, CRC(eb24e033) SHA1(6db9f5df6ee90cfcb8566f3b771d97f752b464eb) )
	ROM_LOAD16_BYTE( "over04k.p1", 0x000000, 0x080000, CRC(c7e6636a) SHA1(52186f1814c085c9cb45f6ca3425b44387dc9d73) )
	ROM_LOAD16_BYTE( "over04r.p1", 0x000000, 0x080000, CRC(556c71a2) SHA1(b76f856204344eca847af5c4b49c37866627e95f) )
	ROM_LOAD16_BYTE( "over04s.p1", 0x000000, 0x080000, CRC(2ec75337) SHA1(17b4b1ed772cd98cbcbe90ad660ad326b30dbd07) )
	ROM_LOAD16_BYTE( "over04y.p1", 0x000000, 0x080000, CRC(5c87a2e8) SHA1(8bf6bd8ec7f7cddc4b9d9ef1ee1a14ad0f9bd520) )
ROM_END

ROM_START( m5overld02 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ovtr02s.p1", 0x000000, 0x080000, CRC(df8e97ec) SHA1(32184106840f9a108589b0fa39b213656ad2be9a) )
	ROM_LOAD16_BYTE( "ovtr02.p2", 0x000001, 0x080000, CRC(98e952cf) SHA1(17cd3e983ce5fc926b97b14165822f1368b21fc0) )
	ROM_LOAD16_BYTE( "ovtr02.p3", 0x100000, 0x080000, CRC(8f3820d5) SHA1(6936cb9cbcf3ad8256b2117a9258264d3b9ae510) ) // == 04
	ROM_LOAD16_BYTE( "ovtr02.p4", 0x100001, 0x080000, CRC(a51dce10) SHA1(e9f992c0110df7d472262a2b42adae256021fb3b) ) // == 04

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "ovtr02ad.p1", 0x000000, 0x080000, CRC(0e212027) SHA1(5f942a5a1770219cf3af5832d5e4f505d2760f05) )
	ROM_LOAD16_BYTE( "ovtr02b.p1", 0x000000, 0x080000, CRC(388bc4a2) SHA1(0bb059783d0f066e107270688e99107f3a6e884e) )
	ROM_LOAD16_BYTE( "ovtr02bd.p1", 0x000000, 0x080000, CRC(5d86e2f0) SHA1(5e424c2f74cfeb83df725b2505e65623770ffe59) )
	ROM_LOAD16_BYTE( "ovtr02d.p1", 0x000000, 0x080000, CRC(fb5de9a7) SHA1(dda866d39bdec1f3557cad512bc2d817c020d7eb) )
	ROM_LOAD16_BYTE( "ovtr02dy.p1", 0x000000, 0x080000, CRC(dfc55faf) SHA1(3056c1845a3631c2e9475f1370768d9e70aaedc0) )
	ROM_LOAD16_BYTE( "ovtr02k.p1", 0x000000, 0x080000, CRC(3819d62b) SHA1(bdcac76c95d77d84d69cfe8acfa60dc6fe2be240) )
	ROM_LOAD16_BYTE( "ovtr02y.p1", 0x000000, 0x080000, CRC(fb1621e4) SHA1(8525f9ad1d62ad56c6a87e6b4b298182d59b858b) )
ROM_END

ROM_START( m5overld10 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ovtr10s.p1", 0x000000, 0x080000, CRC(492d7256) SHA1(d0d8d7716dda58dbc4c2742989d65b9fd7aec874) )
	ROM_LOAD16_BYTE( "ovtr10.p2", 0x000001, 0x080000, CRC(59ea2bfb) SHA1(0e70cc772d2a9812ad975496e6183e6e0cdf5992) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "ovtr10ad.p1", 0x000000, 0x080000, CRC(fe7244ac) SHA1(4495788540663baec60e0cd7622ca486b689b6ab) )
	ROM_LOAD16_BYTE( "ovtr10b.p1", 0x000000, 0x080000, CRC(fb3ca925) SHA1(67ebc6c6ed59b9f34e8685788899ec19f2df3ff6) )
	ROM_LOAD16_BYTE( "ovtr10bd.p1", 0x000000, 0x080000, CRC(9bd2c3b3) SHA1(f49815f134514d696940ec45d1ebe93ea51fb0f7) )
	ROM_LOAD16_BYTE( "ovtr10d.p1", 0x000000, 0x080000, CRC(7aa9385f) SHA1(ee239554a16d503f257fcb9ab7ddb303f2d8cda3) )
	ROM_LOAD16_BYTE( "ovtr10dy.p1", 0x000000, 0x080000, CRC(680a2bc6) SHA1(489cfc45f04e6cba08a8cc376fef7d0d58761057) )
	ROM_LOAD16_BYTE( "ovtr10k.p1", 0x000000, 0x080000, CRC(55add8e7) SHA1(b5a96ea588fbb396128bc7c7ba652aa09efee5a1) )
	ROM_LOAD16_BYTE( "ovtr10r.p1", 0x000000, 0x080000, CRC(2ae78420) SHA1(11877ff2c2fa7d9423b00e2b7ab72eaedd5ecb75) )
	ROM_LOAD16_BYTE( "ovtr10y.p1", 0x000000, 0x080000, CRC(5b8e61cf) SHA1(8f089f886ec2ca4bbcaec8ccc61471f059fe11ed) )
ROM_END


ROM_START( m5overld11 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ovtr11s.p1", 0x000000, 0x080000, CRC(d90d0474) SHA1(f6264b9363b67c8d4f6ca187bf356f618b2d5b3d) )
	ROM_LOAD16_BYTE( "ovtr11.p2", 0x000001, 0x080000, CRC(738eaac3) SHA1(12353c1dbf7fb9e2832cc2175b39346fe51b64bb) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "ovtr11ad.p1", 0x000000, 0x080000, CRC(a8927c08) SHA1(27204136500686f05c9166832a5d01347acbe140) )
	ROM_LOAD16_BYTE( "ovtr11b.p1", 0x000000, 0x080000, CRC(abd0f33c) SHA1(ddba1f5187310538d10fd23588d48ab8bf5e20db) )
	ROM_LOAD16_BYTE( "ovtr11bd.p1", 0x000000, 0x080000, CRC(d184b0d6) SHA1(2f0fb7b3595190077bf71922d2e7efdefdb5fe8a) )
	ROM_LOAD16_BYTE( "ovtr11d.p1", 0x000000, 0x080000, CRC(e8ea4f5d) SHA1(596b368213063dc2800e28366b561c71088d05a5) )
	ROM_LOAD16_BYTE( "ovtr11dy.p1", 0x000000, 0x080000, CRC(4f4d0a62) SHA1(f533a1df6b27663a399a0a1815237c949316ddbc) )
	ROM_LOAD16_BYTE( "ovtr11k.p1", 0x000000, 0x080000, CRC(82a19750) SHA1(4d77bf448615786a2a485d1d05d09a64effd9720) )
	ROM_LOAD16_BYTE( "ovtr11r.p1", 0x000000, 0x080000, CRC(22da998e) SHA1(9c41846c0fd6756f97b6071b2a94ffaadbf843f5) )
	ROM_LOAD16_BYTE( "ovtr11y.p1", 0x000000, 0x080000, CRC(7eaa414b) SHA1(615aa48eeba17e9095dd45a6fcf3260fe63dac10) )
ROM_END


ROM_START( m5ptyani )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pani03ad.p1", 0x000000, 0x080000, CRC(761e72d3) SHA1(f8f3012db690e15a12725e40dd59b9be37a144ef) )
	ROM_LOAD16_BYTE( "pani03.p2", 0x000001, 0x080000, CRC(2ea4ead9) SHA1(a937c9f79bf5767c630adc7c7a698bff709465f3) )
	ROM_LOAD16_BYTE( "pani03.p3", 0x100000, 0x080000, CRC(76eb81df) SHA1(6fd6376eedf74330aa2b019ad0044ce39210610a) )
	ROM_LOAD16_BYTE( "pani03.p4", 0x100001, 0x080000, CRC(64f081ec) SHA1(22fc6ea30bb503247fe108e645c197e0fc90063a) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "pani03b.p1", 0x000000, 0x080000, CRC(29bb4427) SHA1(29810e31a9f08a1c079b87f1aab186fe4a0eb2c7) )
	ROM_LOAD16_BYTE( "pani03bd.p1", 0x000000, 0x080000, CRC(eadfac56) SHA1(e8d9e2893648d55591ed51812c42ffed3253c26f) )
	ROM_LOAD16_BYTE( "pani03d.p1", 0x000000, 0x080000, CRC(7a56c5b0) SHA1(167a86a298a78c9477a27c22617837334762bcd1) )
	ROM_LOAD16_BYTE( "pani03dy.p1", 0x000000, 0x080000, CRC(6c3fa2b1) SHA1(f109bc19f9f8841dc594faea06c00a24cd4c2d28) )
	ROM_LOAD16_BYTE( "pani03h.p1", 0x000000, 0x080000, CRC(2d3e8643) SHA1(bf4acd4fd84c2f5efa7bd54bb872445de9c2d32c) )
	ROM_LOAD16_BYTE( "pani03r.p1", 0x000000, 0x080000, CRC(88cd0a8f) SHA1(13f2edf275b8dc0e7a19c062267222bb6291f7c1) )
	ROM_LOAD16_BYTE( "pani03s.p1", 0x000000, 0x080000, CRC(b9322dc1) SHA1(7611162717ab6e524cdc85657e84f705a6a6b11d) )
	ROM_LOAD16_BYTE( "pani03y.p1", 0x000000, 0x080000, CRC(af5b4ac0) SHA1(a54d32092e7bebf8edcbc2accf20b35397faa9a8) )
ROM_END

ROM_START( m5ptyani01 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "patr01s.p1", 0x000000, 0x080000, CRC(0ff04c56) SHA1(c670b36b01d3e344a113f2195811de5b90144e85) )
	ROM_LOAD16_BYTE( "patr01.p2", 0x000001, 0x080000, CRC(aacd9a52) SHA1(0bc676d9708148f02ae121185a5b4f6eae34f3d4) )
	ROM_LOAD16_BYTE( "patr01.p3", 0x100000, 0x080000, CRC(76eb81df) SHA1(6fd6376eedf74330aa2b019ad0044ce39210610a) ) // == 03
	ROM_LOAD16_BYTE( "patr01.p4", 0x100001, 0x080000, CRC(64f081ec) SHA1(22fc6ea30bb503247fe108e645c197e0fc90063a) ) // == 03

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "patr01d.p1", 0x000000, 0x080000, CRC(71085bfa) SHA1(87dd1334077da967796c1847e3010de1acc8c32f) )
	ROM_LOAD16_BYTE( "patr01dy.p1", 0x000000, 0x080000, CRC(a9be6297) SHA1(b5d8b6e26d19a9af98665429d29bfcda7f3d4bfa) )
	ROM_LOAD16_BYTE( "patr01y.p1", 0x000000, 0x080000, CRC(d746753b) SHA1(c01b30937dd4d32ce7a306dcd6a2eb58bae39034) )
ROM_END

ROM_START( m5peepsh )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "peep05ad.p1", 0x000000, 0x080000, CRC(19d9c6e9) SHA1(47023aa50703352ca5cae9efc97e9f7e15c7e2a0) )
	ROM_LOAD16_BYTE( "peep05.p2", 0x000001, 0x080000, CRC(cff02d14) SHA1(998681cce858d0df2e5df3b5bb641b99c8afc48d) )
	ROM_LOAD16_BYTE( "peep05.p3", 0x100000, 0x080000, CRC(a9b2342a) SHA1(b7a76008a1bebc2aa279f4be49d810635f26b42e) )
	ROM_LOAD16_BYTE( "peep05.p4", 0x100001, 0x080000, CRC(d76f18bc) SHA1(70dfc9d8343c93c7a62df1ccc77dea401dd45e83) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "peep05b.p1", 0x000000, 0x080000, CRC(4c45d495) SHA1(b272850287a61f13240ddc9d5957b3d2b27a171d) )
	ROM_LOAD16_BYTE( "peep05bd.p1", 0x000000, 0x080000, CRC(19c34f02) SHA1(af80402ed22217a48b1ebfae5755ac0206aeed33) )
	ROM_LOAD16_BYTE( "peep05d.p1", 0x000000, 0x080000, CRC(39a776fb) SHA1(b2d9c1511612fdda3ff490ef5fd27a8cf39eb90d) )
	ROM_LOAD16_BYTE( "peep05dy.p1", 0x000000, 0x080000, CRC(361419ee) SHA1(a3fa02756b36d4b16f0ab5dcb6df70955027c302) )
	ROM_LOAD16_BYTE( "peep05h.p1", 0x000000, 0x080000, CRC(80b0d5b2) SHA1(711eb509bcd7dbb5e91a05853f7dcbdba661268b) )
	ROM_LOAD16_BYTE( "peep05r.p1", 0x000000, 0x080000, CRC(60c1ef29) SHA1(4e931324c91bcbf300ea6753c76cf23399eed7ba) )
	ROM_LOAD16_BYTE( "peep05s.p1", 0x000000, 0x080000, CRC(6c21ed6c) SHA1(c0315edb5646742d4efb83683a6c2b445b58bd65) )
	ROM_LOAD16_BYTE( "peep05y.p1", 0x000000, 0x080000, CRC(63928279) SHA1(2657bff6737b5355e4ca9d5fcca0e5cba1ffb46e) )
ROM_END

ROM_START( m5piefac )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pfrsja_3.0_1", 0x000000, 0x080000, CRC(1ef752cc) SHA1(078d462e5f6a55e9453de52574d3548b7d1fa3ab) )
	ROM_LOAD16_BYTE( "pfrsja_3.0_2", 0x000001, 0x080000, CRC(62314ea7) SHA1(41060cd58ab46cf889225132c060643276294c0c) )
	ROM_LOAD16_BYTE( "pfrsja_3.0_3", 0x100000, 0x080000, CRC(2423bd65) SHA1(2af707487ae5c9249b003f373532166b09ceed88) )
	ROM_LOAD16_BYTE( "pfrsja_3.0_4", 0x100001, 0x080000, CRC(5e002813) SHA1(e25023c9c529a9b6188ec8b25894e123f0d8d938) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "pfa20h2.7d1", 0x000000, 0x080000, CRC(fcdb0cd6) SHA1(267511de78c917f835bebb7885aed6e32d29954b) )
	ROM_LOAD16_BYTE( "pfasjh2.7d1", 0x000000, 0x080000, CRC(b00f044e) SHA1(933592ed5858c37ffa3362298c7f9e93dd52300c) )
	ROM_LOAD16_BYTE( "pfasjk2.7_1", 0x000000, 0x080000, CRC(5e4c58c6) SHA1(f1b86d44389598a101d5149414a5c22838a308be) )
	ROM_LOAD16_BYTE( "pfasjs2.7_1", 0x000000, 0x080000, CRC(470e2ce5) SHA1(7a726eeeb7b2000e7e438f42bf7f81fac9cedec7) )
	ROM_LOAD16_BYTE( "pfasjs2.7_2", 0x000000, 0x080000, CRC(260c96bb) SHA1(41d142b1ca00d62cf30a9048b50f9ef8f3a94323) )
	ROM_LOAD16_BYTE( "pfasjs2.7d1", 0x000000, 0x080000, CRC(a94d706d) SHA1(a47bed0a1a1e7bbf4de91c578d7f350c43d4313b) )
ROM_END

ROM_START( m5piefac23 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pfa_sjs2.3_1", 0x000000, 0x080000, CRC(208b6374) SHA1(e86da312b5974ed8d2bd607366544bc928616d01) )
	ROM_LOAD16_BYTE( "pfa_sjs2.3_2", 0x000001, 0x080000, CRC(1e3bfab1) SHA1(3faed74e454feb7892931ee7b5297d9a9e0eb1a9) )
	ROM_LOAD16_BYTE( "pfa_sjs2.3_3", 0x100000, 0x080000, CRC(f377dfea) SHA1(cf287d37c1b2cf39d91aa7879505a7608f1f9f87) )
	ROM_LOAD16_BYTE( "pfa_sjs2.3_4", 0x100001, 0x080000, CRC(e87ecff2) SHA1(a5c3200fd4171e45bd90580086f417ac807716d9) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "pfa_sjs2.3d1", 0x000000, 0x080000, CRC(46a29cd2) SHA1(e6896d7b96a8ef4b92d84b80bbc405629f1909aa) )
	ROM_LOAD16_BYTE( "pfa_sjs2.3d2", 0x000000, 0x080000, CRC(1e3bfab1) SHA1(3faed74e454feb7892931ee7b5297d9a9e0eb1a9) )
ROM_END

ROM_START( m5piefac12 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pfasjs1.2_1", 0x000000, 0x080000, CRC(5a8fa986) SHA1(19b0a4aa21926846af506b63945bf9e57c2a51e0) )
	ROM_LOAD16_BYTE( "pfasjs1.2_2", 0x000001, 0x080000, CRC(f35723c4) SHA1(83654a2f591c0c337ab02d628a4e818d91474e07) )
	/* 3+4 */
ROM_END

ROM_START( m5piefaca )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "p fac.p1", 0x0000, 0x080000, CRC(9fc9ca62) SHA1(c8b84c61877206ec1f78bb104b2d69f74f446a9d) )
	ROM_LOAD16_BYTE( "p fac.p2", 0x0001, 0x080000, NO_DUMP )
	/* 3+4 */
ROM_END

ROM_START( m5piefc2 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pf2sjs1.0_1", 0x000000, 0x080000, CRC(944bde9c) SHA1(392448db7609634e67b76c52cdbec17457f43be3) )
	ROM_LOAD16_BYTE( "pf2sjs1.0_2", 0x000001, 0x080000, CRC(8b814cb8) SHA1(6c5fd97a9c35f3c0a67eb25b3df8ad83f7b3072b) )
	ROM_LOAD16_BYTE( "pf2sjs1.0_3", 0x100000, 0x080000, CRC(8c5e99e7) SHA1(0a392c5389af8a9d046cae60c9ee4ac8309ac965) )
	ROM_LOAD16_BYTE( "pf2sjs1.0_4", 0x100001, 0x080000, CRC(a0a8a45c) SHA1(236894fb4d5be78c4a22219b979ebed1a98a4929) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "pf220h1.0d1", 0x000000, 0x080000, CRC(c23e0413) SHA1(a515233b7d2f12da9a1327f5c2c16e327f8880a7) )
	ROM_LOAD16_BYTE( "pf2sja1.0_1", 0x000000, 0x080000, CRC(7794c23d) SHA1(7807e482429202d19351b1a9cc4d833590c37a56) )
	ROM_LOAD16_BYTE( "pf2sjh1.0d1", 0x000000, 0x080000, CRC(26ae7e53) SHA1(a2fbea16a3e498648e988b81fd6865926f0d2eb5) )
	ROM_LOAD16_BYTE( "pf2sjk1.0_1", 0x000000, 0x080000, CRC(53d04513) SHA1(4c51023f30a18c5fccf928bd727cf46b055e6867) )
	ROM_LOAD16_BYTE( "pf2sjl1.0d1", 0x000000, 0x080000, CRC(1b9ef9fa) SHA1(d4782d44f81f03821fb7e10ad77403a5c2c6f6dd) )
	ROM_LOAD16_BYTE( "pf2sjs1.0d1", 0x000000, 0x080000, CRC(e135e5dc) SHA1(9578dda7d2b3c2101b744dd8b42513ad1e85b792) )
ROM_END

ROM_START( m5piefc2a )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "p fac2.p1",   0x000000, 0x080000, CRC(4182af62) SHA1(3db0e6f752f9d5195327fdeb859997a6e0da9c78) )
	ROM_LOAD16_BYTE( "p fac2.p2",   0x000001, 0x080000, CRC(a77513c5) SHA1(0a11d73f50036293d5d03bced8633c30d80566ae) )
	/* 3+4 */
ROM_END

ROM_START( m5piefc2b )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pie fac2.p1", 0x000000, 0x080000, CRC(fc802f6e) SHA1(6924f7e06653afee9a338f85d1403c1692f85a05) )
	ROM_LOAD16_BYTE( "pie fac2.p2", 0x000001, 0x080000, CRC(287fe650) SHA1(51c78838bb7d3268cc91ff01e3b8eb531b6d6976) )
	/* 3+4 */
ROM_END


ROM_START( m5piefcr )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pfrsjb_2.0_1", 0x000000, 0x080000, CRC(ddf3899a) SHA1(2094d4c9137bb114e05aff01d9ab508177a3fe2e) )
	ROM_LOAD16_BYTE( "pfrsjb_2.0_2", 0x000001, 0x080000, CRC(0fc2503e) SHA1(5a18b699159f6c98c692be0123b4d4d4aaadedde) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "pfrsjbd2.0_1", 0x000000, 0x080000, CRC(ba1b45c9) SHA1(fffed1b6c8702baf7f6726b2fef8741b22b69342) )
	ROM_LOAD16_BYTE( "pfrsjbg2.0_1", 0x000000, 0x080000, CRC(b43f188c) SHA1(22a9f32dfcd753911bbc3b112d51deeb4be9c49a) )
ROM_END




ROM_START( m5qshot )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "qstr02ad.p1", 0x000000, 0x080000, CRC(21e53ba3) SHA1(8c45cc4d89b7189338cc12c8abe00e584d76d06f) )
	ROM_LOAD16_BYTE( "qstr02.p2", 0x000001, 0x080000, CRC(c1aa4145) SHA1(8de4254d2ee4587309262aefd229e0707d65ca32) )
	ROM_LOAD16_BYTE( "qstr02.p3", 0x100000, 0x080000, CRC(b4c0bcd6) SHA1(75f40b2dfd25e3b927efc9e6874d90214813df3d) )
	ROM_LOAD16_BYTE( "qstr02.p4", 0x100001, 0x080000, CRC(3f78b3b9) SHA1(cb455e81f8910b88fd729e96c33b3d2b5ebba5a9) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "qstr02b.p1", 0x000000, 0x080000, CRC(af9b1b61) SHA1(bad13dbb658d8f298d1e69588af2e8fbec326bdb) )
	ROM_LOAD16_BYTE( "qstr02bd.p1", 0x000000, 0x080000, CRC(14b7022f) SHA1(cd2055042509941b5c023f713bf1f3e33eeaa5d4) )
	ROM_LOAD16_BYTE( "qstr02d.p1", 0x000000, 0x080000, CRC(7c83430d) SHA1(ba5ea0f2715e587ab49b19f772790685753e858f) )
	ROM_LOAD16_BYTE( "qstr02dy.p1", 0x000000, 0x080000, CRC(027ed8a9) SHA1(e9ab07acee04c1e77f212cc75df70f200d5f184e) )
	ROM_LOAD16_BYTE( "qstr02k.p1", 0x000000, 0x080000, CRC(9cdc20a7) SHA1(6857b501ea09fee021e1038ac9cb2f80bc40c9cc) )
	ROM_LOAD16_BYTE( "qstr02r.p1", 0x000000, 0x080000, CRC(36d69b6a) SHA1(6dc043520c0096320d95832ac18f66d9e871874b) )
	ROM_LOAD16_BYTE( "qstr02s.p1", 0x000000, 0x080000, CRC(a0379286) SHA1(fc47fbabea1ac0529d90947328703682cb0ed508) )
	ROM_LOAD16_BYTE( "qstr02y.p1", 0x000000, 0x080000, CRC(deca0922) SHA1(35c28f9a705bf9e9494a2c33812cd11942cae17f) )
ROM_END

ROM_START( m5qshot04 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "quac04s.p1", 0x000000, 0x080000, CRC(6aba0288) SHA1(c7970330c326a58cb96f632052a09c34ad066786) )
	ROM_LOAD16_BYTE( "quac04.p2", 0x000001, 0x080000, CRC(f03d404a) SHA1(6ee348eecded01fe6da5ff6e84d9c0a9473cd363) )
	ROM_LOAD16_BYTE( "quac04.p3", 0x100000, 0x080000, CRC(a76440d1) SHA1(24121278713ac7462a704c17919f54e02c4ef65e) )
	ROM_LOAD16_BYTE( "quac04.p4", 0x100001, 0x080000, CRC(31a978ee) SHA1(47019625bce45d4c706bb9d53f41138c1f92a960) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "quac04ad.p1", 0x000000, 0x080000, CRC(68cd7556) SHA1(23fa9af40cbad627664779743c87d712886924d8) )
	ROM_LOAD16_BYTE( "quac04b.p1", 0x000000, 0x080000, CRC(e28f3def) SHA1(4d7cad57f5eada5d2c10539533754aab87e08e29) )
	ROM_LOAD16_BYTE( "quac04bd.p1", 0x000000, 0x080000, CRC(3fd43dd5) SHA1(804ceccaecd2cce6e2a446edb80ea402d9892f19) )
	ROM_LOAD16_BYTE( "quac04d.p1", 0x000000, 0x080000, CRC(ec169c39) SHA1(f69df12b6b4b4e7f52895df238d708b26153c5a4) )
	ROM_LOAD16_BYTE( "quac04dy.p1", 0x000000, 0x080000, CRC(3a9a066b) SHA1(42851514b746ee527da79f7c83cd1c1aa1e9be65) )
	ROM_LOAD16_BYTE( "quac04h.p1", 0x000000, 0x080000, CRC(f1397c21) SHA1(c6ddc11e9df1093a5fe5e15dd3d3b4900ef249ec) )
	ROM_LOAD16_BYTE( "quac04k.p1", 0x000000, 0x080000, CRC(58664d66) SHA1(0d0908730a728689e581778c1071e1ebdff816a8) )
	ROM_LOAD16_BYTE( "quac04r.p1", 0x000000, 0x080000, CRC(4a2febe8) SHA1(c5ed21f47c34ba79a32923467bf57fca3173ee27) )
	ROM_LOAD16_BYTE( "quac04y.p1", 0x000000, 0x080000, CRC(1c7f751c) SHA1(b8f9b47a710b73fae5e44640fa52111c5fffae10) )
ROM_END


ROM_START( m5qdraw )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "qdr_sjs1.3_1", 0x000000, 0x080000, CRC(49b7cdd5) SHA1(dbade7ecc7189a1db7c38fd6b161ca67d3d142dc) )
	ROM_LOAD16_BYTE( "qdr_sjs1.3_2", 0x000001, 0x080000, CRC(0801be78) SHA1(d32e849d0ecf69c688dd85fffc55eacc907da998) )
	ROM_LOAD16_BYTE( "qdr_sjs1.3_3", 0x100000, 0x080000, CRC(f5205098) SHA1(1eedf489953a1328a62c6ae1fc696d783d102d15) )
	ROM_LOAD16_BYTE( "qdr_sjs1.3_4", 0x100001, 0x080000, CRC(28faa2a7) SHA1(debfa564e89c516a3c1de60dcdbfc26744797238) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "qdr_sjs1.3d1", 0x000000, 0x080000, CRC(f6a68804) SHA1(cce19e59e477fa9ee01636840030d1e2ad6e6d5b) )
ROM_END

ROM_START( m5qdraw12 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "qdr_sjs1.2_1", 0x000000, 0x080000, CRC(65cdc371) SHA1(e3d2de755ceb8e6144f316b49e5117d9c5e71e2e) )
	ROM_LOAD16_BYTE( "qdr_sjs1.2_2", 0x000001, 0x080000, CRC(661b6d33) SHA1(0ec2a2efc7ba28c7b1c40c471f2dda46b278f6ed) )
	/* 3+4 */
ROM_END

ROM_START( m5qdraw14 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "qdr_sjs1.4_1", 0x000000, 0x080000, CRC(59d1cf89) SHA1(d4e6b66589f6ac3ae99c4ac8a4fceafd1b3a9041) )
	ROM_LOAD16_BYTE( "qdr_sjs1.4_2", 0x000001, 0x080000, CRC(400099b2) SHA1(29fc0066393e28c949d513e972b7bd906151096d) )
	/* 3+4 */
ROM_END

ROM_START( m5qdraw15 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "qdr_sjs1.5_1", 0x000000, 0x080000, CRC(4b67abc5) SHA1(5c8453b7abb16f2f19c2340748ef87a2966868d8) )
	ROM_LOAD16_BYTE( "qdr_sjs1.5_2", 0x000001, 0x080000, CRC(a0d0a773) SHA1(5c2f4511b086dbf3376d5ae0d332bd0ee66d3db1) )
	/* 3+4 */
ROM_END

ROM_START( m5qdrawa )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "q_draw_pound8.p1", 0x0000, 0x080000, CRC(e093cb39) SHA1(d1de5249ee72c3ef589c08b712a2bf12b5d77785) )
	ROM_LOAD16_BYTE( "q_draw_pound8.p2", 0x0001, 0x080000, CRC(42fad134) SHA1(79d51b4fc3196b7c24ea2ce8bc4b4d3ef0326aa8) )
	/* 3+4 */
ROM_END

ROM_START( m5qdrawb )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "q_draw.p1", 0x0000, 0x080000, CRC(4647847f) SHA1(da84faf9f98a29e36e1fcb2236c1e294fee91ed7) )
	ROM_LOAD16_BYTE( "q_draw.p2", 0x0001, 0x080000, CRC(61747051) SHA1(9f556913b70cb5113e81bf112215e14b4a199a14) )
	/* 3+4 */
ROM_END



ROM_START( m5rainrn )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rar11s.p1", 0x000000, 0x080000, CRC(3d87c16a) SHA1(f604c653ab8ba55b222f553437c0b8a25006b3e9) )
	ROM_LOAD16_BYTE( "rar11s.p2", 0x000001, 0x080000, CRC(b51f90ee) SHA1(ef2f08b35e0d5021726c96f36578030d8b5da2a3) )
	ROM_LOAD16_BYTE( "rar11s.p3", 0x100000, 0x080000, CRC(286d6767) SHA1(4a13580003f7eccb86560b4fc9989c6ccd8a41ef) )
	ROM_LOAD16_BYTE( "rar11s.p4", 0x100001, 0x080000, CRC(821d2a8f) SHA1(583c28bc4400331ac942961efb08010029581e40) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "rar11a.p1", 0x000000, 0x080000, CRC(460a1847) SHA1(f9a9e1493f5d92df5e962f6e1166956d74f612a7) )
	ROM_LOAD16_BYTE( "rar11d.p1", 0x000000, 0x080000, CRC(bb6f9166) SHA1(64329c7c6b6398eb5efa91e18c3bf69a79b19e8f) )
	ROM_LOAD16_BYTE( "rar11k.p1", 0x000000, 0x080000, CRC(19455086) SHA1(5cb1a6ebcaa03d021fe043a7f963da2151e4a022) )
ROM_END


ROM_START( m5rainrna )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rar12s.p1", 0x000000, 0x080000, CRC(643f1f75) SHA1(3662f61c1ff24e469dcd44099b0358d65b2e55ec) )
	ROM_LOAD16_BYTE( "rar12s.p2", 0x000001, 0x080000, CRC(5448175c) SHA1(ca5217dcbf9c25f9fe09899feb9d1707a2fca63b) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "rar12a.p1", 0x000000, 0x080000, CRC(1fb2c658) SHA1(d0f7270023adfcbea747489375425c93e02632cb) )
	ROM_LOAD16_BYTE( "rar12d.p1", 0x000000, 0x080000, CRC(e2d74f79) SHA1(3b1bd26555c48beb744d8f7db57d4cf8f613dee0) )
	ROM_LOAD16_BYTE( "rar12k.p1", 0x000000, 0x080000, CRC(40fd8e99) SHA1(e5b80705d0ceea85b09384a4af1f286ab96a1595) )
ROM_END


ROM_START( m5ramrd )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "raid1_0.p1", 0x000000, 0x080000, CRC(5986bdc5) SHA1(a81fc789dfbab754a2d6dd00627f8cd1b6d5f83c) )
	ROM_LOAD16_BYTE( "raid1_0.p2", 0x000001, 0x080000, CRC(a4318fbe) SHA1(e97f28b451500b91fd086bc70d852ff0a7477567) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "raid1_0d.p1", 0x000000, 0x080000, CRC(3a2f2a06) SHA1(eae6c04749913f1eecd8af5a389b0c3b0fd61114) )
ROM_END

ROM_START( m5ramrcl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "crai0_1.p1", 0x000000, 0x080000, CRC(850234ef) SHA1(b1a2638c1109a6d60147e6ec25467516d212f027) )
	ROM_LOAD16_BYTE( "crai0_1.p2", 0x000001, 0x080000, CRC(388061f4) SHA1(d25d12e216dbe72c2c69618057f22590fd43f1f1) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "crai0_1d.p1", 0x000000, 0x080000, CRC(ddee6e88) SHA1(8fbceb79d3fe7288edab850361d446283aab4c25) )
ROM_END

ROM_START( m5rampg )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ram10s.p1", 0x000000, 0x080000, CRC(7b3ff3ee) SHA1(e5a66782a619bfdee1b72e34bcfd2f18071cbd0c) )
	ROM_LOAD16_BYTE( "ram10s.p2", 0x000001, 0x080000, CRC(30b298ef) SHA1(a4b0ec72c29252c93df0881dea2ce045f194a9e9) )
	ROM_LOAD16_BYTE( "ram10s.p3", 0x100000, 0x080000, CRC(7f6506f3) SHA1(cdddd27e4a08c2096615fdda139eecb9f0f7b9c9) )
	ROM_LOAD16_BYTE( "ram10s.p4", 0x100001, 0x080000, CRC(6c064d79) SHA1(636e76daf22d1b879a71a6fb53a42e0d6e72fa02) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "ram10d.p1", 0x000000, 0x080000, CRC(fdd7a3e2) SHA1(5a60ea931272fc3c9d940a12acf4ab62dbf567a1) )
	ROM_LOAD16_BYTE( "ram10k.p1", 0x000000, 0x080000, CRC(5ffd6202) SHA1(27f7cdcdb4e6688ddcd01cd4acb07a2f59909069) )
ROM_END

ROM_START( m5rampg11 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ram11s.p1", 0x000000, 0x080000, CRC(6de5a249) SHA1(066e22e4c952aad2e0cf1662abf9717c7d566fc7) )
	ROM_LOAD16_BYTE( "ram11s.p2", 0x000001, 0x080000, CRC(7d767a18) SHA1(edfdb4f9106755206b9793a3d915f78ec528dac2) )
	ROM_LOAD16_BYTE( "ram11s.p3", 0x100000, 0x080000, CRC(7f6506f3) SHA1(cdddd27e4a08c2096615fdda139eecb9f0f7b9c9) ) // == 10
	ROM_LOAD16_BYTE( "ram11s.p4", 0x100001, 0x080000, CRC(6c064d79) SHA1(636e76daf22d1b879a71a6fb53a42e0d6e72fa02) ) // == 10

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "ram11d.p1", 0x000000, 0x080000, CRC(eb0df245) SHA1(0e33abdba5327c0759181aebb4cf3a2abe443f0c) )
	ROM_LOAD16_BYTE( "ram11k.p1", 0x000000, 0x080000, CRC(492733a5) SHA1(8df4c05cd962b73f368f99770350233f12aedfc5) )
ROM_END

ROM_START( m5rampg12 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ram12s.p1", 0x000000, 0x080000, CRC(1993bff3) SHA1(914fe7dc038fa16bff9dc01fb1eaca6fe8247427) )
	ROM_LOAD16_BYTE( "ram12s.p2", 0x000001, 0x080000, CRC(fa643375) SHA1(c360f28c4c268e674e12fc4c82ef82c3861bcfc6) )
	ROM_LOAD16_BYTE( "ram12s.p3", 0x100000, 0x080000, CRC(7f6506f3) SHA1(cdddd27e4a08c2096615fdda139eecb9f0f7b9c9) ) // == 10
	ROM_LOAD16_BYTE( "ram12s.p4", 0x100001, 0x080000, CRC(6c064d79) SHA1(636e76daf22d1b879a71a6fb53a42e0d6e72fa02) ) // == 10

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "ram12d.p1", 0x000000, 0x080000, CRC(9f7befff) SHA1(0d95e21fc751ffcb671c49cfe48ab7f79d9a1f92) )
	ROM_LOAD16_BYTE( "ram12k.p1", 0x000000, 0x080000, CRC(3d512e1f) SHA1(f441f7b01882497b7334abf91aefb2cd218ba440) )
ROM_END

ROM_START( m5ratpk )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rpa_11s.p1", 0x000000, 0x080000, CRC(eaa2391e) SHA1(628925fdbfef41a4b68c33580349bacb5864c83a) )
	ROM_LOAD16_BYTE( "rpa_11l.p2", 0x000001, 0x080000, CRC(1a42be5f) SHA1(8917d7d9640d33bb6d017aee10df72bc9d3bf94a) )
	ROM_LOAD16_BYTE( "rpa_11l.p3", 0x100000, 0x080000, CRC(80b1ff60) SHA1(d076bfb0aa1fdf98b9cffde2778e2d29477628bf) )
	ROM_LOAD16_BYTE( "rpa_11l.p4", 0x100001, 0x080000, CRC(b590ac98) SHA1(342d5d930826b882a607088fd5e87949a0e0509a) )
ROM_END

ROM_START( m5ratpka )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rat pack.p1", 0x0000, 0x080000, CRC(1f35dd38) SHA1(2767b168957b6c6eb906137a3d3f19b45f351ada) )
	ROM_LOAD16_BYTE( "ratpack.p2",  0x0001, 0x080000, CRC(b038b966) SHA1(b272f741ac3bc0e9c65829246ea28b2384b5f7c0) )
	/* 3+4 */
ROM_END


ROM_START( m5razdz )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "razd0_1d.p1", 0x000000, 0x080000, CRC(92972cd4) SHA1(553276d112c23f5ab050631ca08c1b7ef7905168) )
	ROM_LOAD16_BYTE( "razd0_1.p2", 0x000001, 0x080000, CRC(7fcccbb0) SHA1(45a4583f67d1e5d4859cb93f237ea1ef8b66f4d0) )
	ROM_LOAD16_BYTE( "razd0_1.p3", 0x100000, 0x080000, CRC(797b287c) SHA1(6ba19a64012a11d1fbc419ca19de3f10f23c2fba) )
	ROM_LOAD16_BYTE( "razd0_1.p4", 0x100001, 0x080000, CRC(4ea310f5) SHA1(a551eaa7ec60357467f2f064c2532c8e2e3ee847) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "razd0_1f.p1", 0x000000, 0x080000, CRC(670bcb60) SHA1(c583b0bf9bee9d7f80358cb00e8f87a9ab496fbd) )
	ROM_LOAD16_BYTE( "razd0_1s.p1", 0x000000, 0x080000, CRC(3f4ada4c) SHA1(d9aa2c9ea921a922cbcc7154888527b29852fb2e) )
	ROM_LOAD16_BYTE( "razd0_1z.p1", 0x000000, 0x080000, CRC(125de68d) SHA1(61b34737a1be840e66d9dd9424ead2d6f10cd804) )
ROM_END


ROM_START( m5razdz10 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "razd10s.p1", 0x000000, 0x080000, CRC(73e1c88f) SHA1(eba35b684c49f57cf9d187fe7f45afc9cb42b579) )
	ROM_LOAD16_BYTE( "razd1_0.p2", 0x000001, 0x080000, CRC(261a6659) SHA1(d35eb70a75d3b42e6a88572ed9066e7463ba3da4) )
	ROM_LOAD16_BYTE( "razd1_0.p3", 0x100000, 0x080000, CRC(797b287c) SHA1(6ba19a64012a11d1fbc419ca19de3f10f23c2fba) ) // == 01
	ROM_LOAD16_BYTE( "razd1_0.p4", 0x100001, 0x080000, CRC(4ea310f5) SHA1(a551eaa7ec60357467f2f064c2532c8e2e3ee847) ) // == 01

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "razd1_0d.p1", 0x000000, 0x080000, CRC(a1a8f5c8) SHA1(ebb7335014f159086c09f0613a2ee4cf44973085) )
	ROM_LOAD16_BYTE( "razd1_0f.p1", 0x000000, 0x080000, CRC(8c50c7e1) SHA1(3108eff77132ac4f3afbc1bc28e60fa5ac850112) )
	ROM_LOAD16_BYTE( "razd1_0z.p1", 0x000000, 0x080000, CRC(96079b30) SHA1(00c7b73c950545e33f237508cd9489159904150e) )
ROM_END

ROM_START( m5razdz11 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "razd11s.p1", 0x000000, 0x080000, CRC(24ff1d6f) SHA1(5e48bce3531077a91523e1b1a0f33f20f127bcc3) )
	ROM_LOAD16_BYTE( "razd11.p2", 0x000001, 0x080000, CRC(18d92ffe) SHA1(2255db0988579cba11a67de33d84a5d4a2dc1ec2) )
	ROM_LOAD16_BYTE( "razd11.p3", 0x100000, 0x080000, CRC(797b287c) SHA1(6ba19a64012a11d1fbc419ca19de3f10f23c2fba) ) // == 01
	ROM_LOAD16_BYTE( "razd11.p4", 0x100001, 0x080000, CRC(4ea310f5) SHA1(a551eaa7ec60357467f2f064c2532c8e2e3ee847) ) // == 01

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "razd11d.p1", 0x000000, 0x080000, CRC(94b4738c) SHA1(68b5f8cfeec7ae5ce0fc7c1ed01a20fa665a722d) )
	ROM_LOAD16_BYTE( "razd11dz.p1", 0x000000, 0x080000, CRC(a3eae895) SHA1(7bc1144ab90a87186b93d9a627a20d199783f378) )
	ROM_LOAD16_BYTE( "razd11f.p1", 0x000000, 0x080000, CRC(835bc34c) SHA1(a87c734b43ba00ba908abf13eb9cc00e151c65f7) )
	ROM_LOAD16_BYTE( "razd11z.p1", 0x000000, 0x080000, CRC(edb2fb16) SHA1(57f33a7df1c76991d42cd24079e007b2f923f6b8) )
ROM_END


ROM_START( m5redrck )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rtro03ad.p1", 0x000000, 0x080000, CRC(1ff60355) SHA1(ecca3df147b3a07f32abdecaa7dba82dc97963f4) )
	ROM_LOAD16_BYTE( "rtro03.p2", 0x000001, 0x080000, CRC(05687914) SHA1(86d92f954746216155cd4ab3fddc2b02892d7dcb) )
	ROM_LOAD16_BYTE( "rtro03.p3", 0x100000, 0x080000, CRC(9519dc05) SHA1(32c46c95ccedb99f1e517242bc89249020fbd66c) )
	ROM_LOAD16_BYTE( "rtro03.p4", 0x100001, 0x080000, CRC(5af465d8) SHA1(e484e495a4fe92ab6ccdfc3069ce528293d4f277) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "rtro03b.p1", 0x000000, 0x080000, CRC(690e661b) SHA1(15e2851c4d8f26f0084f6d02b38d49d7069a7a32) )
	ROM_LOAD16_BYTE( "rtro03bd.p1", 0x000000, 0x080000, CRC(8e496439) SHA1(c0eb1a553b151a1ec6828df5f19ce1006a993201) )
	ROM_LOAD16_BYTE( "rtro03d.p1", 0x000000, 0x080000, CRC(21b58714) SHA1(02c5ae8dd4730eddd6a80e4e9b14262cd07433bc) )
	ROM_LOAD16_BYTE( "rtro03dy.p1", 0x000000, 0x080000, CRC(e4af6001) SHA1(ffc7dd17d8964c6be231eb3746de61d95d275a5b) )
	ROM_LOAD16_BYTE( "rtro03h.p1", 0x000000, 0x080000, CRC(e4ceb656) SHA1(920dbdcbdf57e34b062e0642a481389c85772793) )
	ROM_LOAD16_BYTE( "rtro03k.p1", 0x000000, 0x080000, CRC(24871c20) SHA1(5d0642c4eed2e9426ab6a71ce3f194477e7f466c) )
	ROM_LOAD16_BYTE( "rtro03r.p1", 0x000000, 0x080000, CRC(61c44f77) SHA1(30957a46198bab6b57319f720f549f4c5120eaa1) )
	ROM_LOAD16_BYTE( "rtro03s.p1", 0x000000, 0x080000, CRC(2ed29353) SHA1(3b4e7a85a0f45167f115e24511b443532926bb3b) )
	ROM_LOAD16_BYTE( "rtro03y.p1", 0x000000, 0x080000, CRC(ebc87446) SHA1(7193718c21fc73d76665941e1d58fe0c98eb0daa) )
ROM_END

ROM_START( m5redrck10 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rttr10s.p1", 0x000000, 0x080000, CRC(60229f1d) SHA1(653d6f11bf1a5665a203137a8129e03632bee22b) )
	ROM_LOAD16_BYTE( "rttr10.p2", 0x000001, 0x080000, CRC(38066b10) SHA1(116b8dac97ed2060243e78ed94bc121076c66dfe) )
	ROM_LOAD16_BYTE( "rttr10.p3", 0x100000, 0x080000, CRC(9519dc05) SHA1(32c46c95ccedb99f1e517242bc89249020fbd66c) )
	ROM_LOAD16_BYTE( "rttr10.p4", 0x100001, 0x080000, CRC(5af465d8) SHA1(e484e495a4fe92ab6ccdfc3069ce528293d4f277) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "rttr10ad.p1", 0x000000, 0x080000, CRC(28ebaa1a) SHA1(5f93efc9b089680cfcf854f9ce91a7fabcd0646c) )
	ROM_LOAD16_BYTE( "rttr10b.p1", 0x000000, 0x080000, CRC(05e3597f) SHA1(fe5b6f71f1dc84be90ed4c687c0f07bed7ab9fce) )
	ROM_LOAD16_BYTE( "rttr10bd.p1", 0x000000, 0x080000, CRC(03117f03) SHA1(e92491e25df2744b5c6de06acf4f4144d9b481a7) )
	ROM_LOAD16_BYTE( "rttr10d.p1", 0x000000, 0x080000, CRC(f6c96bca) SHA1(a95d55205a4a5c43693bfaea19d29b411e3b05d7) )
	ROM_LOAD16_BYTE( "rttr10dy.p1", 0x000000, 0x080000, CRC(58f38053) SHA1(3edbf81c1c402b14f1055947b862c5d7eb962631) )
	ROM_LOAD16_BYTE( "rttr10k.p1", 0x000000, 0x080000, CRC(e775d2ab) SHA1(074601f7f13db908bed2e42f4c7e4940b062767c) )
	ROM_LOAD16_BYTE( "rttr10r.p1", 0x000000, 0x080000, CRC(b314837b) SHA1(f3c2e607cfc4c8056915771f4d67a6c990f29aa0) )
	ROM_LOAD16_BYTE( "rttr10y.p1", 0x000000, 0x080000, CRC(ce187484) SHA1(bce0c86418efd7f15ef75c5d73e95ed1b10ac458) )
ROM_END

ROM_START( m5redrcka )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "r_trock.p1", 0x00000, 0x080000, CRC(ba0b2f5d) SHA1(e4ac11da00f5c0b2b2a3bc68d429870039583036) ) // don't know what this pairs with
	ROM_LOAD16_BYTE( "r_trock.p2", 0x00001, 0x080000, NO_DUMP )
	/* 3+4 */
ROM_END


ROM_START( m5rdwarf )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rdw05s.p1", 0x000000, 0x080000, CRC(51fd1aed) SHA1(767b5c91783f8394f817ec222c0653b2fee8061f) )
	ROM_LOAD16_BYTE( "rdw05s.p2", 0x000001, 0x080000, CRC(e0e4d0e8) SHA1(69e4d1fb43fecb902d4d954c64f6e018240c955c) )
	ROM_LOAD16_BYTE( "rdw05s.p3", 0x100000, 0x080000, CRC(3cbac357) SHA1(efcfa7b1c72cd5e18ecfe01c0880eafc34d58278) )
	ROM_LOAD16_BYTE( "rdw05s.p4", 0x100001, 0x080000, CRC(86d28605) SHA1(44d3e5000bd485eaf7c833acb822ce875ba50de9) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "rdw05d.p3", 0x000000, 0x080000, CRC(f848ea4c) SHA1(0958c402f2207ab7da8c988b78e9d552799376ad) )
ROM_END


ROM_START( m5rhkni )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rhkn08ad.p1", 0x000000, 0x080000, CRC(8c1eaf80) SHA1(1a4cc97061f20c11790707559d795a96aab4276e) )
	ROM_LOAD16_BYTE( "rhkn08.p2", 0x000001, 0x080000, CRC(c0fafdbb) SHA1(76458624424f651f2d1e6d7c2940cd3603c24e3f) )
	ROM_LOAD16_BYTE( "rhkn08.p3", 0x100000, 0x080000, CRC(3e7747de) SHA1(a7e8088099fcf3428531421003be81535aec4469) )
	ROM_LOAD16_BYTE( "rhkn08.p4", 0x100001, 0x080000, CRC(04017640) SHA1(f3c9877c7a210a2ae23e324b05fafa9cb0004612) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "rhkn08b.p1", 0x000000, 0x080000, CRC(3ebbdcdd) SHA1(5bc574b74e0ad3c001cb8d3affd1aa764b673fbf) )
	ROM_LOAD16_BYTE( "rhkn08bd.p1", 0x000000, 0x080000, CRC(ead30865) SHA1(0644c86a5a9da523494f9e610656f3ef0e5e4823) )
	ROM_LOAD16_BYTE( "rhkn08d.p1", 0x000000, 0x080000, CRC(ecbb4f4e) SHA1(0b77e65b5bb5a59ad8fbb18d2308580dcf5132d9) )
	ROM_LOAD16_BYTE( "rhkn08dy.p1", 0x000000, 0x080000, CRC(75669a46) SHA1(5fee1b705d495cda1e6a6391760b4793b9ff050b) )
	ROM_LOAD16_BYTE( "rhkn08h.p1", 0x000000, 0x080000, CRC(af7e63fb) SHA1(0c50ce7889b588b28599ffef0ca66f82bfd5bcf9) )
	ROM_LOAD16_BYTE( "rhkn08r.p1", 0x000000, 0x080000, CRC(0d531092) SHA1(444ea8c332501d987a5800955fa839e1e6234d7e) )
	ROM_LOAD16_BYTE( "rhkn08s.p1", 0x000000, 0x080000, CRC(38d39bf6) SHA1(d0b1d26206c0315949179cef1e8fec20a61943e1) )
	ROM_LOAD16_BYTE( "rhkn08y.p1", 0x000000, 0x080000, CRC(a10e4efe) SHA1(559de4e5d0f5c5b8177d3c545f4eeba2627b4644) )
ROM_END


ROM_START( m5rhrg )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "grhr08ad.p1", 0x000000, 0x080000, CRC(d20fea65) SHA1(365bf493561bc8bbda53a4e16edfb2dbbfcdaf2f) )
	ROM_LOAD16_BYTE( "grhr08.p2", 0x000001, 0x080000, CRC(9d194292) SHA1(a5b61b3fafb36ca791002926a93ae14299fd96cc) )
	ROM_LOAD16_BYTE( "grhr08.p3", 0x100000, 0x080000, CRC(096ec192) SHA1(d01768d8cdc9e21e499b61b60a15905d7abc5597) )
	ROM_LOAD16_BYTE( "grhr08.p4", 0x100001, 0x080000, CRC(a231bcf6) SHA1(f24072053c4679437140c89cdcd3452ae5a60e45) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "grhr08b.p1", 0x000000, 0x080000, CRC(ab9a7821) SHA1(22dfe44ceb1e840b09e877f5cc9140499e63eeb7) )
	ROM_LOAD16_BYTE( "grhr08bd.p1", 0x000000, 0x080000, CRC(273c2852) SHA1(4dfd2450e6b5623ad881da5339b77f1e0f3ca109) )
	ROM_LOAD16_BYTE( "grhr08d.p1", 0x000000, 0x080000, CRC(23240874) SHA1(20254f9b1ba38c10311cbcf4e01781cfd0dd956f) )
	ROM_LOAD16_BYTE( "grhr08dy.p1", 0x000000, 0x080000, CRC(01517cfa) SHA1(50c0af562c1327c9d32ee7831360aed0575db107) )
	ROM_LOAD16_BYTE( "grhr08r.p1", 0x000000, 0x080000, CRC(9522d0e1) SHA1(f082d665c35a10d713de802d8648fde8f210c31e) )
	ROM_LOAD16_BYTE( "grhr08s.p1", 0x000000, 0x080000, CRC(ab11f041) SHA1(ca901f3d738c18bf3f412e4141198d0b3e39b855) )
	ROM_LOAD16_BYTE( "grhr08y.p1", 0x000000, 0x080000, CRC(896484cf) SHA1(53fc51848ff3816a9bb3c4197cdd450e7c9273ea) )
ROM_END

ROM_START( m5rhrga )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rhr_gold.p1", 0x00000, 0x080000, CRC(2ddaedaa) SHA1(0a96d0faffec393bf3d719829a33f0efb98f393d) )
	ROM_LOAD16_BYTE( "rhr_gold.p2", 0x00001, 0x080000, CRC(b90b0f08) SHA1(2b83ed20fd229fb32da68bc04af14216335fb1cb) )
	/* 3+4 */
ROM_END


ROM_START( m5rhrgt )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rhrt13ad.p1", 0x000000, 0x080000, CRC(b712fbaf) SHA1(571cbf727f186f360b1fd5ff7acda5cc9b5c954e) )
	ROM_LOAD16_BYTE( "rhrt13.p2", 0x000001, 0x080000, CRC(fe5490f5) SHA1(1dfb53ec6f5d6dc5ab015568163578cd89fee81c) )
	ROM_LOAD16_BYTE( "rhrt13.p3", 0x100000, 0x080000, CRC(77a16da8) SHA1(1598e547b39d4fe58e8f05aea5f949d2b0a9678d) )
	ROM_LOAD16_BYTE( "rhrt13.p4", 0x100001, 0x080000, CRC(eb880b1f) SHA1(65402cd0a5cdc95e5c3a7be2f100e1e19eb45152) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */

	ROM_LOAD16_BYTE( "rhrt13b.p1", 0x000000, 0x080000, CRC(a13b98b6) SHA1(908667963b144be7f8c1a2f3fe56bfb01dc50546) )
	ROM_LOAD16_BYTE( "rhrt13bd.p1", 0x000000, 0x080000, CRC(2fecaf4d) SHA1(f5817e81c407c03a207e76671f0dab8185ee7316) )
	ROM_LOAD16_BYTE( "rhrt13d.p1", 0x000000, 0x080000, CRC(5b1e28f1) SHA1(143ccab26b2b0f65f7d45041dcf8f3600bc0f4f6) )
	ROM_LOAD16_BYTE( "rhrt13dy.p1", 0x000000, 0x080000, CRC(2ef881d1) SHA1(0b876e09257dbb26460cca4a3b6137eca46618aa) )
	ROM_LOAD16_BYTE( "rhrt13k.p1", 0x000000, 0x080000, CRC(bff8c28a) SHA1(ff43032c7943ab39acde84fe5deb5c2e82b8c85f) )
	ROM_LOAD16_BYTE( "rhrt13r.p1", 0x000000, 0x080000, CRC(5cc0bb5a) SHA1(611ea18816046d385ab3b7bf37c7be9f672f7d32) )
	ROM_LOAD16_BYTE( "rhrt13s.p1", 0x000000, 0x080000, CRC(7755253c) SHA1(d3a35796c5bfb5523ca73fd4cdd1462695cb10c9) )
ROM_END

ROM_START( m5rhrgt12 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rhrt12.p1", 0x000000, 0x080000, CRC(c0a03387) SHA1(e6613ad6a969276566d6d408d49f7cd88bde6f0c) )
	ROM_LOAD16_BYTE( "rhrt12.p2", 0x000001, 0x080000, CRC(9a9a9c45) SHA1(2b178bbc57a1dd8bf347002f9deb7f1690e89423) )
	ROM_LOAD16_BYTE( "rhrt12.p3", 0x100000, 0x080000, CRC(77a16da8) SHA1(1598e547b39d4fe58e8f05aea5f949d2b0a9678d) ) // == 13
	ROM_LOAD16_BYTE( "rhrt12.p4", 0x100001, 0x080000, CRC(eb880b1f) SHA1(65402cd0a5cdc95e5c3a7be2f100e1e19eb45152) ) // == 13

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "rhrt12ad.p1", 0x000000, 0x080000, CRC(65d6d9dd) SHA1(d53278b0b5e8b3bcab8bc9da008b016e1b3a2dfa) )
	ROM_LOAD16_BYTE( "rhrt12b.p1", 0x000000, 0x080000, CRC(7643fca7) SHA1(03916131ce827e2b903644b1857c20dc8af6dc20) )
	ROM_LOAD16_BYTE( "rhrt12bd.p1", 0x000000, 0x080000, CRC(433a5db5) SHA1(dafbbd7c3eb694ecad346d52f9b43283ca261e61) )
	ROM_LOAD16_BYTE( "rhrt12d.p1", 0x000000, 0x080000, CRC(562bff53) SHA1(12748f1c5d53d322b94acc6418ef11be2a64dbe0) )
	ROM_LOAD16_BYTE( "rhrt12dy.p1", 0x000000, 0x080000, CRC(5da1d744) SHA1(9c6d911816543cd426bf4b519a1d0ce48e58ccad) )
	ROM_LOAD16_BYTE( "rhrt12k.p1", 0x000000, 0x080000, CRC(c4d11d45) SHA1(c1859bfdc4345c287acd9194863e24fec2fd190d) )
	ROM_LOAD16_BYTE( "rhrt12r.p1", 0x000000, 0x080000, CRC(19e13c56) SHA1(435dd10d46ba2561d4c90bf83666f2740e88efa0) )
	ROM_LOAD16_BYTE( "rhrt12s.p1", 0x000000, 0x080000, CRC(c0a03387) SHA1(e6613ad6a969276566d6d408d49f7cd88bde6f0c) )
	ROM_LOAD16_BYTE( "rhrt12y.p1", 0x000000, 0x080000, CRC(cb2a1b90) SHA1(4a633db25be468336bc61bcf272571ab587eac00) )
ROM_END

ROM_START( m5rhrgt02 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rhrb02.p1", 0x000000, 0x080000, CRC(eca8267f) SHA1(f503b98c2b6000128c8a32b097e69316922263ca) )
	ROM_LOAD16_BYTE( "rhrb02.p2", 0x000001, 0x080000, CRC(4966f2fc) SHA1(2e334a344ddf0779d2d663f13560c17befd5888a) )
	/* 3+4 */
ROM_END



ROM_START( m5redx )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rex10s.p1", 0x000000, 0x080000, CRC(d1722f4f) SHA1(c08104d2f21a7bfff0ddd981f57ea6f56cde4b3b) )
	ROM_LOAD16_BYTE( "rex10s.p2", 0x000001, 0x080000, CRC(0017b3f2) SHA1(4a4663c1bd03f1238661816a8cec63136e41729b) )
	ROM_LOAD16_BYTE( "rex10s.p3", 0x100000, 0x080000, CRC(bbb762ca) SHA1(0cf47a2484cc8f9247bfbade15fdc3c59d40908b) )
	ROM_LOAD16_BYTE( "rex10s.p4", 0x100001, 0x080000, CRC(354cf631) SHA1(55975fb404415764e4a968a97f9ea6b9fea55166) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "rex10d.p1", 0x000000, 0x080000, CRC(579a7f43) SHA1(e8334e664febb06b1ab6cc64ac95f1796cadc99d) )
	ROM_LOAD16_BYTE( "rex10e.p1", 0x000000, 0x080000, CRC(c31367b9) SHA1(a35bc9b9cf18db23be9d30dce586223819d5347b) )
	ROM_LOAD16_BYTE( "rex10k.p1", 0x000000, 0x080000, CRC(f5b0bea3) SHA1(7090b39296169f21b68c4dc6ea2d91f7338ecfc6) )
ROM_END

ROM_START( m5redx12 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rex12s.p1", 0x000000, 0x080000, CRC(4174b386) SHA1(93aed5290d88e1950cc6fecbfa22cf046a550d50) )
	ROM_LOAD16_BYTE( "rex12s.p2", 0x000001, 0x080000, CRC(69a353d7) SHA1(5bbe832a0bfc6e8a96c282db310c262da90b6ca5) )
	ROM_LOAD16_BYTE( "rex12s.p3", 0x100000, 0x080000, CRC(bbb762ca) SHA1(0cf47a2484cc8f9247bfbade15fdc3c59d40908b) ) // == 10
	ROM_LOAD16_BYTE( "rex12s.p4", 0x100001, 0x080000, CRC(354cf631) SHA1(55975fb404415764e4a968a97f9ea6b9fea55166) ) // == 10

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "rex12d.p1", 0x000000, 0x080000, CRC(c79ce38a) SHA1(f3a79598c3b2790601bb4d23f609e5f2733c3c54) )
	ROM_LOAD16_BYTE( "rex12k.p1", 0x000000, 0x080000, CRC(65b6226a) SHA1(56d2f881d23d8d4c157d4519f39f154777bbd1a7) )
ROM_END


ROM_START( m5thtsmg )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "t magic.p1", 0x000000, 0x080000, CRC(25270bf7) SHA1(e91efeb41e641c7f354f5a28ade8d50a92afc00a) )
	ROM_LOAD( "t magic.p2", 0x000001, 0x080000, CRC(990b7ff1) SHA1(60aadda1e7628e12fde543c7ce21a1e470a4da86) )
	ROM_LOAD( "t magic.p3", 0x100000, 0x080000, CRC(964dcee7) SHA1(fe55bed33a64f01f4414582b232ce96d412d8040) )
	ROM_LOAD( "t magic.p4", 0x100001, 0x080000, CRC(1b22663a) SHA1(ed9dc51cffad9c8634cbad3feab0bfef83ca0824) )
ROM_END

ROM_START( m5ronr )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "reel0_4.p1", 0x000000, 0x080000, CRC(d02655e6) SHA1(8fa3a10c7f6ec6550dadac76ef08bb6325e7216e) )
	ROM_LOAD16_BYTE( "reel0_4.p2", 0x000001, 0x080000, CRC(f1d445bc) SHA1(ff4af707dba0f896418cabf06ab550fd3904e4a2) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "reel0_4d.p1", 0x000000, 0x080000, CRC(97220bfd) SHA1(b5b4801a908ba8900f7bdf63a9dbefe46a89019c) )
ROM_END

ROM_START( m5ronr05 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "reel0_5.p1", 0x000000, 0x080000, CRC(b189633b) SHA1(f446a4c09db67acf08abdcccdd6691fb054abc2f) )
	ROM_LOAD16_BYTE( "reel0_5.p2", 0x000001, 0x080000, CRC(e3b1a2fc) SHA1(0f862f024a415afde9017b5cb365d2af6b7d90c8) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "reel0_5d.p1", 0x000000, 0x080000, CRC(399d294c) SHA1(dd1c0921ecac5b4f4c6687e9b329da18b8d0ccc0) )
ROM_END

ROM_START( m5ronr07 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "reel0_7.p1", 0x000000, 0x080000, CRC(c0b67cd9) SHA1(f7e906f29ee66a95e3ca823ddfd0e1fa6051fd10) )
	ROM_LOAD16_BYTE( "reel0_7.p2", 0x000001, 0x080000, CRC(6f3c3513) SHA1(42881d4deccb9d4ea82f68555d84504b5769b48e) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "reel0_7d.p1", 0x000000, 0x080000, CRC(7201e47c) SHA1(f0d92b6d0ff528ca6de19be3262ff47f9c486a22) )
ROM_END


ROM_START( m5resfrg )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "frog0_4.p1", 0x000000, 0x080000, CRC(33d98382) SHA1(69714b9e7ad795f1a926b310d249ee430d58fb2c) )
	ROM_LOAD16_BYTE( "frog0_4.p2", 0x000001, 0x080000, CRC(7b02d59b) SHA1(803a8d6155ab7d70dfcf7e8212dffdbb94b27991) )
	ROM_LOAD16_BYTE( "frog0_4.p3", 0x100000, 0x080000, CRC(1c24ce06) SHA1(cad539550e891764855923fc5c48493c70f42013) )
	ROM_LOAD16_BYTE( "frog0_4.p4", 0x100001, 0x080000, CRC(d47c1e09) SHA1(b6a941861fc78024273ba23210d0ed2ee4f961ae) )
ROM_END


ROM_START( m5rthh )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hous0_3.p1", 0x000000, 0x080000, CRC(6998fe56) SHA1(c0d29ece53946897479ba4e79b7ff0a28ecbf19a) )
	ROM_LOAD16_BYTE( "hous0_3.p2", 0x000001, 0x080000, CRC(668c1b67) SHA1(c254cba8bba22dcecd09259eeee62ecfe32f2616) )
	ROM_LOAD16_BYTE( "hous0_3.p3", 0x100000, 0x080000, CRC(89746560) SHA1(fa677a91440492e51c3946e8fe69d778579c891c) )
	ROM_LOAD16_BYTE( "hous0_3.p4", 0x100001, 0x080000, CRC(6ec92919) SHA1(4dab8a5f7533ba96783cd448b0596243d52dbfaf) )
ROM_END

ROM_START( m5revo )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "retr01d.p1", 0x000000, 0x080000, CRC(aea0285a) SHA1(a4e26107d23ec15c12647b0ad8aca2e983d3272d) )
	ROM_LOAD16_BYTE( "retr01.p2", 0x000001, 0x080000, CRC(30c7d689) SHA1(7e93fe545156ddf9ac7aaa5291d32a0ccc1b381f) )
	ROM_LOAD16_BYTE( "retr01.p3", 0x100000, 0x080000, CRC(594cd05f) SHA1(6183a9e3c9c14e19245ca457a3267ceceb8490b3) )
	ROM_LOAD16_BYTE( "retr01.p4", 0x100001, 0x080000, CRC(d80f34a2) SHA1(87d3bd9809a1fa96217e27de3778b231ef6c41a1) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "retr01dy.p1", 0x000000, 0x080000, CRC(0a21733d) SHA1(14d708ca5f56d2600b82e95e247780709beda00c) )
	ROM_LOAD16_BYTE( "retr01k.p1", 0x000000, 0x080000, CRC(e2b24e4b) SHA1(9923e12a40a69fff200d8d57a3e85b1bdae16cc8) )
	ROM_LOAD16_BYTE( "retr01s.p1", 0x000000, 0x080000, CRC(0a7cd89f) SHA1(a749fda39311699cfdd001ca5c58574c857fff26) )
	ROM_LOAD16_BYTE( "retr01y.p1", 0x000000, 0x080000, CRC(aefd83f8) SHA1(a1aacdc4ba66eaffb92cb2a9f0a3cd46150b47d3) )
ROM_END

ROM_START( m5revo13 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "revl13s.p1", 0x000000, 0x080000, CRC(c0bb81e9) SHA1(32ae7ec84fd4dcef09e86af9af21fd07301c0ffc) )
	ROM_LOAD16_BYTE( "revl13.p2", 0x000001, 0x080000, CRC(44bb41e0) SHA1(3a55312369e13f007e3541e6cbe4cf0694c681a4) )
	ROM_LOAD16_BYTE( "revl13.p3", 0x100000, 0x080000, CRC(594cd05f) SHA1(6183a9e3c9c14e19245ca457a3267ceceb8490b3) ) // == 01
	ROM_LOAD16_BYTE( "revl13.p4", 0x100001, 0x080000, CRC(d80f34a2) SHA1(87d3bd9809a1fa96217e27de3778b231ef6c41a1) ) // == 01

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "revl13ad.p1", 0x000000, 0x080000, CRC(48eb39a1) SHA1(c78145688c66215d971d18fcf7a5ff593efe4aff) )
	ROM_LOAD16_BYTE( "revl13b.p1", 0x000000, 0x080000, CRC(45555b0c) SHA1(6483bd52cc82f2943e261f9fd8ec0409222deb25) )
	ROM_LOAD16_BYTE( "revl13bd.p1", 0x000000, 0x080000, CRC(f656251f) SHA1(b74d17498b6432cc8baa5a86b072ad29605d316b) )
	ROM_LOAD16_BYTE( "revl13d.p1", 0x000000, 0x080000, CRC(73b8fffa) SHA1(0416813d7a8cef51409e468727ac675d8ca883c0) )
	ROM_LOAD16_BYTE( "revl13dy.p1", 0x000000, 0x080000, CRC(84022296) SHA1(ecc64dfb5c4309bccacd73b6a61596540be6ed29) )
	ROM_LOAD16_BYTE( "revl13h.p1", 0x000000, 0x080000, CRC(e9249a08) SHA1(244c58ba795f5ee9c403e9fd4da82358992a9092) )
	ROM_LOAD16_BYTE( "revl13k.p1", 0x000000, 0x080000, CRC(4149b71b) SHA1(cb4ffe1d971d54d48bc27709a6ca21c53b5dd20f) )
	ROM_LOAD16_BYTE( "revl13r.p1", 0x000000, 0x080000, CRC(cc878b86) SHA1(e6e3a4535647f695d16678660e6d7f0e129d81ea) )
	ROM_LOAD16_BYTE( "revl13y.p1", 0x000000, 0x080000, CRC(37015c85) SHA1(168f5c280f5cb9ab4923b136b9e4984f609bc6fb) )
ROM_END

ROM_START( m5revoa )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "revltn.p1", 0x00000, 0x080000, CRC(c78ed23e) SHA1(918d4a7c44c493a50edf8af0f1e8a4fdfbc1561d) )
	ROM_LOAD16_BYTE( "revltn.p2", 0x00001, 0x080000, CRC(57fc569a) SHA1(b93dc5366a914855a6f640145ec6be37556259bb) )
	/* 3+4 */
ROM_END


ROM_START( m5rgclb )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rgrd10ad.p1", 0x000000, 0x080000, CRC(6d8d3e4c) SHA1(36c18411318b5d580237865851a97fabbde3a94e) )
	ROM_LOAD16_BYTE( "rgrd10.p2", 0x000001, 0x080000, CRC(0198d79c) SHA1(3ecfd3b0f1c369c807b2e86413ce84ec4508ecda) )
	ROM_LOAD16_BYTE( "rgrd10.p3", 0x100000, 0x080000, CRC(70561f08) SHA1(68e5addbb7e2cbf9b2ba7d9b67f7f8573012691c) )
	ROM_LOAD16_BYTE( "rgrd10.p4", 0x100001, 0x080000, CRC(75bdd705) SHA1(126c7499852c83470fc377ba321bfa940898b4d1) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "rgrd10d.p1", 0x000000, 0x080000, CRC(368099d9) SHA1(abefb56764b63b9715fef2940764b3780ecb64db) )
	ROM_LOAD16_BYTE( "rgrd10dz.p1", 0x000000, 0x080000, CRC(17558773) SHA1(127ca3ef5fb7f962db9f1e6e21bb44a3f64c21c3) )
	ROM_LOAD16_BYTE( "rgrd10f.p1", 0x000000, 0x080000, CRC(31c94199) SHA1(b44e4a97b6aff7d861e32d54fee7a2b779210594) )
	ROM_LOAD16_BYTE( "rgrd10s.p1", 0x000000, 0x080000, CRC(ab0e679e) SHA1(dc1a0c9a0054f5bf164cd8c96dae56856764ab5e) )
	ROM_LOAD16_BYTE( "rgrd10z.p1", 0x000000, 0x080000, CRC(b7898460) SHA1(d592cb3b26d043fcb10eacd92ebff4a72084a4db) )
ROM_END

ROM_START( m5rgclb11 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rgrd11s.p1", 0x000000, 0x080000, CRC(e2585ef6) SHA1(db8bf9bcd73146c816e8f26693051944c71b1fdb) )
	ROM_LOAD16_BYTE( "rgrd11.p2", 0x000001, 0x080000, CRC(78baf476) SHA1(6b8f26a2de236637838940649886fdcf55ac44f6) )
	ROM_LOAD16_BYTE( "rgrd11.p3", 0x100000, 0x080000, CRC(3a4500da) SHA1(91a136802e8dbd97f9fffcc7fb141883be095357) )
	ROM_LOAD16_BYTE( "rgrd11.p4", 0x100001, 0x080000, CRC(3f41a551) SHA1(d5d005b4bd52d940d77c174d43a00c530b1048ac) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "rgrd11ad.p1", 0x000000, 0x080000, CRC(5a34decf) SHA1(13327305fec5ad9b552ba27ec212e4e5298e8d5b) )
	ROM_LOAD16_BYTE( "rgrd11d.p1", 0x000000, 0x080000, CRC(37fc7036) SHA1(05834a773b15a9fd0b2d117f3092e3f189fc4d8c) )
	ROM_LOAD16_BYTE( "rgrd11dz.p1", 0x000000, 0x080000, CRC(ac6e4a62) SHA1(43f69ec6c60bf85d4a71f223f81a2d7a11634187) )
	ROM_LOAD16_BYTE( "rgrd11f.p1", 0x000000, 0x080000, CRC(7ae309e6) SHA1(570c4ee2ddd554708929bd8e0aa2efadfe08d854) )
	ROM_LOAD16_BYTE( "rgrd11z.p1", 0x000000, 0x080000, CRC(33bd923d) SHA1(49421cb956b2cba1869909e5eedab4431b66db56) )
ROM_END

ROM_START( m5rgclb12 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rgrd12s.p1", 0x000000, 0x080000, CRC(c80320b4) SHA1(3320e3be578fa50c5c99b1bb805eca5e5706672f) )
	ROM_LOAD16_BYTE( "rgrd12.p2", 0x000001, 0x080000, CRC(7f9d77c5) SHA1(06f00dbc8aab75658d2286b8a6167700ec9d81fb) )
	ROM_LOAD16_BYTE( "rgrd12.p3", 0x100000, 0x080000, CRC(3a4500da) SHA1(91a136802e8dbd97f9fffcc7fb141883be095357) ) // == 11
	ROM_LOAD16_BYTE( "rgrd12.p4", 0x100001, 0x080000, CRC(3f41a551) SHA1(d5d005b4bd52d940d77c174d43a00c530b1048ac) ) // == 11

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "rgrd12d.p1", 0x000000, 0x080000, CRC(c5d288d2) SHA1(216683139b197b106b8ff6fc59b03f13364457b2) )
	ROM_LOAD16_BYTE( "rgrd12dz.p1", 0x000000, 0x080000, CRC(4b5ece49) SHA1(b5a5cd4071d7fc0389d58c90dcdbb15161a51506) )
	ROM_LOAD16_BYTE( "rgrd12f.p1", 0x000000, 0x080000, CRC(ca522928) SHA1(ea6992946f9f02af068fae04f8db736a92d52353) )
	ROM_LOAD16_BYTE( "rgrd12z.p1", 0x000000, 0x080000, CRC(3ad02c75) SHA1(f7b6b571316c1d997cb4b1ad97c025869b76b21f) )
ROM_END

ROM_START( m5rgclb20 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rgrd2_0s.p1", 0x000000, 0x080000, CRC(117b145c) SHA1(a1436433ab19309f7b414d0373ff0cddb8c22574) )
	ROM_LOAD16_BYTE( "rgrd2_0.p2", 0x000001, 0x080000, CRC(84f814a6) SHA1(f3a8331776bb4bea8c85bf1ceef253b6ca86b6be) )
	ROM_LOAD16_BYTE( "rgrd2_0.p3", 0x100000, 0x080000, CRC(3a4500da) SHA1(91a136802e8dbd97f9fffcc7fb141883be095357) ) // == 11
	ROM_LOAD16_BYTE( "rgrd2_0.p4", 0x100001, 0x080000, CRC(3f41a551) SHA1(d5d005b4bd52d940d77c174d43a00c530b1048ac) ) // == 11

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "rgrd2_0d.p1", 0x000000, 0x080000, CRC(dfb791a9) SHA1(1d88a77cf0167dd2b62d1489f8829fdbf07fb066) )
	ROM_LOAD16_BYTE( "rgrd2_0f.p1", 0x000000, 0x080000, CRC(a7233ad4) SHA1(98856761bdad41ecd5b5f95caab976bae3de8336) )
	ROM_LOAD16_BYTE( "rgrd2_0z.p1", 0x000000, 0x080000, CRC(45d1d501) SHA1(60a8999c6de449d53643445035b884501d7745d8) )
	ROM_LOAD16_BYTE( "rgrd20dz.p1", 0x000000, 0x080000, CRC(0680d075) SHA1(06a2902b6dbb0367a960b072821684c792e248d4) )
ROM_END

ROM_START( m5rgclb21 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rgrd21s.p1", 0x000000, 0x080000, CRC(5c588df6) SHA1(9f70f8d0b8bd784aa45080e2bbf90cdc71f795dd) )
	ROM_LOAD16_BYTE( "rgrd21.p2", 0x000000, 0x080000, CRC(481a686d) SHA1(9acab4b87b112fcbf5b124d7101e4937ca754c80) )
	ROM_LOAD16_BYTE( "rgrd21.p3", 0x000000, 0x080000, CRC(3a4500da) SHA1(91a136802e8dbd97f9fffcc7fb141883be095357) ) // == 11
	ROM_LOAD16_BYTE( "rgrd21.p4", 0x000000, 0x080000, CRC(3f41a551) SHA1(d5d005b4bd52d940d77c174d43a00c530b1048ac) ) // == 11

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "rgrd21d.p1", 0x000000, 0x080000, CRC(bcffa73e) SHA1(ea6c37ec35b80364b872153fdbdc502587a3ba70) )
	ROM_LOAD16_BYTE( "rgrd21dz.p1", 0x000000, 0x080000, CRC(22b3818c) SHA1(45679c82c0aba39a8cdf3d09f45ca8ec60f5a38e) )
	ROM_LOAD16_BYTE( "rgrd21f.p1", 0x000000, 0x080000, CRC(20001df3) SHA1(c4fdba37e7ef3a1903dd9d14bc3b1fccd5be8cda) )
	ROM_LOAD16_BYTE( "rgrd21z.p1", 0x000000, 0x080000, CRC(c1033c4e) SHA1(956bb4e9ac0a9116e4c86ae920669e4f042c2fa8) )
ROM_END

ROM_START( m5rgclb03 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "trgb03s.p1", 0x000000, 0x080000, CRC(443282e6) SHA1(d904554fffd0bdc79fc29b38a00fa6abbf1bb877) )
	ROM_LOAD16_BYTE( "trgb03.p2", 0x000001, 0x080000, CRC(097fb87a) SHA1(100eb1235508fb84a0c70b91f4eaefd5da47f6cb) )
	ROM_LOAD16_BYTE( "trgb03.p3", 0x100000, 0x080000, CRC(97d97647) SHA1(2dd1af527e614703690fb3c881c88602e6f7636e) )
	ROM_LOAD16_BYTE( "trgb03.p4", 0x100001, 0x080000, CRC(58a7a86f) SHA1(a5847bc90b6ae3f013b7954b6cdb379997f820c6) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "trgb03d.p1", 0x000000, 0x080000, CRC(2d04f3b2) SHA1(6b63d701b99b25b2014683a8dfb4b565ed58207b) )
	ROM_LOAD16_BYTE( "trgb03dz.p1", 0x000000, 0x080000, CRC(9e522f80) SHA1(7192fef47f2c0dae04fb3810faa0c41e765994fb) )
	ROM_LOAD16_BYTE( "trgb03f.p1", 0x000000, 0x080000, CRC(0a7c741c) SHA1(a36bd9d14d3e6ca72f71ed964e5eaad538324abe) )
	ROM_LOAD16_BYTE( "trgb03z.p1", 0x000000, 0x080000, CRC(453d4aeb) SHA1(4802695999a2dc402d484d698bbde387a1a368af) )
ROM_END

ROM_START( m5rgclb01 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "trgb0_1s.p1", 0x000000, 0x080000, CRC(22438be4) SHA1(ff6c1ddacb59283cbcf2a78de24586863bf6c224) )
	ROM_LOAD16_BYTE( "trgb0_1.p2", 0x000001, 0x080000, CRC(0b52162a) SHA1(41e84d3fa1f5ab148d59dadb91a1ce9690255a14) )
	ROM_LOAD16_BYTE( "trgb0_1.p3", 0x100000, 0x080000, CRC(97d97647) SHA1(2dd1af527e614703690fb3c881c88602e6f7636e) ) // == 03
	ROM_LOAD16_BYTE( "trgb0_1.p4", 0x100001, 0x080000, CRC(58a7a86f) SHA1(a5847bc90b6ae3f013b7954b6cdb379997f820c6) ) // == 03

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "trgb0_1d.p1", 0x000000, 0x080000, CRC(5c3f22d9) SHA1(b5c09c5b913958ca723c6e15d29c57437e8bdf22) )
	ROM_LOAD16_BYTE( "trgb0_1f.p1", 0x000000, 0x080000, CRC(d7a4f6c6) SHA1(666b9444e3d49bfc6754c4ca93eb0ccc80fd7db3) )
	ROM_LOAD16_BYTE( "trgb0_1z.p1", 0x000000, 0x080000, CRC(e48a6483) SHA1(b5348b720f4f7e1d229214777527503c4a204665) )
ROM_END

ROM_START( m5rgclb01a )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "trgt0_1.p1", 0x000000, 0x080000, CRC(f95ef3cd) SHA1(39138eb9d93e3384cd00ed0b373d798eb39ff14a) )
	ROM_LOAD16_BYTE( "trgt0_1.p2", 0x000001, 0x080000, CRC(cf639023) SHA1(ffbbdfb728db82c9ed09d626ef0fe34bf4011c0c) )
	ROM_LOAD16_BYTE( "trgt0_1.p3", 0x100000, 0x080000, CRC(475dbfba) SHA1(3777ec1cd5e7c8c2a5e059e71680a7e51e217af6) )
	ROM_LOAD16_BYTE( "trgt0_1.p4", 0x100001, 0x080000, CRC(e3762417) SHA1(dec2b476019520aff5a1343db4bbd86473d74e43) )
ROM_END



ROM_START( m5rollup )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rrusjb_1.9_1", 0x000000, 0x080000, CRC(d7f2b763) SHA1(fcf32e169134056487a7f0e94bd0fb0fbeed4596) )
	ROM_LOAD16_BYTE( "rrusjb_1.9_2", 0x000001, 0x080000, CRC(56fd2ef6) SHA1(045eb2d84be1b04d225f000c40ec1df8c3fbe26f) )
	ROM_LOAD16_BYTE( "rrusjb_1.9_3", 0x100000, 0x080000, CRC(c7a90660) SHA1(53405c1f0df900ab32872269a1e2f19693dea209) )
	ROM_LOAD16_BYTE( "rrusjb_1.9_4", 0x100001, 0x080000, CRC(52e10142) SHA1(737e225fe7a30a705a36d980f2c70f72ae29736b) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "rrusja_1.9_1", 0x000000, 0x080000, CRC(f8ee1d72) SHA1(69dcd632a6134669d1e3c73da8a1168541eca290) )
	ROM_LOAD16_BYTE( "rrusjad1.9_1", 0x000000, 0x080000, CRC(38960fc9) SHA1(3c5c180b6fbafe0bb3087bf0863f18d053360d23) )
	ROM_LOAD16_BYTE( "rrusjbd1.9_1", 0x000000, 0x080000, CRC(178aa5d8) SHA1(d72c66c57e58aa575755af7a4a50a676c420cff2) )
ROM_END

ROM_START( m5rollx )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "roll0_1.p1", 0x000000, 0x080000, CRC(106a3860) SHA1(513ea0d20426835bd2fd0d025236b5b8f1582e95) )
	ROM_LOAD16_BYTE( "roll0_1.p2", 0x000001, 0x080000, CRC(386d40c3) SHA1(b189cef5d0912ccba910d77b14d5f4cd9b3da68e) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "roll0_1d.p1", 0x000000, 0x080000, CRC(62c7bc9f) SHA1(175c05bb14fa64df1cbae1029ba48b965f3cbeb6) )
ROM_END

ROM_START( m5rollx12 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "roll1_2.p1", 0x000000, 0x080000, CRC(e03478f4) SHA1(730a5d6b496d7295e42c6a08817ccc4e224142e2) )
	ROM_LOAD16_BYTE( "roll1_2.p2", 0x000001, 0x080000, CRC(6a2ac261) SHA1(a33737f8a586da07f2363b6af4a6d6325c95729b) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "roll1_2d.p1", 0x000000, 0x080000, CRC(f0b08f9b) SHA1(e00dcbcc6466bc6e76ce5b65db5accab2a0e7def) )
ROM_END


ROM_START( m5rcx )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rexg07d.p1", 0x000000, 0x080000, CRC(4e2cadfe) SHA1(16a0f93ccc611eebf066686d56391e17e291aba4) )
	ROM_LOAD16_BYTE( "rexg07.p2", 0x000001, 0x080000, CRC(a7b8c4fb) SHA1(6d1732ddb0a43a2ecd9fc3149c173b05c1376a63) )
	ROM_LOAD16_BYTE( "rexg07.p3", 0x100000, 0x080000, CRC(b57e21cc) SHA1(82aa46af24102e5926f9795125a255bf2e364586) )
	ROM_LOAD16_BYTE( "rexg07.p4", 0x100001, 0x080000, CRC(9506e010) SHA1(41a52fc51668c5212323eb1b8ff5a2b90345763b) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "rexg07dz.p1", 0x000000, 0x080000, CRC(489a23a0) SHA1(2120fa9f4c85adda6aff278496668959e7bf73bf) )
	ROM_LOAD16_BYTE( "rexg07f.p1", 0x000000, 0x080000, CRC(fde39242) SHA1(8297fe62f2ee80e0e2d084bbc55215ddd03f5880) )
	ROM_LOAD16_BYTE( "rexg07s.p1", 0x000000, 0x080000, CRC(70a67696) SHA1(da9f1836f0b523d4ff282f1dfb8c381a094913d4) )
	ROM_LOAD16_BYTE( "rexg07z.p1", 0x000000, 0x080000, CRC(810af718) SHA1(b78368d8aea9e07fda61bc87b3f9f8ee4e9b0a89) )
ROM_END

ROM_START( m5rcxa )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "r.exc", 0x00000, 0x080000, CRC(78ba3908) SHA1(b2728fd099a6540e7d53b5268257f844dbb772db) )
	ROM_LOAD16_BYTE( "r_exclub.p2", 0x00001, 0x080000, CRC(14456741) SHA1(0a8bfcc806b014f863855f9aae8a17a385a31255) )
ROM_END

ROM_START( m5ritj )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ritj03ad.p1", 0x000000, 0x080000, CRC(1c45d96f) SHA1(7f4aba13073e35bc88b6d67d6182b9c9b57a63f7) )
	ROM_LOAD16_BYTE( "ritj03.p2", 0x000001, 0x080000, CRC(c8d2c53c) SHA1(c3ab1959a522f19ec3ceff3496f388debe22f0ae) )
	ROM_LOAD16_BYTE( "ritj03.p3", 0x100000, 0x080000, CRC(68b68bbf) SHA1(a722c3701439ad49cd5c87ab57fccacb6f61500d) )
	ROM_LOAD16_BYTE( "ritj03.p4", 0x100001, 0x080000, CRC(83ab3e1a) SHA1(27fd29e25ab5ea720f7987341ae78450fbaba751) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "ritj03b.p1", 0x000000, 0x080000, CRC(ee15d813) SHA1(fdfffcc51091fe361990205baacb886b9b9356be) )
	ROM_LOAD16_BYTE( "ritj03bd.p1", 0x000000, 0x080000, CRC(17f79b78) SHA1(b76d89b4eb29288b4dcc87de5d7f4922f06ce784) )
	ROM_LOAD16_BYTE( "ritj03d.p1", 0x000000, 0x080000, CRC(b6cd82eb) SHA1(04a31f882ba06631234933c9ebfee81061dd38cd) )
	ROM_LOAD16_BYTE( "ritj03dy.p1", 0x000000, 0x080000, CRC(1f92445c) SHA1(f52c32478f8010d672722cf2fdc0251037ba0a30) )
	ROM_LOAD16_BYTE( "ritj03h.p1", 0x000000, 0x080000, CRC(df360a3d) SHA1(c74aeedf933d9d0d24287ed57c25638ee67037da) )
	ROM_LOAD16_BYTE( "ritj03r.p1", 0x000000, 0x080000, CRC(f0d5141e) SHA1(197cc644e091b7776e1ff398e5e7f1b639623679) )
	ROM_LOAD16_BYTE( "ritj03s.p1", 0x000000, 0x080000, CRC(4f2fc180) SHA1(5f9bb6bbde75de5745231468e7bfde40da1b63ae) )
	ROM_LOAD16_BYTE( "ritj03y.p1", 0x000000, 0x080000, CRC(e6700737) SHA1(89711ef1c2383a4cb2fb2b8f3ae0466d5391dd41) )
ROM_END


ROM_START( m5rfymc )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rmnc05d.p1", 0x000000, 0x080000, CRC(84d971a3) SHA1(46f2b7da01c0692494015d9498f3a3dcfacfebd6) )
	ROM_LOAD16_BYTE( "rmnc05.p2", 0x000001, 0x080000, CRC(73decd42) SHA1(ba969813c9b1cfe0062507f6425a18197b661b30) )
	ROM_LOAD16_BYTE( "rmnc05.p3", 0x100000, 0x080000, CRC(054ed61f) SHA1(b45418cbd0b77accfa70cbcdfb4c06906682ad3a) )
	ROM_LOAD16_BYTE( "rmnc05.p4", 0x100001, 0x080000, CRC(b10c292a) SHA1(cc9bebd291a8ed3f12ddcd4dcde379dc9cd4841e) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "rmnc05_1.p1", 0x000000, 0x080000, CRC(9c83704e) SHA1(128fd987f9f9f1382175af73b40b9cd4e7c25d6c) )
	ROM_LOAD16_BYTE( "rmnc05ad.p1", 0x000000, 0x080000, CRC(08ec5a33) SHA1(77389d07807a2073fdbc5eb901e90df7be281970) )
	ROM_LOAD16_BYTE( "rmnc05dy.p1", 0x000000, 0x080000, CRC(c9c3e6ae) SHA1(6e963ddb3183d8f8e8668a4e0c5222caa43eda05) )
	ROM_LOAD16_BYTE( "rmnc05s.p1", 0x000000, 0x080000, CRC(a589ebde) SHA1(3389275b7be1b413db79365b7af69395621e8f40) )
	ROM_LOAD16_BYTE( "rmnc05y.p1", 0x000000, 0x080000, CRC(e8937cd3) SHA1(d205e78d61f1b73f74b8980dab404d78df4b5919) )
	ROM_LOAD16_BYTE( "rmnc05z.p1", 0x000000, 0x080000, CRC(1d1d3709) SHA1(d8b5302cd078096b117a98106433c914abdd2245) )
ROM_END

ROM_START( m5rfymc06 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rmnc06s.p1", 0x000000, 0x080000, CRC(e5484915) SHA1(4f3fdbcd5a51d1f8f1c52590bbc20a02492446ee) )
	ROM_LOAD16_BYTE( "rmnc06.p2", 0x000001, 0x080000, CRC(b32b6828) SHA1(21a4a3ef187e3b7abe0c2d2e13d70e80e641eee4) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "rmnc06d.p1", 0x000000, 0x080000, CRC(b3b0bf0f) SHA1(2fd76e51b49b3170ccc8eeef204d1e98e379e9c2) )
	ROM_LOAD16_BYTE( "rmnc06dz.p1", 0x000000, 0x080000, CRC(e58d950c) SHA1(1a7ee76bb0086f539e96f86ae240c82dc065b27a) )
	ROM_LOAD16_BYTE( "rmnc06z.p1", 0x000000, 0x080000, CRC(1e899db6) SHA1(c0f93bde0b973e604f3fc84cb62664d03b8ab4a7) )
ROM_END


ROM_START( m5sheik )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "shek04ad.p1", 0x000000, 0x080000, CRC(4f645f4b) SHA1(d68edd7db7c48914f24ef33b2d62b6c333cfd243) )
	ROM_LOAD16_BYTE( "shek04.p2", 0x000001, 0x080000, CRC(99923f49) SHA1(7add80a0e971ee480f71bbb1c30e7a94cbe93b51) )
	ROM_LOAD16_BYTE( "shek04.p3", 0x100000, 0x080000, CRC(5c552146) SHA1(04ec7f4ac1962afaf57b20d30be0788531d37404) )
	ROM_LOAD16_BYTE( "shek04.p4", 0x100001, 0x080000, CRC(ae914300) SHA1(849aeac64eb423faea2461b1e05c57b6f388b620) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "shek04b.p1", 0x000000, 0x080000, CRC(5c6c30b4) SHA1(c616362b79b7e4847b2972af6cdcf7ea67ef60d6) )
	ROM_LOAD16_BYTE( "shek04bd.p1", 0x000000, 0x080000, CRC(0e768133) SHA1(94190315dad07a87586c86d710fe97e574eac453) )
	ROM_LOAD16_BYTE( "shek04d.p1", 0x000000, 0x080000, CRC(809f24d6) SHA1(8be8d570e7cb5ba2bfa2ba78dad05caa0a4ef6ae) )
	ROM_LOAD16_BYTE( "shek04dy.p1", 0x000000, 0x080000, CRC(190e3a5d) SHA1(6c0014ec6b68dd481930a0ba72ba48c6b62311a3) )
	ROM_LOAD16_BYTE( "shek04h.p1", 0x000000, 0x080000, CRC(8e0bd9aa) SHA1(d1801f2f424bf48c38e976a00d77259278b520b6) )
	ROM_LOAD16_BYTE( "shek04r.p1", 0x000000, 0x080000, CRC(e92d63eb) SHA1(891adfa1129b2a3fcbee6a9565618f994e01878d) )
	ROM_LOAD16_BYTE( "shek04s.p1", 0x000000, 0x080000, CRC(d2859551) SHA1(4806d3326bbc313da1bc9df45ef67020ea2f1a01) )
	ROM_LOAD16_BYTE( "shek04y.p1", 0x000000, 0x080000, CRC(4b148bda) SHA1(02d4f84b7225a1bdcc89fcec2cc701bea21e1ce1) )
ROM_END


ROM_START( m5silver )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "silv05ad.p1", 0x000000, 0x080000, CRC(1fb99113) SHA1(f38679052d428a120801c58791a690c4b9a9787f) )
	ROM_LOAD16_BYTE( "silv05.p2", 0x000001, 0x080000, CRC(a69d519f) SHA1(2e22dbed37729ca1fedb7f619c767e9de14bc152) )
	ROM_LOAD16_BYTE( "silv05.p3", 0x100000, 0x080000, CRC(eaf54d45) SHA1(16e416d3075921074873850edde4bc82ca2623ec) )
	ROM_LOAD16_BYTE( "silv05.p4", 0x100001, 0x080000, CRC(75b51281) SHA1(f8bf611b4712f4315bc060cbbc1d5515788e0a8d) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "silv05b.p1", 0x000000, 0x080000, CRC(c098b104) SHA1(d8bbe987380a3e46a43c21b921a268596f60bcb8) )
	ROM_LOAD16_BYTE( "silv05bd.p1", 0x000000, 0x080000, CRC(741e0bfe) SHA1(d00824550c821fc634f8c33eb6a1d590c8dfe5bd) )
	ROM_LOAD16_BYTE( "silv05d.p1", 0x000000, 0x080000, CRC(bcf6c2ba) SHA1(70a198b58bdbdb09bfcad88b414ff35231d54e50) )
	ROM_LOAD16_BYTE( "silv05dy.p1", 0x000000, 0x080000, CRC(992b655b) SHA1(4be9cbf6e95dd3fa4d1845ef017eac91070cc2f6) )
	ROM_LOAD16_BYTE( "silv05h.p1", 0x000000, 0x080000, CRC(04fad24a) SHA1(1e71b9a34a18951847d785fb5ca64462dae79d24) )
	ROM_LOAD16_BYTE( "silv05r.p1", 0x000000, 0x080000, CRC(4f81a1ec) SHA1(b2429f8e3c154369a6c390d4efe1cd6c5ae0fb62) )
	ROM_LOAD16_BYTE( "silv05s.p1", 0x000000, 0x080000, CRC(07cea32b) SHA1(054bb836f3a4b70202205d4e8e2b96e52bfc8494) )
	ROM_LOAD16_BYTE( "silv05y.p1", 0x000000, 0x080000, CRC(221304ca) SHA1(1554abed77aa7a5893f2e7bb3874530fc73213b3) )
ROM_END

ROM_START( m5silver06 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "silv06s.p1", 0x000000, 0x080000, CRC(d6e8defd) SHA1(82594f867dc62a5a0c7f24bfb831c13fbbd0f039) )
	ROM_LOAD16_BYTE( "silv06.p2", 0x000001, 0x080000, CRC(52346eee) SHA1(b6e1603694a06bf956d9c032a9ec34a56d59b9e5) )
	ROM_LOAD16_BYTE( "silv06.p3", 0x100000, 0x080000, CRC(eaf54d45) SHA1(16e416d3075921074873850edde4bc82ca2623ec) ) // == 05
	ROM_LOAD16_BYTE( "silv06.p4", 0x100001, 0x080000, CRC(75b51281) SHA1(f8bf611b4712f4315bc060cbbc1d5515788e0a8d) ) // == 05

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "silv06ad.p1", 0x000000, 0x080000, CRC(1c83328d) SHA1(3db07ae710dcefacf489f075452cea638e533648) )
	ROM_LOAD16_BYTE( "silv06b.p1", 0x000000, 0x080000, CRC(c04177c3) SHA1(1baeec7da7faafe3b6a249adb051dcecb4293f1d) )
	ROM_LOAD16_BYTE( "silv06bd.p1", 0x000000, 0x080000, CRC(01ab7909) SHA1(9032c0778330c4f557297fe4e8d1b9a5d8e39c46) )
	ROM_LOAD16_BYTE( "silv06d.p1", 0x000000, 0x080000, CRC(94c7ecee) SHA1(f6c3a54dc98457987fc8c1494db7158a10c1c1bb) )
	ROM_LOAD16_BYTE( "silv06dy.p1", 0x000000, 0x080000, CRC(3b907d85) SHA1(ffbfbeb790cd7edee6219aa377e52a5dee452d33) )
	ROM_LOAD16_BYTE( "silv06h.p1", 0x000000, 0x080000, CRC(782d55b3) SHA1(fa3fd52d7e58fbd15b535859d9abf4a1387c7494) )
	ROM_LOAD16_BYTE( "silv06r.p1", 0x000000, 0x080000, CRC(d30f59b2) SHA1(878b68c465a213e048b9d7545bf65917ffc11a4d) )
	ROM_LOAD16_BYTE( "silv06y.p1", 0x000000, 0x080000, CRC(ccb21d02) SHA1(c70be2c7ea3585d5a28dacf79294feafdf12596d) )
ROM_END

ROM_START( m5silver03 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "slvd03s.p1", 0x000000, 0x080000, CRC(d5875f82) SHA1(e564e1aed9e91a9c768f0f1c430abd0c2c30b3ae) )
	ROM_LOAD16_BYTE( "slvd03.p2", 0x000001, 0x080000, CRC(c7015d69) SHA1(347a2ec78f7bcef1f3eab8c054e696ac8be50e1c) )
	ROM_LOAD16_BYTE( "slvd03.p3", 0x100000, 0x080000, CRC(1fa52e9a) SHA1(4d38abeee540e79abd7a09d7a8ffa5762c96acc4) )
	ROM_LOAD16_BYTE( "slvd03.p4", 0x100001, 0x080000, CRC(ba4c1994) SHA1(b1c0c0d8997976fafde4e6e7e2967bc90e264590) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "slvd03ad.p1", 0x000000, 0x080000, CRC(1f355253) SHA1(7d8e6fe968e252848903ae6d75052cfcce53ba52) )
	ROM_LOAD16_BYTE( "slvd03b.p1", 0x000000, 0x080000, CRC(c275d848) SHA1(8dbf4244507de3a25eb80c1102f654ee0b20ca86) )
	ROM_LOAD16_BYTE( "slvd03bd.p1", 0x000000, 0x080000, CRC(f11cb0bc) SHA1(f6a88b5f91aa20155a992d6704848eaf4e5dc2eb) )
	ROM_LOAD16_BYTE( "slvd03d.p1", 0x000000, 0x080000, CRC(ff53ff2c) SHA1(8b11b475365f122d6687d294d24cd308bda41373) )
	ROM_LOAD16_BYTE( "slvd03dy.p1", 0x000000, 0x080000, CRC(d27180c6) SHA1(d51a9d7485a4d802fbe55aafb4a9f28198831368) )
	ROM_LOAD16_BYTE( "slvd03h.p1", 0x000000, 0x080000, CRC(39969460) SHA1(94a3c5566a1f7fc2a632176c1071d957d7f57bc8) )
	ROM_LOAD16_BYTE( "slvd03r.p1", 0x000000, 0x080000, CRC(67a5dc00) SHA1(b9be078e12a319a181ddd007ab7b3422b35c5cd5) )
	ROM_LOAD16_BYTE( "slvd03y.p1", 0x000000, 0x080000, CRC(f8a52068) SHA1(d59c12307b0eed20ea06e5288d1682b49bcd83b3) )
ROM_END


ROM_START( m5skulcl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "skul1_0.p1", 0x000000, 0x080000, CRC(1975c377) SHA1(b130b8e977a7f59de240e5fe43c74bbf3915131f) )
	ROM_LOAD16_BYTE( "skul1_0.p2", 0x000001, 0x080000, CRC(befa4888) SHA1(6f1edfe7c49c16e6cd5486999aba5c2f4a165690) )
	ROM_LOAD16_BYTE( "skul1_0.p3", 0x100000, 0x080000, CRC(3d8ca5d1) SHA1(0f2271a8518b7b39a94a0027066d49034bc3a658) )
	ROM_LOAD16_BYTE( "skul1_0.p4", 0x100001, 0x080000, CRC(f86c5a73) SHA1(36dc61e7e56c0d778294a967921e09fc0364cc76) )
ROM_END

ROM_START( m5skulcl20 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "skul2_0.p1", 0x000000, 0x080000, CRC(1b9c8153) SHA1(8ed78aba934309fa6e1aecf8c5ce5e2186dea406) )
	ROM_LOAD16_BYTE( "skul2_0.p2", 0x000001, 0x080000, CRC(28d599b6) SHA1(073293ff314fbcf94acf5b160257968ab93afccc) )
	/* 3+4 */
ROM_END

ROM_START( m5skulcl23 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "skul2_3.p1", 0x000000, 0x080000, CRC(d852cec9) SHA1(cc9c1af8e2fbedd12f7908cde70bfa620d763e73) )
	ROM_LOAD16_BYTE( "skul2_3.p2", 0x000001, 0x080000, CRC(a4c949de) SHA1(3efc6cc2557f1ea83785ce6ca59b8aafe3efe716) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "skul2_3d.p1", 0x000000, 0x080000, CRC(653e17d7) SHA1(dd1abcc4ba1250c433b6cb6fc7a3d70580dce611) )
ROM_END



ROM_START( m5sondr )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "s1tr01d.p1", 0x000000, 0x080000, CRC(ea52b00b) SHA1(b62afb0973fa4f86ad193b288516eedec7041145) )
	ROM_LOAD16_BYTE( "s1tr01.p2", 0x000001, 0x080000, CRC(e8893c62) SHA1(765f814fe3180efd36451a4eebb9f08c5b5b142b) )
	ROM_LOAD16_BYTE( "s1tr01.p3", 0x100000, 0x080000, CRC(868d1a96) SHA1(0ebca6a12b1ae912d836c2ee86b555dbdf6fe16f) )
	ROM_LOAD16_BYTE( "s1tr01.p4", 0x100001, 0x080000, CRC(b4a37232) SHA1(c85a185786ad5e0a0a2fb7c525f9be8c0c376d3e) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "s1tr01dy.p1", 0x000000, 0x080000, CRC(fde77790) SHA1(3ce600af7eea438ba6ec0511b2fe79f8ad51cb37) )
	ROM_LOAD16_BYTE( "s1tr01k.p1", 0x000000, 0x080000, CRC(60ed1547) SHA1(4dc35ea493ef574b0bade6db84d852831f702393) )
	ROM_LOAD16_BYTE( "s1tr01s.p1", 0x000000, 0x080000, CRC(73e4c5b2) SHA1(f4296a8811d3ed396c29c69f4d1a5a2992745465) )
	ROM_LOAD16_BYTE( "s1tr01y.p1", 0x000000, 0x080000, CRC(64510229) SHA1(e87248eaf08ef9617ae88ae8eca4c39428c22b20) )
	ROM_LOAD16_BYTE( "sdtr01.p2", 0x000000, 0x080000, CRC(7da34f4a) SHA1(95ebb5d89c3ca9268f9507e6fb9bc0dc1f550967) )
	//ROM_LOAD16_BYTE( "sdtr01.p4", 0x000000, 0x080000, CRC(b4a37232) SHA1(c85a185786ad5e0a0a2fb7c525f9be8c0c376d3e) )
	ROM_LOAD16_BYTE( "sdtr01d.p1", 0x000000, 0x080000, CRC(f9d259cb) SHA1(6f4bef0db425291afaea66d7ad629c2e1f645a63) )
	ROM_LOAD16_BYTE( "sdtr01dy.p1", 0x000000, 0x080000, CRC(61d4cb2d) SHA1(f63594141f2d8692cf6ac1acbeb8faef954802c1) )
	ROM_LOAD16_BYTE( "sdtr01s.p1", 0x000000, 0x080000, CRC(2b8dc04d) SHA1(dfe7c7a4a14cbb26da691c77b240999a87912361) )
	ROM_LOAD16_BYTE( "sdtr01y.p1", 0x000000, 0x080000, CRC(b38b52ab) SHA1(58fabe1778f9da4fad952f29e97190fa88bcbca7) )


	ROM_LOAD16_BYTE( "sonofdracula25.bin", 0x000000, 0x080000, CRC(0f3a5421) SHA1(990f4557775942a806c36fdcd9e4db9b1e62e702) ) // corrupt or not MPU5 (encrypted?)
ROM_END

ROM_START( m5sondr05 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sono05s.p1", 0x000000, 0x080000, CRC(1f4d2bd4) SHA1(bbda8cac21e472cd406f945f78ee2e43d334bec5) )
	ROM_LOAD16_BYTE( "sono05.p2", 0x000001, 0x080000, CRC(8ffae24a) SHA1(0a20ea91fb8241f9da665cc9b2bc86a66b153b8c) )
	ROM_LOAD16_BYTE( "sono05.p3", 0x100000, 0x080000, CRC(868d1a96) SHA1(0ebca6a12b1ae912d836c2ee86b555dbdf6fe16f) ) // == 01
	ROM_LOAD16_BYTE( "sono05.p4", 0x100001, 0x080000, CRC(b4a37232) SHA1(c85a185786ad5e0a0a2fb7c525f9be8c0c376d3e) ) // == 01

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "sono05ad.p1", 0x000000, 0x080000, CRC(4e162d7f) SHA1(9533f8ac7ca385b9b5aeaa0767628e16a6ee8e5d) )
	ROM_LOAD16_BYTE( "sono05b.p1", 0x000000, 0x080000, CRC(d62b4d88) SHA1(6760752341091c4aeb5b908c89de7c868af4e455) )
	ROM_LOAD16_BYTE( "sono05bd.p1", 0x000000, 0x080000, CRC(fcadc29b) SHA1(608803bc1295f7cfc1f67758d68cda9e0d450e3a) )
	ROM_LOAD16_BYTE( "sono05d.p1", 0x000000, 0x080000, CRC(35cba4c7) SHA1(6ae99a3991066db6e40a5b104f872494d306141d) )
	ROM_LOAD16_BYTE( "sono05dy.p1", 0x000000, 0x080000, CRC(ba776295) SHA1(4baccf14b7297d747ab4e6115c5a6656059c11e7) )
	ROM_LOAD16_BYTE( "sono05h.p1", 0x000000, 0x080000, CRC(0fcfef69) SHA1(fa93ebf1deb0616e25e511dddd9e170d547d9ef7) )
	ROM_LOAD16_BYTE( "sono05r.p1", 0x000000, 0x080000, CRC(d8291bc1) SHA1(846f887d4726927393895096222429bc77600a19) )
	ROM_LOAD16_BYTE( "sono05y.p1", 0x000000, 0x080000, CRC(90f1ed86) SHA1(d383206ff62a8f699569f1310daee48fa5b50647) )
ROM_END



ROM_START( m5sondra )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "s_dr_olh.p1", 0x000000, 0x080000, CRC(21d4e08d) SHA1(f2d2563e045b104a865e4c5baa41145a55ca00c3) )
	ROM_LOAD16_BYTE( "s_dr_olh.p2", 0x000001, 0x080000, CRC(e07796af) SHA1(04b60456df38bb895182f36fb13689d92d209f1d) )
	ROM_LOAD16_BYTE( "s_dr_olh.p3", 0x100000, 0x080000, CRC(f521bd36) SHA1(07f5d0af8dc6b06fb69ae604a94946458052ad9b) )
	ROM_LOAD16_BYTE( "s_dr_olh.p4", 0x100001, 0x080000, CRC(4d886a03) SHA1(0b6c7e99b1ad47a7834c23f14d1d95a51fbd796a) )
ROM_END

ROM_START( m5speccl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "spe_sjs1.3_1", 0x000000, 0x080000, CRC(c3935432) SHA1(76c672b0f4387f304d89d92302e9347d7d545427) )
	ROM_LOAD16_BYTE( "spe_sjs1.3_2", 0x000001, 0x080000, CRC(7025423a) SHA1(f1a76e98878841dff540d85625d6155f772c16d6) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "spe_86_1.3_1", 0x000000, 0x080000, CRC(46eac92e) SHA1(6307d92bd67b13f7d1ebc19d1083b433f49171a6) )
	ROM_LOAD16_BYTE( "spe_86_1.3d1", 0x000000, 0x080000, CRC(74d4f52f) SHA1(418d30181acf9e5caff1c63e5aa0573108a0bb29) )
	ROM_LOAD16_BYTE( "spe_ga1.3_1", 0x000000, 0x080000, CRC(c6daaf53) SHA1(588cfa286ff535a00d2e388f6e5276d80f2ebc05) )
	ROM_LOAD16_BYTE( "spe_ge1.3_1", 0x000000, 0x080000, CRC(81ba7b58) SHA1(150c37526b3c4ddcfb2dde839f98065abe06e4bf) )
	ROM_LOAD16_BYTE( "spe_gg1.3_1", 0x000000, 0x080000, CRC(c15924f3) SHA1(20b27d67eff4038ae7002b1db6847696a1856895) )
	ROM_LOAD16_BYTE( "spe_gj1.3_1", 0x000000, 0x080000, CRC(db469989) SHA1(8e9a3d6f1302da84f12605fa31335ec51dcf8101) )
	ROM_LOAD16_BYTE( "spe_sjs1.3d1", 0x000000, 0x080000, CRC(c09fb1a8) SHA1(2f873974c4ab4a010325b21631b022c7fe01bc97) )
ROM_END

ROM_START( m5spddmn )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "spd_sjs1.0_1", 0x000000, 0x080000, CRC(37a51ce6) SHA1(d8885d010bde277d085f48441b391e18aec6a9e6) )
	ROM_LOAD16_BYTE( "spd_sjs1.0_2", 0x000001, 0x080000, CRC(ca270e48) SHA1(5816501ed1e9420cdc2c67742e1e40649debb815) )
	ROM_LOAD16_BYTE( "spd_sjs1.0_3", 0x100000, 0x080000, CRC(8dfe1036) SHA1(7d962003172d224cea6c3b3a1679f9bfedbd92c5) )
	ROM_LOAD16_BYTE( "spd_sjs1.0_4", 0x100001, 0x080000, CRC(80bcb46a) SHA1(1d093f7de603fc477bff044aa3a2a99554a78f4a) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "spd_sja1.0_1", 0x000000, 0x080000, CRC(16f40a65) SHA1(7f72eebe2dca145e3bd9b3a81dd5db76ff67e99b) )
	ROM_LOAD16_BYTE( "spd_sjh1.0d1", 0x000000, 0x080000, CRC(36d57dfa) SHA1(e57f45b5570de08e1fd2900d57e0dc2ac442ddb4) )
	ROM_LOAD16_BYTE( "spd_sjk1.0_1", 0x000000, 0x080000, CRC(d5c8fe54) SHA1(0446b1590bbdb119ccd734f32ddeba21ee693c32) )
	ROM_LOAD16_BYTE( "spd_sjl1.0d1", 0x000000, 0x080000, CRC(7cc2601c) SHA1(cc1be289637be047a558d8826ee04ece2f768654) )
	ROM_LOAD16_BYTE( "spd_sjs1.0d1", 0x000000, 0x080000, CRC(d4b89f48) SHA1(7cb6ec1467f573863bc2e68e2f6c323e63e128c2) )
ROM_END

ROM_START( m5spicer )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tsir08ad.p1", 0x000000, 0x080000, CRC(6b8fbb61) SHA1(14ffd0602f61c7e832c5e36ac076472c18514789) )
	ROM_LOAD16_BYTE( "tsir08.p2", 0x000001, 0x080000, CRC(1cbe4b35) SHA1(0b3e044241c51da82921fc25ae945e83e177fdae) )
	ROM_LOAD16_BYTE( "tsir08.p3", 0x100000, 0x080000, CRC(2465fccc) SHA1(30ed14d9317560456eb9ecf4b7b3c839dcf05da6) )
	ROM_LOAD16_BYTE( "tsir08.p4", 0x100001, 0x080000, CRC(a368587c) SHA1(398ea3c308c823429b79d23e9ee6e3b4ca55cdd9) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "tsir08b.p1", 0x000000, 0x080000, CRC(7939acb2) SHA1(35bf98ab8cf7237c1f4dc56e248e4cbbe121a839) )
	ROM_LOAD16_BYTE( "tsir08bd.p1", 0x000000, 0x080000, CRC(0256a97b) SHA1(d0ab0c20097b7912dc8b14f3abe4a0fe6f06bbe6) )
	ROM_LOAD16_BYTE( "tsir08d.p1", 0x000000, 0x080000, CRC(f290d7f8) SHA1(3059fded06652a6bc6b2b063d771fd2fa86c9855) )
	ROM_LOAD16_BYTE( "tsir08dy.p1", 0x000000, 0x080000, CRC(2a6b8a03) SHA1(fc583d9b08ad89e32c3d260bfb891da86e6868f1) )
	ROM_LOAD16_BYTE( "tsir08h.p1", 0x000000, 0x080000, CRC(fabac5f0) SHA1(99e6bf0107aaede4b8331ea264171b9965366f43) )
	ROM_LOAD16_BYTE( "tsir08r.p1", 0x000000, 0x080000, CRC(74493000) SHA1(1c52ba87b13d3e67588c1c0f14ffa718d7ffd528) )
	ROM_LOAD16_BYTE( "tsir08s.p1", 0x000000, 0x080000, CRC(89ffd231) SHA1(f9a56ca145a022730debdbb05e230a666c0cd005) )
	ROM_LOAD16_BYTE( "tsir08y.p1", 0x000000, 0x080000, CRC(51048fca) SHA1(3edc45d1dd0db88a93b4f35c6864b5c74b2caea5) )
ROM_END

ROM_START( m5spicer06 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tsir06s.p1", 0x000000, 0x080000, CRC(3189d73d) SHA1(1a149334c5798f2956b98382894c2a0f1de2cbee) )
	ROM_LOAD16_BYTE( "tsir06.p2", 0x000001, 0x080000, CRC(418cbe74) SHA1(7013a838860bdb7b2f9486bcd31241002f6af934) )
	/* 3+4 */
ROM_END


ROM_START( m5spiker )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bike03ad.p1", 0x000000, 0x080000, CRC(a95a5eed) SHA1(522aa75fef5a333a8d2a3971a5ea57ae8d41e29b) )
	ROM_LOAD16_BYTE( "bike03.p2", 0x000001, 0x080000, CRC(d4efda2e) SHA1(00ec6a0ea0e24719bb5a561127ff485315f27475) )
	ROM_LOAD16_BYTE( "bike03.p3", 0x100000, 0x080000, CRC(cbd103db) SHA1(a4b49e74a5a37c8779d9f6e960c7d904fc9b4a9a) )
	ROM_LOAD16_BYTE( "bike03.p4", 0x100001, 0x080000, CRC(0d5a7af8) SHA1(14dde9f4f2437b40ad5756931bb4428b571acc2c) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "bike03b.p1", 0x000000, 0x080000, CRC(a700838b) SHA1(ea02186057961e9a970f7d2ae8c5c013feae12b0) )
	ROM_LOAD16_BYTE( "bike03bd.p1", 0x000000, 0x080000, CRC(ffe430f7) SHA1(955edf1fcb7b890e10bda0d087a3a46f90e8c449) )
	ROM_LOAD16_BYTE( "bike03d.p1", 0x000000, 0x080000, CRC(ace08085) SHA1(bef0ddd6ae7e3ff1866edbfba3e3350a788a7612) )
	ROM_LOAD16_BYTE( "bike03dy.p1", 0x000000, 0x080000, CRC(7d7da247) SHA1(c1bc33295711032fe933e3b1ba0c9c15f2813705) )
	ROM_LOAD16_BYTE( "bike03h.p1", 0x000000, 0x080000, CRC(92276ee8) SHA1(ddd28171847d20e658e8ad85823bf0cf22f547a6) )
	ROM_LOAD16_BYTE( "bike03r.p1", 0x000000, 0x080000, CRC(b76bbc4e) SHA1(0c9d7296547dc870145fd21ffd79ce478d8eaa72) )
	ROM_LOAD16_BYTE( "bike03s.p1", 0x000000, 0x080000, CRC(f40433f9) SHA1(8c4f41b0a1c0ef0f1ec7ae479724e2144994be64) )
	ROM_LOAD16_BYTE( "bike03y.p1", 0x000000, 0x080000, CRC(2599113b) SHA1(5dab7a12c7823cbc3ffcbc51ec09394942107a4c) )
ROM_END

ROM_START( m5spiker02 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sbtr02s.p1", 0x000000, 0x080000, CRC(83564725) SHA1(1808294d25426dc407572692531f81cbb3d20c83) )
	ROM_LOAD16_BYTE( "sbtr02.p2", 0x000001, 0x080000, CRC(00ada2e2) SHA1(d6979b9d29d3a5da04d9d9910ca2f2af91f5531e) )
	ROM_LOAD16_BYTE( "sbtr02.p3", 0x100000, 0x080000, CRC(cbd103db) SHA1(a4b49e74a5a37c8779d9f6e960c7d904fc9b4a9a) ) // == 03
	ROM_LOAD16_BYTE( "sbtr02.p4", 0x100001, 0x080000, CRC(0d5a7af8) SHA1(14dde9f4f2437b40ad5756931bb4428b571acc2c) ) // == 03

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "sbtr02ad.p1", 0x000000, 0x080000, CRC(cad3e3bd) SHA1(f6076754bcd560aaf09730bd23fb3c94138579c0) )
	ROM_LOAD16_BYTE( "sbtr02b.p1", 0x000000, 0x080000, CRC(6a3bfc70) SHA1(202d453cd3e44337168e9e023f2d5daefcbe2c6c) )
	ROM_LOAD16_BYTE( "sbtr02bd.p1", 0x000000, 0x080000, CRC(48289f9a) SHA1(8533bf4027c9cbd529d6515bfe0f2f0479641c2c) )
	ROM_LOAD16_BYTE( "sbtr02d.p1", 0x000000, 0x080000, CRC(a14524cf) SHA1(0b3ed43f5de4a44015ea5a345aef43a823cb46a9) )
	ROM_LOAD16_BYTE( "sbtr02dy.p1", 0x000000, 0x080000, CRC(d87090f7) SHA1(842eb81c83793f74e9075dfeae8033403913d7c5) )
	ROM_LOAD16_BYTE( "sbtr02k.p1", 0x000000, 0x080000, CRC(30fe78a8) SHA1(e36c51340c5044fb1a4e63a0f98b48eec598417e) )
	ROM_LOAD16_BYTE( "sbtr02y.p1", 0x000000, 0x080000, CRC(fa63f31d) SHA1(7b1ec05a24a64d83e04738baf1d408d84a08dc06) )
ROM_END

ROM_START( m5spikera )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sbtro1.bin", 0x000000, 0x080000, CRC(45c80d46) SHA1(ee5fc418fad5b3a27b025dac38cfa93619a4c058) )
	ROM_LOAD16_BYTE( "sbtro2.bin", 0x000001, 0x080000, CRC(bf8727c3) SHA1(a647f3ff8291b96acb194f4d362e216efdcf9312) )
	ROM_LOAD16_BYTE( "sbtro3.bin", 0x100000, 0x080000, CRC(cbd103db) SHA1(a4b49e74a5a37c8779d9f6e960c7d904fc9b4a9a) ) // == 03
	ROM_LOAD16_BYTE( "sbtro4.bin", 0x100001, 0x080000, CRC(0d5a7af8) SHA1(14dde9f4f2437b40ad5756931bb4428b571acc2c) ) // == 03
ROM_END


ROM_START( m5squids )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sqid04s.p1", 0x000000, 0x080000, CRC(79a6658c) SHA1(258605c3abb70c2b326fc2ef51bc43f720363e70) )
	ROM_LOAD16_BYTE( "sqid04.p2", 0x000001, 0x080000, CRC(a9b674c5) SHA1(ad0fa8c9fb3b7eb3bb98a8a68da544b6d791a8fd) )
	ROM_LOAD16_BYTE( "sqid04.p3", 0x100000, 0x080000, CRC(2808fd4a) SHA1(1f75987b42f83b54dd2eb376cc9ce0fccfbb693c) )
	ROM_LOAD16_BYTE( "sqid04.p4", 0x100001, 0x080000, CRC(5983db8c) SHA1(1cf396ea35a6b947fd7fd827fc79e8bf2ef95811) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "sqid04b.p1", 0x000000, 0x080000, CRC(144df95b) SHA1(ccf92fe31bae1496755e9929c3045e5f2b9c2ae5) )
	ROM_LOAD16_BYTE( "sqid04bd.p1", 0x000000, 0x080000, CRC(02dedde6) SHA1(3fcfb73d70d9b7e77370a2b787b5ba210f56d1bc) )
	ROM_LOAD16_BYTE( "sqid04d.p1", 0x000000, 0x080000, CRC(1597d856) SHA1(9ce017a58aec6dd87108125b02446b1eb4303e32) )
	ROM_LOAD16_BYTE( "sqid04dy.p1", 0x000000, 0x080000, CRC(0d40179c) SHA1(dcaf5427c5e29ddf2eb7ceed7ccd0250bd8d8004) )
	ROM_LOAD16_BYTE( "sqid04h.p1", 0x000000, 0x080000, CRC(40e9a2f2) SHA1(78dfabf91adcbe8fb45103a06bd9683807776b18) )
	ROM_LOAD16_BYTE( "sqid04r.p1", 0x000000, 0x080000, CRC(b204b129) SHA1(7afd47c64c75e33c2247c6ea5afd03f94a681b62) )
	ROM_LOAD16_BYTE( "sqid04ad.p1", 0x000000, 0x080000, CRC(53d58b54) SHA1(6b68e23e2e6d7ec2bda08e6fae16500536a31031) )
	ROM_LOAD16_BYTE( "sqid04y.p1", 0x000000, 0x080000, CRC(6171aa46) SHA1(d4b570b97caec10d39ad813653fa6b2b4c96e99f) )
ROM_END

ROM_START( m5squids04a )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sqin04.p1", 0x000000, 0x080000, CRC(8682df90) SHA1(d11b6cad477f7ae0545f73c871e1f3595d38564e) )
	ROM_LOAD16_BYTE( "sqin04.p2", 0x000001, 0x080000, CRC(8fa6544e) SHA1(1299f72ed30ad6c5bde99c566e36845cbe6e5835) )
	ROM_LOAD16_BYTE( "sqin04.p3", 0x100000, 0x080000, CRC(ef3b411e) SHA1(eb38c441aebc82451675d9e52a95eaded484ed4f) )
	ROM_LOAD16_BYTE( "sqin04.p4", 0x100001, 0x080000, CRC(4f5d9fe1) SHA1(5cf60c0702b5d01f734ae112e6b57f1397647fe1) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "sqin04a.p1", 0x000000, 0x080000, CRC(5a82309b) SHA1(6502d59710af150084438769a5f2abe392eac3cf) )
	ROM_LOAD16_BYTE( "sqin04b.p1", 0x000000, 0x080000, CRC(70a2e02f) SHA1(d5371a610e0b1e70d4ef192c7e67cc55cbfaabf6) )
	ROM_LOAD16_BYTE( "sqin04d.p1", 0x000000, 0x080000, CRC(2f4d036e) SHA1(33a33f81a4c7a8b699315de52c9292790c250420) )
	ROM_LOAD16_BYTE( "sqin04dy.p1", 0x000000, 0x080000, CRC(23c3d34d) SHA1(78cd0f046a47f82bfd2f7b648ed1341a1efa2354) )
	ROM_LOAD16_BYTE( "sqin04h.p1", 0x000000, 0x080000, CRC(b6f84795) SHA1(c1b131f4e39ab57b3582cd65cef78be380cdda08) )
	ROM_LOAD16_BYTE( "sqin04k.p1", 0x000000, 0x080000, CRC(cd0f0878) SHA1(144123581afed313a61f920ab1148b604f67510f) )
	ROM_LOAD16_BYTE( "sqin04r.p1", 0x000000, 0x080000, CRC(5bbb6a6c) SHA1(1cd6772354517b4b193225cb5c099e2e3ba91a34) )
ROM_END

ROM_START( m5squids05 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sqid05s.p1", 0x000000, 0x080000, CRC(01e41c6b) SHA1(ff8aef917292fab4b3cc95354bc3c05548dbfd67) )
	ROM_LOAD16_BYTE( "sqid05.p2", 0x000001, 0x080000, CRC(3a5c50cc) SHA1(082b2918fbb0718d567bc4ed1f7f03034138f5c1) )
	ROM_LOAD16_BYTE( "sqid05.p3", 0x100000, 0x080000, CRC(2808fd4a) SHA1(1f75987b42f83b54dd2eb376cc9ce0fccfbb693c) ) // == 04
	ROM_LOAD16_BYTE( "sqid05.p4", 0x100001, 0x080000, CRC(5983db8c) SHA1(1cf396ea35a6b947fd7fd827fc79e8bf2ef95811) ) // == 04

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "sqid05ad.p1", 0x000000, 0x080000, CRC(15051db0) SHA1(b4cfb41aa9f218144511e5e54630c67d8fd744c5) )
	ROM_LOAD16_BYTE( "sqid05b.p1", 0x000000, 0x080000, CRC(6ea2eb9e) SHA1(28a1c92688daf90a8363798d797035b661282189) )
	ROM_LOAD16_BYTE( "sqid05bd.p1", 0x000000, 0x080000, CRC(ccd892cb) SHA1(eb6a243e5caa182597180b9136bf80641b34a92c) )
	ROM_LOAD16_BYTE( "sqid05d.p1", 0x000000, 0x080000, CRC(efc8a569) SHA1(3ff21cad27c7fd9e9d0427168040edfe7bfc10f9) )
	ROM_LOAD16_BYTE( "sqid05dy.p1", 0x000000, 0x080000, CRC(bd2d6a22) SHA1(154dccbf7475c85eaf0dd3bab0c9d0f2cafefcc3) )
	ROM_LOAD16_BYTE( "sqid05h.p1", 0x000000, 0x080000, CRC(9c7840d3) SHA1(865f6130746ae72f558644029f168607bef08c6b) )
	ROM_LOAD16_BYTE( "sqid05r.p1", 0x000000, 0x080000, CRC(c83f5627) SHA1(5d36d5ac3f23a993ad418afa20a83ae9447b9ceb) )
	ROM_LOAD16_BYTE( "sqid05y.p1", 0x000000, 0x080000, CRC(0a9253c1) SHA1(01171c1b1ae79a973553e3e897e038fcb7748c10) )
ROM_END

ROM_START( m5squids06 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sqin06s.p1", 0x000000, 0x080000, CRC(3848674c) SHA1(6da13e4f68254f6061492e7464c8d22791b9e9fc) )
	ROM_LOAD16_BYTE( "sqin06.p2", 0x000001, 0x080000, CRC(2b21434a) SHA1(fa8f35bc56d0bdc19680d913e4cc720f73cbca57) )
	ROM_LOAD16_BYTE( "sqin06.p3", 0x100000, 0x080000, CRC(ef3b411e) SHA1(eb38c441aebc82451675d9e52a95eaded484ed4f) ) // == 04a
	ROM_LOAD16_BYTE( "sqin06.p4", 0x100001, 0x080000, CRC(4f5d9fe1) SHA1(5cf60c0702b5d01f734ae112e6b57f1397647fe1) ) // == 04a

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "sqin06ad.p1", 0x000000, 0x080000, CRC(ac1f33e5) SHA1(08db8d6da76befe22e730294b1c9b7a2a4b1ce7a) )
	ROM_LOAD16_BYTE( "sqin06b.p1", 0x000000, 0x080000, CRC(44fe0cf0) SHA1(7ebc4399e31f874c47593face44c80919d2a76d2) )
	ROM_LOAD16_BYTE( "sqin06bd.p1", 0x000000, 0x080000, CRC(eb374da9) SHA1(028eec7ad58647a6294878b95839f3174544d866) )
	ROM_LOAD16_BYTE( "sqin06d.p1", 0x000000, 0x080000, CRC(97812615) SHA1(f168727818833c99c6b88efc0f8147518421203f) )
	ROM_LOAD16_BYTE( "sqin06dy.p1", 0x000000, 0x080000, CRC(7f07e409) SHA1(6279417c44bff598b9ba4023545c54d648d5cfb2) )
	ROM_LOAD16_BYTE( "sqin06h.p1", 0x000000, 0x080000, CRC(48ecf8dd) SHA1(0bf743302e52eb9b22c11eef247c5f8ee450183b) )
	ROM_LOAD16_BYTE( "sqin06r.p1", 0x000000, 0x080000, CRC(ac5d79c9) SHA1(98805372e5ffa6bbafa0a18a8f7efbcec3817c0b) )
	ROM_LOAD16_BYTE( "sqin06y.p1", 0x000000, 0x080000, CRC(34e8a047) SHA1(855aeccf82d62e7a43c02d077b5308d2a4809299) )
ROM_END


ROM_START( m5stars ) // ssss filenames
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ssssjb_1.3_1", 0x000000, 0x080000, CRC(49518254) SHA1(db7dadb54adba0171489cc9388c6f8ad89bcf752) )
	ROM_LOAD16_BYTE( "ssssjb_1.3_2", 0x000001, 0x080000, CRC(78b4423e) SHA1(50e8f355db91078d50c53abe98bc7c4cf5625064) )
	ROM_LOAD16_BYTE( "ssssjb_1.3_3", 0x100000, 0x080000, CRC(867fbcc7) SHA1(ee8091ee5caee5b5920f3e5ad85b4cdb7a6fd785) )
	ROM_LOAD16_BYTE( "ssssjb_1.3_4", 0x100001, 0x080000, CRC(04ee9dd4) SHA1(49ec743bac7eb8aedce6a908745535ecad88fcfc) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "ssssja_1.3_1", 0x000000, 0x080000, CRC(935227f9) SHA1(7b1f52d9ba55455d030f127ddff3ef07ad0206d9) )
	ROM_LOAD16_BYTE( "ssssjad1.3_1", 0x000000, 0x080000, CRC(0d01691d) SHA1(a6bed055c94949147d0736a5045301bc46ba1665) )
	ROM_LOAD16_BYTE( "ssssjbd1.3_1", 0x000000, 0x080000, CRC(d702ccb0) SHA1(3a0dfa0a4b8f2ab077694371fa29df61117864ff) )
	ROM_LOAD16_BYTE( "ssssjbg1.3_1", 0x000000, 0x080000, CRC(3a2dbf8a) SHA1(07f23adcb3799cd9056bd013bf657cb81d6fec60) )
	ROM_LOAD16_BYTE( "ssssjbt1.3_1", 0x000000, 0x080000, CRC(135e834f) SHA1(181dc93bf285d2699aed45fecb2e7a0dbac5a299) )
ROM_END

ROM_START( m5stars13a ) // why has this got alt 3+4 roms, but ssts filenames closer to the other sets?
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sstsjbd1.3_1", 0x000000, 0x080000, CRC(6b1d8456) SHA1(766a5de930bf50b9e0e595c5cd5b242bd71987df) )
	ROM_LOAD16_BYTE( "sstsjbd1.3_2", 0x000001, 0x080000, CRC(361b8185) SHA1(7b6b9272c320714373af2778343160cfc276588f) )
	ROM_LOAD16_BYTE( "sstsjb_1.3_3", 0x100000, 0x080000, CRC(161c6cd2) SHA1(b9e86b041c68221e2c420992222d5b4959dae73d) )
	ROM_LOAD16_BYTE( "sstsjb_1.3_4", 0x100001, 0x080000, CRC(350faf02) SHA1(1a52a66fa533f136630ae25a6d7f604db443b071) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "sstsjb_1.3_1", 0x000000, 0x080000, CRC(bbef5d7c) SHA1(86ab7d247e9d9fef9eaf66fedc8becf2d6e48fac) )
ROM_END

ROM_START( m5stars26 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sstsjb_2.6_1", 0x000000, 0x080000, CRC(0336c2de) SHA1(b6f7a4f470743d09c80207aeaa9830aec1ce7fd2) )
	ROM_LOAD16_BYTE( "sstsjb_2.6_2", 0x000001, 0x080000, CRC(c00ebd69) SHA1(7d52d3ee09cb866186e1872987c63c5d5986c355) )
	ROM_LOAD16_BYTE( "sstsjb_2.6_3", 0x100000, 0x080000, CRC(867fbcc7) SHA1(ee8091ee5caee5b5920f3e5ad85b4cdb7a6fd785) )
	ROM_LOAD16_BYTE( "sstsjb_2.6_4", 0x100001, 0x080000, CRC(04ee9dd4) SHA1(49ec743bac7eb8aedce6a908745535ecad88fcfc) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "sstsja_2.6_1", 0x000000, 0x080000, CRC(956a9648) SHA1(d21263f09a51523cad8b02d913ea3762dbd8df98) )
	ROM_LOAD16_BYTE( "sstsjbd2.6_1", 0x000000, 0x080000, CRC(e3cf3384) SHA1(c35b38dfbe2dc47b2ec76d047656c9fd64f09d3c) )
	ROM_LOAD16_BYTE( "sstsjbt2.6_1", 0x000000, 0x080000, CRC(af1a352c) SHA1(200ab5a8921b0c99afdf2df58ffc98569afa8e6b) )
	ROM_LOAD16_BYTE( "sstsjbg2.6_1", 0x000000, 0x080000, CRC(2b612733) SHA1(f8bb2019545b15f9abf6c4ede71f82ccd64ce493) )
	ROM_LOAD16_BYTE( "sstsjad2.6_1", 0x000000, 0x080000, CRC(75936712) SHA1(e5c1fd167be6490074105029ede4a60bf362bbce) )
ROM_END

ROM_START( m5stars25a ) // sstw filenames
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sstwjb_2.5_1", 0x000000, 0x080000, CRC(6f2d65a2) SHA1(b1c8b28496e1ad3f53cb23f5dd825c47cf4f2d59) )
	ROM_LOAD16_BYTE( "sstwjb_2.5_2", 0x000001, 0x080000, CRC(b5399cc8) SHA1(ad3b256d2b40b070b77639fd7a636ad2e7f212ff) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "sstwja_2.5_1", 0x000000, 0x080000, CRC(20bfa3ea) SHA1(3f1e76788912d9bc0f1c5b9bf728c87970d4862d) )
	ROM_LOAD16_BYTE( "sstwjad2.5_1", 0x000000, 0x080000, CRC(7108e9d0) SHA1(e080ea274ff0a3ced1c0047f59c5be7a1b5a0e60) )
	ROM_LOAD16_BYTE( "sstwjbd2.5_1", 0x000000, 0x080000, CRC(3e9a2f98) SHA1(59ac08784cff6b12559d7eedb0cf25a02bc675bd) )
ROM_END

ROM_START( m5stars25 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sstsjb_2.5_1", 0x000000, 0x080000, CRC(e995cf70) SHA1(b742fc741d11a5f0c6f42b22d78a73a10b9908bf) )
	ROM_LOAD16_BYTE( "sstsjb_2.5_2", 0x000000, 0x080000, CRC(c5337c56) SHA1(cd6f22e3c89c448538cdfec13c587ffd6213a038) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "sstsja_2.5_1", 0x000000, 0x080000, CRC(9990bc2e) SHA1(d5349297d0a144b465fbb5e0b9c9b9d2c9bd227b) )
	ROM_LOAD16_BYTE( "sstsjbd2.5_1", 0x000000, 0x080000, CRC(5792186e) SHA1(2f0675bf71004a07909c6f88f353e5919ba762b3) )
	ROM_LOAD16_BYTE( "sstsjad2.5_1", 0x000000, 0x080000, CRC(27976b30) SHA1(4eb25b89178ef4d1f14fa0e95410401fa60a0e0c) )
ROM_END

ROM_START( m5stars22 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sstsjb_2.2_1", 0x000000, 0x080000, CRC(bd666849) SHA1(1b66d346dc8592e9ffc86dabce361c548de4d634) )
	ROM_LOAD16_BYTE( "sstsjb_2.2_2", 0x000000, 0x080000, CRC(3a80af6c) SHA1(88a772fb5112b8730558fa5b45edeef86413be08) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "sstsjbd2.2_1", 0x000000, 0x080000, CRC(c9808abe) SHA1(a21e89e191f4e3ad89934ad12fe36a756e666fd3) )
	ROM_LOAD16_BYTE( "sstsjbg2.2_1", 0x000000, 0x080000, CRC(a760160a) SHA1(47cf45763fd3e16fad5bd419a32b63d80016aaf3) )
	ROM_LOAD16_BYTE( "sstsjbt2.2_1", 0x000000, 0x080000, CRC(b7f782d4) SHA1(9d5a2a6fa8b194adad5776ddc4bfcc98e669a45c) )
	ROM_LOAD16_BYTE( "sstsja_2.2_1", 0x000000, 0x080000, CRC(fe415e44) SHA1(63cdb5ca7216e3c75b15260c678a22f2c040f9d9) )
	ROM_LOAD16_BYTE( "sstsjad2.2_1", 0x000000, 0x080000, CRC(8aa7bcb3) SHA1(1a0152e44ce3fbdfc7c9ad6a6b8584afb474901b) )
ROM_END

ROM_START( m5stars20 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sstsjb_2.0_1", 0x000000, 0x080000, CRC(40ae8934) SHA1(2be06fff15b66d76a14474cd4d09e18ed990930d) )
	ROM_LOAD16_BYTE( "sstsjb_2.0_2", 0x000001, 0x080000, CRC(2bcec185) SHA1(900a7f9ac7b601b04618f6a6b6359d3917f4c732) )
ROM_END

ROM_START( m5stars10 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sstsjad1.0_1", 0x000000, 0x080000, CRC(3e98505c) SHA1(2790ff41b5a1056c05a116935beeba49a480f880) )
	ROM_LOAD16_BYTE( "sstsjad1.0_2", 0x000001, 0x080000, CRC(9a3269ef) SHA1(5d3e2945cf3ab11083fc0e736556691c9538d68d) )
ROM_END

ROM_START( m5stars10a ) // why is rom 2 different?
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sstsjbg1.0_1", 0x000000, 0x080000, CRC(17e894bd) SHA1(c1c656b4c9a3b9a1e10a4d0d709802ee7b3d72d5) )
	ROM_LOAD16_BYTE( "sstsjbg1.0_2", 0x000001, 0x080000, CRC(40f7d1b0) SHA1(815e73288b24f9c7d0a78b087b3227a1cc02558a) )
ROM_END


ROM_START( m5starcl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rwc__c_1.3_1", 0x000000, 0x080000, CRC(e90e3eeb) SHA1(44d92be2ffae2379f0dfef82b7b82fa80a74a9c2) )
	ROM_LOAD16_BYTE( "rwc__c_1.3_2", 0x000001, 0x080000, CRC(85d2d9ed) SHA1(e8631e808e8238256b7def1ee924c68a97462799) )
	ROM_LOAD16_BYTE( "rwc__c_1.3_3", 0x100000, 0x080000, CRC(c6f54119) SHA1(965aedb9b0a598e3612de58ef17f054c2f8d7b97) )
	ROM_LOAD16_BYTE( "rwc__c_1.3_4", 0x100001, 0x080000, CRC(0deb6922) SHA1(db3b8630d51b931f47b3b14acd02150df868acf0) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "rwc____1.3_1", 0x000000, 0x080000, CRC(b53925a7) SHA1(95646d99aec69bb2442eb36a976a6526c9423cf9) )
	ROM_LOAD16_BYTE( "rwc___d1.3_1", 0x000000, 0x080000, CRC(53f4d3dc) SHA1(764f1649dc3fe31480b13cfca81e728becaefe69) )
	ROM_LOAD16_BYTE( "rwc__cd1.3_1", 0x000000, 0x080000, CRC(0fc3c890) SHA1(dee7789aac3968a91259f7313e60326c7717eef9) )
	ROM_LOAD16_BYTE( "rwc_h__1.3_1", 0x000000, 0x080000, CRC(c4f6ae1f) SHA1(76521aaf72959df42617aa979bcb65f44453ccf9) )
	ROM_LOAD16_BYTE( "rwc_h_d1.3_1", 0x000000, 0x080000, CRC(e8b102f5) SHA1(37ad23cde7dc03a7fc2067509ec866337b2ad902) )
	ROM_LOAD16_BYTE( "rwc_hc_1.3_1", 0x000000, 0x080000, CRC(98c1b553) SHA1(b19e05fed1984bc25444ec8da705b80371127894) )
	ROM_LOAD16_BYTE( "rwc_hcd1.3_1", 0x000000, 0x080000, CRC(b48619b9) SHA1(1577d2a4142280aba8cb8506b089ba94b9b728fc) )
	ROM_LOAD16_BYTE( "rwc_lc_1.3_1", 0x000000, 0x080000, CRC(451e1488) SHA1(2f53e08b080233fd504c522959146731a9bb4d45) )
	ROM_LOAD16_BYTE( "rwc_lcd1.3_1", 0x000000, 0x080000, CRC(a3d3e2f3) SHA1(6815bb4cb84aa350466b484f235fa08a1feec5e5) )
ROM_END

ROM_START( m5startr )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sstwjb_2.6_1", 0x000000, 0x080000, CRC(e3f2c86c) SHA1(1b5e2dd2c70351842c97ae354373e09e61638414) )
	ROM_LOAD16_BYTE( "sstwjb_2.6_2", 0x000001, 0x080000, CRC(d760a231) SHA1(a3de9a712ab20d42e0c6f395f42be50ccc2aba02) )
	ROM_LOAD16_BYTE( "sstwjb_2.6_3", 0x100000, 0x080000, CRC(867fbcc7) SHA1(ee8091ee5caee5b5920f3e5ad85b4cdb7a6fd785) )
	ROM_LOAD16_BYTE( "sstwjb_2.6_4", 0x100001, 0x080000, CRC(04ee9dd4) SHA1(49ec743bac7eb8aedce6a908745535ecad88fcfc) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "sstwja_2.6_1", 0x000000, 0x080000, CRC(1e6ad09c) SHA1(2da43c46923b1707d1505dd3b3bc15ac02c60bc0) )
	ROM_LOAD16_BYTE( "sstwjad2.6_1", 0x000000, 0x080000, CRC(c140785a) SHA1(6c43065912437cd65d18b815e9001bde1d3e9a52) )
	ROM_LOAD16_BYTE( "sstwjbd2.6_1", 0x000000, 0x080000, CRC(3cd860aa) SHA1(b219d79dba2ab40f3223c7b709200f1124068b2d) )
	ROM_LOAD16_BYTE( "sstwjbg2.6_1", 0x000000, 0x080000, CRC(41d99bc8) SHA1(48087497a2e0b09e829771d0447576f0b7e727c4) )
	ROM_LOAD16_BYTE( "sstwjbt2.6_1", 0x000000, 0x080000, CRC(61b1d185) SHA1(0b55bc5a006a56317bbe5ba4e49fa2475c146ef1) )
ROM_END

ROM_START( m5stax )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "stax0_9s.p1",0x000000, 0x080000, CRC(e3df64b8) SHA1(43fb01d06fc02d0c63fe44554cf865607f860131) )
	ROM_LOAD16_BYTE( "stax0_9.p2", 0x000001, 0x080000, CRC(916ec2bb) SHA1(d2dc2c2d4b618984fd6730da6f2b0a51119a26d7) )
	ROM_LOAD16_BYTE( "stax0_9.p3", 0x100000, 0x080000, CRC(ab8e575a) SHA1(7c541858029ce09371aaff2ff745f0d72dae331e) )
	ROM_LOAD16_BYTE( "stax0_9.p4", 0x100001, 0x080000, CRC(5142eaf1) SHA1(95984ee6a182be31d8e22fa9a589fe3cc9526353) )
ROM_END

ROM_START( m5scharg )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "scha04.p1", 0x000000, 0x080000, CRC(92537e63) SHA1(482118f11478463dd1061ed29bfc458462de0dea) )
	ROM_LOAD16_BYTE( "scha04.p2", 0x000001, 0x080000, CRC(60d41088) SHA1(951305a7bb4a09574048e547468312a64c5b04ce) )
	ROM_LOAD16_BYTE( "scha04.p3", 0x100000, 0x080000, CRC(9276e73f) SHA1(dc4d1bf6ed29c9a82b89d98ac9d216a2257d504b) )
	ROM_LOAD16_BYTE( "scha04.p4", 0x100001, 0x080000, CRC(799408c4) SHA1(42fc19c4c93f5ee9bdcf9cc6bad3e0c506c81bd3) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "scha04ad.p1", 0x000000, 0x080000, CRC(7274b604) SHA1(3a5d0e67acddeded89da915e7d5cf6f3f171e6cb) )
	ROM_LOAD16_BYTE( "scha04b.p1", 0x000000, 0x080000, CRC(85d7e36c) SHA1(565f2c514bff1d3304897d6d595841de83742bae) )
	ROM_LOAD16_BYTE( "scha04bd.p1", 0x000000, 0x080000, CRC(667539f8) SHA1(8b1b2cde0b6ee1b95ab97ba5ee0867b756667397) )
	ROM_LOAD16_BYTE( "scha04d.p1", 0x000000, 0x080000, CRC(9b6cb02e) SHA1(155d208a30caebf383911626cd3ea64cbac74694) )
	ROM_LOAD16_BYTE( "scha04dy.p1", 0x000000, 0x080000, CRC(529e3c57) SHA1(df6b4ea2af813773ab297aa0ae9523910d30d380) )
	ROM_LOAD16_BYTE( "scha04k.p1", 0x000000, 0x080000, CRC(468c5fda) SHA1(c042f3d708500d64b8adc05ba187f899cbb58e15) )
	ROM_LOAD16_BYTE( "scha04r.p1", 0x000000, 0x080000, CRC(5929c975) SHA1(af10d3c9eada1e0854365ccb4c69cf41b398ec56) )
	ROM_LOAD16_BYTE( "scha04s.p1", 0x000000, 0x080000, CRC(92537e63) SHA1(482118f11478463dd1061ed29bfc458462de0dea) )
	ROM_LOAD16_BYTE( "scha04y.p1", 0x000000, 0x080000, CRC(5ba1f21a) SHA1(729baf64e52697f3e5e790235984ee594a2e4dfe) )
ROM_END

ROM_START( m5scharg05 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "scha05s.p1", 0x000000, 0x080000, CRC(c0c9115f) SHA1(6ae4080775692bdca6b800addc5d9b429a9db05c) )
	ROM_LOAD16_BYTE( "scha05.p2", 0x000001, 0x080000, CRC(8101d89d) SHA1(1bca2e5242c6b2ea7972c9c227fa53d8b67556a4) )
	ROM_LOAD16_BYTE( "scha05.p3", 0x100000, 0x080000, CRC(9276e73f) SHA1(dc4d1bf6ed29c9a82b89d98ac9d216a2257d504b) ) // == 04
	ROM_LOAD16_BYTE( "scha05.p4", 0x100001, 0x080000, CRC(799408c4) SHA1(42fc19c4c93f5ee9bdcf9cc6bad3e0c506c81bd3) ) // == 04

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "scha05ad.p1", 0x000000, 0x080000, CRC(2b4ae3c9) SHA1(d1f15d886b24c74d467a493e02eedc38464c1317) )
	ROM_LOAD16_BYTE( "scha05b.p1", 0x000000, 0x080000, CRC(e0227cbd) SHA1(8615cb719e5e99861b974ca3a9b5def34cc29898) )
	ROM_LOAD16_BYTE( "scha05bd.p1", 0x000000, 0x080000, CRC(e74228b8) SHA1(677c83f4f26660e5dd3bbcf3be2c8d95511e4550) )
	ROM_LOAD16_BYTE( "scha05d.p1", 0x000000, 0x080000, CRC(7ba4258e) SHA1(a5f381b0450b58bae5db9b8712404284b4270a96) )
	ROM_LOAD16_BYTE( "scha05dy.p1", 0x000000, 0x080000, CRC(4c63b53e) SHA1(f979007de6088f018d98ffe824959e530857bb0c) )
	ROM_LOAD16_BYTE( "scha05k.p1", 0x000000, 0x080000, CRC(0d3821aa) SHA1(a0eab6165ad0aa632b2f396dc8a845d8e50ab449) )
	ROM_LOAD16_BYTE( "scha05r.p1", 0x000000, 0x080000, CRC(a832eb63) SHA1(5e11e07fcef78cae9c168b1ad742823ea29c1df5) )
	ROM_LOAD16_BYTE( "scha05y.p1", 0x000000, 0x080000, CRC(f70e81ef) SHA1(403e019005803af77c361c91e16e9bf2fb5606b1) )
ROM_END

ROM_START( m5scharg06 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "scha06s.p1", 0x000000, 0x080000, CRC(9877e0fb) SHA1(afa371ebcf42cce3ce24b78951c28025b09dd617) )
	ROM_LOAD16_BYTE( "scha0_6.p2", 0x000001, 0x080000, CRC(0bd01c27) SHA1(56c078a48a6ae281e0779513b66d3a3d31d187b0) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "scha06ad.p1", 0x000000, 0x080000, CRC(f6daac46) SHA1(f2e1d6c1f6c563b043d80937e8dc033f88ada00e) )
	ROM_LOAD16_BYTE( "scha06bd.p1", 0x000000, 0x080000, CRC(5d96adc6) SHA1(abf1318f7b371561aad5d485a9231a18aa949b84) )
	ROM_LOAD16_BYTE( "scha06d.p1", 0x000000, 0x080000, CRC(7f8f6c13) SHA1(acd6774246650a0b267bbd57f0838407c67e07cc) )
	ROM_LOAD16_BYTE( "scha06dy.p1", 0x000000, 0x080000, CRC(f2ed6790) SHA1(d3d52bb6b194e4f9d7f049d676fe25433230b1a8) )
	ROM_LOAD16_BYTE( "scha06k.p1", 0x000000, 0x080000, CRC(0dd3a506) SHA1(fdeafb0fe705dd11d35602ad3006ea58d7d445f2) )
	ROM_LOAD16_BYTE( "scha06r.p1", 0x000000, 0x080000, CRC(5a62fe06) SHA1(6d7dc55e980dbcd9829f4ade6360cbb4ddba081d) )
	ROM_LOAD16_BYTE( "scha06y.p1", 0x000000, 0x080000, CRC(1515eb78) SHA1(f92ed6a02e4883c9b4b9c03373cb222f56ffe688) )
ROM_END

ROM_START( m5scharga )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "su_charg.p1", 0x00000, 0x080000, CRC(31a6dd3f) SHA1(1d4eb8d8fe230c39fb49cdd170ac1385fd9e86b7) )
	ROM_LOAD16_BYTE( "su_charg.p2", 0x00001, 0x080000, CRC(6ffd53df) SHA1(9ff711290511a689eb8a168bb43dac4f4949a80d) )
	/* 3+4 */
ROM_END


ROM_START( m5supro )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "srosjb_3.0_1", 0x000000, 0x080000, CRC(f09cb109) SHA1(6fe26e64bf51adb6f274816559e9de3c8e751272) )
	ROM_LOAD16_BYTE( "srosjb_3.0_2", 0x000001, 0x080000, CRC(4d3d1884) SHA1(949b063b4ed4c7c65697d31f53eb771abe6e9166) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "srosjbd3.0_1", 0x000000, 0x080000, CRC(2cff5336) SHA1(ed194276ee1b45743006043331d49cde1d89a71b) )
ROM_END

ROM_START( m5suproa )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "srt____3.0_1", 0x000000, 0x080000, CRC(3ec58187) SHA1(e4b3f355f0b14b40aab5cbc1a6182e1af3c0ddd2) )
	ROM_LOAD16_BYTE( "srt____3.0_2", 0x000001, 0x080000, CRC(7e67a377) SHA1(121171a1660338e998c188ff890df320f43e5642) )
ROM_END


ROM_START( m5supstr )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ssta02d.p1", 0x000000, 0x080000, CRC(ee801bfd) SHA1(a6c2878d326b51fe132a68b628692bdede64b150) )
	ROM_LOAD16_BYTE( "ssta02.p2", 0x000001, 0x080000, CRC(851d07e2) SHA1(a29264521d7244da2e0ad011e00604d61adff6a3) )
	ROM_LOAD16_BYTE( "ssta02.p3", 0x100000, 0x080000, CRC(02489b15) SHA1(245742ce6afa222d38494a602ec10a672b4028eb) )
	ROM_LOAD16_BYTE( "ssta02.p4", 0x100001, 0x080000, CRC(a453beda) SHA1(79a7ba97e2d2e0abb9c54c0f36552dc1bd7408c6) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "ssta02ad.p1", 0x000000, 0x080000, CRC(27bcf2fe) SHA1(02d2c465139be5fea8ec1c2dfbcc8cfba9d1cb5c) )
	ROM_LOAD16_BYTE( "ssta02b.p1", 0x000000, 0x080000, CRC(34cd88ec) SHA1(92028a7ff3e4aa78dc544019fbc2862eb3f2e3cd) )
	ROM_LOAD16_BYTE( "ssta02bd.p1", 0x000000, 0x080000, CRC(483656ac) SHA1(fc5619f42ea872d9cd89d327bc4437d44e80661b) )
	ROM_LOAD16_BYTE( "ssta02dy.p1", 0x000000, 0x080000, CRC(f69c3272) SHA1(9307d0b63ec2dc3195a0d6f8c7cc6881ae50aab8) )
	ROM_LOAD16_BYTE( "ssta02k.p1", 0x000000, 0x080000, CRC(86133b20) SHA1(5ced921b1455f3f8bbaa6a0aa72d4b7768affa5d) )
	ROM_LOAD16_BYTE( "ssta02r.p1", 0x000000, 0x080000, CRC(c10750c2) SHA1(3ef557488fe1295d3a4760c911009bcbd588213b) )
	ROM_LOAD16_BYTE( "ssta02s.p1", 0x000000, 0x080000, CRC(8e93cda1) SHA1(639c4ea0281de69cb3e4f8ba2dd22c1273e25943) )
	ROM_LOAD16_BYTE( "ssta02y.p1", 0x000000, 0x080000, CRC(968fe42e) SHA1(4ca0807e3c0465c755a097c6b179ec03e84d5404) )
ROM_END

ROM_START( m5supstra )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "s_star.p1", 0x00000, 0x080000, CRC(bd14a2b2) SHA1(71c1a14111f473d8440ad0b9b6a03d1d29d7c500) )
	ROM_LOAD16_BYTE( "s_star.p2", 0x00001, 0x080000, CRC(45b0aea0) SHA1(8cf7ac02ce97e94c07e21b1d7f66e8897b92b641) )
	/* 3+4 */
ROM_END


ROM_START( m5sstrk )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "strt02d.p1", 0x000000, 0x080000, CRC(0b042edf) SHA1(1a1a62811401ab981b78f47532fc6c13b7b3d739) )
	ROM_LOAD16_BYTE( "strt02.p2", 0x000001, 0x080000, CRC(637ee0be) SHA1(ceab0d3e4ecad7a95847d05f07348c5b66a37bba) )
	ROM_LOAD16_BYTE( "strt02.p3", 0x100000, 0x080000, CRC(3e4350f6) SHA1(ae892bf921369a4188c1eddb51b0491124f9bf5b) )
	ROM_LOAD16_BYTE( "strt02.p4", 0x100001, 0x080000, CRC(c73f0a2a) SHA1(7bbc80d5e5fb2ebd312bb3c6bcb4be358eb978d4) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "strt02ad.p1", 0x000000, 0x080000, CRC(7fc3adf2) SHA1(3a7a0ac9924d4fae3fd4486c54b5913a62d1eb50) )
	ROM_LOAD16_BYTE( "strt02b.p1", 0x000000, 0x080000, CRC(fc8d1019) SHA1(1a96b3a4f57cb67e367e7efb53b8659f9ce511c2) )
	ROM_LOAD16_BYTE( "strt02bd.p1", 0x000000, 0x080000, CRC(5a3adb3b) SHA1(a20fd98940bab6de5cbcea97549a77253c2b83d0) )
	ROM_LOAD16_BYTE( "strt02dy.p1", 0x000000, 0x080000, CRC(b2343575) SHA1(6fb09904f92cf850da1fe903a6e66d2dfeccb2d4) )
	ROM_LOAD16_BYTE( "strt02r.p1", 0x000000, 0x080000, CRC(656344b0) SHA1(7c353b679720fef8c3f7897d15ce773f2f63d762) )
	ROM_LOAD16_BYTE( "strt02s.p1", 0x000000, 0x080000, CRC(8d174ce4) SHA1(4cf3400e36dbd8af7fdd67791b9ae7f0846233db) )
	ROM_LOAD16_BYTE( "strt02y.p1", 0x000000, 0x080000, CRC(3427574e) SHA1(7d575efb123d0afb0de8bc3652e949a5cdfcbf3c) )
ROM_END

ROM_START( m5sstrk02a )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "strb0_2.p1", 0x000000, 0x080000, CRC(2b058cf8) SHA1(816c3a4a85570fd9991822383bd515a616b67e6d) )
	ROM_LOAD16_BYTE( "strb0_2.p2", 0x000001, 0x080000, CRC(04ec1ef6) SHA1(48c94c4886d2c7c25bd0743b40544e806a7272d8) )
	/* 3+4 */
ROM_END


ROM_START( m5tempp )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tplwsjb_1.2_1", 0x000000, 0x080000, CRC(db9553fc) SHA1(6b6a07776d7191c4e540849f0dd9405d18e9f778) )
	ROM_LOAD16_BYTE( "tplwsjb_1.2_2", 0x000001, 0x080000, CRC(48959ffe) SHA1(82c78c7466862837b8b4cf568ac2722507a2fe55) )
	ROM_LOAD16_BYTE( "tplwsjb_1.2_3", 0x100000, 0x080000, CRC(c6040456) SHA1(9c640e965e2c386d496dbc26405c59d49e8d96bb) )
	ROM_LOAD16_BYTE( "tplwsjb_1.2_4", 0x100001, 0x080000, CRC(2aef4ecc) SHA1(545732a3962dbdd6d5a0df64e6a185689be1c88c) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "tplwsja_1.2_1", 0x000000, 0x080000, CRC(1507cb7f) SHA1(f2edabd1ce0f4ccef7a0ee515f2fb1ce060b0cb1) )
	ROM_LOAD16_BYTE( "tplwsjad1.2_1", 0x000000, 0x080000, CRC(eabfa8c1) SHA1(d97b9aa93925efc95fa9d7b50243daf3facc24a0) )
	ROM_LOAD16_BYTE( "tplwsjbd1.2_1", 0x000000, 0x080000, CRC(1af53f85) SHA1(0135baee21fce2e98ae6d6ba7b162ac88cd4d013) )
	ROM_LOAD16_BYTE( "tplwsjbg1.2_1", 0x000000, 0x080000, CRC(58f9e49b) SHA1(05705483fc943bbd1d1d795968805e546806310e) )
	ROM_LOAD16_BYTE( "tplwsjbt1.2_1", 0x000000, 0x080000, CRC(14526e73) SHA1(7e8ced86afa086c68a7f9a83d98fe5bb73ffb907) )
ROM_END

ROM_START( m5tempt )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "t1tr10d.p1", 0x000000, 0x080000, CRC(b8d90c17) SHA1(c9bc8f9b58a0317ffb7c4887e5eb76c0043bf81b) )
	ROM_LOAD16_BYTE( "t1tr10.p2", 0x000001, 0x080000, CRC(d3d036bf) SHA1(4b09d9e5ae42698e93ab6a2192bd39154e9fa0d2) )
	ROM_LOAD16_BYTE( "t1tr10.p3", 0x100000, 0x080000, CRC(c6714248) SHA1(d531c3535aa97f4d08e5629e85018d380c07f92c) )
	ROM_LOAD16_BYTE( "t1tr10.p4", 0x100001, 0x080000, CRC(ccd6599b) SHA1(96bcc9404ba3e6186be18f322ae6531bc863e589) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "t1tr10dy.p1", 0x000000, 0x080000, CRC(a28ea617) SHA1(6901835a50dd031061c38eae80fde6a4b27e356d) )
	ROM_LOAD16_BYTE( "t1tr10s.p1", 0x000000, 0x080000, CRC(3e044d74) SHA1(62d72686a307556b7f1e592fa7ec724c9c60e013) )
	ROM_LOAD16_BYTE( "t1tr10y.p1", 0x000000, 0x080000, CRC(2453e774) SHA1(6bf24a7f046b2d7a1887ba199487f1a0829c28ac) )
ROM_END

ROM_START( m5tempt05 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "totr05y.p1", 0x000000, 0x080000, CRC(26487d03) SHA1(595a39724c587c7bb7d8b10e3838be1d5c8b3d3e) )
	ROM_LOAD16_BYTE( "totr05.p2", 0x000001, 0x080000, CRC(052afdcb) SHA1(c9c2531d9c7a9cfd0b60f29d54efab06c72e64dd) )
	ROM_LOAD16_BYTE( "totr05.p3", 0x100000, 0x080000, CRC(c6714248) SHA1(d531c3535aa97f4d08e5629e85018d380c07f92c) ) // == 10
	ROM_LOAD16_BYTE( "totr05.p4", 0x100001, 0x080000, CRC(ccd6599b) SHA1(96bcc9404ba3e6186be18f322ae6531bc863e589) ) // == 10

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "totr05b.p1", 0x000000, 0x080000, CRC(b780ea14) SHA1(8431404ed39d49665bd67a75a88bd4cdd2fab2b4) )
	ROM_LOAD16_BYTE( "totr05bd.p1", 0x000000, 0x080000, CRC(90f2449c) SHA1(71cce259af2438477a009cc06a21dd5c9d5661d3) )
	ROM_LOAD16_BYTE( "totr05d.p1", 0x000000, 0x080000, CRC(ff0dc97e) SHA1(48749a3b7de5829139901b9bb3bd52146978d0a0) )
	ROM_LOAD16_BYTE( "totr05dy.p1", 0x000000, 0x080000, CRC(013ad38b) SHA1(d0bd6805d825f3ac1075a229bfc6e1ca8c8503b4) )
	ROM_LOAD16_BYTE( "totr05k.p1", 0x000000, 0x080000, CRC(fc24ed59) SHA1(0d8fcd8aeaf3bddd7797bfe004ca68ebd7c685ce) )
ROM_END

ROM_START( m5tempta )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tot_1mux.p1", 0x00000, 0x080000, CRC(f13cbb76) SHA1(9a401efe7b4c1ac8b76180455408ed94cd5c1244) )
	ROM_LOAD16_BYTE( "tot_1mux.p2", 0x00001, 0x080000, CRC(a422cdd6) SHA1(cfcd63ce66de881a7184c68fa468fe1350579c4d) )
	/* 3+4 */
ROM_END

ROM_START( m5temptb )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tot_1mux.p1_a", 0x00000, 0x080000, CRC(171c1380) SHA1(efac1f8711f60c53ec3f5604207cc320307f345e) )
	ROM_LOAD16_BYTE( "tot_1mux.p2_a", 0x00001, 0x080000, CRC(b538dd51) SHA1(358a2319a54b38300b9ed5398abe96fcdb9773bc) )
	/* 3+4 */
ROM_END



ROM_START( m5tempt2 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "t2tr10d.p1",0x000000, 0x080000, CRC(fcadfa89) SHA1(817e8d91d062b7725fdca77b98bb5a7b409a7864) )
	ROM_LOAD16_BYTE( "t2tr10.p2", 0x000001, 0x080000, CRC(ef53eae7) SHA1(964c6d74e2695b8f236d21fdb2c83b979e152c67) )
	ROM_LOAD16_BYTE( "t2tr10.p3", 0x100000, 0x080000, CRC(c6714248) SHA1(d531c3535aa97f4d08e5629e85018d380c07f92c) )
	ROM_LOAD16_BYTE( "t2tr10.p4", 0x100001, 0x080000, CRC(ccd6599b) SHA1(96bcc9404ba3e6186be18f322ae6531bc863e589) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "t2tr10dy.p1", 0x000000, 0x080000, CRC(a800d477) SHA1(ccbba560c7ea0b7fa0efe66fefe2527e1fef4cfc) )
	ROM_LOAD16_BYTE( "t2tr10s.p1", 0x000000, 0x080000, CRC(8f67c4b7) SHA1(14cbf15dcffe054cee285034b16a11f0dcac6213) )
	ROM_LOAD16_BYTE( "t2tr10y.p1", 0x000000, 0x080000, CRC(dbcaea49) SHA1(01463d5e4a003cb2b9d8e8f8304285781c7e15d5) )
ROM_END

ROM_START( m5tempt203 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tot203s.p1", 0x000000, 0x080000, CRC(100ba124) SHA1(9716a5a430a2f811f093acd776b6342530613fd6) )
	ROM_LOAD16_BYTE( "tot203.p2", 0x000001, 0x080000, CRC(d9bc060a) SHA1(97d7c083f62066456380e96ad9c0020562845a5f) )
	ROM_LOAD16_BYTE( "tot203.p3", 0x100000, 0x080000, CRC(c6714248) SHA1(d531c3535aa97f4d08e5629e85018d380c07f92c) ) // == 10
	ROM_LOAD16_BYTE( "tot203.p4", 0x100001, 0x080000, CRC(ccd6599b) SHA1(96bcc9404ba3e6186be18f322ae6531bc863e589) ) // == 10

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "tot203b.p1", 0x000000, 0x080000, CRC(d56f9fc9) SHA1(2a1fff24ce515bd06924e95996946e3b84779e5d) )
	ROM_LOAD16_BYTE( "tot203bd.p1", 0x000000, 0x080000, CRC(e258b0da) SHA1(8a2eaa1253da8f03edb5fb20c78e32e74b29a28a) )
	ROM_LOAD16_BYTE( "tot203d.p1", 0x000000, 0x080000, CRC(273c8e37) SHA1(7efa094c73b879cb55cb4c996850c64ecd134508) )
	ROM_LOAD16_BYTE( "tot203dy.p1", 0x000000, 0x080000, CRC(5a99bf68) SHA1(a6a84a1b9c3b94df9eae6327b6d3b74913679b46) )
	ROM_LOAD16_BYTE( "tot203h.p1", 0x000000, 0x080000, CRC(7f67a2c8) SHA1(e7790dbadeffd01039f1ceef84b8a57f59b593d8) )
	ROM_LOAD16_BYTE( "tot203k.p1", 0x000000, 0x080000, CRC(35db2f99) SHA1(4c1e8bce8ed1e31883acf906adfaf8e0323f74a3) )
	ROM_LOAD16_BYTE( "tot203r.p1", 0x000000, 0x080000, CRC(0ec20b5b) SHA1(1e919fb4e5b22c95b642cfe6d61db6c8def8adce) )
	ROM_LOAD16_BYTE( "tot203y.p1", 0x000000, 0x080000, CRC(6dae907b) SHA1(8033fd901ac25ebbbe0b3562eb3c55cde607fe99) )
ROM_END

ROM_START( m5tempt2a )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tot_2mux.p1", 0x00000, 0x080000, CRC(b851e2f3) SHA1(7fbb2fd9cb03e7d16ba8b3aeb8dc17d72304c7df) )
	ROM_LOAD16_BYTE( "tot_2mux.p2", 0x00001, 0x080000, CRC(57ce89bf) SHA1(e7b666700ee2f6bfa1dbca7c32d7e92fd10101c2) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "tot_2mux.p2_a", 0x00001, 0x080000, CRC(a0874f29) SHA1(0e29fb2703fdc34244d5ba6169d774abd0cacf70) )
ROM_END

ROM_START( m5tempcl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "totc01.p1", 0x000000, 0x080000, CRC(5840b478) SHA1(78758f10189cdd2d99925c8546b1553535ff9598) )
	ROM_LOAD16_BYTE( "totc01.p2", 0x000001, 0x080000, CRC(b2f21815) SHA1(e32539ae8d43d01019a304617401789c45b7f092) )
	ROM_LOAD16_BYTE( "totc01.p3", 0x100000, 0x080000, CRC(acb2511c) SHA1(59fd87ffcc96cabb56c5e4708a475de533aa20f6) )
	ROM_LOAD16_BYTE( "totc01.p4", 0x100001, 0x080000, CRC(070ffa66) SHA1(5e0a756485ce07a2508c65f81911b7be0f638b54) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "totc01d.p1", 0x000000, 0x080000, CRC(f29d62cc) SHA1(2b147e63a4c906cf872ac34c1aab362faa5744c1) )
	ROM_LOAD16_BYTE( "totc01dz.p1", 0x000000, 0x080000, CRC(31757244) SHA1(c179ae359cfb698d96549aa583ace9d98485f732) )
	ROM_LOAD16_BYTE( "totc01f.p1", 0x000000, 0x080000, CRC(9368f6bc) SHA1(25e1f9d72bfeff8697c067a427133541ecd6ff5a) )
	ROM_LOAD16_BYTE( "totc01s.p1", 0x000000, 0x080000, CRC(5840b478) SHA1(78758f10189cdd2d99925c8546b1553535ff9598) )
	ROM_LOAD16_BYTE( "totc01z.p1", 0x000000, 0x080000, CRC(03f646ba) SHA1(d337552dd05a21c6ef9301e1bcf53769eb40f0af) )
ROM_END

ROM_START( m5tball )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "thun0_3.p1", 0x000000, 0x080000, CRC(5b6397de) SHA1(f20faffd4ba7b654aad3974593a8fda0455d83b6) )
	ROM_LOAD16_BYTE( "thun0_3.p2", 0x000001, 0x080000, CRC(19e649fa) SHA1(dd71327f31b14edc37ea94e460b221a6caaf683b) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "thun0_3d.p1", 0x000000, 0x080000, CRC(12202800) SHA1(eb7a8d5c33954c6949e1efdde8c79fae62522815) )
ROM_END

ROM_START( m5tbird )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tbrd20ad.p1", 0x000000, 0x080000, CRC(c42dac13) SHA1(ac4d6136945cbe07eba02edf75ac0251ea8e028a) )
	ROM_LOAD16_BYTE( "tbrd20.p2", 0x000001, 0x080000, CRC(4223b137) SHA1(7334ffda0151e821b67d5006c524b54419c1b2c7) )
	ROM_LOAD16_BYTE( "tbrd20.p3", 0x100000, 0x080000, CRC(7f172b77) SHA1(7215ce9966a72e797c0d5c25256fe5ccfaee8990) )
	ROM_LOAD16_BYTE( "tbrd20.p4", 0x100001, 0x080000, CRC(1d064f45) SHA1(dba3ed766c99de68a25ec3e6cd08c3b8d0ad2518) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "tbrd20b.p1", 0x000000, 0x080000, CRC(7019b3f9) SHA1(556675a7441642666d68b27a7a0283b1b8e11723) )
	ROM_LOAD16_BYTE( "tbrd20bd.p1", 0x000000, 0x080000, CRC(510d9762) SHA1(9dafafaaf962bac416eb8f090f2355d97977057b) )
	ROM_LOAD16_BYTE( "tbrd20d.p1", 0x000000, 0x080000, CRC(64b988fc) SHA1(12f261dca11e88c587a76e9c2c5725835389c68c) )
	ROM_LOAD16_BYTE( "tbrd20dy.p1", 0x000000, 0x080000, CRC(f6fe9a3e) SHA1(e4c898dd793c956b09eb8cae280af1015df92435) )
	ROM_LOAD16_BYTE( "tbrd20h.p1", 0x000000, 0x080000, CRC(41395740) SHA1(a60c72dc7cdd733dd99e241a751b6d01b9dd3c94) )
	ROM_LOAD16_BYTE( "tbrd20r.p1", 0x000000, 0x080000, CRC(48083177) SHA1(fbf201108dc7cf02e31226c86ea32139c81a9c13) )
	ROM_LOAD16_BYTE( "tbrd20s.p1", 0x000000, 0x080000, CRC(f66f633a) SHA1(1cebcb41d40d177e2575eb3c41728d079e35327e) )
	ROM_LOAD16_BYTE( "tbrd20y.p1", 0x000000, 0x080000, CRC(642871f8) SHA1(ea424360e6e9a12a3e2edeee83756b8cc71c91a6) )
	ROM_LOAD16_BYTE( "t_birds.p1", 0x0000, 0x080000, CRC(d1a616c7) SHA1(cb917146e30e39ef87dd32f6ac1d4254f402f293) )
	ROM_LOAD16_BYTE( "t_birds.p2", 0x0000, 0x080000, CRC(516c3d6b) SHA1(4aa36dde3840956374fab4c34071625f865370d5) )

ROM_END

ROM_START( m5trclb )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ctom0_3.p1", 0x000000, 0x080000, CRC(e5a6df0b) SHA1(1bfabef4802df305512cb0ca0be06981aae6f438) )
	ROM_LOAD16_BYTE( "ctom0_3.p2", 0x000001, 0x080000, CRC(64335126) SHA1(e3fe46c5cd22653e4c869ae2bb44e9156fd76825) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "ctom0_3d.p1", 0x000000, 0x080000, CRC(b5938bb8) SHA1(cfb9b1db4e1f52816d726103d7c6fe65fbdb6b89) )
ROM_END

ROM_START( m5topdog )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tdog03ad.p1", 0x000000, 0x080000, CRC(b11b3ad4) SHA1(939bafce76a80395fa517496ef993f5cfe647cd8) )
	ROM_LOAD16_BYTE( "tdog03.p2", 0x000001, 0x080000, CRC(5df45ea9) SHA1(38a49e36761c43e642215daa2d6138132f50ffd1) )
	ROM_LOAD16_BYTE( "tdog03.p3", 0x100000, 0x080000, CRC(a44fe7d2) SHA1(628ac27fc24d69f3e7e1f3cab7d0a920b92235c9) )
	ROM_LOAD16_BYTE( "tdog03.p4", 0x100001, 0x080000, CRC(353a5f78) SHA1(f011b457ec0291a9d1d091dc0ce85d2a83acd3b0) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "tdog03b.p1", 0x000000, 0x080000, CRC(a6b677be) SHA1(774921590f2eb4f59cae98f97c72c2f605819f18) )
	ROM_LOAD16_BYTE( "tdog03bd.p1", 0x000000, 0x080000, CRC(864ca52b) SHA1(fcbe7cc2a4e3855861b64ca747c2d1de62a59971) )
	ROM_LOAD16_BYTE( "tdog03d.p1", 0x000000, 0x080000, CRC(5f112944) SHA1(805816e64dc6a97fe82c6b75aa376c67d8e4d2ad) )
	ROM_LOAD16_BYTE( "tdog03dy.p1", 0x000000, 0x080000, CRC(b8549378) SHA1(4b390a7489dbb99d3005b36d5fab618202b129f2) )
	ROM_LOAD16_BYTE( "tdog03k.p1", 0x000000, 0x080000, CRC(357b4b89) SHA1(3ac9807e57b66560d0c2dbcd9840a584e788df1d) )
	ROM_LOAD16_BYTE( "tdog03r.p1", 0x000000, 0x080000, CRC(fc431f59) SHA1(6030aa6f5b736cf0bc033cda741b4d144ea2e390) )
	ROM_LOAD16_BYTE( "tdog03s.p1", 0x000000, 0x080000, CRC(2b4766ea) SHA1(26541113ba75adaf5813752d93e93dde1dd5e758) )
	ROM_LOAD16_BYTE( "tdog03y.p1", 0x000000, 0x080000, CRC(cc02dcd6) SHA1(0bfbe6d714af67cb9454af9a90703b4008c170e8) )
ROM_END

ROM_START( m5topdog04 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tdog04s.p1", 0x000000, 0x080000, CRC(aa37190c) SHA1(b3d11d0ff3b75b6a037acb1fd815f34128db55ae) )
	ROM_LOAD16_BYTE( "tdog04.p2", 0x000001, 0x080000, CRC(4b4e13f3) SHA1(fa1d834b1fd7cdcdf8df282a943e3a83eecf57e8) )
	ROM_LOAD16_BYTE( "tdog04.p3", 0x100000, 0x080000, CRC(a44fe7d2) SHA1(628ac27fc24d69f3e7e1f3cab7d0a920b92235c9) ) // == 03
	ROM_LOAD16_BYTE( "tdog04.p4", 0x100001, 0x080000, CRC(353a5f78) SHA1(f011b457ec0291a9d1d091dc0ce85d2a83acd3b0) ) // == 03

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "tdog04ad.p1", 0x000000, 0x080000, CRC(aa83dca6) SHA1(8d92d071d773ecb191d5bab7371f868cbfc84e85) )
	ROM_LOAD16_BYTE( "tdog04b.p1", 0x000000, 0x080000, CRC(9d43f02a) SHA1(818d76061074af2d4790c19cfc42be00d93ac755) )
	ROM_LOAD16_BYTE( "tdog04bd.p1", 0x000000, 0x080000, CRC(889de7e6) SHA1(2f4ec93a72864ef577b3b8911e2534e8ddd5f9fc) )
	ROM_LOAD16_BYTE( "tdog04d.p1", 0x000000, 0x080000, CRC(b1a55bb2) SHA1(29b048edd8fcc925d885109090ab73de391b37cd) )
	ROM_LOAD16_BYTE( "tdog04dy.p1", 0x000000, 0x080000, CRC(0fa3032b) SHA1(99a315274bf323ad3921f73c1d6113d2f3589544) )
	ROM_LOAD16_BYTE( "tdog04k.p1", 0x000000, 0x080000, CRC(dd7314ce) SHA1(3cf26aa5c86627396f0c36cfaeca8e61f1657d69) )
	ROM_LOAD16_BYTE( "tdog04r.p1", 0x000000, 0x080000, CRC(df8b18c0) SHA1(7cf8fab80b2d76bc23f0a3ba361eae0a39a95fa5) )
	ROM_LOAD16_BYTE( "tdog04y.p1", 0x000000, 0x080000, CRC(14314195) SHA1(5d5289f806590f731b3a4ee706a962482a934078) )
ROM_END

ROM_START( m5topdoga )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "top_dog.p1", 0x00000, 0x080000, CRC(c869752f) SHA1(2a5318a968b6d1de07926f3f33f72fa5533fc6b7) )
	ROM_LOAD16_BYTE( "top_dog.p2", 0x00001, 0x080000, CRC(8fca93e2) SHA1(712a3b4877d9703f7a846231332961518ed2ae76) )
	/* 3+4 */
ROM_END


ROM_START( m5topdol )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tpd13s.p1", 0x000000, 0x080000, CRC(4e90e029) SHA1(252c308d034d03c7e89ef2b581b5651dedd13888) )
	ROM_LOAD16_BYTE( "tpd13s.p2", 0x000001, 0x080000, CRC(31fc8222) SHA1(efd08b7162f85b6505738908b28eaddb54554aba) )
	ROM_LOAD16_BYTE( "tpd13s.p3", 0x100000, 0x080000, CRC(a0df85d6) SHA1(18248f1cdf1c791f62e929ea324d38e3103ea441) )
	ROM_LOAD16_BYTE( "tpd13s.p4", 0x100001, 0x080000, CRC(68659698) SHA1(23c9f0ea7340046181cce0ed9ae2c8879652a6d4) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "tpd13d.p1", 0x000000, 0x080000, CRC(c878b025) SHA1(3c594a94b53491edc42e91b5f7ff80c5194d38e1) )
	ROM_LOAD16_BYTE( "tpd13k.p1", 0x000000, 0x080000, CRC(6a5271c5) SHA1(1b576d4de1ce7d2da37e17143eb3719d6bc703c8) )
ROM_END

ROM_START( m5topdola ) // only has alt sound roms? are they bad?
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "t doller.p1", 0x000000, 0x080000, CRC(4e90e029) SHA1(252c308d034d03c7e89ef2b581b5651dedd13888) )
	ROM_LOAD16_BYTE( "t doller.p2", 0x000001, 0x080000, CRC(31fc8222) SHA1(efd08b7162f85b6505738908b28eaddb54554aba) )
	ROM_LOAD16_BYTE( "t doller.p3", 0x100000, 0x080000, CRC(11ce71f0) SHA1(30603a897d5226d3d5c064b77d3c6314d21fe709) )
	ROM_LOAD16_BYTE( "t doller.p4", 0x100001, 0x080000, CRC(abd514e0) SHA1(a42f35b57ad41a02a9f6969fbeb380152b5b8bf0) )
ROM_END


ROM_START( m5trail )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "trbl06d.p1", 0x000000, 0x080000, CRC(c0b4b38b) SHA1(ff5e8c6a373dd218780be6edb6eb8c757c07f386) )
	ROM_LOAD16_BYTE( "trbl06.p2", 0x000001, 0x080000, CRC(80fbbc38) SHA1(11e45ce5630c73260a697bd609079168a4971c04) )
	ROM_LOAD16_BYTE( "trbl06.p3", 0x100000, 0x080000, CRC(70ff9130) SHA1(eaf9b199b720993139b7c88700202022fb4f937f) )
	ROM_LOAD16_BYTE( "trbl06.p4", 0x100001, 0x080000, CRC(2d6c5a68) SHA1(626ee962e5a7b8db6bad8cf36be5484a51d737be) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "trbl06dz.p1", 0x000000, 0x080000, CRC(1d72e21b) SHA1(4c99a8cbb9fd06dc6e2bf24bcbc0e41a8d018cd3) )
	ROM_LOAD16_BYTE( "trbl06f.p1", 0x000000, 0x080000, CRC(a40c0c07) SHA1(7b4bca474402372abc985f79c482d4ba9f810cd2) )
	ROM_LOAD16_BYTE( "trbl06s.p1", 0x000000, 0x080000, CRC(f8035826) SHA1(0c5cdc355721b9be35e819fa6c5d09e6ebc6cc07) )
	ROM_LOAD16_BYTE( "trbl06z.p1", 0x000000, 0x080000, CRC(c7d1d42f) SHA1(6b70f64fdbc00542f5717bc5e4f27a044d27d2ff) )
ROM_END

ROM_START( m5sblz )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "s_blazer.p1", 0x00000, 0x080000, CRC(e4840160) SHA1(8521f590ee43f2014ab874731971c86f5b7a877f) )
	ROM_LOAD16_BYTE( "s_blazer.p2", 0x00001, 0x080000, CRC(a6760014) SHA1(f49d14ea91cbe842a0d5fec6ca99248a962885d6) )
ROM_END


ROM_START( m5ttop )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "treb0_1.p1", 0x000000, 0x080000, CRC(160801f1) SHA1(6e46b965a72d888c03a81e21a49128f6fdd62bc7) )
	ROM_LOAD16_BYTE( "treb0_1.p2", 0x000001, 0x080000, CRC(076a8a59) SHA1(f702d1b3599d05f456c86ce6bc2f14bdd50cf092) )
	ROM_LOAD16_BYTE( "treb0_1.p3", 0x100000, 0x080000, CRC(c9d4f59c) SHA1(4d98ba26295955be86f74c0a557c4ae0509edd04) )
	ROM_LOAD16_BYTE( "treb0_1.p4", 0x100001, 0x080000, CRC(7870b15a) SHA1(773ad400a6f7d453b57d46341e7f20f5eed322f7) )
ROM_END

ROM_START( m5ttop04 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "treb0_4.p1", 0x000000, 0x080000, CRC(63586be3) SHA1(15f258a8c7d3478f0492dc0ae03c348440ae1423) )
	ROM_LOAD16_BYTE( "treb0_4.p2", 0x000001, 0x080000, CRC(300962d5) SHA1(84e29456057a75ef6e89f44b42560c2a0eeb6a65) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "treb0_4d.p1", 0x000000, 0x080000, CRC(14a72539) SHA1(13b6fceba88ebf4a45960ece1d5b3c0441f8393e) )
ROM_END

ROM_START( m5ttop10 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "treb1_0.p1", 0x000000, 0x080000, CRC(1c349abc) SHA1(04b80982f4c5ec0545549410b1ef8128c459a5e4) )
	ROM_LOAD16_BYTE( "treb1_0.p2", 0x000001, 0x080000, CRC(8ea40d3e) SHA1(56fc6f1cb4b1cc5d475d332fe8f19ceaeac2a1f4) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "treb1_0d.p1", 0x000000, 0x080000, CRC(a10546a6) SHA1(a4bb0dd99d5c0122be73db34278109846eaed334) )
ROM_END


ROM_START( m5ttopcl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ctre0_4.p1", 0x000000, 0x080000, CRC(6e714652) SHA1(ddb18e0608de665614e6606d794d346e94215c16) )
	ROM_LOAD16_BYTE( "ctre0_4.p2", 0x000001, 0x080000, CRC(6183a650) SHA1(fddfdbcd15bedcd4821cb79bf88f5024f6e5db9e) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "ctre0_4d.p1", 0x000000, 0x080000, CRC(f8245ad1) SHA1(90ac3786c4a484754a5069c985225e115c62c09d) )
ROM_END

ROM_START( m5tsar )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tsar0_2.p1", 0x000000, 0x080000, CRC(91efceec) SHA1(f571e9c0b3278065894ad98b5b1c41fe59bdbb2d) )
	ROM_LOAD16_BYTE( "tsar0_2.p2", 0x000001, 0x080000, CRC(8a760ff1) SHA1(d5774db6dc8c7430eeae6b1466e7b823eae8227b) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "tsar0_2d.p1", 0x000000, 0x080000, CRC(1ad54507) SHA1(fde9df26862cd229790fca4447792e3482abf8ad) )
ROM_END

ROM_START( m5ultimo )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ulti03ad.p1", 0x000000, 0x080000, CRC(6ebb8aaa) SHA1(47f9c1c3455d6782a3808324df65a1b5a64cd392) )
	ROM_LOAD16_BYTE( "ulti03.p2", 0x000001, 0x080000, CRC(4a634758) SHA1(088aaf29401a84614697e3f24e69e079b8e5c932) )
	ROM_LOAD16_BYTE( "ulti03.p3", 0x100000, 0x080000, CRC(bfc649eb) SHA1(85f31848918e35385ab96cbadadbc33ddb1e42b1) )
	ROM_LOAD16_BYTE( "ulti03.p4", 0x100001, 0x080000, CRC(8579127e) SHA1(5ba966008c0f7b4c910eddc6400ed2178c70e096) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "ulti03b.p1", 0x000000, 0x080000, CRC(9bc07f21) SHA1(c8f32d13b35a53c40dc3233b77f134ca6eb0111e) )
	ROM_LOAD16_BYTE( "ulti03bd.p1", 0x000000, 0x080000, CRC(63459648) SHA1(038d8e56d0718c628b802cc6919e8a19ac19cfab) )
	ROM_LOAD16_BYTE( "ulti03d.p1", 0x000000, 0x080000, CRC(b9eae053) SHA1(b2c5b723005e44a2be7691640f2b52f11caa2332) )
	ROM_LOAD16_BYTE( "ulti03dy.p1", 0x000000, 0x080000, CRC(32ddeda8) SHA1(1f48d56b15dada7ab8b7b88fc8f2656ffbcf1a3d) )
	ROM_LOAD16_BYTE( "ulti03h.p1", 0x000000, 0x080000, CRC(543f0844) SHA1(62003302c70de51092a2742fc498e6eaed850683) )
	ROM_LOAD16_BYTE( "ulti03k.p1", 0x000000, 0x080000, CRC(92537e11) SHA1(0094749ee9856035b97c3afeb08a3c3c51d12291) )
	ROM_LOAD16_BYTE( "ulti03r.p1", 0x000000, 0x080000, CRC(119fc875) SHA1(2eed3458e5a49ac4a8bb78154c8a4f260a4ee336) )
	ROM_LOAD16_BYTE( "ulti03s.p1", 0x000000, 0x080000, CRC(a36a7616) SHA1(0ae8ffe9c5daf6ae4373eeed3e4bfc893b9199c2) )
	ROM_LOAD16_BYTE( "ulti03y.p1", 0x000000, 0x080000, CRC(285d7bed) SHA1(1198555279ad2c4d66f13a03d372125118232c40) )
ROM_END

ROM_START( m5ultimo03a )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ultr03s.p1", 0x000000, 0x080000, CRC(cfc11235) SHA1(0c09e9860d2ac3589cf75a11ca43bd755a9a432f) )
	ROM_LOAD16_BYTE( "ultr03.p2", 0x000001, 0x080000, CRC(b78b8f57) SHA1(ce1c20e8a97727c2b37983edb47c0603ac21a59a) )
	ROM_LOAD16_BYTE( "ultr03.p3", 0x100000, 0x080000, CRC(bfc649eb) SHA1(85f31848918e35385ab96cbadadbc33ddb1e42b1) ) // == 03
	ROM_LOAD16_BYTE( "ultr03.p4", 0x100001, 0x080000, CRC(8579127e) SHA1(5ba966008c0f7b4c910eddc6400ed2178c70e096) ) // == 03

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "ultr03ad.p1", 0x000000, 0x080000, CRC(94e18680) SHA1(80e7024464abff00a880c0f88f2b336e168ad025) )
	ROM_LOAD16_BYTE( "ultr03b.p1", 0x000000, 0x080000, CRC(8c66b958) SHA1(cd6a5c931bb432169ffe337ca6b2ffdd08d3fc5a) )
	ROM_LOAD16_BYTE( "ultr03bd.p1", 0x000000, 0x080000, CRC(07b7b114) SHA1(37035e750cc76b6e9e58c68e4e0bbbf201e48a8b) )
	ROM_LOAD16_BYTE( "ultr03d.p1", 0x000000, 0x080000, CRC(10907f74) SHA1(34f7e6b6f504816a4f055ad7faf8ffa9c88a6d2b) )
	ROM_LOAD16_BYTE( "ultr03dy.p1", 0x000000, 0x080000, CRC(b27c237b) SHA1(dc3a44e831243374fc9f9663489a277dea9bbd99) )
	ROM_LOAD16_BYTE( "ultr03k.p1", 0x000000, 0x080000, CRC(973a4942) SHA1(de1839d04fd1d7e8fb8a8ce75747084767786858) )
	ROM_LOAD16_BYTE( "ultr03y.p1", 0x000000, 0x080000, CRC(6d2d4e3a) SHA1(6b223415b70c8d372212ef88541ac921586cb46b) )
ROM_END

ROM_START( m5ultimo04 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ultr0_4s.p1", 0x000000, 0x080000, CRC(8d657946) SHA1(2bbb12ee9c30a7c53d0befc8ce55b006fedb7d57) )
	ROM_LOAD16_BYTE( "ultr0_4.p2", 0x000001, 0x080000, CRC(46b55954) SHA1(6cbfbb15ba3aafbd691e8f4a937a9a2a1c449072) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "ultr04ad.p1", 0x000000, 0x080000, CRC(f9770f88) SHA1(3ad37ceff574053b7054b47b33ed17c103bd627d) )
	ROM_LOAD16_BYTE( "ultr04bd.p1", 0x000000, 0x080000, CRC(b3485628) SHA1(3251a8a33037f9c26a53ec83cfaac544cd2dbc1b) )
	ROM_LOAD16_BYTE( "ultr04dy.p1", 0x000000, 0x080000, CRC(c0a52dd3) SHA1(9efd7daf47e906cc1063cadfbeeeca62a05e1e69) )
	ROM_LOAD16_BYTE( "ultr0_4b.p1", 0x000000, 0x080000, CRC(57ac885a) SHA1(cf5e677d11073a8a6ad7e53e55724afbe475bdaf) )
	ROM_LOAD16_BYTE( "ultr0_4d.p1", 0x000000, 0x080000, CRC(ac2aad1e) SHA1(28dbc41bc951346c7cdeb04df591b53967d02946) )
	ROM_LOAD16_BYTE( "ultr0_4k.p1", 0x000000, 0x080000, CRC(1c660fbb) SHA1(f01980a023c7c171f6b9b6e962fa7a34a17b45d1) )
	ROM_LOAD16_BYTE( "ultr0_4r.p1", 0x000000, 0x080000, CRC(e0ded0f6) SHA1(d0253768e4890a9c04519614248b8eae0fb43c73) )
	ROM_LOAD16_BYTE( "ultr0_4y.p1", 0x000000, 0x080000, CRC(e1eaf98b) SHA1(a3cbfe52deb737cf350b84f97c2261d216e6e127) )
ROM_END


ROM_START( m5upover )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "upov04ad.p1", 0x000000, 0x080000, CRC(e9d906ed) SHA1(acd3e880df445c1aa72cb2d1e0d432068e63482f) )
	ROM_LOAD16_BYTE( "upov04.p2", 0x000001, 0x080000, CRC(19a9c3de) SHA1(a51f85661394a9624bebba99330dd6dc86e0f76d) )
	ROM_LOAD16_BYTE( "upov04.p3", 0x100000, 0x080000, CRC(8484546f) SHA1(66172beb929c06f6bd58a06ba271426879cea820) )
	ROM_LOAD16_BYTE( "upov04.p4", 0x100001, 0x080000, CRC(457912e9) SHA1(4198ae81be577861310253ad32a56e65a4369ad3) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "upov04b.p1", 0x000000, 0x080000, CRC(7b56eca3) SHA1(44c8f3372601c7154d590d96b3064c8058a1b095) )
	ROM_LOAD16_BYTE( "upov04bd.p1", 0x000000, 0x080000, CRC(fe4b6f31) SHA1(1f652dfeb5e5df2b293b2cdc0bf92bf84a0d1420) )
	ROM_LOAD16_BYTE( "upov04d.p1", 0x000000, 0x080000, CRC(7e63fb58) SHA1(59c30334c75490bd85bc481f78a0165cf30b4072) )
	ROM_LOAD16_BYTE( "upov04dy.p1", 0x000000, 0x080000, CRC(6cd0e603) SHA1(6f45fa1dd2b47f6c994c89bc3430f82ad054f6ef) )
	ROM_LOAD16_BYTE( "upov04h.p1", 0x000000, 0x080000, CRC(d92cdcc9) SHA1(3ce851719382b15f8dd2185821bf45732d237a17) )
	ROM_LOAD16_BYTE( "upov04r.p1", 0x000000, 0x080000, CRC(d79a2e73) SHA1(08bd94f552431ab827afa31915d2684b201d7042) )
	ROM_LOAD16_BYTE( "upov04s.p1", 0x000000, 0x080000, CRC(7f47acb0) SHA1(b9ee1879c4be48c33559ca5debb27323a22bccd0) )
	ROM_LOAD16_BYTE( "upov04y.p1", 0x000000, 0x080000, CRC(6df4b1eb) SHA1(34cda403ffea53b2ef19400a0ceeffce2f96251f) )
ROM_END

ROM_START( m5upover15 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "upov15f.p1", 0x000000, 0x080000, CRC(f98d4f46) SHA1(0ce2a177d05de481115ba63397b8cce2785ac2ba) )
	ROM_LOAD16_BYTE( "upov15f.p2", 0x000001, 0x080000, NO_DUMP )
ROM_END


ROM_START( m5vampup )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "vampitupp1.bin", 0x000000, 0x080000, CRC(2c3e1a3b) SHA1(70ed775617e48bd0e12ba7391c86b95412ba4ac8) )
	ROM_LOAD16_BYTE( "vampitupp2.bin", 0x000001, 0x080000, CRC(d77e4e71) SHA1(2235b920499968c5b186f4ff4ce4cd8abfe3185d) )
	ROM_LOAD16_BYTE( "vampitupp3.bin", 0x100000, 0x080000, CRC(84df31b2) SHA1(83a10e4cab25a02108e18fcf705013b23a7c8aa4) )
	ROM_LOAD16_BYTE( "vampitupp4.bin", 0x100001, 0x080000, CRC(1529b3ef) SHA1(c5aa6eee1e6a6ef95d9a75a1aa92ca42bfc3f8d5) )
ROM_END

ROM_START( m5vertgo )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "vert0_5.p1", 0x000000, 0x080000, CRC(deba6573) SHA1(da9402991adb0fbc2ce7dfec744d355012bfdb76) )
	ROM_LOAD16_BYTE( "vert0_5.p2", 0x000001, 0x080000, CRC(a641ec03) SHA1(22ee2a6789a31b3aebe7608e5288cc33ec45b593) )
	ROM_LOAD16_BYTE( "vert0_5.p3", 0x100000, 0x080000, CRC(1a2e3a16) SHA1(2fcc74289d845ca2c66a68898ad84a5fac2ae064) )
	ROM_LOAD16_BYTE( "vert0_5.p4", 0x100001, 0x080000, CRC(182fa830) SHA1(804662c007813d46c7129be84a7b8941ac0589da) )
ROM_END

ROM_START( m5vertcl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cver0_5.p1", 0x000000, 0x080000, CRC(aefd3835) SHA1(83919a13101268c04f821612e39c88cb4501241d) )
	ROM_LOAD16_BYTE( "cver0_5.p2", 0x000001, 0x080000, CRC(31d1df01) SHA1(d1cc249dae236466b2357ea6d042d4b920bc557a) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "cver0_5d.p1", 0x000000, 0x080000, CRC(0fc6e802) SHA1(323bf91e9fd85f28805a5a60af63e4d1ac0d2a84) )
ROM_END

ROM_START( m5wking )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "witr02b.p1", 0x000000, 0x080000, CRC(704814ee) SHA1(febc1f2b1d354204c1a3565cf793792ffb3b1eb4) )
	ROM_LOAD16_BYTE( "witr02.p2", 0x000001, 0x080000, CRC(edf69b22) SHA1(1dfd03e54f8958dffbb575e7802e443654132d30) )
	ROM_LOAD16_BYTE( "witr02.p3", 0x100000, 0x080000, CRC(baaf43e4) SHA1(a15fcb8048295250b15805a067c84aecc55f241c) )
	ROM_LOAD16_BYTE( "witr02.p4", 0x100001, 0x080000, CRC(5df58764) SHA1(5d39afbe8d1614647a47e68978141ddd2e33a549) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "witr02ad.p1", 0x000000, 0x080000, CRC(1ce74a1f) SHA1(8092804f82d13e77ecf83a86cba3daa2da7dd86f) )
	ROM_LOAD16_BYTE( "witr02d.p1", 0x000000, 0x080000, CRC(0c29c42f) SHA1(6aed2541fd42c8e36c67ad498e9d8b5cdb774aae) )
	ROM_LOAD16_BYTE( "witr02dy.p1", 0x000000, 0x080000, CRC(fbeb51f2) SHA1(655a67529e5417a1a786c35d3749c099a602d2ab) )
	ROM_LOAD16_BYTE( "witr02k.p1", 0x000000, 0x080000, CRC(7e285967) SHA1(2d4f18af39d060834603fb83dcf6c4437d70f0dd) )
	ROM_LOAD16_BYTE( "witr02r.p1", 0x000000, 0x080000, CRC(0189c25d) SHA1(3d1eb417a1c26121160a2ef7f1f526208b07bd2d) )
	ROM_LOAD16_BYTE( "witr02s.p1", 0x000000, 0x080000, CRC(4586f27f) SHA1(2de55da99621edb2e25509a9c5ac20a5deea95ee) )
	ROM_LOAD16_BYTE( "witr02y.p1", 0x000000, 0x080000, CRC(b24467a2) SHA1(c592c296cd7a5e85bde73d57e6b07a5dd3aaad71) )
ROM_END

ROM_START( m5wking05 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "wild05.p1", 0x000000, 0x080000, CRC(990329d3) SHA1(7ddce7826614b95fbefb848d5c5b8d68d3977516) )
	ROM_LOAD16_BYTE( "wild05.p2", 0x000001, 0x080000, CRC(ed676a88) SHA1(3321edd3c4e1f04c0227aacdf430310c05c9afd4) )
	/* 3+4 */
ROM_END

ROM_START( m5wthing )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "wtc1_0.p1", 0x000000, 0x080000, CRC(4f64bf11) SHA1(781e06fbc083d4be70fadb48e7f67f4ce5c3730c) )
	ROM_LOAD16_BYTE( "wtc1_0.p2", 0x000001, 0x080000, CRC(258f8ee5) SHA1(0d52961b062c7d0600786be487d13dd568ab26cd) )
	ROM_LOAD16_BYTE( "wtc1_0.p3", 0x100000, 0x080000, CRC(b5e07ec4) SHA1(a49eca5d19d7f7591a91dd38ca49b5d7dafbf2cf) )
	ROM_LOAD16_BYTE( "wtc1_0.p4", 0x100001, 0x080000, CRC(435df5f6) SHA1(a5908514b7687180df31f882dc27510b439de59c) )
ROM_END

ROM_START( m5wthing11 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "wtc1_1.p1", 0x000000, 0x080000, CRC(243ae248) SHA1(a6520bf65d7522830318145175969d3c258073ac) )
	ROM_LOAD16_BYTE( "wtc1_1.p2", 0x000001, 0x080000, CRC(f7825c0c) SHA1(511a065dd8e507e825268dd5aa073c669968fbe3) )
	/* 3+4 */
ROM_END

ROM_START( m5wthing20 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "wtc2_0.p1", 0x000000, 0x080000, CRC(5b4ab64c) SHA1(92c8342b891d226725f26ec3952b5be2b23ead79) )
	ROM_LOAD16_BYTE( "wtc2_0.p2", 0x000001, 0x080000, CRC(68bb5fef) SHA1(11c7837e1bac5751c89d8094c9a6d9e31368d38b) )
	/* 3+4 */
ROM_END


ROM_START( m5xfact )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "xfac0_1.p1", 0x000000, 0x080000, CRC(232ad018) SHA1(ec5c5519dc134f3b03bf738d6cd82a6d4b9f04cf) )
	ROM_LOAD16_BYTE( "xfac0_1.p2", 0x000001, 0x080000, CRC(42093263) SHA1(b0e8542916e832f29a4032e2b0308229bba25bce) )
	ROM_LOAD16_BYTE( "xfac0_1.p3", 0x100000, 0x080000, CRC(d32dd45e) SHA1(57f43df56ea2813471d39e5131193dd681a5d5ba) )
	ROM_LOAD16_BYTE( "xfac0_1.p4", 0x100001, 0x080000, CRC(fd6469e9) SHA1(52eb3ab7b375b576070bbf2ab4c5e101e3879408) )
ROM_END

ROM_START( m5xfact02 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "xfac0_2.p1", 0x000000, 0x080000, CRC(ab2ddbe1) SHA1(39d65eb00aea8753db6964ec60715055962b7e39) )
	ROM_LOAD16_BYTE( "xfac0_2.p2", 0x000001, 0x080000, CRC(f941e22e) SHA1(fe7ba6a304aaeaf802df1aa2fd11e935da793b31) )
	ROM_LOAD16_BYTE( "xfac0_2.p3", 0x100000, 0x080000, CRC(d32dd45e) SHA1(57f43df56ea2813471d39e5131193dd681a5d5ba) )
	ROM_LOAD16_BYTE( "xfac0_2.p4", 0x100001, 0x080000, CRC(fd6469e9) SHA1(52eb3ab7b375b576070bbf2ab4c5e101e3879408) )
ROM_END

ROM_START( m5xfact04 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "xfac0_4.p1", 0x000000, 0x080000, CRC(2336356e) SHA1(4332fada68b55f9e70d6b059509ab82715ec1ac0) )
	ROM_LOAD16_BYTE( "xfac0_4.p2", 0x000001, 0x080000, CRC(bc2296d6) SHA1(d3fe3f806300e1de907288180a6af87e1ca75033) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "xfac0_4d.p1", 0x000000, 0x080000, CRC(61c805a5) SHA1(1465117ae0f9d7982db70bc1dec657cb3e1940fb) )
	ROM_LOAD16_BYTE( "xfac0_4m.p1", 0x000000, 0x080000, CRC(509d7101) SHA1(6ed398307758d8268d2c92c84b95137f2af0d493) )
ROM_END

ROM_START( m5xfact11 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "xfac1_1.p1", 0x000000, 0x080000, CRC(f3303449) SHA1(61284a5e950df86efc72aef436b81a80a8370380) )
	ROM_LOAD16_BYTE( "xfac1_1.p2", 0x000001, 0x080000, CRC(c1508630) SHA1(1ad9f38ddef32bc5a64db434c681b06967edd988) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "xfac1_1d.p1", 0x000000, 0x080000, CRC(ffbcb9f2) SHA1(a57760ff5b092d7550a5b2d69065782cbdd056e4) )
ROM_END



ROM_START( m5barxdx )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bard0_2.p1", 0x000000, 0x080000, CRC(5e0a1a2e) SHA1(3acaeb5234eea8080ec31669d5f5974b668cec1f) )
	ROM_LOAD16_BYTE( "bard0_2.p2", 0x000001, 0x080000, CRC(20d9dfec) SHA1(1aafbb1a7a9b06b93eb06e255756c7ac1f5cb335) )
	ROM_LOAD16_BYTE( "bard0_2.p3", 0x100000, 0x080000, CRC(dd28e140) SHA1(b5e330f78c57756760c7be14cfa35eab4a5cbb91) )
	ROM_LOAD16_BYTE( "bard0_2.p4", 0x100001, 0x080000, CRC(d6c5a6b1) SHA1(11d5fc1ece4980993fa741ef0c0c480d7b3e5103) )
ROM_END



ROM_START( m5cshstx )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cx__sjh1.1_1", 0x000000, 0x080000, CRC(049a6d79) SHA1(6efa6582183ba7e8a4df919bb43aac1d478e46ea) )
	ROM_LOAD16_BYTE( "cx__sjh1.1_2", 0x000001, 0x080000, CRC(eb68b3a7) SHA1(40863c254d2535f7869aaca3b56e74bb4a0230f8) )
	ROM_LOAD16_BYTE( "cx__sjh1.1_3", 0x100000, 0x080000, CRC(d07afe56) SHA1(a3b6bd0223e88c9c346f358851eb8ee25e0ee7a9) )
	ROM_LOAD16_BYTE( "cx__sjh1.1_4", 0x100001, 0x080000, CRC(7531047a) SHA1(76d222419be3c9879f78e0709638b8bb67f54fec) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "cx__sja1.1_1", 0x000000, 0x080000, CRC(a7e3acaa) SHA1(986abe5afccaee480291afd463494f7872b93a90) )
	ROM_LOAD16_BYTE( "cx__sja1.1_2", 0x000000, 0x080000, CRC(eb68b3a7) SHA1(40863c254d2535f7869aaca3b56e74bb4a0230f8) )
	ROM_LOAD16_BYTE( "cx__sjs1.1_1", 0x000000, 0x080000, CRC(6f4fc007) SHA1(fd8759c238dd367fd85e18f602be4d299dfd3106) )
	ROM_LOAD16_BYTE( "cx__sjs1.1_2", 0x000000, 0x080000, CRC(eb68b3a7) SHA1(40863c254d2535f7869aaca3b56e74bb4a0230f8) )
	ROM_LOAD16_BYTE( "cx__sjs1.1_3", 0x000000, 0x080000, CRC(d07afe56) SHA1(a3b6bd0223e88c9c346f358851eb8ee25e0ee7a9) )
	ROM_LOAD16_BYTE( "cx__sjs1.1_4", 0x000000, 0x080000, CRC(7531047a) SHA1(76d222419be3c9879f78e0709638b8bb67f54fec) )
ROM_END


ROM_START( m5circus )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bt_____0.5_1", 0x000000, 0x080000, CRC(2224349c) SHA1(72ccac03821dd29383169cd0bc3077c8f4793671) )
	ROM_LOAD16_BYTE( "bt_____0.5_2", 0x000001, 0x080000, CRC(c88a65db) SHA1(0d4a03b8fa88ceb79051e9a451a1d29e17c86577) )
ROM_END

ROM_START( m5circus0a )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "c3_sjb_0.a_1", 0x000000, 0x080000, CRC(2940f7b6) SHA1(7643850e80c0d6c84f9e44e3de31271ce8d41c8e) )
	ROM_LOAD16_BYTE( "c3_sjb_0.a_2", 0x000001, 0x080000, CRC(85625de6) SHA1(9e83532b2b4e677e12ecbd76f5a9b8c628c22e04) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "c3_sjbd0.a_1", 0x000000, 0x080000, CRC(0a9ddff2) SHA1(d82a4b62863bbf7a070920ccf15a9dc89d807b10) )
	ROM_LOAD16_BYTE( "c3_sjbd0.a_2", 0x000000, 0x080000, CRC(85625de6) SHA1(9e83532b2b4e677e12ecbd76f5a9b8c628c22e04) )
ROM_END

ROM_START( m5circus0b )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "c3_sjbd0.b_1", 0x000000, 0x080000, CRC(799019bf) SHA1(3cc0ed664474bc8b38f764c1f4ce2b7e7bf835de) )
	ROM_LOAD16_BYTE( "c3_sjbd0.b_2", 0x000001, 0x080000, CRC(b06d66cd) SHA1(234134f2453585be75868532d3630fb544458773) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "c3_sjb_0.b_1", 0x000000, 0x080000, CRC(91085aca) SHA1(3ada1ceeaaef4fbfe2c202771a9bff532d1ebc3c) )
	ROM_LOAD16_BYTE( "c3_sjb_0.b_2", 0x000000, 0x080000, CRC(b06d66cd) SHA1(234134f2453585be75868532d3630fb544458773) )
ROM_END

ROM_START( m5circus20 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "c3_sjb_2.0_1", 0x000000, 0x080000, CRC(498c8e95) SHA1(a1bd6fbe11cd20d71e807963ee3d61387ec0a0e7) )
	ROM_LOAD16_BYTE( "c3_sjb_2.0_2", 0x000001, 0x080000, CRC(d0bcb8f4) SHA1(f0d9de495dd0a1eae3496322668260804904bf63) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "c3_sjb_2.0u1", 0x000000, 0x080000, CRC(6cbe4255) SHA1(84c6079e5324c6a281573a6eaa8d86c1d2bb9318) )
	ROM_LOAD16_BYTE( "c3_sjb_2.0u2", 0x000000, 0x080000, CRC(5bd3fe89) SHA1(e85a7833ee2d275e19f520f3b261cbbe52d0abe7) )
	ROM_LOAD16_BYTE( "c3_sjbd2.0_1", 0x000000, 0x080000, CRC(0136f1ea) SHA1(f8fd152e1cb68c60b5a49256a58f97ab8cc6f322) )
	ROM_LOAD16_BYTE( "c3_sjbd2.0_2", 0x000000, 0x080000, CRC(d0bcb8f4) SHA1(f0d9de495dd0a1eae3496322668260804904bf63) )
	ROM_LOAD16_BYTE( "c3_sjbd2.0u1", 0x000000, 0x080000, CRC(741f3ccd) SHA1(b7845737fcab4fe9cd657a84d8636207b42201c8) )
	ROM_LOAD16_BYTE( "c3_sjbd2.0u2", 0x000000, 0x080000, CRC(5bd3fe89) SHA1(e85a7833ee2d275e19f520f3b261cbbe52d0abe7) )
	ROM_LOAD16_BYTE( "c3_sjbg2.0_1", 0x000000, 0x080000, CRC(848ff63b) SHA1(2198883296d239bf66b2204a191ef3d8284c0efd) )
	ROM_LOAD16_BYTE( "c3_sjbg2.0_2", 0x000000, 0x080000, CRC(7588f595) SHA1(23c4c611990fdf994d4131dc0b8cd5d959f25aed) )
	ROM_LOAD16_BYTE( "c3_sjbg2.0u1", 0x000000, 0x080000, CRC(73a3dcff) SHA1(c0f5c6a217449e0b2e795a99a1e1359c5457301b) )
	ROM_LOAD16_BYTE( "c3_sjbg2.0u2", 0x000000, 0x080000, CRC(7e261bd9) SHA1(03fed6a8e4cf37f3e42783808946324efc002dce) )
ROM_END

ROM_START( m5circus21 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "c3_sjb_2.1_1", 0x000000, 0x080000, CRC(5f86dce6) SHA1(1b986f720bd735d4d372894fc23e1e723464d5c3) )
	ROM_LOAD16_BYTE( "c3_sjb_2.1_2", 0x000001, 0x080000, CRC(ea3613ef) SHA1(10286ff17936a1d26dc709a92019d573aabfb56b) )
ROM_END

ROM_START( m5circus11 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "c3_sjbg1.1_1", 0x000000, 0x080000, CRC(667ba9d6) SHA1(ef0866f1c25f677d5d862157066c6e956e79369f) )
	ROM_LOAD16_BYTE( "c3_sjbg1.1_2", 0x000001, 0x080000, CRC(2a2aa3f3) SHA1(08ffe24a4f620b5e56926ea4dc33fcbf526bd5de) )
ROM_END


ROM_START( m5circlb )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rc5__cd1.1_1", 0x000000, 0x080000, CRC(ae9cabd1) SHA1(ea1eaf63d595ab5e0037e829f6e0f13ecdebeaf9) )
	ROM_LOAD16_BYTE( "rc5__cd1.1_2", 0x000001, 0x080000, CRC(6d935cb8) SHA1(216ff5edacd991a10d878035eb6d614e2542e197) )
	ROM_LOAD16_BYTE( "rc5__cd1.1_3", 0x100000, 0x080000, CRC(ba1b771e) SHA1(15eed53ab2e35a2e5e855fc35abf8592c341e6ec) )
	ROM_LOAD16_BYTE( "rc5__cd1.1_4", 0x100001, 0x080000, CRC(cab3a69b) SHA1(5866d8a1328b38f9ea8d596f84663b65a3b12ead) )
ROM_END

ROM_START( m5circlb00 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "b5_____0.0_1", 0x000000, 0x080000, CRC(fac176e7) SHA1(b822b7e80bbc01888894dfb9cc02c5ae314d2296) )
	ROM_LOAD16_BYTE( "b5_____0.0_2", 0x000001, 0x080000, CRC(2256faa5) SHA1(697ae710c9739967ff7bc7855cd9bc9a14724fbc) )
	// 3+4 ?
ROM_END

ROM_START( m5circlb15 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "c5_____1.5_1", 0x000000, 0x080000, CRC(5a93e73a) SHA1(7d32b67e9b8cd4f9bf2952c367b27fbcc247651d) )
	ROM_LOAD16_BYTE( "c5_____1.5_2", 0x000001, 0x080000, CRC(441efbbc) SHA1(145d93598d714195664f758bd7d4b10b0954fb89) )
	// 3+4?

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "c5___cd1.5_1", 0x000000, 0x080000, CRC(a1c9b023) SHA1(952587533b1ee1404df5c7ce308ba24ccd355722) )
	ROM_LOAD16_BYTE( "c5___cd1.5_2", 0x000000, 0x080000, CRC(441efbbc) SHA1(145d93598d714195664f758bd7d4b10b0954fb89) )
ROM_END

ROM_START( m5circlb33 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rc5____3.3_1", 0x000000, 0x080000, CRC(bd127d6a) SHA1(0157a2ed786c80024c2559c781a13fc3eb7360ee) )
	ROM_LOAD16_BYTE( "rc5____3.3_2", 0x000001, 0x080000, CRC(9f0dec0a) SHA1(13a58545b71dc1a495abd8ec8630ae9136907af0) )
	// 3+4?

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "rc5___d3.3_1", 0x000000, 0x080000, CRC(a19ff513) SHA1(2c459a5ccee75be60e935a12418869d1dfbe6d84) )
	ROM_LOAD16_BYTE( "rc5___d3.3_2", 0x000000, 0x080000, CRC(9f0dec0a) SHA1(13a58545b71dc1a495abd8ec8630ae9136907af0) )
	ROM_LOAD16_BYTE( "rc5__c_3.3_1", 0x000000, 0x080000, CRC(c06c3759) SHA1(f6496ca3734eb4fc92ade7c0397799bfb4612a37) )
	ROM_LOAD16_BYTE( "rc5__c_3.3_2", 0x000000, 0x080000, CRC(9f0dec0a) SHA1(13a58545b71dc1a495abd8ec8630ae9136907af0) )
	ROM_LOAD16_BYTE( "rc5__cd3.3_1", 0x000000, 0x080000, CRC(dce1bf20) SHA1(42a82869f208ba18b70fce6b60447eedf9bf3003) )
	ROM_LOAD16_BYTE( "rc5__cd3.3_2", 0x000000, 0x080000, CRC(9f0dec0a) SHA1(13a58545b71dc1a495abd8ec8630ae9136907af0) )
	ROM_LOAD16_BYTE( "rc5_hc_3.3_1", 0x000000, 0x080000, CRC(a36b4d24) SHA1(73042d01f9beb53cc8d403d32d55f80e5de53b06) )
	ROM_LOAD16_BYTE( "rc5_hc_3.3_2", 0x000000, 0x080000, CRC(9f0dec0a) SHA1(13a58545b71dc1a495abd8ec8630ae9136907af0) )
	ROM_LOAD16_BYTE( "rc5_hcd3.3_1", 0x000000, 0x080000, CRC(bfe6c55d) SHA1(6d004ae0b67986852157e9e96f51b987518838ee) )
	ROM_LOAD16_BYTE( "rc5_hcd3.3_2", 0x000000, 0x080000, CRC(9f0dec0a) SHA1(13a58545b71dc1a495abd8ec8630ae9136907af0) )
ROM_END




ROM_START( m5clown )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "carsj__1.2_1", 0x000000, 0x080000, CRC(9eb78643) SHA1(cbfe1f3d44dcbf89286c204b7126246640606d58) )
	ROM_LOAD16_BYTE( "carsj__1.2_2", 0x000001, 0x080000, CRC(3bc97219) SHA1(095f8415b22b427a4f07296ed5f39656d8da7d09) )
	ROM_LOAD16_BYTE( "carsj__1.2_3", 0x100000, 0x080000, CRC(a750bd3b) SHA1(3578abc707ce03b28206806923ef095a7706a6e6) )
	ROM_LOAD16_BYTE( "carsj__1.2_4", 0x100001, 0x080000, CRC(6425e43f) SHA1(93ec81f226c014e22fe923fc985e04a77d9574a0) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "carsj_d1.2_1", 0x000000, 0x080000, CRC(c27da90b) SHA1(fa139bec5ff283e307ec6a18e0c6af70f40d6f6c) )
	ROM_LOAD16_BYTE( "carsj_d1.2_2", 0x000000, 0x080000, CRC(3bc97219) SHA1(095f8415b22b427a4f07296ed5f39656d8da7d09) )
ROM_END

ROM_START( m5clown11 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "carsj__1.1_1", 0x000000, 0x080000, CRC(d492fcb8) SHA1(0242c3b0ccbb56e0d9654cd8b22c24da1e34504f) )
	ROM_LOAD16_BYTE( "carsj__1.1_2", 0x000001, 0x080000, CRC(033c8a94) SHA1(e5385dd848b06140bf639aee9ed13976992ef705) )
	/* 3+4 */
ROM_END

ROM_START( m5clown13 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "carsj__1.3_1", 0x000000, 0x080000, CRC(0f9cc548) SHA1(4a65e31c38dd2fe1d58e2effd91ac60020ec9dc5) )
	ROM_LOAD16_BYTE( "carsj__1.3_2", 0x000001, 0x080000, CRC(7dcae3bb) SHA1(fb72ac108a936fa762a6ff20167381e52f4b8ab6) )
	/* 3+4 */
ROM_END



ROM_START( m5clubsn )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "csa__1.5_1", 0x000000, 0x080000, CRC(30838b8f) SHA1(0737e173236f0dfd9d818dcb716a5867407bcd36) )
	ROM_LOAD16_BYTE( "csa__1.5_2", 0x000001, 0x080000, CRC(92e42fbd) SHA1(29bc12682ee1c9834d4366c17d5dd92532d0f82f) )
	ROM_LOAD16_BYTE( "csa__1.5_3", 0x100000, 0x080000, CRC(e2440819) SHA1(9f19f972d9d9489bdeec8227d5ee33c4e54de58d) )
	ROM_LOAD16_BYTE( "csa__1.5_4", 0x100001, 0x080000, CRC(6c3a52fb) SHA1(228e74fba3fc024b1065fb6ac7c0b545cdfd039d) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "csa__1.5d1", 0x000000, 0x080000, CRC(a6d05ff0) SHA1(56e247a8246271e591d62e57d13dad4e33a3aa77) )
ROM_END

ROM_START( m5clubsn11 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "csa___1.1_1", 0x000000, 0x080000, CRC(a5e7a44f) SHA1(c4ff6f21bb134410232880bfd6eac60bc90f685b) )
	ROM_LOAD16_BYTE( "csa___1.1_2", 0x000001, 0x080000, CRC(8a47f58a) SHA1(b636be7fa22d6a595267083c9df6e14e1656ec0c) )
	/* 3+4 */
ROM_END

ROM_START( m5clubsn14 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "csa___1.4_1", 0x000000, 0x080000, CRC(3b5ef795) SHA1(19cfc7c0bac12bac3feea698ec24c6e7d630e0f7) )
	ROM_LOAD16_BYTE( "csa___1.4_2", 0x000001, 0x080000, CRC(30027dde) SHA1(756dafa60c59e436ebafd465e89c7498dfb71da2) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "csa___1.4d1", 0x000000, 0x080000, CRC(3761f7df) SHA1(eaa755ae4938a2bd36ab4e77eacf3df511e1235d) )
ROM_END

ROM_START( m5clubsn16 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "csa___1.6_1", 0x000000, 0x080000, CRC(50322a9f) SHA1(373f58f75d496a46c82a45fa77040aec2f27ce6d) )
	ROM_LOAD16_BYTE( "csa___1.6_2", 0x000001, 0x080000, CRC(ddab9236) SHA1(945baf8efd93e9771de038c75b1bacee49252b1d) )
	ROM_LOAD16_BYTE( "csa___1.6_3", 0x100000, 0x080000, CRC(e2440819) SHA1(9f19f972d9d9489bdeec8227d5ee33c4e54de58d) ) // == 1.5
	ROM_LOAD16_BYTE( "csa___1.6_4", 0x100001, 0x080000, CRC(6c3a52fb) SHA1(228e74fba3fc024b1065fb6ac7c0b545cdfd039d) ) // == 1.5

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "csa___1.6d1", 0x000000, 0x080000, CRC(24134058) SHA1(4d5613608f4719ff3fc3509da8f85d99ef36e3bf) )
ROM_END




ROM_START( m5dick )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "dtusj__1.1_1", 0x000000, 0x080000, CRC(450bc407) SHA1(c1d876f9ad6fff7b241a3b00bca8c2c1d0442691) )
	ROM_LOAD16_BYTE( "dtusj__1.1_2", 0x000001, 0x080000, CRC(32d49a83) SHA1(1e5ede1f23022b18f22210e5c34dcdcaaba40853) )
	ROM_LOAD16_BYTE( "dtusj__1.1_3", 0x100000, 0x080000, CRC(0bc78951) SHA1(5f65eea2f2c19ea9bcbc46422f1be17489614179) )
	ROM_LOAD16_BYTE( "dtusj__1.1_4", 0x100001, 0x080000, CRC(cdd5d8d7) SHA1(9c9487a8af3f8ecd44c9fe51c18f180d99db14e7) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "dtusj_d1.1_1", 0x000000, 0x080000, CRC(d9439742) SHA1(d543680610df1c830b07fa883a3285f3d770511c) )
	ROM_LOAD16_BYTE( "dtusj_d1.1_2", 0x000000, 0x080000, CRC(32d49a83) SHA1(1e5ede1f23022b18f22210e5c34dcdcaaba40853) )
ROM_END


ROM_START( m5dick10 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "dtusj__1.0_1", 0x000000, 0x080000, CRC(9c7f12cd) SHA1(794bc08356d3a7c706234b229a334fc58e1a66ea) )
	ROM_LOAD16_BYTE( "dtusj__1.0_2", 0x000001, 0x080000, CRC(dd86e819) SHA1(d9ce12be8a42319bcb7ee4b3be2e7d0153e5858e) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "dtusj_d1.0_1", 0x000000, 0x080000, CRC(676c9f5a) SHA1(cf5cf2f69133aeddd9a27c32d6b966597b71f445) )
	ROM_LOAD16_BYTE( "dtusj_d1.0_2", 0x000000, 0x080000, CRC(dd86e819) SHA1(d9ce12be8a42319bcb7ee4b3be2e7d0153e5858e) )
ROM_END


ROM_START( m5donna ) // donna_kebab_(bwb)_[dx01_1280_25jp].hex
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "dkesj__0.6_1", 0x000000, 0x080000, CRC(287f43da) SHA1(5303f6a33ccca29164a20f808737346193e784fd) )
	ROM_LOAD16_BYTE( "dkesj__0.6_2", 0x000001, 0x080000, CRC(8d191415) SHA1(1f0d45c3af59e475a9573419c355ae9e4e240964) )

	ROM_REGION( 0x400000, "altrevs", 0 ) // or is this king kebab?
	ROM_LOAD( "kebab.p1", 0x0000, 0x080000, CRC(bbcad7da) SHA1(5c090d5d5224fd45660ec03ebf46f70cc6bb2c91) )
	ROM_LOAD( "kebab.p2", 0x0000, 0x080000, CRC(43496ef1) SHA1(f7b4721dd5c9c388c3c81e43cb51bd36c29bff86) )
	ROM_LOAD( "kebeb.g1", 0x0000, 0x080000, CRC(0d92a46a) SHA1(7b5870fe4ca71ef17cb227b66e617606d5351eb5) )
ROM_END

ROM_START( m5donnad )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "dkesj_d0.6_1", 0x000000, 0x080000, CRC(460f903f) SHA1(895a64ee237da75b332cdd63fc0bc6c67b64dda1) )
	ROM_LOAD16_BYTE( "dkesj__0.6_2", 0x000001, 0x080000, CRC(8d191415) SHA1(1f0d45c3af59e475a9573419c355ae9e4e240964) )
ROM_END

ROM_START( m5donnaa )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "dkesj__0.5_1", 0x000000, 0x080000, CRC(456b7fa7) SHA1(a5f0d1a57c11e6e03141d40604fc2d6bf7897e31) )
	ROM_LOAD16_BYTE( "dkesj__0.5_2", 0x000001, 0x080000, CRC(e7199c08) SHA1(23a43b5aaebd28cc7c252ee099e79a7c3bf7dbe7) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "dkesj_d0.5_1", 0x000000, 0x080000, CRC(dec3c94d) SHA1(65c0a8696004d57ab82d92ecfbc3415e91acbdf7) )
	ROM_LOAD16_BYTE( "dkesj_d0.5_2", 0x000000, 0x080000, CRC(e7199c08) SHA1(23a43b5aaebd28cc7c252ee099e79a7c3bf7dbe7) )
ROM_END



ROM_START( m5dblqts ) // double_or_quits_(bwb)_[c01_1024_15jp].hex
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "m5dblqt_p1", 0x000000, 0x080000, CRC(a65119bb) SHA1(f28905e4a3b4b95295634cdf36d5592178a51f50) )
	ROM_LOAD16_BYTE( "m5dblqt_p2", 0x000001, 0x080000, CRC(4f1327b0) SHA1(73226112c0d8c24e7cb6cb5286bfef2aa30ee5c6) )
	ROM_LOAD16_BYTE( "m5dblqt_p3", 0x100000, 0x080000, CRC(8d516e58) SHA1(c6e40c28a9e7c387b4341609d09bc6612575dedc) )
	ROM_LOAD16_BYTE( "m5dblqt_p4", 0x100001, 0x080000, CRC(b0552839) SHA1(0251350a7fbf6e0677c6c2e50bac49231b7a064a) )
ROM_END


ROM_START( m5dblqtsa )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "do__sjs1.9d1", 0x000000, 0x080000, CRC(0d003ae8) SHA1(eeb4bea7bf81bde9cdd9689849a0ae1f1f300ca7) )
	ROM_LOAD16_BYTE( "do__sjs1.9d2", 0x000001, 0x080000, CRC(bf464bb1) SHA1(915edcd4de1c5cc7c2b47d0836f9129a40cf8c6f) )
	/* 3+4? */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "do_sjs1.p1", 0x000000, 0x080000, CRC(489eb298) SHA1(382cc0766444d8f6d4429416f211e2dfde8873f1) )
	ROM_LOAD16_BYTE( "do_sjs1.p2", 0x000000, 0x080000, CRC(bf464bb1) SHA1(915edcd4de1c5cc7c2b47d0836f9129a40cf8c6f) )
ROM_END

ROM_START( m5dblqtsb )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "d_quit5_.p1", 0x00000, 0x080000, CRC(f7875ba4) SHA1(8124e5f90c715cf0740dfabb3382e3e4e2d3e04e) )
	ROM_LOAD16_BYTE( "d_quit5_.p2", 0x00001, 0x080000, CRC(ea12e037) SHA1(155ff54adcb14505fc35fb2d7a356beef5e548f3) )
	/* 3+4? */
ROM_END

ROM_START( m5dblqts1b )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "do_sjs1.b_1", 0x000000, 0x080000, CRC(475a9893) SHA1(025262e5b74de68e248875d602e987885fafc876) )
	ROM_LOAD16_BYTE( "do_sjs1.b_2", 0x000001, 0x080000, CRC(381599db) SHA1(f7e9d2ca6ca4d20f31892f91211134a610b37b40) )
	/* 3+4? */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "do_sjs1.bd1", 0x000000, 0x080000, CRC(a6ad800b) SHA1(5c4f4ce829eaabb584bf3f897387d0f08ecc9c7f) )
	ROM_LOAD16_BYTE( "do_sjs1.bd2", 0x000000, 0x080000, CRC(381599db) SHA1(f7e9d2ca6ca4d20f31892f91211134a610b37b40) )
ROM_END







ROM_START( m5hgl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hlusj__1.5_1", 0x000000, 0x080000, CRC(b7f513ef) SHA1(862bc98d72a18a35056a273bc135330aa97e542c) )
	ROM_LOAD16_BYTE( "hlusj__1.5_2", 0x000001, 0x080000, CRC(ca0117ad) SHA1(3ec6659da9ab78cb79c8938b35efed45ba727552) )
	ROM_LOAD16_BYTE( "hlusj__1.5_3", 0x100000, 0x080000, CRC(3e6bb0f9) SHA1(6ed3b2baa5ba9312a249aee30bdd22ef8e824049) )
	ROM_LOAD16_BYTE( "hlusj__1.5_4", 0x100001, 0x080000, CRC(1b19d681) SHA1(3794f9b24fa685c1e81fb38b2109ecde7c55f917) )
ROM_END

ROM_START( m5hgl16 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hlusj__1.6_1", 0x000000, 0x080000, CRC(dc6daea5) SHA1(1b19086cfde56212adff20b92cc299bbe2b03b50) )
	ROM_LOAD16_BYTE( "hlusj__1.6_2", 0x000001, 0x080000, CRC(72a19dbc) SHA1(992bcdc770640f45014a7b1e2014e5864e9e7bac) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "hlusj_d1.6_1", 0x000000, 0x080000, CRC(7fdc38aa) SHA1(8382c750f0c3bebcaf56c75b01e26ab8e508e223) )
	ROM_LOAD16_BYTE( "hlusj_d1.6_2", 0x000000, 0x080000, CRC(72a19dbc) SHA1(992bcdc770640f45014a7b1e2014e5864e9e7bac) )
ROM_END

ROM_START( m5hgl14 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hlusj__1.4_1", 0x000000, 0x080000, CRC(5b748aab) SHA1(80e4c5082f515dd6d29023b0e11426c829ec7147) )
	ROM_LOAD16_BYTE( "hlusj__1.4_2", 0x000001, 0x080000, CRC(24792a34) SHA1(c35ff5d078fd9bcd30a318a8080a681dffdf9c29) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "hlusj_d1.4_1", 0x000000, 0x080000, CRC(8f5d5c11) SHA1(ad184f572a537ebdd1da868cfad99ce0ea7e672a) )
	ROM_LOAD16_BYTE( "hlusj_d1.4_2", 0x000000, 0x080000, CRC(24792a34) SHA1(c35ff5d078fd9bcd30a318a8080a681dffdf9c29) )
ROM_END


ROM_START( m5carpet )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mg_sj__1.1_1", 0x000000, 0x080000, CRC(555fa205) SHA1(0820738a347034ce3e63eedcf671b8660112fa3e) )
	ROM_LOAD16_BYTE( "mg_sj__1.1_2", 0x000001, 0x080000, CRC(5cd37b31) SHA1(024a274f543d71cde316432f49f66f26d3cce2c5) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "mg_sj_b1.1_1", 0x000000, 0x080000, CRC(8104f194) SHA1(7c9c455464c7220763d89bcd74e7fac8207e7c5f) )
	ROM_LOAD16_BYTE( "mg_sj_b1.1_2", 0x000000, 0x080000, CRC(5cd37b31) SHA1(024a274f543d71cde316432f49f66f26d3cce2c5) )
ROM_END

ROM_START( m5carpet12 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mg_sj__1.2_1", 0x000000, 0x080000, CRC(680a622e) SHA1(48d150674ab63c9ce1354f7ff306f2373aec2708) )
	ROM_LOAD16_BYTE( "mg_sj__1.2_2", 0x000001, 0x080000, CRC(2bc5582a) SHA1(0439e6f2172b5affb5cc1bfc3ef90ed8bc7fdda0) )
ROM_END



ROM_START( m5clr )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mpu5clr.p1", 0x000000, 0x080000, CRC(db6406f5) SHA1(ee82fefda1d9c6294d6bad15dd00ed8d6857da7f) )
	ROM_LOAD16_BYTE( "mpu5clr.p2", 0x000001, 0x080000, CRC(ba9ec6e6) SHA1(5ddc4d9b7d3cbcda44e1440db9d2619c93143818) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "clr4", 0x000000, 0x002000, CRC(880ee483) SHA1(c40486c8e2b1d816034440c5c50ec11d90a9df71) ) // surely this is an MPU4 rom??
ROM_END

ROM_START( m5tst )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "test05.p1", 0x000000, 0x080000, CRC(d551ff40) SHA1(a0bb341a2ebd6083e80b4a4b5c8c4cd434417b5d) )
	ROM_LOAD16_BYTE( "test05.p2", 0x000001, 0x080000, CRC(c9d4c4a9) SHA1(8c86496a4e8e958a120a82b42ed3052891bfabcf) )
ROM_END


ROM_START( m5ppussy )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pusy0_9.p1", 0x000000, 0x080000, CRC(ee582479) SHA1(f8522d8b2000ed0f2b16fa10a0d40c5be7d54ffd) )
	ROM_LOAD16_BYTE( "pusy0_9.p2", 0x000001, 0x080000, CRC(13ffb062) SHA1(48df71cfb91750c1e471d49dae0c3062985e1c03) )
	ROM_LOAD16_BYTE( "pusy0_9.p3", 0x100000, 0x080000, CRC(09ac29d9) SHA1(ecc0a0ac21df46453b6b21228fa5afb4cbaa5fb7) )
	ROM_LOAD16_BYTE( "pusy0_9.p4", 0x100001, 0x080000, CRC(efa983b8) SHA1(833db5f2f87d7c667c83568ee06e6af0aec73d12) )
ROM_END

ROM_START( m5showtm )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "top04.p1", 0x000000, 0x080000, CRC(ef3512be) SHA1(d7c0b0d1f6c04b4ae6a4b59ce0b3ab9a843aeb52) )
	ROM_LOAD16_BYTE( "top04.p2", 0x000001, 0x080000, CRC(a8310e11) SHA1(9e4c22cf47762056b38634a8b957cdd22faaf327) )
	ROM_LOAD16_BYTE( "top04.p3", 0x100000, 0x080000, CRC(68faa07f) SHA1(a6ceeac33bb495ab2546c4fa1dddb25230547a89) )
	ROM_LOAD16_BYTE( "top04.p4", 0x100001, 0x080000, CRC(6801cbab) SHA1(dd6f9bca48b36911f19ce1e781d7bc9209bdb8fc) )
ROM_END

/* Converted from .hex files */

ROM_START( m5barmy )  // barmy_army_(barcrest)_[c01_800_5jp].hex
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "m5barmy_p1", 0x000000, 0x080000, CRC(b5f6e46e) SHA1(0e0fb5e65bd04d6e672ccce39362e999f81b41bb) )
	ROM_LOAD16_BYTE( "m5barmy_p2", 0x000001, 0x080000, CRC(08e5817f) SHA1(c6b34b5894f2cd9f9663663eab6214b8f9935be1) )
	ROM_LOAD16_BYTE( "m5barmy_p3", 0x100000, 0x080000, CRC(2b9931b6) SHA1(39b62751c37cd31c89aafba4763a38779cc0998c) )
	ROM_LOAD16_BYTE( "m5barmy_p4", 0x100001, 0x080000, CRC(48a6983a) SHA1(7cc98d6be38da1b0b775903ae217428bcbbee2be) )
ROM_END

ROM_START( m5beans ) // full_of_beans_(barcrest)_[c01_800_25jp].hex
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "m5beans_p1", 0x000000, 0x080000, CRC(a285ecf3) SHA1(68618c47370ef7028a691be60737990fdaf9ba69) )
	ROM_LOAD16_BYTE( "m5beans_p2", 0x000001, 0x080000, CRC(f43d445d) SHA1(a8c30397aac4572161d895b82e0b73138370ea7f) )
	ROM_LOAD16_BYTE( "m5beans_p3", 0x100000, 0x080000, CRC(30a824de) SHA1(19289f4ecc28b52842c49f3ab4d657c550f17044) )
	ROM_LOAD16_BYTE( "m5beans_p4", 0x100001, 0x080000, CRC(f3d6513c) SHA1(641a3e2ad31a8a5626fe71ded03dd0ce51c2b506) )
ROM_END

ROM_START( m5beansa )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fbean.p1", 0x00000, 0x080000, CRC(32a3aeb9) SHA1(1cb8ce5dc1011422c3109ad09b4a1ef6a17a0711) )
	ROM_LOAD16_BYTE( "fbean.p2", 0x00001, 0x080000, CRC(94dffb0a) SHA1(600b54233b45d95a89030673887037b77d0c6e84) )
	/* 3+4? */
ROM_END

ROM_START( m5bling ) // bling_king_crazy_(barcrest)_[c01_800_25jp].hex
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "m5bling_p1", 0x000000, 0x080000, CRC(83383741) SHA1(2981b1a3d712d595dfe9e10b5bb962777d6c5278) )
	ROM_LOAD16_BYTE( "m5bling_p2", 0x000001, 0x080000, CRC(3d2132c4) SHA1(c3b35f19a2a715c125b5459895c3fc2ce60f6b00) )
	ROM_LOAD16_BYTE( "m5bling_p3", 0x100000, 0x080000, CRC(6faa0731) SHA1(3eb0606c90a3f3b5b404875661d820254443f2a8) )
	ROM_LOAD16_BYTE( "m5bling_p4", 0x100001, 0x080000, CRC(7ee13f10) SHA1(c8a7a58983100b2d8bd428b964e5308bed0f3a2d) )
ROM_END

ROM_START( m5card ) // card_shark_(vivid)_[c01_1024_15jp].hex
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "m5card_p1", 0x000000, 0x080000, CRC(fe125513) SHA1(e1fa24b59057de2ae4ee9cf66c201ae3611bec9b) )
	ROM_LOAD16_BYTE( "m5card_p2", 0x000001, 0x080000, CRC(9fe9cc38) SHA1(578efa9d0597e65ed1795646bf1c9db8309524ed) )
	ROM_LOAD16_BYTE( "m5card_p3", 0x100000, 0x080000, CRC(f3b20a1c) SHA1(e489d6fe215e1e132043246aa814262a3994c91e) )
	ROM_LOAD16_BYTE( "m5card_p4", 0x100001, 0x080000, CRC(549491a7) SHA1(81cbbf1692ba5cf47dcae1b80db38a39e3e01894) )

	ROM_REGION( 0x400000, "altrevs", 0 )
	ROM_LOAD( "cshsjs2.0d1", 0x0000, 0x080000, CRC(33d77c0c) SHA1(9de0a6f24365383b63cbde6a6850565fe246a200) )
ROM_END



ROM_START( m5devil ) // devil_of_a_deal_(vivid)_[c01_800_25jp].hex
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "m5devil_p1", 0x000000, 0x080000, CRC(48faabe3) SHA1(e9acce4675f2d2a49e02e7381194ca68ab7eee09) )
	ROM_LOAD16_BYTE( "m5devil_p2", 0x000001, 0x080000, CRC(0e6b4e3c) SHA1(d36d7cb422fcd3c3818bed7359fd4f180489bd2c) )
	ROM_LOAD16_BYTE( "m5devil_p3", 0x100000, 0x080000, CRC(9c4d636f) SHA1(dc4d22242f0d7bf1309736ceee04cc927649dcb4) )
	ROM_LOAD16_BYTE( "m5devil_p4", 0x100001, 0x080000, CRC(8aeb16fc) SHA1(13eeaaa4abf69aa2ada32423d6cc6441de581fd0) )

	ROM_REGION( 0x400000, "altrevs", 0 )
	ROM_LOAD( "devil_de.p1", 0x0000, 0x080000, CRC(0b8408f7) SHA1(7460a4619fe2217e99d360121c45eb680a3a3e3a) )
	ROM_LOAD( "deal2.2p1", 0x0000, 0x080000, CRC(e1f2dc1d) SHA1(629d82e2bb9e1c6a1bc961e54edea87d070b93cd) )
ROM_END









ROM_START( m5fire ) // all_fired_up_(barcrest)_[c01_800_25jp].hex
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "m5fire_p1", 0x000000, 0x080000, CRC(5c4a22a2) SHA1(affc41ca73d619dc4cf0542d4d71283be817672f) )
	ROM_LOAD16_BYTE( "m5fire_p2", 0x000001, 0x080000, CRC(564a6247) SHA1(a9e39544270280cbf9a80fa2d8e5ccb9f39367a8) )
	ROM_LOAD16_BYTE( "m5fire_p3", 0x100000, 0x080000, CRC(7e2dbbe1) SHA1(6d9283e21db0b56fa8c123088a52f654af086bf8) )
	ROM_LOAD16_BYTE( "m5fire_p4", 0x100001, 0x080000, CRC(a95bc24d) SHA1(40d247111728249b78f4660971bfbeab787f3b9e) )
ROM_END

ROM_START( m5hilok ) // hi_lo_karate_(vivid)_[c01_800_25jp].hex
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "m5hilok_p1", 0x000000, 0x080000, CRC(99a50cd5) SHA1(eef619fa66b6558ce49b3ca8e62e3b22c43465d7) )
	ROM_LOAD16_BYTE( "m5hilok_p2", 0x000001, 0x080000, CRC(64edceb7) SHA1(0a0a6383c74fb0288bd2d40e21adf0666b47b931) )
	ROM_LOAD16_BYTE( "m5hilok_p3", 0x100000, 0x080000, CRC(ee3c30f7) SHA1(a69858bcf33f3882e398dd9f60c00e03747489ac) )
	ROM_LOAD16_BYTE( "m5hilok_p4", 0x100001, 0x080000, CRC(b61eb919) SHA1(73e1ecb4b78d11a086711dbb0f485d06d0d523b0) )
ROM_END

ROM_START( m5jcy ) // juicy_fruits_(empire)_[dx01_1280_25jp].hex
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "m5jcy_p1", 0x000000, 0x080000, CRC(eb07c5be) SHA1(a4d810b6886c14988d9a24e7965b57ab1de7e44d) )
	ROM_LOAD16_BYTE( "m5jcy_p2", 0x000001, 0x080000, CRC(22fc000b) SHA1(30f5dff01332d72dfab46a509325a91306bb5729) )
ROM_END

ROM_START( m5mega ) // mega_zone_(barcrest)_[c01_800_25jp].hex
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "m5mega_p1", 0x000000, 0x080000, CRC(811d37da) SHA1(dbf1ebbd0952648096df4caffa8387d4b67e5e3b) )
	ROM_LOAD16_BYTE( "m5mega_p2", 0x000001, 0x080000, CRC(8feea46b) SHA1(696e67de08cde7717233e1709f13251caa3187ca) )
	ROM_LOAD16_BYTE( "m5mega_p3", 0x100000, 0x080000, CRC(830354c5) SHA1(e7d74489b11021116d581fe257f2c75860481164) )
	ROM_LOAD16_BYTE( "m5mega_p4", 0x100001, 0x080000, CRC(0aa822f3) SHA1(6c67a803527f3720e78dafc32e45660a895e8d0d) )
ROM_END

ROM_START( m5mprio ) // monty_python_rio_(barcrest)_[c01_800_25jp].hex
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "m5mprio_p1", 0x000000, 0x080000, CRC(468e49a6) SHA1(5b27bdb557f6209be27e875254890d46df3a7976) )
	ROM_LOAD16_BYTE( "m5mprio_p2", 0x000001, 0x080000, CRC(ef61769e) SHA1(4c2746ee3cc338a74fce838374c5d3eb671d3ca7) )
	ROM_LOAD16_BYTE( "m5mprio_p3", 0x100000, 0x080000, CRC(9926918e) SHA1(48b9fe6b69b4c6a90aa436e78fc27996ac43efd8) )
	ROM_LOAD16_BYTE( "m5mprio_p4", 0x100001, 0x080000, CRC(779fdc4b) SHA1(2c5056c73d5f9977ba9ce3812186d6a77050d5ae) )
ROM_END

ROM_START( m5redbal ) // random_red_ball_(vivid)_[c01_800_25jp].hex
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "m5redbal_p1", 0x000000, 0x080000, CRC(6e168859) SHA1(f973f6c0efc70a2f91c35f6cf57d9c030c6a1c22) )
	ROM_LOAD16_BYTE( "m5redbal_p2", 0x000001, 0x080000, CRC(7396b988) SHA1(ac431f50809fffc604a9ea0a0c68430b38baf225) )
	ROM_LOAD16_BYTE( "m5redbal_p3", 0x100000, 0x080000, CRC(65de31db) SHA1(b23a1d0a5297bc82870b346fb9d151e6f9264a37) )
	ROM_LOAD16_BYTE( "m5redbal_p4", 0x100001, 0x080000, CRC(6f8b028e) SHA1(10a19a7579b63cc8f4f7725cee8f5d1dd52ffcc1) )
ROM_END

ROM_START( m5roof ) // raise_the_roof_(barcrest)_[c01_800_25jp].hex
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "m5roof_p1", 0x000000, 0x080000, CRC(43876f81) SHA1(b783943150c5eee599209c6d00b330a23301613e) )
	ROM_LOAD16_BYTE( "m5roof_p2", 0x000001, 0x080000, CRC(bc7995e5) SHA1(ea785102efa3f0e193e1c532d7ed19b090cf7ab0) )
	ROM_LOAD16_BYTE( "m5roof_p3", 0x100000, 0x080000, CRC(791d353a) SHA1(d50c5ee168dae0257a6de48322360f5d94ab547d) )
	ROM_LOAD16_BYTE( "m5roof_p4", 0x100001, 0x080000, CRC(6e2c4f44) SHA1(246869eb38b62e3a2cf9efbc525218ee9be750fb) )
ROM_END

ROM_START( m5rub ) // rubies_&_diamonds_(barcrest)_[c01_800_25jp].hex
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "m5rub_p1", 0x000000, 0x080000, CRC(2c4621be) SHA1(dbf0dcf7bed5215f9f11b7bc7da4a58115137cd9) )
	ROM_LOAD16_BYTE( "m5rub_p2", 0x000001, 0x080000, CRC(fe1d4ebb) SHA1(eb1d90a6d548e4480fe18cca2a299c72b4b2742f) )
	ROM_LOAD16_BYTE( "m5rub_p3", 0x100000, 0x080000, CRC(a1cddb42) SHA1(8a8e9a62beab44972f6ca4cd698fca990082237c) )
	ROM_LOAD16_BYTE( "m5rub_p4", 0x100001, 0x080000, CRC(994ff2f8) SHA1(d3f887c5f3d40e11c08f5dc9524530a416f1c50c) )
ROM_END

ROM_START( m5seven ) // seven_deadly_spins_(barcrest)_[c01_1024_25jp].hex
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "m5seven_p1", 0x000000, 0x080000, CRC(6117ac2a) SHA1(30ef83dc052527f69183fcfcad7508b6ef556309) )
	ROM_LOAD16_BYTE( "m5seven_p2", 0x000001, 0x080000, CRC(e6340bbb) SHA1(1c89c7b1cb755508b04c25242ace81e165eba6c0) )
	ROM_LOAD16_BYTE( "m5seven_p3", 0x100000, 0x080000, CRC(94554d37) SHA1(d4fed257babaf0367ec1281773e9b79fb707443e) )
	ROM_LOAD16_BYTE( "m5seven_p4", 0x100001, 0x080000, CRC(3c0e7d9d) SHA1(8f3a82ddeebe0c4ddd1de0d4c6ec1f42df5a69ed) )
ROM_END

ROM_START( m5shark ) // shark_raving_mad_(vivid)_[c01_800_25jp].hex
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "m5shark_p1", 0x000000, 0x080000, CRC(85a9847a) SHA1(c819799cd98c0b3fdce8ef3f01ad5b2dc4cbac20) )
	ROM_LOAD16_BYTE( "m5shark_p2", 0x000001, 0x080000, CRC(eb5d1814) SHA1(3dcb3e77cb546d8f77b3ba0e9d2a0f9831c5e83e) )
	ROM_LOAD16_BYTE( "m5shark_p3", 0x100000, 0x080000, CRC(6552161c) SHA1(31fc00ecb64777cdeb54c7a15df0a5f160724fd8) )
	ROM_LOAD16_BYTE( "m5shark_p4", 0x100001, 0x080000, CRC(b4e8137e) SHA1(000bbe4a8dedad0f752eecee6208cc0250c0e742) )
ROM_END

ROM_START( m5sharka )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sharkrav.p1", 0x00000, 0x080000, CRC(8a672a5f) SHA1(04349940a3b8e2d282f256fed205c0ff97371541) )
	ROM_LOAD16_BYTE( "sharkrav.p2", 0x00001, 0x080000, CRC(650ea6c6) SHA1(c6569d6e0163116c4997250a9db8658fed1e1cdd) )
	/* 3+4 */
ROM_END


ROM_START( m5supnov ) // supernova_(barcrest)_[c01_800_25jp].hex
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "m5supnov_p1", 0x000000, 0x080000, CRC(0e036414) SHA1(080df85904df1ae0568cb4f5f180b66943411988) )
	ROM_LOAD16_BYTE( "m5supnov_p2", 0x000001, 0x080000, CRC(c58d5fa0) SHA1(2e90c811977be290c020184ebd27f4a47e272ea7) )
	ROM_LOAD16_BYTE( "m5supnov_p3", 0x100000, 0x080000, CRC(1ec42684) SHA1(20c73adbcbbfbe2df6a87e6c9aa112d7fdfd5b6f) )
	ROM_LOAD16_BYTE( "m5supnov_p4", 0x100001, 0x080000, CRC(4e1ad394) SHA1(ef051be5b13585fd61cc185f4c279a322d1d1f94) )
ROM_END

ROM_START( m5supnova )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "s_nova.p1", 0x00000, 0x080000, CRC(b4ccd869) SHA1(e93edeae1689b684f35fd299d9377ccd87bf2bb5) )
	ROM_LOAD16_BYTE( "s_nova.p2", 0x00001, 0x080000, CRC(dd433e0a) SHA1(cdf2ab3e5d8921797e5041e5feeb4a235031c3b8) )
	/* 3+4 */
ROM_END


ROM_START( m5xchn ) // exchanges_unlimited_(barcrest)_[mpu5]_[dx01_1280_5jp].hex
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "m5xchn_p1", 0x000000, 0x080000, CRC(1534c53c) SHA1(4dab78f986ea0b516d2c6e5e175a43bffb097082) )
	ROM_LOAD16_BYTE( "m5xchn_p2", 0x000001, 0x080000, CRC(60de9a3f) SHA1(5603880f404855631fa7501bf5c55b5de8de48ad) )
ROM_END

/* misc roms below.. */

ROM_START( m5bankrl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bnkr0_5.p1", 0x000000, 0x080000, CRC(de7bbebf) SHA1(239a319c3b41a64852881ed32a4d827e3138e037) )
	ROM_LOAD16_BYTE( "bnkr0_5.p2", 0x000001, 0x080000, CRC(fc4cab14) SHA1(91582d72df63bee8fde30a096bd9811d1ef56623) )
	ROM_LOAD16_BYTE( "bnkr0_5.p3", 0x100000, 0x080000, CRC(8ccf3631) SHA1(e9354cc9777f04f3a59634a39e4f85027c5de71b) )
	ROM_LOAD16_BYTE( "bnkr0_5.p4", 0x100001, 0x080000, CRC(b2842013) SHA1(f754def54f44a41ce8a669670278f33b1bc6974c) )
ROM_END

ROM_START( m5caesc )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ccd_21b.p1", 0x000000, 0x080000, CRC(e5f6fc5c) SHA1(a0208d95831d164069af546f737de64e814d2f81) )
	ROM_LOAD16_BYTE( "ccd_21ge.p2", 0x000001, 0x080000, CRC(8740841a) SHA1(f573b9e9758b12ce966a0e44aba22acf1a0b8f0f) )
ROM_END

ROM_START( m5carwsh )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cwa_sjh1.1d1", 0x000000, 0x080000, CRC(920c6cba) SHA1(5bb91bc79c26db06b04802e009389de84f93cdab) )
	ROM_LOAD16_BYTE( "cwa_sjh1.1d2", 0x000001, 0x080000, CRC(00ebc167) SHA1(5b52260b4f459f4ebf4f65d912c747b0b4a1bff9) )
	ROM_LOAD16_BYTE( "cwa_sjs1.1_3", 0x100000, 0x080000, CRC(09d0464b) SHA1(2a59284dbb546ccf4fa4dcae4ec21c4a74ebe694) )
	ROM_LOAD16_BYTE( "cwa_sjs1.1_4", 0x100001, 0x080000, CRC(2aa7cb3f) SHA1(3ac7c4685eaccab1fef55ffb823f6ed6436f4e99) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "cwa_sjk1.1_1", 0x000000, 0x080000, CRC(550d44f2) SHA1(e310b655c64bd58a29c5928465d551fb98ea510e) )
	ROM_LOAD16_BYTE( "cwa_sjs1.1_1", 0x000000, 0x080000, CRC(b2ef674b) SHA1(b5234236f1cb7d2c70b245f22473e4aefa653cfe) )
	ROM_LOAD16_BYTE( "cwa_sjs1.1d1", 0x000000, 0x080000, CRC(75ee4f03) SHA1(ca08bbc221a3fd8e295265fcab63e0186ef65e46) )
	ROM_LOAD16_BYTE( "carwash.p1",   0x000000, 0x080000, CRC(04aac0c7) SHA1(17d973152cbec43de9c6933299d518448e6d7e01) ) // seems to be a 1.1 rom
ROM_END

ROM_START( m5carwsh10 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cwa_sjs1.0_1", 0x000000, 0x080000, CRC(15e43e69) SHA1(dac457264768c2c760e29aadd8c85c17c4c4532c) )
	ROM_LOAD16_BYTE( "cwa_sjs1.0_2", 0x000001, 0x080000, CRC(0aa7991a) SHA1(1c717274ea3cdc5ade4400c30a4126a2c6649412) )
ROM_END


ROM_START( m5casfev )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cfc11s.p1", 0x000000, 0x080000, CRC(92403682) SHA1(1b3c40a301b9231cc4c42bcd1a1733863014464e) )
	ROM_LOAD16_BYTE( "cfc11s.p2", 0x000001, 0x080000, CRC(98172baa) SHA1(992fb000a0b6ba6d1a891d1f8d496fb95968982b) )
	ROM_LOAD16_BYTE( "cfc11s.p3", 0x100000, 0x080000, CRC(97f65982) SHA1(ffdd993adcbb06ffd1998d50dccbdc9b0d827181) )
	ROM_LOAD16_BYTE( "cfc11s.p4", 0x100001, 0x080000, CRC(b014e6d0) SHA1(bd0ad74d6527c00fe4121ec8a7a9af778f2c8d22) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "cfc11d.p1", 0x000000, 0x080000, CRC(ae943f15) SHA1(2c469ba205bdcaedb9714db101bc41a1ab66ec15) )
ROM_END

ROM_START( m5casfev12 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cfc12s.p1", 0x000000, 0x080000, CRC(8db7841d) SHA1(2875405ec0cfd1433cf62f6cf776e92f862ec93b) )
	ROM_LOAD16_BYTE( "cfc12s.p2", 0x000001, 0x080000, CRC(40bd95ac) SHA1(a6013b0b2d74ff5b303a514a9b69283ba02640f2) )
	ROM_LOAD16_BYTE( "cfc12s.p3", 0x100000, 0x080000, CRC(97f65982) SHA1(ffdd993adcbb06ffd1998d50dccbdc9b0d827181) )
	ROM_LOAD16_BYTE( "cfc12s.p4", 0x100001, 0x080000, CRC(b014e6d0) SHA1(bd0ad74d6527c00fe4121ec8a7a9af778f2c8d22) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "cfc12d.p1", 0x000000, 0x080000, CRC(b1638d8a) SHA1(b6b56df6e96a84d6b733f20e130def79a83eb96f) )
ROM_END


ROM_START( m5dblfun )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "dod_2_1.p1", 0x000000, 0x080000, CRC(8c61150c) SHA1(2edde30953f96f27bf346f7dcca659ff2773777b) )
	ROM_LOAD16_BYTE( "dod_2_1.p2", 0x000001, 0x080000, CRC(2aa3e4f4) SHA1(1ef2c926b254aff36af1294ca5f4cb852efc4da8) )
ROM_END

ROM_START( m5eggold )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "egosjb_1.7_1", 0x000000, 0x080000, CRC(370b2f82) SHA1(f0949b10061f5327bc24feb2b3b4eb03b95de82a) )
	ROM_LOAD16_BYTE( "egosjb_1.7_2", 0x000001, 0x080000, CRC(578bf881) SHA1(6df7d60171f8edce7e291eeeec01d4d87ed37742) )
	ROM_LOAD16_BYTE( "egosjb_1.7_3", 0x100000, 0x080000, CRC(5012e825) SHA1(e55456d85ed65dd33def9bb9e097ba8df7164e2c) )
	ROM_LOAD16_BYTE( "egosjb_1.7_4", 0x100001, 0x080000, CRC(ba986365) SHA1(bb5a501a8c4ae63baa4c538a64e799854a7544df) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "egosjbd1.7_1", 0x000000, 0x080000, CRC(88d928de) SHA1(c44264c7e86d2f318237c83696e8401bd669d97b) )
	//ROM_LOAD16_BYTE( "egosjbd1.7_2", 0x000000, 0x080000, CRC(578bf881) SHA1(6df7d60171f8edce7e291eeeec01d4d87ed37742) )
ROM_END

ROM_START( m5egr ) // elvis_gold_rush_(barcrest)_[c01_800_25jp].hex
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "m5egr_p1", 0x000000, 0x080000, CRC(07a5b281) SHA1(cd755089a6a5db5d6923e3fb1660d2748af57b4e) )
	ROM_LOAD16_BYTE( "m5egr_p2", 0x000001, 0x080000, CRC(2f0a54dc) SHA1(c0a4fd277b6b8cae9109222961c4c86f0a9b9103) )
	ROM_LOAD16_BYTE( "m5egr_p3", 0x100000, 0x080000, CRC(a485956c) SHA1(9d16ef5d060ee663d7f68b9f5bd00ccad7a58825) )
	ROM_LOAD16_BYTE( "m5egr_p4", 0x100001, 0x080000, CRC(84fe0436) SHA1(f8a072ae19314dde195e4fde5032aba041e72703) )
ROM_END

ROM_START( m5egra ) // these should probably be split into odd/even
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "egrw0_2.bin", 0x000000, 0x200000, CRC(7e4743d7) SHA1(2beb4600b7d05b8a6552101b13c804044c69fb86) )

	ROM_REGION( 0x200000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD( "egrw0_2d.bin", 0x000000, 0x200000, CRC(fd0fbb58) SHA1(2259dcf39d71211d488e0463c54126bd81b925b8) )
	ROM_LOAD( "egrw0_2da.bin", 0x000000, 0x200000, CRC(e9b773e8) SHA1(5db9f318344ee793db64860d9ce28ee4b25e4221) )
	ROM_LOAD( "egrw0_2de.bin", 0x000000, 0x200000, CRC(171ba8e0) SHA1(191b7142b405df5d591fc1b9513a7ba3856bf613) )
ROM_END

ROM_START( m5evgrhr ) // elvis_gold_red_hot_roll_(barcrest)_[c01_800_25jp].hex
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "m5evgrhr_p1", 0x000000, 0x080000, CRC(43a5f54e) SHA1(bf84a27da357206bed44abac72802a0572923066) )
	ROM_LOAD16_BYTE( "m5evgrhr_p2", 0x000001, 0x080000, CRC(6fc39a43) SHA1(6c8711f5c76dd1e2f3d14f89da6ef7a98c833049) )
	ROM_LOAD16_BYTE( "m5evgrhr_p3", 0x100000, 0x080000, CRC(3a9a1c94) SHA1(f135904aa4b33d709830c5baebbc604d7b2873ee) )
	ROM_LOAD16_BYTE( "m5evgrhr_p4", 0x100001, 0x080000, CRC(235dee6d) SHA1(fbc6c82cac5cad16f1190193109de78844d2e482) )
ROM_END

ROM_START( m5evgrhra ) // these should probably be split into odd/even
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "errw0_2.bin", 0x000000, 0x200000, CRC(2b352b8b) SHA1(4c514f304d4d5eee53c450059edb62eabde8838e) )

	ROM_REGION( 0x200000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD( "errw0_2d.bin", 0x000000, 0x200000, CRC(dc5d2e8d) SHA1(f5949dd800c16812fd16d6b8f2dd392ecf6ac36c) )
	ROM_LOAD( "errw0_2da.bin", 0x000000, 0x200000, CRC(adb7d298) SHA1(5e5f8326a5ff1602a1e85a6fcd2415ada5934449) )
	ROM_LOAD( "errw0_2de.bin", 0x000000, 0x200000, CRC(72f3ed0d) SHA1(0dec445dc21aadfcbd35ce7f2fbc216e9abaa723) )
ROM_END

ROM_START( m5egss ) // elvis_gold_super_streak_(barcrest)_[c01_800_25jp].hex
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "m5egss_p1", 0x000000, 0x080000, CRC(40b520b3) SHA1(2d8e827a00dba1f09e66cb0329ba332fc05335bb) )
	ROM_LOAD16_BYTE( "m5egss_p2", 0x000001, 0x080000, CRC(47504b14) SHA1(82ee76f4299dc9b92b774eb4ecfbc618a90465ab) )
	ROM_LOAD16_BYTE( "m5egss_p3", 0x100000, 0x080000, CRC(f8f38e74) SHA1(25a815f7c694eca441116cde78c6159b8640b4b9) )
	ROM_LOAD16_BYTE( "m5egss_p4", 0x100001, 0x080000, CRC(4e68217a) SHA1(172cb7bd733b6c9f5da64ca50b9ad8457c0491b8) )
ROM_END

ROM_START( m5egssa )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "essw0_1.bin", 0x000000, 0x200000, CRC(069a7b18) SHA1(da08a297de367b226d41d81905600c070c02634e) )

	ROM_REGION( 0x200000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD( "essw0_1ad.bin", 0x000000, 0x200000, CRC(28486881) SHA1(806aa1d06bf85ac223eb517ee848ac0e82c4aa48) )
	ROM_LOAD( "essw0_1d.bin", 0x000000, 0x200000, CRC(7de832e0) SHA1(b3841ef34e7f2aa9f5c906b13a32b4514d67cf8b) )
	ROM_LOAD( "essw0_1de.bin", 0x000000, 0x200000, CRC(924bc9dd) SHA1(af5019b49f7555616bb9b0945c8c16d2fe0977be) )
ROM_END

ROM_START( m5psy2 ) // these should probably be split into odd/even
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "psco0_2.bin", 0x000000, 0x300000, CRC(a7808949) SHA1(040730642d1e753cfa1d5fa69b9fa418b48a2a1c) )
ROM_END

ROM_START( m5fair )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fa2_23s.p1", 0x000000, 0x080000, CRC(d409ca11) SHA1(3f1413617f5a90ac924ce578fd24dc1fe018392c) )
	ROM_LOAD16_BYTE( "fa2_23j.p2", 0x000001, 0x080000, CRC(9eb18bc7) SHA1(befbc337a8d120173ec21e61c4837504aa134926) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "fat_25s.p1", 0x000000, 0x080000, CRC(f62316e8) SHA1(48fd20e1662c715e604f978d308f38eef1a88cda) )
ROM_END


ROM_START( m55050 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ffi_sjs1.2_1", 0x000000, 0x080000, CRC(7e0143d5) SHA1(5368a4cd8184c69363c7c79e9abe4f8c5a3f635c) )
	ROM_LOAD16_BYTE( "ffi_sjs1.2_2", 0x000001, 0x080000, CRC(78cd4afa) SHA1(cf9751822071de2a802db75d39a1140938dfaf7e) )
	ROM_LOAD16_BYTE( "fifty_fi.p3", 0x100000, 0x080000, CRC(8962dd15) SHA1(e5f8251370b575dbccdce0e5a38d0ad24895077d) )
	ROM_LOAD16_BYTE( "fifty_fi.p4", 0x100001, 0x080000, CRC(5334ba78) SHA1(3ac22f8beb2770a1d48562ec57e9755d52a706f3) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "ffi_sjs1.2d1", 0x000000, 0x080000, CRC(2deaaec9) SHA1(79aab4096f8438f95a4bee54b23193aa35e43229) )
	ROM_LOAD16_BYTE( "ffi_sjs1.2d2", 0x000000, 0x080000, CRC(78cd4afa) SHA1(cf9751822071de2a802db75d39a1140938dfaf7e) )
ROM_END




ROM_START( m5gpclub )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gplsj__0.2_1", 0x000000, 0x080000, CRC(0485374a) SHA1(67bfce41eb71f42641a0534f6f7c4ff5b03d66ea) )
	ROM_LOAD16_BYTE( "gplsj__0.2_2", 0x000001, 0x080000, CRC(4a30ed39) SHA1(ed561e6ad4fcf7d8de52cc4e68e04239e48e90ee) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "gplsj_d0.2_1", 0x000000, 0x080000, CRC(2721cd0b) SHA1(7dbf5a4df804c766f3ee24219a1e2ea1cc1add65) )
	ROM_LOAD16_BYTE( "gplsj_d0.2_2", 0x000000, 0x080000, CRC(4a30ed39) SHA1(ed561e6ad4fcf7d8de52cc4e68e04239e48e90ee) )
ROM_END


ROM_START( m5goape )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ape_sjs1.2_1", 0x000000, 0x080000, CRC(06ed6cc6) SHA1(2db73e2de60efc46e947b324b55cf9c25c7b26e1) )
	ROM_LOAD16_BYTE( "ape_sjs1.2_2", 0x000001, 0x080000, CRC(0720c194) SHA1(179833d09e879cbd6feab363d132630ef1198662) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "ape_sjs1.2d1", 0x000000, 0x080000, CRC(9a3c70b3) SHA1(91510d3500d817426a1ec65d0c5add822ac3a702) )
	ROM_LOAD16_BYTE( "ape_sjs1.2d2", 0x000000, 0x080000, CRC(0720c194) SHA1(179833d09e879cbd6feab363d132630ef1198662) )
ROM_END


ROM_START( m5invad )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "inv10s.p1", 0x000000, 0x080000, CRC(1ef7b687) SHA1(4ef180da702d7bbafcb67d9aabbea780803382cf) )
	ROM_LOAD16_BYTE( "inv10s.p2", 0x000001, 0x080000, CRC(898f3bcb) SHA1(5b37e87fb883fd9215d432b88e9de5e52ff24359) )
	ROM_LOAD16_BYTE( "inv10s.p3", 0x100000, 0x080000, CRC(0f3b5515) SHA1(f4e6c324be8d401a5d097d7fa2c22379a7d2224e) )
	ROM_LOAD16_BYTE( "inv10s.p4", 0x100001, 0x080000, CRC(ff30031c) SHA1(619787a61365524436310b78188dd8e2b14642e4) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "inv10d.p1", 0x000000, 0x080000, CRC(2223bf10) SHA1(518414e2e7781b8615b8bcd5a4e05b6c23433951) )
	ROM_LOAD16_BYTE( "inv10k.p1", 0x000000, 0x080000, CRC(7cd9fb7e) SHA1(abb1279a43db6828e6493bca9c706ffa6838ae2d) )
ROM_END

ROM_START( m5jcptgn )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jp_genie.p1", 0x000000, 0x080000, CRC(cab55d2a) SHA1(bfe1719a0bdb6df08a9b8deb16a8f85cec6c9f80) )
	ROM_LOAD16_BYTE( "jp_genie.p2", 0x000001, 0x080000, CRC(96fa3be3) SHA1(fe7d0cc25f9544dbe982c1791f107efb5cc1f4b1) )
ROM_END

ROM_START( m5jakjok )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jjd_3_1.p1", 0x000000, 0x080000, CRC(b820f0be) SHA1(d91f0d2d29e34fc99691fe8d555a094e1086a0ad) )
	ROM_LOAD16_BYTE( "jjd_3_1.p2", 0x000001, 0x080000, CRC(6a6f720a) SHA1(f7059148aa75162f8d7efe6ec38f3ee337b12cd9) )
ROM_END


ROM_START( m5jlstrk )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jewel s.p1", 0x000000, 0x080000, CRC(5036918d) SHA1(c1e5d98e9dbc2b03e5e12b30c4902d5b5f27e19b) )
	ROM_LOAD16_BYTE( "jewel s.p2", 0x000001, 0x080000, CRC(dc853ee6) SHA1(f127b85ebd8b9115409d73c5e1efb188552270b4) )
	ROM_LOAD16_BYTE( "jewel s.p3", 0x100000, 0x080000, CRC(4dd82061) SHA1(da936a79390050693ac23563efbca6676241e8bc) )
	ROM_LOAD16_BYTE( "jewel s.p4", 0x100001, 0x080000, CRC(0fbb9858) SHA1(033113fc064934373edb116c1f070d00a32fe0e0) )
ROM_END

ROM_START( m5jokpak )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jpa_sjs1.0_1", 0x000000, 0x080000, CRC(3829a1bf) SHA1(46ea0d00b87d726ae66387a9a5a5d596290d8bce) )
	ROM_LOAD16_BYTE( "jpa_sjs1.0_2", 0x000001, 0x080000, CRC(3d677754) SHA1(5239bd36706fc5d71d35fb9a00c63863018c2a1c) )
ROM_END



ROM_START( m5lvwire )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lw__sjh1.1d1", 0x000000, 0x080000, CRC(b718940a) SHA1(a96bdada794c15b55c16113ad69ea28ea93e51ba) )
	ROM_LOAD16_BYTE( "lw__sjh1.1d2", 0x000001, 0x080000, CRC(6ac5a16a) SHA1(a81b79aba4826f40086c05618689328970532a4b) )
	ROM_LOAD16_BYTE( "lw__sjh1.1d3", 0x100000, 0x080000, CRC(fec1fba9) SHA1(292de255b530912bc8866efffbec469677877ec1) )
	ROM_LOAD16_BYTE( "lw__sjh1.1d4", 0x100001, 0x080000, CRC(1d5cd718) SHA1(b2c76883f0cbd41e0e060f3c5d4f229f22ba19ee) )
ROM_END

ROM_START( m5lvwirea )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lw__sjk1.1_1", 0x000000, 0x080000, CRC(ab54ed24) SHA1(7553efa901e1b49435cd5d1a770531a0c8df646b) )
	ROM_LOAD16_BYTE( "lw__sjk1.1_2", 0x000001, 0x080000, CRC(6ac5a16a) SHA1(a81b79aba4826f40086c05618689328970532a4b) )
	ROM_LOAD16_BYTE( "lw__sjk1.1_3", 0x100000, 0x080000, CRC(fec1fba9) SHA1(292de255b530912bc8866efffbec469677877ec1) )
	ROM_LOAD16_BYTE( "lw__sjk1.1_4", 0x100000, 0x080000, CRC(1d5cd718) SHA1(b2c76883f0cbd41e0e060f3c5d4f229f22ba19ee) )
ROM_END







ROM_START( m5minesw )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ms__sjh2.2d1", 0x000000, 0x080000, CRC(336e5e15) SHA1(c513cdf1f4c22d7555c697efabb96aa7f53a8529) )
	ROM_LOAD16_BYTE( "ms__sjh2.2d2", 0x000001, 0x080000, CRC(31187bc2) SHA1(d352d67feddbf670dbbaff224b19701ce1231fd9) )
	ROM_LOAD16_BYTE( "ms__sjh2.2d3", 0x100000, 0x080000, CRC(8c44ed26) SHA1(35b29f7e4d9c1c73dd9d4891decc131443fc21c5) )
	ROM_LOAD16_BYTE( "ms__sjh2.2d4", 0x100001, 0x080000, CRC(eb3b0f16) SHA1(4664110800c2f5005ffe8d8c1cf5b588dd994347) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "ms__sjs2.2_1", 0x000000, 0x080000, CRC(312c2501) SHA1(95e16c5013ea8e246c04b828af6f6b3ca82d18e0) )
//  ROM_LOAD16_BYTE( "ms__sjs2.2_2", 0x000000, 0x080000, CRC(31187bc2) SHA1(d352d67feddbf670dbbaff224b19701ce1231fd9) )
//  ROM_LOAD16_BYTE( "ms__sjs2.2_3", 0x000000, 0x080000, CRC(8c44ed26) SHA1(35b29f7e4d9c1c73dd9d4891decc131443fc21c5) )
//  ROM_LOAD16_BYTE( "ms__sjs2.2_4", 0x000000, 0x080000, CRC(eb3b0f16) SHA1(4664110800c2f5005ffe8d8c1cf5b588dd994347) )
ROM_END


ROM_START( m5paint )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ptr10s.p1", 0x000000, 0x080000, CRC(3dc58348) SHA1(11573db7637577311a59c41d22a58e4c9d8c4d2b) )
	ROM_LOAD16_BYTE( "ptr10s.p2", 0x000001, 0x080000, CRC(a1a2d506) SHA1(8ca7bf46dd71e845ccf6ca532298c0ba1b6ccbec) )
	ROM_LOAD16_BYTE( "ptr10s.p3", 0x100000, 0x080000, CRC(888aeb56) SHA1(0d416d6a06cebe96d89f9c59252ab9b413e81d1c) )
	ROM_LOAD16_BYTE( "ptr10s.p4", 0x100001, 0x080000, CRC(edcfb408) SHA1(b9592ad9d4330cb9a10e7adbccd0b75d3707498e) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "ptr10d.p1", 0x000000, 0x080000, CRC(01118adf) SHA1(efc9c150d45921196b334dfd7a566b36d71526ed) )
	ROM_LOAD16_BYTE( "ptr10k.p1", 0x000000, 0x080000, CRC(5febceb1) SHA1(01e060545ce54bb2c65dd505dc6fbc2755ea6f11) )
ROM_END

ROM_START( m5quake )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "quake.p1", 0x000000, 0x080000, CRC(3a83ba9e) SHA1(dc71926959fb0aec59cc20ec7afa50e47ae8dd41) )
	ROM_LOAD16_BYTE( "quake.p2", 0x000001, 0x080000, CRC(bcf47ebd) SHA1(14e07d07edb147dc4ec6ffeac01b46eaa50e0c3a) )
ROM_END


ROM_START( m5psycho )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pcab10ad.p1", 0x000000, 0x080000, CRC(c3967c70) SHA1(9ae5e13e934cc31260e091b8ba19fb8def8ba8b9) )
	ROM_LOAD16_BYTE( "pcab10.p2", 0x000001, 0x080000, CRC(0be072e9) SHA1(faedf0629e7a56b162396ccb52b52222df2677fd) )
	ROM_LOAD16_BYTE( "pcab10.p3", 0x100000, 0x080000, CRC(3758e3d5) SHA1(99a05b033056a22ad689fe6810d5492c3017d6c5) )
	ROM_LOAD16_BYTE( "pcab10.p4", 0x100001, 0x080000, CRC(9d88adcc) SHA1(ce7f0ff6568006f31d2b05d8c1be8ace964604ac) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "pcab10b.p1", 0x000000, 0x080000, CRC(27bac3c9) SHA1(4f7f9d81ef34c846eeda5332fd3a3870247f7ff0) )
	ROM_LOAD16_BYTE( "pcab10bd.p1", 0x000000, 0x080000, CRC(e543cb89) SHA1(9bd7b6060f968406ef3ec338ef59b014736cd262) )
	ROM_LOAD16_BYTE( "pcab10d.p1", 0x000000, 0x080000, CRC(f99f1f97) SHA1(315061a376b5b567f0e7052637d0f89db8ad15e4) )
	ROM_LOAD16_BYTE( "pcab10dy.p1", 0x000000, 0x080000, CRC(573e2784) SHA1(d37c3b448edd33b14d0019b9667bdf0e37d5d772) )
	ROM_LOAD16_BYTE( "pcab10h.p1", 0x000000, 0x080000, CRC(0fe9bf2c) SHA1(5020559a64e30ac4c8d2a495fd5dab578d0f1a62) )
	ROM_LOAD16_BYTE( "pcab10k.p1", 0x000000, 0x080000, CRC(ad095dba) SHA1(4b76da6945706addb6da4d3045c81886b78f97fb) )
	ROM_LOAD16_BYTE( "pcab10r.p1", 0x000000, 0x080000, CRC(ee462948) SHA1(507d5d30e194da48491b3b9ba7ae530dac928f9c) )
	ROM_LOAD16_BYTE( "pcab10s.p1", 0x000000, 0x080000, CRC(3b6617d7) SHA1(04d61bb41eacbfe77b31481c2757fe196d1c9dcc) )
	ROM_LOAD16_BYTE( "pcab10y.p1", 0x000000, 0x080000, CRC(95c72fc4) SHA1(d602907decdf9f210f7c4c0f1745f3d40dbce36e) )
ROM_END

ROM_START( m5psycho06 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pcab06s.p1",   0x000000, 0x080000, CRC(3337d95d) SHA1(add517827e39e440485d697ea5071f92e8f75912) )
	ROM_LOAD16_BYTE( "pcab06.p2",    0x000001, 0x080000, CRC(10d167dd) SHA1(9493be46ab01695f42d26f746bbbfbaecb389760) )
	ROM_LOAD16_BYTE( "pcab06.p3",    0x100000, 0x080000, CRC(3cad2a53) SHA1(38ae7e368136ec0566324e8a9fa4815d4be0c5c6) )
	ROM_LOAD16_BYTE( "pcab06.p4",    0x100001, 0x080000, CRC(a0300b0b) SHA1(bd3d3f4ecf25fbc0d116d524f5cf3a5eca698c09) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "pcab06_alt.p2", 0x000000, 0x080000, CRC(f29126df) SHA1(33077ad7f43475fc2e31aeea5e55800c0186fe31) )
ROM_END


ROM_START( m5psychoa )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ps_sja_2.0_1", 0x000000, 0x080000, CRC(78433b76) SHA1(2d019f09ef87e147171c04627a2b4d289ffa0ad9) )
	ROM_LOAD16_BYTE( "ps_sja_2.0_2", 0x000001, 0x080000, CRC(cc6e2a64) SHA1(612be10c5577c1194d5319f1ca37f6308a7ef962) )
	ROM_LOAD16_BYTE( "ps_sja_2.0_3", 0x100000, 0x080000, CRC(07c62c20) SHA1(dde94f3ac2db6bc8b7273ce3fcae971fc680a15f) )
	ROM_LOAD16_BYTE( "ps_sja_2.0_4", 0x100001, 0x080000, CRC(d2a6bdcf) SHA1(ad76bc4930f183d1d23906b0a4ca7b1cb03c9678) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "ps_sj__2.0_1", 0x000000, 0x080000, CRC(75df1f85) SHA1(d9e52fb053d9b24899b16b4abe68974a36967baf) )
	ROM_LOAD16_BYTE( "ps_sj__2.0_2", 0x000000, 0x080000, CRC(cc6e2a64) SHA1(612be10c5577c1194d5319f1ca37f6308a7ef962) )
	ROM_LOAD16_BYTE( "ps_sj_d2.0_1", 0x000000, 0x080000, CRC(56170b95) SHA1(34da89783384ace48e774a88d65d60e804cd01c6) )
	ROM_LOAD16_BYTE( "ps_sj_d2.0_2", 0x000000, 0x080000, CRC(cc6e2a64) SHA1(612be10c5577c1194d5319f1ca37f6308a7ef962) )
	ROM_LOAD16_BYTE( "ps_sj_d2.0_3", 0x000000, 0x080000, CRC(07c62c20) SHA1(dde94f3ac2db6bc8b7273ce3fcae971fc680a15f) )
	ROM_LOAD16_BYTE( "ps_sj_d2.0_4", 0x000000, 0x080000, CRC(d2a6bdcf) SHA1(ad76bc4930f183d1d23906b0a4ca7b1cb03c9678) )
ROM_END

ROM_START( m5psychoa21 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ps_sj__2.1_1", 0x000000, 0x080000, CRC(692aa84d) SHA1(e715c0502edf576baec312d8d7d3517bcd804aae) )
	ROM_LOAD16_BYTE( "ps_sj__2.1_2", 0x000001, 0x080000, CRC(55ef63d5) SHA1(41f5a0f7cc7c1841a7f7dd52a407e1fa76ca90d5) )
	/* 3+4 */

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "ps_sj_d2.1_1", 0x000000, 0x080000, CRC(47d0bf3c) SHA1(9b4e75b3709767138c4e2c000c374382f2563fe4) )
	ROM_LOAD16_BYTE( "ps_sj_d2.1_2", 0x000000, 0x080000, CRC(55ef63d5) SHA1(41f5a0f7cc7c1841a7f7dd52a407e1fa76ca90d5) )
ROM_END


ROM_START( m5psyccl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pcbc20d.p1", 0x000000, 0x080000, CRC(4bd1b66d) SHA1(5ddcea59ab078ed908181decf36e55f2856d9e93) )
	ROM_LOAD16_BYTE( "pcbc20.p2", 0x000001, 0x080000, CRC(7653c03a) SHA1(418687ce3fedab0c913226e48da56237843a182f) )
	ROM_LOAD16_BYTE( "pcbc20.p3", 0x100000, 0x080000, CRC(9ab26ce1) SHA1(b1e994b991431a3a8c33cc00323650fee2b65086) )
	ROM_LOAD16_BYTE( "pcbc20.p4", 0x100001, 0x080000, CRC(658f2497) SHA1(a76853b41782fc1cd737e1ccdc123295395c35a7) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "pcbc20dz.p1", 0x000000, 0x080000, CRC(7ecf2e71) SHA1(084f95dfc37fa3bcc62b81d1a00296262f9b55f1) )
	ROM_LOAD16_BYTE( "pcbc20f.p1", 0x000000, 0x080000, CRC(bc2ea836) SHA1(e09905c0ed0a649f72db5501e88a900453dc3421) )
	ROM_LOAD16_BYTE( "pcbc20fz.p1", 0x000000, 0x080000, CRC(4eaf9a96) SHA1(663fa66563efa4f0fde1938d6cd95c8b3a00eb2d) )
	ROM_LOAD16_BYTE( "pcbc20s.p1", 0x000000, 0x080000, CRC(96da2f33) SHA1(f8bf6e750e55b974dde6655681691e8ac49bcb67) )
	ROM_LOAD16_BYTE( "pcbc20z.p1", 0x000000, 0x080000, CRC(645b1d93) SHA1(3472e39afd8ac289799a56d7bd4180916b581d6a) )
ROM_END

ROM_START( m5psyccl01 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pcc01.p1", 0x000000, 0x080000, CRC(cc32b82f) SHA1(0faeb359942bd4c97d259057b4665cc466ef7e75) )
	ROM_LOAD16_BYTE( "pcc01.p2", 0x000001, 0x080000, CRC(6e1d2455) SHA1(1b3e2a31c6da0d338e788f0ca3433b2baa1eb172) )
	ROM_LOAD16_BYTE( "pcc01.p3", 0x100000, 0x080000, CRC(9ab26ce1) SHA1(b1e994b991431a3a8c33cc00323650fee2b65086) )
	ROM_LOAD16_BYTE( "pcc01.p4", 0x100001, 0x080000, CRC(658f2497) SHA1(a76853b41782fc1cd737e1ccdc123295395c35a7) )
ROM_END

ROM_START( m5psyccla )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bpc__2.3_1", 0x000000, 0x080000, CRC(df1f22da) SHA1(8c186b2d4d75518872b152b84d920959433d30e8) )
	ROM_LOAD16_BYTE( "bpc__2.3_2", 0x000001, 0x080000, CRC(c7cd10f2) SHA1(e0c2350064dd237a90a2319fc3abcbf7c56077ab) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "bpc__2.3d1", 0x000000, 0x080000, CRC(9e310b44) SHA1(fc3f90f78e14f54bb617df2fe80fb0baae782dd8) )
	ROM_LOAD16_BYTE( "bpc__2.3d2", 0x000000, 0x080000, CRC(c7cd10f2) SHA1(e0c2350064dd237a90a2319fc3abcbf7c56077ab) )
ROM_END

ROM_START( m5psyccla24 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bpc__2.4_1", 0x000000, 0x080000, CRC(4349b016) SHA1(292e80e152671afdafa0b9620b45b3c799b326c0) )
	ROM_LOAD16_BYTE( "bpc__2.4_2", 0x000001, 0x080000, CRC(bd86111e) SHA1(dfbc979cea74c8d85c01898cb7902e302c4e7812) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "bpc__2.4d1", 0x000000, 0x080000, CRC(7755ba59) SHA1(a84f41a942725761a51e44951b9abbc9a3fba18f) )
	ROM_LOAD16_BYTE( "bpc__2.4d2", 0x000001, 0x080000, CRC(bd86111e) SHA1(dfbc979cea74c8d85c01898cb7902e302c4e7812) )
ROM_END

ROM_START( m5psyccla02 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bpc___0.2_1", 0x000000, 0x080000, CRC(bad791f5) SHA1(02f4221be7072ae254e342afc59171e9f2ae72dd) )
	ROM_LOAD16_BYTE( "bpc___0.2_2", 0x000001, 0x080000, CRC(64fef1ce) SHA1(52b466c840b4abb410d3d491808fe889a535d272) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "bpc___0.2d1", 0x000000, 0x080000, CRC(3326565c) SHA1(1c1ace2b5ff9ec21c20e18f01968f34ca7191989) )
	ROM_LOAD16_BYTE( "bpc___0.2d2", 0x000000, 0x080000, CRC(64fef1ce) SHA1(52b466c840b4abb410d3d491808fe889a535d272) )
ROM_END





ROM_START( m5roofa ) // these were in a zip called RED Hot Fever, but it's clearly 'Raise The Roof' (see strings in ROM)
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rtr_sjs1.4_1", 0x000000, 0x080000, CRC(84b8f1e1) SHA1(9cfc9edc40ea66b4d10960cd3bb595c266c1a662) )
	ROM_LOAD16_BYTE( "rtr_sjs1.4_2", 0x000001, 0x080000, CRC(fd7c7292) SHA1(3dd6109cc201879840d88d8948552ad13cc77f71) )


	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "rtr_sjs1.4d1", 0x000000, 0x080000, CRC(b2cff48f) SHA1(5233e80db90873707e31375f264e0f910cbca474) )
	ROM_LOAD16_BYTE( "rtr_sjs1.4d2", 0x000001, 0x080000, CRC(fd7c7292) SHA1(3dd6109cc201879840d88d8948552ad13cc77f71) )
ROM_END

ROM_START( m5rwb ) // red_white_&_blue_(barcrest)_[c01_800_15jp].hex
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "m5rwb_p1", 0x000000, 0x080000, CRC(fd4f4909) SHA1(49d567404dc70a0a1b5c218afbf5d1179ca36931) )
	ROM_LOAD16_BYTE( "m5rwb_p2", 0x000001, 0x080000, CRC(35271032) SHA1(82ff88ae6687534463d3f01a72f93e36532698ce) )
ROM_END

ROM_START( m5rwbbwb )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rb_sjb_0.4_1", 0x000000, 0x080000, CRC(f061ba9d) SHA1(fb78b53bb482184527e98d1ce22a837b5d9807d4) )
	ROM_LOAD16_BYTE( "rb_sjb_0.4_2", 0x000001, 0x080000, CRC(2f8d2328) SHA1(d2cfd4f2f3d05bf487c48ff555ccb935c4ef127c) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "rb_sjbd0.4_1", 0x000000, 0x080000, CRC(cf7bc937) SHA1(7bb3b65ce58cf19b05d04af5b282df96601e2d90) )
	ROM_LOAD16_BYTE( "rb_sjbd0.4_2", 0x000000, 0x080000, CRC(2f8d2328) SHA1(d2cfd4f2f3d05bf487c48ff555ccb935c4ef127c) )
	ROM_LOAD16_BYTE( "rb_sjbg0.4_1", 0x000000, 0x080000, CRC(5fcd233c) SHA1(38877d4f76ac7340b96aca2cd3523fe227436e3d) )
	ROM_LOAD16_BYTE( "rb_sjbg0.4_2", 0x000000, 0x080000, CRC(2f8d2328) SHA1(d2cfd4f2f3d05bf487c48ff555ccb935c4ef127c) )
	// something else? top box?
	ROM_LOAD16_BYTE( "tb_____0.4_1", 0x000000, 0x080000, CRC(2d444fa8) SHA1(2d0b23ac731a8591cc563726a8cbfe780f3c1260) )
	ROM_LOAD16_BYTE( "tb_____0.4_2", 0x000000, 0x080000, CRC(6384d46d) SHA1(0610e55e2b656882d48d33157c11464d6b6964ce) )
ROM_END

ROM_START( m5rwbbwb24 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rb_sjb_2.4_1", 0x000000, 0x080000, CRC(a3902792) SHA1(cbfdd5017220f57c11a21bcd047f65a20ff510c1) )
	ROM_LOAD16_BYTE( "rb_sjb_2.4_2", 0x000001, 0x080000, CRC(00da6cee) SHA1(8bca731ed005c66b0c0410603cc134b87bcd51cc) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "rb_sjbd2.4_1", 0x000000, 0x080000, CRC(6a7ddea2) SHA1(31caa3adb1f6ba6fa6f350d37c3089c92a1f8db6) )
	ROM_LOAD16_BYTE( "rb_sjbd2.4_2", 0x000000, 0x080000, CRC(00da6cee) SHA1(8bca731ed005c66b0c0410603cc134b87bcd51cc) )
	ROM_LOAD16_BYTE( "rb_sjbg2.4_1", 0x000000, 0x080000, CRC(a7c2f03e) SHA1(59e9208d60e9d339ac555319196130dfe7e0f6e6) )
	ROM_LOAD16_BYTE( "rb_sjbg2.4_2", 0x000000, 0x080000, CRC(00da6cee) SHA1(8bca731ed005c66b0c0410603cc134b87bcd51cc) )
ROM_END

ROM_START( m5rwbbwb25 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rb_sjb_2.5_1", 0x000000, 0x080000, CRC(3c676e36) SHA1(f012d94cfbf95d292a0dff4b0e99c7c9a97a5b65) )
	ROM_LOAD16_BYTE( "rb_sjb_2.5_2", 0x000001, 0x080000, CRC(ee1e15b9) SHA1(d48b0fbaf2ef9114368fa1ccf58649d8916fa9b1) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "rb_sjbd2.5_1", 0x000000, 0x080000, CRC(23f84352) SHA1(4d5a70107e52d922994ff8d8c80c521eef1e8c22) )
	ROM_LOAD16_BYTE( "rb_sjbg2.5_1", 0x000000, 0x080000, CRC(b4267357) SHA1(aea70b58be932e88280a92ac587888f733b7cdfa) )
	ROM_LOAD16_BYTE( "rb_sjbt2.5_1", 0x000000, 0x080000, CRC(4323adbe) SHA1(fd27dfed5f04d4d1a84c8df746b5db35766a9592) )
ROM_END

ROM_START( m5rwbbwb15 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rb_sjb_1.5_1", 0x000000, 0x080000, CRC(d1d4890c) SHA1(8490ffc2b55239644e5e556300297eafe2e5c261) )
	ROM_LOAD16_BYTE( "rb_sjb_1.5_2", 0x000001, 0x080000, CRC(6664f683) SHA1(efd9dd7292b4d2b803cea9e7ad8bffd426e783bb) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "rb_sjbg1.5_1", 0x000000, 0x080000, CRC(29dc29f8) SHA1(8159a3f5527e96cbf6f368e998096876f978bbc1) )
	ROM_LOAD16_BYTE( "rb_sjbg1.5_2", 0x000000, 0x080000, CRC(6664f683) SHA1(efd9dd7292b4d2b803cea9e7ad8bffd426e783bb) )
	ROM_LOAD16_BYTE( "rb_sjbd1.5_1", 0x000000, 0x080000, CRC(a5fe48d9) SHA1(e00672428644601a012b4b85412e64817dd0e563) )
	ROM_LOAD16_BYTE( "rb_sjbd1.5_2", 0x000000, 0x080000, CRC(6664f683) SHA1(efd9dd7292b4d2b803cea9e7ad8bffd426e783bb) )
ROM_END



ROM_START( m5reelwn )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rinsjs1.7_1", 0x000000, 0x080000, CRC(ed6fe3b2) SHA1(24437dc3fa0645bb7684b4948b4b846726cc4d28) )
	ROM_LOAD16_BYTE( "rinsjs1.7_2", 0x000001, 0x080000, CRC(e945f64e) SHA1(0cf502898b42e9578d1ead9f0d0f05311d29a107) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "rinsjs1.7d1", 0x000000, 0x080000, CRC(6b6cfed7) SHA1(54248c34693fd58ed8b89bddf158ce15bbffaf68) )
	ROM_LOAD16_BYTE( "rinsjs1.7d2", 0x000000, 0x080000, CRC(e945f64e) SHA1(0cf502898b42e9578d1ead9f0d0f05311d29a107) )
ROM_END

ROM_START( m5reelwn24 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rinsjs2.4_1", 0x000000, 0x080000, CRC(345da61b) SHA1(cf98f96706b1b92fb0848b3464fb02669819480a) )
	ROM_LOAD16_BYTE( "rinsjs2.4_2", 0x000001, 0x080000, CRC(61ea678f) SHA1(5560d11dc085586e19babbb74489505e5fb90e58) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "rinsjs2.4d1", 0x000000, 0x080000, CRC(504aad42) SHA1(53e81f5d14d64722a2f8469ffe4a45647b5dc01c) )
	ROM_LOAD16_BYTE( "rinsjs2.4d2", 0x000000, 0x080000, CRC(61ea678f) SHA1(5560d11dc085586e19babbb74489505e5fb90e58) )
ROM_END


ROM_START( m5reelth )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "brt11s.p1", 0x000000, 0x080000, CRC(fabe439f) SHA1(7b1adbdcd9b48b5ce0a68fa571512c8dc955b734) )
	ROM_LOAD16_BYTE( "brt11s.p2", 0x000001, 0x080000, CRC(181ff295) SHA1(ab461340eca3349402589e6563a7ba88c6e8e5e6) )
	ROM_LOAD16_BYTE( "brt11s.p3", 0x100000, 0x080000, CRC(83a16825) SHA1(4a07873bb80935554c50984dcba4b75eeb570cac) )
	ROM_LOAD16_BYTE( "brt11s.p4", 0x100001, 0x080000, CRC(05dd047c) SHA1(1fac14dcbcb435823eb396f75b891948cb5fc5fa) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "brt11d.p1", 0x000000, 0x080000, CRC(7c561393) SHA1(b31bed100c17d0e4f642b4061488ee404525583d) )
ROM_END


ROM_START( m5rlup )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rrusja_1.6_1", 0x000000, 0x080000, CRC(9313fcc8) SHA1(0ede6c7d93b2e9600704bc2a276342181121d5bf) )
	ROM_LOAD16_BYTE( "rrusja_1.6_2", 0x000001, 0x080000, CRC(c968b0fa) SHA1(76c6cf2eb7bc81f27e4c4255aba804f53a9b260a) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "rrusjbd1.6_1", 0x000000, 0x080000, CRC(156800c7) SHA1(d76691f0373a2bf398fa4b1e04397b787c1372e8) )
	ROM_LOAD16_BYTE( "rrusjbd1.6_2", 0x000000, 0x080000, CRC(c968b0fa) SHA1(76c6cf2eb7bc81f27e4c4255aba804f53a9b260a) )
	ROM_LOAD16_BYTE( "rrusjbg1.6_1", 0x000000, 0x080000, CRC(0b0c29a1) SHA1(9d7406c1f41f2ce109a1cd381be8b3ef416a1451) )
	ROM_LOAD16_BYTE( "rrusjbg1.6_2", 0x000000, 0x080000, CRC(c968b0fa) SHA1(76c6cf2eb7bc81f27e4c4255aba804f53a9b260a) )
ROM_END


ROM_START( m5round )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rro_sjh1.1d1", 0x000000, 0x080000, CRC(b48cbc1a) SHA1(56f715dd0a4e5196f7bd823b20e21e82dc05fa29) )
	ROM_LOAD16_BYTE( "rro_sjh1.1d2", 0x000001, 0x080000, CRC(4e9dc7b6) SHA1(058589bd7d57de8b339e34b0d27633f94b4c337a) )
ROM_END


ROM_START( m5roundl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rrd_3_2.p1", 0x000000, 0x080000, CRC(4c6b2f85) SHA1(e03eb9fd05ca0fbe2b99805f728a89d56f8db38f) )
	ROM_LOAD16_BYTE( "rrd_3_2.p2", 0x000001, 0x080000, CRC(7dc76c8b) SHA1(e4afc12799db4b9abc20bae6c5a10cb8cca46378) )
	ROM_LOAD16_BYTE( "rrd_3_2.p3", 0x100000, 0x080000, CRC(333cf38c) SHA1(234c94e081f616c801f382aacdfeadacd07e499a) )
	ROM_LOAD16_BYTE( "rrd_3_2.p4", 0x100001, 0x080000, CRC(0b3858b8) SHA1(901c1c37b4f61b96d9e2b3929b74e0583b57475f) )
ROM_END


ROM_START( m5sec7 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "se__sjk1.8_1", 0x000000, 0x080000, CRC(00d415da) SHA1(4626fc6fb1aa41695a1493335965b2e3b4544ae3) )
	ROM_LOAD16_BYTE( "se__sjk1.8_2", 0x000001, 0x080000, CRC(43a2a1a5) SHA1(3d56a64dc3a8b6625e2590b37df2bfe7b5f7b536) )
	ROM_LOAD16_BYTE( "se__sjk1.8_3", 0x100000, 0x080000, CRC(9807b332) SHA1(a955c76cc3146f381b430e7555a7a92afc60c785) )
	ROM_LOAD16_BYTE( "se__sjk1.8_4", 0x100001, 0x080000, CRC(3b6e84fe) SHA1(75060abaab2507b6901662670ef22bcf12d79652) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "se__sjh1.8d1", 0x000000, 0x080000, CRC(4f0fdcba) SHA1(0da92771a121b3fce30749de13a821f6086fcff0) )
	ROM_LOAD16_BYTE( "se__sjh1.8d2", 0x000000, 0x080000, CRC(43a2a1a5) SHA1(3d56a64dc3a8b6625e2590b37df2bfe7b5f7b536) )
ROM_END

ROM_START( m5sec7a )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sec_7s.p1", 0x00000, 0x080000, CRC(c0b3b295) SHA1(450af7c534cc7e21ecf8712f77160114925ffaf1) )
	ROM_LOAD16_BYTE( "sec_7s.p2", 0x00001, 0x080000, CRC(bd0fa3b8) SHA1(ed5ca97d2f3c5af283db79ee67b3607ebeddb777) )
	/* 3+4 */
ROM_END



ROM_START( m5sil7 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sv__sja1.2_1", 0x000000, 0x080000, CRC(ba011ed5) SHA1(a697008f8f209747eb0a4e08e5b7f2673acf10e4) )
	ROM_LOAD16_BYTE( "sv__sja1.2_2", 0x000001, 0x080000, CRC(d6e3f6cb) SHA1(d08f8f39920afdea1f0c42bc273d346f4dc42249) )
	ROM_LOAD16_BYTE( "sv__sja1.2_3", 0x100000, 0x080000, CRC(9807b332) SHA1(a955c76cc3146f381b430e7555a7a92afc60c785) )
	ROM_LOAD16_BYTE( "sv__sja1.2_4", 0x100001, 0x080000, CRC(3b6e84fe) SHA1(75060abaab2507b6901662670ef22bcf12d79652) )
ROM_END

ROM_START( m5sil7a )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "silv_7.p1", 0x00000, 0x080000, CRC(ceb61409) SHA1(f7a75bdbcbfc379f823b5268c9aad5055c226af1) )
	ROM_LOAD16_BYTE( "silv_7.p2", 0x00001, 0x080000, CRC(bd0fa3b8) SHA1(ed5ca97d2f3c5af283db79ee67b3607ebeddb777) )
	/* 3+4 */
ROM_END


ROM_START( m5smobik )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sbisj__1.1_1", 0x000000, 0x080000, CRC(ba134131) SHA1(a68c21a942a8f5d3cad2bd62f83f3ca4e671d555) )
	ROM_LOAD16_BYTE( "sbisj__1.1_2", 0x000001, 0x080000, CRC(4c60f324) SHA1(b744f447ebd88a1e193f3b33b289042c10705837) )
ROM_END

ROM_START( m5smobik12 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sbisj__1.2_1", 0x000000, 0x080000, CRC(55b572ee) SHA1(1ecfe74b9e440261e018f14a493e40a4ac785ec8) )
	ROM_LOAD16_BYTE( "sbisj__1.2_2", 0x000001, 0x080000, CRC(518ddc56) SHA1(2977f8901bfd1c6190fe31cfbf0226d0f6c0da62) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "sbisj_d1.2_1", 0x000000, 0x080000, CRC(3980cb60) SHA1(1eb444269090c6b56fb634be83eba5025857eac6) )
	ROM_LOAD16_BYTE( "sbisj_d1.2_2", 0x000000, 0x080000, CRC(518ddc56) SHA1(2977f8901bfd1c6190fe31cfbf0226d0f6c0da62) )
ROM_END

ROM_START( m5scfinl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "scd_2_1.p1", 0x000000, 0x080000, CRC(f3e3d1f6) SHA1(7f8f090e8f1c916160923ea51935567b46af931a) )
	ROM_LOAD16_BYTE( "scd_2_1.p2", 0x000001, 0x080000, CRC(6ad1db65) SHA1(f315f04667d5ba75c1f10c00cb1d4ccaceff21ee) )
ROM_END

ROM_START( m5tictac ) // tic_tac_tut_(vivid)_[c01_800_25jp].hex
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "m5tictac_p1", 0x000000, 0x080000, CRC(e9a93b5f) SHA1(bac44df1f11f185a91ff4e0d8c897a91f467ec01) )
	ROM_LOAD16_BYTE( "m5tictac_p2", 0x000001, 0x080000, CRC(f13feda3) SHA1(663a6bd8321a8fe4f397122b0afa3e87b33ddcc4) )
	ROM_LOAD16_BYTE( "m5tictac_p3", 0x100000, 0x080000, CRC(015ca199) SHA1(397c4f8e25be24f9551c72bce548c9c0cb9702a0) )
	ROM_LOAD16_BYTE( "m5tictac_p4", 0x100001, 0x080000, CRC(81bf32ed) SHA1(8ae7a18b4dcef78f7058e92a271a69ab61b6a062) )
	ROM_LOAD16_BYTE( "m5tictac_p5", 0x200000, 0x080000, CRC(0b789a92) SHA1(6d712b89359983434b4833fa6a4b7cacfc3968ff) )
	ROM_LOAD16_BYTE( "m5tictac_p6", 0x200001, 0x080000, CRC(f13feda3) SHA1(663a6bd8321a8fe4f397122b0afa3e87b33ddcc4) )
ROM_END

ROM_START( m5tictacbwb )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ttusjbg2.0_1", 0x000000, 0x080000, CRC(7c87424d) SHA1(b74068fd1e578023347b3abe78459e9b4d676c59) )
	ROM_LOAD16_BYTE( "ttusjbg2.0_2", 0x000001, 0x080000, CRC(17ec73b0) SHA1(fb372b33d6741c04e467b60afcd69e69d20278b6) )
	ROM_LOAD16_BYTE( "ttusjbg2.0_3", 0x100000, 0x080000, CRC(faa7814f) SHA1(fd3c72dc1e1398a8854ea5ebf326f79d2c4ec6e1) )
	ROM_LOAD16_BYTE( "ttusjbg2.0_4", 0x100001, 0x080000, CRC(c4275afa) SHA1(8ac4b7ba83e9d30f998544b53bc14e76930f2556) )
ROM_END

ROM_START( m5tictacbwb16 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ttusjbg1.6_1", 0x000000, 0x080000, CRC(de58b053) SHA1(9ccac27c77d57ef3ff235d385d2504497e6435a6) )
	ROM_LOAD16_BYTE( "ttusjbg1.6_2", 0x000001, 0x080000, CRC(62fd9768) SHA1(a6a682b8cc39e7d9e601e0e4fa5db61c3d5e5deb) )
	/* 3+4 */
ROM_END


ROM_START( m5xena )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "xen_sjs1.0_1", 0x000000, 0x080000, CRC(9b83d8e5) SHA1(41bb38ee51b787bd79b295ab555dbecab285b346) )
	ROM_LOAD16_BYTE( "xen_sjs1.0_2", 0x000001, 0x080000, CRC(fd9fb843) SHA1(39e7484cc8c79044e8b563ccd05778fe4812a15f) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "xen_sjs1.0d1", 0x000000, 0x080000, CRC(6f6eab6c) SHA1(a78575c1c448abc99afd0e29324197268aa21528) )
	ROM_LOAD16_BYTE( "xen_sjs1.0d2", 0x000000, 0x080000, CRC(fd9fb843) SHA1(39e7484cc8c79044e8b563ccd05778fe4812a15f) )
ROM_END






ROM_START( m5whdres )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "wdares.p1", 0x000000, 0x080000, CRC(321bbfe4) SHA1(d75e3527e4bd3e3c8f4ede4c3c8d95e37e5a90f4) )
	ROM_LOAD16_BYTE( "wdares.p2", 0x000001, 0x080000, CRC(c0fac58e) SHA1(3aef7067d73cfe36383dbdf69bdc7dc5ad7dacae) )
ROM_END

ROM_START( m5winway )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "wwa_sjs1.4_1", 0x000000, 0x080000, CRC(1a48966f) SHA1(694541a767cb62cecaa8d3d17245379f8f9425f8) )
	ROM_LOAD16_BYTE( "wwa_sjs1.4_2", 0x000001, 0x080000, CRC(cf7b46d3) SHA1(b2b10f10b2757043ce0a45a9f304e8444771ad4d) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	ROM_LOAD16_BYTE( "wwa_86_1.4_1", 0x000000, 0x080000, CRC(03330ce3) SHA1(2b75c694db6dc3d1eea19aea6afe2a8b06b78ff4) )
	ROM_LOAD16_BYTE( "wwa_86_1.4d1", 0x000000, 0x080000, CRC(a1b991fc) SHA1(c584af1f304725d9a187407c737deba69b59f1df) )
	ROM_LOAD16_BYTE( "wwa_ga1.4_1", 0x000000, 0x080000, CRC(2d33579b) SHA1(bfbfc308f180ad0d658b11ad542954057fc568ec) )
	ROM_LOAD16_BYTE( "wwa_ge1.4_1", 0x000000, 0x080000, CRC(21dd1d19) SHA1(b1422b770e7c4aa24e776a6c91d3494ef4c4e3a9) )
	ROM_LOAD16_BYTE( "wwa_gg1.4_1", 0x000000, 0x080000, CRC(93f70cb7) SHA1(9fc3ca9a57158d6897cf4fd13b0f33ee4e30f7d1) )
	ROM_LOAD16_BYTE( "wwa_gj1.4_1", 0x000000, 0x080000, CRC(81663c81) SHA1(cdd1186e3a67be824cb19c6e2e15e8b9556d52f3) )
	ROM_LOAD16_BYTE( "wwa_sjs1.4d1", 0x000000, 0x080000, CRC(09a34cae) SHA1(7f10014bac60668a6d7b4ddc7672910cc5305692) )
ROM_END


/* More Empire Sets.. */

ROM_START( m5bukroo )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "buck1_1.p1", 0x000000, 0x080000, CRC(d7723fac) SHA1(0188bb0b1b8ef42f9e2a23bcecba5ff2cab3231d) )
	ROM_LOAD16_BYTE( "buck1_1.p2", 0x000001, 0x080000, CRC(f409a41d) SHA1(88c77751627779e29a7d96527a020cae203a328c) )
ROM_END


ROM_START( m5coloss )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "colo1_1.p1", 0x000000, 0x080000, CRC(f3bffd91) SHA1(12026c468b74216519abf979daf1547d58cac870) )
	ROM_LOAD16_BYTE( "colo1_1.p2", 0x000001, 0x080000, CRC(7157bb5e) SHA1(3c208ced69e48b0fd236e5acb8f5d355cdcd1c8e) )
ROM_END



ROM_START( m5fatcat )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fcat0_2.p1", 0x000000, 0x080000, CRC(7389859a) SHA1(6e54c2623c749b8559d329acc49c0bb6b0f0afff) )
	ROM_LOAD16_BYTE( "fcat0_2.p2", 0x000001, 0x080000, CRC(b2c81b32) SHA1(69fcb8d89576a606c42af19b3988e1166b4e3d48) )
ROM_END



ROM_START( m5jmpgmc )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cgem1_0.p1", 0x000000, 0x080000, CRC(792fb276) SHA1(461c548c580eba1f4517abf06555a63aceb0e9f7) )
	ROM_LOAD16_BYTE( "cgem1_0.p2", 0x000001, 0x080000, CRC(6d07827c) SHA1(154a5e87c87db4fc48074d35f32c0fbcb536f476) )
ROM_END


ROM_START( m5tomb )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tomb0_3.p1", 0x000000, 0x080000, CRC(33363c98) SHA1(228d92041a6f7069281a32203ac8c392f036d94e) )
	ROM_LOAD16_BYTE( "tomb0_3.p2", 0x000001, 0x080000, CRC(6cc4b7b3) SHA1(69923823a363ea3b10c6a92b633841df7625b211) )
ROM_END


ROM_START( m5monjok )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "barcrestespanasl_monedinjoker_p1.bin", 0x000000, 0x080000, CRC(924bc734) SHA1(3cc2f865cf0d61131e19be89b94c7fe7a05dbb9e) )
	ROM_LOAD16_BYTE( "barcrestespanasl_monedinjoker_p2.bin", 0x000001, 0x080000, CRC(bc9f3dbe) SHA1(95b90c5f13e9a7784e94469f53fed81338fc7f4c) )
ROM_END

ROM_START( m5monjoka )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "jok05n_p1.bin", 0x000000, 0x080000, CRC(9295683a) SHA1(2a148a4eee9d3cfafbb9f0cb998b0d1b5f6e99bd) )
	ROM_LOAD16_BYTE( "jok05n_p2.bin", 0x000001, 0x080000, NO_DUMP ) // missing?
	ROM_LOAD16_BYTE( "jok00ss_p3.bin", 0x100000, 0x080000, CRC(577ae7fe) SHA1(f79a5f42ae1f1fbf762aff993ddd1b6b109f3e6b) )
	ROM_LOAD16_BYTE( "jok00ss_p4.bin", 0x100001, 0x080000, CRC(4afc0bab) SHA1(82a71ac300c708eef249aed6fa3554014b7203f8) )
ROM_END


ROM_START( m5spins )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "spin0_1.p1", 0x000000, 0x080000, CRC(92e73a27) SHA1(9f247f4bbd0f0bfe0b18bbc820939821a5e12aae) )
	ROM_LOAD16_BYTE( "spin0_1.p2", 0x000001, 0x080000, CRC(51d76f77) SHA1(47a7205bded87657e202b5dd3f842773ec300126) )
	ROM_LOAD16_BYTE( "spin0_1.p3", 0x100000, 0x080000, CRC(e2bbd587) SHA1(038749be1b583d4d0d880ca97b91100458654be0) )
	ROM_LOAD16_BYTE( "spin0_1.p4", 0x100001, 0x080000, CRC(df150570) SHA1(b635ff9884e0d60ca5828f0d4d84540ed05f1f32) )
ROM_END

ROM_START( m5costa )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "costa_de.p1", 0x000000, 0x080000, CRC(c1f69be5) SHA1(a254d4318f32947da034a9590e13487b1c9bf447) )
	ROM_LOAD16_BYTE( "costa_de.p2", 0x000001, 0x080000, CRC(79f4006b) SHA1(6fb13057a32024c62383339aea24169e0580000f) )
	ROM_LOAD16_BYTE( "costa_de.p3", 0x100000, 0x080000, CRC(3e5d89ae) SHA1(8ef36524fa3c1d933c6047f5257d285aa48a03c0) )
	ROM_LOAD16_BYTE( "costa_de.p4", 0x100001, 0x080000, CRC(dbe1362a) SHA1(5e148a08eb720015e154bd9dd41e283233d72a14) )
ROM_END



ROM_START( m5dbubl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "dbubble.p1", 0x000000, 0x080000, CRC(ea046f04) SHA1(ec5b24fbd4df03c61f3ab9ca52661d9aab0bdef8) )
	ROM_LOAD16_BYTE( "dbubble.p2", 0x000001, 0x080000, CRC(a040b387) SHA1(8bb14da05dc0762c4b3829bacf6f325f1e94e2c6) )
	ROM_LOAD16_BYTE( "dbubble.p3", 0x100000, 0x080000, CRC(985f7267) SHA1(05a9aa5353b8830663f86971ffc819f01d20824d) )
	ROM_LOAD16_BYTE( "dbubble.p4", 0x100001, 0x080000, CRC(9b43f59d) SHA1(acb8d1f66c68f23c3989ba8f35af43f7b5352418) )
ROM_END


ROM_START( m5dragnd )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "dragon_d.p1", 0x000000, 0x080000, CRC(ae4f778b) SHA1(0fab7173f9da64fb5ff15452aa9249ebefd40ae2) )
	ROM_LOAD16_BYTE( "dragon_d.p2", 0x000001, 0x080000, CRC(d56f498c) SHA1(01ddeba4154826f4608ae7c37256e43d55210120) )
ROM_END

ROM_START( m5dragnda )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "drop_pound5.p1", 0x000000, 0x080000, CRC(197bc0b3) SHA1(02949cd86a4d4569a540d11f8bda1d49d4e94c2f) )
	ROM_LOAD16_BYTE( "drop_pound5.p2", 0x000001, 0x080000, CRC(cdb76803) SHA1(69534f33ab2dac80049c566d385cf67a9feff343) )
ROM_END

ROM_START( m5ttwo )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "take_two.p1", 0x000000, 0x080000, CRC(16b0dbc3) SHA1(c8893115c2bbdaaf20af2a2a36b414539342f3e5) )
	ROM_LOAD16_BYTE( "take_two.p2", 0x000001, 0x080000, CRC(406f8212) SHA1(afa5016b5689787ee076a1f243b9f7516aad1fc6) )
	ROM_LOAD16_BYTE( "take_two.p3", 0x100000, 0x080000, CRC(5517f852) SHA1(dc90c1279c65d6040391abe4499bd39abdd7505b) )
	ROM_LOAD16_BYTE( "take_two.p4", 0x100001, 0x080000, CRC(23cf044c) SHA1(dc168cd2f06fbab5822a35ea87966b351ba21e3c) )
ROM_END

ROM_START( m5zigzag )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "zigzag.p1", 0x000000, 0x080000, CRC(68f4e9ab) SHA1(b22063976cf971dbbff9adb0d14c4eda5f6a9e23) )
	ROM_LOAD16_BYTE( "zigzag.p2", 0x000001, 0x080000, CRC(8c237c1d) SHA1(e5332000569bfa1ef8363f4e2825e508eb2ab792) )
	ROM_LOAD16_BYTE( "zigzag.p3", 0x100000, 0x080000, CRC(e34e8da2) SHA1(d1d38881b8f90de73058cad429d913b5676a2712) )
	ROM_LOAD16_BYTE( "zigzag.p4", 0x100001, 0x080000, CRC(a82600af) SHA1(4d2bf77a15eaf57fa0b020eadcd315f5e32eff5a) )
ROM_END

ROM_START( m5rawin )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "r_a_win.p1", 0x00000, 0x080000, CRC(af94dff4) SHA1(4415eaab3cf3810367d4ea7a4ab79f8f5bd2cde1) )
	ROM_LOAD16_BYTE( "r_a_win.p2", 0x00001, 0x080000, CRC(30be41d4) SHA1(bb3e972b2b885d28eae641dc44823f57eb654538) )
ROM_END

ROM_START( m5horn )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "horn_0f.p1", 0x00000, 0x080000, CRC(c01732dd) SHA1(6b157ad01535d8c7bbf6c3b2db96e7fed271f5f9) )
	ROM_LOAD16_BYTE( "horn_of.p2", 0x00001, 0x080000, CRC(8b270edf) SHA1(0d6011bdd4c38f97813dae187bf2c67f09066de5) )
ROM_END

ROM_START( m5hotrk )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hot_rock.p1", 0x00000, 0x080000, CRC(cef5b5ae) SHA1(1ed34ea307c40bdfd0ea7296b9a2be4d3ecc796c) )
	ROM_LOAD16_BYTE( "hot_rock.p2", 0x00001, 0x080000, CRC(c90efc7b) SHA1(6f82a7d31f9b891b13879ed40fc2f47829422e7e) )
ROM_END


ROM_START( m5cmass )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "c_mass",   0x00000, 0x080000, CRC(96427dfc) SHA1(762cccf773bc81b06e6197492207f91385002c7b) )
	ROM_LOAD16_BYTE( "c_mass.p2", 0x00001, 0x080000, CRC(837930ac) SHA1(5b5ae91b13dd7ae671fb60df3e7545c8239e7dec) )
ROM_END

ROM_START( m5slide )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "slider.p1", 0x00000, 0x080000, CRC(7b75e001) SHA1(b6cdeddc16d399e77ff9ba53e1603273ecbe23c6) )
	ROM_LOAD16_BYTE( "slider.p2", 0x00001, 0x080000, CRC(963267bf) SHA1(17541a075733184fee0c4b3acdb6d60407e31a64) )
ROM_END





/* Barcrest */
GAME(  199?, m5clr,       0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","MPU 5 Ram & Meter Clear (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5tst,       0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","MPU 5 Test Rom (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5addams,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Addams Family (Barcrest) (MPU5) (v0.5, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5addams )
GAMEL( 199?, m5addamsa,   m5addams, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Addams Family (Barcrest) (MPU5) (v0.5, set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5addams )
GAMEL( 199?, m5addamsb,   m5addams, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Addams Family (Barcrest) (MPU5) (v0.5, set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5addams )
GAMEL( 199?, m5addamsc,   m5addams, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Addams Family (Barcrest) (MPU5) (v0.5, set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5addams )
GAMEL( 199?, m5addamsd,   m5addams, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Addams Family (Barcrest) (MPU5) (v0.5, set 5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5addams )
GAMEL( 199?, m5addamse,   m5addams, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Addams Family (Barcrest) (MPU5) (v0.5, set 6)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5addams )
GAMEL( 199?, m5addamsf,   m5addams, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Addams Family (Barcrest) (MPU5) (v0.5, set 7)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5addams )
GAMEL( 199?, m5addamsg,   m5addams, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Addams Family (Barcrest) (MPU5) (v0.5, set 8)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5addams )
GAMEL( 199?, m5addamsh,   m5addams, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Addams Family (Barcrest) (MPU5) (v0.2, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5addams )
GAMEL( 199?, m5addamsi,   m5addams, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Addams Family (Barcrest) (MPU5) (v0.2, set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5addams )
GAMEL( 199?, m5addamsj,   m5addams, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Addams Family (Barcrest) (MPU5) (v0.2, set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5addams )
GAMEL( 199?, m5addamsk,   m5addams, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Addams Family (Barcrest) (MPU5) (v0.3, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5addams )
GAMEL( 199?, m5addamsl,   m5addams, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Addams Family (Barcrest) (MPU5) (v0.3, set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5addams )
GAMEL( 199?, m5addamsm,   m5addams, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Addams Family (Barcrest) (MPU5) (v0.3, set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5addams )
GAMEL( 199?, m5addamsn,   m5addams, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Addams Family (Barcrest) (MPU5) (v0.3, set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5addams )
GAMEL( 199?, m5addamso,   m5addams, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Addams Family (Barcrest) (MPU5) (v0.3, set 5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5addams )
GAMEL( 199?, m5addamsp,   m5addams, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Addams Family (Barcrest) (MPU5) (v0.3, set 6)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5addams )
GAMEL( 199?, m5addamsq,   m5addams, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Addams Family (Barcrest) (MPU5) (v0.3, set 7)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5addams )
GAMEL( 199?, m5addamsr,   m5addams, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Addams Family (Barcrest) (MPU5) (v0.3, set 8)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5addams )
GAMEL( 199?, m5addamss,   m5addams, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Addams Family (Barcrest) (MPU5) (v0.3, set 9)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5addams )

GAME(  199?, m5addlad,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Adders & Ladders (Barcrest) (MPU5, v0.6, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5addlada,   m5addlad, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Adders & Ladders (Barcrest) (MPU5, v0.6, set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5addladb,   m5addlad, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Adders & Ladders (Barcrest) (MPU5, v0.6, set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5addladc,   m5addlad, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Adders & Ladders (Barcrest) (MPU5, v0.6, set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5addladd,   m5addlad, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Adders & Ladders (Barcrest) (MPU5, v0.6, set 5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5addlade,   m5addlad, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Adders & Ladders (Barcrest) (MPU5, v0.6, set 6)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5addladf,   m5addlad, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Adders & Ladders (Barcrest) (MPU5, v0.6, set 7)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5addladg,   m5addlad, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Adders & Ladders (Barcrest) (MPU5, v0.6, set 8)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5addladh,   m5addlad, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Adders & Ladders (Barcrest) (MPU5, v0.6, set 9)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5addladi,   m5addlad, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Adders & Ladders (Barcrest) (MPU5, v0.6, set 10)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5addladj,   m5addlad, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Adders & Ladders (Barcrest) (MPU5, v0.1, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5addladk,   m5addlad, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Adders & Ladders (Barcrest) (MPU5, v0.1, set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5addladl,   m5addlad, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Adders & Ladders (Barcrest) (MPU5, v0.1, set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5addladm,   m5addlad, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Adders & Ladders (Barcrest) (MPU5, v0.1, set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5addladn,   m5addlad, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Adders & Ladders (Barcrest) (MPU5, v0.1, set 5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5addlado,   m5addlad, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Adders & Ladders (Barcrest) (MPU5, v0.1, set 6)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5addladp,   m5addlad, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Adders & Ladders (Barcrest) (MPU5, v0.1, set 7)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5addladq,   m5addlad, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Adders & Ladders (Barcrest) (MPU5, v0.4, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5addladr,   m5addlad, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Adders & Ladders (Barcrest) (MPU5, v0.4, set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5addlads,   m5addlad, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Adders & Ladders (Barcrest) (MPU5, v?.?)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )


GAMEL( 199?, m5fire,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","All Fired Up (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5fire )

GAMEL( 199?, m5arab,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Arabian Nights (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5arab )
GAMEL( 199?, m5arab03,    m5arab,   mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Arabian Nights (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5arab )

GAMEL( 199?, m5austin,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Austin Powers (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5austin11 )
GAMEL( 199?, m5austin10,  m5austin, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Austin Powers (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5austin11 )
GAMEL( 199?, m5austin11,  m5austin, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Austin Powers (Barcrest) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5austin11 )

GAME(  199?, m5bankrl,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","The Bank Roll (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5barkng,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Barking Mad (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5barkng )

GAMEL( 199?, m5barmy,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Barmy Army (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5barmy )

GAMEL( 199?, m5baxe,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Battle Axe (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5baxe04 )
GAMEL( 199?, m5baxe04,    m5baxe,   mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Battle Axe (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5baxe04 )

GAMEL( 199?, m5bbro,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Big Brother (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5bbro )
GAMEL( 199?, m5bbro02,    m5bbro,   mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Big Brother (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5bbro )

GAMEL( 199?, m5bbrocl,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Big Brother Club (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5bbrocl )

GAMEL( 199?, m5bigchs,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","The Big Cheese (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5bigchs )
GAMEL( 199?, m5bigchs05,  m5bigchs, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","The Big Cheese (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5bigchs )

GAMEL( 199?, m5biggam,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","The Big Game (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5biggam )
GAMEL( 199?, m5biggam11,  m5biggam, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","The Big Game (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5biggam )

GAMEL( 199?, m5blkwht,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Black & White (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5blkwht11 )
GAMEL( 199?, m5blkwht11,  m5blkwht, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Black & White (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5blkwht11 )
GAMEL( 199?, m5blkwht01,  m5blkwht, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Black & White (Barcrest) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5blkwht11 )

GAMEL( 199?, m5bwaves,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Brain Waves (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5bwaves )
GAMEL( 199?, m5bwaves07,  m5bwaves, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Brain Waves (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5bwaves )

GAMEL( 199?, m5bling,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Bling King Crazy (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5bling )

GAMEL( 199?, m5cbw,       0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Ca$h Bang Wallop (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5cbw )
GAMEL( 199?, m5cbwa,      m5cbw,    mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Ca$h Bang Wallop (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5cbw )

GAME(  199?, m5cpcash,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Captain Cash (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5carclb,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Caribbean Club (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5cashar,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Cash Arena (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5cashar04,  m5cashar, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Cash Arena (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5cashat,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Cash Attack (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5cashat )

GAME(  199?, m5cashln,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Cash Lines (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5cashrn,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Cash Run (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5cashrn )
GAMEL( 199?, m5cashrn01,  m5cashrn, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Cash Run (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5cashrn )
GAMEL( 199?, m5cashrn02,  m5cashrn, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Cash Run (Barcrest) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5cashrn )
GAMEL( 199?, m5cashrn04,  m5cashrn, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Cash Run (Barcrest) (MPU5) (set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5cashrn )

GAMEL( 199?, m5codft,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","The Codfather (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5codft )
GAMEL( 199?, m5codft02,   m5codft,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","The Codfather (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5codft )

GAME(  199?, m5cos,       0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Costa Del Cash Casino (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5cosclb,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Costa Del Cash Club (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5cosclb )

GAMEL( 199?, m5crzkni,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Crazy Crazy Knights (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5crzkni )
GAMEL( 199?, m5crzkni03,  m5crzkni, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Crazy Crazy Knights (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5crzkni )

GAMEL( 199?, m5doshpk,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Do$h 'n' Pecks (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5doshpk05 )
GAMEL( 199?, m5doshpk05,  m5doshpk, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Do$h 'n' Pecks (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5doshpk05 )

GAME(  199?, m5draclb,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Ooh Aah Dracula Club (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5draclb07,  m5draclb, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Ooh Aah Dracula Club (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5draclb01,  m5draclb, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Ooh Aah Dracula Club (Barcrest) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5ewn,       0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Each Way Nudge (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5ewn )
GAMEL( 199?, m5ewn08,     m5ewn,    mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Each Way Nudge (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5ewn )

GAMEL( 199?, m5elim,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Eliminator (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5elim )
GAMEL( 199?, m5elim03,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Eliminator (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5elim )
GAMEL( 199?, m5elim04,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Eliminator (Barcrest) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5elim )

GAMEL( 199?, m5egr,       0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Elvis Gold Rush (Barcrest) (MPU5, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5egr )
GAMEL( 199?, m5egra,      m5egr,    mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Elvis Gold Rush (Barcrest) (MPU5, set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5egr )

GAME(  199?, m5egss,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Elvis Gold Super Streak (Barcrest) (MPU5, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5egssa,     m5egss,   mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Elvis Gold Super Streak (Barcrest) (MPU5, set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5evgrhr,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Elvis Gold Red Hot Roll (Barcrest) (MPU5, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5evgrhr )
GAMEL( 199?, m5evgrhra,   m5evgrhr, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Elvis Gold Red Hot Roll (Barcrest) (MPU5, set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5evgrhr )

GAMEL( 199?, m5xchn,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Exchanges Unlimited (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5xchn )

GAMEL( 199?, m5firebl,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Fireball (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5firebl )

GAMEL( 199?, m5flipcr,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Flippin Crazy (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5flipcr )

GAMEL( 199?, m5fortby,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Fort Boyard (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5fortby )
GAMEL( 199?, m5fortby01,  m5fortby, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Fort Boyard (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5fortby )

GAMEL( 199?, m5frnzy,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Frenzy (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5frnzy )
GAMEL( 199?, m5frnzya,    m5frnzy,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Frenzy (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5frnzy )

GAMEL( 199?, m5beans,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Full Of Beans (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5beansa )
GAMEL( 199?, m5beansa,    m5beans,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Full Of Beans (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5beansa )

GAMEL( 199?, m5funsun,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Fun In The Sun (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5funsun )
GAMEL( 199?, m5funsun03,  m5funsun, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Fun In The Sun (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5funsun )

GAMEL( 199?, m5gimmie,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Gimmie Gimmie Gimmie (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5gimmie )

GAMEL( 199?, m5grush,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Gold Rush (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5grush )
GAMEL( 199?, m5grush10,   m5grush,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Gold Rush (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5grush )
GAMEL( 199?, m5grush04,   m5grush,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Gold Rush (Barcrest) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5grush )
GAMEL( 199?, m5grush03,   m5grush,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Gold Rush (Barcrest) (MPU5) (set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5grush )
GAMEL( 199?, m5grush02,   m5grush,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Gold Rush (Barcrest) (MPU5) (set 5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5grush )
GAMEL( 199?, m5grush01,   m5grush,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Gold Rush (Barcrest) (MPU5) (set 6)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5grush )

GAMEL( 199?, m5grush5,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Gold Rush Five Liner (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5grush5 )
GAMEL( 199?, m5grush504,  m5grush5, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Gold Rush Five Liner (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5grush5 )

GAME(  199?, m5gruss,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Gold Rush Sit Down (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5grusst,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Gold Rush Stampede (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5grusst04,  m5grusst, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Gold Rush Stampede (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5grusst03,  m5grusst, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Gold Rush Stampede (Barcrest) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5gstrik,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Gold Strike (Barcrest) (MPU5) (set 1)",                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5gstrik )
GAMEL( 199?, m5gstriks,   m5gstrik, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Gold Strike (Barcrest) (MPU5) (V1.00, Spanish, Bilso S.A.)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5gstrik )
GAMEL( 199?, m5gstrik11,  m5gstrik, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Gold Strike (Barcrest) (MPU5) (set 2)",                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5gstrik )
GAMEL( 199?, m5gstrik02,  m5gstrik, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Gold Strike (Barcrest) (MPU5) (set 3)",                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5gstrik )
GAMEL( 199?, m5gstrik01,  m5gstrik, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Gold Strike (Barcrest) (MPU5) (set 4)",                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5gstrik )
GAMEL( 199?, m5gstrik01a, m5gstrik, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Gold Strike (Barcrest) (MPU5) (set 5)",                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5gstrik )
GAMEL( 199?, m5gstrika,   m5gstrik, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Gold Strike (Barcrest) (MPU5) (set 6)",                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5gstrik )

GAMEL( 199?, m5gsstrk,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Gold Super Streak (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5gsstrk07 )
GAMEL( 199?, m5gsstrk07,  m5gsstrk, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Gold Super Streak (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5gsstrk07 )

GAMEL( 199?, m5gdrag,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Golden Dragon (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5gdrag )

GAME(  199?, m5gdrgcl,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Golden Dragon Club (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5gdrgcl05,  m5gdrgcl, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Golden Dragon Club (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5gkeys,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Golden Keys (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5hellrz,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Hellraiser (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5hellrz )

GAMEL( 199?, m5hlsumo,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Hi Lo Sumo (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5hlsumo )

GAMEL( 199?, m5hifly,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","High Flyer (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5hifly )
GAMEL( 199?, m5hifly03,   m5hifly,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","High Flyer (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5hifly )
GAMEL( 199?, m5hifly04,   m5hifly,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","High Flyer (Barcrest) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5hifly )

GAMEL( 199?, m5holy,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","The Holy Grail (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5holy )
GAMEL( 199?, m5holy10,    m5holy,   mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","The Holy Grail (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5holy )

GAMEL( 199?, m5hotslt,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Hot Slot (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5hotslt )

GAMEL( 199?, m5hotstf,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Hot Stuff (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5hotstf )

GAMEL( 199?, m5hypvip,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Hyper Viper (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5hypvip )

GAMEL( 199?, m5jackpt,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Jackpoteers (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5jackpt )
GAMEL( 199?, m5jackpt07,  m5jackpt, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Jackpoteers (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5jackpt )

GAMEL( 199?, m5jackp2,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Jackpoteers 2 (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5jackp2 )
GAMEL( 199?, m5jackp2a,   m5jackp2, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Jackpoteers 2 (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5jackp2 )

GAMEL( 199?, m5jlyjwl,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Jolly Jewels (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5jlyjwl )
GAMEL( 199?, m5jlyjwl01,  m5jlyjwl, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Jolly Jewels (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5jlyjwl )
GAMEL( 199?, m5jlyjwl02,  m5jlyjwl, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Jolly Jewels (Barcrest) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5jlyjwl )

GAME(  199?, m5jlyrog,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Jolly Roger (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5jlyroga,   m5jlyrog, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Jolly Roger (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5kkebab,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","King Kebab (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5kkebab )
GAMEL( 199?, m5kkebab10,  m5kkebab, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","King Kebab (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5kkebab )
GAMEL( 199?, m5kkebaba,   m5kkebab, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","King Kebab (Barcrest) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5kkebab )

GAME(  199?, m5kingko,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","King KO (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5kingko04,  m5kingko, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","King KO (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5kingko05,  m5kingko, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","King KO (Barcrest) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5lotta,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Lotta Luck (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5lotta )

GAMEL( 199?, m5mega,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Mega Zone (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5mega )

GAMEL( 199?, m5martns,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Money Mad Martians (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5martns07 )
GAMEL( 199?, m5martns07,  m5martns, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Money Mad Martians (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5martns07 )

GAMEL( 199?, m5mmak,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Money Maker (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5mmak06 )
GAMEL( 199?, m5mmak06,    m5mmak,   mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Money Maker (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5mmak06 )

GAME(  199?, m5monjok,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Monedin Joker (Spanish) (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5monjoka,   m5monjok, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Monedin Joker (Spanish) (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5monty,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Monty Python (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5mprio,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Monty Python Rio (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5mprio )

GAMEL( 199?, m5mpfc,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Monty Python's Flying Circus (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5mpfc )

GAME(  199?, m5mpfccl,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Monty Python's Flying Circus Club (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5neptun,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Neptunes Treasure (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5neptun )

GAMEL( 199?, m5nnww,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5nnww )

GAME(  199?, m5nnwwgl,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Nudge Nudge Wink Wink Gold (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5fiddle,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","On The Fiddle (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5fiddle )
GAMEL( 199?, m5fiddle03,  m5fiddle, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","On The Fiddle (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5fiddle )

GAMEL( 199?, m5oohaah,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Ooh Aah Dracula (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5oohaah )
GAMEL( 199?, m5oohaah01,  m5oohaah, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Ooh Aah Dracula (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5oohaah )

GAMEL( 199?, m5oohrio,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Ooh Ahh Dracula Rio (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5oohrio )

GAMEL( 199?, m5openbx,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Open The Box (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5openbx05 )
GAMEL( 199?, m5openbx06,  m5openbx, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Open The Box (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5openbx05 )
GAMEL( 199?, m5openbx05,  m5openbx, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Open The Box (Barcrest) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5openbx05 )
GAMEL( 199?, m5openbx01,  m5openbx, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Open The Box (Barcrest) (MPU5) (set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5openbx05 )

GAMEL( 199?, m5overld,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Overload (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5overld )
GAMEL( 199?, m5overld02,  m5overld, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Overload (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5overld )
GAMEL( 199?, m5overld10,  m5overld, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Overload (Barcrest) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5overld )
GAMEL( 199?, m5overld11,  m5overld, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Overload (Barcrest) (MPU5) (set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5overld )

GAMEL( 199?, m5ptyani,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Party Animal (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5ptyani )
GAMEL( 199?, m5ptyani01,  m5ptyani, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Party Animal (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5ptyani )

GAMEL( 199?, m5peepsh,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Peep Show (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5peepsh )

GAME(  199?, m5psy2,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Psycho Cash Beast 2 (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5qshot,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Quack Shot (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5qshot04 )
GAMEL( 199?, m5qshot04,   m5qshot,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Quack Shot (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5qshot04 )

GAME(  199?, m5roof,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Raise The Roof (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5roofa,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Raise The Roof (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5razdz,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Razzle Dazzle Club (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5razdz10 )
GAMEL( 199?, m5razdz10,   m5razdz,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Razzle Dazzle Club (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5razdz10 )
GAMEL( 199?, m5razdz11,   m5razdz,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Razzle Dazzle Club (Barcrest) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5razdz10 )

GAMEL( 199?, m5redrck,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Ready To Rock (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5redrcka )
GAMEL( 199?, m5redrck10,  m5redrck, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Ready To Rock (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5redrcka )
GAMEL( 199?, m5redrcka,   m5redrck, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Ready To Rock (Barcrest) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5redrcka )

GAME(  199?, m5rhkni,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Red Hot Knights (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5rhrg,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Red Hot Roll Gold (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5rhrga,     m5rhrg,   mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Red Hot Roll Gold (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5rhrgt,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Red Hot Roll Triple (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5rhrgt02 )
GAMEL( 199?, m5rhrgt12,   m5rhrgt,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Red Hot Roll Triple (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5rhrgt02 )
GAMEL( 199?, m5rhrgt02,   m5rhrgt,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Red Hot Roll Triple (Barcrest) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5rhrgt02 )

GAMEL( 199?, m5revo,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Revolution (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5revo13 )
GAMEL( 199?, m5revo13,    m5revo,   mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Revolution (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5revo13 )
GAMEL( 199?, m5revoa,     m5revo,   mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Revolution (Barcrest) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5revo13 )

GAMEL( 199?, m5rgclb,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Rio Grande Club (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5rgclb12 )
GAMEL( 199?, m5rgclb11,   m5rgclb,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Rio Grande Club (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5rgclb12 )
GAMEL( 199?, m5rgclb12,   m5rgclb,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Rio Grande Club (Barcrest) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5rgclb12 )
GAMEL( 199?, m5rgclb20,   m5rgclb,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Rio Grande Club (Barcrest) (MPU5) (set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5rgclb12 )
GAMEL( 199?, m5rgclb21,   m5rgclb,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Rio Grande Club (Barcrest) (MPU5) (set 5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5rgclb12 )
GAMEL( 199?, m5rgclb03,   m5rgclb,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Rio Grande Club (Barcrest) (MPU5) (set 6)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5rgclb12 )
GAMEL( 199?, m5rgclb01,   m5rgclb,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Rio Grande Club (Barcrest) (MPU5) (set 7)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5rgclb12 )
GAMEL( 199?, m5rgclb01a,  m5rgclb,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Rio Grande Club (Barcrest) (MPU5) (set 8)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5rgclb12 )

GAME(  199?, m5rcx,       0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Royal Exchange Club (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5rcxa,      m5rcx,    mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Royal Exchange Club (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5rub,       0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Rubies & Diamonds (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5rub )

GAMEL( 199?, m5ritj,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Rumble In The Jungle (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5ritj )

GAMEL( 199?, m5rfymc,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Run For Your Money Club (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5rfymc )
GAMEL( 199?, m5rfymc06,   m5rfymc,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Run For Your Money Club (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5rfymc )

GAMEL( 199?, m5seven,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Seven Deadly Spins (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5seven )

GAMEL( 199?, m5sheik,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Sheik Yer Money (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5sheik )

GAME(  199?, m5showtm,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Showtime (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5silver,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Silver Screen (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5silver06,  m5silver, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Silver Screen (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5silver03,  m5silver, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Silver Screen (Barcrest) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5sondr,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Son Of Dracula (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5sondra )
GAMEL( 199?, m5sondr05,   m5sondr,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Son Of Dracula (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5sondra )
GAMEL( 199?, m5sondra,    m5sondr,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Son Of Dracula (Barcrest) (MPU5) (15GBP Jackpot)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5sondra )

GAME(  199?, m5spicer,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","The Spice Is Right (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5spicer06,  m5spicer, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","The Spice Is Right (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5spiker,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Spiker The Biker (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5spiker )
GAMEL( 199?, m5spiker02,  m5spiker, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Spiker The Biker (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5spiker )
GAMEL( 199?, m5spikera,   m5spiker, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Spiker The Biker (Barcrest) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5spiker )

GAMEL( 199?, m5spins,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Spinsation (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5spins )

GAMEL( 199?, m5squids,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Squids In (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5squids06 )
GAMEL( 199?, m5squids04a, m5squids, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Squids In (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5squids06 )
GAMEL( 199?, m5squids05,  m5squids, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Squids In (Barcrest) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5squids06 )
GAMEL( 199?, m5squids06,  m5squids, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Squids In (Barcrest) (MPU5) (set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5squids06 )

GAMEL( 199?, m5stax,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Stax Of Cash (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5stax )

GAMEL( 199?, m5scharg,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Super Charged (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5scharg )
GAMEL( 199?, m5scharg05,  m5scharg, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Super Charged (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5scharg )
GAMEL( 199?, m5scharg06,  m5scharg, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Super Charged (Barcrest) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5scharg )
GAMEL( 199?, m5scharga,   m5scharg, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Super Charged (Barcrest) (MPU5) (set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5scharg )

GAME(  199?, m5supstr,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Super Star (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5supstra,   m5supstr, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Super Star (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5sstrk,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Super Streak (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5sstrk )
GAMEL( 199?, m5sstrk02a,  m5sstrk,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Super Streak (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5sstrk )

GAMEL( 199?, m5supnov,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Supernova (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5supnov )
GAMEL( 199?, m5supnova,   m5supnov, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Supernova (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5supnov )

GAME(  199?, m5tempt,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Temple Of Treasure (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5tempt05,   m5tempt,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Temple Of Treasure (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5tempta,    m5tempt,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Temple Of Treasure (Barcrest) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5temptb,    m5tempt,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Temple Of Treasure (Barcrest) (MPU5) (set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5tempt2,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Temple Of Treasure 2 (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5tempt2 )
GAMEL( 199?, m5tempt203,  m5tempt2, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Temple Of Treasure 2 (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5tempt2 )
GAMEL( 199?, m5tempt2a,   m5tempt2, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Temple Of Treasure 2 (Barcrest) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5tempt2 )

GAMEL( 199?, m5tempcl,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Temple Of Treasure Club (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5tempcl )

GAMEL( 199?, m5tbird,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Thunderbird (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5tbird )

GAME(  199?, m5topdog,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Top Dog (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5topdog04,  m5topdog, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Top Dog (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5topdoga,   m5topdog, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Top Dog (Barcrest) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5trail,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Trailblazer Club (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5trail )

GAMEL( 199?, m5ultimo,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Ultimo (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5ultimo04 )
GAMEL( 199?, m5ultimo03a, m5ultimo, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Ultimo (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5ultimo04 )
GAMEL( 199?, m5ultimo04,  m5ultimo, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Ultimo (Barcrest) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5ultimo04 )

GAMEL( 199?, m5upover,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Up & Over (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5upover )
GAMEL( 199?, m5upover15,  m5upover, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Up & Over (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5upover )

GAMEL( 199?, m5vampup,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Vamp It Up (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5vampup )

GAMEL( 199?, m5wking,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Wild King (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5wking05 )
GAMEL( 199?, m5wking05,   m5wking,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Wild King (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5wking05 )

GAME(  199?, m5costa,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Costa Del Cash (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5ttwo,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Take Two (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5horn,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Whitbread","Horn Of Plenty (Barcrest / Whitbread) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5hotrk,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Hot Rocks (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

/* Barcrest / Red Gaming */

GAME(  199?, m5ashock,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Aftershock (Barcrest - Red Gaming) (MPU5, v1.2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5ashocka,   m5ashock, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Aftershock (Barcrest - Red Gaming) (MPU5, v1.3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5bigsht,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Big Shot (Barcrest - Red Gaming) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5bigsht04,  m5bigsht, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Big Shot (Barcrest - Red Gaming) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5bigsht11,  m5bigsht, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Big Shot (Barcrest - Red Gaming) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5bigsht13,  m5bigsht, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Big Shot (Barcrest - Red Gaming) (MPU5) (set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5bigshta,   m5bigsht, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Big Shot (Barcrest - Red Gaming) (MPU5) (set 5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5bnkrs,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Bonkers (Barcrest - Red Gaming) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5bbank,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Break The Bank (Barcrest - Red Gaming) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5bbank13,   m5bbank,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Break The Bank (Barcrest - Red Gaming) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5casfev ,   0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Casino Fever (Red Gaming) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5casfev12,  m5casfev, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Casino Fever (Red Gaming) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5dmnstr,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Demon Streak (Barcrest - Red Gaming) (MPU5, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5dmnstra,   m5dmnstr, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Demon Streak (Barcrest - Red Gaming) (MPU5, set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5dbubl,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Double Bubble (Barcrest - Red Gaming) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5dragnd,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Dragon Drop (Barcrest - Red Gaming) (MPU5, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5dragnda,   m5dragnd, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Dragon Drop (Barcrest - Red Gaming) (MPU5, set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5fnfair,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Funfair (Barcrest - Red Gaming) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5fnfaird,   m5fnfair, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Funfair (Barcrest - Red Gaming) (MPU5) (Datapak)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5fusir,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Fruits U Sir (Barcrest - Red Gaming) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5fusir11,   m5fusir,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Fruits U Sir (Barcrest - Red Gaming) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5fusir12,   m5fusir,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Fruits U Sir (Barcrest - Red Gaming) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5hypalx,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Hypalinx (Barcrest - Red Gaming) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5invad,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Invaders (Barcrest - Red Gaming) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5jcptgn,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Jackpot Genie (Barcrest - Red Gaming) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5jlstrk,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Jewel Strike (Barcrest - Red Gaming) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5lock,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Lock 'n' Load (Barcrest - Red Gaming) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5lock13,    m5lock,   mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Lock 'n' Load (Barcrest - Red Gaming) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5lock12,    m5lock,   mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Lock 'n' Load (Barcrest - Red Gaming) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5lockcl,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Lock 'n' Load Club (Barcrest - Red Gaming) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5lockcl15,  m5lockcl, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Lock 'n' Load Club (Barcrest - Red Gaming) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5lockcl14,  m5lockcl, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Lock 'n' Load Club (Barcrest - Red Gaming) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5nitro,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Nitro (Barcrest - Red Gaming) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5paint,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Paint The Town Red (Barcrest - Red Gaming) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5quake,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Quake (Barcrest - Red Gaming) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5rainrn,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Rainbow Runner (Barcrest - Red Gaming) (MPU5, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5rainrna,   m5rainrn, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Rainbow Runner (Barcrest - Red Gaming) (MPU5, set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5rampg,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Rampage (Barcrest - Red Gaming) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5rampg11,   m5rampg,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Rampage (Barcrest - Red Gaming) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5rampg12,   m5rampg,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Rampage (Barcrest - Red Gaming) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5rdwarf,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Red Dwarf (Barcrest - Red Gaming) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5redx,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Red X (Barcrest - Red Gaming) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5redx12,    m5redx,   mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Red X (Barcrest - Red Gaming) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5thtsmg,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","That's Magic (Barcrest - Red Gaming) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5topdol,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Top Dollar (Barcrest - Red Gaming) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5topdola,   m5topdol, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Top Dollar (Barcrest - Red Gaming) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5zigzag,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Zig Zag (Barcrest - Red Gaming) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5cmass,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Critical Mass (Barcrest - Red Gaming) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5sblz,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Snail Blazer (Barcrest - Red Gaming) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5slide,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest / Red Gaming","Slider (Barcrest - Red Gaming) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

/* Vivid */

GAME(  199?, m5sixsht,   0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Six Shooter (Vivid) (MPU5) (v1.1, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5sixshta,  m5sixsht, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Six Shooter (Vivid) (MPU5) (v1.1, set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5sixshtb,  m5sixsht, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Six Shooter (Vivid) (MPU5) (v2.0, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5sixshtc,  m5sixsht, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Six Shooter (Vivid) (MPU5) (v2.0, set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5sixshtd,  m5sixsht, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Six Shooter (Vivid) (MPU5) (v2.0, set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5sixshte,  m5sixsht, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Six Shooter (Vivid) (MPU5) (v2.0, set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5sixshtf,  m5sixsht, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Six Shooter (Vivid) (MPU5) (v2.0, set 5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5sixshtg,  m5sixsht, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Six Shooter (Vivid) (MPU5) (v2.0, set 6)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5sixshth,  m5sixsht, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Six Shooter (Vivid) (MPU5) (v2.0, set 7)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5sixshti,  m5sixsht, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Six Shooter (Vivid) (MPU5) (v2.1, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5sixshtj,  m5sixsht, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Six Shooter (Vivid) (MPU5) (v2.1, set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5sixshtk,  m5sixsht, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Six Shooter (Vivid) (MPU5) (v2.1, set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5sixshtl,  m5sixsht, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Six Shooter (Vivid) (MPU5) (v2.1, set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5sixshtm,  m5sixsht, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Six Shooter (Vivid) (MPU5) (v2.1, set 5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5sixshtn,  m5sixsht, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Six Shooter (Vivid) (MPU5) (v2.1, set 6)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5all41,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","All 4 One (Vivid) (MPU5, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5all41d )
GAMEL( 199?, m5all41a,   m5all41,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","All 4 One (Vivid) (MPU5, set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5all41d )
GAMEL( 199?, m5all41b,   m5all41,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","All 4 One (Vivid) (MPU5, set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5all41d )
GAMEL( 199?, m5all41c,   m5all41,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","All 4 One (Vivid) (MPU5, set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5all41d )
GAMEL( 199?, m5all41d,   m5all41,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","All 4 One (Vivid) (MPU5, set 5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5all41d )
GAMEL( 199?, m5all41e,   m5all41,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","All 4 One (Vivid) (MPU5, set 6)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5all41d )
GAMEL( 199?, m5all41f,   m5all41,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","All 4 One (Vivid) (MPU5, set 7)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5all41d )
GAMEL( 199?, m5all41g,   m5all41,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","All 4 One (Vivid) (MPU5, set 8)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5all41d )
GAMEL( 199?, m5all41h,   m5all41,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","All 4 One (Vivid) (MPU5, set 9)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5all41d )
GAMEL( 199?, m5all41i,   m5all41,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","All 4 One (Vivid) (MPU5, set 10)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5all41d )
GAMEL( 199?, m5all41j,   m5all41,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","All 4 One (Vivid) (MPU5, set 11)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5all41d )
GAMEL( 199?, m5all41k,   m5all41,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","All 4 One (Vivid) (MPU5, set 12)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5all41d )
GAMEL( 199?, m5all41l,   m5all41,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","All 4 One (Vivid) (MPU5, set 13)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5all41d )
GAMEL( 199?, m5all41m,   m5all41,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","All 4 One (Vivid) (MPU5, set 14)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5all41d )

GAME(  199?, m5atlan,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Atlantic (Vivid) (MPU5, v1.4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5atlana,   m5atlan,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Atlantic (Vivid) (MPU5, v1.2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5bttf,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Back To The Features (Vivid) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5bttf )
GAMEL( 199?, m5bttfa,    m5bttf,   mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Back To The Features (Vivid) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5bttf )

GAMEL( 199?, m5btlbnk,   0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Bottle Bank (Vivid) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5btlbnk )

GAME(  199?, m5caesc,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Caesar's Cash (Vivid) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5card,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Card Shark (Vivid) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5cshkcb,   0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Card Shark Club (Vivid) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5cshkcb )
GAMEL( 199?, m5cshkcb12, m5cshkcb, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Card Shark Club (Vivid) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5cshkcb )
GAMEL( 199?, m5cshkcb13, m5cshkcb, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Card Shark Club (Vivid) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5cshkcb )

GAMEL( 199?, m5clifhn,   0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Cliffhanger (Vivid) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5clifhn )

GAME(  199?, m5cnct4,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Connect 4 (Vivid) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5cnct415,  m5cnct4,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Connect 4 (Vivid) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5cnct420,  m5cnct4,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Connect 4 (Vivid) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5devil,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Devil Of A Deal (Vivid) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5devil )

GAMEL( 199?, m5elband,   0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","El Bandido Club (Vivid) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5elband )

GAME(  199?, m5fair,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Fairground Attraction (Vivid) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5ggems,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Giant Gems (Vivid) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5ggems20 )
GAMEL( 199?, m5ggems20,  m5ggems,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Giant Gems (Vivid) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5ggems20 )

GAME(  199?, m5groll,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Golden Roll (Vivid) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5hilok,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Hi Lo Karate (Vivid) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5hilok )

GAMEL( 199?, m5hiclau,   0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","High Claudius (Vivid) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5hiclau )

GAME(  199?, m5honmon,   0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Honey Money (Vivid) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5honmona,  m5honmon, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Honey Money (Vivid) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5hopidl,   0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Hop Idol (Vivid) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5hopidl )

GAME(  199?, m5hypno,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Hypnotic (Vivid) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5jmpjok,   0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Jumpin Jokers (Vivid) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5jmpjok11, m5jmpjok, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Jumpin Jokers (Vivid) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5jmpjoka,  m5jmpjok, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Jumpin Jokers (Vivid) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5jmpjokb,  m5jmpjok, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Jumpin Jokers (Vivid) (MPU5) (set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5loony,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Loony Juice (Vivid) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5loony )

GAMEL( 199?, m5loot,     0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Loot 'n' Khamun (Vivid) (MPU5, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5loot ) // aka 3-in-1 ?
GAMEL( 199?, m5loota,    m5loot,   mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Loot 'n' Khamun (Vivid) (MPU5, set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5loot ) // aka 3-in-1 ?

GAME(  199?, m5mag7s,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Magnificent 7s (Vivid) (MPU5, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5mag7sa,   m5mag7s,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Magnificent 7s (Vivid) (MPU5, set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5mag7sb,   m5mag7s,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Magnificent 7s (Vivid) (MPU5, set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5mag7sc,   m5mag7s,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Magnificent 7s (Vivid) (MPU5, set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5mag7sd,   m5mag7s,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Magnificent 7s (Vivid) (MPU5, set 5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5mag7se,   m5mag7s,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Magnificent 7s (Vivid) (MPU5, set 6)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5msf,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Manic Streak Features (Vivid) (MPU5, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5msfa,     m5msf,    mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Manic Streak Features (Vivid) (MPU5, set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5piefac,   0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Pie Factory (Vivid) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5piefac )
GAMEL( 199?, m5piefac23, m5piefac, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Pie Factory (Vivid) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5piefac )
GAMEL( 199?, m5piefac12, m5piefac, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Pie Factory (Vivid) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5piefac )
GAMEL( 199?, m5piefaca,  m5piefac, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Pie Factory (Vivid) (MPU5) (set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5piefac )

GAME(  199?, m5piefc2,   0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Pie Factory 2 (Vivid) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5piefc2a,  m5piefc2, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Pie Factory 2 (Vivid) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5piefc2b,  m5piefc2, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Pie Factory 2 (Vivid) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5piefcr,   0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Pie Factory Rio (Vivid) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5piefcr )

GAMEL( 199?, m5qdraw,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Quick On The Draw (Vivid) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5qdrawb )
GAMEL( 199?, m5qdraw12,  m5qdraw,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Quick On The Draw (Vivid) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5qdrawb )
GAMEL( 199?, m5qdraw14,  m5qdraw,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Quick On The Draw (Vivid) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5qdrawb )
GAMEL( 199?, m5qdraw15,  m5qdraw,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Quick On The Draw (Vivid) (MPU5) (set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5qdrawb )
GAMEL( 199?, m5qdrawa,   m5qdraw,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Quick On The Draw (Vivid) (MPU5) (set 5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5qdrawb )
GAMEL( 199?, m5qdrawb,   m5qdraw,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Quick On The Draw (Vivid) (MPU5) (set 6)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5qdrawb )

GAMEL( 199?, m5redbal,   0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Random Red Ball (Vivid) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5redbal )

GAMEL( 199?, m5ratpk,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Rat Pack (Vivid) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5ratpka )
GAMEL( 199?, m5ratpka,   m5ratpk,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Rat Pack (Vivid) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5ratpka )

GAME(  199?, m5rawin,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid / Whitbread","Reel A Win (Vivid / Whitbread) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5rollup,   0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Roll Up Roll Up (Vivid) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5rollup )

GAMEL( 199?, m5shark,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Shark Raving Mad (Vivid) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5shark )
GAMEL( 199?, m5sharka,   m5shark,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Shark Raving Mad (Vivid) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5shark )

GAMEL( 199?, m5speccl,   0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Spectrum Club (Vivid) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5speccl )

GAME(  199?, m5spddmn,   0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Speed Demon (Vivid) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5stars,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Stars & Stripes (Vivid) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5stars26 )
GAMEL( 199?, m5stars13a, m5stars,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Stars & Stripes (Vivid) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5stars26 )
GAMEL( 199?, m5stars26,  m5stars,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Stars & Stripes (Vivid) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5stars26 )
GAMEL( 199?, m5stars25a, m5stars,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Stars & Stripes (Vivid) (MPU5) (set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5stars26 )
GAMEL( 199?, m5stars25,  m5stars,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Stars & Stripes (Vivid) (MPU5) (set 5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5stars26 )
GAMEL( 199?, m5stars22,  m5stars,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Stars & Stripes (Vivid) (MPU5) (set 6)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5stars26 )
GAMEL( 199?, m5stars20,  m5stars,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Stars & Stripes (Vivid) (MPU5) (set 7)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5stars26 )
GAMEL( 199?, m5stars10,  m5stars,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Stars & Stripes (Vivid) (MPU5) (set 8)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5stars26 )
GAMEL( 199?, m5stars10a, m5stars,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Stars & Stripes (Vivid) (MPU5) (set 9)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5stars26 )

GAMEL( 199?, m5starcl,   0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Stars & Stripes Club (Vivid) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5starcl )

GAME(  199?, m5startr,   0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Stars & Stripes Triple (Vivid) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5supro,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Super Roulette (Vivid) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5supro )
GAMEL( 199?, m5suproa,   m5supro,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Super Roulette (Vivid) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5supro )

GAMEL( 199?, m5tempp,    0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Temple Of Pleasure (Vivid) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5tempp )

GAME(  199?, m5whdres,   0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Who Dares Spins (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5winway,   0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Winning Ways (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )



/* Empire */

GAME(  199?, m5fewmor,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","A Few Dollars More (Empire) (MPU5) (v0.2, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5fewmora,     m5fewmor, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","A Few Dollars More (Empire) (MPU5) (v0.2, set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5fewmorb,     m5fewmor, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","A Few Dollars More (Empire) (MPU5) (v0.3, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5fewmorc,     m5fewmor, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","A Few Dollars More (Empire) (MPU5) (v0.3, set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5wonga,       0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","A Fish Called Wonga (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5wonga )

GAME(  199?, m5aceclb,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Ace Of Clubs (Empire) (MPU5, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5aceclba,     m5aceclb, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Ace Of Clubs (Empire) (MPU5, set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5aceclbb,     m5aceclb, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Ace Of Clubs (Empire) (MPU5, set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5barxdx,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Bar X Deluxe (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5bnzclb,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Bonanza Club (Empire) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5bnzclb )
GAMEL( 199?, m5bnzclb11,    m5bnzclb, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Bonanza Club (Empire) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5bnzclb )

GAME(  199?, m5bukroo,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Buckaroo (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5cbrun,       0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Cannonball Run (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5carou,       0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Carousel (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5carou )

GAME(  199?, m5casroc,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Casino Royale Club (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5centcl,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Centurion Club (Empire) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5centcl )
GAMEL( 199?, m5centcl20,    m5centcl, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Centurion Club (Empire) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5centcl )
GAMEL( 199?, m5centcl21,    m5centcl, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Centurion Club (Empire) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5centcl )
GAMEL( 199?, m5centcl21a,   m5centcl, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Centurion Club (Empire) (MPU5) (set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5centcl )
GAMEL( 199?, m5centcla,     m5centcl, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Centurion Club (Empire) (MPU5) (set 5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5centcl )

GAME(  199?, m5cworan,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Clockwork Oranges (Empire) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5cworan12,    m5cworan, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Clockwork Oranges (Empire) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5clbtro,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Club Tropicana (Empire) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5clbtro24,    m5clbtro, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Club Tropicana (Empire) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5clbtro25,    m5clbtro, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Club Tropicana (Empire) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5cockdd,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Cock A Doodle Dough! (Empire) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5cockdd05,    m5cockdd, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Cock A Doodle Dough! (Empire) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5coloss,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Colossus Club (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5crocrk,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Crocodile Rock (Empire) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5crocrk10,    m5crocrk, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Crocodile Rock (Empire) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5croclb,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Crocodile Rock Club (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5crsfir,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Crossfire (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5dmnf,        0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Diamonds Are Forever (Empire) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5dmnf10,      m5dmnf,   mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Diamonds Are Forever (Empire) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5dmnfcl,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Diamonds Are Forever Club (Empire) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5dmnfcl04,    m5dmnfcl, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Diamonds Are Forever Club (Empire) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5extrm,       0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Extreme (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5extrm )

GAME(  199?, m5extrmm,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Extreme Madness (Empire) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5extrmm04a,   m5extrmm, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Extreme Madness (Empire) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5extrmm04b,   m5extrmm, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Extreme Madness (Empire) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5extrmm10,    m5extrmm, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Extreme Madness (Empire) (MPU5) (set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5fatcat,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Fat Cat (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5fishdl,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Fish Full Of Dollars (Empire) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5fishdl10,    m5fishdl, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Fish Full Of Dollars (Empire) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5fishcl,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Fish Full Of Dollars Club (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5fmonty,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","The Full Monty (Empire) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5fmonty04a,   m5fmonty, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","The Full Monty (Empire) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5fmonty04b,   m5fmonty, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","The Full Monty (Empire) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5fmonty04c,   m5fmonty, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","The Full Monty (Empire) (MPU5) (set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5fmount,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Full Mountie (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5gophr,       0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Gopher Gold (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5gophcl,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Gopher Gold Club (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5hisprt,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","High Spirits (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5hisprt )

GAME(  199?, m5hocus,       0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Hocus Pocus (Empire) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5hocus10,     m5hocus,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Hocus Pocus (Empire) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5hocscl,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Hocus Pocus Club (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5hotsht,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Hot Shots (Empire) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5hotsht07a,   m5hotsht, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Hot Shots (Empire) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5hotsht08,    m5hotsht, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Hot Shots (Empire) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5hotsht08a,   m5hotsht, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Hot Shots (Empire) (MPU5) (set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5hotsht10,    m5hotsht, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Hot Shots (Empire) (MPU5) (set 5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5hotsht10a,   m5hotsht, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Hot Shots (Empire) (MPU5) (set 6)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5hula,        0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Hula Moolah (Empire) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5hula10,      m5hula,   mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Hula Moolah (Empire) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5hulacl,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Hula Moolah Club (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5jackbx,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Jack In The Box (Empire) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5jackbx )
GAMEL( 199?, m5jackbx03,    m5jackbx, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Jack In The Box (Empire) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5jackbx )

GAME(  199?, m5jcy,         0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Juicy Fruits (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5jmpgem,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Jumping Gems (Empire) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5jmpgem01 )
GAMEL( 199?, m5jmpgem01,    m5jmpgem, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Jumping Gems (Empire) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5jmpgem01 )
GAMEL( 199?, m5jmpgem03,    m5jmpgem, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Jumping Gems (Empire) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5jmpgem01 )

GAME(  199?, m5jmpgmc,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Jumping Gems Club (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5kaleid,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Kaleidoscope Club (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5kcclb,       0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","King Cobra Club (Empire) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5kcclb24,     m5kcclb,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","King Cobra Club (Empire) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5kingqc,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Kings & Queens Club (Empire) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5kingqc06 )
GAMEL( 199?, m5kingqc06,    m5kingqc, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Kings & Queens Club (Empire) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5kingqc06 )
GAMEL( 199?, m5kingqc07,    m5kingqc, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Kings & Queens Club (Empire) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5kingqc06 )
GAMEL( 199?, m5kingqc08,    m5kingqc, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Kings & Queens Club (Empire) (MPU5) (set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5kingqc06 )

GAMEL( 199?, m5korma,       0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Korma Chameleon (Empire) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5korma )
GAMEL( 199?, m5korma12,     m5korma,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Korma Chameleon (Empire) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5korma )

GAME(  199?, m5kormcl,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Korma Chameleon Club (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5monmst,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Money Monster (Empire) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5monmst )
GAMEL( 199?, m5monmsta,     m5monmst, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Money Monster (Empire) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5monmst )

GAME(  199?, m5ramrd,       0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Ram Raid (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5ramrcl,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Ram Raid Club (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5ronr,        0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Reel Or No Reel (Empire) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5ronr05,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Reel Or No Reel (Empire) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5ronr07,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Reel Or No Reel (Empire) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5resfrg,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Reservoir Frogs (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5resfrg )

GAMEL( 199?, m5rthh,        0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Return To The Haunted House (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5rthh )

GAMEL( 199?, m5rollx,       0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Roll X (Empire) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5rollx )
GAMEL( 199?, m5rollx12,     m5rollx,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Roll X (Empire) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5rollx )

GAMEL( 199?, m5skulcl,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Skullduggery Club (Empire) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5skulcl20 )
GAMEL( 199?, m5skulcl20,    m5skulcl, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Skullduggery Club (Empire) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5skulcl20 )
GAMEL( 199?, m5skulcl23,    m5skulcl, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Skullduggery Club (Empire) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5skulcl20 )

GAME(  199?, m5tball,       0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Thunderball (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5tomb,        0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Tomb Raiders (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5trclb,       0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Tomb Raiders Club (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5ttop,        0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Treble Top (Empire) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5ttop04,      m5ttop,   mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Treble Top (Empire) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5ttop10,      m5ttop,   mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Treble Top (Empire) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5ttopcl,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Treble Top Club (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5tsar,        0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Tsar Wars (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5vertgo,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Vertigo (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5vertgo )

GAME(  199?, m5vertcl,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Vertigo Club (Empire) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5wthing,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Wild Thing Club (Empire) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5wthing20 )
GAMEL( 199?, m5wthing11,    m5wthing, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Wild Thing Club (Empire) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5wthing20 )
GAMEL( 199?, m5wthing20,    m5wthing, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","Wild Thing Club (Empire) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5wthing20 )

GAMEL( 199?, m5xfact,       0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","X Factor (Empire) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5xfact11 )
GAMEL( 199?, m5xfact02,     m5xfact,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","X Factor (Empire) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5xfact11 )
GAMEL( 199?, m5xfact04,     m5xfact,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","X Factor (Empire) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5xfact11 )
GAMEL( 199?, m5xfact11,     m5xfact,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Empire","X Factor (Empire) (MPU5) (set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5xfact11 )

/* Bwb */
GAME(  199?, m5carwsh,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Car Wash (Bwb) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5carwsh10,    m5carwsh, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Car Wash (Bwb) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5cshstx,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Cash Stax (Bwb) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5cshstx )

GAMEL( 199?, m5circus,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Circus (Bwb) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5circus0a )
GAMEL( 199?, m5circus0a,    m5circus, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Circus (Bwb) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5circus0a )
GAMEL( 199?, m5circus0b,    m5circus, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Circus (Bwb) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5circus0a )
GAMEL( 199?, m5circus20,    m5circus, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Circus (Bwb) (MPU5) (set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5circus0a )
GAMEL( 199?, m5circus21,    m5circus, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Circus (Bwb) (MPU5) (set 5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5circus0a )
GAMEL( 199?, m5circus11,    m5circus, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Circus (Bwb) (MPU5) (set 6)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5circus0a )

GAMEL( 199?, m5circlb,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Circus Club (Bwb) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5circlb33 )
GAMEL( 199?, m5circlb00,    m5circlb, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Circus Club (Bwb) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5circlb33 )
GAMEL( 199?, m5circlb15,    m5circlb, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Circus Club (Bwb) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5circlb33 )
GAMEL( 199?, m5circlb33,    m5circlb, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Circus Club (Bwb) (MPU5) (set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5circlb33 )

GAMEL( 199?, m5clown,       0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Clown In Around (Bwb) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5clown11 )
GAMEL( 199?, m5clown11,     m5clown,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Clown In Around (Bwb) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5clown11 )
GAMEL( 199?, m5clown13,     m5clown,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Clown In Around (Bwb) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5clown11 )

GAME(  199?, m5clubsn,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Club Sandwich (Bwb) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5clubsn11,    m5clubsn, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Club Sandwich (Bwb) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5clubsn14,    m5clubsn, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Club Sandwich (Bwb) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5clubsn16,    m5clubsn, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Club Sandwich (Bwb) (MPU5) (set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5dick,        0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Dick Turnip (Bwb) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5dick10 )
GAMEL( 199?, m5dick10,      m5dick,   mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Dick Turnip (Bwb) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5dick10 )

GAME(  199?, m5donna,       0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Donna Kebab (Bwb) (MPU5, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5donnad,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Donna Kebab (Bwb) (MPU5, set 1, Datapak)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5donnaa,      m5donna,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Donna Kebab (Bwb) (MPU5, set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5dblqts,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Double Or Quits (Bwb) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5dblqtsb )
GAMEL( 199?, m5dblqtsa,     m5dblqts, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Double Or Quits (Bwb) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5dblqtsb )
GAMEL( 199?, m5dblqtsb,     m5dblqts, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Double Or Quits (Bwb) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5dblqtsb )
GAMEL( 199?, m5dblqts1b,    m5dblqts, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Double Or Quits (Bwb) (MPU5) (set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5dblqtsb )

GAME(  199?, m5eggold,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Egyptian Gold (Bwb) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m55050,        0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Fifty Fifty (Bwb) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5gpclub,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Get Plastered Club (Bwb) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5goape,       0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Going Ape (Bwb) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5hgl,         0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Happy Go Lucky (Bwb) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5hgl14 )
GAMEL( 199?, m5hgl16,       m5hgl,    mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Happy Go Lucky (Bwb) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5hgl14 )
GAMEL( 199?, m5hgl14,       m5hgl,    mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Happy Go Lucky (Bwb) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5hgl14 )

GAME(  199?, m5jokpak,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Joker In The Pack (Bwb) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5lvwire,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Live Wire (Bwb) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5lvwirea,     m5lvwire, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Live Wire (Bwb) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5carpet,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Magic Carpet (Bwb) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5carpet12,    m5carpet, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Magic Carpet (Bwb) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5minesw,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Minesweeper (Bwb) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5psycho,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Psycho Cash Beast (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5psycho )
GAMEL( 199?, m5psycho06,    m5psycho, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Psycho Cash Beast (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5psycho )
GAMEL( 199?, m5psychoa,     m5psycho, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Psycho Cash Beast (Bwb) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5psycho )
GAMEL( 199?, m5psychoa21,   m5psycho, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Psycho Cash Beast (Bwb) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5psycho )

GAMEL( 199?, m5psyccl,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Psycho Cash Beast Club (Barcrest) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5psyccl01 )
GAMEL( 199?, m5psyccl01,    m5psyccl, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Psycho Cash Beast Club (Barcrest) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5psyccl01 )
GAMEL( 199?, m5psyccla,     m5psyccl, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Psycho Cash Beast Club (Bwb) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5psyccl01 )
GAMEL( 199?, m5psyccla24,   m5psyccl, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Psycho Cash Beast Club (Bwb) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5psyccl01 )
GAMEL( 199?, m5psyccla02,   m5psyccl, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Psycho Cash Beast Club (Bwb) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5psyccl01 )

GAMEL( 199?, m5rwb,         0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Barcrest","Red White & Blue (Barcrest) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5rwb )
GAMEL( 199?, m5rwbbwb,      m5rwb,    mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Red White & Blue (Bwb) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5rwb )
GAMEL( 199?, m5rwbbwb24,    m5rwb,    mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Red White & Blue (Bwb) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5rwb )
GAMEL( 199?, m5rwbbwb25,    m5rwb,    mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Red White & Blue (Bwb) (MPU5) (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5rwb )
GAMEL( 199?, m5rwbbwb15,    m5rwb,    mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Red White & Blue (Bwb) (MPU5) (set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5rwb )

GAME(  199?, m5reelwn,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Reel A Win (Bwb) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5reelwn24,    m5reelwn, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Reel A Win (Bwb) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5reelth,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Reel Thunder (Bwb) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5rlup,        0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Roll Up (Bwb) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5round,       0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Round & Round (Bwb) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5sec7,        0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Secret 7s (Bwb) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5sec7a,       m5sec7,   mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Secret 7s (Bwb) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5sil7,        0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Silver 7s (Bwb) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5sil7a,       m5sil7,   mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Silver 7s (Bwb) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME(  199?, m5smobik,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Smokey Bikin (Bwb) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5smobik12,    m5smobik, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Smokey Bikin (Bwb) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAMEL( 199?, m5tictac,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Vivid","Tic Tac Tut (Vivid) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5tictacbwb )
GAMEL( 199?, m5tictacbwb,   m5tictac, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Tic Tac Tut (Bwb) (MPU5) (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5tictacbwb )
GAMEL( 199?, m5tictacbwb16, m5tictac, mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Tic Tac Tut (Bwb) (MPU5) (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5tictacbwb )

GAME(  199?, m5xena,        0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Bwb","Xena Warrior Princess (Bwb) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )


/* Lowen */
GAME(  199?, m5all41low,    m5all41,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Lowen","All 4 One (Lowen) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5dblfun,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Lowen","Double Fun (Lowen) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5jakjok,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Lowen","Jackpot Jokers (Lowen) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5roundl,      m5round,  mpu5, mpu5, mpu5_state, empty_init, ROT0, "Lowen","Round & Round (Lowen) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(  199?, m5scfinl,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Lowen","Super Cup Final (Lowen) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

/* Others / Uncertain */
GAMEL( 199?, m5ppussy,      0,        mpu5, mpu5, mpu5_state, empty_init, ROT0, "Mdm","Pink Pussy (Mdm) (MPU5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_m5ppussy )
