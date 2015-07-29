// license:BSD-3-Clause
// copyright-holders:David Haywood
/* MPU4 games by 'Union' */

#include "emu.h"
#include "includes/mpu4.h"

MACHINE_CONFIG_EXTERN( mod4oki );
MACHINE_CONFIG_EXTERN( mod2 );
INPUT_PORTS_EXTERN( mpu4 );

#define GAME_FLAGS (MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL)


ROM_START( m4cwalk )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "cw1_1.bin", 0x0000, 0x010000, CRC(a1108d79) SHA1(fa2a5510f2bb2d3811550547bad7c3ef0eb0ddc0) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "cakesnd1.bin", 0x000000, 0x080000, CRC(abd16cc4) SHA1(744dd1da6c2126bc8673f14aac556af29878f2c4) )
	ROM_LOAD( "cakesnd2.bin", 0x080000, 0x080000, CRC(34da47c0) SHA1(b55f352a7a62172d1dfe990bef39bcbd50e48597) )
ROM_END


ROM_START( m4eezee )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "ez2511.bin", 0x0000, 0x010000, CRC(86e93c3f) SHA1(a999830685da5d183058769a0598c338c20accdf) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "eezeefruitssnd.bin", 0x0000, 0x080000, CRC(e6e5aa12) SHA1(0f35eaf0a29050365f53d039e4a7880240c28dc4) )
ROM_END

ROM_START( m4frdrop )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "fruitdrop.bin", 0x0000, 0x010000, CRC(7235d3f0) SHA1(e327e28e341ec859f503b71065c40b5d47f448fe) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "fruitdropsnd.bin", 0x0000, 0x080000, CRC(27880a95) SHA1(6286ab0c342db7de174c3582f56cf9dd60c46883) )
ROM_END


ROM_START( m4gobana )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_b_11_dr_c468.bin", 0x0000, 0x010000, CRC(036776d1) SHA1(73d63731978495cedc5e4c095a83925424ac79c3) )
ROM_END


ROM_START( m4gobanaa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_b_11p_9877.bin", 0x0000, 0x010000, CRC(d985a6eb) SHA1(59576ad9e7589651a6479adb57e8c8bf32a8ef97) )
ROM_END

ROM_START( m4gobanab )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_b_11p_dr_c46b.bin", 0x0000, 0x010000, CRC(7343588f) SHA1(e40416a8fdea5a71677493047e569d452ba193df) )
ROM_END

ROM_START( m4gobanac )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_b_v11_9873.bin", 0x0000, 0x010000, CRC(3c23581c) SHA1(81daa4903ace4419c1329acbba8907c46b6e6233) )
ROM_END

ROM_START( m4gobanad )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_bananas_mecca_v2_31a5.bin", 0x0000, 0x010000, CRC(0c7ae1da) SHA1(9b144a2f1b91def5d9f054fba3d9f1adfe957646) )
ROM_END

ROM_START( m4lotty )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lottytime.bin", 0x0000, 0x010000, CRC(c032c422) SHA1(37c6e10c1fa1cab3de7b4d27ed027604ecea4394) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "lottytimesnd.bin", 0x0000, 0x080000, CRC(e1966300) SHA1(2a69e39310b49c685bc4307e0396a3b9a0849472) )
ROM_END



ROM_START( m4maxmze )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "maximize_dr_v13_5146.bin", 0x0000, 0x010000, CRC(86507f09) SHA1(e2dbf9b77a5155faeeba05a939ae217289949bce) )
ROM_END

ROM_START( m4maxmzea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "maximize_dr_v13p_514c.bin", 0x0000, 0x010000, CRC(3d483ae3) SHA1(0687912795a597254815b8f359965bdf2e887be1) )
ROM_END

ROM_START( m4maxmzeb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "maximize_mecca_v2_50cf.bin", 0x0000, 0x010000, CRC(88426c07) SHA1(2b026d75cbda9375cbfb62f19c25f31c658c56bc) )
ROM_END

ROM_START( m4maxmzec )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "maximize_v13_7107.bin", 0x0000, 0x010000, CRC(ed52f047) SHA1(d6b62934a92dcd6d4df3ac9ae1d9d143000e7c92) )
ROM_END

ROM_START( m4maxmzed )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "maximize_v13p_710b.bin", 0x0000, 0x010000, CRC(1e71a801) SHA1(4cd213e422c91fa10dceea0a93fd81576b1c3f7f) )
ROM_END


ROM_START( m4mecca )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mecca_money_dr_v4_ef11.bin", 0x0000, 0x010000, CRC(2429ee8b) SHA1(785304c687a658706b26080f6b8f4bc65830dcea) )
ROM_END

ROM_START( m4purmad )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pmad.p1", 0x0000, 0x010000, CRC(e430510b) SHA1(bb8d443429aa7d39a99de5cd1387154398b74d9c) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "pmadsnd.bin", 0x0000, 0x080000, CRC(88312507) SHA1(64e386c3c9b3f82a390777b61c7207567cc962e7) )
ROM_END

ROM_START( m4revolv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "revolva.bin", 0x0000, 0x010000, CRC(e195feac) SHA1(dc8d736784819fd15f0e7e29e9f91cf1c601ebb9) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "revolvasnd.p1", 0x000000, 0x080000, CRC(a9138bdf) SHA1(aaa5b3e82a8d89776b636e46ada9aae8a4febaab) )
	ROM_LOAD( "revolvasnd.p2", 0x080000, 0x080000, CRC(5ae7b26a) SHA1(a17cef58e37bccb5954b9acab01f92ff21375ab1) )
ROM_END

ROM_START( m4rotex )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rotex1_2.bin", 0x0000, 0x010000, CRC(202e794d) SHA1(15b7459db7f3a5317a92138d997e9817d4367750) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "rotsnd.bin", 0x0000, 0x080000, CRC(02f8c2e2) SHA1(33a051e7af6d8c33708f7b4e0654d66312eeede5) )
ROM_END


ROM_START( m4select )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "selectawp.bin", 0x0000, 0x010000, CRC(e248539c) SHA1(b1db8fdc221c70d2a699cb86cea5681527d7d06a) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "selectsnd.bin", 0x0000, 0x080000, CRC(93fd4253) SHA1(69feda7ffc56defd515c9cd1ce204af3d9731a3f) )
ROM_END

ROM_START( m4supfru )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "supa11l.bin", 0x0000, 0x010000, CRC(6cdbf08a) SHA1(4c0faf7144b9ac19c8d55b81ce51b519570b0d1f) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "supafruitssnd.bin", 0x0000, 0x080000, CRC(b70184ca) SHA1(dba2204cb606f0c6dad8a4c46fbbb1beb5b5e31c) )
ROM_END


ROM_START( m4supfrua )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "supafruits.bin", 0x0000, 0x010000, CRC(c6242e33) SHA1(810be1fb99ddb810c5506a974c9214ce23426a87) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "supafruitssnd.bin", 0x0000, 0x080000, CRC(b70184ca) SHA1(dba2204cb606f0c6dad8a4c46fbbb1beb5b5e31c) )
ROM_END

ROM_START( m4trimad )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tmad1_1p", 0x000000, 0x010000, CRC(7a49546b) SHA1(cfce6fda74682e6c60a5731fee44c41c5f5bbbeb) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "triple.s1", 0x000000, 0x080000, CRC(e808f0ae) SHA1(4d88c1d8ed9396629509a0c7614e7510401f1325) )
	ROM_LOAD( "triple.s2", 0x080000, 0x080000, CRC(40161063) SHA1(a24edb311fea466c0c15aebce40044f8db448e50) )
	ROM_LOAD( "triple.s3", 0x100000, 0x080000, CRC(f9109afb) SHA1(4e4a863b60915ddb2865c12af19cc38bcad6d062) )
ROM_END


ROM_START( m4unibox )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "unibox.bin", 0x0000, 0x010000, CRC(9e99eed4) SHA1(c59f9ab2cae487991b0202bd18c6ab514ba3b29d) )

	ROM_REGION( 0x80000, "msm6376", 0 )
	ROM_LOAD( "uniboxsnd.bin", 0x0000, 0x080000, CRC(cce8fda1) SHA1(e9d6514dc2badb046201ea44802690cf104e6075) )
ROM_END

ROM_START( m4uniboxa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "unibox1.bin", 0x0000, 0x010000, CRC(dd8fa3bd) SHA1(34719608e429e6ef0cc0ea9a75df7b00439e53ed) )

	ROM_REGION( 0x80000, "msm6376", 0 )
	ROM_LOAD( "uniboxsnd.bin", 0x0000, 0x080000, CRC(cce8fda1) SHA1(e9d6514dc2badb046201ea44802690cf104e6075) )
ROM_END

ROM_START( m4unique )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "unique.bin", 0x000000, 0x010000, CRC(ac8c83da) SHA1(11912c1b9a028e17db2395ca85792c4d10fae3ad) )

	ROM_REGION( 0x80000, "msm6376", 0 )
	ROM_LOAD( "uniquesnd.bin", 0x000000, 0x080000, CRC(177a4c07) SHA1(180f51ba982ccf7d19dfa50e94c295395799c360) )
ROM_END

ROM_START( m4uniquep )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "uniquepatched64k.bin", 0x000000, 0x010000, CRC(889575a2) SHA1(dcd0830d75cb4ddc901f22d71a28836e55d969cc) )

	ROM_REGION( 0x80000, "msm6376", 0 )
	ROM_LOAD( "uniquesnd.bin", 0x000000, 0x080000, CRC(177a4c07) SHA1(180f51ba982ccf7d19dfa50e94c295395799c360) )
ROM_END

ROM_START( m4crzbn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "crazybingo.bin", 0x0000, 0x010000, CRC(51b3856c) SHA1(ed04771c69de090f4895920c213ba30754fcc1f9) )

	ROM_REGION( 0x80000, "msm6376", 0 )
	ROM_LOAD( "crazybingosnd.bin", 0x0000, 0x080000, CRC(6a60c412) SHA1(4f038675646510372022145033884ca704d31033) )
ROM_END


ROM_START( m4gvibes )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "gv0_1.bin", 0x0000, 0x010000, CRC(ba9a507b) SHA1(ba3b2038b50248ec1f3319e2b39d02313ce3ad08) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "gvsnd.p1", 0x0000, 0x080000, CRC(ac56b475) SHA1(8017784e5dd8e6d85857ff989c553d04c2ea217a) )

	ROM_REGION( 0x080000, "altmsm6376", 0 )
	// different SFX, does this belong to a specific revision?
	ROM_LOAD( "gv2snd.bin", 0x0000, 0x080000, CRC(e11ebc9c) SHA1(3f4e8148bc3687af77838b770bbc219a3f50f1c6) )
ROM_END

ROM_START( m4gvibesa )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "gv0_3.bin", 0x0000, 0x010000, CRC(c20e896e) SHA1(9105ba56b9f4b55a158e4dc2b4268a4b93765c88) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "gvsnd.p1", 0x0000, 0x080000, CRC(ac56b475) SHA1(8017784e5dd8e6d85857ff989c553d04c2ea217a) )
ROM_END


ROM_START( m4rckrol )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rock0_1.hex", 0x0000, 0x010000, CRC(2c786a34) SHA1(3800af04550f12e8f58b1929b49e21572f19d589) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "rocksnd.bin", 0x0000, 0x080000, CRC(c3e96650) SHA1(71952267d3149786cfef1dd49cc070664bb007a4) )
ROM_END

ROM_START( m4rckrola )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rock1.hex", 0x0000, 0x010000, CRC(b7dfd181) SHA1(566531bcb9d0b7d3caebed3f7c96f5fb23a7cef2) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "rocksnd.bin", 0x0000, 0x080000, CRC(c3e96650) SHA1(71952267d3149786cfef1dd49cc070664bb007a4) )
ROM_END

ROM_START( m4rckrolb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rock3.bin", 0x0000, 0x010000, CRC(c5134d03) SHA1(c4dfa43ffd077690d0b9a20cb82bbf92a1dc1ac9) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "rocksnd.bin", 0x0000, 0x080000, CRC(c3e96650) SHA1(71952267d3149786cfef1dd49cc070664bb007a4) )
ROM_END





/* Union
  these don't boot, at best you get a 'CLEAR' message */
GAME(199?, m4cwalk,   0,        mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Union","Cake Walk (Union) (MPU4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4eezee,   0,        mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Union","Eezee Fruits (Union) (MPU4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4frdrop,  0,        mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Union","Fruit Drop (Union) (MPU4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4gobana,  0,        mod2    ,mpu4, mpu4_state, m4default, ROT0,   "Union","Go Bananas (Union) (MPU4, set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4gobanaa, m4gobana, mod2    ,mpu4, mpu4_state, m4default, ROT0,   "Union","Go Bananas (Union) (MPU4, set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4gobanab, m4gobana, mod2    ,mpu4, mpu4_state, m4default, ROT0,   "Union","Go Bananas (Union) (MPU4, set 3)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4gobanac, m4gobana, mod2    ,mpu4, mpu4_state, m4default, ROT0,   "Union","Go Bananas (Union) (MPU4, set 4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4gobanad, m4gobana, mod2    ,mpu4, mpu4_state, m4default, ROT0,   "Union","Go Bananas (Union) (MPU4, set 5)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4lotty,   0,        mod2    ,mpu4, mpu4_state, m4default, ROT0,   "Union","Lotty Time (Union) (MPU4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4maxmze,  0,        mod2    ,mpu4, mpu4_state, m4default, ROT0,   "Union","Maximize (Union) (MPU4, set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4maxmzea, m4maxmze, mod2    ,mpu4, mpu4_state, m4default, ROT0,   "Union","Maximize (Union) (MPU4, set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4maxmzeb, m4maxmze, mod2    ,mpu4, mpu4_state, m4default, ROT0,   "Union","Maximize (Union) (MPU4, set 3)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4maxmzec, m4maxmze, mod2    ,mpu4, mpu4_state, m4default, ROT0,   "Union","Maximize (Union) (MPU4, set 4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4maxmzed, m4maxmze, mod2    ,mpu4, mpu4_state, m4default, ROT0,   "Union","Maximize (Union) (MPU4, set 5)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4mecca,   0,        mod2    ,mpu4, mpu4_state, m4default, ROT0,   "Union","Mecca Money (Union) (MPU4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4purmad,  0,        mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Union","Pure Madness (Union)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4revolv,  0,        mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Union","Revolva (Union) (MPU4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4rotex,  0,         mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Union","Rotex (Union) (MPU4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4select, 0,         mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Union","Select (Union) (MPU4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4supfru, 0,         mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Union","Supafruits (Union) (MPU4, set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4supfrua,m4supfru,  mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Union","Supafruits (Union) (MPU4, set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4trimad, 0,         mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Union","Triple Madness (Union) (MPU4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4unibox, 0,         mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Union","Unibox (Union) (MPU4, set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4uniboxa,m4unibox,  mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Union","Unibox (Union) (MPU4, set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4unique, 0,         mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Union","Unique (Union) (MPU4, set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4uniquep,m4unique,  mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Union","Unique (Union) (MPU4, set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4crzbn,  0,         mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Union","Crazy Bingo (Union) (MPU4)",   GAME_FLAGS|MACHINE_NO_SOUND )
/* Union + Empire
   same as Union above */
GAME(199?, m4gvibes,  0,        mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Union / Empire","Good Vibrations (Union - Empire) (MPU4, set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4gvibesa, m4gvibes, mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Union / Empire","Good Vibrations (Union - Empire) (MPU4, set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4rckrol,  0,        mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Union / Empire","Rock 'n' Roll (Union - Empire) (MPU4, set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4rckrola, m4rckrol, mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Union / Empire","Rock 'n' Roll (Union - Empire) (MPU4, set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4rckrolb, m4rckrol, mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Union / Empire","Rock 'n' Roll (Union - Empire) (MPU4, set 3)",   GAME_FLAGS|MACHINE_NO_SOUND )
