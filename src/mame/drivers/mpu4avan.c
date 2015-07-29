// license:BSD-3-Clause
// copyright-holders:David Haywood
/* MPU4 'Avantine' games
  These seem to be 3rd party games for various regions

  split out because there might be hardware differences, and there are an awful lot of them, especially american beauty! ;-)
  I think some might be dual unit setups, some roms are marked top/bottom
*/


#include "emu.h"
#include "includes/mpu4.h"

MACHINE_CONFIG_EXTERN( mod4oki );
INPUT_PORTS_EXTERN( mpu4 );

#define GAME_FLAGS (MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL)



#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent ,mod2    ,mpu4 , mpu4_state,m4default ,ROT0,company,title,GAME_FLAGS )


// robotica - latvia

GAME_CUSTOM( 199?, m4robo,       0,      "rol1a305.bin", 0x0000, 0x010000, CRC(a6ce02e0) SHA1(cfe0229e4ab8a977c171473bb048568c889a5433), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Latvia, set 1)" )
GAME_CUSTOM( 199?, m4robo__1,    m4robo, "rol1b306.bin", 0x0000, 0x010000, CRC(55c83858) SHA1(dba5d3fedf599fd8aec3a9c9d1f515e026cc7e47), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Latvia, set 2)" )
GAME_CUSTOM( 199?, m4robo__2,    m4robo, "rol1b310.bin", 0x0000, 0x010000, CRC(2e17a780) SHA1(d19a5192754ab76447e7f1b5086d867fb6cfd167), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Latvia, set 3)" )
GAME_CUSTOM( 199?, m4robo__3,    m4robo, "rol1a311.bin", 0x0000, 0x010000, CRC(cf09ca0e) SHA1(8380eb5a04885bbf9a5a0fc505a8ed7fa5427c79), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Latvia, set 4)" )
GAME_CUSTOM( 199?, m4robo__4,    m4robo, "rol1b311.bin", 0x0000, 0x010000, CRC(69c4fb5e) SHA1(03e88fdde3cd1085bb820d71d773d843e2f51717), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Latvia, set 5)" )
GAME_CUSTOM( 199?, m4robo__5,    m4robo, "rol1a312.bin", 0x0000, 0x010000, CRC(d8f62159) SHA1(3295bb956bd3cb8bbfc7a438d92e665ac46848b2), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Latvia, set 6)" )
GAME_CUSTOM( 199?, m4robo__a,    m4robo, "rol1b312.bin", 0x0000, 0x010000, CRC(a7f337e0) SHA1(3901e0a76d5591a4bbb6afd9557e8979d55b8766), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Latvia, set 7)" )
GAME_CUSTOM( 199?, m4robo__b,    m4robo, "rol1a314.bin", 0x0000, 0x010000, CRC(d357a48f) SHA1(759b22a9610d4bd6ca2056ff5c6524dde8ec5d9c), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Latvia, set 8)" )
GAME_CUSTOM( 199?, m4robo__c,    m4robo, "rol1b314.bin", 0x0000, 0x010000, CRC(34e0a470) SHA1(9274bd47127905ca53c7e62470dad9b0e8993b0b), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Latvia, set 9)" )
GAME_CUSTOM( 199?, m4robo__d,    m4robo, "rol1a315.bin", 0x0000, 0x010000, CRC(5f00eaf5) SHA1(463c25b649ed12cb126324c3d84070dd3738bec8), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Latvia, set 10)" )
GAME_CUSTOM( 199?, m4robo__e,    m4robo, "rol1b315.bin", 0x0000, 0x010000, CRC(0bfe64fb) SHA1(faae44877c94544ba2c8f32ba7195d120639b186), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Latvia, set 11)" )
GAME_CUSTOM( 199?, m4robo__f,    m4robo, "rol1a316.bin", 0x0000, 0x010000, CRC(ba88e7f5) SHA1(5abd3359ef68b7020b57928995fd72e2997f127b), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Latvia, set 12)" )
GAME_CUSTOM( 199?, m4robo__g,    m4robo, "rol1b316.bin", 0x0000, 0x010000, CRC(52e77e26) SHA1(f656d69a807c60db2f4f95a5820102db7687abc2), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Latvia, set 13)" )
GAME_CUSTOM( 199?, m4robo__h,    m4robo, "rol1a317.bin", 0x0000, 0x010000, CRC(6211818c) SHA1(b9b80b66a4fb93ce90ca91d43b68e36cc7884fa4), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Latvia, set 14)" )
GAME_CUSTOM( 199?, m4robo__i,    m4robo, "rol1b317.bin", 0x0000, 0x010000, CRC(ee243d1b) SHA1(b046bdb300a90e197a2e0c41a939f75a7a07dc05), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Latvia, set 15)" )

// robotica - russia
GAME_CUSTOM( 199?, m4robo__6,    m4robo, "ror1a301.bin", 0x0000, 0x010000, CRC(36ccbd04) SHA1(9dfe04d19ca76fb805880d60f7ef00f38e5e49ee), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Russia, set 1)" )
GAME_CUSTOM( 199?, m4robo__7,    m4robo, "ror1b301.bin", 0x0000, 0x010000, CRC(b91f772a) SHA1(06340815b1cfbfe0df4f8afe7326c351ed4fb910), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Russia, set 2)" )
GAME_CUSTOM( 199?, m4robo__8,    m4robo, "ror1a302.bin", 0x0000, 0x010000, CRC(65ebbde0) SHA1(327af87323cb302c73cb39a07498befcecb0d34a), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Russia, set 3)" )
GAME_CUSTOM( 199?, m4robo__9,    m4robo, "ror1b302.bin", 0x0000, 0x010000, CRC(35b943df) SHA1(b652a96c273e9c8857c4089c17a9f176c8a0f1ae), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Russia, set 4)" )
GAME_CUSTOM( 199?, m4robo__aa,   m4robo, "ror1a304.bin", 0x0000, 0x010000, CRC(1cc05eef) SHA1(949b6093ee340514177291ab1b4074a0ab72c37c), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Russia, set 5)" )
GAME_CUSTOM( 199?, m4robo__ab,   m4robo, "ror1b304.bin", 0x0000, 0x010000, CRC(0315a934) SHA1(2b9c07c104b9883d8a1ce0ca5cb918f5e6f80b1f), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Russia, set 6)" )
GAME_CUSTOM( 199?, m4robo__j,    m4robo, "ror1a315.bin", 0x0000, 0x010000, CRC(ceed653b) SHA1(f9b42d4f64f28ae72d75e99867ce9fda5fea61df), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Russia, set 7)" )
GAME_CUSTOM( 199?, m4robo__k,    m4robo, "ror1b315.bin", 0x0000, 0x010000, CRC(d7aaa742) SHA1(84534aa4d31f1a522cc808566a076a458f7cf56d), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Russia, set 8)" )
GAME_CUSTOM( 199?, m4robo__l,    m4robo, "ror1a314.bin", 0x0000, 0x010000, CRC(83a90df1) SHA1(d515289e717cdb84c4e9b581f44421d8ec3e14e5), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Russia, set 9)" )
GAME_CUSTOM( 199?, m4robo__m,    m4robo, "ror1b314.bin", 0x0000, 0x010000, CRC(26b8f6df) SHA1(43204fdc59a90a462e9df41e0f0e20300b31962a), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Russia, set 10)" )
// robotica - ukraine
GAME_CUSTOM( 199?, m4robo__n,    m4robo, "rou1a214.bin", 0x0000, 0x010000, CRC(be047cea) SHA1(6c3b371392babc6fcacac5b18ea1e9c0ffde811f), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Ukraine, set 1)" )
GAME_CUSTOM( 199?, m4robo__o,    m4robo, "rou1b214.bin", 0x0000, 0x010000, CRC(d0005551) SHA1(497e67b969fc8db4badad116e66bd56ef591fe55), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Ukraine, set 2)" )
GAME_CUSTOM( 199?, m4robo__p,    m4robo, "rou1a215.bin", 0x0000, 0x010000, CRC(8d9c419f) SHA1(a215148c11bc855429c04e427e8e3b923fb1544b), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Ukraine, set 3)" )
GAME_CUSTOM( 199?, m4robo__q,    m4robo, "rou1b215.bin", 0x0000, 0x010000, CRC(a1c54015) SHA1(890f150264e840b4d2b564ad1ee5fd3013721e7e), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Ukraine, set 4)" )
GAME_CUSTOM( 199?, m4robo__r,    m4robo, "rou1a314.bin", 0x0000, 0x010000, CRC(87a0aea9) SHA1(8c163c0da24ac091234d13926301c7ff4bfbf3fa), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Ukraine, set 5)" )
GAME_CUSTOM( 199?, m4robo__s,    m4robo, "rou1b314.bin", 0x0000, 0x010000, CRC(fab1392f) SHA1(5094f1a417b6b4300d26ab12214ef3581e2aad77), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Ukraine, set 6)" )
GAME_CUSTOM( 199?, m4robo__t,    m4robo, "rou1a315.bin", 0x0000, 0x010000, CRC(f321c23e) SHA1(9d8b5ec1cf69b4e2cae7e115edfd1139104ed49b), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Ukraine, set 7)" )
GAME_CUSTOM( 199?, m4robo__u,    m4robo, "rou1b315.bin", 0x0000, 0x010000, CRC(ea660047) SHA1(9ca5dbf9451d92f1d17292c8bca647f2fff11933), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Ukraine, set 8)" )
GAME_CUSTOM( 199?, m4robo__v,    m4robo, "rou1a414.bin", 0x0000, 0x010000, CRC(bb854ced) SHA1(3de424ecc3991ae7bcfae21f191f0e38de922023), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Ukraine, set 9)" )
GAME_CUSTOM( 199?, m4robo__w,    m4robo, "rou1b414.bin", 0x0000, 0x010000, CRC(712e4927) SHA1(3640637d54e5081152f826bea87c0e75ee29ee8e), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Ukraine, set 10)" )
GAME_CUSTOM( 199?, m4robo__x,    m4robo, "rou1a415.bin", 0x0000, 0x010000, CRC(469113c9) SHA1(ee41c7af019f48ffd2fb18c04cb9b235cae02b35), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Ukraine, set 11)" )
GAME_CUSTOM( 199?, m4robo__y,    m4robo, "rou1b415.bin", 0x0000, 0x010000, CRC(abbccf1d) SHA1(92046a477adf90da69557697d7716f00f498e8b1), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Ukraine, set 12)" )
GAME_CUSTOM( 199?, m4robo__z,    m4robo, "rou2a317.bin", 0x0000, 0x010000, CRC(bc1bf926) SHA1(b49d575abbe789bd675070759de3ffc1609f5445), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Ukraine, set 13)" )
GAME_CUSTOM( 199?, m4robo__0,    m4robo, "rou2b317.bin", 0x0000, 0x010000, CRC(8a0d28df) SHA1(5bf0131c361b66fb825f69291dc9f42696a5a376), "Avantime?","Robotica / Dream Machine (Avantime?) (MPU4) (Ukraine, set 14)" )




// uses 3 reels at the bottom, 3 reels at the top + 1 bonus reel at the top
// is it a dual mpu4 board configuration (hence the a/b roms? or are these just alt versions / banks?)
// many of the 'xxx timer' sets use the same configuration driven from a single board tho..

#define M4ABEAUT_EXTRA_ROMS \
	ROM_REGION( 0x100000, "asm", 0 ) \
	ROM_LOAD( "abbcz1.prn", 0x0000, 0x08e0c1, CRC(f333bd68) SHA1(05c30186116b043c0da2ca2b1fc2f1add7bd1574) ) /* this is the source code ASM? */ \
	ROM_REGION( 0x100000, "msm6376", 0 ) /* which sound roms go with which sets?! */ \
	ROM_LOAD( "abs011.bin", 0x0000, 0x080000, CRC(e9364aa0) SHA1(775d94f8665387538d4d8d8ddec38e2ccec2e345) ) \
	ROM_LOAD( "abs012.bin", 0x0000, 0x080000, CRC(72371572) SHA1(4504a91486952bc42e31632131d31cf5414853d6) ) \
	ROM_LOAD( "abs021.bin", 0x0000, 0x080000, CRC(add61ab9) SHA1(1e611bc69f129c18ce460c051f78c726f5efdf68) ) \
	ROM_LOAD( "abs031.bin", 0x0000, 0x080000, CRC(7fe9b7f5) SHA1(4c8fedec056647f7ca9b7da66c020bcdb1e161d0) ) \
	ROM_LOAD( "abs041.bin", 0x0000, 0x080000, CRC(563c7603) SHA1(58883c0d7ce006a5bb15760f22f291f7a01576a4) ) \
	ROM_LOAD( "abs051.bin", 0x0000, 0x080000, CRC(5f073add) SHA1(661889257fbecc4644360aed241967674a42214b) ) \
	ROM_LOAD( "abs041.bin", 0x0000, 0x080000, CRC(563c7603) SHA1(58883c0d7ce006a5bb15760f22f291f7a01576a4) ) \
	ROM_REGION( 0x100000, "gals", 0 ) \
	ROM_LOAD( "abcz.jd", 0x0000, 0x000580, CRC(86fcaa33) SHA1(c0146cd4a289d5dff26e6edc15a0781f437f46ac) ) \
	ROM_LOAD( "abl.jd", 0x0000, 0x000580, CRC(d99e4b62) SHA1(6ae11967f7bc6edd69bd8be20c0c1960e6f3369e) ) \
	ROM_LOAD( "abr.jd", 0x0000, 0x000580, CRC(389c4cea) SHA1(c431c51c47cbd8a7f3bd0497dd7fffccb4a3416f) ) \
	ROM_LOAD( "abu.jd", 0x0000, 0x000580, CRC(95f7892d) SHA1(dc2ab97ff60f3532967119ff6d2dc8d9596caa01) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4ABEAUT_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default ,ROT0,company,title,GAME_FLAGS )

// ab
GAME_CUSTOM( 199?, m4abeaut,       0,          "aba305.bin",   0x0000, 0x010000, CRC(ab70802c) SHA1(0cb59947b0bc177b10c55e02a0c8016620e0c346), "Avantime?","American Beauty (Avantime?) (MPU4) (AB, set 1)" )
GAME_CUSTOM( 199?, m4abeaut_1,     m4abeaut,   "abb305.bin",   0x0000, 0x010000, CRC(eeb3f0e8) SHA1(b4daa4e0817cb37a6f91bf12ba9fbff0f631abd1), "Avantime?","American Beauty (Avantime?) (MPU4) (AB, set 2)" )
GAME_CUSTOM( 199?, m4abeaut_2,     m4abeaut,   "aba306.bin",   0x0000, 0x010000, CRC(6034f4eb) SHA1(8e5442ad275d48cda8327f1b86c6842807ae33b8), "Avantime?","American Beauty (Avantime?) (MPU4) (AB, set 3)" )
GAME_CUSTOM( 199?, m4abeaut_3,     m4abeaut,   "abb306.bin",   0x0000, 0x010000, CRC(d11f8a09) SHA1(8a391e7fb7e0567b1971ec118d85739910e32c33), "Avantime?","American Beauty (Avantime?) (MPU4) (AB, set 4)" )
GAME_CUSTOM( 199?, m4abeaut_4,     m4abeaut,   "aba308.bin",   0x0000, 0x010000, CRC(3aef7d45) SHA1(e11cb0175bd82c208839c8172ffee2639727bf15), "Avantime?","American Beauty (Avantime?) (MPU4) (AB, set 5)" )
GAME_CUSTOM( 199?, m4abeaut_5,     m4abeaut,   "abb308.bin",   0x0000, 0x010000, CRC(fac4099b) SHA1(643ef922c6a5abc5ac679d31a04ffd5a0ac30c61), "Avantime?","American Beauty (Avantime?) (MPU4) (AB, set 6)" )
GAME_CUSTOM( 199?, m4abeaut_6,     m4abeaut,   "aba309.bin",   0x0000, 0x010000, CRC(87204db5) SHA1(be4cc1de841e4c8581a649c86aefafaa70a28332), "Avantime?","American Beauty (Avantime?) (MPU4) (AB, set 7)" )
GAME_CUSTOM( 199?, m4abeaut_7,     m4abeaut,   "abb309.bin",   0x0000, 0x010000, CRC(dbc126af) SHA1(1190d57228918bcfa481e86d79759854c47945a6), "Avantime?","American Beauty (Avantime?) (MPU4) (AB, set 8)" )
GAME_CUSTOM( 199?, m4abeaut_8,     m4abeaut,   "ab1.bin",      0x0000, 0x010000, CRC(f4c93368) SHA1(fc57947fdc98bf544de601814e93c098667bcae3), "Avantime?","American Beauty (Avantime?) (MPU4) (AB, set 9)" ) // unpaired
GAME_CUSTOM( 199?, m4abeaut_9,     m4abeaut,   "ab303.bin",    0x0000, 0x010000, CRC(493a6de5) SHA1(6bde7f2cec4a94132cc3d4f902626c66c8858276), "Avantime?","American Beauty (Avantime?) (MPU4) (AB, set 10)" ) // unpaired
// am1 - new - ukraine
GAME_CUSTOM( 199?, m4abeaut_u1,    m4abeaut,   "abu1a214.bin", 0x0000, 0x010000, CRC(20456dce) SHA1(29fac37f0ff90a01f1d754ef953d489eadb478d8), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 1)" )
GAME_CUSTOM( 199?, m4abeaut_u2,    m4abeaut,   "abu1b214.bin", 0x0000, 0x010000, CRC(4f4c6fd3) SHA1(3ce07496ff6d6fccabc41d62477eb0a30ed7808b), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 2)" )
GAME_CUSTOM( 199?, m4abeaut_u3,    m4abeaut,   "abu1a215.bin", 0x0000, 0x010000, CRC(702b9cb6) SHA1(13728154c44e6acc97555dc03bd47b4ba8619739), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 3)" )
GAME_CUSTOM( 199?, m4abeaut_u4,    m4abeaut,   "abu1b215.bin", 0x0000, 0x010000, CRC(5722255c) SHA1(12bcf680b5ea994239fe92ee73103484e8385383), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 4)" )
GAME_CUSTOM( 199?, m4abeaut_u5,    m4abeaut,   "abu1a216.bin", 0x0000, 0x010000, CRC(1a430dfd) SHA1(e3f7cd15c8fbb42f40c5b473d07eac4b4a919349), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 5)" )
GAME_CUSTOM( 199?, m4abeaut_u6,    m4abeaut,   "abu1b216.bin", 0x0000, 0x010000, CRC(2edee140) SHA1(6a0472debfe93ca7c0c8f22945cdc14b28f7a18d), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 6)" )
GAME_CUSTOM( 199?, m4abeaut_u7,    m4abeaut,   "abu1a217.bin", 0x0000, 0x010000, CRC(7f372d59) SHA1(874eca687f6915ae3d777b91e1925da11c0888ef), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 7)" )
GAME_CUSTOM( 199?, m4abeaut_u8,    m4abeaut,   "abu1b217.bin", 0x0000, 0x010000, CRC(d0eb875a) SHA1(46234883d01079e71f952b69271d62607968cbfa), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 8)" )
GAME_CUSTOM( 199?, m4abeaut_u9,    m4abeaut,   "abu1a219.bin", 0x0000, 0x010000, CRC(54a67982) SHA1(72e7309d0782bad8edc19dc1b793d2961b8adc02), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 9)" )
GAME_CUSTOM( 199?, m4abeaut_u10,   m4abeaut,   "abu1b219.bin", 0x0000, 0x010000, CRC(b8dd3b7f) SHA1(ecadf9834e59dcff12a9dc4985cc362d750c301c), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 10)" )
GAME_CUSTOM( 199?, m4abeaut_u11,   m4abeaut,   "abu1a221.bin", 0x0000, 0x010000, CRC(c95f25a1) SHA1(3cfc57507aacb8a0f29aeb5b3028b356daf8d770), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 11)" )
GAME_CUSTOM( 199?, m4abeaut_u12,   m4abeaut,   "abu1b221.bin", 0x0000, 0x010000, CRC(ddd4afbe) SHA1(95a407cd9e544bbaecd41a270f9c017904740bc3), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 12)" )
GAME_CUSTOM( 199?, m4abeaut_u13,   m4abeaut,   "abu1a222.bin", 0x0000, 0x010000, CRC(b88e349c) SHA1(1459fd8d0a2f98d919b688d7ea61fc679db4e4d5), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 13)" )
GAME_CUSTOM( 199?, m4abeaut_u14,   m4abeaut,   "abu1b222.bin", 0x0000, 0x010000, CRC(3879a348) SHA1(618a1c49c3cfec8846e83d9b1655a6c283eb7fda), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 14)" )
GAME_CUSTOM( 199?, m4abeaut_u15,   m4abeaut,   "abu1a224.bin", 0x0000, 0x010000, CRC(d4668aee) SHA1(d24cf0e22c1162cd92bc3a2bc53035690220843a), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 15)" )
GAME_CUSTOM( 199?, m4abeaut_u16,   m4abeaut,   "abu1b224.bin", 0x0000, 0x010000, CRC(d55bddda) SHA1(424d8832d1df39bb1250ba506226f969ec99e450), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 16)" )
GAME_CUSTOM( 199?, m4abeaut_u17,   m4abeaut,   "abu1a225.bin", 0x0000, 0x010000, CRC(b4904ccf) SHA1(1aac8a0e749946df74c6a9a5f2a645b468098b54), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 17)" )
GAME_CUSTOM( 199?, m4abeaut_u18,   m4abeaut,   "abu1b225.bin", 0x0000, 0x010000, CRC(541f51f4) SHA1(2ade2998201c65e0723dacf106c95375f807bcc1), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 18)" )
GAME_CUSTOM( 199?, m4abeaut_u19,   m4abeaut,   "abu1a233.bin", 0x0000, 0x010000, CRC(437a4466) SHA1(bd59705f33df39da28ab6f447dd4d99bc5c05efe), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 19)" )
GAME_CUSTOM( 199?, m4abeaut_u20,   m4abeaut,   "abu1b233.bin", 0x0000, 0x010000, CRC(ca42eaae) SHA1(a3d20d978a6d08ef6da6467678cd7afb7c1a5b3d), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 20)" )
GAME_CUSTOM( 199?, m4abeaut_u21,   m4abeaut,   "abu1a234.bin", 0x0000, 0x010000, CRC(46ef47fc) SHA1(ef07199c69239604c72bc8192bf3a92b055282c7), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 21)" )
GAME_CUSTOM( 199?, m4abeaut_u22,   m4abeaut,   "abu1b234.bin", 0x0000, 0x010000, CRC(12c627ab) SHA1(d0a47743b46a982e8c2e969c2131fa6f1132fa8c), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 22)" )
GAME_CUSTOM( 199?, m4abeaut_u23,   m4abeaut,   "abu1a235.bin", 0x0000, 0x010000, CRC(a206f7dc) SHA1(cf1c726302da28d2bc101828448adc8d96f50021), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 23)" )
GAME_CUSTOM( 199?, m4abeaut_u24,   m4abeaut,   "abu1b235.bin", 0x0000, 0x010000, CRC(dce59ddb) SHA1(778e7030a7e7e8b2eeb28f9e88edfbf350abd955), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 24)" )
GAME_CUSTOM( 199?, m4abeaut_u25,   m4abeaut,   "abu1a313.bin", 0x0000, 0x010000, CRC(2961d917) SHA1(7396f7dd50e7d6c94a64bad694f752ef8e80d5f8), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 25)" )
GAME_CUSTOM( 199?, m4abeaut_u26,   m4abeaut,   "abu1b313.bin", 0x0000, 0x010000, CRC(fb7a721e) SHA1(6dc1320c079dc1d7677516e57a3cb0defa75056e), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 26)" )
GAME_CUSTOM( 199?, m4abeaut_u27,   m4abeaut,   "abu1a315.bin", 0x0000, 0x010000, CRC(42e45fea) SHA1(154119329a0ca4698492990eb36e1633449e270a), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 27)" )
GAME_CUSTOM( 199?, m4abeaut_u28,   m4abeaut,   "abu1b315.bin", 0x0000, 0x010000, CRC(175f3a6b) SHA1(9fcd9c5784eb4f2f51ed33812909daa2ae36b31b), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 28)" )
GAME_CUSTOM( 199?, m4abeaut_u29,   m4abeaut,   "abu1a316.bin", 0x0000, 0x010000, CRC(d2fa634d) SHA1(0fb2529823bbab20075f3b246a54f5d23d74ee17), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 29)" )
GAME_CUSTOM( 199?, m4abeaut_u30,   m4abeaut,   "abu1b316.bin", 0x0000, 0x010000, CRC(fe8d6ac8) SHA1(28a927817b1e9c8e69e71e34955bb2d71da6abc2), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 30)" )
GAME_CUSTOM( 199?, m4abeaut_u31,   m4abeaut,   "abu1a317.bin", 0x0000, 0x010000, CRC(37cb227a) SHA1(5af003102210cf8e21564da6fa2afe1dc197f2bd), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 31)" )
GAME_CUSTOM( 199?, m4abeaut_u32,   m4abeaut,   "abu1b317.bin", 0x0000, 0x010000, CRC(d29f0aa2) SHA1(e518523a32c6f5711f39a20f1866e6246e1f39e4), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 32)" )
GAME_CUSTOM( 199?, m4abeaut_u33,   m4abeaut,   "abu1a319.bin", 0x0000, 0x010000, CRC(f9e17cdd) SHA1(5a89b2b5b42c8d15fbb2efd59ce808642935e275), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 33)" )
GAME_CUSTOM( 199?, m4abeaut_u34,   m4abeaut,   "abu1b319.bin", 0x0000, 0x010000, CRC(8f510581) SHA1(7d4d8514df7a2bd7b415e33856d7b61c875b6041), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 34)" )
GAME_CUSTOM( 199?, m4abeaut_u35,   m4abeaut,   "abu1a321.bin", 0x0000, 0x010000, CRC(77eaf769) SHA1(9b32504f16906d1121ade88e926fcc00bf5976a6), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 35)" )
GAME_CUSTOM( 199?, m4abeaut_u36,   m4abeaut,   "abu1b321.bin", 0x0000, 0x010000, CRC(36fae9c1) SHA1(63a2395fced3268ecbe9b95118c8d07a411042f6), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 36)" )
GAME_CUSTOM( 199?, m4abeaut_u37,   m4abeaut,   "abu1a322.bin", 0x0000, 0x010000, CRC(c1d0f65c) SHA1(f4272be41f60d664490477cdddc837efbb5da573), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 37)" )
GAME_CUSTOM( 199?, m4abeaut_u38,   m4abeaut,   "abu1b322.bin", 0x0000, 0x010000, CRC(86007ee4) SHA1(b49d1f46dd73b1ee5811b723dd12c9b7c5def6db), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 38)" )
GAME_CUSTOM( 199?, m4abeaut_u39,   m4abeaut,   "abu1a325.bin", 0x0000, 0x010000, CRC(1c386b75) SHA1(a286dc423534ad9e32da131868483419a8bf313d), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 39)" )
GAME_CUSTOM( 199?, m4abeaut_u40,   m4abeaut,   "abu1b325.bin", 0x0000, 0x010000, CRC(1f2577ec) SHA1(b11cdaedb1896ecaa1655a5b8cd93615a64fcf55), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 40)" )
GAME_CUSTOM( 199?, m4abeaut_u41,   m4abeaut,   "abu1a335.bin", 0x0000, 0x010000, CRC(732bcb35) SHA1(d044f83c294bb639cbbb963ec7c40efee86c7ed6), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 41)" )
GAME_CUSTOM( 199?, m4abeaut_u42,   m4abeaut,   "abu1b335.bin", 0x0000, 0x010000, CRC(725e82db) SHA1(78eba2de7474509e73cd15a1e1ff648d00f7c14d), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 42)" )
GAME_CUSTOM( 199?, m4abeaut_u43,   m4abeaut,   "abu1a424.bin", 0x0000, 0x010000, CRC(265809eb) SHA1(9752ffa958878df2a0d44da5e5682320d0ec06ae), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 43)" )
GAME_CUSTOM( 199?, m4abeaut_u44,   m4abeaut,   "abu1b424.bin", 0x0000, 0x010000, CRC(0b48ddbc) SHA1(7ebd5cd91da859538ab4a7922df50df51a74c545), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 44)" )
GAME_CUSTOM( 199?, m4abeaut_u45,   m4abeaut,   "abu1a425.bin", 0x0000, 0x010000, CRC(9e21fccb) SHA1(e5ac645107b8e5d58c4a3e2b09049cb478547792), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 45)" )
GAME_CUSTOM( 199?, m4abeaut_u46,   m4abeaut,   "abu1b425.bin", 0x0000, 0x010000, CRC(18c5da7a) SHA1(8a496659b6cf7762afe2dfd691d1d66fd99ecbf2), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 46)" )
GAME_CUSTOM( 199?, m4abeaut_u47,   m4abeaut,   "abu1a435.bin", 0x0000, 0x010000, CRC(318841c3) SHA1(0057d8730e020816313288e87172671cfac637aa), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 47)" )
GAME_CUSTOM( 199?, m4abeaut_u48,   m4abeaut,   "abu1b435.bin", 0x0000, 0x010000, CRC(f99413e3) SHA1(caf168b5504352f8accb831326bf94b7141f5b76), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU1, Ukraine, set 48)" )
// abu
GAME_CUSTOM( 199?, m4abeaut_u49,   m4abeaut,   "abu2a328.bin", 0x0000, 0x010000, CRC(52f787d3) SHA1(60fb468c6db705f97702bfbac69c70435dea23b2), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU2, Ukraine, set 1)" )
GAME_CUSTOM( 199?, m4abeaut_u50,   m4abeaut,   "abu2b328.bin", 0x0000, 0x010000, CRC(590beca7) SHA1(40e7ace0a2bb27ce617d039f91e39030881b4b10), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU2, Ukraine, set 2)" )
GAME_CUSTOM( 199?, m4abeaut_u51,   m4abeaut,   "abu2a335.bin", 0x0000, 0x010000, CRC(94177b51) SHA1(5e4e6dae888c8212a05064d754260cc923e0a733), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU2, Ukraine, set 3)" )
GAME_CUSTOM( 199?, m4abeaut_u52,   m4abeaut,   "abu2b335.bin", 0x0000, 0x010000, CRC(d1e2967c) SHA1(df567d328f8f9f9f5ebbfc99344641198aa0d7f8), "Avantime?","American Beauty (Avantime?) (MPU4) (ABU2, Ukraine, set 4)" )
// a2u2
GAME_CUSTOM( 199?, m4abeaut_u53,   m4abeaut,   "a2u2a310.bin", 0x0000, 0x010000, CRC(af3725e3) SHA1(7d36391d1f419bf7df131de12ce5e04e166a6ead), "Avantime?","American Beauty (Avantime?) (MPU4) (A2U2, Ukraine, set 1)" )
GAME_CUSTOM( 199?, m4abeaut_u54,   m4abeaut,   "a2u2b310.bin", 0x0000, 0x010000, CRC(b74947ca) SHA1(c1b668320bf22df773a00d5be1904d9bce9100a1), "Avantime?","American Beauty (Avantime?) (MPU4) (A2U2, Ukraine, set 2)" )
GAME_CUSTOM( 199?, m4abeaut_u55,   m4abeaut,   "a2u2a315.bin", 0x0000, 0x010000, CRC(31801bdf) SHA1(dff63e7838f4686fdf65503d8cf8746038d4a26e), "Avantime?","American Beauty (Avantime?) (MPU4) (A2U2, Ukraine, set 3)" )
GAME_CUSTOM( 199?, m4abeaut_u56,   m4abeaut,   "a2u2b315.bin", 0x0000, 0x010000, CRC(31aaa46e) SHA1(8026a5908ce2abbd4edc3537a618f700801cf78f), "Avantime?","American Beauty (Avantime?) (MPU4) (A2U2, Ukraine, set 4)" )
GAME_CUSTOM( 199?, m4abeaut_u57,   m4abeaut,   "a2u2a316.bin", 0x0000, 0x010000, CRC(0cb892dc) SHA1(7b7ad3515d171c20ab6102f0a439d5aac5ed3b4f), "Avantime?","American Beauty (Avantime?) (MPU4) (A2U2, Ukraine, set 5)" )
GAME_CUSTOM( 199?, m4abeaut_u58,   m4abeaut,   "a2u2b316.bin", 0x0000, 0x010000, CRC(ed2cedfa) SHA1(4c153c2f30c4c5821fcb378ffc9340452b041fe1), "Avantime?","American Beauty (Avantime?) (MPU4) (A2U2, Ukraine, set 6)" )
// a2u3
GAME_CUSTOM( 199?, m4abeaut_u59,   m4abeaut,   "a2u3a319.bin", 0x0000, 0x010000, CRC(586fb90e) SHA1(bd2c87593de2fd25eeb3a8c6c7576ae1bf80e4ad), "Avantime?","American Beauty (Avantime?) (MPU4) (A2U3, Ukraine, set 1)" )
GAME_CUSTOM( 199?, m4abeaut_u60,   m4abeaut,   "a2u3b319.bin", 0x0000, 0x010000, CRC(90aa2af2) SHA1(d4a99e817e1de8058a5f17ba758f63d1fa9535fe), "Avantime?","American Beauty (Avantime?) (MPU4) (A2U3, Ukraine, set 2)" )
// a3u2
GAME_CUSTOM( 199?, m4abeaut_u61,   m4abeaut,   "a3u2a313.bin", 0x0000, 0x010000, CRC(708beb37) SHA1(6102a9211fba876c98c1198a7125cf415a9d11fc), "Avantime?","American Beauty (Avantime?) (MPU4) (A3U2, Ukraine, set 1)" )
GAME_CUSTOM( 199?, m4abeaut_u62,   m4abeaut,   "a3u2b313.bin", 0x0000, 0x010000, CRC(9255447c) SHA1(731980e074b5409af3ee2dd4ecd9548fda8f3382), "Avantime?","American Beauty (Avantime?) (MPU4) (A3U2, Ukraine, set 2)" )
GAME_CUSTOM( 199?, m4abeaut_u63,   m4abeaut,   "a3u2a315.bin", 0x0000, 0x010000, CRC(d49177ab) SHA1(759851624172e792427d517ddef5e54ca36228ee), "Avantime?","American Beauty (Avantime?) (MPU4) (A3U2, Ukraine, set 3)" )
GAME_CUSTOM( 199?, m4abeaut_u64,   m4abeaut,   "a3u2b315.bin", 0x0000, 0x010000, CRC(73851092) SHA1(664f8edaa5bc12b94e645f0727247a21eb1b4d81), "Avantime?","American Beauty (Avantime?) (MPU4) (A3U2, Ukraine, set 4)" )
// am1 - new - russia
GAME_CUSTOM( 199?, m4abeaut_r1,    m4abeaut,   "abr1a313.bin", 0x0000, 0x010000, CRC(60a9175a) SHA1(8365712a12c9bb96b310428ffae85451798ca37f), "Avantime?","American Beauty (Avantime?) (MPU4) (ABR1, Russia, set 1)" )
GAME_CUSTOM( 199?, m4abeaut_r2,    m4abeaut,   "abr1b313.bin", 0x0000, 0x010000, CRC(c55389cc) SHA1(028f08b8c8d927dc6c5300af7bdce293644e1b3c), "Avantime?","American Beauty (Avantime?) (MPU4) (ABR1, Russia, set 2)" )
GAME_CUSTOM( 199?, m4abeaut_r3,    m4abeaut,   "abr1a316.bin", 0x0000, 0x010000, CRC(688e011b) SHA1(c52e8b298dc3a123cb8e54feac0d6eeb1dadac00), "Avantime?","American Beauty (Avantime?) (MPU4) (ABR1, Russia, set 3)" )
GAME_CUSTOM( 199?, m4abeaut_r4,    m4abeaut,   "abr1b316.bin", 0x0000, 0x010000, CRC(15265baf) SHA1(8aee3b20cb93d5fb5cb07110ed3a7670897ea877), "Avantime?","American Beauty (Avantime?) (MPU4) (ABR1, Russia, set 4)" )
GAME_CUSTOM( 199?, m4abeaut_r5,    m4abeaut,   "abr1a317.bin", 0x0000, 0x010000, CRC(0c251f11) SHA1(9d690836d87a5fd9aff82b4bf860e0fc519c5553), "Avantime?","American Beauty (Avantime?) (MPU4) (ABR1, Russia, set 5)" )
GAME_CUSTOM( 199?, m4abeaut_r6,    m4abeaut,   "abr1b317.bin", 0x0000, 0x010000, CRC(d7745260) SHA1(c38d10e346e80d609aef8c03940ab43ee3a891a0), "Avantime?","American Beauty (Avantime?) (MPU4) (ABR1, Russia, set 6)" )
GAME_CUSTOM( 199?, m4abeaut_r7,    m4abeaut,   "abr1a319.bin", 0x0000, 0x010000, CRC(30edf15d) SHA1(58ff8943af96f6c4d639a370c5b498a8c37ecee8), "Avantime?","American Beauty (Avantime?) (MPU4) (ABR1, Russia, set 7)" )
GAME_CUSTOM( 199?, m4abeaut_r8,    m4abeaut,   "abr1b319.bin", 0x0000, 0x010000, CRC(465d8801) SHA1(8b089b252ce6a8aeee4f109fbc343fc1314a60ff), "Avantime?","American Beauty (Avantime?) (MPU4) (ABR1, Russia, set 8)" )
GAME_CUSTOM( 199?, m4abeaut_r9,    m4abeaut,   "abr1a321.bin", 0x0000, 0x010000, CRC(63f89df8) SHA1(e1b7f87b84dd4de1df2487d1c2cb9f2787c4899e), "Avantime?","American Beauty (Avantime?) (MPU4) (ABR1, Russia, set 9)" )
GAME_CUSTOM( 199?, m4abeaut_r10,   m4abeaut,   "abr1b321.bin", 0x0000, 0x010000, CRC(2a1c3ea6) SHA1(dac86d12b3339ac2b48981aea0407448d87287c0), "Avantime?","American Beauty (Avantime?) (MPU4) (ABR1, Russia, set 10)" )
GAME_CUSTOM( 199?, m4abeaut_r11,   m4abeaut,   "abr1a325.bin", 0x0000, 0x010000, CRC(a7da1ce9) SHA1(557d387992796bf39310b9c6b71d9b2ac736f38d), "Avantime?","American Beauty (Avantime?) (MPU4) (ABR1, Russia, set 11)" )
GAME_CUSTOM( 199?, m4abeaut_r12,   m4abeaut,   "abr1b325.bin", 0x0000, 0x010000, CRC(c4006f90) SHA1(55826a6408572b894331b577d088a74f1f9bba2c), "Avantime?","American Beauty (Avantime?) (MPU4) (ABR1, Russia, set 12)" )
GAME_CUSTOM( 199?, m4abeaut_r13,   m4abeaut,   "abr1a335.bin", 0x0000, 0x010000, CRC(a696c9d0) SHA1(8a8af53f129f48ffeef5e4f325e378c9bab40537), "Avantime?","American Beauty (Avantime?) (MPU4) (ABR1, Russia, set 13)" )
GAME_CUSTOM( 199?, m4abeaut_r14,   m4abeaut,   "abr1b335.bin", 0x0000, 0x010000, CRC(79718339) SHA1(4d90362e85b51a9253ecf16447693887f2432ad6), "Avantime?","American Beauty (Avantime?) (MPU4) (ABR1, Russia, set 14)" )
// abl0 - new - latvia
GAME_CUSTOM( 199?, m4abeaut_l1,    m4abeaut,   "abl0a312.bin", 0x0000, 0x010000, CRC(c35780bb) SHA1(8bbbe0ec5e96eec60b91de84d320361d1d235300), "Avantime?","American Beauty (Avantime?) (MPU4) (ABL0, Latvia, set 1)" )
GAME_CUSTOM( 199?, m4abeaut_l2,    m4abeaut,   "abl0b312.bin", 0x0000, 0x010000, CRC(b7fb62b6) SHA1(c19bfda992a4e2da54942353f8c359caaa8a3cf2), "Avantime?","American Beauty (Avantime?) (MPU4) (ABL0, Latvia, set 2)" )
GAME_CUSTOM( 199?, m4abeaut_l3,    m4abeaut,   "abl0a313.bin", 0x0000, 0x010000, CRC(19230b2a) SHA1(2c496796330f3790bad0bad8f55bd20ff50e3a0c), "Avantime?","American Beauty (Avantime?) (MPU4) (ABL0, Latvia, set 3)" )
GAME_CUSTOM( 199?, m4abeaut_l4,    m4abeaut,   "abl0b313.bin", 0x0000, 0x010000, CRC(fa08f549) SHA1(c4fa9799e8d41ce939139d1c72638720fabcc796), "Avantime?","American Beauty (Avantime?) (MPU4) (ABL0, Latvia, set 4)" )
GAME_CUSTOM( 199?, m4abeaut_l5,    m4abeaut,   "abl0a317.bin", 0x0000, 0x010000, CRC(dfbc4386) SHA1(b6dd1697b4422f1c37d284c17916ddf68e2011da), "Avantime?","American Beauty (Avantime?) (MPU4) (ABL0, Latvia, set 5)" )
GAME_CUSTOM( 199?, m4abeaut_l6,    m4abeaut,   "abl0b317.bin", 0x0000, 0x010000, CRC(cf2899ec) SHA1(36eb53b383575659ef66fde70098e07911cce793), "Avantime?","American Beauty (Avantime?) (MPU4) (ABL0, Latvia, set 6)" )
GAME_CUSTOM( 199?, m4abeaut_l7,    m4abeaut,   "abl0a325.bin", 0x0000, 0x010000, CRC(e33dd097) SHA1(178fea0050386b55cfcc6ce4ef7e1d118a2943b3), "Avantime?","American Beauty (Avantime?) (MPU4) (ABL0, Latvia, set 7)" )
GAME_CUSTOM( 199?, m4abeaut_l8,    m4abeaut,   "abl0b325.bin", 0x0000, 0x010000, CRC(3d5d4236) SHA1(e93ef0bd14aebda56d87b701bb4fbc22cedcff5b), "Avantime?","American Beauty (Avantime?) (MPU4) (ABL0, Latvia, set 8)" )
GAME_CUSTOM( 199?, m4abeaut_l9,    m4abeaut,   "abl0a331.bin", 0x0000, 0x010000, CRC(4024009d) SHA1(2fb4577fb9c5b4b74f5381ff0feba629c584475f), "Avantime?","American Beauty (Avantime?) (MPU4) (ABL0, Latvia, set 9)" )
GAME_CUSTOM( 199?, m4abeaut_l10,   m4abeaut,   "abl0b331.bin", 0x0000, 0x010000, CRC(154a8c4d) SHA1(3076907a3402a1a8d20498435813f30c75643438), "Avantime?","American Beauty (Avantime?) (MPU4) (ABL0, Latvia, set 10)" )
GAME_CUSTOM( 199?, m4abeaut_l11,   m4abeaut,   "abl0a335.bin", 0x0000, 0x010000, CRC(2d5f1ec2) SHA1(63e75ce4e01f551d35483b27b95524b417a0df2f), "Avantime?","American Beauty (Avantime?) (MPU4) (ABL0, Latvia, set 11)" )
GAME_CUSTOM( 199?, m4abeaut_l12,   m4abeaut,   "abl0b335.bin", 0x0000, 0x010000, CRC(3c3d0177) SHA1(bbd86767a5ef70a7a795f12b111bf64b653ed53c), "Avantime?","American Beauty (Avantime?) (MPU4) (ABL0, Latvia, set 12)" )
// a2l0
GAME_CUSTOM( 199?, m4abeaut_l13,   m4abeaut,   "a2l0a301.bin", 0x0000, 0x010000, CRC(ae30bb11) SHA1(bfaaf232e6e409cf6768be262de9fdaf08d67948), "Avantime?","American Beauty (Avantime?) (MPU4) (A2L0, Latvia, set 1)" )
GAME_CUSTOM( 199?, m4abeaut_l14,   m4abeaut,   "a2l0b301.bin", 0x0000, 0x010000, CRC(2f92ee8e) SHA1(382f8dc0abbe43e9c9aec011a5f66f3bf3d22b9d), "Avantime?","American Beauty (Avantime?) (MPU4) (A2L0, Latvia, set 2)" )
GAME_CUSTOM( 199?, m4abeaut_l15,   m4abeaut,   "a2l0a302.bin", 0x0000, 0x010000, CRC(26f9321e) SHA1(94b9fe9059edef69ce180d8d09087f2f71adb661), "Avantime?","American Beauty (Avantime?) (MPU4) (A2L0, Latvia, set 3)" )
GAME_CUSTOM( 199?, m4abeaut_l16,   m4abeaut,   "a2l0b302.bin", 0x0000, 0x010000, CRC(1418f724) SHA1(30f673c47066f23d2d6719cac22a557f350caab2), "Avantime?","American Beauty (Avantime?) (MPU4) (A2L0, Latvia, set 4)" )
GAME_CUSTOM( 199?, m4abeaut_l17,   m4abeaut,   "a2l0a304.bin", 0x0000, 0x010000, CRC(5d4844ba) SHA1(cdc1f8fe886da7adecc3d3484e760fc1bff5eaf7), "Avantime?","American Beauty (Avantime?) (MPU4) (A2L0, Latvia, set 5)" )
GAME_CUSTOM( 199?, m4abeaut_l18,   m4abeaut,   "a2l0b304.bin", 0x0000, 0x010000, CRC(fdc7e647) SHA1(7a8f09005585571586d7d99415113cbbade403ea), "Avantime?","American Beauty (Avantime?) (MPU4) (A2L0, Latvia, set 6)" )
GAME_CUSTOM( 199?, m4abeaut_l19,   m4abeaut,   "a2l0a305.bin", 0x0000, 0x010000, CRC(91bce727) SHA1(e5fee8376aea2f5b8adc2cf53e445b48ed41cf77), "Avantime?","American Beauty (Avantime?) (MPU4) (A2L0, Latvia, set 7)" )
GAME_CUSTOM( 199?, m4abeaut_l20,   m4abeaut,   "a2l0b305.bin", 0x0000, 0x010000, CRC(1b1065e3) SHA1(8ba3860cdf24ece523b3259df7c44544f841682d), "Avantime?","American Beauty (Avantime?) (MPU4) (A2L0, Latvia, set 8)" )
GAME_CUSTOM( 199?, m4abeaut_l21,   m4abeaut,   "a2l0a306.bin", 0x0000, 0x010000, CRC(79e2902c) SHA1(507507abef4efb5c4b0e9714e898d7a2afb8ebc2), "Avantime?","American Beauty (Avantime?) (MPU4) (A2L0, Latvia, set 9)" )
GAME_CUSTOM( 199?, m4abeaut_l22,   m4abeaut,   "a2l0b306.bin", 0x0000, 0x010000, CRC(507287ea) SHA1(c6db30e7dffcc42f2615c00edf4dee871e763dbc), "Avantime?","American Beauty (Avantime?) (MPU4) (A2L0, Latvia, set 10)" )
GAME_CUSTOM( 199?, m4abeaut_l23,   m4abeaut,   "a2l0a307.bin", 0x0000, 0x010000, CRC(c1e0788f) SHA1(29ac2631bdb603da7ec9c81213efc06f0a920e62), "Avantime?","American Beauty (Avantime?) (MPU4) (A2L0, Latvia, set 11)" )
GAME_CUSTOM( 199?, m4abeaut_l24,   m4abeaut,   "a2l0b307.bin", 0x0000, 0x010000, CRC(997ee9f9) SHA1(3e4dda9c1419e8a0878b92e8693e6bdd5c812dd5), "Avantime?","American Beauty (Avantime?) (MPU4) (A2L0, Latvia, set 12)" )
GAME_CUSTOM( 199?, m4abeaut_l25,   m4abeaut,   "a2l0a309.bin", 0x0000, 0x010000, CRC(92be4a73) SHA1(8558269381f5c82c489b99000378b7f182f8d573), "Avantime?","American Beauty (Avantime?) (MPU4) (A2L0, Latvia, set 13)" )
GAME_CUSTOM( 199?, m4abeaut_l26,   m4abeaut,   "a2l0b309.bin", 0x0000, 0x010000, CRC(7167dc23) SHA1(c9c366fc8a6d486ba02af165e32579a6cc23ca95), "Avantime?","American Beauty (Avantime?) (MPU4) (A2L0, Latvia, set 14)" )
GAME_CUSTOM( 199?, m4abeaut_l27,   m4abeaut,   "a2l0a310.bin", 0x0000, 0x010000, CRC(2471e6e7) SHA1(7b3a17eab83181260ee04d735f65cdf7686fdbaa), "Avantime?","American Beauty (Avantime?) (MPU4) (A2L0, Latvia, set 15)" )
GAME_CUSTOM( 199?, m4abeaut_l28,   m4abeaut,   "a2l0b310.bin", 0x0000, 0x010000, CRC(0e064cf7) SHA1(ea15e727ec83a3c13a865737bb0706b950820111), "Avantime?","American Beauty (Avantime?) (MPU4) (A2L0, Latvia, set 16)" )
GAME_CUSTOM( 199?, m4abeaut_l29,   m4abeaut,   "a2l0a315.bin", 0x0000, 0x010000, CRC(1e25daf5) SHA1(ebedaab2b05c96b1ff9acbb2f7821c6ad3499f6c), "Avantime?","American Beauty (Avantime?) (MPU4) (A2L0, Latvia, set 17)" )
GAME_CUSTOM( 199?, m4abeaut_l30,   m4abeaut,   "a2l0b315.bin", 0x0000, 0x010000, CRC(f7113363) SHA1(3539ee920707a0c5e57563625be626f43295e4c1), "Avantime?","American Beauty (Avantime?) (MPU4) (A2L0, Latvia, set 18)" )
GAME_CUSTOM( 199?, m4abeaut_l31,   m4abeaut,   "a2l0a515.bin", 0x0000, 0x010000, CRC(359c971f) SHA1(35fcd55a5f46694b4807a948f80046105d2350e4), "Avantime?","American Beauty (Avantime?) (MPU4) (A2L0, Latvia, set 19)" )
GAME_CUSTOM( 199?, m4abeaut_l32,   m4abeaut,   "a2l0b515.bin", 0x0000, 0x010000, CRC(54202008) SHA1(6947019df7e86c42e2ec90cc188357a1c93cdda1), "Avantime?","American Beauty (Avantime?) (MPU4) (A2L0, Latvia, set 20)" )
// a3l0
GAME_CUSTOM( 199?, m4abeaut_l33,   m4abeaut,   "a3l0a311.bin", 0x0000, 0x010000, CRC(51b207c5) SHA1(c6a8926b53a95ab8c2a219730bf3cb1e8fc1db31), "Avantime?","American Beauty (Avantime?) (MPU4) (A3L0, Latvia, set 1)" )
GAME_CUSTOM( 199?, m4abeaut_l34,   m4abeaut,   "a3l0b311.bin", 0x0000, 0x010000, CRC(f5c8880a) SHA1(da83e1940123b9440b78ee6bf036bd5272e587e9), "Avantime?","American Beauty (Avantime?) (MPU4) (A3L0, Latvia, set 2)" )
GAME_CUSTOM( 199?, m4abeaut_l35,   m4abeaut,   "a3l0a312.bin", 0x0000, 0x010000, CRC(63546571) SHA1(f203573a932cb65022bb9b159cbccd9abca282d5), "Avantime?","American Beauty (Avantime?) (MPU4) (A3L0, Latvia, set 3)" )
GAME_CUSTOM( 199?, m4abeaut_l36,   m4abeaut,   "a3l0b312.bin", 0x0000, 0x010000, CRC(f7c7a3b7) SHA1(5c993fed16505456a41db6301c28650c8e3753d7), "Avantime?","American Beauty (Avantime?) (MPU4) (A3L0, Latvia, set 4)" )
GAME_CUSTOM( 199?, m4abeaut_l37,   m4abeaut,   "a3l0a315.bin", 0x0000, 0x010000, CRC(9c6d6679) SHA1(e5efadc61b8a7a176ce5ff373092b8a8c8afe085), "Avantime?","American Beauty (Avantime?) (MPU4) (A3L0, Latvia, set 5)" )
GAME_CUSTOM( 199?, m4abeaut_l38,   m4abeaut,   "a3l0b315.bin", 0x0000, 0x010000, CRC(5441f57f) SHA1(d9b0c33a4c076c3539433a904a28df8bdf5437d3), "Avantime?","American Beauty (Avantime?) (MPU4) (A3L0, Latvia, set 6)" )
// am1 - new - projectbar
GAME_CUSTOM( 199?, m4abeaut_pb1,   m4abeaut,   "ajl0a314.bin", 0x0000, 0x010000, CRC(99a1a482) SHA1(1a5cbe86e9b7ba67fccfa87d8706e025edf173e6), "Avantime?","American Beauty (Avantime?) (MPU4) (AJL0, Project Bar, set 1)" )
GAME_CUSTOM( 199?, m4abeaut_pb2,   m4abeaut,   "ajl0b314.bin", 0x0000, 0x010000, CRC(117abb65) SHA1(46418fbab1b27bef840f1f17392054ec8a1fc4d5), "Avantime?","American Beauty (Avantime?) (MPU4) (AJL0, Project Bar, set 2)" )
// am1 - new - israel
GAME_CUSTOM( 199?, m4abeaut_i1,    m4abeaut,   "a2i0a215.bin", 0x0000, 0x010000, CRC(cf2425a7) SHA1(eee9bf97ff6905bea6bdec62c21fb47e2c7cb4e0), "Avantime?","American Beauty (Avantime?) (MPU4) (A2I0, Israel, set 1)" )
GAME_CUSTOM( 199?, m4abeaut_i2,    m4abeaut,   "a2i0b215.bin", 0x0000, 0x010000, CRC(d1ae0a37) SHA1(94d6be1fceec03b9c8dc9d1b48118caded2d4b2d), "Avantime?","American Beauty (Avantime?) (MPU4) (A2I0, Israel, set 2)" )
// am1 - new - czech
GAME_CUSTOM( 199?, m4abeaut_c1,    m4abeaut,   "abc1a127.bin", 0x0000, 0x010000, CRC(c64a3c57) SHA1(42aa460e8463152abed15e180255067ed08d000e), "Avantime?","American Beauty (Avantime?) (MPU4) (ABC1, Czech, set 1)" )
GAME_CUSTOM( 199?, m4abeaut_c2,    m4abeaut,   "abc1b127.bin", 0x0000, 0x010000, CRC(4dd74ff7) SHA1(54b6c5ddea0c6580eea4a9202c16fc53ee634f60), "Avantime?","American Beauty (Avantime?) (MPU4) (ABC1, Czech, set 2)" )
GAME_CUSTOM( 199?, m4abeaut_c3,    m4abeaut,   "abc1a129.bin", 0x0000, 0x010000, CRC(10e59623) SHA1(0e21fdde62e3cc31422494df4b31a8dd41ee8b4a), "Avantime?","American Beauty (Avantime?) (MPU4) (ABC1, Czech, set 3)" )
GAME_CUSTOM( 199?, m4abeaut_c4,    m4abeaut,   "abc1b129.bin", 0x0000, 0x010000, CRC(93a76979) SHA1(bd7bd2528e230392149e3e71f95577cf8d24faf3), "Avantime?","American Beauty (Avantime?) (MPU4) (ABC1, Czech, set 4)" )
GAME_CUSTOM( 199?, m4abeaut_c5,    m4abeaut,   "abc1a130.bin", 0x0000, 0x010000, CRC(ed053c43) SHA1(a6f8cd7c70e46d6088e3439f7a167b9f3a0aee8b), "Avantime?","American Beauty (Avantime?) (MPU4) (ABC1, Czech, set 5)" )
GAME_CUSTOM( 199?, m4abeaut_c6,    m4abeaut,   "abc1b130.bin", 0x0000, 0x010000, CRC(4b5fa794) SHA1(eb3ace534d3d94a181d5fe05966ba726a6c86536), "Avantime?","American Beauty (Avantime?) (MPU4) (ABC1, Czech, set 6)" )
GAME_CUSTOM( 199?, m4abeaut_c7,    m4abeaut,   "abc1a131.bin", 0x0000, 0x010000, CRC(4bc8db07) SHA1(43c63531e194aa509b4b8f07e3cef30c369c0612), "Avantime?","American Beauty (Avantime?) (MPU4) (ABC1, Czech, set 7)" )
GAME_CUSTOM( 199?, m4abeaut_c8,    m4abeaut,   "abc1b131.bin", 0x0000, 0x010000, CRC(1d8690ba) SHA1(c7174553b37cbf4a0916330677e0986848959e50), "Avantime?","American Beauty (Avantime?) (MPU4) (ABC1, Czech, set 8)" )
GAME_CUSTOM( 199?, m4abeaut_c9,    m4abeaut,   "abc1a132.bin", 0x0000, 0x010000, CRC(1dea8037) SHA1(95f40d6c3cf821380d344a8bfaca144bde569cfb), "Avantime?","American Beauty (Avantime?) (MPU4) (ABC1, Czech, set 9)" )
GAME_CUSTOM( 199?, m4abeaut_c10,   m4abeaut,   "abc1b132.bin", 0x0000, 0x010000, CRC(fddfb3c2) SHA1(498f1152abf15423374b8b9a5c8c6635e2530c2c), "Avantime?","American Beauty (Avantime?) (MPU4) (ABC1, Czech, set 10)" )
GAME_CUSTOM( 199?, m4abeaut_c11,   m4abeaut,   "abc1a134.bin", 0x0000, 0x010000, CRC(3caed0da) SHA1(3371aba1956fd1f624f64e5ea56c92a42e68bc9c), "Avantime?","American Beauty (Avantime?) (MPU4) (ABC1, Czech, set 11)" )
GAME_CUSTOM( 199?, m4abeaut_c12,   m4abeaut,   "abc1b134.bin", 0x0000, 0x010000, CRC(b9c68822) SHA1(50302685bec585f8e2884eb3db5508e87f4342f4), "Avantime?","American Beauty (Avantime?) (MPU4) (ABC1, Czech, set 12)" )
GAME_CUSTOM( 199?, m4abeaut_c13,   m4abeaut,   "abc1a135.bin", 0x0000, 0x010000, CRC(9126f0a8) SHA1(3dec7d68cc6cb6e0b23945d47cf219992599a275), "Avantime?","American Beauty (Avantime?) (MPU4) (ABC1, Czech, set 13)" )
GAME_CUSTOM( 199?, m4abeaut_c14,   m4abeaut,   "abc1b135.bin", 0x0000, 0x010000, CRC(7b0ee1fb) SHA1(6262805ac053d17d18e80e86980bbf66613c8662), "Avantime?","American Beauty (Avantime?) (MPU4) (ABC1, Czech, set 14)" )
GAME_CUSTOM( 199?, m4abeaut_c15,   m4abeaut,   "abc1b110.bin", 0x0000, 0x010000, CRC(86c2658d) SHA1(3a8177a95696802a478964c045ca0f1b0aebcba4), "Avantime?","American Beauty (Avantime?) (MPU4) (ABC1, Czech, set 15)" ) // unpaired
// abc2
GAME_CUSTOM( 199?, m4abeaut_c16,   m4abeaut,   "abc2a127.bin", 0x0000, 0x010000, CRC(3094b4e5) SHA1(6b0fbf41ffec4bb21527dd59d1022fa90cee5b4e), "Avantime?","American Beauty (Avantime?) (MPU4) (ABC2, Czech, set 1)" )
GAME_CUSTOM( 199?, m4abeaut_c17,   m4abeaut,   "abc2b127.bin", 0x0000, 0x010000, CRC(44a0639d) SHA1(b370164df7b017c83ea6e0da16d4f7b57d4e3ad3), "Avantime?","American Beauty (Avantime?) (MPU4) (ABC2, Czech, set 2)" )
GAME_CUSTOM( 199?, m4abeaut_c18,   m4abeaut,   "abc2a129.bin", 0x0000, 0x010000, CRC(5c3b0dd7) SHA1(bf380fa763e331893c0d9d365305f600a1d43e24), "Avantime?","American Beauty (Avantime?) (MPU4) (ABC2, Czech, set 3)" )
GAME_CUSTOM( 199?, m4abeaut_c19,   m4abeaut,   "abc2b129.bin", 0x0000, 0x010000, CRC(8e11e0f6) SHA1(e52cf948ebca4884fe0ef006d1b73fa5c613c168), "Avantime?","American Beauty (Avantime?) (MPU4) (ABC2, Czech, set 4)" )
GAME_CUSTOM( 199?, m4abeaut_c20,   m4abeaut,   "abc2a130.bin", 0x0000, 0x010000, CRC(8a7b5f0e) SHA1(11c98a63abe2d7749c55843a62efec9f91874eec), "Avantime?","American Beauty (Avantime?) (MPU4) (ABC2, Czech, set 5)" )
GAME_CUSTOM( 199?, m4abeaut_c21,   m4abeaut,   "abc2b130.bin", 0x0000, 0x010000, CRC(f8d8deae) SHA1(368db35471e0e94e2b680489d1d922d61ec25b23), "Avantime?","American Beauty (Avantime?) (MPU4) (ABC2, Czech, set 6)" )
GAME_CUSTOM( 199?, m4abeaut_c22,   m4abeaut,   "abc2a131.bin", 0x0000, 0x010000, CRC(824be635) SHA1(08b0e923fcaf5b77784cbb1ff9cc1f06b0ccd805), "Avantime?","American Beauty (Avantime?) (MPU4) (ABC2, Czech, set 7)" )
GAME_CUSTOM( 199?, m4abeaut_c23,   m4abeaut,   "abc2b131.bin", 0x0000, 0x010000, CRC(42782d26) SHA1(4924f312be89f264c4d792d46e96425b1ba63ef2), "Avantime?","American Beauty (Avantime?) (MPU4) (ABC2, Czech, set 8)" )
GAME_CUSTOM( 199?, m4abeaut_c24,   m4abeaut,   "abc2a132.bin", 0x0000, 0x010000, CRC(da632b39) SHA1(393430ce3e11610ff7b1aae989eb7ad26a1209f4), "Avantime?","American Beauty (Avantime?) (MPU4) (ABC2, Czech, set 9)" )
GAME_CUSTOM( 199?, m4abeaut_c25,   m4abeaut,   "abc2b132.bin", 0x0000, 0x010000, CRC(ee32aa7d) SHA1(e6925f63227c96ae3ea6b132d444829b6c0bf7ee), "Avantime?","American Beauty (Avantime?) (MPU4) (ABC2, Czech, set 10)" )
GAME_CUSTOM( 199?, m4abeaut_c26,   m4abeaut,   "abc2a134.bin", 0x0000, 0x010000, CRC(d979a744) SHA1(fd5a8eaba442b56b6265e9fef44f8d89d00a8602), "Avantime?","American Beauty (Avantime?) (MPU4) (ABC2, Czech, set 11)" )
GAME_CUSTOM( 199?, m4abeaut_c27,   m4abeaut,   "abc2b134.bin", 0x0000, 0x010000, CRC(0705d6d8) SHA1(4b2535ab1bcf9683a50fe6a16038e37057f48ff6), "Avantime?","American Beauty (Avantime?) (MPU4) (ABC2, Czech, set 12)" )
GAME_CUSTOM( 199?, m4abeaut_c28,   m4abeaut,   "abc2a135.bin", 0x0000, 0x010000, CRC(049d52bb) SHA1(ef9217c1349336410436cc83832c6b18c5dc3b78), "Avantime?","American Beauty (Avantime?) (MPU4) (ABC2, Czech, set 13)" )
GAME_CUSTOM( 199?, m4abeaut_c29,   m4abeaut,   "abc2b135.bin", 0x0000, 0x010000, CRC(e5062947) SHA1(8530dfa32ea9f129b00f7cee29a23191f3484a9d), "Avantime?","American Beauty (Avantime?) (MPU4) (ABC2, Czech, set 14)" )
// am1 - new - czech - 2meg
GAME_CUSTOM( 199?, m4abeaut_c30,   m4abeaut,   "m2c1a135.bin", 0x0000, 0x040000, CRC(98bf42cd) SHA1(d2f8bcc637e36ba053c7bca9034522f77490bbb3), "Avantime?","American Beauty (Avantime?) (MPU4) (M2C1, Czech, set 1)" )
GAME_CUSTOM( 199?, m4abeaut_c31,   m4abeaut,   "m2c1b135.bin", 0x0000, 0x040000, CRC(cc48e2cf) SHA1(2666472f8b495f755cda1ee9fb0448430ff2315d), "Avantime?","American Beauty (Avantime?) (MPU4) (M2C1, Czech, set 2)" )
// abs1 - slovakia? slovenia?
GAME_CUSTOM( 199?, m4abeaut_s1,    m4abeaut,   "abs1a135.bin", 0x0000, 0x010000, CRC(3143baab) SHA1(84949d00fe765e70ce8c5910a0bd859b1355ddc7), "Avantime?","American Beauty (Avantime?) (MPU4) (ABS1, Slovakia, set 1)" )
GAME_CUSTOM( 199?, m4abeaut_s2,    m4abeaut,   "abs1b135.bin", 0x0000, 0x010000, CRC(96e27b72) SHA1(0e3c1f06cedbd3b26ab73aeee92ceadeb995e647), "Avantime?","American Beauty (Avantime?) (MPU4) (ABS1, Slovakia, set 2)" )
// abs2
GAME_CUSTOM( 199?, m4abeaut_s3,    m4abeaut,   "abs2a135.bin", 0x0000, 0x010000, CRC(0664c4ef) SHA1(f642f87ff7d01f2fa995142032e27dfadc76cff7), "Avantime?","American Beauty (Avantime?) (MPU4) (ABS2, Slovakia, set 1)" )
GAME_CUSTOM( 199?, m4abeaut_s4,    m4abeaut,   "abs2b135.bin", 0x0000, 0x010000, CRC(4c3d0187) SHA1(51bd2be8c5098784242dc86254dea7c8714e5fd9), "Avantime?","American Beauty (Avantime?) (MPU4) (ABS2, Slovakia, set 2)" )
// abm1 - new - montenegro
GAME_CUSTOM( 199?, m4abeaut_m1,    m4abeaut,   "abm1a136.bin", 0x0000, 0x010000, CRC(30c987d1) SHA1(c4af4b3e849b46b5634933eb14869cead792d573), "Avantime?","American Beauty (Avantime?) (MPU4) (ABM1, Montenegro, set 1)" )
GAME_CUSTOM( 199?, m4abeaut_m2,    m4abeaut,   "abm1b136.bin", 0x0000, 0x010000, CRC(0d9c9040) SHA1(8904244a1a732b2fe699b7d88718732c487f8049), "Avantime?","American Beauty (Avantime?) (MPU4) (ABM1, Montenegro, set 2)" )
// abm2
GAME_CUSTOM( 199?, m4abeaut_m3,    m4abeaut,   "abm2a136.bin", 0x0000, 0x010000, CRC(92980ddc) SHA1(7f2432db1d97e562859b84cf6be3605b537c4379), "Avantime?","American Beauty (Avantime?) (MPU4) (ABM2, Montenegro, set 1)" )
GAME_CUSTOM( 199?, m4abeaut_m4,    m4abeaut,   "abm2b136.bin", 0x0000, 0x010000, CRC(92a8baef) SHA1(0b3a40aa8a21ec7dd5c3aa14f3fb8b5d4b86582b), "Avantime?","American Beauty (Avantime?) (MPU4) (ABM2, Montenegro, set 2)" )
// am1 - new - a2k
GAME_CUSTOM( 199?, m4abeaut_k1,    m4abeaut,   "a2k0a618.bin", 0x0000, 0x010000, CRC(fb6c2c55) SHA1(5d5631545142aa223ea15fd15cd4710aab5cc908), "Avantime?","American Beauty (Avantime?) (MPU4) (A2K0, set 1)" )
GAME_CUSTOM( 199?, m4abeaut_k2,    m4abeaut,   "a2k0b618.bin", 0x0000, 0x010000, CRC(545d7ffa) SHA1(1008a7a7affe4a56589e5ca691aaeb4a82218036), "Avantime?","American Beauty (Avantime?) (MPU4) (A2K0, set 2)" )




#define M4TRG_EXTRA_ROMS \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "trgsound.dat", 0x0000, 0x080000, CRC(b9eeffbd) SHA1(9ab8005bbabb30358e3e1ccc007372542bc2e799) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4TRG_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default ,ROT0,company,title,GAME_FLAGS )


// roms of different sizes again, might also be main / sub setups
GAME_CUSTOM( 199?, m4trg,     0,      "tglp3.2c",     0x0000, 0x020000, CRC(6c1602cd) SHA1(0cc8aa53584c4da7e39e359cdff08a8b7ab1fd9e), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4trg__a,  m4trg,  "tglp3.3c",     0x0000, 0x020000, CRC(c42e8801) SHA1(bf70ca76eb4748a5b85608c50b9bff2776c1bbd7), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4trg__b,  m4trg,  "tglpv2.2",     0x0000, 0x020000, CRC(6a393a3c) SHA1(1bb98d61cc50828e63993a178a8bf895952de375), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4trg__c,  m4trg,  "tglpv2.3b",    0x0000, 0x020000, CRC(c77c7a08) SHA1(f0104298132666cff9d81829bd5f58904c290290), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4trg__d,  m4trg,  "tglpv1.1s",    0x0000, 0x010000, CRC(a7579e5f) SHA1(b4d78570ef5c32bd0dce75600c2ad64884f894e6), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4trg__e,  m4trg,  "tgpr1.1",      0x0000, 0x020000, CRC(53f9f5fe) SHA1(0787ecff459d8ae748847f17f6d3dbfa15b87db4), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4trg__f,  m4trg,  "tgpu",         0x0000, 0x020000, CRC(4aa9f068) SHA1(947586a7f65743443c0846f7b3043d7c6ffecdd8), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4trg__g,  m4trg,  "tgpu1.1b",     0x0000, 0x020000, CRC(1f3add51) SHA1(d7d2933505cf6e86aeb7efc4d2133a8be320bb25), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4trg__h,  m4trg,  "tgpu1.2b",     0x0000, 0x020000, CRC(b702579d) SHA1(11c67c5a322c71dc4e31c28b83ce810ce3c870d7), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 9)" )
GAME_CUSTOM( 199?, m4trg__i,  m4trg,  "tgpu1.3b",     0x0000, 0x020000, CRC(3c8eca5a) SHA1(09b19660f4f3f319576393f961b61f16d738f6c3), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 10)" )
GAME_CUSTOM( 199?, m4trg__j,  m4trg,  "tgpu1.4b",     0x0000, 0x020000, CRC(344de34d) SHA1(403edd6760a1b0ccddb634fc9bdbe4af5a011c10), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 11)" )
GAME_CUSTOM( 199?, m4trg__k,  m4trg,  "tgpv1.1s",     0x0000, 0x010000, CRC(6f5f33cb) SHA1(6b7708755809a0486e3cbb84f3487f0979c1311d), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 12)" )
GAME_CUSTOM( 199?, m4trg__l,  m4trg,  "tgpv1.2s",     0x0000, 0x010000, CRC(d7b000cb) SHA1(e07643107a00cada3259d79c731ba7c60e1f1e39), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 13)" )
GAME_CUSTOM( 199?, m4trg__m,  m4trg,  "tgpv1.3",      0x0000, 0x020000, CRC(b0e4a452) SHA1(88e2b3bad1b83101f910b9a9c9bd6c000726fc3a), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 14)" )
GAME_CUSTOM( 199?, m4trg__n,  m4trg,  "tgpv1.4b",     0x0000, 0x020000, CRC(18dc2e9e) SHA1(a4fc75ac91cfb0dba3cff3a4d07d8842720ae00f), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 15)" )
GAME_CUSTOM( 199?, m4trg__o,  m4trg,  "trgv1.3",      0x0000, 0x020000, CRC(d04fa5da) SHA1(b0e6ed25337f250abf1f5ac5bb7073306618d3e0), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 16)" )
GAME_CUSTOM( 199?, m4trg__p,  m4trg,  "trgv1.3s",     0x0000, 0x010000, CRC(cbae4b44) SHA1(5db57cebcdaf384e63d7cf5337285b96b0557169), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 17)" )
GAME_CUSTOM( 199?, m4trg__q,  m4trg,  "trgv1.4",      0x0000, 0x020000, CRC(eab15c79) SHA1(a96aeba746f5ec53514b1bc34d93785dc63a8421), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 18)" )
GAME_CUSTOM( 199?, m4trg__r,  m4trg,  "trgv1.4s.4s",  0x0000, 0x010000, CRC(a9f1d7f6) SHA1(508dddaf8e7b747adb8398bc68e14894a792e003), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 19)" )
GAME_CUSTOM( 199?, m4trg__s,  m4trg,  "trgv1.5.5",    0x0000, 0x020000, CRC(e900c054) SHA1(bae3c719e645aeb9b513ef11b92a242bbf8e052e), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 20)" )
GAME_CUSTOM( 199?, m4trg__t,  m4trg,  "trgv1.5b",     0x0000, 0x020000, CRC(c1dc1031) SHA1(a79c7158095e12587e3c120258921b8a1f8610a2), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 21)" )
GAME_CUSTOM( 199?, m4trg__u,  m4trg,  "trgv1.6b",     0x0000, 0x020000, CRC(69e49afd) SHA1(5db4e9c84a03e5ac61fed47a6d5dd991bfe39998), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 22)" )
GAME_CUSTOM( 199?, m4trg__v,  m4trg,  "tot5p.dat",    0x0000, 0x020000, CRC(2bf48dce) SHA1(27b8f482a5486aeafd926af98fdebbc9acbb4aca), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 23)" )
GAME_CUSTOM( 199?, m4trg__w,  m4trg,  "tr0201.1",     0x0000, 0x020000, CRC(6d72c203) SHA1(43262e2b46eab3d52fcd34d6ff45ceadfefb0684), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 24)" )
GAME_CUSTOM( 199?, m4trg__x,  m4trg,  "tr0201.1k",    0x0000, 0x020000, CRC(8d764623) SHA1(61ba1cafb64909042bfd6b23380333cf0d2a5ee4), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 25)" )
GAME_CUSTOM( 199?, m4trg__y,  m4trg,  "trgu1.1b",     0x0000, 0x020000, CRC(0f207e95) SHA1(a63cb9ebcb39de04ba36bd5bcffaa959586fc99b), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 26)" )
GAME_CUSTOM( 199?, m4trg__z,  m4trg,  "trgu1.2b",     0x0000, 0x020000, CRC(a718f459) SHA1(3f2e28c67d442be89fab3514f3397d58c0b54f3f), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 27)" )
GAME_CUSTOM( 199?, m4trg__0,  m4trg,  "trglatv1.3s",  0x0000, 0x010000, CRC(03a6e6d0) SHA1(d73c921ee29054084ffe70ecd6f165f7930526cc), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 28)" )
GAME_CUSTOM( 199?, m4trg__1,  m4trg,  "trglatv1.4s",  0x0000, 0x010000, CRC(61f97a62) SHA1(9b0b55d2c3a00d6095307480587b71ee12e03eb7), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 29)" )
GAME_CUSTOM( 199?, m4trg__2,  m4trg,  "mbpl3.2c",     0x0000, 0x020000, CRC(ea8e58fb) SHA1(186f519fd4ebfa0e61cac8f392d6253df72523ec), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 30)" ) // == m4bel__az
GAME_CUSTOM( 199?, m4trg__3,  m4trg,  "rmtp.s8",      0x0000, 0x010000, CRC(91570052) SHA1(4a7a084403057e193602ee36a623a61c9ccad726), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 31)" )
GAME_CUSTOM( 199?, m4trg__4,  m4trg,  "rmtp4cz",      0x0000, 0x010000, CRC(1c5fd88b) SHA1(a25b78b0a88ec9468c9ede4b3784e017e7cb571c), "Avantime?","Turbo Reel Gambler (Avantime?) (MPU4) (set 32)" )



#define M4RMTP_EXTRA_ROMS \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "rm.s3", 0x0000, 0x080000, CRC(250e64f2) SHA1(627c4dc5cdc7d0a7cb6f74991ae91b71a2f4dbc6) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4RMTP_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default ,ROT0,company,title,GAME_FLAGS )


GAME_CUSTOM( 199?, m4rmtp,       0,      "r4iha202.bin", 0x0000, 0x010000, CRC(b1588632) SHA1(ad21bbc5e99fd6b511e6881e8b20dcad177b937f), "Avantime?","Reel Magic Turbo Play (Avantime?) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4rmtp__a,    m4rmtp, "r4iha203.bin", 0x0000, 0x010000, CRC(7f31cb76) SHA1(9a2a595afb9ff1b3165638d247ab98475ae0bfcd), "Avantime?","Reel Magic Turbo Play (Avantime?) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4rmtp__b,    m4rmtp, "r4iha204.bin", 0x0000, 0x010000, CRC(1cc3a32d) SHA1(b6ed012a6d743ba2416e25e7c49ce9985bbacbd7), "Avantime?","Reel Magic Turbo Play (Avantime?) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4rmtp__c,    m4rmtp, "r4iha205.bin", 0x0000, 0x010000, CRC(1a238632) SHA1(a15ca5801d41985387bc65579b6d6ee2ef7d8eee), "Avantime?","Reel Magic Turbo Play (Avantime?) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4rmtp__d,    m4rmtp, "r4iua202.bin", 0x0000, 0x010000, CRC(c96d630a) SHA1(90ed759602aa3a052434b3f604ec26ec9e204e68), "Avantime?","Reel Magic Turbo Play (Avantime?) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4rmtp__e,    m4rmtp, "r4iua203.bin", 0x0000, 0x010000, CRC(550fdfec) SHA1(d57eaba6690cbff2302559e9cea9e5d0f79cf9f9), "Avantime?","Reel Magic Turbo Play (Avantime?) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4rmtp__f,    m4rmtp, "r4iua204.bin", 0x0000, 0x010000, CRC(cd8d166f) SHA1(4d78726df35914444be26ac9e1e3e1949b6a3d99), "Avantime?","Reel Magic Turbo Play (Avantime?) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4rmtp__g,    m4rmtp, "r4iua205.bin", 0x0000, 0x010000, CRC(46df24f3) SHA1(31000815a90e47e744091bbf0fe9e96baac8d7e3), "Avantime?","Reel Magic Turbo Play (Avantime?) (MPU4) (set 8)" )


#define M4RMTPD_EXTRA_ROMS \
	ROM_REGION( 0x10000, "gal", 0 ) \
	ROM_LOAD( "rmdxi", 0x0000, 0x000b57, CRC(c16021ec) SHA1(df77e410ea2edae1559e40a877e292f0d1969b0a) ) \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) \
	/* missing */

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4RMTPD_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default ,ROT0,company,title,GAME_FLAGS )



GAME_CUSTOM( 199?, m4rmtpd,     0,          "rdiua202.bin", 0x0000, 0x010000, CRC(faa875ea) SHA1(d8d206fed8965a26dd8ded38a3be018311ccf407), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4rmtpd__a,  m4rmtpd,    "r2iha203.bin", 0x0000, 0x010000, CRC(1cea7710) SHA1(a250569800d3679f317a485ac7a31b4f4fa7db78), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4rmtpd__b,  m4rmtpd,    "r2iha204.bin", 0x0000, 0x010000, CRC(c82cd025) SHA1(f26f2bbd83d673c61bd2609914349b45c31f4a5d), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4rmtpd__c,  m4rmtpd,    "r2iha205.bin", 0x0000, 0x010000, CRC(e53da9a5) SHA1(5019f5bd89c230459629670b808c59888a0f1ee9), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4rmtpd__d,  m4rmtpd,    "r2iha206.bin", 0x0000, 0x010000, CRC(f89b73b3) SHA1(34a9a8053e881b8aad578ef58209c8ff888b30f7), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4rmtpd__e,  m4rmtpd,    "r2iua203.bin", 0x0000, 0x010000, CRC(9590a747) SHA1(9f1a1277bdcbe0f23abcf38850eae939997c2e00), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4rmtpd__f,  m4rmtpd,    "r2iua205.bin", 0x0000, 0x010000, CRC(2eefca1a) SHA1(cabc0c8a3dddc881aab899c5419663efff5412d3), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4rmtpd__g,  m4rmtpd,    "r2iua206.bin", 0x0000, 0x010000, CRC(c4a1a218) SHA1(8208468ae9ddde7d387f7194e1f7d44f6e7ca730), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4rmtpd__h,  m4rmtpd,    "r3iha224.bin", 0x0000, 0x010000, CRC(a2e161ac) SHA1(bd63c9726cdf037919c8655221bc6416cef322aa), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 9)" )
GAME_CUSTOM( 199?, m4rmtpd__i,  m4rmtpd,    "r3iha225.bin", 0x0000, 0x010000, CRC(f49a41e9) SHA1(6c29ba4bf76aaafa79ce68f58f6672baa47fe147), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 10)" )
GAME_CUSTOM( 199?, m4rmtpd__j,  m4rmtpd,    "r3iua224.bin", 0x0000, 0x010000, CRC(715b7de7) SHA1(013827680c389968f2f80f97c565716757d696b2), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 11)" )
GAME_CUSTOM( 199?, m4rmtpd__k,  m4rmtpd,    "r3iua225.bin", 0x0000, 0x010000, CRC(37086f91) SHA1(413b32a8e354467a30c71dce3d1cb76795ff813d), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 12)" )
GAME_CUSTOM( 199?, m4rmtpd__l,  m4rmtpd,    "r4iha201.bin", 0x0000, 0x010000, CRC(789cfca1) SHA1(31aa7bf9461cb6c4f692d605463fde1f604b1614), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 13)" )
GAME_CUSTOM( 199?, m4rmtpd__m,  m4rmtpd,    "r4iua201.bin", 0x0000, 0x010000, CRC(ce0e2553) SHA1(4c9df36a7b8950a273cefceb6ba6817d8b862c78), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 14)" )
GAME_CUSTOM( 199?, m4rmtpd__n,  m4rmtpd,    "rdiha202.bin", 0x0000, 0x010000, CRC(02e01481) SHA1(253c2c8e800a4e6d1008745101e2457d76ac57d4), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 15)" )
GAME_CUSTOM( 199?, m4rmtpd__o,  m4rmtpd,    "rdiha212.bin", 0x0000, 0x010000, CRC(90984ae9) SHA1(b25a12f0529af64315c461363c788c22e30d4016), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 16)" )
GAME_CUSTOM( 199?, m4rmtpd__p,  m4rmtpd,    "rdiha213.bin", 0x0000, 0x010000, CRC(e9fa4c97) SHA1(07c75418890231102cf336f2d3f0048fe4884862), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 17)" )
GAME_CUSTOM( 199?, m4rmtpd__q,  m4rmtpd,    "rdiha214.bin", 0x0000, 0x010000, CRC(42f3a5e0) SHA1(3cf26e55edf0dcde9510e50c4b781ba8b906f092), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 18)" )
GAME_CUSTOM( 199?, m4rmtpd__r,  m4rmtpd,    "rdiha215.bin", 0x0000, 0x010000, CRC(2b704591) SHA1(9b880f40d3b268c96af5dab179760994c5a074c9), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 19)" )
GAME_CUSTOM( 199?, m4rmtpd__s,  m4rmtpd,    "rdiha217.bin", 0x0000, 0x010000, CRC(6df58d97) SHA1(df8f419a1e3acc68a3755c49e258db5af9102598), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 20)" )
GAME_CUSTOM( 199?, m4rmtpd__t,  m4rmtpd,    "rdiha219.bin", 0x0000, 0x010000, CRC(66f3ffa6) SHA1(1b1daf4b02e400d943f2a917be0f4452be891aaf), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 21)" )
GAME_CUSTOM( 199?, m4rmtpd__u,  m4rmtpd,    "rdiha220.bin", 0x0000, 0x010000, CRC(2047c55b) SHA1(8f0e6608271634a6a0f06e76df93dddd404c93cd), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 22)" )
GAME_CUSTOM( 199?, m4rmtpd__v,  m4rmtpd,    "rdiha221.bin", 0x0000, 0x010000, CRC(6e87f591) SHA1(750e9f01c1a3143d7d97a5b9b11d09aed72ca928), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 23)" )
GAME_CUSTOM( 199?, m4rmtpd__w,  m4rmtpd,    "rdiha222.bin", 0x0000, 0x010000, CRC(d200f6ec) SHA1(07e0e270a2184f24373cbe0a8a5e44c3d215d9a2), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 24)" )
GAME_CUSTOM( 199?, m4rmtpd__x,  m4rmtpd,    "rdiha223.bin", 0x0000, 0x010000, CRC(042a5a96) SHA1(3bc2dfb89c6781eb9fb105e5f8ea1576d7b49ad3), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 25)" )
GAME_CUSTOM( 199?, m4rmtpd__y,  m4rmtpd,    "rdihb202.bin", 0x0000, 0x010000, CRC(136c31ec) SHA1(abb095bd4ec0a0879f49e668f1ea08df026262e7), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 26)" )
GAME_CUSTOM( 199?, m4rmtpd__z,  m4rmtpd,    "rdiua204.bin", 0x0000, 0x010000, CRC(a6110b45) SHA1(61d08250fa3b5d7eb7cdf63562d7a6cc9a27372c), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 27)" )
GAME_CUSTOM( 199?, m4rmtpd__0,  m4rmtpd,    "rdiua205.bin", 0x0000, 0x010000, CRC(9f20d810) SHA1(e2a576313fa49fc72001d5de67e93c08423e8dd8), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 28)" )
GAME_CUSTOM( 199?, m4rmtpd__1,  m4rmtpd,    "rdiua206.bin", 0x0000, 0x010000, CRC(2954e2c3) SHA1(e4c9f51748bc1296298f95ca817e852f9e0ca38b), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 29)" )
GAME_CUSTOM( 199?, m4rmtpd__2,  m4rmtpd,    "rdiua207.bin", 0x0000, 0x010000, CRC(58f334d1) SHA1(b91288731750445e4cfcf87fe6a9504723b59fa9), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 30)" )
GAME_CUSTOM( 199?, m4rmtpd__3,  m4rmtpd,    "rdiua208.bin", 0x0000, 0x010000, CRC(13e6d84d) SHA1(6f75a75dfd6922349f8d29c955c1849522f8656c), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 31)" )
GAME_CUSTOM( 199?, m4rmtpd__4,  m4rmtpd,    "rdiua209.bin", 0x0000, 0x010000, CRC(f41af938) SHA1(f2d4e23717f49961fe104971b3a0da9aabbf0e05), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 32)" )
GAME_CUSTOM( 199?, m4rmtpd__5,  m4rmtpd,    "rdiua212.bin", 0x0000, 0x010000, CRC(c56a6433) SHA1(7ff8943843c334a79fc3b40bb004abb3f2c2d079), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 33)" )
GAME_CUSTOM( 199?, m4rmtpd__6,  m4rmtpd,    "rdiua213.bin", 0x0000, 0x010000, CRC(cdd5f399) SHA1(a4359c5166fbcd4ea2bb6820bbfead6bc2b2a4ef), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 34)" )
GAME_CUSTOM( 199?, m4rmtpd__7,  m4rmtpd,    "rdiua214.bin", 0x0000, 0x010000, CRC(04fa9d21) SHA1(c337486ece94a7004420edca677e6688eef1ac9e), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 35)" )
GAME_CUSTOM( 199?, m4rmtpd__8,  m4rmtpd,    "rdiua215.bin", 0x0000, 0x010000, CRC(43d8ca5e) SHA1(e5e24ed24bd5c1135392c98910d2797e621ecbd5), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 36)" )
GAME_CUSTOM( 199?, m4rmtpd__9,  m4rmtpd,    "rdiua217.bin", 0x0000, 0x010000, CRC(3c58970e) SHA1(15b7368078750021202ee7b4886a6510fcc1ba0d), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 37)" )
GAME_CUSTOM( 199?, m4rmtpd__aa, m4rmtpd,    "rdiua219.bin", 0x0000, 0x010000, CRC(54f8fe63) SHA1(48d1b04dde6056b839ec84daa40a7d6871893b3e), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 38)" )
GAME_CUSTOM( 199?, m4rmtpd__ab, m4rmtpd,    "rdiua220.bin", 0x0000, 0x010000, CRC(768715f6) SHA1(5c4102b4d2400806dd0f5a6f3e48da4d290d5255), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 39)" )
GAME_CUSTOM( 199?, m4rmtpd__ac, m4rmtpd,    "rdiua222.bin", 0x0000, 0x010000, CRC(07413d93) SHA1(e752fe382d222eefd4fe975fa40559fedd579320), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 40)" )
GAME_CUSTOM( 199?, m4rmtpd__ad, m4rmtpd,    "rdiua223.bin", 0x0000, 0x010000, CRC(80185ebf) SHA1(ae885325f2c63f7dbb034f8e3f3882d0b36aff99), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 41)" )
GAME_CUSTOM( 199?, m4rmtpd__ae, m4rmtpd,    "rdiub202.bin", 0x0000, 0x010000, CRC(4a8d7cf7) SHA1(cb1525d89d3a411163bfe9e70c8b0d1aa6cefdf5), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 42)" )
GAME_CUSTOM( 199?, m4rmtpd__af, m4rmtpd,    "rdmha210.bin", 0x0000, 0x010000, CRC(7061373e) SHA1(67da39d1de4f3877f12bd1fd5545046f9dabfde9), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 43)" )
GAME_CUSTOM( 199?, m4rmtpd__ag, m4rmtpd,    "rdmhb210.bin", 0x0000, 0x010000, CRC(12c71e8a) SHA1(9bb45e72f202d3af19988ebf30ea4c2248d387fc), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 44)" )
GAME_CUSTOM( 199?, m4rmtpd__ah, m4rmtpd,    "rdpka316.bin", 0x0000, 0x010000, CRC(5178175d) SHA1(a732a82226c34be0b7f84e9f9e4700bd72da1c19), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 45)" )
GAME_CUSTOM( 199?, m4rmtpd__ai, m4rmtpd,    "rdpka318.bin", 0x0000, 0x010000, CRC(2789179f) SHA1(8d4b1e75995ea5b64fac1a36a98506aacfd1800a), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 46)" )
GAME_CUSTOM( 199?, m4rmtpd__aj, m4rmtpd,    "rdpkb316.bin", 0x0000, 0x010000, CRC(09d4f4c5) SHA1(fbc2b0710ef048c221b007692e9a97b99f1edbc0), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 47)" )
GAME_CUSTOM( 199?, m4rmtpd__ak, m4rmtpd,    "rdpkb318.bin", 0x0000, 0x010000, CRC(428aa7f2) SHA1(f85d173c25d0ab9d8c3c4d87b4fc27c3342b3dec), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 48)" )
GAME_CUSTOM( 199?, m4rmtpd__al, m4rmtpd,    "rduha511.bin", 0x0000, 0x010000, CRC(823e0323) SHA1(4137a05efe87851a9f9ffcd6519bb57398773095), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 49)" )
GAME_CUSTOM( 199?, m4rmtpd__am, m4rmtpd,    "rduhb511.bin", 0x0000, 0x010000, CRC(2b65eb19) SHA1(b00543b74ad5262b85f66f5e8cfdaee351f62f23), "Avantime?","Reel Magic Turbo Play Deluxe (Avantime?) (MPU4) (set 50)" )


#define M4MBEL_EXTRA_ROMS \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) \
	/* missing */

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4MBEL_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4mbel,       0,      "mb1.1k",           0x0000, 0x020000, CRC(00763b37) SHA1(2314ea5e8541e2be2492135785317f4fdd998692), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4mbel__a,    m4mbel, "mb1.1kw",          0x0000, 0x010000, CRC(d1ccefe4) SHA1(dba63d0a75fe614e77ac24ae23bfd0d924dd3e9a), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4mbel__b,    m4mbel, "mb1.3b",           0x0000, 0x020000, CRC(8650d0d5) SHA1(089ab315e97c7ff1d898357b2d083fe33bb7a329), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4mbel__c,    m4mbel, "mb1.2k",           0x0000, 0x020000, CRC(a84eb1fb) SHA1(3b6bcfa8ae29796fd2effd7c9e5a95c5ba38ec7c), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4mbel__d,    m4mbel, "mb1.2kw",          0x0000, 0x020000, CRC(b4f0c93e) SHA1(3432c5a2c5091f311a981f819f4f4b7af63e041b), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4mbel__e,    m4mbel, "mb1.4b",           0x0000, 0x020000, CRC(89b1d56e) SHA1(ec45a6b6987b7c5048d9cb83cbd477807bae737b), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4mbel__f,    m4mbel, "mb1.4p",           0x0000, 0x020000, CRC(d76c717d) SHA1(0865245fb21ea51f7863815e6ac36abb4ef31bec), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4mbel__g,    m4mbel, "mb1.4pc",          0x0000, 0x020000, CRC(f70c3d82) SHA1(c28ee358217dc94e8b722648c81ff49bd2800dfc), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4mbel__h,    m4mbel, "mb1.4pk",          0x0000, 0x020000, CRC(52d06243) SHA1(96d4eca4b25d06f1efa1f73a529877c7462fd477), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 9)" )
GAME_CUSTOM( 199?, m4mbel__i,    m4mbel, "mb14pks",          0x0000, 0x020000, CRC(d061968e) SHA1(96ec79f065ddeefb0e3c71594afe366435b47d79), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 10)" )
GAME_CUSTOM( 199?, m4mbel__j,    m4mbel, "mb1.4po",          0x0000, 0x020000, CRC(bed2897d) SHA1(b64b92c7b33ed69354eb2a5c2257424c698a3ec9), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 11)" )
GAME_CUSTOM( 199?, m4mbel__k,    m4mbel, "mb1.4pp_ps",           0x0000, 0x020000, CRC(f653a0e2) SHA1(13b2ee53b092b820f9e86583d760a0bc4de5967e), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 12)" )
GAME_CUSTOM( 199?, m4mbel__m,    m4mbel, "mb1.5pc",          0x0000, 0x020000, CRC(f972f856) SHA1(727c9033bb764523f2507aad962ecdd1eb1f1c76), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 14)" )
GAME_CUSTOM( 199?, m4mbel__n,    m4mbel, "mb1.5pk",          0x0000, 0x020000, CRC(fae8e88f) SHA1(c80ac974694fb0cd2b8420ba3c6ef3a45c54cecf), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 15)" )
GAME_CUSTOM( 199?, m4mbel__o,    m4mbel, "mb1.5",            0x0000, 0x020000, CRC(719623b5) SHA1(fdd56de9990d82b8630c1d71f5ef4dfaedc43b96), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 16)" )
GAME_CUSTOM( 199?, m4mbel__p,    m4mbel, "mb1.5b",           0x0000, 0x020000, CRC(21895fa2) SHA1(07d9fe42d71af962a1ba9a005cfd5f44773ebde4), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 17)" )
GAME_CUSTOM( 199?, m4mbel__q,    m4mbel, "mb1.5i",           0x0000, 0x020000, CRC(7cfcfc60) SHA1(d902e735634d5e4f6fa4fc73ddc88bfbbbe27d4b), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 18)" )
GAME_CUSTOM( 199?, m4mbel__r,    m4mbel, "mb1.5is",          0x0000, 0x010000, CRC(2503da02) SHA1(216454810187dfdd1eb4c97e38ab19572a523b70), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 19)" )
GAME_CUSTOM( 199?, m4mbel__s,    m4mbel, "mb1.5p",           0x0000, 0x020000, CRC(ee7ee9a3) SHA1(cbf39b97ecb7a1098416d06fd8e3128a6a3a203f), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 20)" )
GAME_CUSTOM( 199?, m4mbel__t,    m4mbel, "mb1.5pc_alt",      0x0000, 0x020000, CRC(fbe129f0) SHA1(d65dbd129983966752d7b87e40b58c149fcc9f1f), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 21)" )
GAME_CUSTOM( 199?, m4mbel__u,    m4mbel, "mb1.5pk_alt",      0x0000, 0x020000, CRC(47194d12) SHA1(120bd07d92c684f8351920ba1ea1c2c8c0dcd9fb), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 22)" )
GAME_CUSTOM( 199?, m4mbel__v,    m4mbel, "mbl2.2b",          0x0000, 0x020000, CRC(f4930ac4) SHA1(2218764397821d2be6de02f54fd9df299c21359a), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 23)" )
GAME_CUSTOM( 199?, m4mbel__w,    m4mbel, "mbl2.1b",          0x0000, 0x020000, CRC(5cab8008) SHA1(8b69836657a756c84399dc7d867792eddbbc4d96), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 24)" )
GAME_CUSTOM( 199?, m4mbel__x,    m4mbel, "mb3.1cb",          0x0000, 0x020000, CRC(3cbf72fb) SHA1(869faded810b2be6e8cdb9f159ac659e7f2af074), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 25)" )
GAME_CUSTOM( 199?, m4mbel__y,    m4mbel, "mb3.1s",           0x0000, 0x010000, CRC(f3c122a4) SHA1(e72c389bab09baf7b03911ce172dd05e0a2f0154), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 26)" )
GAME_CUSTOM( 199?, m4mbel__z,    m4mbel, "mb31cbs",          0x0000, 0x010000, CRC(65839202) SHA1(9bc8b54e3ac3273a65fe08066e679f73cb22784e), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 27)" )
GAME_CUSTOM( 199?, m4mbel__0,    m4mbel, "mb3.1",            0x0000, 0x020000, CRC(acd2faac) SHA1(fea7e21089ebba3eb1137f86852996fc1b38a395), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 28)" )
GAME_CUSTOM( 199?, m4mbel__1,    m4mbel, "mbp0.1r",          0x0000, 0x020000, CRC(369cebf6) SHA1(a576d1b6c4a4aba6aab75dbd250fce5500d7f657), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 29)" )
GAME_CUSTOM( 199?, m4mbel__2,    m4mbel, "mbp0.2r",          0x0000, 0x020000, CRC(bd107631) SHA1(2d01a16bb339bbd1d65f83416b83d5f3f4b0fe93), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 30)" )
GAME_CUSTOM( 199?, m4mbel__3,    m4mbel, "mbpu0.3",          0x0000, 0x020000, CRC(71e6389e) SHA1(b72ea3dce8ccd83bbb03e545d0e3441bae5ff0fb), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 31)" )
GAME_CUSTOM( 199?, m4mbel__4,    m4mbel, "mbpu0.3v",         0x0000, 0x020000, CRC(50d9e901) SHA1(57e8a4271c3287b80d5db9b03a571d880f974181), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 32)" )
GAME_CUSTOM( 199?, m4mbel__5,    m4mbel, "mbpv0.3b",         0x0000, 0x020000, CRC(09f9dc28) SHA1(998d82f8cf353778f2e81b117809dc059b0a9b80), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 33)" )
GAME_CUSTOM( 199?, m4mbel__6,    m4mbel, "mbpv0.3p",         0x0000, 0x020000, CRC(2ce05ecf) SHA1(a9d9b36f28622ddb4444ec72a21fb50063b1fa7c), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 34)" )
GAME_CUSTOM( 199?, m4mbel__7,    m4mbel, "mbpu0.4",          0x0000, 0x020000, CRC(cf1deb23) SHA1(634be8ceaf2b15c02d3460cb239226c750ff7446), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 35)" )
GAME_CUSTOM( 199?, m4mbel__8,    m4mbel, "mbpu0.5",          0x0000, 0x020000, CRC(449176e4) SHA1(937f391d1bc12def836a6d128a4d15f994a198b9), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 36)" )
GAME_CUSTOM( 199?, m4mbel__9,    m4mbel, "mbp1.1r.1",        0x0000, 0x020000, CRC(ba743d25) SHA1(b2d0f3058ad202f0d58e08eb66a5ba3efc7bb67b), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 37)" )
GAME_CUSTOM( 199?, m4mbel__aa,   m4mbel, "mbp1.1r.1_alt",    0x0000, 0x020000, CRC(9b4becba) SHA1(c8f8c370dcec624bfc0f68463c0e3e317e084994), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 38)" )
GAME_CUSTOM( 199?, m4mbel__ab,   m4mbel, "mbpu11bs",         0x0000, 0x020000, CRC(e02dba46) SHA1(147bddee1c4a534efb494052f1aba0d36b904ec4), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 39)" )
GAME_CUSTOM( 199?, m4mbel__ac,   m4mbel, "mbp2.1rc.1",       0x0000, 0x020000, CRC(2a19b572) SHA1(98d7831d897f14633f8335cb343bf6d1e9d00328), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 40)" )
GAME_CUSTOM( 199?, m4mbel__ad,   m4mbel, "mbp2.1rc.1_alt",   0x0000, 0x020000, CRC(0b2664ed) SHA1(100fa8f637124ce58b5dfa431ea50e93cb4bb57e), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 41)" )
GAME_CUSTOM( 199?, m4mbel__ae,   m4mbel, "mbp2.2po",         0x0000, 0x020000, CRC(7b90836f) SHA1(fb20bf53f2d4878ebb5619637b2449b539f7d99b), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 42)" )
GAME_CUSTOM( 199?, m4mbel__af,   m4mbel, "mbplv2.2b",        0x0000, 0x020000, CRC(96ea5593) SHA1(c2208e90737a4bc15b235daa7e5a15cc7d953feb), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 43)" )
GAME_CUSTOM( 199?, m4mbel__ag,   m4mbel, "mbplv2.2b_alt",    0x0000, 0x020000, CRC(eca1600a) SHA1(ae1eb4587172c64d24737e6564bb8b423155b84d), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 44)" )
GAME_CUSTOM( 199?, m4mbel__ah,   m4mbel, "mbpv2.2b",         0x0000, 0x020000, CRC(543a28ce) SHA1(3a9d45c697b769294d5421bb467ce7436b498ec2), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 45)" )
GAME_CUSTOM( 199?, m4mbel__ai,   m4mbel, "mbpv2.2p",         0x0000, 0x020000, CRC(223b5cb8) SHA1(3196a0bc0e812b4282eb21d2f75776523ac0a17c), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 46)" )
GAME_CUSTOM( 199?, m4mbel__aj,   m4mbel, "mbplv2.3b",        0x0000, 0x020000, CRC(1fed0ec0) SHA1(a9a7f5bb56b30cae552bb7f8bccdabf1651921af), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 47)" )
GAME_CUSTOM( 199?, m4mbel__ak,   m4mbel, "mbplv2.3v",        0x0000, 0x020000, CRC(3ed2df5f) SHA1(a81dfac3c1a913c3b278a178ca653702cbfaa91d), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 48)" )
GAME_CUSTOM( 199?, m4mbel__al,   m4mbel, "mbp2.3i",          0x0000, 0x020000, CRC(8048d05f) SHA1(373a275e9c715263dac6d5bee35aa6bdf4177788), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 49)" )
GAME_CUSTOM( 199?, m4mbel__am,   m4mbel, "mbp2.3is",         0x0000, 0x010000, CRC(5fe13c3f) SHA1(bc4f609745b992f73410c8e0bc018824008773b9), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 50)" )
GAME_CUSTOM( 199?, m4mbel__an,   m4mbel, "mbp2.3pc",         0x0000, 0x020000, CRC(994ee429) SHA1(ae6c3df90a4248f0f8fb86348bba726d983cda48), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 51)" )
GAME_CUSTOM( 199?, m4mbel__ao,   m4mbel, "mbp2.3pg",         0x0000, 0x020000, CRC(a17701c0) SHA1(2587eafee1d324bd27d267d146a03eab8594bc43), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 52)" )
GAME_CUSTOM( 199?, m4mbel__ap,   m4mbel, "mbpv2.3b",         0x0000, 0x020000, CRC(dd3d739d) SHA1(1b70d71a0cf727b1ddb68d983bb0705bcc34281a), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 53)" )
GAME_CUSTOM( 199?, m4mbel__aq,   m4mbel, "mbpv2.3p",         0x0000, 0x020000, CRC(f824f17a) SHA1(c1d3224f4de47e7db7064f516c941119b4434dbc), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 54)" )
GAME_CUSTOM( 199?, m4mbel__ar,   m4mbel, "mbpu2.3",          0x0000, 0x020000, CRC(a522972b) SHA1(6692d97619250099022aa4a78259a92198c660e6), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 55)" )
GAME_CUSTOM( 199?, m4mbel__as,   m4mbel, "mbpu2.3v",         0x0000, 0x020000, CRC(841d46b4) SHA1(cc2606acf765e66ee2a8ea7d44f4339a99d1fb65), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 56)" )
GAME_CUSTOM( 199?, m4mbel__at,   m4mbel, "mbpu2.4",          0x0000, 0x020000, CRC(1bd94496) SHA1(3ce809e822796c3553cc27ab99793f2156f17e04), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 57)" )
GAME_CUSTOM( 199?, m4mbel__au,   m4mbel, "mbpu2.4v",         0x0000, 0x020000, CRC(3ae69509) SHA1(9b0f160734102d8d0c2f91c3ebe039738fe3b017), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 58)" )
GAME_CUSTOM( 199?, m4mbel__av,   m4mbel, "mbpl2.5.5",        0x0000, 0x020000, CRC(21965f32) SHA1(7b848dfd94a9c88df33e9e9858428d4f889205d6), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 59)" )
GAME_CUSTOM( 199?, m4mbel__aw,   m4mbel, "mbplv25v",         0x0000, 0x020000, CRC(00a98ead) SHA1(c7811385552e2effbe39538ef15f341504e297c6), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 60)" )
GAME_CUSTOM( 199?, m4mbel__ax,   m4mbel, "mbpu2.5",          0x0000, 0x020000, CRC(73bf7c1c) SHA1(2b92b77624c0bc6cd4986f75e143b8858f132de4), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 61)" )
GAME_CUSTOM( 199?, m4mbel__ay,   m4mbel, "mbpu2.5v",         0x0000, 0x020000, CRC(f81b831b) SHA1(734950b9fadd956bc16b927c05296a9346e0927a), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 62)" )
GAME_CUSTOM( 199?, m4mbel__az,   m4mbel, "mbpl3.2c",         0x0000, 0x020000, CRC(ea8e58fb) SHA1(186f519fd4ebfa0e61cac8f392d6253df72523ec), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 63)" ) // == m4trg__2
GAME_CUSTOM( 199?, m4mbel__a0,   m4mbel, "mbpl3.3c",         0x0000, 0x020000, CRC(638903a8) SHA1(27f0ed58d98f8f9d5c909afdc04a4570df57161f), "Avantime?","Millennium Bells (Avantime?) (MPU4) (set 64)" )

// Casino Monte Carlo
// cb apparently stands for 'credbottom' again hinting it might be a dual unit setup

#define M4CMONT_EXTRA_ROMS \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "mcsnd1.dat", 0x000000, 0x080000, CRC(9477e648) SHA1(1abefced0cf708ad035720d5e58dc7dae50de5d1) ) \
	ROM_LOAD( "mcsnd2.dat", 0x080000, 0x080000, CRC(088796bd) SHA1(877bf21add8ef95f5384a88e1287bd9aa5dbfa95) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4CMONT_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default ,ROT0,company,title,GAME_FLAGS )

// cmc
GAME_CUSTOM( 199?, m4cmont,     0,          "cmc1.7",       0x0000, 0x020000, CRC(aaebab34) SHA1(36145b7d062ad5a740bcf326933f78274b99495c), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Czech, set 1)" )
GAME_CUSTOM( 199?, m4cmont_1,   m4cmont,    "cmc1.7cb",     0x0000, 0x020000, CRC(3a862363) SHA1(439a2a65d6e90ca9c09ea1115dedab4afa23f0bc), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Czech, set 2)" )
GAME_CUSTOM( 199?, m4cmont_2,   m4cmont,    "cmc1.8",       0x0000, 0x020000, CRC(428f8019) SHA1(2e271a4fd77f4c61678dc061ca4bcd8b15221457), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Czech, set 3)" )
GAME_CUSTOM( 199?, m4cmont_3,   m4cmont,    "cmc1.8cb",     0x0000, 0x020000, CRC(d2e2084e) SHA1(83e145b2d676f67f82f45460994df0e767e814e8), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Czech, set 4)" )
GAME_CUSTOM( 199?, m4cmont_4,   m4cmont,    "cmc1.9",       0x0000, 0x020000, CRC(c5a38e96) SHA1(1e8a006c5e4aa47e9155a7fec535fe2403cf5ce7), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Czech, set 5)" ) // unpaired
GAME_CUSTOM( 199?, m4cmont_5,   m4cmont,    "cmc2.5",       0x0000, 0x020000, CRC(528fc2a1) SHA1(2e815214bf627f22fe81705a973c48ff1af6bc52), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Czech, set 6)" )
GAME_CUSTOM( 199?, m4cmont_6,   m4cmont,    "cmc2.5cb",     0x0000, 0x020000, CRC(8d808c36) SHA1(784d2024569157c1e96ef0c5ad921534a10341bf), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Czech, set 7)" )
GAME_CUSTOM( 199?, m4cmont_7,   m4cmont,    "cmc2.6",       0x0000, 0x020000, CRC(8c453974) SHA1(b8312311c637e81247682c867a30ccfca2ba2913), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Czech, set 8)" )
GAME_CUSTOM( 199?, m4cmont_8,   m4cmont,    "cmc2.6cb",     0x0000, 0x020000, CRC(65e4a71b) SHA1(23aa65bbb704c1e4f5350bcdbb5b2db1e3a9ff5c), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Czech, set 9)" )
GAME_CUSTOM( 199?, m4cmont_9,   m4cmont,    "cmc2.7",       0x0000, 0x020000, CRC(0b6937fb) SHA1(8edc3a9e8dabe4bed7bc96d68dec0e9a25526a7b), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Czech, set 10)" ) // unpaired
GAME_CUSTOM( 199?, m4cmont_10,  m4cmont,    "cmc4.1",       0x0000, 0x020000, CRC(b8a7da9f) SHA1(f4687ea3cb750393bb122b38ef3d1fe1464bb868), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Czech, set 11)" )
GAME_CUSTOM( 199?, m4cmont_11,  m4cmont,    "cmc4.1cb",     0x0000, 0x020000, CRC(28ca52c8) SHA1(f02a114fb39c34b3988672d5c7cae09d3c2b9067), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Czech, set 12)" )
GAME_CUSTOM( 199?, m4cmont_12,  m4cmont,    "cmc5.1",       0x0000, 0x020000, CRC(766d63f2) SHA1(948aa7488331552c716fec4638a4cc7404f819a3), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Czech, set 13)" )
GAME_CUSTOM( 199?, m4cmont_13,  m4cmont,    "cmc5.1cb",     0x0000, 0x020000, CRC(9fccfd9d) SHA1(2432cb2ea23e42816474ab0b4424f427fcd9d052), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Czech, set 14)" )
// cmu
GAME_CUSTOM( 199?, m4cmont_u1,  m4cmont,    "cmu1.6",       0x0000, 0x020000, CRC(f059e7f6) SHA1(49b175b60a69b813055e791bbc4b5c1ffd4cc3fd), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Ukraine, set 1)" )
GAME_CUSTOM( 199?, m4cmont_u2,  m4cmont,    "cmu1.6cb",     0x0000, 0x020000, CRC(60346fa1) SHA1(4235f8686c3172a78ec2349cf21b9cb808b605c6), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Ukraine, set 2)" )
GAME_CUSTOM( 199?, m4cmont_u3,  m4cmont,    "cmu2.1",       0x0000, 0x020000, CRC(c201c172) SHA1(f8ac227d34c9e92dcb4ea2243122691bbe122230), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Ukraine, set 3)" ) // unpaired
GAME_CUSTOM( 199?, m4cmont_u4,  m4cmont,    "cmu2.2",       0x0000, 0x020000, CRC(8b11cb80) SHA1(6a7c448496b207c5cf96e3187a18a46c66808d38), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Ukraine, set 4)" ) // unpaired
GAME_CUSTOM( 199?, m4cmont_u5,  m4cmont,    "cmu2.3",       0x0000, 0x020000, CRC(a808151e) SHA1(2eb7a56a8b43aad7721841ac9b0f8659912f8714), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Ukraine, set 5)" )
GAME_CUSTOM( 199?, m4cmont_u6,  m4cmont,    "cmu2.4",       0x0000, 0x020000, CRC(0ae2c103) SHA1(353b598b1e075f4c0b9aecd7c0ec1e57569671f7), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Ukraine, set 6)" )
GAME_CUSTOM( 199?, m4cmont_u7,  m4cmont,    "cmu2.4cb",     0x0000, 0x020000, CRC(9a8f4954) SHA1(efb9b73d58e143a7a34660650785f2d5d7d66cfe), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Ukraine, set 7)" )
GAME_CUSTOM( 199?, m4cmont_u8,  m4cmont,    "cmu2.4_alt",   0x0000, 0x020000, CRC(0a25b9d9) SHA1(4dfd34b12357e59fe088417aff3ee1ec8e39bc29), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Ukraine, set 8)" ) // wrong label?
GAME_CUSTOM( 199?, m4cmont_u9,  m4cmont,    "cmu5.1",       0x0000, 0x020000, CRC(341ebc78) SHA1(efd3d2536d415f2502dff58abe5ee941644a3efd), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Ukraine, set 9)" )
GAME_CUSTOM( 199?, m4cmont_u10, m4cmont,    "cmu5.1cb",     0x0000, 0x020000, CRC(a473342f) SHA1(7323ec880c972a9bd89fb155a2f720ced9ae2e0b), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Ukraine, set 10)" )
GAME_CUSTOM( 199?, m4cmont_u11, m4cmont,    "cmu5.2",       0x0000, 0x020000, CRC(7d0eb68a) SHA1(8d3058ae37a30262963899357344e5064911cfed), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Ukraine, set 11)" )
GAME_CUSTOM( 199?, m4cmont_u12, m4cmont,    "cmu5.2cb",     0x0000, 0x020000, CRC(ed633edd) SHA1(5c82ee6283d57f00b60f10c3ecc6f1bd6ad6657b), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Ukraine, set 12)" )
GAME_CUSTOM( 199?, m4cmont_u13, m4cmont,    "cmu5.3",       0x0000, 0x020000, CRC(5e176814) SHA1(67fc62d45f3474065f3748c90893bc79a6aa8097), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Ukraine, set 13)" )
GAME_CUSTOM( 199?, m4cmont_u14, m4cmont,    "cmu5.3cb",     0x0000, 0x020000, CRC(ce7ae043) SHA1(714ac1ea60ad5be59a7087f9a6176d770ed14c53), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Ukraine, set 14)" )
GAME_CUSTOM( 199?, m4cmont_u15, m4cmont,    "cmu5.4",       0x0000, 0x020000, CRC(4495a13b) SHA1(9b4f6c12af42dde5b850cf4b65bd3baba7d4f1a0), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Ukraine, set 15)" )
GAME_CUSTOM( 199?, m4cmont_u16, m4cmont,    "cmu5.4cb",     0x0000, 0x020000, CRC(6c574c84) SHA1(18afc1a31d3edcb7985a259cc4035bccb8527ecb), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Ukraine, set 16)" )
GAME_CUSTOM( 199?, m4cmont_u17, m4cmont,    "cmu5.4h",      0x0000, 0x020000, CRC(9b51a25a) SHA1(cec9f9d9de7422d339443f75d790b4f5123c6fa2), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Ukraine, set 17)" )
GAME_CUSTOM( 199?, m4cmont_u18, m4cmont,    "cmu5.5",       0x0000, 0x020000, CRC(fc3ac4d3) SHA1(af951b52ca73b65e664e910427fa584266f8cf83), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Ukraine, set 18)" )
GAME_CUSTOM( 199?, m4cmont_u19, m4cmont,    "cmu5.5h",      0x0000, 0x020000, CRC(53dc93aa) SHA1(0b48d905742ca464c5018a7285db4787facf937d), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Ukraine, set 19)" )
GAME_CUSTOM( 199?, m4cmont_u20, m4cmont,    "cmu5.5k",      0x0000, 0x020000, CRC(09f69f77) SHA1(567e5a8f7d92c561152a44f3f0375ee4af46cef8), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Ukraine, set 20)" )
GAME_CUSTOM( 199?, m4cmont_u21, m4cmont,    "cmu5.6h",      0x0000, 0x020000, CRC(5f2b2d76) SHA1(1d313286ad19f062b0ab80eabaa07b064737f5a8), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Ukraine, set 21)" )
GAME_CUSTOM( 199?, m4cmont_u22, m4cmont,    "cmu5.7h",      0x0000, 0x020000, CRC(ba25a1c1) SHA1(d67c870385b21902b944211944f4712036e9ec1e), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Ukraine, set 22)" )
GAME_CUSTOM( 199?, m4cmont_u23, m4cmont,    "cmu5.8k",      0x0000, 0x020000, CRC(15e0b26c) SHA1(4bc931a72bc3335eee23e986da873b73c5f40925), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Ukraine, set 23)" )
GAME_CUSTOM( 199?, m4cmont_u24, m4cmont,    "cmu5.8kp",     0x0000, 0x020000, CRC(cb73a341) SHA1(fe80260eea057d64d1dfe9edf6ea0b4e6b6cdd6a), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Ukraine, set 24)" )
GAME_CUSTOM( 199?, m4cmont_u25, m4cmont,    "cmu5.9kp",     0x0000, 0x020000, CRC(7b1e568e) SHA1(13803e136d866b9ccc7b25637a47948e8251df22), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Ukraine, set 25)" )
GAME_CUSTOM( 199?, m4cmont_u26, m4cmont,    "cmu6.1",       0x0000, 0x020000, CRC(7775e979) SHA1(6fce6d05b5c03577c7ddb858dda91caedc5de03d), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Ukraine, set 26)" ) // unpaired
GAME_CUSTOM( 199?, m4cmont_u27, m4cmont,    "cmu7.1",       0x0000, 0x020000, CRC(8dcecf8c) SHA1(729aecd339b15e6d4be4e14b81379ddc22e077cf), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Ukraine, set 27)" ) // unpaired
// cml
GAME_CUSTOM( 199?, m4cmont_l1,  m4cmont,    "cml2.1",       0x0000, 0x020000, CRC(4dbb6239) SHA1(6da8b37e477a8ef5fb130cf5a2e34bf795cb9f8d), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Latvia, set 1)" )
GAME_CUSTOM( 199?, m4cmont_l2,  m4cmont,    "cml2.1cb",     0x0000, 0x020000, CRC(ddd6ea6e) SHA1(bd39b32a32a50f72aad498ddc06eff56bab084d0), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Latvia, set 2)" )
GAME_CUSTOM( 199?, m4cmont_l3,  m4cmont,    "cml2.2",       0x0000, 0x020000, CRC(04ab68cb) SHA1(bbe7872974b2e029a2297bdcf87099abe6157a40), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Latvia, set 3)" )
GAME_CUSTOM( 199?, m4cmont_l4,  m4cmont,    "cml2.2cb",     0x0000, 0x020000, CRC(94c6e09c) SHA1(e1d91b90542ab05dc0dc1ff795cfd393145df22d), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Latvia, set 4)" )
GAME_CUSTOM( 199?, m4cmont_l5,  m4cmont,    "cml2.3",       0x0000, 0x020000, CRC(27b2b655) SHA1(1cca53b231484a0bafc90238387a0fcb64ed1ac6), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Latvia, set 5)" )
GAME_CUSTOM( 199?, m4cmont_l6,  m4cmont,    "cml2.3cb",     0x0000, 0x020000, CRC(b7df3e02) SHA1(8347e5612c5db0ac4a2276e9e26569171ddbc793), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Latvia, set 6)" )
GAME_CUSTOM( 199?, m4cmont_l7,  m4cmont,    "cml2.4",       0x0000, 0x020000, CRC(7dd24473) SHA1(8806f4ceefdbd45eb4da6dd61d2080685784a3f3), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Latvia, set 7)" )
GAME_CUSTOM( 199?, m4cmont_l8,  m4cmont,    "cml2.4cb",     0x0000, 0x020000, CRC(15f292c5) SHA1(e24b59e40cf4e4bdc86be1fe67bb5e274aae11ad), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Latvia, set 8)" )
GAME_CUSTOM( 199?, m4cmont_l9,  m4cmont,    "cml5.1",       0x0000, 0x020000, CRC(05d0d872) SHA1(c7658a2385756aff22594c9f8e8a3618188f476c), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Latvia, set 9)" )
GAME_CUSTOM( 199?, m4cmont_l10, m4cmont,    "cml5.1cb",     0x0000, 0x020000, CRC(95bd5025) SHA1(5e6862af044ba0d5265b6acaa418b2b8b27e8bd4), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Latvia, set 10)" )
GAME_CUSTOM( 199?, m4cmont_l11, m4cmont,    "cml5.2",       0x0000, 0x020000, CRC(4cc0d280) SHA1(547218c17d46b2387221bb2d6f31b3651499a36d), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Latvia, set 11)" )
GAME_CUSTOM( 199?, m4cmont_l12, m4cmont,    "cml5.2cb",     0x0000, 0x020000, CRC(dcad5ad7) SHA1(de03bbd58e7514b73bac6afb6b89f5d2e7ec013b), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Latvia, set 12)" )
GAME_CUSTOM( 199?, m4cmont_l13, m4cmont,    "cml5.3",       0x0000, 0x020000, CRC(6fd90c1e) SHA1(e675095e55b57dc27023be1fbfafeedd993333b7), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Latvia, set 13)" )
GAME_CUSTOM( 199?, m4cmont_l14, m4cmont,    "cml5.3cb",     0x0000, 0x020000, CRC(ffb48449) SHA1(eb90a05cfcf3cd8492b6e89f7545a5edaa009824), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Latvia, set 14)" )
GAME_CUSTOM( 199?, m4cmont_l15, m4cmont,    "cml5.4",       0x0000, 0x020000, CRC(cdf4a0d9) SHA1(d68afa7d9a1f04e0784e14b10e3d6df3123e8724), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Latvia, set 15)" )
GAME_CUSTOM( 199?, m4cmont_l16, m4cmont,    "cml5.4cb",     0x0000, 0x020000, CRC(5d99288e) SHA1(dc69954a93cdf38232ef1cd071c3fe3ca37e4a04), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Latvia, set 16)" )
// cmr
GAME_CUSTOM( 199?, m4cmont_r1,  m4cmont,    "cmr1.1",       0x0000, 0x020000, CRC(b7f79cb3) SHA1(a3881ddefedbf08195dcffa9fabc71021e9aeb81), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Russia, set 1)" )
GAME_CUSTOM( 199?, m4cmont_r2,  m4cmont,    "cmr1.1cb",     0x0000, 0x020000, CRC(279a14e4) SHA1(6e123c6167d4e27184c8ba7c4de5645f3d0499e9), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Russia, set 2)" )
GAME_CUSTOM( 199?, m4cmont_r3,  m4cmont,    "cmr1.2",       0x0000, 0x020000, CRC(fee79641) SHA1(546c73af55ba070f756d204749a0b346cfa6f2a9), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Russia, set 3)" )
GAME_CUSTOM( 199?, m4cmont_r4,  m4cmont,    "cmr1.2cb",     0x0000, 0x020000, CRC(6e8a1e16) SHA1(df369265d4bd50a83fe617f85cb7e80e0e046828), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Russia, set 4)" )
GAME_CUSTOM( 199?, m4cmont_r5,  m4cmont,    "cmr1.3",       0x0000, 0x020000, CRC(ddfe48df) SHA1(1ce72fbbac81d806c6823a2149cf217a3e417d7d), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Russia, set 5)" )
GAME_CUSTOM( 199?, m4cmont_r6,  m4cmont,    "cmr1.3cb",     0x0000, 0x020000, CRC(4d93c088) SHA1(1b75b663f48d3e43e7e02d9c6afcfcde4813d7fc), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Russia, set 6)" )
GAME_CUSTOM( 199?, m4cmont_r7,  m4cmont,    "cmr1.4",       0x0000, 0x020000, CRC(7fd3e418) SHA1(893afea31d48995f2805161a0734c870432eee03), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Russia, set 7)" )
GAME_CUSTOM( 199?, m4cmont_r8,  m4cmont,    "cmr1.4cb",     0x0000, 0x020000, CRC(8003f28d) SHA1(17b7469682ca7da570897f3708dfd901dd7ff9b4), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (Russia, set 8)" )
// misc (wrong game?)
GAME_CUSTOM( 199?, m4cmont_gt1, m4cmont,    "gtr5.8g1",     0x0000, 0x020000, CRC(2fb54fd3) SHA1(09e93fc45ab15a655f953e2ba86411034260dfc6), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (GTR, set 1)" )
GAME_CUSTOM( 199?, m4cmont_gt2, m4cmont,    "gtr5.8g2",     0x0000, 0x020000, CRC(f15e032c) SHA1(f44120c0635e5b1726188f52f619c57a3a6766fa), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (GTR, set 2)" )
GAME_CUSTOM( 199?, m4cmont_gt3, m4cmont,    "gtr58gaa",     0x0000, 0x020000, CRC(ad20af25) SHA1(1a5ac760894f4441d11f0974f1d0f90fabd76bf2), "Avantime?","Casino Monte Carlo (Avantime?) (MPU4) (GTR, set 3)" )


#define M4BLKMGC_EXTRA_ROMS \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "bmsnd", 0x0000, 0x080000, CRC(81da8bc9) SHA1(a60fd689e2683b987f48e3a4c8817b169a9c3fdf) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4BLKMGC_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default ,ROT0,company,title,GAME_FLAGS )


// new - latvia
GAME_CUSTOM( 199?, m4blkmgc,       0,          "bml1a305.bin", 0x0000, 0x010000, CRC(429e2642) SHA1(9e6a0ea07adaa7a2327b2ba3706f1966fed0de36), "Avantime?","Black Magic (Avantime?) (MPU4) (Latvia, set 1)" )
GAME_CUSTOM( 199?, m4blkmgc_1,     m4blkmgc,   "bml1b305.bin", 0x0000, 0x010000, CRC(fa9e4567) SHA1(7401103c40d8efd2427953a1e398c158d6f08a64), "Avantime?","Black Magic (Avantime?) (MPU4) (Latvia, set 2)" )
// new - ukraine
GAME_CUSTOM( 199?, m4blkmgc_u1,    m4blkmgc,   "bmu1a306.bin", 0x0000, 0x010000, CRC(4c3ef73a) SHA1(fa1be29e7a5240e6dfefec61d77c3ec78f5f7b11), "Avantime?","Black Magic (Avantime?) (MPU4) (Ukraine, set 1)" )
GAME_CUSTOM( 199?, m4blkmgc_u2,    m4blkmgc,   "bmu1b306.bin", 0x0000, 0x010000, CRC(f1ec2c3e) SHA1(ac776c2020409485265d227fe39f0ca31f23ddd7), "Avantime?","Black Magic (Avantime?) (MPU4) (Ukraine, set 2)" )
GAME_CUSTOM( 199?, m4blkmgc_u3,    m4blkmgc,   "bfu1a307.bin", 0x0000, 0x010000, CRC(68aa272c) SHA1(9f476d55a09776a621e58afddf2e3f618b278374), "Avantime?","Black Magic (Avantime?) (MPU4) (Ukraine, set 3)" )
GAME_CUSTOM( 199?, m4blkmgc_u4,    m4blkmgc,   "bfu1b307.bin", 0x0000, 0x010000, CRC(9c876530) SHA1(1fb95591c6b0a43aa66c7a036268e19cf27e5535), "Avantime?","Black Magic (Avantime?) (MPU4) (Ukraine, set 4)" )
GAME_CUSTOM( 199?, m4blkmgc_u5,    m4blkmgc,   "bau1a307.bin", 0x0000, 0x010000, CRC(41af6959) SHA1(81a4c4db5dd696bfaf7f36455b3970a2d652ab08), "Avantime?","Black Magic (Avantime?) (MPU4) (Ukraine, set 5)" )
GAME_CUSTOM( 199?, m4blkmgc_u6,    m4blkmgc,   "bau1b307.bin", 0x0000, 0x010000, CRC(1405a02b) SHA1(82504289e72900d3315f46ed31aeabc71b2c5b9e), "Avantime?","Black Magic (Avantime?) (MPU4) (Ukraine, set 6)" )



#define M4AMALAD_EXTRA_ROMS \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "aasnd", 0x0000, 0x080000, CRC(6b78f3de) SHA1(4f10afdc5cf7c84e2d048f7c9c5f83323f1e5a6e) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4AMALAD_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4amalad,       0,          "aag1.4",           0x0000, 0x020000, CRC(0da943a1) SHA1(50915ce67687f15a36b5c38b1c1c6773bd3ecf9f), "Avantime?","American Aladdin (Avantime?) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4amalad__a,    m4amalad,   "aag1.3",           0x0000, 0x020000, CRC(3a9552a9) SHA1(7695899b6ed52d0c7530b5ed0829cfdbb3892fa2), "Avantime?","American Aladdin (Avantime?) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4amalad__b,    m4amalad,   "aag1.2",           0x0000, 0x020000, CRC(631d4be8) SHA1(207655c2d4a5500631ed4df8db5625177b5d4d12), "Avantime?","American Aladdin (Avantime?) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4amalad__c,    m4amalad,   "aag1.1",           0x0000, 0x020000, CRC(5e426c67) SHA1(6a59996998eff1870d7492afa1056ca77ec3f281), "Avantime?","American Aladdin (Avantime?) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4amalad__d,    m4amalad,   "aav1.2b",          0x0000, 0x020000, CRC(81905010) SHA1(25866a2f8072031facdd69a07b4c1a46ba560d36), "Avantime?","American Aladdin (Avantime?) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4amalad__e,    m4amalad,   "aav1.2s",          0x0000, 0x010000, CRC(3bfb0faa) SHA1(3a4fe6d47e995f40cb03984dbd1fc98669513b4a), "Avantime?","American Aladdin (Avantime?) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4amalad__f,    m4amalad,   "aav1.1b",          0x0000, 0x020000, CRC(cb8a3385) SHA1(39e2df941977faa1afc88a6189f8b35e8f605c95), "Avantime?","American Aladdin (Avantime?) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4amalad__g,    m4amalad,   "aav1.1s",          0x0000, 0x010000, CRC(2bb83dee) SHA1(06ce5640c76c95ba9a9178b5f75e2cb3b358b4ed), "Avantime?","American Aladdin (Avantime?) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4amalad__h,    m4amalad,   "aalatv1.1b",       0x0000, 0x020000, CRC(3af6291f) SHA1(ca09f66a8c0f0e1c1dadac096c5cbf738ceaf9be), "Avantime?","American Aladdin (Avantime?) (MPU4) (set 9)" )
GAME_CUSTOM( 199?, m4amalad__i,    m4amalad,   "aalatv1.1s",       0x0000, 0x010000, CRC(582bd6ea) SHA1(8c353a0089f5d796fed2b381429a6795eb9e9b11), "Avantime?","American Aladdin (Avantime?) (MPU4) (set 10)" )
GAME_CUSTOM( 199?, m4amalad__j,    m4amalad,   "lglv2.2b",         0x0000, 0x020000, CRC(04015f49) SHA1(23f252c129a54ae7ffe99682c71e14b4b27d465b), "Avantime?","American Aladdin (Avantime?) (MPU4) (set 11)" )
GAME_CUSTOM( 199?, m4amalad__k,    m4amalad,   "lgv1.1b",          0x0000, 0x020000, CRC(1113f54a) SHA1(9cf4b50b4e24be0c31e325fdb02a82e93c31b93d), "Avantime?","American Aladdin (Avantime?) (MPU4) (set 12)" )
GAME_CUSTOM( 199?, m4amalad__l,    m4amalad,   "lgv1.1s",          0x0000, 0x010000, CRC(3e3f9615) SHA1(717e0f371625b44b09be8567bc8a4a230899bcbc), "Avantime?","American Aladdin (Avantime?) (MPU4) (set 13)" )
GAME_CUSTOM( 199?, m4amalad__m,    m4amalad,   "jpaltest.dat",     0x0000, 0x020000, CRC(0d50228b) SHA1(884a4271899d6bb4eadd7315f20146a08e21c5c9), "Avantime?","American Aladdin (Avantime?) (MPU4) (set 14)" )
GAME_CUSTOM( 199?, m4amalad__n,    m4amalad,   "jpaltst2.dat",     0x0000, 0x020000, CRC(872728ce) SHA1(eeab5487e6ce12e239ce6143c0e761737e56bcbe), "Avantime?","American Aladdin (Avantime?) (MPU4) (set 15)" )




#define M4BBEN_EXTRA_ROMS \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "bben1s.bin", 0x0000, 0x02dbc5, CRC(9240317e) SHA1(d9167e52a09ff1783bb10e2e34fb80bdf555f00e) )
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4BBEN_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default ,ROT0,company,title,GAME_FLAGS )


GAME_CUSTOM( 199?, m4bben,       0,      "bbc1a102.bin", 0x0000, 0x010000, CRC(c5010bb6) SHA1(f39ab219eafaa391b5b777c2918f059ba67b4504), "Avantime?","Big Ben (Avantime?) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4bben__a,    m4bben, "bbc1b102.bin", 0x0000, 0x010000, CRC(9eb20181) SHA1(c183e3eab84019b6acb5040ef8d5aa238b914e78), "Avantime?","Big Ben (Avantime?) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4bben__b,    m4bben, "bbc1a103.bin", 0x0000, 0x010000, CRC(34568c49) SHA1(57a772595e258b6e1a41145e4dcaa0486c7d91ae), "Avantime?","Big Ben (Avantime?) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4bben__c,    m4bben, "bbc1b103.bin", 0x0000, 0x010000, CRC(587d1f54) SHA1(c6d5657958f029d38d0b1244032634bd023bf48d), "Avantime?","Big Ben (Avantime?) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4bben__d,    m4bben, "bbc1a104.bin", 0x0000, 0x010000, CRC(ac5d175e) SHA1(0772dcb362c732760d96b9876898cfd5a1038720), "Avantime?","Big Ben (Avantime?) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4bben__e,    m4bben, "bbc1b104.bin", 0x0000, 0x010000, CRC(ac0ba269) SHA1(de9b00f32afc7cabcf6400e3b063f844b1b03f28), "Avantime?","Big Ben (Avantime?) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4bben__f,    m4bben, "bbc1a105.bin", 0x0000, 0x010000, CRC(cece9ee3) SHA1(582def521d5583ab9fc9adeedcdd09e2ab4876b8), "Avantime?","Big Ben (Avantime?) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4bben__g,    m4bben, "bbc1b105.bin", 0x0000, 0x010000, CRC(b31f3989) SHA1(5d5a7fc49486b4f7e9a31a4a469913174ca2cb5b), "Avantime?","Big Ben (Avantime?) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4bben__h,    m4bben, "bbc2a106.bin", 0x0000, 0x010000, CRC(4929273e) SHA1(0703b89884a2d410854d813fe8304eac0dbfb30e), "Avantime?","Big Ben (Avantime?) (MPU4) (set 9)" )
GAME_CUSTOM( 199?, m4bben__i,    m4bben, "bbc2b106.bin", 0x0000, 0x010000, CRC(d06687ed) SHA1(2e5b32c772f47f3f10ee90cbc72c00995871c2c0), "Avantime?","Big Ben (Avantime?) (MPU4) (set 10)" )




#define M4BBOX_EXTRA_ROMS \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "bb1snd.bin", 0x0000, 0x068880, CRC(69d53f5a) SHA1(dd7958060804fda97a1fdf69c230bfab092b9707) )
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4BBOX_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default ,ROT0,company,title,GAME_FLAGS )


GAME_CUSTOM( 199?, m4bbox,       0,      "bbb1.bin",     0x0000, 0x010000, CRC(b668e08e) SHA1(f401405419689ea5ad06dfd815aaef9e1b7ed4e1), "Avantime?","Brain Box (Avantime?) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4bbox__a,    m4bbox, "bbb13.bin",    0x0000, 0x010000, CRC(9601d921) SHA1(e179b5155070af880d10a64d44454d84ec329800), "Avantime?","Brain Box (Avantime?) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4bbox__b,    m4bbox, "bbb2.bin",     0x0000, 0x010000, CRC(1bcf9f83) SHA1(f5d2a352e79d1d2694b3c854a0a532662173416a), "Avantime?","Brain Box (Avantime?) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4bbox__c,    m4bbox, "bbb3.bin",     0x0000, 0x010000, CRC(21608fa5) SHA1(ca565442f8d6d8a8dfca847b5a638551ce15cf07), "Avantime?","Brain Box (Avantime?) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4bbox__d,    m4bbox, "bbb5.bin",     0x0000, 0x010000, CRC(4c0384f3) SHA1(c1ed2171cbe781d3b1842d5ed999e2d601ccf0c4), "Avantime?","Brain Box (Avantime?) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4bbox__e,    m4bbox, "bbb6.bin",     0x0000, 0x010000, CRC(5a4d4ca4) SHA1(127b961ecfb61ec2a201ec736d2c677b0e894b0b), "Avantime?","Brain Box (Avantime?) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4bbox__f,    m4bbox, "bbb7.bin",     0x0000, 0x010000, CRC(c0c0de55) SHA1(d2140ba83b1210a0d3061d23b9e8e6b2b59f7f0e), "Avantime?","Brain Box (Avantime?) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4bbox__g,    m4bbox, "bbb8.bin",     0x0000, 0x010000, CRC(67373388) SHA1(e4f0907783cb4305fea6ea4591bef7f333a5041e), "Avantime?","Brain Box (Avantime?) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4bbox__h,    m4bbox, "loreel2.bin",  0x0000, 0x010000, CRC(77c1d85c) SHA1(8cfc12c8814b42003cfe85e170426e224660b4fb), "Avantime?","Brain Box (Avantime?) (MPU4) (set 9)" )
GAME_CUSTOM( 199?, m4bbox__i,    m4bbox, "bbb9.bin",     0x0000, 0x010000, CRC(191f3da0) SHA1(8ee7cf349d97b1819ddb99a6dc91c0a364597e9f), "Avantime?","Brain Box (Avantime?) (MPU4) (set 10)" )




#define M4FRNUDG_EXTRA_ROMS \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4FRNUDG_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4frnudg,       0,          "fanhc11.bin",  0x0000, 0x010000, CRC(d2fe9df9) SHA1(7b519e4ed447f8c59fda972fc398f6ff423a8f92), "Avantime?","Fruit & Nudge (Avantime?) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4frnudg__a,    m4frnudg,   "fanhc12.bin",  0x0000, 0x010000, CRC(2b2176e0) SHA1(9acffc05c02e76a6cbe4cfd708ff2d94ecb6c308), "Avantime?","Fruit & Nudge (Avantime?) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4frnudg__b,    m4frnudg,   "fanhc8.bin",   0x0000, 0x010000, CRC(d1eecc15) SHA1(7dd3d218bc42ed7a92b8cd04b3de0eecda1d7eb0), "Avantime?","Fruit & Nudge (Avantime?) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4frnudg__c,    m4frnudg,   "fanhc9.bin",   0x0000, 0x010000, CRC(0b581d12) SHA1(e62c831d8d21eb3c9a15ebe7f0245e804f912e32), "Avantime?","Fruit & Nudge (Avantime?) (MPU4) (set 4)" )


#define M4FRMTX_EXTRA_ROMS \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "fmsnd1.bin", 0x0000, 0x080000, CRC(300fcb23) SHA1(c3a7424089e7893972e04a6a5e77cfb4e0ffc8ec)) \
	ROM_REGION( 0x100000, "pals", 0 ) \
	ROM_LOAD( "fm.jed", 0x0000, 0x000580, CRC(dc166c8e) SHA1(0cc49836b7ad57daa54e08f10f07aa279ccc53a0))
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4FRMTX_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default ,ROT0,company,title,GAME_FLAGS )
GAME_CUSTOM( 199?, m4frmtx,     0,          "fm1.bin",      0x0000, 0x010000, CRC(0d11ffee) SHA1(50a3f97cf76855ba503a833e4198c154a57b6847), "Avantime?","Fruit Matrix (Avantime?) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4frmtx__a,  m4frmtx,    "fm2.bin",      0x0000, 0x010000, CRC(ab143a49) SHA1(c4ba0671b154707fd69d58fd3bf65f5ba4d2bf53), "Avantime?","Fruit Matrix (Avantime?) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4frmtx__b,  m4frmtx,    "fm4.bin",      0x0000, 0x010000, CRC(cbe09e1d) SHA1(00d17c6a189ac1a60ec9acbe2babb5a69dec3711), "Avantime?","Fruit Matrix (Avantime?) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4frmtx__c,  m4frmtx,    "fm6.bin",      0x0000, 0x010000, CRC(a6180a22) SHA1(572869b407e6cf048c045562144f703b7a91893f), "Avantime?","Fruit Matrix (Avantime?) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4frmtx__d,  m4frmtx,    "fm7.bin",      0x0000, 0x010000, CRC(7607746e) SHA1(8697842d643cb31bafdee42f5c9cebffbcfac850), "Avantime?","Fruit Matrix (Avantime?) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4frmtx__e,  m4frmtx,    "fmuk1.bin",    0x0000, 0x010000, CRC(1e798ba4) SHA1(e8d06f8281bc9bc52c7ddd3133abf7eaa231d731), "Avantime?","Fruit Matrix (Avantime?) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4frmtx__f,  m4frmtx,    "frmatx3",      0x0000, 0x010000, CRC(d3c47cad) SHA1(e89d8df496405903f51c31bf48774bfa877d90c2), "Avantime?","Fruit Matrix (Avantime?) (MPU4) (set 7)" )




#define M4JOK2K_EXTRA_ROMS \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4JOK2K_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default ,ROT0,company,title,GAME_FLAGS )
// latvia
GAME_CUSTOM( 199?, m4jok2k,     0,          "j300 1.512",       0x0000, 0x010000, CRC(3a52da14) SHA1(b2b7f32093938f8b793fa4c425cd5beaeaa83286), "Avantime?","Joker 2000 (Avantime?) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4jok2k__a,  m4jok2k,    "j300 2.512",       0x0000, 0x010000, CRC(6d3c6b99) SHA1(b5f34043e3cc93908b7ff969ae2546862a3a79ce), "Avantime?","Joker 2000 (Avantime?) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4jok2k__b,  m4jok2k,    "joker2000.dat",    0x0000, 0x020000, CRC(339c5b70) SHA1(17d8ad7a4458780c574ca8b1e30c63d5749d5ec1), "Avantime?","Joker 2000 (Avantime?) (MPU4) (set 3)" )





#define M4MJP_EXTRA_ROMS \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "mjsnd1.bin", 0x0000, 0x080000, CRC(e9b62ebb) SHA1(d930dffdb933f3359ae210d2c1ab5ada9964c398) )
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4MJP_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4mjp,     0,      "code.bin",     0x0000, 0x020000, CRC(c57d0148) SHA1(4d21a501ea64eb4fcf22ae9fba81b8982a55730e), "Avantime?","Mega Jackpot (Avantime?) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4mjp__a,  m4mjp,  "mjl02.bin",    0x0000, 0x010000, CRC(014d1765) SHA1(5cc4039e574b3a68be6d639453b217ce9a0841f0), "Avantime?","Mega Jackpot (Avantime?) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4mjp__b,  m4mjp,  "mju02.bin",    0x0000, 0x010000, CRC(22955f46) SHA1(abeb74772f5bcde8d827ec6f5d47dfb6153176db), "Avantime?","Mega Jackpot (Avantime?) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4mjp__c,  m4mjp,  "mjl03.bin",    0x0000, 0x010000, CRC(0d66868b) SHA1(f90aebfa1024510615ae45436923fc4e4b4004b1), "Avantime?","Mega Jackpot (Avantime?) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4mjp__d,  m4mjp,  "mjl04.bin",    0x0000, 0x010000, CRC(f6eb4d2a) SHA1(09ea2e67fca9b198ecef989a92cf95dcbb2b3895), "Avantime?","Mega Jackpot (Avantime?) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4mjp__e,  m4mjp,  "mju03.bin",    0x0000, 0x010000, CRC(c6b69d82) SHA1(3128550f23bc4398098583f3fd88e1056f18ca3c), "Avantime?","Mega Jackpot (Avantime?) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4mjp__f,  m4mjp,  "mju04.bin",    0x0000, 0x010000, CRC(c493b819) SHA1(9fc95282691cc217d252148db6e2efa8dcff31ee), "Avantime?","Mega Jackpot (Avantime?) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4mjp__g,  m4mjp,  "tl01.bin",     0x0000, 0x010000, CRC(a36a320f) SHA1(5b074d620484c168ce969082aae4051b7a41f64a), "Avantime?","Mega Jackpot (Avantime?) (MPU4) (set 8)" )



#define M4MILROU_EXTRA_ROMS \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4MILROU_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4milrou,       0,          "nonp1.dat",    0x0000, 0x020000, CRC(b7503d57) SHA1(dc0ba6073ae278b8406cc7e30e4b4fed21df61c8), "Avantime?","Millennium Roulette (Avantime?) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4milrou__a,    m4milrou,   "p1.dat",       0x0000, 0x020000, CRC(b3f4a2b0) SHA1(01ee5ef5eea0f83791d68b2d9bdb5be6c6495a28), "Avantime?","Millennium Roulette (Avantime?) (MPU4) (set 2)" )




#define M4KINGG_EXTRA_ROMS \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4KINGG_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4kingg,     0,          "kgiha101.bin", 0x0000, 0x010000, CRC(4da8bea1) SHA1(63fbad82877f772f3f559026eaddb69e09d6556b), "Avantime?","King George (Avantime?) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4kingg__a,  m4kingg,    "kgiha102.bin", 0x0000, 0x010000, CRC(573687b0) SHA1(eafc90120297201fc1ba40029b592ec25d972690), "Avantime?","King George (Avantime?) (MPU4) (set 2)" )
