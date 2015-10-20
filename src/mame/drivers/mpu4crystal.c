// license:BSD-3-Clause
// copyright-holders:David Haywood
/* MPU4 games by Crystal */

/* Crystal games tend to have scrambled ROM + a different sound chip */

#include "emu.h"
#include "includes/mpu4.h"

MACHINE_CONFIG_EXTERN( mod4oki );
MACHINE_CONFIG_EXTERN( mpu4crys );
INPUT_PORTS_EXTERN( mpu4 );

#define GAME_FLAGS (MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL)



#define M4FRKSTN_SOUND \
	ROM_REGION( 0x40000, "upd", 0 ) \
	ROM_LOAD("fr1snd.bin",  0x00000, 0x40000, CRC(2d77bbde) SHA1(0397ede538e913dc2972e260589022564fcd8fe4) )

ROM_START( m4frkstn )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "fr1.536",  0x8000, 0x8000,  CRC(422b7209) SHA1(3c3f942d375a83d2470467651bca20f0feabdd3b))
	M4FRKSTN_SOUND
ROM_END

ROM_START( m4frkstna )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "db16.bin", 0x8000, 0x8000, CRC(9a36efae) SHA1(57d0df28198443502ff6d168d937a56f56dc00b2) )
	M4FRKSTN_SOUND

ROM_END
ROM_START( m4frkstnb )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "db35.bin", 0x8000, 0x8000, CRC(c576a072) SHA1(2a2f7c477ab5013b540777d9525554ddd7d3da06) )
	M4FRKSTN_SOUND
ROM_END

ROM_START( m4frkstnc )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "db35p.bin", 0x8000, 0x8000, CRC(eb929a5e) SHA1(892121a47af12a95d08cc8e4f01076c0169e367f) )
	M4FRKSTN_SOUND
ROM_END

ROM_START( m4frkstnd )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "db51.bin", 0x8000, 0x8000, CRC(d89c80cb) SHA1(88f5e39cef0448628b21a4f283008e8df2d66814) )
	M4FRKSTN_SOUND
ROM_END

ROM_START( m4frkstne )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "db51p.bin", 0x8000, 0x8000, CRC(f678bae7) SHA1(f61778517f5a7a4dd3b0af4b76a3000b9874a116) )
	M4FRKSTN_SOUND
ROM_END

ROM_START( m4frkstnf )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "frav3", 0x8000, 0x8000, CRC(06b1f6e2) SHA1(4f8013f1e07c2c41e4bac7883294d65e473f8ace) )
	M4FRKSTN_SOUND
ROM_END

ROM_START( m4frkstng )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "frav1.6", 0x08000, 0x008000, CRC(c115aaff) SHA1(e87b41f21f13ca9e3c0b5345111607bee3995f61) )
	M4FRKSTN_SOUND
ROM_END

ROM_START( m4frkstnh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "frav1", 0x0000, 0x10000, CRC(ea6f1b01) SHA1(74bcd36846e352e6d7f97e3c6b2479fd2abd76db) )
	M4FRKSTN_SOUND
ROM_END


#define M4ALADN_SOUND \
	ROM_REGION( 0x80000, "upd", 0 ) \
	ROM_LOAD( "alladinscavesnd.bin", 0x0000, 0x080000, CRC(e3831190) SHA1(3dd0e8beafb628f5138a6943518b477095ac2e56) )

ROM_START( m4aladn )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ei15ng.bin", 0x0000, 0x010000, CRC(c6142523) SHA1(b4101b14bbfa4e2b0b94c40b1ca17d484e3cf21d) )
	M4ALADN_SOUND
ROM_END

ROM_START( m4aladna )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ei15ngp.bin", 0x0000, 0x010000, CRC(d8c9fcf2) SHA1(11d3a8fc515348c72b827675991b8e990f04f8f0) )
	M4ALADN_SOUND
ROM_END

ROM_START( m4aladnb )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ei15og.bin", 0x0000, 0x010000, CRC(5bd857d5) SHA1(f673dd7676aa07cb99af9af3721067455a6af889) )
	M4ALADN_SOUND
ROM_END

ROM_START( m4aladnc )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ei15ogp.bin", 0x0000, 0x010000, CRC(45058e04) SHA1(1210502e23d6958860a4d5acd80ba5874d5979c2) )
	M4ALADN_SOUND
ROM_END

ROM_START( m4aladnd )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ei26cng.bin", 0x0000, 0x010000, CRC(5c65c8e4) SHA1(ec0cb3f306fdfc43b15fc15fa6e75dbe05d6851f) )
	M4ALADN_SOUND
ROM_END

ROM_START( m4aladne )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ei26cngp.bin", 0x0000, 0x010000, CRC(42b81135) SHA1(1d1500153a1ef2a2f1282f91ee0f371039852768) )
	M4ALADN_SOUND
ROM_END

ROM_START( m4aladnf )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "eiv35ng.bin", 0x0000, 0x010000, CRC(fc2fbedc) SHA1(af6840359e7721d035ada99839d5f80ed579f7df) )
	M4ALADN_SOUND
ROM_END

ROM_START( m4aladng )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "eiv35ogp.bin", 0x0000, 0x010000, CRC(e982cdef) SHA1(1b8d7f6bb7bcbbf80a90a8d54a27b12d9d8fa24e) )
	M4ALADN_SOUND
ROM_END

ROM_START( m4aladnh )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "eiv36ngp.bin", 0x0000, 0x010000, CRC(e2f2670d) SHA1(c4b5a0c29ff5ca33a9db198c440f8e69b1e0175e) )
	M4ALADN_SOUND
ROM_END

ROM_START( m4aladni )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "eiv36og.bin", 0x0000, 0x010000, CRC(f75f143e) SHA1(416708ce11c10134fa63f36a53e211480939f453) )
	M4ALADN_SOUND
ROM_END


#define M4BASCSH_SOUND \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) \
	/* missing? */
ROM_START( m4bagcsh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cg27.bin", 0x8000, 0x008000, CRC(2e1ce880) SHA1(fcfbbba832ae7d9e79066b27305b3406207caefc) )
	M4BASCSH_SOUND
ROM_END

ROM_START( m4bagcsha )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cg27p.bin", 0x8000, 0x008000, CRC(775384ac) SHA1(3cff28d2e5c01e84e3d01334cb0ac6e451664726) )
	M4BASCSH_SOUND
ROM_END

#define M4BUCCLB_SOUND \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) \
	/* missing? */

ROM_START( m4bucclb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "er16.bin", 0x0000, 0x010000, CRC(c57937fe) SHA1(f54a247c16fb143340fff5ef31bdc8bb9ff93be8) )
	M4BUCCLB_SOUND
ROM_END

ROM_START( m4bucclba )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "er16p.bin", 0x0000, 0x010000, CRC(d2d79dc1) SHA1(2141ce61f53a608270171c94659af1d932988707) )
ROM_END

ROM_START( m4bucclbb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "er14.bin", 0x0000, 0x010000, CRC(d54de33e) SHA1(975d2b5b6d97f6e943d49003da19e17e8d689b80) )
ROM_END

ROM_START( m4bucclbc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "er14p.bin", 0x0000, 0x010000, CRC(c2e34901) SHA1(5811e9b3d49e9a5fa32c764c00d10202aa89455a) )
ROM_END


#define M4BULLIO_SOUND \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) \
	/* missing? */

ROM_START( m4bullio )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ef15.bin", 0x0000, 0x010000, CRC(8ed6bc1f) SHA1(1deff7a819db267897f14dc76be387c6f4bd55f0) )
	M4BULLIO_SOUND
ROM_END

ROM_START( m4bullioa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ef15p.bin", 0x0000, 0x010000, CRC(900b65ce) SHA1(3747651e98ecd06fb55033cb83a6513d5d4ea032) )
	M4BULLIO_SOUND
ROM_END

ROM_START( m4bulliob )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "clbu-v16.bin", 0x0000, 0x010000, CRC(58d2fed8) SHA1(f9680b61505dea88ee62cc40dc6c375845d768dd) )
	M4BULLIO_SOUND
ROM_END


#define M4CAROU_SOUND \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) \
	ROM_LOAD( "fa_sound.bin", 0x0000, 0x080000, CRC(39837e76) SHA1(74b66f77d9af47a5caab5b6441563b196fdadb37) )
ROM_START( m4carou )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fa110.bin", 0x0000, 0x010000, CRC(ade2a7f8) SHA1(18b9287ab9747ee623e58afa2a4e6f517ff7a8ca) )
	M4CAROU_SOUND
ROM_END

ROM_START( m4caroua )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fa110p.bin", 0x0000, 0x010000, CRC(8606427d) SHA1(1a3a1ccfeb8229eaff0d9f627ed50a963c9765ed) )
	M4CAROU_SOUND
ROM_END

ROM_START( m4caroub )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fa210.bin", 0x0000, 0x010000, CRC(a3bde0fb) SHA1(54963af8b5e6836be4c0fda4b3248cf1ea68acf2) )
	M4CAROU_SOUND
ROM_END

ROM_START( m4carouc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fa210p.bin", 0x0000, 0x010000, CRC(8859057e) SHA1(3082a8612a97d868b232c2fb3cd55ef9df166b3a) )
	M4CAROU_SOUND
ROM_END


#define M4CCLIMB_SOUND \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) \
	/* missing? */

ROM_START( m4cclimb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fu204.bin", 0x0000, 0x010000, CRC(64746a30) SHA1(cdb841270851655d16b42668c4f875d7dfbfd0f1) )
	M4CCLIMB_SOUND
ROM_END

ROM_START( m4cclimba )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fu204p.bin", 0x0000, 0x010000, CRC(af0dd470) SHA1(d62bdd347130fa34c1f7f46ac9ddaaf6d035dd1c) )
	M4CCLIMB_SOUND
ROM_END

#define M4CRZCL_SOUND \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) \
	/* missing? */
ROM_START( m4crzcl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fu105.bin", 0x0000, 0x010000, CRC(d7ae1644) SHA1(d04c6f96c0f59c782a170bfabfbf670be28c9d3a) )
	M4CRZCL_SOUND
ROM_END

ROM_START( m4crzcla )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fu105f.bin", 0x0000, 0x010000, CRC(d33ea841) SHA1(8672fc875b82d7ca248bd5938fa80f540f5a3842) )
	M4CRZCL_SOUND
ROM_END

ROM_START( m4crzclb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fu105fp.bin", 0x0000, 0x010000, CRC(d31ed3f7) SHA1(c8e7916cc7faae3cbb2779b3856b65d03d702666) )
	M4CRZCL_SOUND
ROM_END

ROM_START( m4crzclc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fu105p.bin", 0x0000, 0x010000, CRC(dfff65c9) SHA1(c962e5c159301daae952bb7a5b3393aea206a4b1) )
	M4CRZCL_SOUND
ROM_END

ROM_START( m4crzcld )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cc106.bin", 0x0000, 0x010000, CRC(ff9a5f7f) SHA1(672e8b0fc0d6e985d0862e5f474bc49e1bb0c56c) )
	M4CRZCL_SOUND
ROM_END

#define M4ELITC_SOUND \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) \
	/* missing? */

ROM_START( m4elitc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ed16.bin", 0x0000, 0x010000, CRC(b4216f45) SHA1(b9d8fa4471979f074f327ac6a261000fa929d349) )
	M4ELITC_SOUND
ROM_END

ROM_START( m4elitca )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ed16p.bin", 0x0000, 0x010000, CRC(3251c0b4) SHA1(30483a67a4c363fa154e3912b4f834b97d955cc2) )
	M4ELITC_SOUND
ROM_END


#define M4FAIRG_SOUND \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) \
	ROM_LOAD( "fairgroundcrysv2-1snd.bin", 0x0000, 0x040000, CRC(9b09f98a) SHA1(e980bb0039f087ee563165a3aeb66e627fc3afe9) )
ROM_START( m4fairg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fairgroundcrysv2-16cash.bin", 0x0000, 0x010000, CRC(4a6c6470) SHA1(3211fb0245343d0fcf4581352faf606b7785f00c) )
	M4FAIRG_SOUND
ROM_END


#define M4FRMANI_SOUND \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) \
	/* missing? */

ROM_START( m4frmani )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fo23a.bin", 0x0000, 0x010000, CRC(9c7e5208) SHA1(eae08a464d0dd9c8b9ebf12eeb0c396ddd6c8779) )
	M4FRMANI_SOUND
ROM_END

ROM_START( m4frmania )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fo23ap.bin", 0x0000, 0x010000, CRC(7965c24a) SHA1(8f5ddbf2902fefa23ed0a8b3d634d4d4f81cb1b6) )
	M4FRMANI_SOUND
ROM_END

ROM_START( m4frmanib )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fo23s.bin", 0x0000, 0x010000, CRC(3cb65a7c) SHA1(6d242729cad68658a135364ddd967664760112c2) )
	M4FRMANI_SOUND
ROM_END

ROM_START( m4frmanic )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fo23sp.bin", 0x0000, 0x010000, CRC(f3f24d6d) SHA1(251c5bac250fc5e854004f16858884f3905ab26e) )
	M4FRMANI_SOUND
ROM_END



#define M4GOLDXC_SOUND \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) \
	/* missing? */
ROM_START( m4goldxc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gx105.b8", 0x0000, 0x010000, CRC(3a94ea0d) SHA1(e81f5edec6bca1d098d2c72e063b2c7456c99eda) )
	M4GOLDXC_SOUND
ROM_END

ROM_START( m4goldxca )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gx105p.b8", 0x0000, 0x010000, CRC(0e63e76c) SHA1(3af9d635043b54950f56bc5e1da514e5a3683ca0) )
	M4GOLDXC_SOUND
ROM_END

ROM_START( m4goldxcb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gx107.b8", 0x0000, 0x010000, CRC(68d6c894) SHA1(8f69243ddbe7f49191843f1b3e03e41ecbe06d59) )
	M4GOLDXC_SOUND
ROM_END

ROM_START( m4goldxcc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gx107b.b8", 0x0000, 0x010000, CRC(a93a50f9) SHA1(957513761093351ba5f776e925568c2951eea6c0) )
	M4GOLDXC_SOUND
ROM_END

ROM_START( m4goldxcd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gx207.b8", 0x0000, 0x010000, CRC(f32e6821) SHA1(cdcafd7a18237df060144ffc1969b502bdf48e9a) )
	M4GOLDXC_SOUND
ROM_END

ROM_START( m4goldxce )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gx207b.b8", 0x0000, 0x010000, CRC(b91efda9) SHA1(cf8aa37cee983ebc73b6aa1606e7404d0f323d17) )
	M4GOLDXC_SOUND
ROM_END


#define M4GOLDFC_SOUND \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) \
	ROM_LOAD( "goldfeversamplesound.bin", 0x0000, 0x080000, CRC(eb7d3c7b) SHA1(53b7c048e78506f0188b4dd2750c8dc31a625523) )


ROM_START( m4goldfc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "eo12a.bin", 0x0000, 0x010000, CRC(df519368) SHA1(10d0846ddc23eb49bcc9ba2eb9d779107e3b33f4) )
	M4GOLDFC_SOUND
ROM_END

ROM_START( m4goldfca )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "eo12ap.bin", 0x0000, 0x010000, CRC(c18c4ab9) SHA1(d2752fd09f46c2ad6b5ab0ee96aef9810483b5a4) )
	M4GOLDFC_SOUND
ROM_END

ROM_START( m4goldfcb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "goldfevergamev1.1.bin", 0x0000, 0x010000, CRC(7e5f90e5) SHA1(7e4fc6d07e9b9ff0b4c6ef59d494438c2ee0c27b) )
	M4GOLDFC_SOUND
ROM_END



#define M4HIROL_SOUND \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) \
	/* missing? */

ROM_START( m4hirol )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fv105.bin", 0x0000, 0x010000, CRC(d12963a0) SHA1(c5b6f9475ae62f15c4ba9fa391bfb17ece658091) )
	M4HIROL_SOUND
ROM_END

ROM_START( m4hirola )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fv105p.bin", 0x0000, 0x010000, CRC(facd8625) SHA1(2ace151546a9f8f191713c798cd1f26f4c69ded2) )
	M4HIROL_SOUND
ROM_END


#define M4KINGQN_SOUND \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) \
	ROM_LOAD( "kingsandqueenscrystalsnd.bin", 0x0000, 0x080000, CRC(93e4b644) SHA1(920579db52c5bb820437023e35707780ed503acc) )

ROM_START( m4kingqn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fi17.bin", 0x0000, 0x010000, CRC(c3ef9e0c) SHA1(8f3def3d5bb3e38df1fc5bec64ed47fc307856d9) )
	M4KINGQN_SOUND
ROM_END

ROM_START( m4kingqna )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fi17p.bin", 0x0000, 0x010000, CRC(f726850d) SHA1(feed467b209920c943ff55fffc5fbdcc175dfa49) )
	M4KINGQN_SOUND
ROM_END



#define M4LOTCLB_SOUND \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) \
	/* missing? */
ROM_START( m4lotclb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ds14.bin", 0x0000, 0x010000, CRC(a7e9969b) SHA1(daa1b38002c75cf4802078789955ae58d0cf163e) )
	M4LOTCLB_SOUND
ROM_END

ROM_START( m4lotclba )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ds14p.bin", 0x0000, 0x010000, CRC(2199396a) SHA1(070bca2951261f83baf1a21bf740fd12d1d8daa5) )
	M4LOTCLB_SOUND
ROM_END





#define M4MONTRL_SOUND \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) \
	ROM_LOAD( "moneytrailsnd.bin", 0x0000, 0x040000, CRC(0f0d52dc) SHA1(79e1a89858f95006a1d2a0dd18d677c84a3087c6) )

ROM_START( m4montrl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dt17.bin", 0x0000, 0x010000, CRC(18d0c6a0) SHA1(34b055a19f3c0ab975c68a273d8dfd4c326e2089) )
	M4MONTRL_SOUND
ROM_END

ROM_START( m4montrla )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dt17p.bin", 0x0000, 0x010000, CRC(a45f8f82) SHA1(a1f070d34fa260d591a20f3786218c64995cafff) )
	M4MONTRL_SOUND
ROM_END

ROM_START( m4montrlb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dt36.bin", 0x0000, 0x010000, CRC(e294cb1b) SHA1(59267859d8db90ca2d7b5413f196f9ac6b9c0f23) )
	M4MONTRL_SOUND
ROM_END

ROM_START( m4montrlc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dt36p.bin", 0x0000, 0x010000, CRC(fc4912ca) SHA1(a99fec8b94adca1c372ed0f836f071ccfa1c2415) )
	M4MONTRL_SOUND
ROM_END

ROM_START( m4montrld )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "moneytrail.bin", 0x0000, 0x010000, CRC(e5533728) SHA1(f1abd59d57a5eca42640992bc543990a3ee8058d) )
	M4MONTRL_SOUND
ROM_END




#define M4MYSTIQ_SOUND \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) \
	/* missing? */

ROM_START( m4mystiq )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fw103.p1", 0x0000, 0x010000, CRC(85234d9b) SHA1(06457892c6a0fafb826d3d8bc99f23c8b6c4374d) )
	M4MYSTIQ_SOUND
ROM_END

ROM_START( m4mystiqa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fw103-fp.p1", 0x0000, 0x010000, CRC(a5530d17) SHA1(d8585e0ca3ed66db272291561aa8baaab1dff66b) )
	M4MYSTIQ_SOUND
ROM_END

ROM_START( m4mystiqb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fw103d.p1", 0x0000, 0x010000, CRC(b1ea569a) SHA1(fa82a25043162930d2424f5df29f71c2c330f666) )
	M4MYSTIQ_SOUND
ROM_END

ROM_START( m4mystiqc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fw103fpd.p1", 0x0000, 0x010000, CRC(919a1616) SHA1(15e5db55ecdf1151090401c46a0379d339d9c615) )
	M4MYSTIQ_SOUND
ROM_END


#define M4NUDWIN_SOUND \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) \
	/* missing? */
ROM_START( m4nudwin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dn56.bin", 0x8000, 0x008000, CRC(ca90f7a8) SHA1(1ae92162f02feb5f391617d4180ef1c154e10d1a) )
	M4NUDWIN_SOUND
ROM_END

ROM_START( m4nudwina )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dn56p.bin", 0x8000, 0x008000, CRC(1640a1ef) SHA1(70d573746244166b358457710e6e2c9437ea2745) )
	M4NUDWIN_SOUND
ROM_END


#define M4PARACL_SOUND \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) \
	/* missing? */


ROM_START( m4paracl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dl12.bin", 0x0000, 0x010000, CRC(6398aecf) SHA1(290a21a5b3a15643f657939bccf3d677f22a3ef4) )
	M4PARACL_SOUND
ROM_END

ROM_START( m4paracla )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dl12p.bin", 0x0000, 0x010000, CRC(e5e8013e) SHA1(8b6cb5aed1b82c8a298c9e798f89ab07ff330caf) )
	M4PARACL_SOUND
ROM_END


#define M4RLPICK_SOUND \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) \
	/* missing? */
ROM_START( m4rlpick )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fh15a.bin", 0x0000, 0x010000, CRC(ac448a6e) SHA1(4a99f7b293476e3e477f37cbd28f1e2a99b0f2d2) )
	M4RLPICK_SOUND
ROM_END

ROM_START( m4rlpicka )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fh15pa.bin", 0x0000, 0x010000, CRC(29f857b9) SHA1(0f8858bac5865c6c789be052cc037fbdbe87f135) )
	M4RLPICK_SOUND
ROM_END

ROM_START( m4rlpickb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fh15ps.bin", 0x0000, 0x010000, CRC(b6ec1edf) SHA1(759f18332ac50452c9725ce1a3e53b56334bd365) )
	M4RLPICK_SOUND
ROM_END

ROM_START( m4rlpickc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fh15s.bin", 0x0000, 0x010000, CRC(55b0a7e7) SHA1(d4568f82040de0fc6eb08fe90754d2b33e427ab0) )
	M4RLPICK_SOUND
ROM_END



#define M4TWSTR_SOUND \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) \
	ROM_LOAD( "twistersound.bin", 0x000000, 0x100000, CRC(5aa2729b) SHA1(cf490ec6c75c038addcff24655cd6e498cad60c1) )

ROM_START( m4twstr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fda103.bin", 0x0000, 0x010000, CRC(5520b91c) SHA1(0b219c9232e89c5a7da8f857aa58f828cc5730f4) )
	M4TWSTR_SOUND
ROM_END

ROM_START( m4twstra )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fda103p.bin", 0x0000, 0x010000, CRC(61e9a21d) SHA1(940abedbbfaa40534c8be7136369cbf404e4c9a8) )
	M4TWSTR_SOUND
ROM_END

ROM_START( m4twstrb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fds103.bin", 0x0000, 0x010000, CRC(4282cba9) SHA1(a664d342482e50604f9a585a1887f342b95caf00) )
	M4TWSTR_SOUND
ROM_END

ROM_START( m4twstrc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fds103p.bin", 0x0000, 0x010000, CRC(764bd0a8) SHA1(cea826c94ce28ab3e47071846756a7e1effa5d1b) )
	M4TWSTR_SOUND
ROM_END

ROM_START( m4twstrd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "twisterprog.bin", 0x0000, 0x010000, CRC(96209a3e) SHA1(434aa824a517929d9ace959b06f2d38464a54541) )
	M4TWSTR_SOUND
ROM_END





#define M4TWSTCL_SOUND \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) \
	ROM_LOAD( "twistersound.bin", 0x000000, 0x100000, CRC(5aa2729b) SHA1(cf490ec6c75c038addcff24655cd6e498cad60c1) )/* From original, may be wrong */
ROM_START( m4twstcl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fl106.bin", 0x0000, 0x010000, CRC(d43f06a6) SHA1(d3762853dee2779a06a02ad3c1dfd804053d7f7d) )
	M4TWSTCL_SOUND
ROM_END

ROM_START( m4twstcla )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fl106fp.bin", 0x0000, 0x010000, CRC(6716baf7) SHA1(f4f60e1c798682aee17a74ddf376aca113b6b701) )
	M4TWSTCL_SOUND
ROM_END

ROM_START( m4twstclb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fl106p.bin", 0x0000, 0x010000, CRC(93dd36e7) SHA1(eded1b5748cb24af80ab392a5fd4d2cc312a66a2) )
	M4TWSTCL_SOUND
ROM_END




#define M4DZ_SOUND \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) \
	ROM_LOAD( "dangerzonesnd.bin", 0x0000, 0x080000, CRC(bdfcffa2) SHA1(9e3be8fd1c42fd19afcde682662bef82f7e0f7e9) )
ROM_START( m4dz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dangerzone1_2.bin", 0x8000, 0x08000, CRC(eb4582f8) SHA1(df4cbbbb927b512b1ace34986ce29b17d7815e49) )
	M4DZ_SOUND
ROM_END

#define M4TYLB_SOUND \
	ROM_REGION( 0x080000, "msm6376", ROMREGION_ERASE00 ) \
	ROM_LOAD( "tylbsnd.bin", 0x0000, 0x080000, CRC(781175c7) SHA1(43cf6fe91c756cdd4acc735411ac166647bf29e7) )

ROM_START( m4tylb )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "tylb-10n.bin", 0x0000, 0x010000, CRC(8339329b) SHA1(3b20be519cc94f03a899372a3cb4f1a584457879) ) /* only 0x8000-0xffff used */
	M4TYLB_SOUND
ROM_END

ROM_START( m4tylba )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "tylb1_3.bin", 0x0000, 0x010000, CRC(84b1a12d) SHA1(b78270c84cd4b2fc6cd1b0177f37ced83ff7054a) ) /* only 0x8000-0xffff used */
	M4TYLB_SOUND
ROM_END


#define M4MAGI7_SOUND \
	ROM_REGION( 0x080000, "msm6376", ROMREGION_ERASE00 ) \
	/* missing? */
ROM_START( m4magi7 )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "m716.bin", 0x0000, 0x010000, CRC(a26d52a8) SHA1(34228654a922f6c2b01c3fbf1a58755ec6968cbc) ) /* only 0x8000-0xffff used */
	M4MAGI7_SOUND
ROM_END

ROM_START( m4magi7a )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "m799.bin", 0x0000, 0x010000, CRC(2084e4a1) SHA1(5e365fb19b51b2f8c8a42d18c8b7db5b17ac77ff) ) /* only 0x8000-0xffff used */
	M4MAGI7_SOUND
ROM_END


#define M4RAGS_SOUND \
	ROM_REGION( 0x080000, "msm6376", ROMREGION_ERASE00 ) \
	/* missing? */
ROM_START( m4rags )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "ct15.bin", 0x8000, 0x008000, CRC(27eab355) SHA1(0ad6a09015b2ddfe87563f3d88f84d2d5b3c74a0) )
	M4RAGS_SOUND
ROM_END

ROM_START( m4ragsa )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "ct15p.bin", 0x8000, 0x008000, CRC(7ea5df79) SHA1(d1c355db97010dc9075217efe40d6620258ae3e4) )
	M4RAGS_SOUND
ROM_END

ROM_START( m4ragsb )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "ct21.bin", 0x8000, 0x008000, CRC(78a37f00) SHA1(83c1f17317eecacb1f1172a49540b6e48ba19f7e) )
	M4RAGS_SOUND
ROM_END

ROM_START( m4ragsc )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "ct21p.bin", 0x8000, 0x008000, CRC(21ec132c) SHA1(91279edc0d680219dfe03160b6abd32cafa74bdf) )
	M4RAGS_SOUND
ROM_END

#define M4RIOCR_SOUND \
	ROM_REGION( 0x080000, "msm6376", ROMREGION_ERASE00 ) \
	/* missing? */

ROM_START( m4riocr )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "rg13n.bin", 0x8000, 0x008000, CRC(ebb2c0da) SHA1(14a7efb5747a14eb6aed90b79ceb0622aae88370) )
	M4RIOCR_SOUND
ROM_END

ROM_START( m4riocra )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "rg13p.bin", 0x8000, 0x008000, CRC(013fc8db) SHA1(ef3541c6a57dbe4701e08488527da32d398593d4) )
	M4RIOCR_SOUND
ROM_END

#define M4NDUP_SOUND \
	ROM_REGION( 0x080000, "msm6376", ROMREGION_ERASE00 ) \
	/* missing? */
ROM_START( m4ndup )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "ndu26n.bin", 0x8000, 0x008000, CRC(562668f6) SHA1(aaebdb649e0399551f32520d28b27d7654271fee) )
	M4NDUP_SOUND
ROM_END

ROM_START( m4ndupa )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "ndu26p.bin", 0x8000, 0x008000, CRC(f766875e) SHA1(13682878289a248a1540a24bf5c56b200c59da42) )
	M4NDUP_SOUND
ROM_END

ROM_START( m4ndupb )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "ndub81n.bin", 0x8000, 0x008000, CRC(39d52323) SHA1(37f510560b71705860044f5d4259cf1a34210403) )
	M4NDUP_SOUND
ROM_END

ROM_START( m4ndupc )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "nduj81n.bin", 0x8000, 0x008000, CRC(769fce55) SHA1(ab53507991f4af96fca3fd82d66ea29baabb69d1) )
	M4NDUP_SOUND
ROM_END



GAME(199?, m4frkstn ,0          ,mpu4crys   ,mpu4       , mpu4_state,m_frkstn,  ROT0,   "Crystal","Frank 'n' Stein (Crystal) (MPU4, set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4frkstna,m4frkstn   ,mpu4crys   ,mpu4       , mpu4_state,m_frkstn,  ROT0,   "Crystal","Frank 'n' Stein (Crystal) (MPU4, set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4frkstnb,m4frkstn   ,mpu4crys   ,mpu4       , mpu4_state,m_frkstn,  ROT0,   "Crystal","Frank 'n' Stein (Crystal) (MPU4, set 3)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4frkstnc,m4frkstn   ,mpu4crys   ,mpu4       , mpu4_state,m_frkstn,  ROT0,   "Crystal","Frank 'n' Stein (Crystal) (MPU4, set 4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4frkstnd,m4frkstn   ,mpu4crys   ,mpu4       , mpu4_state,m_frkstn,  ROT0,   "Crystal","Frank 'n' Stein (Crystal) (MPU4, set 5)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4frkstne,m4frkstn   ,mpu4crys   ,mpu4       , mpu4_state,m_frkstn,  ROT0,   "Crystal","Frank 'n' Stein (Crystal) (MPU4, set 6)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4frkstnf,m4frkstn   ,mpu4crys   ,mpu4       , mpu4_state,m_frkstn,  ROT0,   "Crystal","Frank 'n' Stein (Crystal) (MPU4, set 7)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4frkstng,m4frkstn   ,mpu4crys   ,mpu4       , mpu4_state,m_frkstn,  ROT0,   "Crystal","Frank 'n' Stein (Crystal) (MPU4, set 8)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4frkstnh,m4frkstn   ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Frank 'n' Stein (Crystal) (MPU4, set 9)",   GAME_FLAGS|MACHINE_NO_SOUND ) // this set is encrypted
GAME(199?, m4aladn  ,0          ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Aladdin's Cave (Crystal) (MPU4) (set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4aladna ,m4aladn    ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Aladdin's Cave (Crystal) (MPU4) (set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4aladnb ,m4aladn    ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Aladdin's Cave (Crystal) (MPU4) (set 3)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4aladnc ,m4aladn    ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Aladdin's Cave (Crystal) (MPU4) (set 4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4aladnd ,m4aladn    ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Aladdin's Cave (Crystal) (MPU4) (set 5)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4aladne ,m4aladn    ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Aladdin's Cave (Crystal) (MPU4) (set 6)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4aladnf ,m4aladn    ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Aladdin's Cave (Crystal) (MPU4) (set 7)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4aladng ,m4aladn    ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Aladdin's Cave (Crystal) (MPU4) (set 8)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4aladnh ,m4aladn    ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Aladdin's Cave (Crystal) (MPU4) (set 9)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4aladni ,m4aladn    ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Aladdin's Cave (Crystal) (MPU4) (set 10)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4bagcsh ,0          ,mpu4crys   ,mpu4       , mpu4_state,m_frkstn,  ROT0,   "Crystal","Bags Of Cash Club (Crystal) (MPU4) (set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4bagcsha,m4bagcsh   ,mpu4crys   ,mpu4       , mpu4_state,m_frkstn,  ROT0,   "Crystal","Bags Of Cash Club (Crystal) (MPU4) (set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4bucclb ,0          ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Buccaneer Club (Crystal) (MPU4) (set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4bucclba,m4bucclb   ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Buccaneer Club (Crystal) (MPU4) (set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4bucclbb,m4bucclb   ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Buccaneer Club (Crystal) (MPU4) (set 3)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4bucclbc,m4bucclb   ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Buccaneer Club (Crystal) (MPU4) (set 4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4bullio ,0          ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Bullion Club (Crystal) (MPU4) (set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4bullioa,m4bullio   ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Bullion Club (Crystal) (MPU4) (set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4bulliob,m4bullio   ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Bullion Club (Crystal) (MPU4) (set 3)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4carou  ,0          ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Carousel Club (Crystal) (MPU4) (set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4caroua ,m4carou    ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Carousel Club (Crystal) (MPU4) (set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4caroub ,m4carou    ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Carousel Club (Crystal) (MPU4) (set 3)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4carouc ,m4carou    ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Carousel Club (Crystal) (MPU4) (set 4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4cclimb ,0          ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Crazy Climber (Crystal) (MPU4) (set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4cclimba,m4cclimb   ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Crazy Climber (Crystal) (MPU4) (set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4crzcl  ,0          ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Crazy Climber Club (Crystal) (MPU4) (set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4crzcla ,m4crzcl    ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Crazy Climber Club (Crystal) (MPU4) (set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4crzclb ,m4crzcl    ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Crazy Climber Club (Crystal) (MPU4) (set 3)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4crzclc ,m4crzcl    ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Crazy Climber Club (Crystal) (MPU4) (set 4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4crzcld ,m4crzcl    ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Crazy Climber Club (Crystal) (MPU4) (set 5)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4elitc  ,0          ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Elite Club (Crystal) (MPU4) (set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4elitca ,m4elitc    ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Elite Club (Crystal) (MPU4) (set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4fairg  ,0          ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Fairground (Crystal) (MPU4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4frmani ,0          ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Fruit Mania (Crystal) (MPU4) (set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4frmania,m4frmani   ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Fruit Mania (Crystal) (MPU4) (set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4frmanib,m4frmani   ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Fruit Mania (Crystal) (MPU4) (set 3)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4frmanic,m4frmani   ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Fruit Mania (Crystal) (MPU4) (set 4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4goldxc ,0          ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Gold Exchange Club (Crystal) (MPU4) (set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4goldxca,m4goldxc   ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Gold Exchange Club (Crystal) (MPU4) (set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4goldxcb,m4goldxc   ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Gold Exchange Club (Crystal) (MPU4) (set 3)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4goldxcc,m4goldxc   ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Gold Exchange Club (Crystal) (MPU4) (set 4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4goldxcd,m4goldxc   ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Gold Exchange Club (Crystal) (MPU4) (set 5)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4goldxce,m4goldxc   ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Gold Exchange Club (Crystal) (MPU4) (set 6)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4goldfc ,0          ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Gold Fever (Crystal) (MPU4) (set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4goldfca,m4goldfc   ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Gold Fever (Crystal) (MPU4) (set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4goldfcb,m4goldfc   ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Gold Fever (Crystal) (MPU4) (set 3)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4hirol  ,0          ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Hi Roller Club (Crystal) (MPU4) (set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4hirola ,m4hirol    ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Hi Roller Club (Crystal) (MPU4) (set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4kingqn ,0          ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Kings & Queens Club (Crystal) (MPU4) (set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4kingqna,m4kingqn   ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Kings & Queens Club (Crystal) (MPU4) (set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4lotclb ,0          ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Lottery Club (Crystal) (MPU4) (set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4lotclba,m4lotclb   ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Lottery Club (Crystal) (MPU4) (set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4montrl ,0          ,mpu4crys   ,mpu4       , mpu4_state,crystali,  ROT0,   "Crystal","Money Trail (Crystal) (MPU4) (set 1)",   GAME_FLAGS|MACHINE_NO_SOUND ) // encryption is inverted!
GAME(199?, m4montrla,m4montrl   ,mpu4crys   ,mpu4       , mpu4_state,crystali,  ROT0,   "Crystal","Money Trail (Crystal) (MPU4) (set 2)",   GAME_FLAGS|MACHINE_NO_SOUND ) // encryption is inverted!
GAME(199?, m4montrlb,m4montrl   ,mpu4crys   ,mpu4       , mpu4_state,crystali,  ROT0,   "Crystal","Money Trail (Crystal) (MPU4) (set 3)",   GAME_FLAGS|MACHINE_NO_SOUND ) // encryption is inverted!
GAME(199?, m4montrlc,m4montrl   ,mpu4crys   ,mpu4       , mpu4_state,crystali,  ROT0,   "Crystal","Money Trail (Crystal) (MPU4) (set 4)",   GAME_FLAGS|MACHINE_NO_SOUND ) // encryption is inverted!
GAME(199?, m4montrld,m4montrl   ,mpu4crys   ,mpu4       , mpu4_state,crystali,  ROT0,   "Crystal","Money Trail (Crystal) (MPU4) (set 5)",   GAME_FLAGS|MACHINE_NO_SOUND ) // encryption is inverted!
GAME(199?, m4mystiq ,0          ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Mystique Club (Crystal) (MPU4) (set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4mystiqa,m4mystiq   ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Mystique Club (Crystal) (MPU4) (set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4mystiqb,m4mystiq   ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Mystique Club (Crystal) (MPU4) (set 3)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4mystiqc,m4mystiq   ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Mystique Club (Crystal) (MPU4) (set 4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4nudwin ,0          ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Nudge & Win (Crystal) (MPU4) (set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4nudwina,m4nudwin   ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Nudge & Win (Crystal) (MPU4) (set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4paracl ,0          ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Paradise Club (Crystal) (MPU4) (set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4paracla,m4paracl   ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Paradise Club (Crystal) (MPU4) (set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4rlpick ,0          ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Reel Picks (Crystal) (MPU4) (set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4rlpicka,m4rlpick   ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Reel Picks (Crystal) (MPU4) (set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4rlpickb,m4rlpick   ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Reel Picks (Crystal) (MPU4) (set 3)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4rlpickc,m4rlpick   ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Reel Picks (Crystal) (MPU4) (set 4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4twstr  ,0          ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Twister (Crystal) (MPU4) (set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4twstra ,m4twstr    ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Twister (Crystal) (MPU4) (set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4twstrb ,m4twstr    ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Twister (Crystal) (MPU4) (set 3)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4twstrc ,m4twstr    ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Twister (Crystal) (MPU4) (set 4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4twstrd ,m4twstr    ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Twister (Crystal) (MPU4) (set 5)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4twstcl ,0          ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Twister Club (Crystal) (MPU4) (set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4twstcla,m4twstcl   ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Twister Club (Crystal) (MPU4) (set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4twstclb,m4twstcl   ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Twister Club (Crystal) (MPU4) (set 3)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4dz     ,0          ,mpu4crys   ,mpu4       , mpu4_state,crystal,   ROT0,   "Crystal","Danger Zone (Crystal) (MPU4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4tylb   ,0          ,mod4oki    ,mpu4       , mpu4_state,m4default, ROT0,   "Crystal","Thank Your Lucky Bars (Crystal) (MPU4) (set 1)",GAME_FLAGS|MACHINE_NO_SOUND|MACHINE_MECHANICAL )
GAME(199?, m4tylba  ,m4tylb     ,mod4oki    ,mpu4       , mpu4_state,m4default, ROT0,   "Crystal","Thank Your Lucky Bars (Crystal) (MPU4) (set 2)",GAME_FLAGS|MACHINE_NO_SOUND|MACHINE_MECHANICAL )
GAME(199?, m4magi7  ,0          ,mod4oki    ,mpu4       , mpu4_state,m4default, ROT0,   "Crystal","Magic 7's (Crystal) (MPU4) (set 1)",GAME_FLAGS|MACHINE_NO_SOUND|MACHINE_MECHANICAL )
GAME(199?, m4magi7a ,m4magi7    ,mod4oki    ,mpu4       , mpu4_state,m4default, ROT0,   "Crystal","Magic 7's (Crystal) (MPU4) (set 2)",GAME_FLAGS|MACHINE_NO_SOUND|MACHINE_MECHANICAL )
GAME(199?, m4rags   ,0          ,mod4oki    ,mpu4       , mpu4_state,m4default, ROT0,   "Crystal","Rags To Riches Club (Crystal) (MPU4) (set 1)",GAME_FLAGS|MACHINE_NO_SOUND|MACHINE_MECHANICAL )
GAME(199?, m4ragsa  ,m4rags     ,mod4oki    ,mpu4       , mpu4_state,m4default, ROT0,   "Crystal","Rags To Riches Club (Crystal) (MPU4) (set 2)",GAME_FLAGS|MACHINE_NO_SOUND|MACHINE_MECHANICAL )
GAME(199?, m4ragsb  ,m4rags     ,mod4oki    ,mpu4       , mpu4_state,m4default, ROT0,   "Crystal","Rags To Riches Club (Crystal) (MPU4) (set 3)",GAME_FLAGS|MACHINE_NO_SOUND|MACHINE_MECHANICAL )
GAME(199?, m4ragsc  ,m4rags     ,mod4oki    ,mpu4       , mpu4_state,m4default, ROT0,   "Crystal","Rags To Riches Club (Crystal) (MPU4) (set 4)",GAME_FLAGS|MACHINE_NO_SOUND|MACHINE_MECHANICAL )
GAME(199?, m4riocr  ,0          ,mod4oki    ,mpu4       , mpu4_state,m4default, ROT0,   "Crystal","Rio Grande (Crystal) (MPU4) (set 1)",GAME_FLAGS|MACHINE_NO_SOUND|MACHINE_MECHANICAL )
GAME(199?, m4riocra ,m4riocr    ,mod4oki    ,mpu4       , mpu4_state,m4default, ROT0,   "Crystal","Rio Grande (Crystal) (MPU4) (set 2)",GAME_FLAGS|MACHINE_NO_SOUND|MACHINE_MECHANICAL )
GAME(199?, m4ndup   ,0          ,mod4oki    ,mpu4       , mpu4_state,m4default, ROT0,   "Crystal","Nudge Double Up Deluxe (Crystal) (MPU4) (set 1)",GAME_FLAGS|MACHINE_NO_SOUND|MACHINE_MECHANICAL )
GAME(199?, m4ndupa  ,m4ndup     ,mod4oki    ,mpu4       , mpu4_state,m4default, ROT0,   "Crystal","Nudge Double Up Deluxe (Crystal) (MPU4) (set 2)",GAME_FLAGS|MACHINE_NO_SOUND|MACHINE_MECHANICAL )
GAME(199?, m4ndupb  ,m4ndup     ,mod4oki    ,mpu4       , mpu4_state,m4default, ROT0,   "Crystal","Nudge Double Up Deluxe (Crystal) (MPU4) (set 3)",GAME_FLAGS|MACHINE_NO_SOUND|MACHINE_MECHANICAL )
GAME(199?, m4ndupc  ,m4ndup     ,mod4oki    ,mpu4       , mpu4_state,m4default, ROT0,   "Crystal","Nudge Double Up Deluxe (Crystal) (MPU4) (set 4)",GAME_FLAGS|MACHINE_NO_SOUND|MACHINE_MECHANICAL )
