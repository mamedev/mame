// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "includes/multfish.h"


/*************************************************************************************************************************************************************/
/*************************************************************************************************************************************************************/
/*************************************************************************************************************************************************************/
/*************************************************************************************************************************************************************/
/**************************************************************************************************************************************************************

  Sets below are bootleg / hacked sets, descriptions of what has been modified are provided where known
 -- use of these sets may present a risk to the operator, you should always ensure you're using original Igrosoft program roms on your PCB!

 --------------------------------

  The sets which differs from the originals by 5-6 bytes are bootlegs that simply change
     the banking address. Usually to convert a Crazy Monkey PCB which use the address "F9".
     Software exists to automatic modify any program rom's banking address for any PCB.
     This has resulted in dozens of different bootleg versions floating around the net.

   Some sets simply changed the version text to show a newer version.  This was likely done
     so the opperator appears to be running a version that meets the standards of the changed
     gambling law (see below).

   Sets marked as "backdoor" are identical to originals, but have added code.  This code is
     activated by a secret sequence of actions that leads to a guaranteed win, or dramatically
     increases odds for winning. These backdoor version were commonly used by administrators
     or PCB sellers to steal money from the slots owners.
   Software does exist to automatically add the backdoor code and allows for custom key
     sequences.  As a result of this, there is also software to dectect the backdoor code
     in program roms.
   How to activate the backdoor (each combination is: No of lines, Bet value then Start):
     Enter the first five combinations - you'll become very lucky from this moment.
     Note: backdoor state stored in NVRAM.
   How to deactivate the backdoor:
     Enter the sixth combination twice.
czmon_7a 1,1 1,3 1,5 1,7  3,3  3,4
czmon_7b 1,5 5,5 1,7 3,2  3,3  3,4
czmon_8a 1,1 1,3 1,5 1,7  3,3  3,4
fcockt_6b 1,1 1,3 1,5 1,7  3,3  3,4
fcockt_7a 1,1 1,3 1,5 1,7  3,3  3,4
lhaunt_4a 1,1 1,3 1,5 1,7  3,3  3,4
lhaunt_5a 1,1 1,3 1,5 1,7  3,3  3,4
lhaunt_6a 1,5 9,1 5,1 1,5  3,3  3,4
garage_4a 1,1 1,3 1,5 1,7  3,3  3,4
garage_5a 1,1 1,3 1,5 1,7  3,3  3,4
rclimb_3a 1,5 9,1 5,1 1,5  3,3  3,4
sweetla   1,5 9,1 5,3 1,5  3,3  3,4
resdnt_2a 1,5 9,1 5,1 1,5  3,3  3,4
islanda   1,1 1,3 1,5 1,7  3,3  3,4
keksb     1,1 1,3 1,5 1,7  3,3  3,4

**************************************************************************************************************************************************************/
/*************************************************************************************************************************************************************/
/*************************************************************************************************************************************************************/
/*************************************************************************************************************************************************************/
/*************************************************************************************************************************************************************/

ROM_START( mfish_3a ) // 021124 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "bootleg_mf021124a.rom", 0x00000, 0x40000, CRC(31344b4e) SHA1(33c7f30b55d22c087a02e840456d475177df8bf1) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "multi_fish.001", 0x000000, 0x80000, CRC(2f2a7367) SHA1(ce7ee9ca4f374ec61edc3b89d4752f0edb64a910) )
	ROM_LOAD( "multi_fish.002", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "multi_fish.003", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "multi_fish.004", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "multi_fish.005", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "multi_fish.006", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "multi_fish.007", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "multi_fish.008", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END

ROM_START( mfish_12a ) // 040308 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "bootleg_mf040308a.rom", 0x00000, 0x40000, CRC(44537648) SHA1(7bce6085778ff0b21c052ae91703de3b78b8eed0) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "multi_fish.001", 0x000000, 0x80000, CRC(2f2a7367) SHA1(ce7ee9ca4f374ec61edc3b89d4752f0edb64a910) )
	ROM_LOAD( "multi_fish.002", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "multi_fish.003", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "multi_fish.004", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "multi_fish.005", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "multi_fish.006", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "multi_fish.007", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "multi_fish.008", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END



ROM_START( czmon_7a ) // 031110 backdoor 1,1 1,3 1,5 1,7  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_cm_m_031110a.rom", 0x00000, 0x40000, CRC(80666246) SHA1(e15a210b11ba769ca4fd637c962932417555dc0e) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "crazymonkey_m.001", 0x000000, 0x80000, CRC(683f2be3) SHA1(6fdba4ec07752bf049787a11638895352e9d5f10) )
	ROM_LOAD( "crazymonkey_m.002", 0x100000, 0x80000, CRC(e21ce6a4) SHA1(942ffe323ddbcaaad887cb5bc9f356550926083b) )
	ROM_LOAD( "crazymonkey_m.003", 0x200000, 0x80000, CRC(c3d0e3d5) SHA1(5b0cb436c6b0bac1213c1df56702fa7f16856106) )
	ROM_LOAD( "crazymonkey_m.004", 0x300000, 0x80000, CRC(f79df52c) SHA1(b99fa9f61849b62668bf9edff1c80212a9108b15) )
	ROM_LOAD( "crazymonkey_m.005", 0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006", 0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007", 0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008", 0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( czmon_7b ) // 031110 backdoor 1,5 5,5 1,7 3,2  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_cm_m_031110b.rom", 0x00000, 0x40000, CRC(bb6f4f85) SHA1(a2f44632f857392eb422412b55a19decae4c8620) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "crazymonkey_m.001", 0x000000, 0x80000, CRC(683f2be3) SHA1(6fdba4ec07752bf049787a11638895352e9d5f10) )
	ROM_LOAD( "crazymonkey_m.002", 0x100000, 0x80000, CRC(e21ce6a4) SHA1(942ffe323ddbcaaad887cb5bc9f356550926083b) )
	ROM_LOAD( "crazymonkey_m.003", 0x200000, 0x80000, CRC(c3d0e3d5) SHA1(5b0cb436c6b0bac1213c1df56702fa7f16856106) )
	ROM_LOAD( "crazymonkey_m.004", 0x300000, 0x80000, CRC(f79df52c) SHA1(b99fa9f61849b62668bf9edff1c80212a9108b15) )
	ROM_LOAD( "crazymonkey_m.005", 0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006", 0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007", 0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008", 0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( czmon_8a ) // 050120 backdoor 1,1 1,3 1,5 1,7  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_cm_m_050120a.rom", 0x00000, 0x40000, CRC(e20a6997) SHA1(50e0f0f354dd6db2be64d42e36b4043915c4276b) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "crazymonkey_m.001", 0x000000, 0x80000, CRC(683f2be3) SHA1(6fdba4ec07752bf049787a11638895352e9d5f10) )
	ROM_LOAD( "crazymonkey_m.002", 0x100000, 0x80000, CRC(e21ce6a4) SHA1(942ffe323ddbcaaad887cb5bc9f356550926083b) )
	ROM_LOAD( "crazymonkey_m.003", 0x200000, 0x80000, CRC(c3d0e3d5) SHA1(5b0cb436c6b0bac1213c1df56702fa7f16856106) )
	ROM_LOAD( "crazymonkey_m.004", 0x300000, 0x80000, CRC(f79df52c) SHA1(b99fa9f61849b62668bf9edff1c80212a9108b15) )
	ROM_LOAD( "crazymonkey_m.005", 0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006", 0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007", 0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008", 0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( czmon_8b ) // 050120 changed version text to 070315
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_cm_m_050120b.rom", 0x00000, 0x40000, CRC(9350d184) SHA1(ccb79bb6c5e9025d64fe07c02334c43d75ee3334) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "crazymonkey_m.001", 0x000000, 0x80000, CRC(683f2be3) SHA1(6fdba4ec07752bf049787a11638895352e9d5f10) )
	ROM_LOAD( "crazymonkey_m.002", 0x100000, 0x80000, CRC(e21ce6a4) SHA1(942ffe323ddbcaaad887cb5bc9f356550926083b) )
	ROM_LOAD( "crazymonkey_m.003", 0x200000, 0x80000, CRC(c3d0e3d5) SHA1(5b0cb436c6b0bac1213c1df56702fa7f16856106) )
	ROM_LOAD( "crazymonkey_m.004", 0x300000, 0x80000, CRC(f79df52c) SHA1(b99fa9f61849b62668bf9edff1c80212a9108b15) )
	ROM_LOAD( "crazymonkey_m.005", 0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006", 0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007", 0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008", 0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( czmon_8c ) // 050120 custom alteras, modified graphics, changed version text to "VIDEO GAME-1 CM01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_cm_m_050120c.rom", 0x00000, 0x40000, CRC(231008a7) SHA1(81f77644ba971946cfdf40e6f886652550b10bae) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1b", 0x000000, 0x80000, CRC(6595a293) SHA1(87b6adc837ca7540dd1ae10b544bd753abc95c38) )
	ROM_LOAD( "bootleg_2b", 0x100000, 0x80000, CRC(5a0ce771) SHA1(15c7364eba1316128508acb5407d68dc06b3f506) )
	ROM_LOAD( "bootleg_3b", 0x200000, 0x80000, CRC(12b85ca4) SHA1(22228b0283487b0c01412944030d8e83cd20529d) )
	ROM_LOAD( "bootleg_4b", 0x300000, 0x80000, CRC(8eabc9d8) SHA1(4ade17740f1c584c417c2a404220249168f350ac) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(032fb030) SHA1(392941cd0538bf776cae8422056e8f67cbf4dcb8) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(4f8a61aa) SHA1(cb266f31ad573f627c8b8d54af88fa9e5a3676ea) )
	ROM_LOAD( "bootleg_7b", 0x280000, 0x80000, CRC(a055fea1) SHA1(69498f7410acd3d66b391f9c8a1d541d3400674c) )
	ROM_LOAD( "bootleg_8b", 0x380000, 0x80000, CRC(aa4b0eb3) SHA1(39a98e89a8137c9c986932398b748e48d7f21d9d) )
ROM_END

ROM_START( czmon_8d ) // 050120 modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_cm_m_050120d.rom", 0x00000, 0x40000, CRC(ad37f261) SHA1(0e03bf6134b59340cb43b7d3bdd0e746ebfb112a) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1e", 0x000000, 0x80000, CRC(0a087fce) SHA1(32bb353a45ea39e16fafb78bb10dbd3d330365a6) )
	ROM_LOAD( "bootleg_2e", 0x100000, 0x80000, CRC(210c9cf0) SHA1(ede3cdb9be7dddd4619cd91e1eb51656ba49e250) )
	ROM_LOAD( "bootleg_3e", 0x200000, 0x80000, CRC(08f5933a) SHA1(16a232c634778e4a5f39c6890606ad402b2a52ca) )
	ROM_LOAD( "bootleg_4e", 0x300000, 0x80000, CRC(e12abf21) SHA1(cc015cdda4e1693eb1452c9d11996dbd8b906610) )
	ROM_LOAD( "crazymonkey_m.005", 0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006", 0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007", 0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008", 0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( czmon_8e ) // 050120 modified graphics, many texts changed, changed version text to "LOTO PROGRAM V-CM2"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_cm_m_050120e.rom", 0x00000, 0x40000, CRC(b4b79f13) SHA1(7efb67eb66e18885760422a239f2f56052aa7aa2) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1f", 0x000000, 0x80000, CRC(a2825411) SHA1(ad2d8bdd7b8031e69d3357911b4be7913cdde668) )
	ROM_LOAD( "bootleg_2f", 0x100000, 0x80000, CRC(2a446252) SHA1(4d4daa22421a9ac5a577afdbd257e38b034270c9) )
	ROM_LOAD( "bootleg_3f", 0x200000, 0x80000, CRC(12fa2d2c) SHA1(b3e91cf796e177204c1fde64b83ab389c8b71724) )
	ROM_LOAD( "bootleg_4f", 0x300000, 0x80000, CRC(12e0981f) SHA1(e69e8c635573aedfa177aa6fb2b3188c6fa265b5) )
	ROM_LOAD( "crazymonkey_m.005", 0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006", 0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007", 0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008", 0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( czmon_8f ) // 050120 custom_alteras, modified graphics, many texts changed, changed version text to "LOTOS CM01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_cm_m_050120f.rom", 0x00000, 0x40000, CRC(1ea2e07e) SHA1(964b7381573918880b1b537ab5f8006702d2eedb) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1g", 0x000000, 0x80000, CRC(88ef6dfb) SHA1(a8e323b363f30f8b3ec1f57dbffbedd0a868dc91) )
	ROM_LOAD( "bootleg_2g", 0x100000, 0x80000, CRC(5ae3c4ab) SHA1(7e2d5c4c7900222920f1a6babd7b655aa4c3e849) )
	ROM_LOAD( "bootleg_3g", 0x200000, 0x80000, CRC(70c12376) SHA1(fbb92baed3102975125aa36d377b2f07d66e3a4a) )
	ROM_LOAD( "bootleg_4g", 0x300000, 0x80000, CRC(65da15c4) SHA1(e853cf8aad61a26bdfa864fa16fcf82fd99522b3) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(032fb030) SHA1(392941cd0538bf776cae8422056e8f67cbf4dcb8) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(4f8a61aa) SHA1(cb266f31ad573f627c8b8d54af88fa9e5a3676ea) )
	ROM_LOAD( "bootleg_7b", 0x280000, 0x80000, CRC(a055fea1) SHA1(69498f7410acd3d66b391f9c8a1d541d3400674c) )
	ROM_LOAD( "bootleg_8b", 0x380000, 0x80000, CRC(aa4b0eb3) SHA1(39a98e89a8137c9c986932398b748e48d7f21d9d) )
ROM_END


ROM_START( czmon_9a ) // 070315 custom alteras, modified graphics, changed version text to "VIDEO GAME-1 O01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_cm_070315a.rom", 0x00000, 0x40000, CRC(1c4ea4ac) SHA1(ffbc5345170afdca9154a8a53c2887439b04b489) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1c", 0x000000, 0x80000, CRC(449253a7) SHA1(766e3b9b9585ee8c3b952033e5fb78c256075925) )
	ROM_LOAD( "bootleg_2c", 0x100000, 0x80000, CRC(9475f993) SHA1(a69326bcf5b98510ab996dee8aecee62ef3f2319) )
	ROM_LOAD( "bootleg_3c", 0x200000, 0x80000, CRC(45376e79) SHA1(097b779b5aeea952e4b900123909b441e2f32003) )
	ROM_LOAD( "bootleg_4c", 0x300000, 0x80000, CRC(4d2dbb3d) SHA1(da1d9b1901d1dc63643529f1e60d67fc3635b966) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(032fb030) SHA1(392941cd0538bf776cae8422056e8f67cbf4dcb8) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(4f8a61aa) SHA1(cb266f31ad573f627c8b8d54af88fa9e5a3676ea) )
	ROM_LOAD( "bootleg_7b", 0x280000, 0x80000, CRC(a055fea1) SHA1(69498f7410acd3d66b391f9c8a1d541d3400674c) )
	ROM_LOAD( "bootleg_8b", 0x380000, 0x80000, CRC(aa4b0eb3) SHA1(39a98e89a8137c9c986932398b748e48d7f21d9d) )
ROM_END

ROM_START( czmon_9b ) // 070315 modified graphics, changed version text to "VIDEO GAME-1 O01" (czmon_9a, decoded gfx)
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_cm_070315a.rom", 0x00000, 0x40000, CRC(1c4ea4ac) SHA1(ffbc5345170afdca9154a8a53c2887439b04b489) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1d", 0x000000, 0x80000, CRC(19e03366) SHA1(0050d54f816a10f03b3e76b8472038d39ebe6fc7) )
	ROM_LOAD( "bootleg_2d", 0x100000, 0x80000, CRC(57a08a6a) SHA1(f8691f35efe61ad2e0e22f60a626dec38f6da49c) )
	ROM_LOAD( "bootleg_3d", 0x200000, 0x80000, CRC(2c3bd0c9) SHA1(181f8c84478ad06af941710142bd3c3ab808455b) )
	ROM_LOAD( "bootleg_4d", 0x300000, 0x80000, CRC(d054838f) SHA1(21c2466d45706a80563fc5873d64a7b1abe8da94) )
	ROM_LOAD( "crazymonkey_m.005", 0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006", 0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007", 0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008", 0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( czmon_9c ) // 070315 payout percentage 70%
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_cm_070315b.rom", 0x00000, 0x40000, CRC(d49d3d36) SHA1(00c1ddac53ada6905fe1a8c450158c87c87f33d6) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "crazymonkey.001", 0x000000, 0x80000, CRC(665ae6a1) SHA1(2ef2d657918d66d303d45d2d82430d13108f3fad) )
	ROM_LOAD( "crazymonkey.002", 0x100000, 0x80000, CRC(1a8e235a) SHA1(6d562cc5250283fc0c8ca7e103231a2e5bab4c69) )
	ROM_LOAD( "crazymonkey.003", 0x200000, 0x80000, CRC(415133eb) SHA1(227f7c8858fd5b928fdde691017104d3bd69910a) )
	ROM_LOAD( "crazymonkey.004", 0x300000, 0x80000, CRC(ec45fe14) SHA1(4a0fc87e2f19ea05c9a5746bb4ca7cafe5592d33) )
	ROM_LOAD( "crazymonkey_m.005", 0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006", 0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007", 0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008", 0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END



ROM_START( fcockt_6a ) // 040216 bank F8
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc_m_040216a.rom", 0x00000, 0x40000, CRC(58e7a0c6) SHA1(8022f92af05e9ff6999ff936bad6048d6c264086) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail_m.001", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "fruitcocktail_m.002", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "fruitcocktail_m.003", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "fruitcocktail_m.004", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_6b ) // 040216 backdoor 1,1 1,3 1,5 1,7  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc_m_040216b.rom", 0x00000, 0x40000, CRC(0f6bcf03) SHA1(6c8765f836f1d899aec3be9c842d5064fd70a435) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail_m.001", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "fruitcocktail_m.002", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "fruitcocktail_m.003", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "fruitcocktail_m.004", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_6c ) // 040216 modified graphics, some code changes, description says "for Lat-02 terminals", older set
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc_m_040216c.rom", 0x00000, 0x40000, CRC(890940db) SHA1(5180d711fb3222e3d86c691cb8a23eed6ea0030e) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1c", 0x000000, 0x80000, CRC(3ac27b83) SHA1(72a06f217158367e97d226b3479896319aa3e089) )
	ROM_LOAD( "bootleg_2c", 0x100000, 0x80000, CRC(d00183b8) SHA1(cb4a6f8fdf3b4f1540e53b956e676a9c2c2e1de0) )
	ROM_LOAD( "bootleg_3c", 0x200000, 0x80000, CRC(c509f3c6) SHA1(4a0553e971d7e17971e0d66ff0ed04f29bc42ca7) )
	ROM_LOAD( "bootleg_4c", 0x300000, 0x80000, CRC(5baa91fa) SHA1(dd0ef9972f6f372a6ed8280c887384b0cfa9bce6) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_6d ) // 040216 modified graphics, some code changes, changed version text to "VIDEO GAME-1 FR01", description says "for Lat-02 terminals", newer set
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc_m_040216d.rom", 0x00000, 0x40000, CRC(c1db9659) SHA1(ff01794b7007b5b0c5e77f9a40d87fb6b125143c) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1d", 0x000000, 0x80000, CRC(72ce6a0c) SHA1(e9a6adc9e077fd2a6242282af3d2edc2d3db2a68) )
	ROM_LOAD( "bootleg_2d", 0x100000, 0x80000, CRC(ca1b02c7) SHA1(bc0f2fdf6f6a21d53150642ee155197e94f159aa) )
	ROM_LOAD( "bootleg_3d", 0x200000, 0x80000, CRC(1f1af38e) SHA1(37877bd13c628f3664f6fd9b2597134735e5397c) )
	ROM_LOAD( "bootleg_4d", 0x300000, 0x80000, CRC(e3696471) SHA1(1065dbca6ffb962329dd8faf41a40ac75358216b) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_7a ) // 050118 backdoor 1,1 1,3 1,5 1,7  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc_m_050118a.rom", 0x00000, 0x40000, CRC(eb2bd908) SHA1(b8e9ef469767fb9e95ff181876ffeaee4b7b9361) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail_m.001", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "fruitcocktail_m.002", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "fruitcocktail_m.003", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "fruitcocktail_m.004", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_7b ) // 050118 custom alteras, modified graphics, many texts changed, changed version text to "VIDEO GAME-1 FR01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc_m_050118b.rom", 0x00000, 0x40000, CRC(5f3067f9) SHA1(db5eb951e481ac169b1b0c2439e6553cd7845383) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1e", 0x000000, 0x80000, CRC(8d8d246a) SHA1(c422a7fb06bf4e94c0fec9de5e6f07ad756c9f02) )
	ROM_LOAD( "bootleg_2e", 0x100000, 0x80000, CRC(a9b1496b) SHA1(09c2316dfce4793b2c3fe8b75b79ebf7cc9dfe3d) )
	ROM_LOAD( "bootleg_3e", 0x200000, 0x80000, CRC(fa755764) SHA1(1e057ede7f3b5739548d45b513f324b83a94a9ce) )
	ROM_LOAD( "bootleg_4e", 0x300000, 0x80000, CRC(83b83f0d) SHA1(a739b3bad3f092eb8fd54662281f0cd7cfd77c62) )
	ROM_LOAD( "bootleg_5e", 0x080000, 0x80000, CRC(a43d6253) SHA1(7d58230efdb89a090b456345a44a363a039cdee8) )
	ROM_LOAD( "bootleg_6e", 0x180000, 0x80000, CRC(32ede872) SHA1(b2f2c8c9ad00ee98d19fcd16efff8e4b28ce44d0) )
	ROM_LOAD( "bootleg_7e", 0x280000, 0x80000, CRC(391b0471) SHA1(5d20ff6a00425b66f6e53766e41afbed34a6aa42) )
	ROM_LOAD( "bootleg_8e", 0x380000, 0x80000, CRC(a634af6b) SHA1(91eeaa7932368b1240a3c0180841b010f5c52362) )
ROM_END

ROM_START( fcockt_7c ) // 050118 payout percentage 40%
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc_m_050118c.rom", 0x00000, 0x40000, CRC(65ce8d61) SHA1(f8b63d0cc5ad314013dad4d566a08c87430817d7) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail_m.001", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "fruitcocktail_m.002", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "fruitcocktail_m.003", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "fruitcocktail_m.004", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_7d ) // 050118 payout percentage 60%
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc_m_050118d.rom", 0x00000, 0x40000, CRC(f13165ce) SHA1(5340be60a1f1d530b164eddf9d2717ac752253b9) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail_m.001", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "fruitcocktail_m.002", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "fruitcocktail_m.003", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "fruitcocktail_m.004", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_7e ) // 050118 payout percentage 70%
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc_m_050118e.rom", 0x00000, 0x40000, CRC(0f25d1d3) SHA1(2b16a06f6126d110f8664d0874476b687265bb91) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail_m.001", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "fruitcocktail_m.002", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "fruitcocktail_m.003", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "fruitcocktail_m.004", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_7f ) // 050118 changed version text to 070305
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc_m_050118f.rom", 0x00000, 0x40000, CRC(a6be3c68) SHA1(53cd6d4d2860ba75c95fa1609efa9ff8ea5e2048) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail_m.001", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "fruitcocktail_m.002", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "fruitcocktail_m.003", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "fruitcocktail_m.004", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_7g ) // 050118 modified graphics, many texts changed, changed version text to "LOTO PROGRAM V-FC2"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc_m_050118g.rom", 0x00000, 0x40000, CRC(e9d1b425) SHA1(caf76dda7bf2383ce44b81c560d91dbde4d7421e) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1h", 0x000000, 0x80000, CRC(5144f9a3) SHA1(e530bb6e1e7c44f7378d6923286f78a74ac34ad4) )
	ROM_LOAD( "bootleg_2h", 0x100000, 0x80000, CRC(c9e57914) SHA1(44e2c3e8739b8943babb18fd7483a84d170cb85e) )
	ROM_LOAD( "bootleg_3h", 0x200000, 0x80000, CRC(f18c6fce) SHA1(632fc7b3b9927071081b66ac0fd01f85e1290266) )
	ROM_LOAD( "bootleg_4h", 0x300000, 0x80000, CRC(9ac3e603) SHA1(f2fab6122d7304543e804b56878f067952f5963d) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_7h ) // 050118 modified graphics, many texts changed, changed version text to "LOTOS FR01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc_m_050118h.rom", 0x00000, 0x40000, CRC(fb191121) SHA1(e08587ac260bc57b16fbb7c7022ae7849946565b) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1i", 0x000000, 0x80000, CRC(5186689f) SHA1(afaacaaff8b4ba37bba4c0fb800c953fa1a37b04) )
	ROM_LOAD( "bootleg_2i", 0x100000, 0x80000, CRC(7263bb27) SHA1(38e46d40b76e83311472192aadcc7daf65a407b1) )
	ROM_LOAD( "bootleg_3i", 0x200000, 0x80000, CRC(02a6e05e) SHA1(dd1f4d50bb46fd3e3b3d8dcb14c5f5084ecf2e35) )
	ROM_LOAD( "bootleg_4i", 0x300000, 0x80000, CRC(99b28f09) SHA1(85eb3788fd413c5d2a131982e158e5e552a930b4) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_8a ) // 060111 modified graphics, many texts changed, changed version text to "LOTO COCKTAIL V01-0001"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc_m_060111a.rom", 0x00000, 0x40000, CRC(d594f40a) SHA1(291c4a84ba90df5a50205c67874f69efb3346194) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1f", 0x000000, 0x80000, CRC(4e905563) SHA1(63f2e74947fd4918d0fa744db85ca646686bd459) )
	ROM_LOAD( "bootleg_2f", 0x100000, 0x80000, CRC(25049350) SHA1(51dae347f4da5dad6bc83fd26a855527fe8cb8ef) )
	ROM_LOAD( "bootleg_3f", 0x200000, 0x80000, CRC(ca9bf738) SHA1(b5f82b9f082ada0299c4585c22fcbf785b62320f) )
	ROM_LOAD( "bootleg_4f", 0x300000, 0x80000, CRC(b65f200c) SHA1(16a3c612a6c6ed1d2956ebba55a933aa2c905974) )
	ROM_LOAD( "bootleg_5",  0x080000, 0x80000, CRC(64b547e3) SHA1(285421fa3aa67a16cf6a9dadb20d74e6a8471dc0) )
	ROM_LOAD( "bootleg_6",  0x180000, 0x80000, CRC(965d6363) SHA1(5c229238a09ec54147d492e9843595962ce79952) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "bootleg_8",  0x380000, 0x80000, CRC(8384e4d4) SHA1(83d0bbbd7cca7328a66a69cf802632fd8d22d5b8) )
ROM_END

ROM_START( fcockt_8b ) // 060111 modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc_m_060111b.rom", 0x00000, 0x40000, CRC(71796ce6) SHA1(e4f4140cce3dbe7735c13b23682e23c9e1331f3a) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1g", 0x000000, 0x80000, CRC(5b658094) SHA1(13801c3267772cecb22578fa6f01c20fe5da687d) )
	ROM_LOAD( "bootleg_2g", 0x100000, 0x80000, CRC(4bbad46c) SHA1(d44d224c18922a7837a47894d2e679a64be7f3cb) )
	ROM_LOAD( "bootleg_3g", 0x200000, 0x80000, CRC(38ad449a) SHA1(eebb826a702cf7a6373a8ef51b4d40ceac758204) )
	ROM_LOAD( "bootleg_4g", 0x300000, 0x80000, CRC(4e676629) SHA1(c85ea9322bdc375f26771b3996d8ca69c43b4d2c) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END



ROM_START( lhaunt_4a ) // 031111 backdoor 1,1 1,3 1,5 1,7  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_lh_m_031111a.rom", 0x00000, 0x40000, CRC(83d487c9) SHA1(5b88745d06acba542e2d0660298c9058f2bdfa3f) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "luckyhaunter_m.001", 0x000000, 0x80000, CRC(5f1000c6) SHA1(05154d786caf7f5fb9ed90c7d3391cec42e871f8) )
	ROM_LOAD( "luckyhaunter_m.002", 0x100000, 0x80000, CRC(b18abebc) SHA1(66c28fd3f338360b9236dcf414548bfb53655951) )
	ROM_LOAD( "luckyhaunter_m.003", 0x200000, 0x80000, CRC(2e67a1d9) SHA1(a3eff78f25e4e4878706d7c9e4ca71f6914006c6) )
	ROM_LOAD( "luckyhaunter_m.004", 0x300000, 0x80000, CRC(90c963f6) SHA1(2a9c689315cdfb67425f0710511dc0e0241741a7) )
	ROM_LOAD( "luckyhaunter_m.005", 0x080000, 0x80000, CRC(b50c90a3) SHA1(74749f4ffb5b0630631b511fc3230c6e7b50dc3b) )
	ROM_LOAD( "luckyhaunter_m.006", 0x180000, 0x80000, CRC(4eaaab64) SHA1(9fed16f8e0308200fd16c4b1e511e1bf6c22ae66) )
	ROM_LOAD( "luckyhaunter_m.007", 0x280000, 0x80000, CRC(64d16ba9) SHA1(3b897183d6e0f1256be7657441f234fc72077682) )
	ROM_LOAD( "luckyhaunter_m.008", 0x380000, 0x80000, CRC(1bdf6252) SHA1(7b5ae82a95a744b236e109024d47b526dccf9c14) )
ROM_END

ROM_START( lhaunt_5a ) // 040216 backdoor 1,1 1,3 1,5 1,7  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_lh_m_040216a.rom", 0x00000, 0x40000, CRC(2a6c7ff9) SHA1(4a0137c7df5003e8fd843d5489d416d15f001f46) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "luckyhaunter_m.001", 0x000000, 0x80000, CRC(5f1000c6) SHA1(05154d786caf7f5fb9ed90c7d3391cec42e871f8) )
	ROM_LOAD( "luckyhaunter_m.002", 0x100000, 0x80000, CRC(b18abebc) SHA1(66c28fd3f338360b9236dcf414548bfb53655951) )
	ROM_LOAD( "luckyhaunter_m.003", 0x200000, 0x80000, CRC(2e67a1d9) SHA1(a3eff78f25e4e4878706d7c9e4ca71f6914006c6) )
	ROM_LOAD( "luckyhaunter_m.004", 0x300000, 0x80000, CRC(90c963f6) SHA1(2a9c689315cdfb67425f0710511dc0e0241741a7) )
	ROM_LOAD( "luckyhaunter_m.005", 0x080000, 0x80000, CRC(b50c90a3) SHA1(74749f4ffb5b0630631b511fc3230c6e7b50dc3b) )
	ROM_LOAD( "luckyhaunter_m.006", 0x180000, 0x80000, CRC(4eaaab64) SHA1(9fed16f8e0308200fd16c4b1e511e1bf6c22ae66) )
	ROM_LOAD( "luckyhaunter_m.007", 0x280000, 0x80000, CRC(64d16ba9) SHA1(3b897183d6e0f1256be7657441f234fc72077682) )
	ROM_LOAD( "luckyhaunter_m.008", 0x380000, 0x80000, CRC(1bdf6252) SHA1(7b5ae82a95a744b236e109024d47b526dccf9c14) )
ROM_END

ROM_START( lhaunt_6a ) // 040825 backdoor 1,5 9,1 5,1 1,5  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_lh_m_040825a.rom", 0x00000, 0x40000, CRC(18ba5704) SHA1(3c77ed129db0e5181217167b76292f8e4ee78728) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "luckyhaunter_m.001", 0x000000, 0x80000, CRC(5f1000c6) SHA1(05154d786caf7f5fb9ed90c7d3391cec42e871f8) )
	ROM_LOAD( "luckyhaunter_m.002", 0x100000, 0x80000, CRC(b18abebc) SHA1(66c28fd3f338360b9236dcf414548bfb53655951) )
	ROM_LOAD( "luckyhaunter_m.003", 0x200000, 0x80000, CRC(2e67a1d9) SHA1(a3eff78f25e4e4878706d7c9e4ca71f6914006c6) )
	ROM_LOAD( "luckyhaunter_m.004", 0x300000, 0x80000, CRC(90c963f6) SHA1(2a9c689315cdfb67425f0710511dc0e0241741a7) )
	ROM_LOAD( "luckyhaunter_m.005", 0x080000, 0x80000, CRC(b50c90a3) SHA1(74749f4ffb5b0630631b511fc3230c6e7b50dc3b) )
	ROM_LOAD( "luckyhaunter_m.006", 0x180000, 0x80000, CRC(4eaaab64) SHA1(9fed16f8e0308200fd16c4b1e511e1bf6c22ae66) )
	ROM_LOAD( "luckyhaunter_m.007", 0x280000, 0x80000, CRC(64d16ba9) SHA1(3b897183d6e0f1256be7657441f234fc72077682) )
	ROM_LOAD( "luckyhaunter_m.008", 0x380000, 0x80000, CRC(1bdf6252) SHA1(7b5ae82a95a744b236e109024d47b526dccf9c14) )
ROM_END

ROM_START( lhaunt_6b ) // 040825 custom alteras, modified graphics, many texts changed, changed version text to "VIDEO GAME-1 PB01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_lh_m_040825b.rom", 0x00000, 0x40000, CRC(71bc9811) SHA1(64dc0d81aec657b8c722e530ad5858d831ab7d51) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1b", 0x000000, 0x80000, CRC(82e0dd1a) SHA1(7f99fe1e5d304fadae9f1f46ebe8680787c9175b) )
	ROM_LOAD( "bootleg_2b", 0x100000, 0x80000, CRC(fa41dc9b) SHA1(3a2d46cc3bb1e5a8c9a42e28e0b195548d365865) )
	ROM_LOAD( "bootleg_3b", 0x200000, 0x80000, CRC(761fcd19) SHA1(08c5eb77a8829f4cdb7508a1f3c2361f8bdefb17) )
	ROM_LOAD( "bootleg_4b", 0x300000, 0x80000, CRC(be5f40c3) SHA1(62ac5b3c98db7352d241a115030aef0266c9e737) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(99fb98df) SHA1(66dd5a07ce72d88f5612c3dc8ec2bcd76229a586) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(208fb837) SHA1(857a615fa674cd55771cc4c12afd61fe8a891d3c) )
	ROM_LOAD( "bootleg_7b", 0x280000, 0x80000, CRC(a4d609cd) SHA1(bcf193745012a59ec706f54c157dd413b49f0a35) )
	ROM_LOAD( "bootleg_8b", 0x380000, 0x80000, CRC(6703695b) SHA1(7730aa1297cc1584cfe1236c16f1a90829727baa) )
ROM_END

ROM_START( lhaunt_6c ) // 040825 changed version text to 070604
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_lh_m_040825c.rom", 0x00000, 0x40000, CRC(b2b027c2) SHA1(18e5d7b396103a7301b2a6641ca3d85ab2091d2e) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "luckyhaunter_m.001", 0x000000, 0x80000, CRC(5f1000c6) SHA1(05154d786caf7f5fb9ed90c7d3391cec42e871f8) )
	ROM_LOAD( "luckyhaunter_m.002", 0x100000, 0x80000, CRC(b18abebc) SHA1(66c28fd3f338360b9236dcf414548bfb53655951) )
	ROM_LOAD( "luckyhaunter_m.003", 0x200000, 0x80000, CRC(2e67a1d9) SHA1(a3eff78f25e4e4878706d7c9e4ca71f6914006c6) )
	ROM_LOAD( "luckyhaunter_m.004", 0x300000, 0x80000, CRC(90c963f6) SHA1(2a9c689315cdfb67425f0710511dc0e0241741a7) )
	ROM_LOAD( "luckyhaunter_m.005", 0x080000, 0x80000, CRC(b50c90a3) SHA1(74749f4ffb5b0630631b511fc3230c6e7b50dc3b) )
	ROM_LOAD( "luckyhaunter_m.006", 0x180000, 0x80000, CRC(4eaaab64) SHA1(9fed16f8e0308200fd16c4b1e511e1bf6c22ae66) )
	ROM_LOAD( "luckyhaunter_m.007", 0x280000, 0x80000, CRC(64d16ba9) SHA1(3b897183d6e0f1256be7657441f234fc72077682) )
	ROM_LOAD( "luckyhaunter_m.008", 0x380000, 0x80000, CRC(1bdf6252) SHA1(7b5ae82a95a744b236e109024d47b526dccf9c14) )
ROM_END

ROM_START( lhaunt_6d ) // 040825 modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_lh_m_040825d.rom", 0x00000, 0x40000, CRC(bf442051) SHA1(0c96b5cbf24dc8786feae658f23518bef6c0d589) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1c", 0x000000, 0x80000, CRC(9ff7618e) SHA1(1c191a211f2391f5aa5786c2071dc34d909bbe46) )
	ROM_LOAD( "bootleg_2c", 0x100000, 0x80000, CRC(094e040f) SHA1(254348b1690ce82bb2551006365d630fb82413c2) )
	ROM_LOAD( "bootleg_3c", 0x200000, 0x80000, CRC(47f7d726) SHA1(d43797c50fa0a8f25a5cd198e906f67d9f3e9a13) )
	ROM_LOAD( "bootleg_4c", 0x300000, 0x80000, CRC(1aa0998f) SHA1(078c09774ac3664a76d83b28596d0e07f3f7b9e2) )
	ROM_LOAD( "luckyhaunter_m.005", 0x080000, 0x80000, CRC(b50c90a3) SHA1(74749f4ffb5b0630631b511fc3230c6e7b50dc3b) )
	ROM_LOAD( "luckyhaunter_m.006", 0x180000, 0x80000, CRC(4eaaab64) SHA1(9fed16f8e0308200fd16c4b1e511e1bf6c22ae66) )
	ROM_LOAD( "luckyhaunter_m.007", 0x280000, 0x80000, CRC(64d16ba9) SHA1(3b897183d6e0f1256be7657441f234fc72077682) )
	ROM_LOAD( "luckyhaunter_m.008", 0x380000, 0x80000, CRC(1bdf6252) SHA1(7b5ae82a95a744b236e109024d47b526dccf9c14) )
ROM_END

ROM_START( lhaunt_6e ) // 040825 modified graphics, many texts changed, changed version text to "LOTO PROGRAM V-LH2"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_lh_m_040825e.rom", 0x00000, 0x40000, CRC(087d4172) SHA1(ec9ca4192031d8b8e3cbaf809e8d1643978481f3) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1d", 0x000000, 0x80000, CRC(41e22133) SHA1(b3d88b62fed9aa4ea5bbdff3f92a4120a239cfb0) )
	ROM_LOAD( "bootleg_2d", 0x100000, 0x80000, CRC(ea948a43) SHA1(ccf0450ed095cf4d48a0775a50a630d7e2d50fcb) )
	ROM_LOAD( "bootleg_3d", 0x200000, 0x80000, CRC(df715e6b) SHA1(be8c143be66a638eeb971406e323c3e857a50e19) )
	ROM_LOAD( "bootleg_4d", 0x300000, 0x80000, CRC(e70b3a0e) SHA1(f3905c117467fb6817fd02ce08af6188439821bf) )
	ROM_LOAD( "bootleg_5d", 0x080000, 0x80000, CRC(666d1923) SHA1(5ef416c2c567c56d2188ba27d8f0f205551eca7f) )
	ROM_LOAD( "bootleg_6d", 0x180000, 0x80000, CRC(304475f2) SHA1(de532496c4534ff09c88d87d0ccadad542ff5637) )
	ROM_LOAD( "bootleg_7d", 0x280000, 0x80000, CRC(278aab12) SHA1(cb35e95babddcb126c10dfd6abf2986133a6fbf1) )
	ROM_LOAD( "bootleg_8d", 0x380000, 0x80000, CRC(79c110bc) SHA1(4198b6003af7e52720f84776f51fe4e1c1cc8713) )
ROM_END

ROM_START( lhaunt_6f ) // 040825 custom alteras, modified graphics, many texts changed, changed version text to "LOTOS PB01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_lh_m_040825f.rom", 0x00000, 0x40000, CRC(7033af62) SHA1(c6b859672816f650eb88bc363e6f6d3a197eb99c) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1e", 0x000000, 0x80000, CRC(c7b0191a) SHA1(d33ababbefcb39f4e6b063f448b8a08b015d6f24) )
	ROM_LOAD( "bootleg_2e", 0x100000, 0x80000, CRC(befb39e6) SHA1(6d6fdbb6e74d7418037dda5f76c8f007508df916) )
	ROM_LOAD( "bootleg_3e", 0x200000, 0x80000, CRC(a9cdb4b4) SHA1(7054e47913d8e14d3219d6780f912f730e3a67cb) )
	ROM_LOAD( "bootleg_4e", 0x300000, 0x80000, CRC(302cf08c) SHA1(eede494a0d47f5c911868a785145994f5e72829e) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(99fb98df) SHA1(66dd5a07ce72d88f5612c3dc8ec2bcd76229a586) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(208fb837) SHA1(857a615fa674cd55771cc4c12afd61fe8a891d3c) )
	ROM_LOAD( "bootleg_7b", 0x280000, 0x80000, CRC(a4d609cd) SHA1(bcf193745012a59ec706f54c157dd413b49f0a35) )
	ROM_LOAD( "bootleg_8b", 0x380000, 0x80000, CRC(6703695b) SHA1(7730aa1297cc1584cfe1236c16f1a90829727baa) )
ROM_END



ROM_START( garage_4a ) // 040219 backdoor 1,1 1,3 1,5 1,7  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gg_m_040219a.rom", 0x00000, 0x40000, CRC(e16b213a) SHA1(af0d78116d985efe5f09eb86eb67df2535765527) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "garage_m.001", 0x000000, 0x80000, CRC(57acd4bc) SHA1(8796f463787c359cce6ac09c1b6895f871bbb7c9) )
	ROM_LOAD( "garage_m.002", 0x100000, 0x80000, CRC(6d591fa3) SHA1(ddbdf87e0e88dc848b963fbfcb6e14d7b3b9efdc) )
	ROM_LOAD( "garage_m.003", 0x200000, 0x80000, CRC(6a15eeda) SHA1(ac35a20893b0518a159207401f6b7f58e3de45fa) )
	ROM_LOAD( "garage_m.004", 0x300000, 0x80000, CRC(38f2cd3c) SHA1(5a4463ac352e4e340c6aaa61102841541e9f4c48) )
	ROM_LOAD( "garage_m.005", 0x080000, 0x80000, CRC(5bf85bc5) SHA1(ff9d2b9cbcd2af6f5fda972e387820d830c196a9) )
	ROM_LOAD( "garage_m.006", 0x180000, 0x80000, CRC(e5082b26) SHA1(6547409d39dd51498ce8e3f82ff813a8ac3c6522) )
	ROM_LOAD( "garage_m.007", 0x280000, 0x80000, CRC(dfa2ceb1) SHA1(ec7de8a8f6e7785a563df973841cc1f1603f79fc) )
	ROM_LOAD( "garage_m.008", 0x380000, 0x80000, CRC(90c5416b) SHA1(583bfb517bc2e30d7b7903aa19fc3b4b5188d7d2) )
ROM_END

ROM_START( garage_4b ) // 040219 changed version text to 070329
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gg_m_040219b.rom", 0x00000, 0x40000, CRC(b8ebf3b6) SHA1(14c5263571123d251125723b32861bef578de67b) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "garage_m.001", 0x000000, 0x80000, CRC(57acd4bc) SHA1(8796f463787c359cce6ac09c1b6895f871bbb7c9) )
	ROM_LOAD( "garage_m.002", 0x100000, 0x80000, CRC(6d591fa3) SHA1(ddbdf87e0e88dc848b963fbfcb6e14d7b3b9efdc) )
	ROM_LOAD( "garage_m.003", 0x200000, 0x80000, CRC(6a15eeda) SHA1(ac35a20893b0518a159207401f6b7f58e3de45fa) )
	ROM_LOAD( "garage_m.004", 0x300000, 0x80000, CRC(38f2cd3c) SHA1(5a4463ac352e4e340c6aaa61102841541e9f4c48) )
	ROM_LOAD( "garage_m.005", 0x080000, 0x80000, CRC(5bf85bc5) SHA1(ff9d2b9cbcd2af6f5fda972e387820d830c196a9) )
	ROM_LOAD( "garage_m.006", 0x180000, 0x80000, CRC(e5082b26) SHA1(6547409d39dd51498ce8e3f82ff813a8ac3c6522) )
	ROM_LOAD( "garage_m.007", 0x280000, 0x80000, CRC(dfa2ceb1) SHA1(ec7de8a8f6e7785a563df973841cc1f1603f79fc) )
	ROM_LOAD( "garage_m.008", 0x380000, 0x80000, CRC(90c5416b) SHA1(583bfb517bc2e30d7b7903aa19fc3b4b5188d7d2) )
ROM_END

ROM_START( garage_4c ) // 040219 modified graphics, many texts changed, changed version text to "LOTO PROGRAM V-GG2"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gg_m_040219c.rom", 0x00000, 0x40000, CRC(2e0ee194) SHA1(2dab6f80cd81b34792c1d347e17c2410bf22a48b) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1d", 0x000000, 0x80000, CRC(63cf83f8) SHA1(ac96acd353049e77855cee618521fdba17e1b3f1) )
	ROM_LOAD( "bootleg_2d", 0x100000, 0x80000, CRC(be09cf31) SHA1(067abe9a58cc1ec42524cda33d9c899625ef9e12) )
	ROM_LOAD( "bootleg_3d", 0x200000, 0x80000, CRC(7da936ed) SHA1(f13cb3b3c3e3a3aa39bed72601ca7b6d5f517579) )
	ROM_LOAD( "bootleg_4d", 0x300000, 0x80000, CRC(8f12f84e) SHA1(10f774a36382b52e35738f6060cbe8bbd990dc66) )
	ROM_LOAD( "bootleg_5d", 0x080000, 0x80000, CRC(39ca3b8a) SHA1(e456c2965f1d4519ee1b8b9e7689004b467c4957) )
	ROM_LOAD( "bootleg_6d", 0x180000, 0x80000, CRC(df4ff9ff) SHA1(89cc80923544e44367ba27e42f4f49744de9782c) )
	ROM_LOAD( "bootleg_7d", 0x280000, 0x80000, CRC(e65de46e) SHA1(ffcc48449e764329864800befcb31b884bd7ff20) )
	ROM_LOAD( "bootleg_8d", 0x380000, 0x80000, CRC(15fcac6d) SHA1(3c1f44225686b42eba5229fb0ad03d2592c09203) )
ROM_END

ROM_START( garage_5a ) // 050311 backdoor 1,1 1,3 1,5 1,7  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gg_m_050311a.rom", 0x00000, 0x40000, CRC(874a2c27) SHA1(9bf586314f375c2c6f7d79557cc777ac3559cb64) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "garage_m.001", 0x000000, 0x80000, CRC(57acd4bc) SHA1(8796f463787c359cce6ac09c1b6895f871bbb7c9) )
	ROM_LOAD( "garage_m.002", 0x100000, 0x80000, CRC(6d591fa3) SHA1(ddbdf87e0e88dc848b963fbfcb6e14d7b3b9efdc) )
	ROM_LOAD( "garage_m.003", 0x200000, 0x80000, CRC(6a15eeda) SHA1(ac35a20893b0518a159207401f6b7f58e3de45fa) )
	ROM_LOAD( "garage_m.004", 0x300000, 0x80000, CRC(38f2cd3c) SHA1(5a4463ac352e4e340c6aaa61102841541e9f4c48) )
	ROM_LOAD( "garage_m.005", 0x080000, 0x80000, CRC(5bf85bc5) SHA1(ff9d2b9cbcd2af6f5fda972e387820d830c196a9) )
	ROM_LOAD( "garage_m.006", 0x180000, 0x80000, CRC(e5082b26) SHA1(6547409d39dd51498ce8e3f82ff813a8ac3c6522) )
	ROM_LOAD( "garage_m.007", 0x280000, 0x80000, CRC(dfa2ceb1) SHA1(ec7de8a8f6e7785a563df973841cc1f1603f79fc) )
	ROM_LOAD( "garage_m.008", 0x380000, 0x80000, CRC(90c5416b) SHA1(583bfb517bc2e30d7b7903aa19fc3b4b5188d7d2) )
ROM_END

ROM_START( garage_5b ) // 050311 custom alteras, modified graphics, changed version text to "VIDEO GAME-1 GA01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gg_m_050311b.rom", 0x00000, 0x40000, CRC(79f9a8e7) SHA1(f6ddd586b72ca004ac18284a0394d5a9d56fbc19) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1b", 0x000000, 0x80000, CRC(df71e54b) SHA1(e5899151aea13b527e4f0819609be6d9ee8cc1b2) )
	ROM_LOAD( "bootleg_2b", 0x100000, 0x80000, CRC(72067f6a) SHA1(be1d4e72b03d32dce8f0f86f860b1e976ab6d459) )
	ROM_LOAD( "bootleg_3b", 0x200000, 0x80000, CRC(a432329a) SHA1(ab526e4c1909d205f69f934a3a808337d087b1c7) )
	ROM_LOAD( "bootleg_4b", 0x300000, 0x80000, CRC(44bcf56b) SHA1(19fd698d8d712e00735e2a70836ef3135de0e351) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(9d210b24) SHA1(0bbf2a855880b9f82e3f356adb7b151db3daaed6) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(45b039a0) SHA1(1166815fa1271e44d08bad1f5a3a786c93cb5030) )
	ROM_LOAD( "bootleg_7b", 0x280000, 0x80000, CRC(b4e1ecdb) SHA1(7cd5ffbe21ff48434f3be995388e6bc398fe2aa4) )
	ROM_LOAD( "bootleg_8b", 0x380000, 0x80000, CRC(06c5149f) SHA1(012573a144532bb6d697c0390b97136e47aaca99) )
ROM_END

ROM_START( garage_5c ) // 050311 payout percentage 70%
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gg_m_050311c.rom", 0x00000, 0x40000, CRC(039af644) SHA1(548e2990f15d6f7f68758a91aae5bf297a220698) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "garage_m.001", 0x000000, 0x80000, CRC(57acd4bc) SHA1(8796f463787c359cce6ac09c1b6895f871bbb7c9) )
	ROM_LOAD( "garage_m.002", 0x100000, 0x80000, CRC(6d591fa3) SHA1(ddbdf87e0e88dc848b963fbfcb6e14d7b3b9efdc) )
	ROM_LOAD( "garage_m.003", 0x200000, 0x80000, CRC(6a15eeda) SHA1(ac35a20893b0518a159207401f6b7f58e3de45fa) )
	ROM_LOAD( "garage_m.004", 0x300000, 0x80000, CRC(38f2cd3c) SHA1(5a4463ac352e4e340c6aaa61102841541e9f4c48) )
	ROM_LOAD( "garage_m.005", 0x080000, 0x80000, CRC(5bf85bc5) SHA1(ff9d2b9cbcd2af6f5fda972e387820d830c196a9) )
	ROM_LOAD( "garage_m.006", 0x180000, 0x80000, CRC(e5082b26) SHA1(6547409d39dd51498ce8e3f82ff813a8ac3c6522) )
	ROM_LOAD( "garage_m.007", 0x280000, 0x80000, CRC(dfa2ceb1) SHA1(ec7de8a8f6e7785a563df973841cc1f1603f79fc) )
	ROM_LOAD( "garage_m.008", 0x380000, 0x80000, CRC(90c5416b) SHA1(583bfb517bc2e30d7b7903aa19fc3b4b5188d7d2) )
ROM_END

ROM_START( garage_5d ) // 050311 modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gg_m_050311d.rom", 0x00000, 0x40000, CRC(00db1210) SHA1(90eb11d6055b464546657233b44a6783c34161ae) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1c", 0x000000, 0x80000, CRC(e3deb6f3) SHA1(a1fe01040bd8f4b08268e2b99cfa240e50dce00b) )
	ROM_LOAD( "bootleg_2c", 0x100000, 0x80000, CRC(540abed4) SHA1(c3f7c9c9c71da2092b13cafd51017751b65eea44) )
	ROM_LOAD( "bootleg_3c", 0x200000, 0x80000, CRC(c8ff1697) SHA1(6edc15793ce28abcd85b4bfcdb8dec3b6b9fc86b) )
	ROM_LOAD( "bootleg_4c", 0x300000, 0x80000, CRC(5cf6c32a) SHA1(4f3baf788a7a868bee3343ac623e0d73d11833dd) )
	ROM_LOAD( "bootleg_5c", 0x080000, 0x80000, CRC(5670c51f) SHA1(823d2152342df0129dbe91dceb58282919a30bd1) )
	ROM_LOAD( "bootleg_6c", 0x180000, 0x80000, CRC(b8a6a574) SHA1(157d68736960f351b148be0b81bcdba472437508) )
	ROM_LOAD( "bootleg_7c", 0x280000, 0x80000, CRC(94f2b464) SHA1(879c435a80eb8770fe47d4120078c6a447025f08) )
	ROM_LOAD( "bootleg_8c", 0x380000, 0x80000, CRC(0cf53598) SHA1(c456add6b56169a5116643a27aace1ee19a68340) )
ROM_END

ROM_START( garage_5e ) // 050311 custom alteras, modified graphics, many texts changed, changed version text to "LOTOS GA01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gg_m_050311e.rom", 0x00000, 0x40000, CRC(2c7971ba) SHA1(512c6a76d22ba46c9bbb6cc1255f156a8d20befe) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1e", 0x000000, 0x80000, CRC(c89cc739) SHA1(31e62dd6ef841dc3f2a3da3e89649b7250cc3a6d) )
	ROM_LOAD( "bootleg_2e", 0x100000, 0x80000, CRC(c224a356) SHA1(c684b69ca8998f74d13edc56f3c3201c6cce6e01) )
	ROM_LOAD( "bootleg_3e", 0x200000, 0x80000, CRC(a83cfda9) SHA1(c754b47da998cf9aa11a2332ba66756599d79340) )
	ROM_LOAD( "bootleg_4e", 0x300000, 0x80000, CRC(e262b94c) SHA1(f6e22a38226a55b99ddd9fcf714599acbd3ee27c) )
	ROM_LOAD( "bootleg_5e", 0x080000, 0x80000, CRC(ad6c6919) SHA1(ff152adf63372a203d2a2f629766706f482d8065) )
	ROM_LOAD( "bootleg_6e", 0x180000, 0x80000, CRC(ddbd373e) SHA1(cebb6b666cc1ee6e1f0a27c6b989f3818d67419b) )
	ROM_LOAD( "bootleg_7e", 0x280000, 0x80000, CRC(13880471) SHA1(037b5e331da282aba310bc72346d66a0b4a08e6d) )
	ROM_LOAD( "bootleg_8e", 0x380000, 0x80000, CRC(10cc710b) SHA1(2bcf01b9d4730d8377edfd563c1c508ea128fd46) )
ROM_END



ROM_START( rclimb_3a ) // 040827 backdoor 1,5 9,1 5,1 1,5  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_rc_m_040827a.rom", 0x00000, 0x40000, CRC(6420f8b4) SHA1(64e4018dbea245ddc06a65fb2f8cf38e77f60999) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "rockclimber_m.001", 0x000000, 0x80000, CRC(934f18c7) SHA1(da3a7cddc68e104d415d947e89c0e7f0d067c056) )
	ROM_LOAD( "rockclimber_m.002", 0x100000, 0x80000, CRC(7364bd2b) SHA1(c0edfd3b8de813c95fe5d6072662fa0e39fec89e) )
	ROM_LOAD( "rockclimber_m.003", 0x200000, 0x80000, CRC(e7befb17) SHA1(8a214680142cd657784a667ab3f6422165fea224) )
	ROM_LOAD( "rockclimber_m.004", 0x300000, 0x80000, CRC(dc6d43a0) SHA1(62fc47136775f3fa9369857ec91fe897a1f1ebd6) )
	ROM_LOAD( "rockclimber_m.005", 0x080000, 0x80000, CRC(ea127c3d) SHA1(a6391eed69a4723b68d727f59b6baebe51633e66) )
	ROM_LOAD( "rockclimber_m.006", 0x180000, 0x80000, CRC(277fa273) SHA1(6320e6c5b5e48dc451cc48189054c42d85e8ccc1) )
	ROM_LOAD( "rockclimber_m.007", 0x280000, 0x80000, CRC(3ca7f69a) SHA1(878cca181d915dc3548d5285a4bbb51aef31a64e) )
	ROM_LOAD( "rockclimber_m.008", 0x380000, 0x80000, CRC(8cf6b4c2) SHA1(4c36c217b83c82acfdd615f5547bf597af7b8833) )
ROM_END

ROM_START( rclimb_3b ) // 040827 - new service menu
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_rc_m_040827.rom", 0x00000, 0x40000, CRC(3ba55647) SHA1(56e96be0d9782da4b3d5d911ea67962257626ae0) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_rc1_.bin", 0x000000, 0x80000, CRC(4bc00c66) SHA1(e4d89c2b188f253b642ae341a1a4c04af33024c8) )
	ROM_LOAD( "bootleg_rc2_.bin", 0x100000, 0x80000, CRC(89237f10) SHA1(a493a03f79656332089f2794872be44e62d7e306) )
	ROM_LOAD( "bootleg_rc3_.bin", 0x200000, 0x80000, CRC(c4147d05) SHA1(8497fca6e64896cf8f03877c3455ee7bf9965b60) )
	ROM_LOAD( "bootleg_rc4_.bin", 0x300000, 0x80000, CRC(06176cb1) SHA1(0644861042ca4be2d459b31870369a2e46d80aa4) )
	ROM_LOAD( "bootleg_rc5_.bin", 0x080000, 0x80000, CRC(fb5f2036) SHA1(5f8885332a2d9249d34a0a50ac464a5637b9bc95) )
	ROM_LOAD( "bootleg_rc6_.bin", 0x180000, 0x80000, CRC(36a8148f) SHA1(57d84e44c77f2bb1e97ec8a1acd3ef71246e3274) )
	ROM_LOAD( "bootleg_rc7_.bin", 0x280000, 0x80000, CRC(26c620e3) SHA1(b14f2cfc0e5b740360e6ecca97f6bf941136141f) )
	ROM_LOAD( "bootleg_rc8_.bin", 0x380000, 0x80000, CRC(8cf6b4c2) SHA1(4c36c217b83c82acfdd615f5547bf597af7b8833) )
ROM_END

ROM_START( rclimb_3c ) // 040827 custom alteras, modified graphics, many texts changed, changed version text to "VIDEO GAME-1 SK01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_rc_m_040827c.rom", 0x00000, 0x40000, CRC(4b8b3e0e) SHA1(6554073d589de67807c3b3779c1f2fa610aa03df) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1b", 0x000000, 0x80000, CRC(a66e718a) SHA1(84493efde5a70032e709ebde5635ed50e151d2ab) )
	ROM_LOAD( "bootleg_2b", 0x100000, 0x80000, CRC(3486c691) SHA1(658e0f7e1e7c874c6e8f9f1908d242ad57f66cf1) )
	ROM_LOAD( "bootleg_3b", 0x200000, 0x80000, CRC(4600b9c7) SHA1(ce8e8474705592b246933662b24c579bee45d6b1) )
	ROM_LOAD( "bootleg_4b", 0x300000, 0x80000, CRC(610826f6) SHA1(52ad56da44bf3803ea1a30fbeaf7c7cab3c347dc) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(77413756) SHA1(41eac1ee200d9d850e494f79555a82bb2a591e1f) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(6f8f3678) SHA1(b9c0e5093f93a3e9ebc715a24588a68105ca15c7) )
	ROM_LOAD( "bootleg_7b", 0x280000, 0x80000, CRC(72d3cba8) SHA1(2f14bc23574276cb1ceef4306653e79f7bf19ac9) )
	ROM_LOAD( "bootleg_8b", 0x380000, 0x80000, CRC(4210c5e2) SHA1(053cee86e3fa27e43ef74615cd4bb1f28c900ec4) )
ROM_END

ROM_START( rclimb_3d ) // 040827 modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_rc_m_040827d.rom", 0x00000, 0x40000, CRC(b7796cff) SHA1(a180397d8827adbc161e282a4ee6d064d4837199) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1c", 0x000000, 0x80000, CRC(b846d5c8) SHA1(235ebe08ef89b308dd9642ebb8ce0eb102ffaf7d) )
	ROM_LOAD( "bootleg_2c", 0x100000, 0x80000, CRC(600cfee9) SHA1(b0f251cd9e6bff195f44bd70a961ccc489688574) )
	ROM_LOAD( "bootleg_3c", 0x200000, 0x80000, CRC(d8394c53) SHA1(a20862c0753cba6b5f4db03c9a208381c769a3e5) )
	ROM_LOAD( "bootleg_4c", 0x300000, 0x80000, CRC(7b0911b8) SHA1(25fa9b021cc041eccbbf0af1ec613895cc79a973) )
	ROM_LOAD( "rockclimber_m.005", 0x080000, 0x80000, CRC(ea127c3d) SHA1(a6391eed69a4723b68d727f59b6baebe51633e66) )
	ROM_LOAD( "rockclimber_m.006", 0x180000, 0x80000, CRC(277fa273) SHA1(6320e6c5b5e48dc451cc48189054c42d85e8ccc1) )
	ROM_LOAD( "rockclimber_m.007", 0x280000, 0x80000, CRC(3ca7f69a) SHA1(878cca181d915dc3548d5285a4bbb51aef31a64e) )
	ROM_LOAD( "rockclimber_m.008", 0x380000, 0x80000, CRC(8cf6b4c2) SHA1(4c36c217b83c82acfdd615f5547bf597af7b8833) )
ROM_END

ROM_START( rclimb_3e ) // 040827 custom alteras, modified graphics, many texts changed, changed version text to "LOTOS SK01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_rc_m_040827e.rom", 0x00000, 0x40000, CRC(0ae53856) SHA1(af6b744addc8fb429464e5dbe28c8ebcd2925dec) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1d", 0x000000, 0x80000, CRC(085caab6) SHA1(32a6a9160f9a175ba74ef60d36e1715efe7599b4) )
	ROM_LOAD( "bootleg_2d", 0x100000, 0x80000, CRC(29af4561) SHA1(dcc9f8b910a8b55abb79d47ce6b3e3c0fc37f2e8) )
	ROM_LOAD( "bootleg_3d", 0x200000, 0x80000, CRC(f0576bfd) SHA1(3b51893589843f52c6d83328c8f0cdb80e094e07) )
	ROM_LOAD( "bootleg_4d", 0x300000, 0x80000, CRC(66e10e16) SHA1(3ed013ec287347d817345815c7e8d2a6a4e5563a) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(77413756) SHA1(41eac1ee200d9d850e494f79555a82bb2a591e1f) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(6f8f3678) SHA1(b9c0e5093f93a3e9ebc715a24588a68105ca15c7) )
	ROM_LOAD( "bootleg_7b", 0x280000, 0x80000, CRC(72d3cba8) SHA1(2f14bc23574276cb1ceef4306653e79f7bf19ac9) )
	ROM_LOAD( "bootleg_8b", 0x380000, 0x80000, CRC(4210c5e2) SHA1(053cee86e3fa27e43ef74615cd4bb1f28c900ec4) )
ROM_END



ROM_START( sweetla ) // 041220 backdoor 1,5 9,1 5,3 1,5  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_sl_m_041220a.rom", 0x00000, 0x40000, CRC(920fd9fe) SHA1(0b5ad099ae4c8e3ba0f99baf8fc9322cae24e9d2) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "sweetlife.001", 0x000000, 0x80000, CRC(a096c786) SHA1(81f6b083cb089e9412a8506889196354c670d945) )
	ROM_LOAD( "sweetlife.002", 0x100000, 0x80000, CRC(c5e1e22c) SHA1(973ad27681a0f3beee7084b1b85fc9deb79d638e) )
	ROM_LOAD( "sweetlife.003", 0x200000, 0x80000, CRC(af335323) SHA1(b8afdce231a8ec0f313cc47e00a27f05461bbbc4) )
	ROM_LOAD( "sweetlife.004", 0x300000, 0x80000, CRC(a35c7503) SHA1(78f7a868660bbaa066e8e9e341db52018aaf3af1) )
	ROM_LOAD( "sweetlife.005", 0x080000, 0x80000, CRC(e2d6b632) SHA1(65d05e55671b8c335cae2dfbf6a6f5bd8cc90e2c) )
	ROM_LOAD( "sweetlife.006", 0x180000, 0x80000, CRC(d34e0905) SHA1(cc4afe64fb9052a31f759be41ff07a727e0a9093) )
	ROM_LOAD( "sweetlife.007", 0x280000, 0x80000, CRC(978b67bb) SHA1(87357d5832588f00272bd76df736c06c599f3853) )
	ROM_LOAD( "sweetlife.008", 0x380000, 0x80000, CRC(75954355) SHA1(e6ef2b70d859b61e8e3d1751de8558b8778e502d) )
ROM_END

ROM_START( sweetlb ) // 041220 bank F9, changed version text to 070412
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_sl_m_041220b.rom", 0x00000, 0x40000, CRC(53f1a0e1) SHA1(a00a8e8d95bd2be27313c1421d40120ee79ff29d) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "sweetlife.001", 0x000000, 0x80000, CRC(a096c786) SHA1(81f6b083cb089e9412a8506889196354c670d945) )
	ROM_LOAD( "sweetlife.002", 0x100000, 0x80000, CRC(c5e1e22c) SHA1(973ad27681a0f3beee7084b1b85fc9deb79d638e) )
	ROM_LOAD( "sweetlife.003", 0x200000, 0x80000, CRC(af335323) SHA1(b8afdce231a8ec0f313cc47e00a27f05461bbbc4) )
	ROM_LOAD( "sweetlife.004", 0x300000, 0x80000, CRC(a35c7503) SHA1(78f7a868660bbaa066e8e9e341db52018aaf3af1) )
	ROM_LOAD( "sweetlife.005", 0x080000, 0x80000, CRC(e2d6b632) SHA1(65d05e55671b8c335cae2dfbf6a6f5bd8cc90e2c) )
	ROM_LOAD( "sweetlife.006", 0x180000, 0x80000, CRC(d34e0905) SHA1(cc4afe64fb9052a31f759be41ff07a727e0a9093) )
	ROM_LOAD( "sweetlife.007", 0x280000, 0x80000, CRC(978b67bb) SHA1(87357d5832588f00272bd76df736c06c599f3853) )
	ROM_LOAD( "sweetlife.008", 0x380000, 0x80000, CRC(75954355) SHA1(e6ef2b70d859b61e8e3d1751de8558b8778e502d) )
ROM_END


ROM_START( sweetl2_2a ) // 080320 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_sl2_m_080320a.rom", 0x00000, 0x40000, CRC(425066e9) SHA1(f2b11c3c6e08cb59c7c6841ee3575503ed4abb80) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "sweetlife2.001", 0x000000, 0x80000, CRC(b1e6157c) SHA1(e042aaaf85b13865d56d1709be280f5a3d5c95e3) )
	ROM_LOAD( "sweetlife2.002", 0x100000, 0x80000, CRC(8bc54047) SHA1(9179cf02376fec77ae50b5603de66754237698f8) )
	ROM_LOAD( "sweetlife2.003", 0x200000, 0x80000, CRC(709c5d34) SHA1(d702576d70ebef667c77376effecc02abbaaa8cd) )
	ROM_LOAD( "sweetlife2.004", 0x300000, 0x80000, CRC(7bcfa2ce) SHA1(8df894e9b7afa52e47e4dabd1802e878bbbeaa80) )
	ROM_LOAD( "sweetlife2.005", 0x080000, 0x80000, CRC(5c385d43) SHA1(ddb362e5894d146ce90acbaafe9dd7aad2a7c242) )
	ROM_LOAD( "sweetlife2.006", 0x180000, 0x80000, CRC(868fe1cb) SHA1(692679f8242950e009c30cc084c4ddc5d1963502) )
	ROM_LOAD( "sweetlife2.007", 0x280000, 0x80000, CRC(6ce87282) SHA1(586e08994db4ca2b967d47b16ba5b458e240d30f) )
	ROM_LOAD( "sweetlife2.008", 0x380000, 0x80000, CRC(c2ad2b74) SHA1(c78e3ca5d15acb17ee671d2205405f287ad9c464) )
ROM_END

ROM_START( sweetl2_2b ) // 080320 bank F9, some fixes
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_sl2_m_080320b.rom", 0x00000, 0x40000, CRC(faffe8c3) SHA1(66a8247fc4bc6d7fb2eefcec37d56eabb1fbca57) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "sweetlife2.001", 0x000000, 0x80000, CRC(b1e6157c) SHA1(e042aaaf85b13865d56d1709be280f5a3d5c95e3) )
	ROM_LOAD( "sweetlife2.002", 0x100000, 0x80000, CRC(8bc54047) SHA1(9179cf02376fec77ae50b5603de66754237698f8) )
	ROM_LOAD( "sweetlife2.003", 0x200000, 0x80000, CRC(709c5d34) SHA1(d702576d70ebef667c77376effecc02abbaaa8cd) )
	ROM_LOAD( "sweetlife2.004", 0x300000, 0x80000, CRC(7bcfa2ce) SHA1(8df894e9b7afa52e47e4dabd1802e878bbbeaa80) )
	ROM_LOAD( "sweetlife2.005", 0x080000, 0x80000, CRC(5c385d43) SHA1(ddb362e5894d146ce90acbaafe9dd7aad2a7c242) )
	ROM_LOAD( "sweetlife2.006", 0x180000, 0x80000, CRC(868fe1cb) SHA1(692679f8242950e009c30cc084c4ddc5d1963502) )
	ROM_LOAD( "sweetlife2.007", 0x280000, 0x80000, CRC(6ce87282) SHA1(586e08994db4ca2b967d47b16ba5b458e240d30f) )
	ROM_LOAD( "sweetlife2.008", 0x380000, 0x80000, CRC(c2ad2b74) SHA1(c78e3ca5d15acb17ee671d2205405f287ad9c464) )
ROM_END

ROM_START( sweetl2_2c ) // 080320 modified graphics, bank F9, changed version text to "VIDEO GAME-1 MD01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_sl2_m_080320c.rom", 0x00000, 0x40000, CRC(d31d682d) SHA1(a8addbed8373f06d90648c01dc6fda584789f25c) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1a", 0x000000, 0x80000, CRC(aa9e41f4) SHA1(50c4078581e902aab0dfd902c5ae82ba50f03e39) )
	ROM_LOAD( "bootleg_2a", 0x100000, 0x80000, CRC(35a92e6b) SHA1(48a1856a8970383619dd5acd3026630ce8c2dfe9) )
	ROM_LOAD( "bootleg_3a", 0x200000, 0x80000, CRC(b8bd46b4) SHA1(c4c9c6c94b7d6e2e6b02f61177407464b75c76bb) )
	ROM_LOAD( "bootleg_4a", 0x300000, 0x80000, CRC(e4a8f35d) SHA1(f45dbdc5757bd86218e2c975f9c127c2b3077aef) )
	ROM_LOAD( "bootleg_5a", 0x080000, 0x80000, CRC(7bc3d048) SHA1(4485c028757b4ade722f7a4d72ce87bc94c08a08) )
	ROM_LOAD( "bootleg_6a", 0x180000, 0x80000, CRC(c334b279) SHA1(55bace43fe2aee54ab9cc787a955f382f05cb666) )
	ROM_LOAD( "bootleg_7a", 0x280000, 0x80000, CRC(9e8cfe95) SHA1(2594c66d65ec4b23fe7760ed7f05761b95ec1bde) )
	ROM_LOAD( "bootleg_8a", 0x380000, 0x80000, CRC(f4b48eee) SHA1(02af6ea807d003504a37c4dc7f9111b716cf78a5) )
ROM_END

ROM_START( sweetl2_2d ) // 080320 bank F9, modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_sl2_m_080320d.rom", 0x00000, 0x40000, CRC(03b26c85) SHA1(9c05e48f8d2e467da8b267021560b121538b8c3e) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1b", 0x000000, 0x80000, CRC(a5119aca) SHA1(ed59477438ba2b5f17f7c3ab9fa769f5aa28ed69) )
	ROM_LOAD( "bootleg_2b", 0x100000, 0x80000, CRC(689f09f5) SHA1(8bf13f453e51a0d1abbc00a2fc92e8f544cb176f) )
	ROM_LOAD( "bootleg_3b", 0x200000, 0x80000, CRC(925abdcb) SHA1(93ffea087f754614e3ceaf43c00a94a3525f4ef8) )
	ROM_LOAD( "bootleg_4b", 0x300000, 0x80000, CRC(94c525a3) SHA1(0c67fd937c85774c10d6671c7ea4b9733d760cdc) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(98d76993) SHA1(39dc0b650d5437f5fae70651ed67dc3f80ea45a4) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(f85255d9) SHA1(eb9e8de3775c9a1ddc1134c7b96e556ec2e87b1d) )
	ROM_LOAD( "sweetlife2.007",  0x280000, 0x80000, CRC(6ce87282) SHA1(586e08994db4ca2b967d47b16ba5b458e240d30f) )
	ROM_LOAD( "sweetlife2.008",  0x380000, 0x80000, CRC(c2ad2b74) SHA1(c78e3ca5d15acb17ee671d2205405f287ad9c464) )
ROM_END



ROM_START( resdnt_2a ) // 040513 backdoor 1,5 9,1 5,1 1,5  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_rs_m_040513a.rom", 0x00000, 0x40000, CRC(5b6480d7) SHA1(e54ddc822819136687d613ce4f38dd98f3e01bb5) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "resident_m.001", 0x000000, 0x80000, CRC(e0645da6) SHA1(dd72f4830d8011f603aa6d430f34ac2598005281) )
	ROM_LOAD( "resident_m.002", 0x100000, 0x80000, CRC(dd8de247) SHA1(498c5b931ce65e289f52d8864b603166f81e3dc4) )
	ROM_LOAD( "resident_m.003", 0x200000, 0x80000, CRC(0d346ec2) SHA1(e2456b28825c54c5e16829525627c40611c0083d) )
	ROM_LOAD( "resident_m.004", 0x300000, 0x80000, CRC(1f95aad9) SHA1(51d003288d5ff23b3c981fbaa99d29b66dd2c101) )
	ROM_LOAD( "resident_m.005", 0x080000, 0x80000, CRC(0cfe7d44) SHA1(9f0e4925e815ff9f79188f18e78c0a7b377daa3f) )
	ROM_LOAD( "resident_m.006", 0x180000, 0x80000, CRC(7437904f) SHA1(630c79cd6a990ce7658a1ffabba5a27efba985a1) )
	ROM_LOAD( "resident_m.007", 0x280000, 0x80000, CRC(6e94728a) SHA1(ab414879cb957d9bc8d653b5e3bb2bbf91139ec0) )
	ROM_LOAD( "resident_m.008", 0x380000, 0x80000, CRC(a9f55043) SHA1(4771df3d45bdc0a21b1c479f45e09ac5bab6c94f) )
ROM_END

ROM_START( resdnt_2b ) // 040513 custom alteras, modified graphics, changed version text to "VIDEO GAME-1 SE01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_rs_m_040513b.rom", 0x00000, 0x40000, CRC(2f8b2249) SHA1(4fb34888a99946937791fdba4659d256e862598b) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1b", 0x000000, 0x80000, CRC(529ac635) SHA1(76cb5b04d61a1e5cb358b55a1b8a5602edd2841a) )
	ROM_LOAD( "bootleg_2b", 0x100000, 0x80000, CRC(84033d15) SHA1(632a18b059ec94d195e311080baf1f5f7d9c8e6c) )
	ROM_LOAD( "bootleg_3b", 0x200000, 0x80000, CRC(2b456ab0) SHA1(bf7971bd6ad6fcf7bc964b5c30cf9ead828e9fd3) )
	ROM_LOAD( "bootleg_4b", 0x300000, 0x80000, CRC(4e02adf4) SHA1(63d2e40577171be0f8c2591d7cd3500c83f3ab1f) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(9613a21c) SHA1(d514cd8e6d7695dc8be52bc5acb04991ebaf3499) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(2a4364c6) SHA1(f2760d7416585123a46ffb47ab9da008532f64bb) )
	ROM_LOAD( "bootleg_7b", 0x280000, 0x80000, CRC(a34d91d0) SHA1(9616952b1e8b6c06c8bca3c776bdfa3526ef60cf) )
	ROM_LOAD( "bootleg_8b", 0x380000, 0x80000, CRC(a0be7be4) SHA1(d6f4a1ba22be638fa801923960da8f5f72fc16c0) )
ROM_END

ROM_START( resdnt_2c ) // 040513 custom alteras, modified graphics, changed version text to "VIDEO GAME-1 SE01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_rs_m_040513b.rom", 0x00000, 0x40000, CRC(2f8b2249) SHA1(4fb34888a99946937791fdba4659d256e862598b) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1c", 0x000000, 0x80000, CRC(31bdf1c6) SHA1(b4c1555cd1cd1eaec5e39ed5879a99633fca5c46) )
	ROM_LOAD( "bootleg_2c", 0x100000, 0x80000, CRC(861fbf05) SHA1(6348236124792719afbd0117dea35a6af059355c) )
	ROM_LOAD( "bootleg_3c", 0x200000, 0x80000, CRC(7de96e4a) SHA1(1641655b95b35825de237b07142f4097a757cf6c) )
	ROM_LOAD( "bootleg_4c", 0x300000, 0x80000, CRC(179cf46c) SHA1(1585a6e54b393154c1e7aa6e5e116d45f34da07b) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(9613a21c) SHA1(d514cd8e6d7695dc8be52bc5acb04991ebaf3499) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(2a4364c6) SHA1(f2760d7416585123a46ffb47ab9da008532f64bb) )
	ROM_LOAD( "bootleg_7b", 0x280000, 0x80000, CRC(a34d91d0) SHA1(9616952b1e8b6c06c8bca3c776bdfa3526ef60cf) )
	ROM_LOAD( "bootleg_8b", 0x380000, 0x80000, CRC(a0be7be4) SHA1(d6f4a1ba22be638fa801923960da8f5f72fc16c0) )
ROM_END

ROM_START( resdnt_2d ) // 040513 custom alteras, modified graphics, many texts changed, changed version text to "VIDEO GAME-1 SE01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_rs_m_040513d.rom", 0x00000, 0x40000, CRC(3481a7b0) SHA1(fb9fa5aea082c80a232d09a938c8b0007e582cc7) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1d", 0x000000, 0x80000, CRC(e13336d0) SHA1(a427a3eeecf29d7760ffb0955719f282b6688f79) )
	ROM_LOAD( "bootleg_2d", 0x100000, 0x80000, CRC(16ed4ed0) SHA1(a9d012fcd53aa912594ba42e677c0061a5daeb89) )
	ROM_LOAD( "bootleg_3d", 0x200000, 0x80000, CRC(bddd68bd) SHA1(97a7899ba0386249a1b40eee32db7ef96734bea6) )
	ROM_LOAD( "bootleg_4d", 0x300000, 0x80000, CRC(78c509f9) SHA1(171ed587d124b90fd68bd747e02322d16f7d08be) )
	ROM_LOAD( "bootleg_5d", 0x080000, 0x80000, CRC(8d4205df) SHA1(669b95bb06d235a9cc977a2e714c2e722ca385f1) )
	ROM_LOAD( "bootleg_6d", 0x180000, 0x80000, CRC(1e06508e) SHA1(df64f3849945f0303e55de7061d3fc3370a27299) )
	ROM_LOAD( "bootleg_7d", 0x280000, 0x80000, CRC(e896b62f) SHA1(fb97ff11b49345fda04d5b1f03ec96fdf4a71419) )
	ROM_LOAD( "bootleg_8d", 0x380000, 0x80000, CRC(a8de4e0a) SHA1(92264e99a7c90967bf71ca440b5cd4df4baeb553) )
ROM_END

ROM_START( resdnt_2e ) // 040513 modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_rs_m_040513e.rom", 0x00000, 0x40000, CRC(83fb15e2) SHA1(59b2fbc218802787bc4c7a6324248d1c4bea459b) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1e", 0x000000, 0x80000, CRC(7e9cb8a1) SHA1(5d089b41897dbb90b8209e60c0746196033b8945) )
	ROM_LOAD( "bootleg_2e", 0x100000, 0x80000, CRC(474c2a5d) SHA1(54e317706d1b4328ff006dcace11a112b9533b8f) )
	ROM_LOAD( "bootleg_3e", 0x200000, 0x80000, CRC(8407a51d) SHA1(d790efa0cd4c38c26e95bf10c63a751694840c52) )
	ROM_LOAD( "bootleg_4e", 0x300000, 0x80000, CRC(65f51297) SHA1(799762a7833510a4c513bd3ffb5bd4e541608cca) )
	ROM_LOAD( "resident_m.005", 0x080000, 0x80000, CRC(0cfe7d44) SHA1(9f0e4925e815ff9f79188f18e78c0a7b377daa3f) )
	ROM_LOAD( "resident_m.006", 0x180000, 0x80000, CRC(7437904f) SHA1(630c79cd6a990ce7658a1ffabba5a27efba985a1) )
	ROM_LOAD( "resident_m.007", 0x280000, 0x80000, CRC(6e94728a) SHA1(ab414879cb957d9bc8d653b5e3bb2bbf91139ec0) )
	ROM_LOAD( "resident_m.008", 0x380000, 0x80000, CRC(a9f55043) SHA1(4771df3d45bdc0a21b1c479f45e09ac5bab6c94f) )
ROM_END

ROM_START( resdnt_2f ) // 040513 modified graphics, many texts changed, changed version text to "LOTO PROGRAM V-RS2"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_rs_m_040513f.rom", 0x00000, 0x40000, CRC(2f711769) SHA1(07333d5837484107681561a2434c8668ec77a7f4) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1f", 0x000000, 0x80000, CRC(8cefec30) SHA1(3bf400bdff230e2af83c377617e162818a8c70a4) )
	ROM_LOAD( "bootleg_2f", 0x100000, 0x80000, CRC(5318fc74) SHA1(8c6f6f0de426228ca24d552b246f5d0f5feb8611) )
	ROM_LOAD( "bootleg_3f", 0x200000, 0x80000, CRC(5ff2bf17) SHA1(c0e23e02cd4e39fb2a99af64a002474f1bc48131) )
	ROM_LOAD( "bootleg_4f", 0x300000, 0x80000, CRC(9e9a90fa) SHA1(dadacfded904401a1f1f0e31c8218763fd01cff8) )
	ROM_LOAD( "resident_m.005", 0x080000, 0x80000, CRC(0cfe7d44) SHA1(9f0e4925e815ff9f79188f18e78c0a7b377daa3f) )
	ROM_LOAD( "resident_m.006", 0x180000, 0x80000, CRC(7437904f) SHA1(630c79cd6a990ce7658a1ffabba5a27efba985a1) )
	ROM_LOAD( "resident_m.007", 0x280000, 0x80000, CRC(6e94728a) SHA1(ab414879cb957d9bc8d653b5e3bb2bbf91139ec0) )
	ROM_LOAD( "resident_m.008", 0x380000, 0x80000, CRC(a9f55043) SHA1(4771df3d45bdc0a21b1c479f45e09ac5bab6c94f) )
ROM_END

ROM_START( resdnt_2g ) // 040513 custom alteras, modified graphics, many texts changed, changed version text to "LOTOS SE01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_rs_m_040513g.rom", 0x00000, 0x40000, CRC(3c061c32) SHA1(c71376da261f1f69cfa78a243444ea9963f9000e) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1g", 0x000000, 0x80000, CRC(f98cda01) SHA1(c8bb11230d4659ae57429ed3ec56dd26af2a6332) )
	ROM_LOAD( "bootleg_2g", 0x100000, 0x80000, CRC(c3ccf6d0) SHA1(adbe8772c731c000309e7e56acdb92823cae7971) )
	ROM_LOAD( "bootleg_3g", 0x200000, 0x80000, CRC(564d2255) SHA1(d589e6c12d62c9c5a316e4cd0512576f5b8ec916) )
	ROM_LOAD( "bootleg_4g", 0x300000, 0x80000, CRC(66b98fc5) SHA1(6010053fd0f4b5b062bbfceb3ce417d6cc618ad1) )
	ROM_LOAD( "bootleg_5d", 0x080000, 0x80000, CRC(8d4205df) SHA1(669b95bb06d235a9cc977a2e714c2e722ca385f1) )
	ROM_LOAD( "bootleg_6d", 0x180000, 0x80000, CRC(1e06508e) SHA1(df64f3849945f0303e55de7061d3fc3370a27299) )
	ROM_LOAD( "bootleg_7d", 0x280000, 0x80000, CRC(e896b62f) SHA1(fb97ff11b49345fda04d5b1f03ec96fdf4a71419) )
	ROM_LOAD( "bootleg_8d", 0x380000, 0x80000, CRC(a8de4e0a) SHA1(92264e99a7c90967bf71ca440b5cd4df4baeb553) )
ROM_END



ROM_START( islanda ) // 050713 backdoor 1,1 1,3 1,5 1,7  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_is_m_050713a.rom", 0x00000, 0x40000, CRC(d3d62cb3) SHA1(2ceb83ac9d59a570435220f06e8317057bb46608) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "island.001", 0x000000, 0x80000, CRC(dbe8cdda) SHA1(4747cf0d85afdef22d3ba9fa5e75b39548725745) )
	ROM_LOAD( "island.002", 0x100000, 0x80000, CRC(64064745) SHA1(91a7bc7204a8f7a7512eeaf4906da20a9f587565) )
	ROM_LOAD( "island.003", 0x200000, 0x80000, CRC(1d993f68) SHA1(b0459d3941d50668f7533909e3f3da91453d3efd) )
	ROM_LOAD( "island.004", 0x300000, 0x80000, CRC(a4739404) SHA1(8f7ffcc13dcb35adfa8060ab1930d07195b6110c) )
	ROM_LOAD( "island.005", 0x080000, 0x80000, CRC(d016eb31) SHA1(a84f18af470f72730b241b9031cd6131c8a03db2) )
	ROM_LOAD( "island.006", 0x180000, 0x80000, CRC(0faaa968) SHA1(0f05546e6e0559e24c6afdde65b3feeb66b6adff) )
	ROM_LOAD( "island.007", 0x280000, 0x80000, CRC(d7277a6c) SHA1(d96a0befc965ad22087381982305d68208978a7e) )
	ROM_LOAD( "island.008", 0x380000, 0x80000, CRC(ac6fba48) SHA1(64dd03d624f16da52bc7fa0702246e91ae39a806) )
ROM_END

ROM_START( islandb ) // 050713 custom alteras, modified graphics, many texts changed, changed version text to "VIDEO GAME-1 OS01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_is_m_050713b.rom", 0x00000, 0x40000, CRC(535d4283) SHA1(d4823b1487aa9c7f07e84968de563a8c18f3e770) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1a", 0x000000, 0x80000, CRC(95195fdc) SHA1(4ef22f9d6e539555303f042e53c42acf658a6027) )
	ROM_LOAD( "bootleg_2a", 0x100000, 0x80000, CRC(0d6d3104) SHA1(2b9f1ee3854a6e0645d698757e89aebd4831e31e) )
	ROM_LOAD( "bootleg_3a", 0x200000, 0x80000, CRC(981a067b) SHA1(3a75caa39c13b7fdb4ce56a6144c7d715d455a3b) )
	ROM_LOAD( "bootleg_4a", 0x300000, 0x80000, CRC(2e235348) SHA1(148d898271feb03dbef5b1efb65571f813c58be7) )
	ROM_LOAD( "bootleg_5a", 0x080000, 0x80000, CRC(feba83ab) SHA1(0609f2d4b98fb5dcdf80102c32a26f29fd30f50d) )
	ROM_LOAD( "bootleg_6a", 0x180000, 0x80000, CRC(f59eaafa) SHA1(c324ff461b997c79572644b2974db33a156a00e8) )
	ROM_LOAD( "bootleg_7a", 0x280000, 0x80000, CRC(5248d6b8) SHA1(1ef6fbc56c81b641003b16eceffb8d0b3d385546) )
	ROM_LOAD( "bootleg_8a", 0x380000, 0x80000, CRC(47d37fe9) SHA1(1c12cadfe265bd18539af8c94deef674e179eea4) )
ROM_END

ROM_START( islandc ) // 050713 custom alteras, modified graphics, many texts changed, changed version text to "LOTOS OS01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_is_m_050713c.rom", 0x00000, 0x40000, CRC(c879a78e) SHA1(61bf87f50cbf4105330e57e1af7802100286d888) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1b", 0x000000, 0x80000, CRC(eccf5648) SHA1(7b9dc15e8fac9dcc8a14bda52df4d03462a242d0) )
	ROM_LOAD( "bootleg_2b", 0x100000, 0x80000, CRC(9d85dcfb) SHA1(3752e9e4c1b8485a37e7c68175dc42aa7927c485) )
	ROM_LOAD( "bootleg_3b", 0x200000, 0x80000, CRC(dd208792) SHA1(ee7cf68b40719e783c70584d7da87875280283dd) )
	ROM_LOAD( "bootleg_4b", 0x300000, 0x80000, CRC(9d8de4db) SHA1(9935569285f456decc2b851bef07d60a4e440229) )
	ROM_LOAD( "bootleg_5a", 0x080000, 0x80000, CRC(feba83ab) SHA1(0609f2d4b98fb5dcdf80102c32a26f29fd30f50d) )
	ROM_LOAD( "bootleg_6a", 0x180000, 0x80000, CRC(f59eaafa) SHA1(c324ff461b997c79572644b2974db33a156a00e8) )
	ROM_LOAD( "bootleg_7a", 0x280000, 0x80000, CRC(5248d6b8) SHA1(1ef6fbc56c81b641003b16eceffb8d0b3d385546) )
	ROM_LOAD( "bootleg_8a", 0x380000, 0x80000, CRC(47d37fe9) SHA1(1c12cadfe265bd18539af8c94deef674e179eea4) )
ROM_END



ROM_START( island2a ) // 060529 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_is2_m_060529a.rom", 0x00000, 0x40000, CRC(4341d65c) SHA1(e7120c805d7dbf0fee5d18243ddf2cfa19a0d88c) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "island2.001", 0x000000, 0x80000, CRC(f8dd9fe9) SHA1(0cf67fbca107b255011fded6390507d12cbac514) )
	ROM_LOAD( "island2.002", 0x100000, 0x80000, CRC(4f9607c0) SHA1(cd3e7b4a88f46231a115c9d18a26b5e30fea74e4) )
	ROM_LOAD( "island2.003", 0x200000, 0x80000, CRC(bceccdba) SHA1(5cf3b51ccfe317ca57d770bff0204b0ee83d1173) )
	ROM_LOAD( "island2.004", 0x300000, 0x80000, CRC(15fdecc7) SHA1(6882ea10f117c85544df51d9abd67ef52db91d95) )
	ROM_LOAD( "island2.005", 0x080000, 0x80000, CRC(2d4905aa) SHA1(c4a1e4db61e8af6cf0fb70aabe5e3896ab5227ca) )
	ROM_LOAD( "island2.006", 0x180000, 0x80000, CRC(55e285d9) SHA1(ba58963441c65220700cd8057e6afe3f5f8faa4f) )
	ROM_LOAD( "island2.007", 0x280000, 0x80000, CRC(edd72be6) SHA1(fb1e63f59e8565c23ae43630fa572fbc022c878f) )
	ROM_LOAD( "island2.008", 0x380000, 0x80000, CRC(c336d608) SHA1(55391183c6d95ecea81354efa70641350860d1f5) )

	ROM_REGION( 0x8000, "m48t35", 0 )
	ROM_LOAD( "m48t35",      0x000000, 0x08000, CRC(c8ec9973) SHA1(7973189d9b380ca2591d3ef3b80446410f7a8ed8) )
ROM_END

ROM_START( island2b ) // 060529 bank F9, changed version text to 070205, skip some start tests
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_is2_m_060529b.rom", 0x00000, 0x40000, CRC(617d9a35) SHA1(96947342216872984e13e9997552f8411b268467) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "island2.001", 0x000000, 0x80000, CRC(f8dd9fe9) SHA1(0cf67fbca107b255011fded6390507d12cbac514) )
	ROM_LOAD( "island2.002", 0x100000, 0x80000, CRC(4f9607c0) SHA1(cd3e7b4a88f46231a115c9d18a26b5e30fea74e4) )
	ROM_LOAD( "island2.003", 0x200000, 0x80000, CRC(bceccdba) SHA1(5cf3b51ccfe317ca57d770bff0204b0ee83d1173) )
	ROM_LOAD( "island2.004", 0x300000, 0x80000, CRC(15fdecc7) SHA1(6882ea10f117c85544df51d9abd67ef52db91d95) )
	ROM_LOAD( "island2.005", 0x080000, 0x80000, CRC(2d4905aa) SHA1(c4a1e4db61e8af6cf0fb70aabe5e3896ab5227ca) )
	ROM_LOAD( "island2.006", 0x180000, 0x80000, CRC(55e285d9) SHA1(ba58963441c65220700cd8057e6afe3f5f8faa4f) )
	ROM_LOAD( "island2.007", 0x280000, 0x80000, CRC(edd72be6) SHA1(fb1e63f59e8565c23ae43630fa572fbc022c878f) )
	ROM_LOAD( "island2.008", 0x380000, 0x80000, CRC(c336d608) SHA1(55391183c6d95ecea81354efa70641350860d1f5) )
ROM_END

ROM_START( island2c ) // 060529 bank F9, modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_is2_m_060529c.rom", 0x00000, 0x40000, CRC(cbf1ec14) SHA1(ec6dc40deacc1a1e0bbe2778d834fb5eb392ca04) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1b", 0x000000, 0x80000, CRC(4ee179af) SHA1(77e48ff2a45bb4eb8f68b92044791f6f4fad09f5) )
	ROM_LOAD( "bootleg_2b", 0x100000, 0x80000, CRC(764a7e33) SHA1(7a0e32c2fb86296cd316e23311f95cc3d183c3b8) )
	ROM_LOAD( "bootleg_3b", 0x200000, 0x80000, CRC(9680c9cb) SHA1(2ca8d02ce8aa168e3561c47ca140397857096811) )
	ROM_LOAD( "bootleg_4b", 0x300000, 0x80000, CRC(8443f48d) SHA1(568fe79ecef3a120cde4d807a2e63d8e8125e859) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(8aa21c72) SHA1(5f1b95af245c81df699ef346ab790b971ec5b8ab) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(b4b7dc6e) SHA1(8b1b195e55d402e298a45b9b5d1b8bff39f6aecd) )
	ROM_LOAD( "bootleg_7",  0x280000, 0x80000, CRC(edd72be6) SHA1(fb1e63f59e8565c23ae43630fa572fbc022c878f) )
	ROM_LOAD( "bootleg_8",  0x380000, 0x80000, CRC(c336d608) SHA1(55391183c6d95ecea81354efa70641350860d1f5) )
ROM_END

ROM_START( island2_3a ) // 061218 bank F9, modified graphics, changed version text to "VIDEO GAME-1 OS2-01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_is2_m_061218a.rom", 0x00000, 0x40000, CRC(f190c856) SHA1(c3b762c8b2e24a75b91e93a6c058d8a4a86de60e) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1a", 0x000000, 0x80000, CRC(ea6c4724) SHA1(f0054dc2e0d7c49e5ca96ce5aaf4c08ddb9d56d9) )
	ROM_LOAD( "bootleg_2a", 0x100000, 0x80000, CRC(f005e13b) SHA1(f16e3af752c35bac6aafa6bc14abbdb80ddb85d3) )
	ROM_LOAD( "bootleg_3a", 0x200000, 0x80000, CRC(df3a9181) SHA1(253a0dc28d027bb7573ae62a0631d7cedb29f344) )
	ROM_LOAD( "bootleg_4a", 0x300000, 0x80000, CRC(5d86af03) SHA1(d79053aa5468e70d102bfedaed3b030c75f32a0d) )
	ROM_LOAD( "bootleg_5a", 0x080000, 0x80000, CRC(8f2a5fa5) SHA1(5c84cf84e6e7141bd315763edc1d7904fb9b1776) )
	ROM_LOAD( "bootleg_6a", 0x180000, 0x80000, CRC(0072562b) SHA1(df6f24149a194d8654fd10bce4969bd9fc9d2165) )
	ROM_LOAD( "bootleg_7a", 0x280000, 0x80000, CRC(a325a291) SHA1(de3d4f1ab803faa08a50e4f5dc5a78cd67f5ec1f) )
	ROM_LOAD( "bootleg_8a", 0x380000, 0x80000, CRC(d6b228ca) SHA1(70c62c38291e7b2bae8f30e1b1488b1d8bc1435c) )
ROM_END

ROM_START( island2_4a ) // 070205 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_is2_070205a.rom", 0x00000, 0x40000, CRC(9bafe560) SHA1(c7238f822d26645207953798c87e67127de1b443) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "island2.001", 0x000000, 0x80000, CRC(f8dd9fe9) SHA1(0cf67fbca107b255011fded6390507d12cbac514) )
	ROM_LOAD( "island2.002", 0x100000, 0x80000, CRC(4f9607c0) SHA1(cd3e7b4a88f46231a115c9d18a26b5e30fea74e4) )
	ROM_LOAD( "island2.003", 0x200000, 0x80000, CRC(bceccdba) SHA1(5cf3b51ccfe317ca57d770bff0204b0ee83d1173) )
	ROM_LOAD( "island2.004", 0x300000, 0x80000, CRC(15fdecc7) SHA1(6882ea10f117c85544df51d9abd67ef52db91d95) )
	ROM_LOAD( "island2.005", 0x080000, 0x80000, CRC(2d4905aa) SHA1(c4a1e4db61e8af6cf0fb70aabe5e3896ab5227ca) )
	ROM_LOAD( "island2.006", 0x180000, 0x80000, CRC(55e285d9) SHA1(ba58963441c65220700cd8057e6afe3f5f8faa4f) )
	ROM_LOAD( "island2.007", 0x280000, 0x80000, CRC(edd72be6) SHA1(fb1e63f59e8565c23ae43630fa572fbc022c878f) )
	ROM_LOAD( "island2.008", 0x380000, 0x80000, CRC(c336d608) SHA1(55391183c6d95ecea81354efa70641350860d1f5) )
ROM_END



ROM_START( pirate2a ) // 061005 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_pr2_m_061005a.rom", 0x00000, 0x40000, CRC(2271eb23) SHA1(6a3e01b59df08a23e5f53ee9b9e473b9f5f5e3b6) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "pirate2.001", 0x000000, 0x80000, CRC(106e7cba) SHA1(289a3ae38b895c83600c920bee0c2dd46e941eac) )
	ROM_LOAD( "pirate2.002", 0x100000, 0x80000, CRC(076a290f) SHA1(2f9bb74e081262e535c8ed9a31589d6a919f5d15) )
	ROM_LOAD( "pirate2.003", 0x200000, 0x80000, CRC(13a91fe7) SHA1(6e127b3827a9271ad19986714747be9367125f62) )
	ROM_LOAD( "pirate2.004", 0x300000, 0x80000, CRC(5ac8c531) SHA1(1da91b9a71a9a8681577342660bfa85e5bbc99bc) )
	ROM_LOAD( "pirate2.005", 0x080000, 0x80000, CRC(98012c74) SHA1(2a5b466353eef3a5cfc9f98eceb7523b00d0204a) )
	ROM_LOAD( "pirate2.006", 0x180000, 0x80000, CRC(366e1465) SHA1(440230d5306c4b424f27839b7fb9c8a5bb922dcc) )
	ROM_LOAD( "pirate2.007", 0x280000, 0x80000, CRC(21fb963e) SHA1(e3f7fb13f326699e34aebcc3ee07016f7cfe6e46) )
	ROM_LOAD( "pirate2.008", 0x380000, 0x80000, CRC(40c59448) SHA1(774af0f376864ec5948904df338bc7493eaed392) )
ROM_END

ROM_START( pirate2b ) // 061005 bank F9, skip raster beam position check
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_pr2_m_061005b.rom", 0x00000, 0x40000, CRC(379f890e) SHA1(0fd05c1bf08fd3e6694f18ffe2ea9741cf465617) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "pirate2.001", 0x000000, 0x80000, CRC(106e7cba) SHA1(289a3ae38b895c83600c920bee0c2dd46e941eac) )
	ROM_LOAD( "pirate2.002", 0x100000, 0x80000, CRC(076a290f) SHA1(2f9bb74e081262e535c8ed9a31589d6a919f5d15) )
	ROM_LOAD( "pirate2.003", 0x200000, 0x80000, CRC(13a91fe7) SHA1(6e127b3827a9271ad19986714747be9367125f62) )
	ROM_LOAD( "pirate2.004", 0x300000, 0x80000, CRC(5ac8c531) SHA1(1da91b9a71a9a8681577342660bfa85e5bbc99bc) )
	ROM_LOAD( "pirate2.005", 0x080000, 0x80000, CRC(98012c74) SHA1(2a5b466353eef3a5cfc9f98eceb7523b00d0204a) )
	ROM_LOAD( "pirate2.006", 0x180000, 0x80000, CRC(366e1465) SHA1(440230d5306c4b424f27839b7fb9c8a5bb922dcc) )
	ROM_LOAD( "pirate2.007", 0x280000, 0x80000, CRC(21fb963e) SHA1(e3f7fb13f326699e34aebcc3ee07016f7cfe6e46) )
	ROM_LOAD( "pirate2.008", 0x380000, 0x80000, CRC(40c59448) SHA1(774af0f376864ec5948904df338bc7493eaed392) )
ROM_END

ROM_START( pirate2c ) // 061005 bank F9, changed version text to 070126
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_pr2_m_061005c.rom", 0x00000, 0x40000, CRC(7c4339df) SHA1(80e2f97629bc963f044c1697c65777a3fd038edc) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "pirate2.001", 0x000000, 0x80000, CRC(106e7cba) SHA1(289a3ae38b895c83600c920bee0c2dd46e941eac) )
	ROM_LOAD( "pirate2.002", 0x100000, 0x80000, CRC(076a290f) SHA1(2f9bb74e081262e535c8ed9a31589d6a919f5d15) )
	ROM_LOAD( "pirate2.003", 0x200000, 0x80000, CRC(13a91fe7) SHA1(6e127b3827a9271ad19986714747be9367125f62) )
	ROM_LOAD( "pirate2.004", 0x300000, 0x80000, CRC(5ac8c531) SHA1(1da91b9a71a9a8681577342660bfa85e5bbc99bc) )
	ROM_LOAD( "pirate2.005", 0x080000, 0x80000, CRC(98012c74) SHA1(2a5b466353eef3a5cfc9f98eceb7523b00d0204a) )
	ROM_LOAD( "pirate2.006", 0x180000, 0x80000, CRC(366e1465) SHA1(440230d5306c4b424f27839b7fb9c8a5bb922dcc) )
	ROM_LOAD( "pirate2.007", 0x280000, 0x80000, CRC(21fb963e) SHA1(e3f7fb13f326699e34aebcc3ee07016f7cfe6e46) )
	ROM_LOAD( "pirate2.008", 0x380000, 0x80000, CRC(40c59448) SHA1(774af0f376864ec5948904df338bc7493eaed392) )
ROM_END

ROM_START( pirate2d ) // 061005 bank F9, changed version text to 070126
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_pr2_m_061005d.rom", 0x00000, 0x40000, CRC(16620bb2) SHA1(bf3a26b06595e51fa37f2f8099a12e906a6ca439) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "pirate2.001", 0x000000, 0x80000, CRC(106e7cba) SHA1(289a3ae38b895c83600c920bee0c2dd46e941eac) )
	ROM_LOAD( "pirate2.002", 0x100000, 0x80000, CRC(076a290f) SHA1(2f9bb74e081262e535c8ed9a31589d6a919f5d15) )
	ROM_LOAD( "pirate2.003", 0x200000, 0x80000, CRC(13a91fe7) SHA1(6e127b3827a9271ad19986714747be9367125f62) )
	ROM_LOAD( "pirate2.004", 0x300000, 0x80000, CRC(5ac8c531) SHA1(1da91b9a71a9a8681577342660bfa85e5bbc99bc) )
	ROM_LOAD( "pirate2.005", 0x080000, 0x80000, CRC(98012c74) SHA1(2a5b466353eef3a5cfc9f98eceb7523b00d0204a) )
	ROM_LOAD( "pirate2.006", 0x180000, 0x80000, CRC(366e1465) SHA1(440230d5306c4b424f27839b7fb9c8a5bb922dcc) )
	ROM_LOAD( "pirate2.007", 0x280000, 0x80000, CRC(21fb963e) SHA1(e3f7fb13f326699e34aebcc3ee07016f7cfe6e46) )
	ROM_LOAD( "pirate2.008", 0x380000, 0x80000, CRC(40c59448) SHA1(774af0f376864ec5948904df338bc7493eaed392) )
ROM_END

ROM_START( pirate2e ) // 061005 bank F9, changed version text to 070126, skip some start tests
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_pr2_m_061005e.rom", 0x00000, 0x40000, CRC(f36edf02) SHA1(25ff61eb80b54298037b4a90d7fc93e8530e0815) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "pirate2.001", 0x000000, 0x80000, CRC(106e7cba) SHA1(289a3ae38b895c83600c920bee0c2dd46e941eac) )
	ROM_LOAD( "pirate2.002", 0x100000, 0x80000, CRC(076a290f) SHA1(2f9bb74e081262e535c8ed9a31589d6a919f5d15) )
	ROM_LOAD( "pirate2.003", 0x200000, 0x80000, CRC(13a91fe7) SHA1(6e127b3827a9271ad19986714747be9367125f62) )
	ROM_LOAD( "pirate2.004", 0x300000, 0x80000, CRC(5ac8c531) SHA1(1da91b9a71a9a8681577342660bfa85e5bbc99bc) )
	ROM_LOAD( "pirate2.005", 0x080000, 0x80000, CRC(98012c74) SHA1(2a5b466353eef3a5cfc9f98eceb7523b00d0204a) )
	ROM_LOAD( "pirate2.006", 0x180000, 0x80000, CRC(366e1465) SHA1(440230d5306c4b424f27839b7fb9c8a5bb922dcc) )
	ROM_LOAD( "pirate2.007", 0x280000, 0x80000, CRC(21fb963e) SHA1(e3f7fb13f326699e34aebcc3ee07016f7cfe6e46) )
	ROM_LOAD( "pirate2.008", 0x380000, 0x80000, CRC(40c59448) SHA1(774af0f376864ec5948904df338bc7493eaed392) )
ROM_END

ROM_START( pirate2f ) // 061005 custom alteras, modified graphics, many texts changed, changed version text to "VIDEO GAME-1 PR01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_pr2_m_061005f.rom", 0x00000, 0x40000, CRC(99960a9f) SHA1(0facd5d1f297d4433d6abefc61cfa2867826e90c) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1a", 0x000000, 0x80000, CRC(011522da) SHA1(c1ce09208b3144cbc38d4afa922a049043a93df6) )
	ROM_LOAD( "bootleg_2a", 0x100000, 0x80000, CRC(8f8a0ea3) SHA1(5b34b43d6aa843550a7aaff3306a4b68722e9966) )
	ROM_LOAD( "bootleg_3a", 0x200000, 0x80000, CRC(7bbfcda9) SHA1(7ebb1ac0b42f4aabcc4be65c44d01b51fd070ab4) )
	ROM_LOAD( "bootleg_4a", 0x300000, 0x80000, CRC(88b8b15d) SHA1(0f0f1c769eb1475e1f74f56a587efce13c910a4c) )
	ROM_LOAD( "bootleg_5a", 0x080000, 0x80000, CRC(85441649) SHA1(171e58eac7fdd5f8f9d0eb31535a824b19ea7342) )
	ROM_LOAD( "bootleg_6a", 0x180000, 0x80000, CRC(913d55e1) SHA1(030eb39a4e34e7c5791059ac2affcded914096ae) )
	ROM_LOAD( "bootleg_7a", 0x280000, 0x80000, CRC(02fb1604) SHA1(7fd9c6f256f22e437cdd7938b353a4ba9ceb006a) )
	ROM_LOAD( "bootleg_8a", 0x380000, 0x80000, CRC(48643b99) SHA1(1b0399df326835a6b442137d233cc741cf7863de) )
ROM_END

ROM_START( pirate2g ) // 061005 bank F9, modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_pr2_m_061005g.rom", 0x00000, 0x40000, CRC(60170053) SHA1(4aca8fd8466199467a5c0020cdbb4379ae38cee4) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1b", 0x000000, 0x80000, CRC(76d3bc6f) SHA1(a748f5d1219fe3f332cebbf1dfb8014a15800135) )
	ROM_LOAD( "bootleg_2b", 0x100000, 0x80000, CRC(e65b7e0a) SHA1(0ce76961b92f7153f3886965d909c522d32313dd) )
	ROM_LOAD( "bootleg_3b", 0x200000, 0x80000, CRC(5d8270ac) SHA1(883435b08cea05dc1531e7871a6b3046e8365b55) )
	ROM_LOAD( "bootleg_4b", 0x300000, 0x80000, CRC(ac0f1bb5) SHA1(d391b5be974a94aecba7fd55a4e2017b5b60ad06) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(48019c90) SHA1(f00effeb84d2c0f46faa40ea519b1d41efbc8e38) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(19155fa8) SHA1(81458b32432caa3effba778971840e330be33585) )
	ROM_LOAD( "bootleg_7b", 0x280000, 0x80000, CRC(7fb69540) SHA1(595ede785962c8af84eee7ff47c4b3bc91c5537a) )
	ROM_LOAD( "bootleg_8b", 0x380000, 0x80000, CRC(f8a97474) SHA1(b8c5be0f7bc3f4a08823a1bf8e2026afe11d052c) )
ROM_END

ROM_START( pirate2h ) // 061005 custom alteras, modified graphics, many texts changed, changed version text to "LOTOS PR01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_pr2_m_061005h.rom", 0x00000, 0x40000, CRC(6dadb485) SHA1(9b61ad0f117dc84af8b7b3523275e5cb48c521e5) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1d", 0x000000, 0x80000, CRC(d830ed6d) SHA1(4cdde127912d250201665534e6371dcfc500dc10) )
	ROM_LOAD( "bootleg_2d", 0x100000, 0x80000, CRC(b54df2b2) SHA1(1a08c4b345423741067a095fb21a6a91abb87dbc) )
	ROM_LOAD( "bootleg_3d", 0x200000, 0x80000, CRC(b87811cf) SHA1(3e9d8d06d7ac5ae5404d1002e4a2123c4f2c0328) )
	ROM_LOAD( "bootleg_4d", 0x300000, 0x80000, CRC(5aa8e647) SHA1(fa5ae55e1fd81068369f2bf9fc711fc78fa3fa24) )
	ROM_LOAD( "bootleg_5a", 0x080000, 0x80000, CRC(85441649) SHA1(171e58eac7fdd5f8f9d0eb31535a824b19ea7342) )
	ROM_LOAD( "bootleg_6a", 0x180000, 0x80000, CRC(913d55e1) SHA1(030eb39a4e34e7c5791059ac2affcded914096ae) )
	ROM_LOAD( "bootleg_7a", 0x280000, 0x80000, CRC(02fb1604) SHA1(7fd9c6f256f22e437cdd7938b353a4ba9ceb006a) )
	ROM_LOAD( "bootleg_8a", 0x380000, 0x80000, CRC(48643b99) SHA1(1b0399df326835a6b442137d233cc741cf7863de) )
ROM_END

ROM_START( pirate2_2a ) // 070126 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_pr2_070126a.rom", 0x00000, 0x40000, CRC(7131239b) SHA1(8210defd0987951a0dfe5b3ca6fcee580257475e) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "pirate2.001", 0x000000, 0x80000, CRC(106e7cba) SHA1(289a3ae38b895c83600c920bee0c2dd46e941eac) )
	ROM_LOAD( "pirate2.002", 0x100000, 0x80000, CRC(076a290f) SHA1(2f9bb74e081262e535c8ed9a31589d6a919f5d15) )
	ROM_LOAD( "pirate2.003", 0x200000, 0x80000, CRC(13a91fe7) SHA1(6e127b3827a9271ad19986714747be9367125f62) )
	ROM_LOAD( "pirate2.004", 0x300000, 0x80000, CRC(5ac8c531) SHA1(1da91b9a71a9a8681577342660bfa85e5bbc99bc) )
	ROM_LOAD( "pirate2.005", 0x080000, 0x80000, CRC(98012c74) SHA1(2a5b466353eef3a5cfc9f98eceb7523b00d0204a) )
	ROM_LOAD( "pirate2.006", 0x180000, 0x80000, CRC(366e1465) SHA1(440230d5306c4b424f27839b7fb9c8a5bb922dcc) )
	ROM_LOAD( "pirate2.007", 0x280000, 0x80000, CRC(21fb963e) SHA1(e3f7fb13f326699e34aebcc3ee07016f7cfe6e46) )
	ROM_LOAD( "pirate2.008", 0x380000, 0x80000, CRC(40c59448) SHA1(774af0f376864ec5948904df338bc7493eaed392) )
ROM_END



ROM_START( keksa ) // 060328 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_ks_m_060328a.rom", 0x00000, 0x40000, CRC(7b387386) SHA1(d6bfc3b0d1f74723902d96dbcb69865cb5274cd0) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "keks_m.001", 0x000000, 0x80000, CRC(f4c20f66) SHA1(bed42ef01dfaa9d5d6ebb703e44ce7c11b8a373c) )
	ROM_LOAD( "keks_m.002", 0x100000, 0x80000, CRC(b7ec3fac) SHA1(c3c62690487a6056415c46888bde8254efca836f) )
	ROM_LOAD( "keks_m.003", 0x200000, 0x80000, CRC(5b6e8568) SHA1(003297e9cd080d91fe6751286eabd3a2f37ceb76) )
	ROM_LOAD( "keks_m.004", 0x300000, 0x80000, CRC(9dc32736) SHA1(7b2091ae802431d1c959b859a58e0076d32abef0) )
	ROM_LOAD( "keks_m.005", 0x080000, 0x80000, CRC(c5b09267) SHA1(7fd0988e63752fdbb31fde60b4726cfd63149622) )
	ROM_LOAD( "keks_m.006", 0x180000, 0x80000, CRC(583da5fd) SHA1(645228db20cdaacb53bfc68731fd1a66a6a8cf56) )
	ROM_LOAD( "keks_m.007", 0x280000, 0x80000, CRC(311c166a) SHA1(5f0ad8d755a6141964d818b98b3f156cbda8fb0d) )
	ROM_LOAD( "keks_m.008", 0x380000, 0x80000, CRC(f69b0831) SHA1(75392349ef02a39cf883206938e2c615445065fc) )
ROM_END

ROM_START( keksb ) // 060328 backdoor 1,1 1,3 1,5 1,7  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_ks_m_060328b.rom", 0x00000, 0x40000, CRC(661c7ee9) SHA1(f07902c4a3ba5fce5bc7fe666d90deb852e40b4c) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "keks_m.001", 0x000000, 0x80000, CRC(f4c20f66) SHA1(bed42ef01dfaa9d5d6ebb703e44ce7c11b8a373c) )
	ROM_LOAD( "keks_m.002", 0x100000, 0x80000, CRC(b7ec3fac) SHA1(c3c62690487a6056415c46888bde8254efca836f) )
	ROM_LOAD( "keks_m.003", 0x200000, 0x80000, CRC(5b6e8568) SHA1(003297e9cd080d91fe6751286eabd3a2f37ceb76) )
	ROM_LOAD( "keks_m.004", 0x300000, 0x80000, CRC(9dc32736) SHA1(7b2091ae802431d1c959b859a58e0076d32abef0) )
	ROM_LOAD( "keks_m.005", 0x080000, 0x80000, CRC(c5b09267) SHA1(7fd0988e63752fdbb31fde60b4726cfd63149622) )
	ROM_LOAD( "keks_m.006", 0x180000, 0x80000, CRC(583da5fd) SHA1(645228db20cdaacb53bfc68731fd1a66a6a8cf56) )
	ROM_LOAD( "keks_m.007", 0x280000, 0x80000, CRC(311c166a) SHA1(5f0ad8d755a6141964d818b98b3f156cbda8fb0d) )
	ROM_LOAD( "keks_m.008", 0x380000, 0x80000, CRC(f69b0831) SHA1(75392349ef02a39cf883206938e2c615445065fc) )
ROM_END

ROM_START( keksc ) // 060328 bank F9, changed version text to 070119
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_ks_m_060328c.rom", 0x00000, 0x40000, CRC(fc70d26e) SHA1(0430ad20b797412967c2e82779eb7f0e9170f77c) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "keks_m.001", 0x000000, 0x80000, CRC(f4c20f66) SHA1(bed42ef01dfaa9d5d6ebb703e44ce7c11b8a373c) )
	ROM_LOAD( "keks_m.002", 0x100000, 0x80000, CRC(b7ec3fac) SHA1(c3c62690487a6056415c46888bde8254efca836f) )
	ROM_LOAD( "keks_m.003", 0x200000, 0x80000, CRC(5b6e8568) SHA1(003297e9cd080d91fe6751286eabd3a2f37ceb76) )
	ROM_LOAD( "keks_m.004", 0x300000, 0x80000, CRC(9dc32736) SHA1(7b2091ae802431d1c959b859a58e0076d32abef0) )
	ROM_LOAD( "keks_m.005", 0x080000, 0x80000, CRC(c5b09267) SHA1(7fd0988e63752fdbb31fde60b4726cfd63149622) )
	ROM_LOAD( "keks_m.006", 0x180000, 0x80000, CRC(583da5fd) SHA1(645228db20cdaacb53bfc68731fd1a66a6a8cf56) )
	ROM_LOAD( "keks_m.007", 0x280000, 0x80000, CRC(311c166a) SHA1(5f0ad8d755a6141964d818b98b3f156cbda8fb0d) )
	ROM_LOAD( "keks_m.008", 0x380000, 0x80000, CRC(f69b0831) SHA1(75392349ef02a39cf883206938e2c615445065fc) )
ROM_END

ROM_START( keks_2a ) // 060403 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_ks_m_060403a.rom", 0x00000, 0x40000, CRC(bd749f63) SHA1(dc3ba624b186370896d3ecf5968a82a17aa019d0) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "keks_m.001", 0x000000, 0x80000, CRC(f4c20f66) SHA1(bed42ef01dfaa9d5d6ebb703e44ce7c11b8a373c) )
	ROM_LOAD( "keks_m.002", 0x100000, 0x80000, CRC(b7ec3fac) SHA1(c3c62690487a6056415c46888bde8254efca836f) )
	ROM_LOAD( "keks_m.003", 0x200000, 0x80000, CRC(5b6e8568) SHA1(003297e9cd080d91fe6751286eabd3a2f37ceb76) )
	ROM_LOAD( "keks_m.004", 0x300000, 0x80000, CRC(9dc32736) SHA1(7b2091ae802431d1c959b859a58e0076d32abef0) )
	ROM_LOAD( "keks_m.005", 0x080000, 0x80000, CRC(c5b09267) SHA1(7fd0988e63752fdbb31fde60b4726cfd63149622) )
	ROM_LOAD( "keks_m.006", 0x180000, 0x80000, CRC(583da5fd) SHA1(645228db20cdaacb53bfc68731fd1a66a6a8cf56) )
	ROM_LOAD( "keks_m.007", 0x280000, 0x80000, CRC(311c166a) SHA1(5f0ad8d755a6141964d818b98b3f156cbda8fb0d) )
	ROM_LOAD( "keks_m.008", 0x380000, 0x80000, CRC(f69b0831) SHA1(75392349ef02a39cf883206938e2c615445065fc) )
ROM_END

ROM_START( keks_2b ) // 060403 bank F9, changed version text to 070119
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_ks_m_060403b.rom", 0x00000, 0x40000, CRC(d089de90) SHA1(9208f411cd97fef89b52019f9655e7d006f91303) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "keks_m.001", 0x000000, 0x80000, CRC(f4c20f66) SHA1(bed42ef01dfaa9d5d6ebb703e44ce7c11b8a373c) )
	ROM_LOAD( "keks_m.002", 0x100000, 0x80000, CRC(b7ec3fac) SHA1(c3c62690487a6056415c46888bde8254efca836f) )
	ROM_LOAD( "keks_m.003", 0x200000, 0x80000, CRC(5b6e8568) SHA1(003297e9cd080d91fe6751286eabd3a2f37ceb76) )
	ROM_LOAD( "keks_m.004", 0x300000, 0x80000, CRC(9dc32736) SHA1(7b2091ae802431d1c959b859a58e0076d32abef0) )
	ROM_LOAD( "keks_m.005", 0x080000, 0x80000, CRC(c5b09267) SHA1(7fd0988e63752fdbb31fde60b4726cfd63149622) )
	ROM_LOAD( "keks_m.006", 0x180000, 0x80000, CRC(583da5fd) SHA1(645228db20cdaacb53bfc68731fd1a66a6a8cf56) )
	ROM_LOAD( "keks_m.007", 0x280000, 0x80000, CRC(311c166a) SHA1(5f0ad8d755a6141964d818b98b3f156cbda8fb0d) )
	ROM_LOAD( "keks_m.008", 0x380000, 0x80000, CRC(f69b0831) SHA1(75392349ef02a39cf883206938e2c615445065fc) )
ROM_END

ROM_START( keks_2c ) // 060403 custom alteras, modified graphics, bank F9, changed version text to "VIDEO GAME-1 KS01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_ks_m_060403c.rom", 0x00000, 0x40000, CRC(ba7f429a) SHA1(6ad7a345b292f2081e49081374c74afc7430bd14) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1b", 0x000000, 0x80000, CRC(2844b89e) SHA1(75b59e6272ef42438a3734e45f131f449ed0aa46) )
	ROM_LOAD( "bootleg_2b", 0x100000, 0x80000, CRC(ae8f47ad) SHA1(16504436c9291ffea7915547e5ad9dfbd53ebd5d) )
	ROM_LOAD( "bootleg_3b", 0x200000, 0x80000, CRC(57ccb876) SHA1(e1f41cf09ac2a535b5dbc05364268b3bd913e8f5) )
	ROM_LOAD( "bootleg_4b", 0x300000, 0x80000, CRC(05344f35) SHA1(ffc60a7581db82878fe625d590c01c7afdf8260c) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(45f9b0bd) SHA1(4b0f59461182753fd47f7fcc6a126faf53dd26b1) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(5ffbfa73) SHA1(0036bdba0ed6f907b8925b55adc33e96913297a3) )
	ROM_LOAD( "bootleg_7b", 0x280000, 0x80000, CRC(ce3f6152) SHA1(6e142e0d448d7a2328b6f6bf014b8e6de6f18407) )
	ROM_LOAD( "bootleg_8b", 0x380000, 0x80000, CRC(6e2e6a3d) SHA1(d4efad82b580c7cd53fca705a24aef51edede8ed) )
ROM_END

ROM_START( keks_2d ) // 060403 modified graphics, bank F9, changed version text to "VIDEO GAME-1 KS01" (keks_2c, decoded gfx)
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_ks_m_060403c.rom", 0x00000, 0x40000, CRC(ba7f429a) SHA1(6ad7a345b292f2081e49081374c74afc7430bd14) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1c", 0x000000, 0x80000, CRC(04ad2c2f) SHA1(ea0819dad0971629056ba0e6cb599415f1d34ad6) )
	ROM_LOAD( "bootleg_2c", 0x100000, 0x80000, CRC(8d0b747d) SHA1(6c40f3abe8bd20d6405829879fbe4718d85d4ba0) )
	ROM_LOAD( "bootleg_3c", 0x200000, 0x80000, CRC(10a43238) SHA1(9404eefad2c62730a771b2521e893bbc78c4ca71) )
	ROM_LOAD( "bootleg_4c", 0x300000, 0x80000, CRC(878b2553) SHA1(1b31c1b67496099c0b955865b63881e4a558c320) )
	ROM_LOAD( "keks_m.005", 0x080000, 0x80000, CRC(c5b09267) SHA1(7fd0988e63752fdbb31fde60b4726cfd63149622) )
	ROM_LOAD( "keks_m.006", 0x180000, 0x80000, CRC(583da5fd) SHA1(645228db20cdaacb53bfc68731fd1a66a6a8cf56) )
	ROM_LOAD( "keks_m.007", 0x280000, 0x80000, CRC(311c166a) SHA1(5f0ad8d755a6141964d818b98b3f156cbda8fb0d) )
	ROM_LOAD( "keks_m.008", 0x380000, 0x80000, CRC(f69b0831) SHA1(75392349ef02a39cf883206938e2c615445065fc) )
ROM_END

ROM_START( keks_2e ) // 060403 bank F9, payout percentage 60%
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_ks_m_060403d.rom", 0x00000, 0x40000, CRC(358e5b48) SHA1(6a91d9ee515401ebe7a8dd7d95f43c4df1c13677) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "keks_m.001", 0x000000, 0x80000, CRC(f4c20f66) SHA1(bed42ef01dfaa9d5d6ebb703e44ce7c11b8a373c) )
	ROM_LOAD( "keks_m.002", 0x100000, 0x80000, CRC(b7ec3fac) SHA1(c3c62690487a6056415c46888bde8254efca836f) )
	ROM_LOAD( "keks_m.003", 0x200000, 0x80000, CRC(5b6e8568) SHA1(003297e9cd080d91fe6751286eabd3a2f37ceb76) )
	ROM_LOAD( "keks_m.004", 0x300000, 0x80000, CRC(9dc32736) SHA1(7b2091ae802431d1c959b859a58e0076d32abef0) )
	ROM_LOAD( "keks_m.005", 0x080000, 0x80000, CRC(c5b09267) SHA1(7fd0988e63752fdbb31fde60b4726cfd63149622) )
	ROM_LOAD( "keks_m.006", 0x180000, 0x80000, CRC(583da5fd) SHA1(645228db20cdaacb53bfc68731fd1a66a6a8cf56) )
	ROM_LOAD( "keks_m.007", 0x280000, 0x80000, CRC(311c166a) SHA1(5f0ad8d755a6141964d818b98b3f156cbda8fb0d) )
	ROM_LOAD( "keks_m.008", 0x380000, 0x80000, CRC(f69b0831) SHA1(75392349ef02a39cf883206938e2c615445065fc) )
ROM_END

ROM_START( keks_2f ) // 060403 bank F9, modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_ks_m_060403e.rom", 0x00000, 0x40000, CRC(80882337) SHA1(af942e27247b0d492799013a623080c02e7213cb) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1d", 0x000000, 0x80000, CRC(82c5f8e9) SHA1(0fbc1b314e15f93c62c3cd79ece453086c506878) )
	ROM_LOAD( "bootleg_2d", 0x100000, 0x80000, CRC(a174f52b) SHA1(45c225762f5885e33eb6848aeca1ae9e2ad918ed) )
	ROM_LOAD( "bootleg_3d", 0x200000, 0x80000, CRC(9486084d) SHA1(780cd6d7b85e9480c7c2a3f707228d37678fb684) )
	ROM_LOAD( "bootleg_4d", 0x300000, 0x80000, CRC(aa552b3c) SHA1(7b051b2cece5a5e84020466827576ca8272f4f96) )
	ROM_LOAD( "bootleg_5d", 0x080000, 0x80000, CRC(53fa2420) SHA1(2faf775c5bc1ff8683d4a70b236c77d6295167e1) )
	ROM_LOAD( "bootleg_6d", 0x180000, 0x80000, CRC(baa7cce5) SHA1(1aacc85498f026358085caa854c2caaad3de5abf) )
	ROM_LOAD( "keks_m.007", 0x280000, 0x80000, CRC(311c166a) SHA1(5f0ad8d755a6141964d818b98b3f156cbda8fb0d) )
	ROM_LOAD( "keks_m.008", 0x380000, 0x80000, CRC(f69b0831) SHA1(75392349ef02a39cf883206938e2c615445065fc) )
ROM_END

ROM_START( keks_2g ) // 060403 custom alteras, modified graphics, bank F9, many texts changed, changed version text to "LOTOS KS01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_ks_m_060403f.rom", 0x00000, 0x40000, CRC(9fd765ab) SHA1(ff061fe3d5d547c25d4ca830ec1f94025ab4e7f5) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1f", 0x000000, 0x80000, CRC(6c9bf198) SHA1(40155c3574f9968e272c6f1af90d663d2a686511) )
	ROM_LOAD( "bootleg_2f", 0x100000, 0x80000, CRC(daf71ad6) SHA1(2107d5eb617ff844eaeaabe3744fb8fad403390c) )
	ROM_LOAD( "bootleg_3f", 0x200000, 0x80000, CRC(ef9dfc94) SHA1(9b6d97dd6dc20caa11da4ced956ab5474e92e6d9) )
	ROM_LOAD( "bootleg_4f", 0x300000, 0x80000, CRC(6e88ac0e) SHA1(6dbc64794bb5afce3ab5e1dfe272b933bf66722d) )
	ROM_LOAD( "bootleg_5f", 0x080000, 0x80000, CRC(a79f6d46) SHA1(345bdf089529d90da861426fffc72bd2efb062f3) )
	ROM_LOAD( "bootleg_6f", 0x180000, 0x80000, CRC(7ecd7ed4) SHA1(e6320741f0def3915c91307cb4e3c277b347dee6) )
	ROM_LOAD( "bootleg_7f", 0x280000, 0x80000, CRC(e952996a) SHA1(3382158ced985c46fad56fb74766a4ed0e35b6c6) )
	ROM_LOAD( "bootleg_8f", 0x380000, 0x80000, CRC(4fefa4eb) SHA1(6a2b633e278a9b2a05dde2c6c37f15fc6b3fa7dc) )
ROM_END

ROM_START( keks_3a ) // 070119 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_ks_070119a.rom", 0x00000, 0x40000, CRC(9325fea5) SHA1(1d396e63d51a81eae75df1ae2646cacac7f49f66) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "keks.001", 0x000000, 0x80000, CRC(fc399595) SHA1(037afd4a613cd58d4a28627b9e395d48c3fa866a) )
	ROM_LOAD( "keks.002", 0x100000, 0x80000, CRC(474b36e7) SHA1(e1e62acd4a706b2654fc1249850806b612fc1419) )
	ROM_LOAD( "keks.003", 0x200000, 0x80000, CRC(7f885e3d) SHA1(09bb4690e86ed4a29eef75ee4e5753ce40a164dd) )
	ROM_LOAD( "keks.004", 0x300000, 0x80000, CRC(a0fc654b) SHA1(3354bdb7aa372816a766b0d36408543de7d3482f) )
	ROM_LOAD( "keks_m.005", 0x080000, 0x80000, CRC(c5b09267) SHA1(7fd0988e63752fdbb31fde60b4726cfd63149622) )
	ROM_LOAD( "keks_m.006", 0x180000, 0x80000, CRC(583da5fd) SHA1(645228db20cdaacb53bfc68731fd1a66a6a8cf56) )
	ROM_LOAD( "keks_m.007", 0x280000, 0x80000, CRC(311c166a) SHA1(5f0ad8d755a6141964d818b98b3f156cbda8fb0d) )
	ROM_LOAD( "keks_m.008", 0x380000, 0x80000, CRC(f69b0831) SHA1(75392349ef02a39cf883206938e2c615445065fc) )
ROM_END

ROM_START( keks_3b ) // 070119 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_ks_070119b.rom", 0x00000, 0x40000, CRC(13fcf022) SHA1(885e5097e5cb5805f2172adc41f2f20d109b2d10) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "keks.001", 0x000000, 0x80000, CRC(fc399595) SHA1(037afd4a613cd58d4a28627b9e395d48c3fa866a) )
	ROM_LOAD( "keks.002", 0x100000, 0x80000, CRC(474b36e7) SHA1(e1e62acd4a706b2654fc1249850806b612fc1419) )
	ROM_LOAD( "keks.003", 0x200000, 0x80000, CRC(7f885e3d) SHA1(09bb4690e86ed4a29eef75ee4e5753ce40a164dd) )
	ROM_LOAD( "keks.004", 0x300000, 0x80000, CRC(a0fc654b) SHA1(3354bdb7aa372816a766b0d36408543de7d3482f) )
	ROM_LOAD( "keks_m.005", 0x080000, 0x80000, CRC(c5b09267) SHA1(7fd0988e63752fdbb31fde60b4726cfd63149622) )
	ROM_LOAD( "keks_m.006", 0x180000, 0x80000, CRC(583da5fd) SHA1(645228db20cdaacb53bfc68731fd1a66a6a8cf56) )
	ROM_LOAD( "keks_m.007", 0x280000, 0x80000, CRC(311c166a) SHA1(5f0ad8d755a6141964d818b98b3f156cbda8fb0d) )
	ROM_LOAD( "keks_m.008", 0x380000, 0x80000, CRC(f69b0831) SHA1(75392349ef02a39cf883206938e2c615445065fc) )
ROM_END



ROM_START( gnomea ) // 070906 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gn_070906a.rom", 0x00000, 0x40000, CRC(5e7f1d53) SHA1(0548db91e8b1cd2f4f292095b4aad3b59cc25fb8) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "gnome.001", 0x000000, 0x80000, CRC(6ed866d7) SHA1(68d75d24d98e6d533cb26ceac0a680203cb26069) )
	ROM_LOAD( "gnome.002", 0x100000, 0x80000, CRC(f6e5e6f0) SHA1(9751e8df87f14a252595547d24b8dd865ee4f08d) )
	ROM_LOAD( "gnome.003", 0x200000, 0x80000, CRC(f8beb972) SHA1(3afbca8ce7e69d2dadae05f69205a6fd9036cf6a) )
	ROM_LOAD( "gnome.004", 0x300000, 0x80000, CRC(83357c38) SHA1(45cd31c4f02f9d7b1888701c2146d1e7229b6cb5) )
	ROM_LOAD( "gnome.005", 0x080000, 0x80000, CRC(687ad3e3) SHA1(23941a4f40c45029b9a43451f78b04c03c3cd7da) )
	ROM_LOAD( "gnome.006", 0x180000, 0x80000, CRC(7ef2b88a) SHA1(7e7de60fc6791731d7cfd6a50e2bc5af1bf5e4b2) )
	ROM_LOAD( "gnome.007", 0x280000, 0x80000, CRC(71976bdf) SHA1(c44dbfa75a0f12893b3177907fc93b3d5e8ad390) )
	ROM_LOAD( "gnome.008", 0x380000, 0x80000, CRC(c86a1586) SHA1(e622bca8dc618ca8edc1a7daa9c8286383caebef) )
ROM_END

ROM_START( gnomeb ) // 070906 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gn_070906b.rom", 0x00000, 0x40000, CRC(0baeb2f7) SHA1(799576a1cf49681d81967678643c17dc0e09be78) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "gnome.001", 0x000000, 0x80000, CRC(6ed866d7) SHA1(68d75d24d98e6d533cb26ceac0a680203cb26069) )
	ROM_LOAD( "gnome.002", 0x100000, 0x80000, CRC(f6e5e6f0) SHA1(9751e8df87f14a252595547d24b8dd865ee4f08d) )
	ROM_LOAD( "gnome.003", 0x200000, 0x80000, CRC(f8beb972) SHA1(3afbca8ce7e69d2dadae05f69205a6fd9036cf6a) )
	ROM_LOAD( "gnome.004", 0x300000, 0x80000, CRC(83357c38) SHA1(45cd31c4f02f9d7b1888701c2146d1e7229b6cb5) )
	ROM_LOAD( "gnome.005", 0x080000, 0x80000, CRC(687ad3e3) SHA1(23941a4f40c45029b9a43451f78b04c03c3cd7da) )
	ROM_LOAD( "gnome.006", 0x180000, 0x80000, CRC(7ef2b88a) SHA1(7e7de60fc6791731d7cfd6a50e2bc5af1bf5e4b2) )
	ROM_LOAD( "gnome.007", 0x280000, 0x80000, CRC(71976bdf) SHA1(c44dbfa75a0f12893b3177907fc93b3d5e8ad390) )
	ROM_LOAD( "gnome.008", 0x380000, 0x80000, CRC(c86a1586) SHA1(e622bca8dc618ca8edc1a7daa9c8286383caebef) )
ROM_END

ROM_START( gnomec ) // 070906 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gn_070906c.rom", 0x00000, 0x40000, CRC(37b4a5e2) SHA1(baffba15c5ba0f139b74e3dedd1d68e14fc7b370) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "gnome.001", 0x000000, 0x80000, CRC(6ed866d7) SHA1(68d75d24d98e6d533cb26ceac0a680203cb26069) )
	ROM_LOAD( "gnome.002", 0x100000, 0x80000, CRC(f6e5e6f0) SHA1(9751e8df87f14a252595547d24b8dd865ee4f08d) )
	ROM_LOAD( "gnome.003", 0x200000, 0x80000, CRC(f8beb972) SHA1(3afbca8ce7e69d2dadae05f69205a6fd9036cf6a) )
	ROM_LOAD( "gnome.004", 0x300000, 0x80000, CRC(83357c38) SHA1(45cd31c4f02f9d7b1888701c2146d1e7229b6cb5) )
	ROM_LOAD( "gnome.005", 0x080000, 0x80000, CRC(687ad3e3) SHA1(23941a4f40c45029b9a43451f78b04c03c3cd7da) )
	ROM_LOAD( "gnome.006", 0x180000, 0x80000, CRC(7ef2b88a) SHA1(7e7de60fc6791731d7cfd6a50e2bc5af1bf5e4b2) )
	ROM_LOAD( "gnome.007", 0x280000, 0x80000, CRC(71976bdf) SHA1(c44dbfa75a0f12893b3177907fc93b3d5e8ad390) )
	ROM_LOAD( "gnome.008", 0x380000, 0x80000, CRC(c86a1586) SHA1(e622bca8dc618ca8edc1a7daa9c8286383caebef) )
ROM_END

ROM_START( gnomed ) // 070906 custom alteras, modified graphics, many texts changed, changed version text to "VIDEO GAME-1 GN01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gn_070906d.rom", 0x00000, 0x40000, CRC(12682805) SHA1(58a5c365e56ce3d679fe88f2930471ac63549d85) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1a", 0x000000, 0x80000, CRC(55a8b1ba) SHA1(0b6c906435e260178292014a9e160a364bbcd6ad) )
	ROM_LOAD( "bootleg_2a", 0x100000, 0x80000, CRC(2ee1cc82) SHA1(081bbbd7c4589ae62efd1ef06b9f593b90492ee7) )
	ROM_LOAD( "bootleg_3a", 0x200000, 0x80000, CRC(7011d0b8) SHA1(24987121c6a0e712ad355eee2d7be5cf9e24e908) )
	ROM_LOAD( "bootleg_4a", 0x300000, 0x80000, CRC(e6bb6057) SHA1(d214f7b4b235cf1c80cb1131f69244c25287938b) )
	ROM_LOAD( "bootleg_5a", 0x080000, 0x80000, CRC(668c21a8) SHA1(1fccab7363350f2a26af40e05b4e13369bcb5a49) )
	ROM_LOAD( "bootleg_6a", 0x180000, 0x80000, CRC(95c2d96f) SHA1(8ac7283b65940fc620e126ad7e8563d96378dfd4) )
	ROM_LOAD( "bootleg_7a", 0x280000, 0x80000, CRC(476d6b00) SHA1(106c4afdb4d4d5afe9ec8395ef26050c814f053a) )
	ROM_LOAD( "bootleg_8a", 0x380000, 0x80000, CRC(658becb6) SHA1(7280ce53796326a48da8e461de8f78ea78a2f8c2) )
ROM_END

ROM_START( gnomee ) // 070906 custom alteras, modified graphics, bank F9, many texts changed, changed version text to "LOTOS GN01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gn_070906e.rom", 0x00000, 0x40000, CRC(33fbae84) SHA1(4b8fde65042de38b24c77665b76782db4efccf0f) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1b", 0x000000, 0x80000, CRC(414b537c) SHA1(65cc2a3bca8792387a83323f51b7345ee1be1125) )
	ROM_LOAD( "bootleg_2b", 0x100000, 0x80000, CRC(3452bf73) SHA1(33e45d38bc7243cb4dbda483138fe529a693fad9) )
	ROM_LOAD( "bootleg_3b", 0x200000, 0x80000, CRC(1e44b605) SHA1(c7e1018fb36cb1f1db006d06dbde94baeb609c46) )
	ROM_LOAD( "bootleg_4b", 0x300000, 0x80000, CRC(19a7ff3e) SHA1(7a58c0db9615b0a4e6cb8480d56b9ecf2a4cec1d) )
	ROM_LOAD( "bootleg_5a", 0x080000, 0x80000, CRC(668c21a8) SHA1(1fccab7363350f2a26af40e05b4e13369bcb5a49) )
	ROM_LOAD( "bootleg_6a", 0x180000, 0x80000, CRC(95c2d96f) SHA1(8ac7283b65940fc620e126ad7e8563d96378dfd4) )
	ROM_LOAD( "bootleg_7a", 0x280000, 0x80000, CRC(476d6b00) SHA1(106c4afdb4d4d5afe9ec8395ef26050c814f053a) )
	ROM_LOAD( "bootleg_8a", 0x380000, 0x80000, CRC(658becb6) SHA1(7280ce53796326a48da8e461de8f78ea78a2f8c2) )
ROM_END

ROM_START( gnome_2a ) // 071115 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gn_071115a.rom", 0x00000, 0x40000, CRC(97e74228) SHA1(6b4d108c6c59ec335c14f0e5ab990cc8782ed236) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "gnome.001", 0x000000, 0x80000, CRC(6ed866d7) SHA1(68d75d24d98e6d533cb26ceac0a680203cb26069) )
	ROM_LOAD( "gnome.002", 0x100000, 0x80000, CRC(f6e5e6f0) SHA1(9751e8df87f14a252595547d24b8dd865ee4f08d) )
	ROM_LOAD( "gnome.003", 0x200000, 0x80000, CRC(f8beb972) SHA1(3afbca8ce7e69d2dadae05f69205a6fd9036cf6a) )
	ROM_LOAD( "gnome.004", 0x300000, 0x80000, CRC(83357c38) SHA1(45cd31c4f02f9d7b1888701c2146d1e7229b6cb5) )
	ROM_LOAD( "gnome.005", 0x080000, 0x80000, CRC(687ad3e3) SHA1(23941a4f40c45029b9a43451f78b04c03c3cd7da) )
	ROM_LOAD( "gnome.006", 0x180000, 0x80000, CRC(7ef2b88a) SHA1(7e7de60fc6791731d7cfd6a50e2bc5af1bf5e4b2) )
	ROM_LOAD( "gnome.007", 0x280000, 0x80000, CRC(71976bdf) SHA1(c44dbfa75a0f12893b3177907fc93b3d5e8ad390) )
	ROM_LOAD( "gnome.008", 0x380000, 0x80000, CRC(c86a1586) SHA1(e622bca8dc618ca8edc1a7daa9c8286383caebef) )
ROM_END


ROM_START( gnome_3a ) // 080303 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gn_m_080303a.rom", 0x00000, 0x40000, CRC(86e3f741) SHA1(e5d9f897daa6cf58a7fb426c42810d71bf839294) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "gnome.001", 0x000000, 0x80000, CRC(6ed866d7) SHA1(68d75d24d98e6d533cb26ceac0a680203cb26069) )
	ROM_LOAD( "gnome.002", 0x100000, 0x80000, CRC(f6e5e6f0) SHA1(9751e8df87f14a252595547d24b8dd865ee4f08d) )
	ROM_LOAD( "gnome.003", 0x200000, 0x80000, CRC(f8beb972) SHA1(3afbca8ce7e69d2dadae05f69205a6fd9036cf6a) )
	ROM_LOAD( "gnome.004", 0x300000, 0x80000, CRC(83357c38) SHA1(45cd31c4f02f9d7b1888701c2146d1e7229b6cb5) )
	ROM_LOAD( "gnome.005", 0x080000, 0x80000, CRC(687ad3e3) SHA1(23941a4f40c45029b9a43451f78b04c03c3cd7da) )
	ROM_LOAD( "gnome.006", 0x180000, 0x80000, CRC(7ef2b88a) SHA1(7e7de60fc6791731d7cfd6a50e2bc5af1bf5e4b2) )
	ROM_LOAD( "gnome.007", 0x280000, 0x80000, CRC(71976bdf) SHA1(c44dbfa75a0f12893b3177907fc93b3d5e8ad390) )
	ROM_LOAD( "gnome.008", 0x380000, 0x80000, CRC(c86a1586) SHA1(e622bca8dc618ca8edc1a7daa9c8286383caebef) )
ROM_END

ROM_START( gnome_3b ) // 080303 bank F9, payout percentage 45%
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gn_m_080303b.rom", 0x00000, 0x40000, CRC(c01d2d38) SHA1(7743d46e2ebce24d88961e52b67a928b29a3aa0d) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "gnome.001", 0x000000, 0x80000, CRC(6ed866d7) SHA1(68d75d24d98e6d533cb26ceac0a680203cb26069) )
	ROM_LOAD( "gnome.002", 0x100000, 0x80000, CRC(f6e5e6f0) SHA1(9751e8df87f14a252595547d24b8dd865ee4f08d) )
	ROM_LOAD( "gnome.003", 0x200000, 0x80000, CRC(f8beb972) SHA1(3afbca8ce7e69d2dadae05f69205a6fd9036cf6a) )
	ROM_LOAD( "gnome.004", 0x300000, 0x80000, CRC(83357c38) SHA1(45cd31c4f02f9d7b1888701c2146d1e7229b6cb5) )
	ROM_LOAD( "gnome.005", 0x080000, 0x80000, CRC(687ad3e3) SHA1(23941a4f40c45029b9a43451f78b04c03c3cd7da) )
	ROM_LOAD( "gnome.006", 0x180000, 0x80000, CRC(7ef2b88a) SHA1(7e7de60fc6791731d7cfd6a50e2bc5af1bf5e4b2) )
	ROM_LOAD( "gnome.007", 0x280000, 0x80000, CRC(71976bdf) SHA1(c44dbfa75a0f12893b3177907fc93b3d5e8ad390) )
	ROM_LOAD( "gnome.008", 0x380000, 0x80000, CRC(c86a1586) SHA1(e622bca8dc618ca8edc1a7daa9c8286383caebef) )
ROM_END

ROM_START( gnome_3c ) // 080303 bank F9, payout percentage 60%
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gn_m_080303c.rom", 0x00000, 0x40000, CRC(a7479ddb) SHA1(d64be81ff20548373e08ccae58d00f5cb2a335da) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "gnome.001", 0x000000, 0x80000, CRC(6ed866d7) SHA1(68d75d24d98e6d533cb26ceac0a680203cb26069) )
	ROM_LOAD( "gnome.002", 0x100000, 0x80000, CRC(f6e5e6f0) SHA1(9751e8df87f14a252595547d24b8dd865ee4f08d) )
	ROM_LOAD( "gnome.003", 0x200000, 0x80000, CRC(f8beb972) SHA1(3afbca8ce7e69d2dadae05f69205a6fd9036cf6a) )
	ROM_LOAD( "gnome.004", 0x300000, 0x80000, CRC(83357c38) SHA1(45cd31c4f02f9d7b1888701c2146d1e7229b6cb5) )
	ROM_LOAD( "gnome.005", 0x080000, 0x80000, CRC(687ad3e3) SHA1(23941a4f40c45029b9a43451f78b04c03c3cd7da) )
	ROM_LOAD( "gnome.006", 0x180000, 0x80000, CRC(7ef2b88a) SHA1(7e7de60fc6791731d7cfd6a50e2bc5af1bf5e4b2) )
	ROM_LOAD( "gnome.007", 0x280000, 0x80000, CRC(71976bdf) SHA1(c44dbfa75a0f12893b3177907fc93b3d5e8ad390) )
	ROM_LOAD( "gnome.008", 0x380000, 0x80000, CRC(c86a1586) SHA1(e622bca8dc618ca8edc1a7daa9c8286383caebef) )
ROM_END

ROM_START( gnome_5a ) // 090406 bank F9, payout percentage 70%
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gn_m_090406a.rom", 0x00000, 0x40000, CRC(51558b99) SHA1(ec9788698f93b96ad26f2f4128733702ac6c2d2e) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "gnome.001", 0x000000, 0x80000, CRC(6ed866d7) SHA1(68d75d24d98e6d533cb26ceac0a680203cb26069) )
	ROM_LOAD( "gnome.002", 0x100000, 0x80000, CRC(f6e5e6f0) SHA1(9751e8df87f14a252595547d24b8dd865ee4f08d) )
	ROM_LOAD( "gnome.003", 0x200000, 0x80000, CRC(f8beb972) SHA1(3afbca8ce7e69d2dadae05f69205a6fd9036cf6a) )
	ROM_LOAD( "gnome.004", 0x300000, 0x80000, CRC(83357c38) SHA1(45cd31c4f02f9d7b1888701c2146d1e7229b6cb5) )
	ROM_LOAD( "gnome.005", 0x080000, 0x80000, CRC(687ad3e3) SHA1(23941a4f40c45029b9a43451f78b04c03c3cd7da) )
	ROM_LOAD( "gnome.006", 0x180000, 0x80000, CRC(7ef2b88a) SHA1(7e7de60fc6791731d7cfd6a50e2bc5af1bf5e4b2) )
	ROM_LOAD( "gnome.007", 0x280000, 0x80000, CRC(71976bdf) SHA1(c44dbfa75a0f12893b3177907fc93b3d5e8ad390) )
	ROM_LOAD( "gnome.008", 0x380000, 0x80000, CRC(c86a1586) SHA1(e622bca8dc618ca8edc1a7daa9c8286383caebef) )
ROM_END

ROM_START( gnome_5b ) // 090406 bank F9, modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gn_m_090406b.rom", 0x00000, 0x40000, CRC(f7b55e2f) SHA1(e7043433acea5387bc9c67f94e09ab3dae0b8ccf) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1c", 0x000000, 0x80000, CRC(a52efb59) SHA1(724094b057dbf0b29e98c2c6c1bde7fac5534969) )
	ROM_LOAD( "bootleg_2c", 0x100000, 0x80000, CRC(ec259703) SHA1(2d9be4b9a2a9a6a34b2e61cd117940c4f2be849b) )
	ROM_LOAD( "bootleg_3c", 0x200000, 0x80000, CRC(68e057fc) SHA1(35e93e404ce47faa384d0c1c8c4dce77cfa0a960) )
	ROM_LOAD( "bootleg_4c", 0x300000, 0x80000, CRC(0dd2f6bf) SHA1(1976f8665353745ea0032c2cf0ce05a75acf030c) )
	ROM_LOAD( "bootleg_5c", 0x080000, 0x80000, CRC(c1b142fb) SHA1(a24dbbe4ae056b682604f44ead6fd1633df33e87) )
	ROM_LOAD( "bootleg_6c", 0x180000, 0x80000, CRC(0efe6ea1) SHA1(0bd96da3a2e7ed47f266c391835cbfb0fe5731e6) )
	ROM_LOAD( "bootleg_7",  0x280000, 0x80000, CRC(71976bdf) SHA1(c44dbfa75a0f12893b3177907fc93b3d5e8ad390) )
	ROM_LOAD( "gnome.008",  0x380000, 0x80000, CRC(c86a1586) SHA1(e622bca8dc618ca8edc1a7daa9c8286383caebef) )
ROM_END


ROM_START( fcockt2a ) // 080707 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc2_080707a.rom", 0x00000, 0x40000, CRC(c4840b3c) SHA1(518079207bb3d13e492003b382d28a8cf3647f63) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail2.001",  0x000000, 0x80000, CRC(d1b9416d) SHA1(4d3cb0a6dbcf02bbd20d5c43df358882b2ad794d) )
	ROM_LOAD( "fruitcocktail2.002",  0x100000, 0x80000, CRC(69236be1) SHA1(9a2e6c8f279714f79a606c0b118b6bf1d8442cda) )
	ROM_LOAD( "fruitcocktail2.003",  0x200000, 0x80000, CRC(29aade8c) SHA1(bef42f8a25c90e3a1cccd13872a10eb8b2b2e276) )
	ROM_LOAD( "fruitcocktail2.004",  0x300000, 0x80000, CRC(4b9646e7) SHA1(26548a018401f4e07383eb145f8f0847677f3272) )
	ROM_LOAD( "fruitcocktail2_old.005", 0x080000, 0x80000, CRC(6b9e6b43) SHA1(c7fb17e91ec62b22da42f110d68b4f37e39de3ce) )
	ROM_LOAD( "fruitcocktail2_old.006", 0x180000, 0x80000, CRC(2c9f712e) SHA1(c3118154eafca74b66b3325a2e07c85f86f3544d) )
	ROM_LOAD( "fruitcocktail2_old.007", 0x280000, 0x80000, CRC(85ba9a86) SHA1(aa9b6170135e9e420509e8f7c1702c9896bc5d8e) )
	ROM_LOAD( "fruitcocktail2_old.008", 0x380000, 0x80000, CRC(a27c49a2) SHA1(7c9ee0e01f76ca3ab6716579f5dde7036050970b) )
ROM_END

ROM_START( fcockt2_4a ) // 081105 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc2_m_081105a.rom", 0x00000, 0x40000,  CRC(411ad706) SHA1(4d29a6b1927241efdcdb3224b9ebe0eb6af92533) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail2.001", 0x000000, 0x80000, CRC(d1b9416d) SHA1(4d3cb0a6dbcf02bbd20d5c43df358882b2ad794d) )
	ROM_LOAD( "fruitcocktail2.002", 0x100000, 0x80000, CRC(69236be1) SHA1(9a2e6c8f279714f79a606c0b118b6bf1d8442cda) )
	ROM_LOAD( "fruitcocktail2.003", 0x200000, 0x80000, CRC(29aade8c) SHA1(bef42f8a25c90e3a1cccd13872a10eb8b2b2e276) )
	ROM_LOAD( "fruitcocktail2.004", 0x300000, 0x80000, CRC(4b9646e7) SHA1(26548a018401f4e07383eb145f8f0847677f3272) )
	ROM_LOAD( "fruitcocktail2.005", 0x080000, 0x80000, CRC(3fc13d72) SHA1(ba0727138ef03d576d190cbce04b9eb0bba88a9a) )
	ROM_LOAD( "fruitcocktail2.006", 0x180000, 0x80000, CRC(8fc75fd7) SHA1(3b7cd8a3e04ca9d4494b37c801e21d1293f094e8) )
	ROM_LOAD( "fruitcocktail2.007", 0x280000, 0x80000, CRC(d37fcc0f) SHA1(57c2ea5dc747f16e2233305f2c73cb4b632aae2c) )
	ROM_LOAD( "fruitcocktail2.008", 0x380000, 0x80000, CRC(e3a9442c) SHA1(cbaba182e858b0f158756118e5da873e3ddfc0b9) )
ROM_END

ROM_START( fcockt2_4b ) // 081105 bank F9, no credit limit, "MaxVin" signature
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc2_m_081105_cm.rom", 0x00000, 0x40000,  CRC(f0c702b9) SHA1(89121ddd325aac567ebd443eb443c4db0549236c) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail2.001", 0x000000, 0x80000, CRC(d1b9416d) SHA1(4d3cb0a6dbcf02bbd20d5c43df358882b2ad794d) )
	ROM_LOAD( "fruitcocktail2.002", 0x100000, 0x80000, CRC(69236be1) SHA1(9a2e6c8f279714f79a606c0b118b6bf1d8442cda) )
	ROM_LOAD( "fruitcocktail2.003", 0x200000, 0x80000, CRC(29aade8c) SHA1(bef42f8a25c90e3a1cccd13872a10eb8b2b2e276) )
	ROM_LOAD( "fruitcocktail2.004", 0x300000, 0x80000, CRC(4b9646e7) SHA1(26548a018401f4e07383eb145f8f0847677f3272) )
	ROM_LOAD( "fruitcocktail2.005", 0x080000, 0x80000, CRC(3fc13d72) SHA1(ba0727138ef03d576d190cbce04b9eb0bba88a9a) )
	ROM_LOAD( "fruitcocktail2.006", 0x180000, 0x80000, CRC(8fc75fd7) SHA1(3b7cd8a3e04ca9d4494b37c801e21d1293f094e8) )
	ROM_LOAD( "fruitcocktail2.007", 0x280000, 0x80000, CRC(d37fcc0f) SHA1(57c2ea5dc747f16e2233305f2c73cb4b632aae2c) )
	ROM_LOAD( "fruitcocktail2.008", 0x380000, 0x80000, CRC(e3a9442c) SHA1(cbaba182e858b0f158756118e5da873e3ddfc0b9) )
ROM_END

ROM_START( fcockt2_4c ) // 081105 custom alteras, modified graphics, bank F9, many texts changed, changed version text to "VIDEO GAME-1 FR02"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc2_m_081105b.rom", 0x00000, 0x40000,  CRC(b627f7d6) SHA1(0bcdb1cd808055d0a9e965f2c6ff522ac3fe6949) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1b", 0x000000, 0x80000, CRC(68b62355) SHA1(387da330fddaa685cb7aca32bf861e44d5cdbe58) )
	ROM_LOAD( "bootleg_2b", 0x100000, 0x80000, CRC(d36e8704) SHA1(80a84a24b3a43a0110a00ed3b01c7156218ccbf3) )
	ROM_LOAD( "bootleg_3b", 0x200000, 0x80000, CRC(57d87c5b) SHA1(6bc28e5c34a7e76d0e601b970ab922e5ad28ae85) )
	ROM_LOAD( "bootleg_4b", 0x300000, 0x80000, CRC(3de30bab) SHA1(c4c4c549ad980f562238054cd0e3ef6b0977f7b7) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(b47d65aa) SHA1(71c642ea7887ea35c6d7d985502082d2424c035a) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(aa982f4e) SHA1(edf75d4ddd60532ac0ca2e17e3fd9004aa768973) )
	ROM_LOAD( "bootleg_7b", 0x280000, 0x80000, CRC(b9b1ead7) SHA1(111e505fce8fe75043ff7cc5dc68e81ce4f98d06) )
	ROM_LOAD( "bootleg_8b", 0x380000, 0x80000, CRC(8e49d1c1) SHA1(4735503335cf499895d1b2d9a405be50f21a2d68) )
ROM_END

ROM_START( fcockt2_4d ) // 081105 bank F9, no credit limit, "MaxVin" signature, payout percentage 70%
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc2_m_081105c.rom", 0x00000, 0x40000,  CRC(b6d88fcc) SHA1(337a59accfbc229be05aff84ba70fc135fde09de) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail2.001", 0x000000, 0x80000, CRC(d1b9416d) SHA1(4d3cb0a6dbcf02bbd20d5c43df358882b2ad794d) )
	ROM_LOAD( "fruitcocktail2.002", 0x100000, 0x80000, CRC(69236be1) SHA1(9a2e6c8f279714f79a606c0b118b6bf1d8442cda) )
	ROM_LOAD( "fruitcocktail2.003", 0x200000, 0x80000, CRC(29aade8c) SHA1(bef42f8a25c90e3a1cccd13872a10eb8b2b2e276) )
	ROM_LOAD( "fruitcocktail2.004", 0x300000, 0x80000, CRC(4b9646e7) SHA1(26548a018401f4e07383eb145f8f0847677f3272) )
	ROM_LOAD( "fruitcocktail2.005", 0x080000, 0x80000, CRC(3fc13d72) SHA1(ba0727138ef03d576d190cbce04b9eb0bba88a9a) )
	ROM_LOAD( "fruitcocktail2.006", 0x180000, 0x80000, CRC(8fc75fd7) SHA1(3b7cd8a3e04ca9d4494b37c801e21d1293f094e8) )
	ROM_LOAD( "fruitcocktail2.007", 0x280000, 0x80000, CRC(d37fcc0f) SHA1(57c2ea5dc747f16e2233305f2c73cb4b632aae2c) )
	ROM_LOAD( "fruitcocktail2.008", 0x380000, 0x80000, CRC(e3a9442c) SHA1(cbaba182e858b0f158756118e5da873e3ddfc0b9) )
ROM_END

ROM_START( fcockt2_4e ) // 081105 bank F9, modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc2_m_081105d.rom", 0x00000, 0x40000,  CRC(37b1a159) SHA1(52c8e825ebb85a59803a6d14c766892e56a72c9e) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1c", 0x000000, 0x80000, CRC(9fe02e19) SHA1(dec48a0990d2341ae86a3a4acd5972844b93fccb) )
	ROM_LOAD( "bootleg_2c", 0x100000, 0x80000, CRC(734bd656) SHA1(e44124fd60675cfc8319bca5bbd90f99d73f4896) )
	ROM_LOAD( "bootleg_3c", 0x200000, 0x80000, CRC(36ff74dc) SHA1(7df93e4fa1c240adf33ef910efb8397abfa1a106) )
	ROM_LOAD( "bootleg_4c", 0x300000, 0x80000, CRC(b88356c9) SHA1(fbdcfc0d30f2a9b1364863d7a06b32f94113e865) )
	ROM_LOAD( "bootleg_5c", 0x080000, 0x80000, CRC(14053302) SHA1(adf0a1493074d16797e8d19dc98f2d5d2cb26e43) )
	ROM_LOAD( "bootleg_6c", 0x180000, 0x80000, CRC(21a91970) SHA1(6f4832c743469171cd0efe4554f243c4bed21e88) )
	ROM_LOAD( "bootleg_7c", 0x280000, 0x80000, CRC(3a8a7340) SHA1(9408791686926dd13216352ffcb04ad9f8954b3e) )
	ROM_LOAD( "bootleg_8c", 0x380000, 0x80000, CRC(fbf48ebd) SHA1(04917e613d9a3f0caeab71c78b9f77c4b5b84988) )
ROM_END

ROM_START( fcockt2_4f ) // 081105 custom alteras, modified graphics, bank F9, many texts changed, changed version text to "LOTOS FR02"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc2_m_081105f.rom", 0x00000, 0x40000,  CRC(b4fa0663) SHA1(aee3cb980fc7d9fbba1a4e9fe80e6be8df58bb3b) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1e", 0x000000, 0x80000, CRC(3c72a5e2) SHA1(15e07d64a0d633ccdadea9eb55556e6aa239c0e5) )
	ROM_LOAD( "bootleg_2e", 0x100000, 0x80000, CRC(017a61f7) SHA1(023cec2be1cfe885c562415d18fc32edf3c61346) )
	ROM_LOAD( "bootleg_3e", 0x200000, 0x80000, CRC(8ea5532f) SHA1(498991cfb7a92c3a6342e824a310c7edc5577cff) )
	ROM_LOAD( "bootleg_4e", 0x300000, 0x80000, CRC(c879025f) SHA1(93f77787ded2a4353320dcb9d4d1b1f5dd7a3b13) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(b47d65aa) SHA1(71c642ea7887ea35c6d7d985502082d2424c035a) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(aa982f4e) SHA1(edf75d4ddd60532ac0ca2e17e3fd9004aa768973) )
	ROM_LOAD( "bootleg_7b", 0x280000, 0x80000, CRC(b9b1ead7) SHA1(111e505fce8fe75043ff7cc5dc68e81ce4f98d06) )
	ROM_LOAD( "bootleg_8b", 0x380000, 0x80000, CRC(8e49d1c1) SHA1(4735503335cf499895d1b2d9a405be50f21a2d68) )
ROM_END


GAME( 2002, mfish_3a,   mfish_parent,   igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Multi Fish (bootleg, 021124, banking address hack)", MACHINE_SUPPORTS_SAVE ) // bank F9
GAME( 2002, mfish_12a,  mfish_parent,   igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Multi Fish (bootleg, 040308, banking address hack)", MACHINE_SUPPORTS_SAVE ) // bank F9

GAME( 2003, czmon_7a,   czmon_parent,   igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Crazy Monkey (bootleg, 031110, backdoor set 1)", MACHINE_SUPPORTS_SAVE ) // backdoor 1,1 1,3 1,5 1,7  3,3  3,4
GAME( 2003, czmon_7b,   czmon_parent,   igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Crazy Monkey (bootleg, 031110, backdoor set 2)", MACHINE_SUPPORTS_SAVE ) // backdoor 1,5 5,5 1,7 3,2  3,3  3,4
GAME( 2003, czmon_8a,   czmon_parent,   igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Crazy Monkey (bootleg, 050120, backdoor)", MACHINE_SUPPORTS_SAVE ) // backdoor 1,1 1,3 1,5 1,7  3,3  3,4
GAME( 2003, czmon_8b,   czmon_parent,   igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Crazy Monkey (bootleg, 050120, changed version text)", MACHINE_SUPPORTS_SAVE ) // changed version text to 070315
GAME( 2003, czmon_8c,   czmon_parent,   igrosoft_gamble, igrosoft_gamble, igrosoft_gamble_state, customl,ROT0,  "bootleg", "Crazy Monkey (bootleg, 050120, VIDEO GAME-1 CM01)", MACHINE_SUPPORTS_SAVE ) // custom alteras, modified graphics, changed version text to "VIDEO GAME-1 CM01"
GAME( 2003, czmon_8d,   czmon_parent,   igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Crazy Monkey (bootleg, 050120, LOTTOGAME (I))", MACHINE_SUPPORTS_SAVE ) // modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
GAME( 2003, czmon_8e,   czmon_parent,   igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Crazy Monkey (bootleg, 050120, LOTO PROGRAM V-CM2)", MACHINE_SUPPORTS_SAVE ) // modified graphics, many texts changed, changed version text to "LOTO PROGRAM V-CM2"
GAME( 2003, czmon_8f,   czmon_parent,   igrosoft_gamble, igrosoft_gamble, igrosoft_gamble_state, customl,ROT0,  "bootleg", "Crazy Monkey (bootleg, 050120, LOTOS CM01)", MACHINE_SUPPORTS_SAVE ) // custom alteras, modified graphics, many texts changed, changed version text to "LOTOS CM01"
GAME( 2003, czmon_9a,   czmon_parent,   igrosoft_gamble, igrosoft_gamble, igrosoft_gamble_state, customl,ROT0,  "bootleg", "Crazy Monkey (bootleg, 070315, VIDEO GAME-1 O01 set 1)", MACHINE_SUPPORTS_SAVE ) // custom alteras, modified graphics, changed version text to "VIDEO GAME-1 O01"
GAME( 2003, czmon_9b,   czmon_parent,   igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Crazy Monkey (bootleg, 070315, VIDEO GAME-1 O01 set 2)", MACHINE_SUPPORTS_SAVE ) // modified graphics, changed version text to "VIDEO GAME-1 O01" (czmon_9a, decoded gfx)
GAME( 2003, czmon_9c,   czmon_parent,   igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Crazy Monkey (bootleg, 070315, payout percentage 70)", MACHINE_SUPPORTS_SAVE ) // payout percentage 70%

GAME( 2003, fcockt_6a,  fcockt_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Fruit Cocktail (bootleg, 040216, banking address hack)", MACHINE_SUPPORTS_SAVE ) // bank F8
GAME( 2003, fcockt_6b,  fcockt_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Fruit Cocktail (bootleg, 040216, backdoor)", MACHINE_SUPPORTS_SAVE ) // backdoor 1,1 1,3 1,5 1,7  3,3  3,4
GAME( 2003, fcockt_6c,  fcockt_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Fruit Cocktail (bootleg, 040216, LotoRossy+)", MACHINE_SUPPORTS_SAVE ) // modified graphics, some code changes, description says "for Lat-02 terminals", older set
GAME( 2003, fcockt_6d,  fcockt_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Fruit Cocktail (bootleg, 040216, VIDEO GAME-1 FR01)", MACHINE_SUPPORTS_SAVE ) // modified graphics, some code changes, changed version text to "VIDEO GAME-1 FR01", description says "for Lat-02 terminals", newer set
GAME( 2003, fcockt_7a,  fcockt_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Fruit Cocktail (bootleg, 050118, backdoor)", MACHINE_SUPPORTS_SAVE ) // backdoor 1,1 1,3 1,5 1,7  3,3  3,4
GAME( 2003, fcockt_7b,  fcockt_parent,  igrosoft_gamble, igrosoft_gamble, igrosoft_gamble_state, customl,ROT0,  "bootleg", "Fruit Cocktail (bootleg, 050118, VIDEO GAME-1 FR01)", MACHINE_SUPPORTS_SAVE ) // custom alteras, modified graphics, many texts changed, changed version text to "VIDEO GAME-1 FR01"
GAME( 2003, fcockt_7c,  fcockt_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Fruit Cocktail (bootleg, 050118, payout percentage 40)", MACHINE_SUPPORTS_SAVE ) // payout percentage 40%
GAME( 2003, fcockt_7d,  fcockt_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Fruit Cocktail (bootleg, 050118, payout percentage 60)", MACHINE_SUPPORTS_SAVE ) // payout percentage 60%
GAME( 2003, fcockt_7e,  fcockt_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Fruit Cocktail (bootleg, 050118, payout percentage 70)", MACHINE_SUPPORTS_SAVE ) // payout percentage 70%
GAME( 2003, fcockt_7f,  fcockt_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Fruit Cocktail (bootleg, 050118, changed version text)", MACHINE_SUPPORTS_SAVE ) // changed version text to 070305
GAME( 2003, fcockt_7g,  fcockt_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Fruit Cocktail (bootleg, 050118, LOTO PROGRAM V-FC2)", MACHINE_SUPPORTS_SAVE ) // modified graphics, many texts changed, changed version text to "LOTO PROGRAM V-FC2"
GAME( 2003, fcockt_7h,  fcockt_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Fruit Cocktail (bootleg, 050118, LOTOS FR01)", MACHINE_SUPPORTS_SAVE ) // modified graphics, many texts changed, changed version text to "LOTOS FR01"
GAME( 2003, fcockt_8a,  fcockt_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Fruit Cocktail (bootleg, 060111, LOTO COCKTAIL V01-0001)", MACHINE_SUPPORTS_SAVE ) // modified graphics, many texts changed, changed version text to "LOTO COCKTAIL V01-0001"
GAME( 2003, fcockt_8b,  fcockt_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Fruit Cocktail (bootleg, 060111, LOTTOGAME (I))", MACHINE_SUPPORTS_SAVE ) // modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"

GAME( 2003, lhaunt_4a,  lhaunt_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Lucky Haunter (bootleg, 031111, backdoor)", MACHINE_SUPPORTS_SAVE ) // backdoor 1,1 1,3 1,5 1,7  3,3  3,4
GAME( 2003, lhaunt_5a,  lhaunt_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Lucky Haunter (bootleg, 040216, backdoor)", MACHINE_SUPPORTS_SAVE ) // backdoor 1,1 1,3 1,5 1,7  3,3  3,4
GAME( 2003, lhaunt_6a,  lhaunt_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Lucky Haunter (bootleg, 040825, backdoor)", MACHINE_SUPPORTS_SAVE ) // backdoor 1,5 9,1 5,1 1,5  3,3  3,4
GAME( 2003, lhaunt_6b,  lhaunt_parent,  igrosoft_gamble, igrosoft_gamble, igrosoft_gamble_state, customl,ROT0,  "bootleg", "Lucky Haunter (bootleg, 040825, VIDEO GAME-1 PB01)", MACHINE_SUPPORTS_SAVE ) // custom alteras, modified graphics, many texts changed, changed version text to "VIDEO GAME-1 PB01"
GAME( 2003, lhaunt_6c,  lhaunt_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Lucky Haunter (bootleg, 040825, changed version text)", MACHINE_SUPPORTS_SAVE ) // changed version text to 070604
GAME( 2003, lhaunt_6d,  lhaunt_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Lucky Haunter (bootleg, 040825, LOTTOGAME (I))", MACHINE_SUPPORTS_SAVE ) // modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
GAME( 2003, lhaunt_6e,  lhaunt_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Lucky Haunter (bootleg, 040825, LOTO PROGRAM V-LH2)", MACHINE_SUPPORTS_SAVE ) // modified graphics, many texts changed, changed version text to "LOTO PROGRAM V-LH2"
GAME( 2003, lhaunt_6f,  lhaunt_parent,  igrosoft_gamble, igrosoft_gamble, igrosoft_gamble_state, customl,ROT0,  "bootleg", "Lucky Haunter (bootleg, 040825, LOTOS PB01)", MACHINE_SUPPORTS_SAVE ) // custom alteras, modified graphics, many texts changed, changed version text to "LOTOS PB01"

GAME( 2004, garage_4a,  garage_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Garage (bootleg, 040219, backdoor)", MACHINE_SUPPORTS_SAVE ) // backdoor 1,1 1,3 1,5 1,7  3,3  3,4
GAME( 2004, garage_4b,  garage_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Garage (bootleg, 040219, changed version text)", MACHINE_SUPPORTS_SAVE ) // changed version text to 070329
GAME( 2004, garage_4c,  garage_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Garage (bootleg, 040219, LOTO PROGRAM V-GG2)", MACHINE_SUPPORTS_SAVE ) // modified graphics, many texts changed, changed version text to "LOTO PROGRAM V-GG2"
GAME( 2004, garage_5a,  garage_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Garage (bootleg, 050311, backdoor)", MACHINE_SUPPORTS_SAVE ) // backdoor 1,1 1,3 1,5 1,7  3,3  3,4
GAME( 2004, garage_5b,  garage_parent,  igrosoft_gamble, igrosoft_gamble, igrosoft_gamble_state, customl,ROT0,  "bootleg", "Garage (bootleg, 050311, VIDEO GAME-1 GA01)", MACHINE_SUPPORTS_SAVE ) // custom alteras, modified graphics, changed version text to "VIDEO GAME-1 GA01"
GAME( 2004, garage_5c,  garage_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Garage (bootleg, 050311, payout percentage 70)", MACHINE_SUPPORTS_SAVE ) // payout percentage 70%
GAME( 2004, garage_5d,  garage_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Garage (bootleg, 050311, LOTTOGAME (I))", MACHINE_SUPPORTS_SAVE ) // modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
GAME( 2004, garage_5e,  garage_parent,  igrosoft_gamble, igrosoft_gamble, igrosoft_gamble_state, customl,ROT0,  "bootleg", "Garage (bootleg, 050311, LOTOS GA01)", MACHINE_SUPPORTS_SAVE ) // custom alteras, modified graphics, many texts changed, changed version text to "LOTOS GA01"

GAME( 2004, rclimb_3a,  rclimb_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Rock Climber (bootleg, 040827, backdoor)", MACHINE_SUPPORTS_SAVE ) // backdoor 1,5 9,1 5,1 1,5  3,3  3,4
GAME( 2004, rclimb_3b,  rclimb_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Rock Climber (bootleg, 040827, new service menu)", MACHINE_SUPPORTS_SAVE ) // new service menu
GAME( 2004, rclimb_3c,  rclimb_parent,  igrosoft_gamble, igrosoft_gamble, igrosoft_gamble_state, customl,ROT0,  "bootleg", "Rock Climber (bootleg, 040827, VIDEO GAME-1 SK01)", MACHINE_SUPPORTS_SAVE ) // custom alteras, modified graphics, many texts changed, changed version text to "VIDEO GAME-1 SK01"
GAME( 2004, rclimb_3d,  rclimb_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Rock Climber (bootleg, 040827, LOTTOGAME (I))", MACHINE_SUPPORTS_SAVE ) // modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
GAME( 2004, rclimb_3e,  rclimb_parent,  igrosoft_gamble, igrosoft_gamble, igrosoft_gamble_state, customl,ROT0,  "bootleg", "Rock Climber (bootleg, 040827, LOTOS SK01)", MACHINE_SUPPORTS_SAVE ) // custom alteras, modified graphics, many texts changed, changed version text to "LOTOS SK01"

GAME( 2004, sweetla,    sweetl_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Sweet Life (bootleg, 041220, backdoor)", MACHINE_SUPPORTS_SAVE ) // backdoor 1,5 9,1 5,3 1,5  3,3  3,4
GAME( 2004, sweetlb,    sweetl_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Sweet Life (bootleg, 041220, banking address hack, changed version text)", MACHINE_SUPPORTS_SAVE ) // bank F9, changed version text to 070412

GAME( 2004, resdnt_2a,  resdnt_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Resident (bootleg, 040513, backdoor)", MACHINE_SUPPORTS_SAVE ) // backdoor 1,5 9,1 5,1 1,5  3,3  3,4
GAME( 2004, resdnt_2b,  resdnt_parent,  igrosoft_gamble, igrosoft_gamble, igrosoft_gamble_state, customl,ROT0,  "bootleg", "Resident (bootleg, 040513, VIDEO GAME-1 SE01 set 1)", MACHINE_SUPPORTS_SAVE ) // custom alteras, modified graphics, changed version text to "VIDEO GAME-1 SE01"
GAME( 2004, resdnt_2c,  resdnt_parent,  igrosoft_gamble, igrosoft_gamble, igrosoft_gamble_state, customl,ROT0,  "bootleg", "Resident (bootleg, 040513, VIDEO GAME-1 SE01 set 2)", MACHINE_SUPPORTS_SAVE ) // custom alteras, modified graphics, changed version text to "VIDEO GAME-1 SE01"
GAME( 2004, resdnt_2d,  resdnt_parent,  igrosoft_gamble, igrosoft_gamble, igrosoft_gamble_state, customl,ROT0,  "bootleg", "Resident (bootleg, 040513, VIDEO GAME-1 SE01 set 3)", MACHINE_SUPPORTS_SAVE ) // custom alteras, modified graphics, many texts changed, changed version text to "VIDEO GAME-1 SE01"
GAME( 2004, resdnt_2e,  resdnt_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Resident (bootleg, 040513, LOTTOGAME (I))", MACHINE_SUPPORTS_SAVE ) // modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
GAME( 2004, resdnt_2f,  resdnt_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Resident (bootleg, 040513, LOTO PROGRAM V-RS2)", MACHINE_SUPPORTS_SAVE ) // modified graphics, many texts changed, changed version text to "LOTO PROGRAM V-RS2"
GAME( 2004, resdnt_2g,  resdnt_parent,  igrosoft_gamble, igrosoft_gamble, igrosoft_gamble_state, customl,ROT0,  "bootleg", "Resident (bootleg, 040513, LOTOS SE01)", MACHINE_SUPPORTS_SAVE ) // custom alteras, modified graphics, many texts changed, changed version text to "LOTOS SE01"

GAME( 2005, islanda,    island_parent,  igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Island (bootleg, 050713, backdoor)", MACHINE_SUPPORTS_SAVE ) // backdoor 1,1 1,3 1,5 1,7  3,3  3,4
GAME( 2005, islandb,    island_parent,  igrosoft_gamble, igrosoft_gamble, igrosoft_gamble_state, customl,ROT0,  "bootleg", "Island (bootleg, 050713, VIDEO GAME-1 OS01)", MACHINE_SUPPORTS_SAVE ) // custom alteras, modified graphics, many texts changed, changed version text to "VIDEO GAME-1 OS01"
GAME( 2005, islandc,    island_parent,  igrosoft_gamble, igrosoft_gamble, igrosoft_gamble_state, customl,ROT0,  "bootleg", "Island (bootleg, 050713, LOTOS OS01)", MACHINE_SUPPORTS_SAVE ) // custom alteras, modified graphics, many texts changed, changed version text to "LOTOS OS01"

GAME( 2006, island2a,   island2_parent, igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Island 2 (bootleg, 060529, banking address hack)", MACHINE_SUPPORTS_SAVE ) // bank F9 (not standart, game not work)
GAME( 2006, island2b,   island2_parent, igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Island 2 (bootleg, 060529, banking address hack, changed version text)", MACHINE_SUPPORTS_SAVE ) // bank F9, changed version text to 070205, skip some start tests
GAME( 2006, island2c,   island2_parent, igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Island 2 (bootleg, 060529, LOTTOGAME (I))", MACHINE_SUPPORTS_SAVE ) // bank F9, modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
GAME( 2006, island2_3a, island2_parent, igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Island 2 (bootleg, 061218, VIDEO GAME-1 OS2-01)", MACHINE_SUPPORTS_SAVE ) // bank F9, modified graphics, changed version text to "VIDEO GAME-1 OS2-01"
GAME( 2006, island2_4a, island2_parent, igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Island 2 (bootleg, 070205, banking address hack)", MACHINE_SUPPORTS_SAVE ) // bank F9

GAME( 2006, pirate2a,   pirate2_parent, igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Pirate 2 (bootleg, 061005, banking address hack set 1)", MACHINE_SUPPORTS_SAVE ) // bank F9
GAME( 2006, pirate2b,   pirate2_parent, igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Pirate 2 (bootleg, 061005, banking address hack set 2)", MACHINE_SUPPORTS_SAVE ) // bank F9, skip raster beam position check
GAME( 2006, pirate2c,   pirate2_parent, igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Pirate 2 (bootleg, 061005, banking address hack, changed version text set 1)", MACHINE_SUPPORTS_SAVE ) // bank F9, changed version text to 070126
GAME( 2006, pirate2d,   pirate2_parent, igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Pirate 2 (bootleg, 061005, banking address hack, changed version text set 2)", MACHINE_SUPPORTS_SAVE ) // bank F9, changed version text to 070126
GAME( 2006, pirate2e,   pirate2_parent, igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Pirate 2 (bootleg, 061005, banking address hack, changed version text set 3)", MACHINE_SUPPORTS_SAVE ) // bank F9, changed version text to 070126, skip some start tests
GAME( 2006, pirate2f,   pirate2_parent, igrosoft_gamble, igrosoft_gamble, igrosoft_gamble_state, customl,ROT0,  "bootleg", "Pirate 2 (bootleg, 061005, VIDEO GAME-1 PR01)", MACHINE_SUPPORTS_SAVE ) // custom alteras, modified graphics, many texts changed, changed version text to "VIDEO GAME-1 PR01"
GAME( 2006, pirate2g,   pirate2_parent, igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Pirate 2 (bootleg, 061005, LOTTOGAME (I))", MACHINE_SUPPORTS_SAVE ) // bank F9, modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
GAME( 2006, pirate2h,   pirate2_parent, igrosoft_gamble, igrosoft_gamble, igrosoft_gamble_state, customl,ROT0,  "bootleg", "Pirate 2 (bootleg, 061005, LOTOS PR01)", MACHINE_SUPPORTS_SAVE ) // custom alteras, modified graphics, many texts changed, changed version text to "LOTOS PR01"
GAME( 2006, pirate2_2a, pirate2_parent, igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Pirate 2 (bootleg, 070126, banking address hack)", MACHINE_SUPPORTS_SAVE ) // bank F9

GAME( 2006, keksa,      keks_parent,    igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Keks (bootleg, 060328, banking address hack)", MACHINE_SUPPORTS_SAVE ) // bank F9
GAME( 2006, keksb,      keks_parent,    igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Keks (bootleg, 060328, backdoor)", MACHINE_SUPPORTS_SAVE ) // backdoor 1,1 1,3 1,5 1,7  3,3  3,4
GAME( 2006, keksc,      keks_parent,    igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Keks (bootleg, 060328, banking address hack, changed version text)", MACHINE_SUPPORTS_SAVE ) // bank F9, changed version text to 070119
GAME( 2006, keks_2a,    keks_parent,    igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Keks (bootleg, 060403, banking address hack)", MACHINE_SUPPORTS_SAVE ) // bank F9
GAME( 2006, keks_2b,    keks_parent,    igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Keks (bootleg, 060403, banking address hack, changed version text)", MACHINE_SUPPORTS_SAVE ) // bank F9, changed version text to 070119
GAME( 2006, keks_2c,    keks_parent,    igrosoft_gamble, igrosoft_gamble, igrosoft_gamble_state, customl,ROT0,  "bootleg", "Keks (bootleg, 060403, VIDEO GAME-1 KS01 set 1)", MACHINE_SUPPORTS_SAVE ) // custom alteras, modified graphics, bank F9, changed version text to "VIDEO GAME-1 KS01"
GAME( 2006, keks_2d,    keks_parent,    igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Keks (bootleg, 060403, VIDEO GAME-1 KS01 set 2)", MACHINE_SUPPORTS_SAVE ) // modified graphics, bank F9, changed version text to "VIDEO GAME-1 KS01" (keks_2c, decoded gfx)
GAME( 2006, keks_2e,    keks_parent,    igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Keks (bootleg, 060403, banking address hack, payout percentage 60)", MACHINE_SUPPORTS_SAVE ) // bank F9, payout percentage 60%
GAME( 2006, keks_2f,    keks_parent,    igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Keks (bootleg, 060403, LOTTOGAME (I))", MACHINE_SUPPORTS_SAVE ) // bank F9, modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
GAME( 2006, keks_2g,    keks_parent,    igrosoft_gamble, igrosoft_gamble, igrosoft_gamble_state, customl,ROT0,  "bootleg", "Keks (bootleg, 060403, LOTOS KS01)", MACHINE_SUPPORTS_SAVE ) // custom alteras, modified graphics, bank F9, many texts changed, changed version text to "LOTOS KS01"
GAME( 2006, keks_3a,    keks_parent,    igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Keks (bootleg, 070119, banking address hack set 1)", MACHINE_SUPPORTS_SAVE ) // bank F9
GAME( 2006, keks_3b,    keks_parent,    igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Keks (bootleg, 070119, banking address hack set 2)", MACHINE_SUPPORTS_SAVE ) // bank F9

GAME( 2007, gnomea,     gnome_parent,   igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0, "bootleg",  "Gnome (bootleg, 070906, banking address hack set 1)", MACHINE_SUPPORTS_SAVE ) // bank F9
GAME( 2007, gnomeb,     gnome_parent,   igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0, "bootleg",  "Gnome (bootleg, 070906, banking address hack set 2)", MACHINE_SUPPORTS_SAVE ) // bank F9
GAME( 2007, gnomec,     gnome_parent,   igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0, "bootleg",  "Gnome (bootleg, 070906, banking address hack set 3)", MACHINE_SUPPORTS_SAVE ) // bank F9
GAME( 2007, gnomed,     gnome_parent,   igrosoft_gamble, igrosoft_gamble, igrosoft_gamble_state, customl,ROT0, "bootleg",  "Gnome (bootleg, 070906, VIDEO GAME-1 GN01)", MACHINE_SUPPORTS_SAVE ) // custom alteras, modified graphics, many texts changed, changed version text to "VIDEO GAME-1 GN01"
GAME( 2007, gnomee,     gnome_parent,   igrosoft_gamble, igrosoft_gamble, igrosoft_gamble_state, customl,ROT0, "bootleg",  "Gnome (bootleg, 070906, LOTOS GN01)", MACHINE_SUPPORTS_SAVE ) // custom alteras, modified graphics, bank F9, many texts changed, changed version text to "LOTOS GN01"
GAME( 2007, gnome_2a,   gnome_parent,   igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0, "bootleg",  "Gnome (bootleg, 071115, banking address hack)", MACHINE_SUPPORTS_SAVE ) // bank F9
GAME( 2007, gnome_3a,   gnome_parent,   igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0, "bootleg",  "Gnome (bootleg, 080303, banking address hack)", MACHINE_SUPPORTS_SAVE ) // bank F9
GAME( 2007, gnome_3b,   gnome_parent,   igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0, "bootleg",  "Gnome (bootleg, 080303, banking address hack, payout percentage 45)", MACHINE_SUPPORTS_SAVE ) // bank F9 payout percentage 45%
GAME( 2007, gnome_3c,   gnome_parent,   igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0, "bootleg",  "Gnome (bootleg, 080303, banking address hack, payout percentage 60)", MACHINE_SUPPORTS_SAVE ) // bank F9 payout percentage 60%
GAME( 2007, gnome_5a,   gnome_parent,   igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0, "bootleg",  "Gnome (bootleg, 090406, banking address hack, payout percentage 70)", MACHINE_SUPPORTS_SAVE ) // bank F9, payout percentage 70%
GAME( 2007, gnome_5b,   gnome_parent,   igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0, "bootleg",  "Gnome (bootleg, 090406, LOTTOGAME (I))", MACHINE_SUPPORTS_SAVE ) // bank F9, modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"

GAME( 2007, sweetl2_2a, sweetl2_parent, igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Sweet Life 2 (bootleg, 080320, banking address hack set 1)", MACHINE_SUPPORTS_SAVE ) // bank F9
GAME( 2007, sweetl2_2b, sweetl2_parent, igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Sweet Life 2 (bootleg, 080320, banking address hack set 2)", MACHINE_SUPPORTS_SAVE ) // bank F9, some fixes
GAME( 2007, sweetl2_2c, sweetl2_parent, igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Sweet Life 2 (bootleg, 080320, VIDEO GAME-1 MD01)", MACHINE_SUPPORTS_SAVE ) // modified graphics, bank F9, changed version text to "VIDEO GAME-1 MD01"
GAME( 2007, sweetl2_2d, sweetl2_parent, igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Sweet Life 2 (bootleg, 080320, LOTTOGAME (I))", MACHINE_SUPPORTS_SAVE ) // bank F9, modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"

GAME( 2008, fcockt2a,   fcockt2_parent, igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Fruit Cocktail 2 (bootleg, 080707, banking address hack)", MACHINE_SUPPORTS_SAVE ) // bank F9
GAME( 2008, fcockt2_4a, fcockt2_parent, igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Fruit Cocktail 2 (bootleg, 081105, banking address hack)", MACHINE_SUPPORTS_SAVE ) // bank F9
GAME( 2008, fcockt2_4b, fcockt2_parent, igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Fruit Cocktail 2 (bootleg, 081105, banking address hack, no credit limit)", MACHINE_SUPPORTS_SAVE ) // bank F9, no credit limit, "MaxVin" signature
GAME( 2008, fcockt2_4c, fcockt2_parent, igrosoft_gamble, igrosoft_gamble, igrosoft_gamble_state, customl,ROT0,  "bootleg", "Fruit Cocktail 2 (bootleg, 081105, VIDEO GAME-1 FR02)", MACHINE_SUPPORTS_SAVE ) // custom alteras, modified graphics, bank F9, many texts changed, changed version text to "VIDEO GAME-1 FR02"
GAME( 2008, fcockt2_4d, fcockt2_parent, igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Fruit Cocktail 2 (bootleg, 081105, banking address hack, payout percentage 70)", MACHINE_SUPPORTS_SAVE ) // bank F9, no credit limit, "MaxVin" signature, payout percentage 70%
GAME( 2008, fcockt2_4e, fcockt2_parent, igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Fruit Cocktail 2 (bootleg, 081105, LOTTOGAME (I))", MACHINE_SUPPORTS_SAVE ) // bank F9, modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
GAME( 2008, fcockt2_4f, fcockt2_parent, igrosoft_gamble, igrosoft_gamble, igrosoft_gamble_state, customl,ROT0,  "bootleg", "Fruit Cocktail 2 (bootleg, 081105, LOTOS FR02)", MACHINE_SUPPORTS_SAVE ) // custom alteras, modified graphics, bank F9, many texts changed, changed version text to "LOTOS FR02"


/* 0x000000 - 0x03ffff Crazy Monkey V03-1110
   0x040000 - 0x07ffff Garage V05-0311
   0x080000 - 0x0bffff Lucky Haunter V04-0825
   0x0c0000 - 0x0fffff Island 2  IS2-060529
   0x100000 - 0x13ffff Fruit Cocktail V05-0118
   0x140000 - 0x17ffff Resident  V04-0513
   0x180000 - 0x1bffff Rock Climber RC040827
   0x1c0000 - 0x1fffff xxxx MENU xxxx  ( Lucky Haunter V04-0825 service mode )
   0x200000 - 0x23ffff Pirate PR060210
   0x240000 - 0x27ffff Sweet Life SL041220
   0x280000 - 0x2bffff Keks KS060403
   0x2c0000 - 0x2fffff xxxx MENU xxxx  ( Lucky Haunter V04-0825 service mode )
   0x300000 - 0x33ffff NOTHING
   0x340000 - 0x37ffff NOTHING
   0x380000 - 0x3bffff NOTHING
   0x3c0000 - 0x3fffff NOTHING
*/
ROM_START( igromult )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "10games.320.bin", 0x0000, 0x400000, CRC(94e5b2e1) SHA1(62544a5b3de02b5f323478dbae19ef2409af2529) )

	ROM_REGION( 0x4000000, "gfx", 0 ) /* There should be 64MB worth of FLASH ROM for the gfx, the menu GFX are unique at least (not in any Igrosoft set) */
	ROM_LOAD( "10games.gfx", 0x000000, 0x4000000, NO_DUMP )
ROM_END

/* 0x000000 - 0x03ffff Crazy Monkey V03-1110
   0x040000 - 0x07ffff Garage V05-0311
   0x080000 - 0x0bffff Lucky Haunter V04-0825
   0x0c0000 - 0x0fffff Island 2  IS2-060529
   0x100000 - 0x13ffff Fruit Cocktail V05-0118
   0x140000 - 0x17ffff Resident  V04-0513
   0x180000 - 0x1bffff Rock Climber RC040827
   0x1c0000 - 0x1fffff Pirate PR060210
   0x200000 - 0x23ffff Sweet Life SL041220
   0x240000 - 0x27ffff Keks KS060403
   0x280000 - 0x2bffff Gnome M GNM-080303
   0x2c0000 - 0x2fffff Multifish 4Game V04-0308
   0x300000 - 0x33ffff Pirate 2 PR2-061005
   0x340000 - 0x37ffff F Cocktail 2 FC2M-081105
   0x380000 - 0x3bffff Sweet Life 2 - SL2M-080320
   0x3c0000 - 0x3fffff  xxxx MENU xxxx  ( Lucky Haunter V04-0825 service mode )
*/

ROM_START( igromula )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "15games.320.bin", 0x0000, 0x400000, CRC(1cfd520d) SHA1(7316b883a13ff98fc7c3ff05aa085f1fb8e0bdcd) )

	ROM_REGION( 0x4000000, "gfx", 0 ) /* There should be 64MB worth of FLASH ROM for the gfx, the menu GFX are unique at least (not in any Igrosoft set) */
	ROM_LOAD( "15games.gfx", 0x000000, 0x4000000, NO_DUMP )
ROM_END

GAME( 2003, igromult,   0,        igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Igrosoft Multigame Bootleg (10 Games)", MACHINE_NOT_WORKING ) // no GFX roms
GAME( 2003, igromula,   igromult, igrosoft_gamble, igrosoft_gamble, driver_device,  0,             ROT0,  "bootleg", "Igrosoft Multigame Bootleg (15 Games)", MACHINE_NOT_WORKING ) // no GFX roms
