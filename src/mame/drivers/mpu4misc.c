// license:BSD-3-Clause
// copyright-holders:David Haywood
/* MPU4 sets from various other manufactuers which appear to be based off unique code / behaviors (no barcrest headers etc.) */

#include "emu.h"
#include "includes/mpu4.h"

MACHINE_CONFIG_EXTERN( mod4oki );
INPUT_PORTS_EXTERN( mpu4 );

#define GAME_FLAGS (MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL)



ROM_START( m4bangin )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "bang.hex", 0x0000, 0x020000, CRC(e40f21d3) SHA1(62319967882f01bbd4d10bca52daffd2fe3ec03a) )
	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "bang.chr", 0x0000, 0x000048, CRC(aacbab22) SHA1(1f394b8947486f319743c0703884ecd35214c433) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "bangs1.hex", 0x000000, 0x080000, CRC(aa460a90) SHA1(e5e80d8b14bd976ed104e376c2e6f995870b0d77) )
	ROM_LOAD( "bangs2.hex", 0x080000, 0x080000, CRC(518ebd38) SHA1(6eaaf0cb34dd430f16b88f9d1ed97d6fb59d00ea) )
	ROM_LOAD( "bangs3.hex", 0x100000, 0x080000, CRC(2da78a75) SHA1(95975bc76fc32d05bd998bb75dcafc6eef7661b3) )
ROM_END

ROM_START( m4bangina )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "bang1-5n.p1", 0x0000, 0x020000, CRC(dabb462a) SHA1(c02fa204bfab07d5edbc784ccaca50f119ce8d5a) )
	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "bang.chr", 0x0000, 0x000048, CRC(aacbab22) SHA1(1f394b8947486f319743c0703884ecd35214c433) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "bangs1.hex", 0x000000, 0x080000, CRC(aa460a90) SHA1(e5e80d8b14bd976ed104e376c2e6f995870b0d77) )
	ROM_LOAD( "bangs2.hex", 0x080000, 0x080000, CRC(518ebd38) SHA1(6eaaf0cb34dd430f16b88f9d1ed97d6fb59d00ea) )
	ROM_LOAD( "bangs3.hex", 0x100000, 0x080000, CRC(2da78a75) SHA1(95975bc76fc32d05bd998bb75dcafc6eef7661b3) )
ROM_END

ROM_START( m4banginb )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "bang1-5p.p1", 0x0000, 0x020000, CRC(84bb8da8) SHA1(ae601957a3cd0b7e4b176987a5592d0b7c9be19d) )
	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "bang.chr", 0x0000, 0x000048, CRC(aacbab22) SHA1(1f394b8947486f319743c0703884ecd35214c433) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "bangs1.hex", 0x000000, 0x080000, CRC(aa460a90) SHA1(e5e80d8b14bd976ed104e376c2e6f995870b0d77) )
	ROM_LOAD( "bangs2.hex", 0x080000, 0x080000, CRC(518ebd38) SHA1(6eaaf0cb34dd430f16b88f9d1ed97d6fb59d00ea) )
	ROM_LOAD( "bangs3.hex", 0x100000, 0x080000, CRC(2da78a75) SHA1(95975bc76fc32d05bd998bb75dcafc6eef7661b3) )
ROM_END


#define M4WWC_SOUND \
	ROM_REGION( 0x180000, "altmsm6376", 0 ) \
	/* 2 sets of sound roms, one contains an extra sample */ \
	ROM_LOAD( "wacky1.hex", 0x000000, 0x080000, CRC(379d7af6) SHA1(3b1988c1ab570c075572d0e9bf03fcb331ea4a2c) ) \
	/* rom 2? should it match? */ \
	ROM_LOAD( "wacky3.hex", 0x100000, 0x080000, CRC(c7def11a) SHA1(6aab2b7f7e4c852891ee09e91a8a085e9b28803f) ) \
	ROM_REGION( 0x180000, "msm6376", 0 ) \
	ROM_LOAD( "wacky1snd.bin", 0x000000, 0x080000, CRC(45d6869a) SHA1(c1294522d190d22852b5c6006c92911f9e89cfac) ) \
	ROM_LOAD( "wacky2snd.bin", 0x080000, 0x080000, CRC(18b5f8c8) SHA1(e4dc312eea777c2375ba8c2be2f3c2be71bea5c4) ) \
	ROM_LOAD( "wacky3snd.bin", 0x100000, 0x080000, CRC(0516acad) SHA1(cfecd089c7250cb19c9e4ca251591f820acefd88) )

ROM_START( m4wwc )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "wack1-9n.p1", 0x0000, 0x020000, CRC(7ba6fd92) SHA1(3a5c7f9b3ebd8593c76132b46163c9d1299e210e) )
	M4WWC_SOUND
ROM_END

ROM_START( m4wwca )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "wack1-9p.p1", 0x0000, 0x020000, CRC(4046b5eb) SHA1(e1ec9158810387b41b574202e9f27e7b741ac81c) )
	M4WWC_SOUND
ROM_END

ROM_START( m4wwcb )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "wacky.hex", 0x0000, 0x020000, CRC(a94a06fd) SHA1(a5856b6903fdd35f9dca19b114ca56c106a308f2) )
	M4WWC_SOUND
ROM_END




ROM_START( m4screw )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "scre0-8n.p1", 0x0000, 0x020000, CRC(5e07b33a) SHA1(6e8835edb61bd0777751bfdfe66d729554a9d6eb) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "screwsnd.p1", 0x000000, 0x080000, CRC(6fe4888c) SHA1(b02b7f322d22080123e8b18326910031aa9d39b4) )
	ROM_LOAD( "screwsnd.p2", 0x080000, 0x080000, CRC(29e842ee) SHA1(3325b137361c69244fffaa0d0e39e60106eaa5f9) )
	ROM_LOAD( "screwsnd.p3", 0x100000, 0x080000, CRC(91ef193f) SHA1(a356642ae1093cf69486c434673531042ae27be7) )
ROM_END

ROM_START( m4screwp )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "scre0-8p.p1", 0x0000, 0x020000, CRC(34a70a77) SHA1(f76de47f6919d380eb0d0eeffc0e5dda72345038) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "screwsnd.p1", 0x000000, 0x080000, CRC(6fe4888c) SHA1(b02b7f322d22080123e8b18326910031aa9d39b4) )
	ROM_LOAD( "screwsnd.p2", 0x080000, 0x080000, CRC(29e842ee) SHA1(3325b137361c69244fffaa0d0e39e60106eaa5f9) )
	ROM_LOAD( "screwsnd.p3", 0x100000, 0x080000, CRC(91ef193f) SHA1(a356642ae1093cf69486c434673531042ae27be7) )
ROM_END

ROM_START( m4screwa )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "screwinaroundv0-7.bin", 0x0000, 0x020000, CRC(78a1e3ca) SHA1(a3d6e76a474a3a5cd74e4b527aa575f21825a7aa) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "screwsnd.p1", 0x000000, 0x080000, CRC(6fe4888c) SHA1(b02b7f322d22080123e8b18326910031aa9d39b4) )
	ROM_LOAD( "screwsnd.p2", 0x080000, 0x080000, CRC(29e842ee) SHA1(3325b137361c69244fffaa0d0e39e60106eaa5f9) )
	ROM_LOAD( "screwsnd.p3", 0x100000, 0x080000, CRC(91ef193f) SHA1(a356642ae1093cf69486c434673531042ae27be7) )
ROM_END

ROM_START( m4screwb )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "screw0_5.p1", 0x000000, 0x020000, CRC(4c0e8300) SHA1(1fea75f3cb1a96c14bd0e56a95bafd22996d002d) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "screwsnd.p1", 0x000000, 0x080000, CRC(6fe4888c) SHA1(b02b7f322d22080123e8b18326910031aa9d39b4) )
	ROM_LOAD( "screwsnd.p2", 0x080000, 0x080000, CRC(29e842ee) SHA1(3325b137361c69244fffaa0d0e39e60106eaa5f9) )
	ROM_LOAD( "screwsnd.p3", 0x100000, 0x080000, CRC(91ef193f) SHA1(a356642ae1093cf69486c434673531042ae27be7) )
ROM_END

ROM_START( m4vfm )
	ROM_REGION( 0x080000, "maincpu", 0 )
	ROM_LOAD( "v_f_mon", 0x0000, 0x020000, CRC(e4add02c) SHA1(5ef1bdd532ef0801b96ceae941f3da789039811c) )
	ROM_REGION( 0x080000, "msm6376", ROMREGION_ERASE00 )
ROM_END

#define M4JIGGIN_SOUND \
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 ) \
	ROM_LOAD( "jigsnd1.oki", 0x000000, 0x080000, CRC(581fa143) SHA1(e35186597fc7932d306080ecc82c55af4b769367) ) \
	ROM_LOAD( "jigsnd2.oki", 0x080000, 0x080000, CRC(34c6fc3a) SHA1(6bfe52a94d8bed5b30d9ed741db7816ddc712aa3) )

ROM_START( m4jiggin )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "jig2-1n.p1", 0x0000, 0x010000, CRC(9ea16d00) SHA1(4b4f1519eb6565ce76665595154c58cd0d0ab6fd) )
	M4JIGGIN_SOUND
ROM_END

ROM_START( m4jiggina )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "jig2-1p.p1", 0x0000, 0x010000, CRC(09e6e111) SHA1(800a1dbc64c6a631cf3e53bd5f17b5d56955c92e) )
	M4JIGGIN_SOUND
ROM_END


#define M4DCRLS_EXTRA_ROMS \
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 ) \
	ROM_LOAD( "dcr_sounds.bin", 0x0000, 0x09664e, CRC(431cecbc) SHA1(b564ae8d083fef84328526192626a220e979d5ad) ) /* intelhex */ \
	ROM_LOAD( "71000110.bin", 0x0000, 0x080000, CRC(0373a197) SHA1(b32bf521e36b5a53170d3a6ec545ce8db3a5094d) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4DCRLS_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default_big ,ROT0,company,title,GAME_FLAGS )


GAME_CUSTOM( 199?, m4dcrls,         0,          "70000116.bin",                 0x0000, 0x040000, CRC(27e5ad77) SHA1(83cabd8b52efc6c0d5530b55683295208f64abb6), "Qps","Double Crazy Reels (Qps) (MPU4) (set 1)" ) // dcr_std_340.bin
GAME_CUSTOM( 199?, m4dcrls__a,      m4dcrls,    "70000117.bin",                 0x0000, 0x080000, CRC(4106758c) SHA1(3d2b12f1820a65f00fd70856b7765b6f35a8688e), "Qps","Double Crazy Reels (Qps) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4dcrls__b,      m4dcrls,    "70000118.bin",                 0x0000, 0x080000, CRC(3603f93c) SHA1(cb969568e0244b465f8b120faba3adb65fe001e6), "Qps","Double Crazy Reels (Qps) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4dcrls__c,      m4dcrls,    "70000134.bin",                 0x0000, 0x080000, CRC(a81e80e7) SHA1(852c0b3afe8c22b6e6afe585efb8fec7aeb2aecb), "Qps","Double Crazy Reels (Qps) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4dcrls__d,      m4dcrls,    "70000130.bin",                 0x0000, 0x080000, CRC(9d700a27) SHA1(c73c7fc4233dace32fac90a6b46dba5c12979160), "Qps","Double Crazy Reels (Qps) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4dcrls__e,      m4dcrls,    "70000131.bin",                 0x0000, 0x080000, CRC(a7fcfcc8) SHA1(cc7b164e79d86d68112ac86e1ab9e81885cfcabf), "Qps","Double Crazy Reels (Qps) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4dcrls__f,      m4dcrls,    "70001117.bin",                 0x0000, 0x080000, CRC(7dcf4e36) SHA1(593089aec7efe8b14b953c5d7b0f552c0906730a), "Qps","Double Crazy Reels (Qps) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4dcrls__g,      m4dcrls,    "70001118.bin",                 0x0000, 0x080000, CRC(6732182c) SHA1(aa6620458d381fc37c226f996eab12840573cf80), "Qps","Double Crazy Reels (Qps) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4dcrls__h,      m4dcrls,    "70001134.bin",                 0x0000, 0x080000, CRC(a382156e) SHA1(db884dac04f556ecf49f7ecaba0bc3e51a4822f8), "Qps","Double Crazy Reels (Qps) (MPU4) (set 9)" )
GAME_CUSTOM( 199?, m4dcrls__i,      m4dcrls,    "70001172.bin",                 0x0000, 0x080000, CRC(32040f0f) SHA1(9f0452bc33e292ce61650f60f2943a3cef0da050), "Qps","Double Crazy Reels (Qps) (MPU4) (set 10)" )
GAME_CUSTOM( 199?, m4dcrls__j,      m4dcrls,    "70001173.bin",                 0x0000, 0x080000, CRC(0d5f138a) SHA1(e65832d01b11010a7c71230596e3fbc2c750d175), "Qps","Double Crazy Reels (Qps) (MPU4) (set 11)" )
GAME_CUSTOM( 199?, m4dcrls__k,      m4dcrls,    "dcr_gala_hopper_340.bin",      0x0000, 0x040000, CRC(e8a19eda) SHA1(14b49d7c9b8ad7c3f8605b2a57740aab2b98d030), "Qps","Double Crazy Reels (Qps) (MPU4) (set 12)" )
GAME_CUSTOM( 199?, m4dcrls__l,      m4dcrls,    "dcr_gala_hopper_340_lv.bin",   0x0000, 0x040000, CRC(e0d08c0e) SHA1(7c6a4e30bacfcbd895e418d4ce66425ec4f118f9), "Qps","Double Crazy Reels (Qps) (MPU4) (set 13)" )
GAME_CUSTOM( 199?, m4dcrls__m,      m4dcrls,    "dcr_mecca_340.bin",            0x0000, 0x040000, CRC(f18ac60f) SHA1(ffdd8d096ebc062a36a8d22cf881d0fa95adc2db), "Qps","Double Crazy Reels (Qps) (MPU4) (set 14)" )
GAME_CUSTOM( 199?, m4dcrls__n,      m4dcrls,    "dcr_mecca_340_lv.bin",         0x0000, 0x040000, CRC(0eb204c1) SHA1(648f5b90776f99155fd54257aabecb8c9f90abec), "Qps","Double Crazy Reels (Qps) (MPU4) (set 15)" )
GAME_CUSTOM( 199?, m4dcrls__o,      m4dcrls,    "dcr_std_340_lv.bin",           0x0000, 0x040000, CRC(d9632301) SHA1(19ac680f00e085d94fc45f765c975f3da1ca1eb3), "Qps","Double Crazy Reels (Qps) (MPU4) (set 16)" )
GAME_CUSTOM( 199?, m4dcrls__p,      m4dcrls,    "70001115.bin",                 0x0000, 0x040000, CRC(26432b07) SHA1(ef7303793252210f3fd07b12f5684b5d2cc828ab), "Qps","Double Crazy Reels (Qps) (MPU4) (set 17)" ) // dcr_data_340_lv.bin
GAME_CUSTOM( 199?, m4dcrls__q,      m4dcrls,    "70001116.bin",                 0x0000, 0x040000, CRC(522191e8) SHA1(f80656290295b556d4b67c4458d8f856f8b937fb), "Qps","Double Crazy Reels (Qps) (MPU4) (set 18)" )
GAME_CUSTOM( 199?, m4dcrls__r,      m4dcrls,    "dcr_data_340.bin",             0x0000, 0x010000, CRC(fc12e68f) SHA1(f07a42323651ef9aefac24c3b9296a98068c2dc2), "Qps","Double Crazy Reels (Qps) (MPU4) (set 19)" ) // too small?


#define M4JUNGJK_EXTRA_ROMS \
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 ) \
	ROM_LOAD( "71000080.lo.hex", 0x0000, 0x134084, CRC(f3866082) SHA1(f33f6d7e078d7072cc7c67672b3afa3e90e1f805) ) \
	ROM_LOAD( "71000080.hi.hex", 0x0000, 0x12680f, CRC(2a9db1df) SHA1(73823c3db5c68068dadf6d9b4c93b47c0cf13bd3) ) \
	ROM_LOAD( "71000080.p1", 0x000000, 0x080000, CRC(b39d5e03) SHA1(94c9208601ea230463b460f5b6ea668363d239f4) ) \
	ROM_LOAD( "71000080.p2", 0x080000, 0x080000, CRC(ad6da9af) SHA1(9ec8c8fd7b9bcd1d4c6ed93726fafe9a50a15894) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4JUNGJK_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default_big ,ROT0,company,title,GAME_FLAGS )



GAME_CUSTOM( 199?, m4jungjk,       0,          "jjsoft_v550_1346_e7a3_lv.bin", 0x0000, 0x040000, CRC(c5315a0c) SHA1(5fd2115e033e0310ded3cfb39f31dc31b4d6bb5a), "Qps","Jungle Jackpots (Qps) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4jungjk__a,    m4jungjk,   "70000102.bin",                 0x0000, 0x040000, CRC(e5f03540) SHA1(9a14cb4eade9f6b1c6d6cf78306259dbc108f1a5), "Qps","Jungle Jackpots (Qps) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4jungjk__b,    m4jungjk,   "jj.bin",                       0x0000, 0x040000, CRC(9e15c1b6) SHA1(9d4f3707f2cc2f0e8eb9051181bf8b368be3cbcf), "Qps","Jungle Jackpots (Qps) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4jungjk__c,    m4jungjk,   "jjlump_v400_19a3.bin",         0x0000, 0x040000, CRC(bc86c415) SHA1(6cd828578835dafe5d8d46810dc70d47abd4e8b2), "Qps","Jungle Jackpots (Qps) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4jungjk__d,    m4jungjk,   "70000092.bin",                 0x0000, 0x040000, CRC(6530bc6c) SHA1(27819e760c84fbb40f354e87910fb15b3058e2a8), "Qps","Jungle Jackpots (Qps) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4jungjk__e,    m4jungjk,   "jungle.p1",                    0x0000, 0x040000, CRC(1dbba129) SHA1(ac71bdb3082caf727736b26cf8727f966a8be243), "Qps","Jungle Jackpots (Qps) (MPU4) (set 6)" )



#define M4RHNOTE_EXTRA_ROMS \
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 ) \
	ROM_LOAD( "71000120.hex", 0x0000, 0x112961, CRC(5eb5245e) SHA1(449b02baf56e5798f656d9aee497b88d34f562cc) ) \
	ROM_LOAD( "rhnsnd.bin", 0x0000, 0x080000, CRC(e03eaa43) SHA1(69117021adc1a8968d50703336147a7344c62100) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4RHNOTE_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default_big ,ROT0,company,title,GAME_FLAGS )
GAME_CUSTOM( 199?, m4rhnote,       0,          "70000120.bin",                 0x0000, 0x040000, CRC(d1ce1e1c) SHA1(2fc2b041b4e9fcade4b2ce6a0bc709f4174e2d88), "Qps","Red Hot Notes (Qps) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4rhnote__a,    m4rhnote,   "70000121.bin",                 0x0000, 0x040000, CRC(1e1a26c0) SHA1(8a80a94d280c82887a0f7da607988597df23e1fb), "Qps","Red Hot Notes (Qps) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4rhnote__b,    m4rhnote,   "70000125.bin",                 0x0000, 0x080000, CRC(67a617a2) SHA1(3900c0cc3f8e4d52105096c1e21903cb83b8c1b7), "Qps","Red Hot Notes (Qps) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4rhnote__c,    m4rhnote,   "70000126.bin",                 0x0000, 0x080000, CRC(68deffbe) SHA1(9b94776aa0416309204987ac9109a65ad3234f1b), "Qps","Red Hot Notes (Qps) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4rhnote__d,    m4rhnote,   "70000132.bin",                 0x0000, 0x080000, CRC(50c06d0d) SHA1(8d629d77390b92c5e30104237245f92dc8f52a6c), "Qps","Red Hot Notes (Qps) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4rhnote__e,    m4rhnote,   "70000133.bin",                 0x0000, 0x080000, CRC(fb198e1b) SHA1(6fb03680ad29ca750fe2e75f48a05f538ddac9b7), "Qps","Red Hot Notes (Qps) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4rhnote__f,    m4rhnote,   "70000135.bin",                 0x0000, 0x080000, CRC(02531c21) SHA1(de9da10bc81ab02ba131da1a1733eda1948dc3cc), "Qps","Red Hot Notes (Qps) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4rhnote__g,    m4rhnote,   "70001122.bin",                 0x0000, 0x040000, CRC(13171ffc) SHA1(e49a2080afd27c0de183da64baa2060020910155), "Qps","Red Hot Notes (Qps) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4rhnote__h,    m4rhnote,   "70001124.bin",                 0x0000, 0x040000, CRC(8acb2d7d) SHA1(ffd4f0e1f80b41b6f54af31e5dcd41fe12e4ea0b), "Qps","Red Hot Notes (Qps) (MPU4) (set 9)" )
GAME_CUSTOM( 199?, m4rhnote__i,    m4rhnote,   "70001125.bin",                 0x0000, 0x080000, CRC(6b202a88) SHA1(63f7325c8dc373f771f02e5bf9ac0c0d33a906bd), "Qps","Red Hot Notes (Qps) (MPU4) (set 10)" )
GAME_CUSTOM( 199?, m4rhnote__j,    m4rhnote,   "70001126.bin",                 0x0000, 0x080000, CRC(0db90e12) SHA1(0b010ca878ecabb47c0a0eec0badd595b2bafbfb), "Qps","Red Hot Notes (Qps) (MPU4) (set 11)" )
GAME_CUSTOM( 199?, m4rhnote__k,    m4rhnote,   "70001135.bin",                 0x0000, 0x080000, CRC(a9ed9178) SHA1(446919e869a9cc20f469954504adf448474d702b), "Qps","Red Hot Notes (Qps) (MPU4) (set 12)" )
GAME_CUSTOM( 199?, m4rhnote__l,    m4rhnote,   "70001150.bin",                 0x0000, 0x040000, CRC(3c3f4e45) SHA1(114c18e0fa8de224992138b72bf789ace39dffa0), "Qps","Red Hot Notes (Qps) (MPU4) (set 13)" )
GAME_CUSTOM( 199?, m4rhnote__m,    m4rhnote,   "70001151.bin",                 0x0000, 0x040000, CRC(0cb1f440) SHA1(7ebdac6ea495d96c7713a284fdad4da0874de3f2), "Qps","Red Hot Notes (Qps) (MPU4) (set 14)" )
GAME_CUSTOM( 199?, m4rhnote__n,    m4rhnote,   "70001153.bin",                 0x0000, 0x040000, CRC(e8ba9b3a) SHA1(71af6dd77da419868391e01f565c24a70d55b396), "Qps","Red Hot Notes (Qps) (MPU4) (set 15)" ) // rhn_gala_hopper_120.bin
GAME_CUSTOM( 199?, m4rhnote__o,    m4rhnote,   "70001160.bin",                 0x0000, 0x040000, CRC(2d532681) SHA1(fb4321b6922cf35780adbdc5f030ef0df8d6cc9a), "Qps","Red Hot Notes (Qps) (MPU4) (set 16)" )
GAME_CUSTOM( 199?, m4rhnote__p,    m4rhnote,   "70001161.bin",                 0x0000, 0x040000, CRC(e9a49319) SHA1(001163ece7a405a27fd71fdeb97489db143749a7), "Qps","Red Hot Notes (Qps) (MPU4) (set 17)" )
GAME_CUSTOM( 199?, m4rhnote__q,    m4rhnote,   "70001502.bin",                 0x0000, 0x040000, CRC(d1b332f1) SHA1(07db228705b0bce47107cf5458986e830b988cee), "Qps","Red Hot Notes (Qps) (MPU4) (set 18)" )
GAME_CUSTOM( 199?, m4rhnote__r,    m4rhnote,   "70001503.bin",                 0x0000, 0x040000, CRC(2a44069a) SHA1(0a1581ba552e0e93d6bc3b7298014ea4b6793da1), "Qps","Red Hot Notes (Qps) (MPU4) (set 19)" )
GAME_CUSTOM( 199?, m4rhnote__s,    m4rhnote,   "70001510.bin",                 0x0000, 0x080000, CRC(87cb4cae) SHA1(49c97e0e79a8cd1417e9e07a13afe736d00ef3df), "Qps","Red Hot Notes (Qps) (MPU4) (set 20)" )
GAME_CUSTOM( 199?, m4rhnote__t,    m4rhnote,   "rhn_data_110_lv.bin",          0x0000, 0x040000, CRC(1f74c472) SHA1(86a170ddb001f817e960e7c166399280ad620bf0), "Qps","Red Hot Notes (Qps) (MPU4) (set 21)" )
GAME_CUSTOM( 199?, m4rhnote__u,    m4rhnote,   "rhn_gala_hopper_120_lv.bin",   0x0000, 0x040000, CRC(521b6402) SHA1(7d260c45fa339f5ca34f8e335875ad47bb093a04), "Qps","Red Hot Notes (Qps) (MPU4) (set 22)" )
GAME_CUSTOM( 199?, m4rhnote__v,    m4rhnote,   "rhn_mecca_120.bin",            0x0000, 0x040000, CRC(f131e386) SHA1(73672e6e66400b953dda7f2254082eff73dbf058), "Qps","Red Hot Notes (Qps) (MPU4) (set 23)" )
GAME_CUSTOM( 199?, m4rhnote__w,    m4rhnote,   "rhn_mecca_120_lv.bin",         0x0000, 0x040000, CRC(471e5263) SHA1(79c205e0d8e748aa72f9f3fadad248edf71f5ae0), "Qps","Red Hot Notes (Qps) (MPU4) (set 24)" )
GAME_CUSTOM( 199?, m4rhnote__x,    m4rhnote,   "rhn_std_110.bin",              0x0000, 0x040000, CRC(439f27d2) SHA1(4ad01c4dc9bbab7520fb281198777aea56f600b0), "Qps","Red Hot Notes (Qps) (MPU4) (set 25)" )
GAME_CUSTOM( 199?, m4rhnote__y,    m4rhnote,   "rhn_std_110_lv.bin",           0x0000, 0x040000, CRC(922b8196) SHA1(6fdbf301aaadacaeabf29ad11c67b22122954051), "Qps","Red Hot Notes (Qps) (MPU4) (set 26)" )

#define M4RHROCK_EXTRA_ROMS \
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 ) /* intelhex, needs converting */ \
	ROM_LOAD( "71000200.hi.hex", 0x0000, 0x0ff0f8, CRC(998e28ea) SHA1(f54a69af16e05119df2697bc01e548ac51ed3e11) ) \
	ROM_LOAD( "71000200.lo.hex", 0x0000, 0x134084, CRC(ccd0b35f) SHA1(6d3ef65577a46c68f8628675d146f829c9a99659) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4RHROCK_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default_big ,ROT0,company,title,GAME_FLAGS )


GAME_CUSTOM( 199?, m4rhrock,       0,          "rhr_v200_1625_da8c_nlv.bin", 0x0000, 0x040000, CRC(dd67f5b3) SHA1(19b7b57ef20a2ad7997cf748396b246fda87db70), "Qps","Red Hot Rocks (Qps) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4rhrock__a,    m4rhrock,   "rhr_v300_1216_ce52_nlv.bin", 0x0000, 0x040000, CRC(86b0d683) SHA1(c6553bf65c055c4f911c215ba112eaa672357290), "Qps","Red Hot Rocks (Qps) (MPU4) (set 2)" )



#define M4RHWHL_EXTRA_ROMS \
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 ) \
	ROM_LOAD( "71000180.hi.hex", 0x0000, 0x04da98, CRC(0ffa11a5) SHA1(a3f8eb00b6771cb49965a717e27d0b544c6b2f4f) ) \
	ROM_LOAD( "71000180.lo.hex", 0x0000, 0x134084, CRC(6dfc7474) SHA1(806b4b8ca5fa868581b4bf33080b9c486ce71bb6) ) \
	ROM_LOAD( "redhotwheelssnd.p1", 0x0000, 0x080000, CRC(7b274a71) SHA1(38ba69084819133253b41f2eb1d784104e5f10f7) ) \
	ROM_LOAD( "redhotwheelssnd.p2", 0x0000, 0x080000, CRC(e36e19e2) SHA1(204554622c9020479b095acd4fbab1f21f829137) )
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4RHWHL_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default_big ,ROT0,company,title,GAME_FLAGS )
GAME_CUSTOM( 199?, m4rhwhl,     0,          "70001184.bin",                 0x0000, 0x080000, CRC(8792d95b) SHA1(24b4f78728db7ee95d1fcd3ba38b49a20baaae6b), "Qps","Red Hot Wheels (Qps) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4rhwhl__a,  m4rhwhl,    "rhw_v100_1333_6d40_lv.bin",    0x0000, 0x080000, CRC(9ef7b655) SHA1(605822eaee44bebf554218ef7346192a6a84077e), "Qps","Red Hot Wheels (Qps) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4rhwhl__b,  m4rhwhl,    "rhw_v310_0925_0773_lv_p.bin",  0x0000, 0x080000, CRC(11880908) SHA1(0165bacf73dd54959975b3f186e256fd8d690d34), "Qps","Red Hot Wheels (Qps) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4rhwhl__c,  m4rhwhl,    "rhw_v310_0931_fa02_lv.bin",    0x0000, 0x080000, CRC(5642892e) SHA1(7a80edf9aefac9731751afa8250de07004c55e77), "Qps","Red Hot Wheels (Qps) (MPU4) (set 4)" )




#define M4RDEAL_EXTRA_ROMS \
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4RDEAL_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default_big ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4rdeal,     0,          "70000703.bin", 0x0000, 0x080000, CRC(11e51311) SHA1(71a4327fa01cd7e899d423adc34c732ed56118d8), "Qps","Reel Deal (Qps) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4rdeal__a,  m4rdeal,    "70000704.bin", 0x0000, 0x080000, CRC(b161c08b) SHA1(bb914eb900aff0f6eeec33ff8a595a288306e073), "Qps","Reel Deal (Qps) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4rdeal__b,  m4rdeal,    "70000723.bin", 0x0000, 0x080000, CRC(bb166401) SHA1(1adf244e97d52cc5a5116a01d804caadd1034507), "Qps","Reel Deal (Qps) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4rdeal__c,  m4rdeal,    "70000724.bin", 0x0000, 0x080000, CRC(37df89e5) SHA1(6e7da02053be91f27257e3c5f952bbea9df3bc09), "Qps","Reel Deal (Qps) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4rdeal__d,  m4rdeal,    "70001622.bin", 0x0000, 0x080000, CRC(1a8fff86) SHA1(e0bdb4fa233acdf18d535821fb9fc2cf69ac4f5e), "Qps","Reel Deal (Qps) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4rdeal__e,  m4rdeal,    "70001623.bin", 0x0000, 0x080000, CRC(97a223f8) SHA1(d1a87391a8178cb47664968031a7d17d9082847b), "Qps","Reel Deal (Qps) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4rdeal__f,  m4rdeal,    "70001703.bin", 0x0000, 0x080000, CRC(1b89777e) SHA1(09b87d352d27c2847dec0a147dea7cc75f07ffa5), "Qps","Reel Deal (Qps) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4rdeal__g,  m4rdeal,    "70001704.bin", 0x0000, 0x080000, CRC(11f3834d) SHA1(8c56cf60b064e8d755c5e760fcf6f0284ef710f9), "Qps","Reel Deal (Qps) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4rdeal__h,  m4rdeal,    "70001744.bin", 0x0000, 0x080000, CRC(6de0d366) SHA1(ec42608f3b9c7cf8e6b81541767a8664478e7ab4), "Qps","Reel Deal (Qps) (MPU4) (set 9)" )
GAME_CUSTOM( 199?, m4rdeal__i,  m4rdeal,    "70001745.bin", 0x0000, 0x080000, CRC(4c0e9cab) SHA1(6003d4553f053a253965ba786553b00b7e197069), "Qps","Reel Deal (Qps) (MPU4) (set 10)" )


#define M4SHOKNR_EXTRA_ROMS \
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 ) \
	ROM_LOAD( "snrsnd.p1", 0x000000, 0x080000, CRC(985c7c8c) SHA1(d2740ff6192c21af3a8a8a9a92b6fd604b40e9d1) ) \
	ROM_LOAD( "snrsnd.p2", 0x080000, 0x080000, CRC(6a3a57ce) SHA1(3aaa0a761e17a2a14196cb023b10a49b44ba1046) ) \
	ROM_LOAD( "shock.s2", 0x080000, 0x080000, CRC(10e9912f) SHA1(833d2b125bf30bdb8de71f6c9d8a9fe92701f741) ) /* alt snd2 */
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4SHOKNR_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default_big ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4shoknr,       0,          "snr_v300_1218_3019_lv.bin",    0x0000, 0x040000, CRC(bec80497) SHA1(08de5e29a063b01fb904a156170a3063633115ab), "Qps","Shock 'n' Roll (Qps) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4shoknr__a,    m4shoknr,   "snr_v300_1221_c8ff_nlv.bin",   0x0000, 0x040000, CRC(d191b361) SHA1(4146e509e77878a51e32de877768504b3c85e6f8), "Qps","Shock 'n' Roll (Qps) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4shoknr__b,    m4shoknr,   "snr_v200_1145_047f_lv.bin",    0x0000, 0x040000, CRC(73ef1e1a) SHA1(6ccaf64daa5acacfba4df576281bb5478f2fbd29), "Qps","Shock 'n' Roll (Qps) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4shoknr__c,    m4shoknr,   "snr_v200_1655_5a69_nlv.bin",   0x0000, 0x040000, CRC(50ba0c6b) SHA1(767fd59858fc55ae95f096f00c54bd619369a56c), "Qps","Shock 'n' Roll (Qps) (MPU4) (set 4)" )

#define M4TORNAD_EXTRA_ROMS \
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 ) \
	ROM_LOAD( "71000300.hi.hex", 0x0000, 0x0be342, CRC(f9021a32) SHA1(4bd7d7306385ef37dd9cbb5085dbc104657abc0e) ) \
	ROM_LOAD( "71000300.lo.hex", 0x0000, 0x134084, CRC(af34658d) SHA1(63a6db1f5ed00fa6208c63e0a2211ba2afe0e9a1) ) \
	ROM_LOAD( "tornadosnd.p1", 0x0000, 0x080000, CRC(cac88f25) SHA1(6ccbf372d983a47a49caedb8a526fc7703b31ed4) ) \
	ROM_LOAD( "tornadosnd.p2", 0x080000, 0x080000, CRC(ef4f563d) SHA1(1268061edd93474296e3454e0a2e706b90c0621c) )
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4TORNAD_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default_big ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4tornad,       0,          "torn_v110_1146_979d_lv.bin",       0x0000, 0x040000, CRC(3160bddd) SHA1(4f36b081c8f6859a3fe55e1f177a0406c2480987), "Qps","Tornado (Qps) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4tornad__a,    m4tornad,   "torn_v110_1153_955f_nlv.bin",      0x0000, 0x040000, CRC(c437040d) SHA1(50c5ba655989b7f6a2ee61af0ad007ce825f4364), "Qps","Tornado (Qps) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4tornad__b,    m4tornad,   "tornsp_v110_1148_95bd_nlv.bin",    0x0000, 0x040000, CRC(f0933eb6) SHA1(a726b02ae6298ecfb6a01f7ecb09bac50ca13114), "Qps","Tornado (Qps) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4tornad__c,    m4tornad,   "tornsp_v110_1151_9693_lv.bin",     0x0000, 0x040000, CRC(05c48766) SHA1(e20916cd67904601cd25b3c6f70030c5302d12a6), "Qps","Tornado (Qps) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4tornad__d,    m4tornad,   "torn_v200_1613_efe5_nlv.bin",      0x0000, 0x040000, CRC(32936ae5) SHA1(f7ec8b9c0e74c0e51ea4b9c8f450f64907ce3300), "Qps","Tornado (Qps) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4tornad__e,    m4tornad,   "torn_v200_1617_ece9_lv.bin",       0x0000, 0x040000, CRC(c7c4d335) SHA1(955303d446e78bfa1ceeb07ca62cb6e11e478592), "Qps","Tornado (Qps) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4tornad__f,    m4tornad,   "tornsp_v200_1623_eee3_nlv.bin",    0x0000, 0x040000, CRC(6b4f8baf) SHA1(fea21f43b3bbc1c969a7426ca956898e3680823f), "Qps","Tornado (Qps) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4tornad__g,    m4tornad,   "tornsp_v200_1626_ec93_lv.bin",     0x0000, 0x040000, CRC(9e18327f) SHA1(7682cd172903cd5c26873306e70394c154e66c30), "Qps","Tornado (Qps) (MPU4) (set 8)" )


#define M4SHKWAV_EXTRA_ROMS \
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 ) \
	ROM_LOAD( "71000250.hi.hex", 0x0000, 0x0c852b, CRC(e3a857c7) SHA1(66619b7926ae7df970045fffd7e20763abfe14a4) ) \
	ROM_LOAD( "71000250.lo.hex", 0x0000, 0x134084, CRC(46758bc5) SHA1(18d02960580646b276e7a6aabdeb4ca449ec5ea0) ) \
	ROM_LOAD( "shocksnd.p1", 0x000000, 0x080000, CRC(54bf0ddb) SHA1(693b855367972b5a45e9d2d6152849ab2cde38a7) ) \
	ROM_LOAD( "shocksnd.p2", 0x080000, 0x080000, CRC(facebc55) SHA1(75367473646cfc735f4d1267e13a9c92ea19c4e3) )
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4SHKWAV_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default_big ,ROT0,company,title,GAME_FLAGS )
GAME_CUSTOM( 199?, m4shkwav,       0,          "swave_v210_1135_08dd_lv.bin",      0x0000, 0x040000, CRC(ca9d40a3) SHA1(65c9e4aa022eb6fe70d619f67638c37ad578ddbf), "Qps","Shockwave (Qps) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4shkwav__a,    m4shkwav,   "swave_v210_11376_0bb3_nlv.bin",    0x0000, 0x040000, CRC(3fcaf973) SHA1(28258c8c60e6b542e1789cd8a4cfd530d1ed6084), "Qps","Shockwave (Qps) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4shkwav__b,    m4shkwav,   "swsplv.bin",                       0x0000, 0x040000, CRC(1e33e93f) SHA1(3e87f8ed35da776e1968c9574c140cc3984ea8de), "Qps","Shockwave (Qps) (MPU4) (set 3)" )
//This rom is possibly bad, data content isn't multiple of 0x800, padding with low bits rather than high
	ROM_START( m4shkwav__c )
		ROM_REGION( 0x080000, "maincpu", 0 )
		ROM_LOAD( "sho1_0lv.bin", 0x0000, 0x080000, BAD_DUMP CRC(a76d8544) SHA1(8277a2ce311840b8405a087d3dc0bbf97054ad87) )
		M4SHKWAV_EXTRA_ROMS
	ROM_END
GAME(199?, m4shkwav__c, m4shkwav ,mod4oki ,mpu4 , mpu4_state,m4default_big ,ROT0,"Qps","Shockwave (Qps) (MPU4) (set 4)",GAME_FLAGS )
GAME_CUSTOM( 199?, m4shkwav__d,    m4shkwav,   "swave_v300_1552_13ed_nlv.bin",     0x0000, 0x040000, CRC(b0e03f04) SHA1(fdd113af30fd9e87b171ecdf3be7e720366476b3), "Qps","Shockwave (Qps) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4shkwav__e,    m4shkwav,   "swave_v300_1555_119d_lv.bin",      0x0000, 0x040000, CRC(45b786d4) SHA1(24fd4fdea684103334385ca329f384796b496e2c), "Qps","Shockwave (Qps) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4shkwav__f,    m4shkwav,   "swsp_v300_1602_e1b2_nlv.bin",      0x0000, 0x040000, CRC(4ed74015) SHA1(0ab2167ba0ce6f1a1317c2087091187b9fa94c27), "Qps","Shockwave (Qps) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4shkwav__g,    m4shkwav,   "swsp_v300_1606_ded8_lv.bin",       0x0000, 0x040000, CRC(bb80f9c5) SHA1(95f577c427204b83bec0128acfd89dda90938d1f), "Qps","Shockwave (Qps) (MPU4) (set 8)" )


#define M4CLAB_EXTRA_ROMS \
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 ) \
	ROM_LOAD( "71000010.lo", 0x0000, 0x134084, CRC(c39bbae4) SHA1(eee333376612a96a4c344729a96cc60c217bfde3) ) \
	ROM_LOAD( "71000010.hi", 0x0000, 0x091c0b, CRC(d0d3cb4f) SHA1(eaacf9ed3a6b6dcda8e1a3edbc3a9a2a51ffcbd8) ) \
	ROM_LOAD( "clab_snd1_c8a6.bin", 0x0000, 0x080000, CRC(cf9de981) SHA1(e5c73e9b9db9ac512602c2dd586ca5cf65f98bc1) ) \
	ROM_LOAD( "clab_snd2_517a.bin", 0x080000, 0x080000, CRC(d4eb949e) SHA1(0ebbd1b5e3c86da94f35c69d9d60e36844cc4d7e) )
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4CLAB_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default_big ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4clab,       0,      "70000019.bin",                     0x0000, 0x040000, CRC(23a12863) SHA1(4047cb8cbc03f96f2b8681b6276e100e8e9194a5), "Qps","Cash Lab (Qps) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4clab__a,    m4clab, "70000020.bin",                     0x0000, 0x040000, CRC(88af7368) SHA1(14dea4267a4365286eea1e02b9b44d4053618cbe), "Qps","Cash Lab (Qps) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4clab__b,    m4clab, "70000052.bin",                     0x0000, 0x040000, CRC(99e60d45) SHA1(ec28bdd4ffb9674c2e9f8ab72aac3cb6011e7d6f), "Qps","Cash Lab (Qps) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4clab__c,    m4clab, "70000053.bin",                     0x0000, 0x040000, CRC(3ddf8b29) SHA1(0087ccd3429c081a3121e97d649914ab2cf8caa6), "Qps","Cash Lab (Qps) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4clab__d,    m4clab, "clab_v300_1028_c23a_lv_10p.bin",   0x0000, 0x040000, CRC(c2ca098a) SHA1(e2bf80366af925a4c880a3377b79d494760f286f), "Qps","Cash Lab (Qps) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4clab__e,    m4clab, "clab_v300_1031_3fb7_nlv_10p.bin",  0x0000, 0x040000, CRC(1502527f) SHA1(8a3dd5600ad9a073ea6fb8412178daa575002e56), "Qps","Cash Lab (Qps) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4clab__f,    m4clab, "clab_v300_1033_1175_nlv_20p.bin",  0x0000, 0x040000, CRC(ba26efd7) SHA1(bf9c0dbf6882ddc42680c7d23c9b858eb12e5646), "Qps","Cash Lab (Qps) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4clab__g,    m4clab, "clab_v400_1249_0162_nlv.bin",      0x0000, 0x040000, CRC(df256c84) SHA1(f4d0fc5acd7d0ac770cae548744ce18dfb9ec67c), "Qps","Cash Lab (Qps) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4clab__h,    m4clab, "clab_v410_1254_1ca2_lv.bin",       0x0000, 0x040000, CRC(6e2ee8a9) SHA1(e64a01c93b879c9ade441bd4f1ce381f6e65a655), "Qps","Cash Lab (Qps) (MPU4) (set 9)" )
GAME_CUSTOM( 199?, m4clab__i,    m4clab, "clab_v410_1254_6e11_nlv.bin",      0x0000, 0x040000, CRC(efe4fcb9) SHA1(8a2f02593c7fbea060c78a98abd82fd970661e05), "Qps","Cash Lab (Qps) (MPU4) (set 10)" )
GAME_CUSTOM( 199?, m4clab__j,    m4clab, "clabrom",                          0x0000, 0x040000, CRC(d80ecff5) SHA1(2608e95b718ecd49d880fd9911cb97e6644a307d), "Qps","Cash Lab (Qps) (MPU4) (set 11)" )

#define M4SDQUID_EXTRA_ROMS \
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4SDQUID_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default_big ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4sdquid,       0,          "70000352.bin", 0x0000, 0x040000, CRC(303d6177) SHA1(aadff8a81244bfd62d1cc088caf01496e1ff61db), "Qps","Sundance Quid (Qps) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4sdquid__a,    m4sdquid,   "70000353.bin", 0x0000, 0x040000, CRC(6e3a9dfc) SHA1(1d5d04140811e17267102c0618ffdaf70f71f717), "Qps","Sundance Quid (Qps) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4sdquid__b,    m4sdquid,   "70000354.bin", 0x0000, 0x080000, CRC(eb938886) SHA1(e6e882f28230b51091b2543df17c65e491a94f94), "Qps","Sundance Quid (Qps) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4sdquid__c,    m4sdquid,   "70000355.bin", 0x0000, 0x080000, CRC(c4b857b3) SHA1(6b58ee5d780551a665e737ea291b04ab641e8029), "Qps","Sundance Quid (Qps) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4sdquid__d,    m4sdquid,   "70001352.bin", 0x0000, 0x040000, CRC(0356e899) SHA1(51666a0ec4d29165f64391efa2339ea82710fa4c), "Qps","Sundance Quid (Qps) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4sdquid__e,    m4sdquid,   "70001353.bin", 0x0000, 0x040000, CRC(324b4a86) SHA1(fd4cd59e3519e23ab824c84546ab50b3803766bf), "Qps","Sundance Quid (Qps) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4sdquid__f,    m4sdquid,   "70001354.bin", 0x0000, 0x080000, CRC(e9aee455) SHA1(f9babcb90fe88fd522e4be7d8b794c9c1cd4f780), "Qps","Sundance Quid (Qps) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4sdquid__g,    m4sdquid,   "70001355.bin", 0x0000, 0x080000, CRC(ebc428c3) SHA1(c75738fc547c5b1783a6c5a6ebb06eea5730683d), "Qps","Sundance Quid (Qps) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4sdquid__h,    m4sdquid,   "70001401.bin", 0x0000, 0x080000, CRC(f83b1551) SHA1(d32f84938edfc4c3d9763fb3eecf56ed9102d979), "Qps","Sundance Quid (Qps) (MPU4) (set 9)" )
GAME_CUSTOM( 199?, m4sdquid__i,    m4sdquid,   "70001411.bin", 0x0000, 0x080000, CRC(fa0c95ec) SHA1(1a792e7ed8fa092fdb34a7b31df316d6afca3e90), "Qps","Sundance Quid (Qps) (MPU4) (set 10)" )
GAME_CUSTOM( 199?, m4sdquid__j,    m4sdquid,   "70001451.bin", 0x0000, 0x080000, CRC(2d288982) SHA1(bf3e1e1e20eb2d1d9ca6d0a8b48ef9c57aeb30bd), "Qps","Sundance Quid (Qps) (MPU4) (set 11)" )
GAME_CUSTOM( 199?, m4sdquid__k,    m4sdquid,   "70001461.bin", 0x0000, 0x080000, CRC(73238425) SHA1(a744adabd8854c6b45820899f15ebb2c2a74dd4d), "Qps","Sundance Quid (Qps) (MPU4) (set 12)" )



#define M4LOOPLT_EXTRA_ROMS \
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 ) \
	ROM_LOAD( "71000500.bin", 0x0000, 0x080000, CRC(94fe58f4) SHA1(e07d8e6d4b1e660abc4fa08d703fc0e586f3570d) )
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4LOOPLT_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default_big ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4looplt,       0,          "70000500.bin",     0x0000, 0x080000, CRC(040699a5) SHA1(e1ebc23684c5bc1faaac7409d2179488c3022872), "Qps","Loop The Loot (Qps) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4looplt__a,    m4looplt,   "70000500a.bin",    0x0000, 0x080000, CRC(0668f52d) SHA1(6560309facf0022e3c14421b848f212b18be7550), "Qps","Loop The Loot (Qps) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4looplt__b,    m4looplt,   "70000501.bin",     0x0000, 0x080000, CRC(e2fbbfcf) SHA1(fc060468bf5e732626af8c3d0d6fc119a529c330), "Qps","Loop The Loot (Qps) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4looplt__c,    m4looplt,   "70000501a.bin",    0x0000, 0x080000, CRC(42bef934) SHA1(c332eb6566ef5f9ac56d1c3944635296c21b3193), "Qps","Loop The Loot (Qps) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4looplt__d,    m4looplt,   "70000504.bin",     0x0000, 0x080000, CRC(15e2c1c3) SHA1(69257749f1909b7ecc9c94cc2a27a5d4e6608251), "Qps","Loop The Loot (Qps) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4looplt__e,    m4looplt,   "70000505.bin",     0x0000, 0x080000, CRC(f28f59bd) SHA1(d5cdb0c020693c7922c5243f9d18054d47ed039d), "Qps","Loop The Loot (Qps) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4looplt__f,    m4looplt,   "70000506.bin",     0x0000, 0x080000, CRC(a3d40e9a) SHA1(97ac40e814824450e6705bc3240fffd4d0015b46), "Qps","Loop The Loot (Qps) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4looplt__g,    m4looplt,   "70000507.bin",     0x0000, 0x080000, CRC(756eefe4) SHA1(b253fbd94fdab5df32375a02d16d9ba333e8d71c), "Qps","Loop The Loot (Qps) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4looplt__h,    m4looplt,   "70001500.bin",     0x0000, 0x080000, CRC(0b9761a4) SHA1(e7a5e4b90d2e60808a7797d124308973130c440d), "Qps","Loop The Loot (Qps) (MPU4) (set 9)" )
GAME_CUSTOM( 199?, m4looplt__i,    m4looplt,   "70001500a.bin",    0x0000, 0x080000, CRC(09f90d2c) SHA1(addfd0d20ef9cafba042aa05ee84db85f060b67a), "Qps","Loop The Loot (Qps) (MPU4) (set 10)" )
GAME_CUSTOM( 199?, m4looplt__j,    m4looplt,   "70001501.bin",     0x0000, 0x080000, CRC(6ce9f76c) SHA1(467701786f8de136c9780a4ef93be6bb932d235d), "Qps","Loop The Loot (Qps) (MPU4) (set 11)" )
GAME_CUSTOM( 199?, m4looplt__k,    m4looplt,   "70001501a.bin",    0x0000, 0x080000, CRC(ccacb197) SHA1(c7573f309e9c79b2999229c46f78fd0283c4a064), "Qps","Loop The Loot (Qps) (MPU4) (set 12)" )
GAME_CUSTOM( 199?, m4looplt__l,    m4looplt,   "70001504.bin",     0x0000, 0x080000, CRC(1a7339c2) SHA1(575477d8abe3765d9cd4345336d0f7fa3a69202a), "Qps","Loop The Loot (Qps) (MPU4) (set 13)" )
GAME_CUSTOM( 199?, m4looplt__m,    m4looplt,   "70001505.bin",     0x0000, 0x080000, CRC(7c9d111e) SHA1(8f98feb70cdcd77b5e7bb6a015c935403a53f428), "Qps","Loop The Loot (Qps) (MPU4) (set 14)" )



/* Global */
// boot to "Percent Change" then "*initializing*"
GAME(199?, m4bangin,  0,        mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Global","Bangin' Away (Global) (MPU4, set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4bangina, m4bangin, mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Global","Bangin' Away (Global) (MPU4, set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4banginb, m4bangin, mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Global","Bangin' Away (Global) (MPU4, set 3)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4wwc,     0,        mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Global","Wacky Weekend Club (Global) (MPU4) (set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4wwca,    m4wwc,    mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Global","Wacky Weekend Club (Global) (MPU4) (set 2)" ,   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4wwcb,    m4wwc,    mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Global","Wacky Weekend Club (Global) (MPU4) (set 3)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4screw,   0,        mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Global","Screwin' Around (Global) (MPU4, v0.8)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4screwp,  m4screw,  mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Global","Screwin' Around (Global) (MPU4, v0.8) (Protocol)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4screwa,  m4screw,  mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Global","Screwin' Around (Global) (MPU4, v0.7)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4screwb,  m4screw,  mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Global","Screwin' Around (Global) (MPU4, v0.5)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4vfm,     0,        mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Global","Value For Money (Global) (MPU4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4jiggin,  0,        mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Global","Jiggin' In The Riggin' (Global) (MPU4) (set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4jiggina, m4jiggin, mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Global","Jiggin' In The Riggin' (Global) (MPU4) (set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
