// license:BSD-3-Clause
// copyright-holders:David Haywood
/* MPU4 games by MDM */


#include "emu.h"
#include "includes/mpu4.h"

MACHINE_CONFIG_EXTERN( mod4oki );
INPUT_PORTS_EXTERN( mpu4 );

#define GAME_FLAGS (MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL)



ROM_START( m42punlm )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "2pun0-0.bin", 0x0000, 0x020000, CRC(f8fd7b92) SHA1(400a66d0b401b2df2e2fb0f70eae6da7e547a50b) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "ctnsnd.hex", 0x0000, 0x080000, CRC(150a4513) SHA1(97147e11b49d18225c527d8a0926118a83ee906c))
ROM_END

ROM_START( m4silnud )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sn1_0.bin", 0x0000, 0x010000, CRC(b51ba096) SHA1(c280b8ba4aecd2256e268b9623f84070f38beeb8) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "ctnsnd.hex", 0x0000, 0x080000, CRC(150a4513) SHA1(97147e11b49d18225c527d8a0926118a83ee906c)) // == 2p Unlimited?
ROM_END

ROM_START( m4nud2p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "classic 2p nudger v0-1 (27512)", 0x0000, 0x010000, CRC(9c22f5f8) SHA1(5391f5d5b3a0861b93702e476d9635ff304a02a2) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "ctnsnd.hex", 0x0000, 0x080000, CRC(150a4513) SHA1(97147e11b49d18225c527d8a0926118a83ee906c)) // == 2p Unlimited?
ROM_END


ROM_START( m4ctn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ctn0_3.hex", 0x0000, 0x010000, CRC(7573bbc7) SHA1(bc15b03c9fddbd0b93be259547c5420f0623fd58) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "ctnsnd.hex", 0x0000, 0x080000, CRC(150a4513) SHA1(97147e11b49d18225c527d8a0926118a83ee906c)) // == 2p Unlimited?
ROM_END


ROM_START( m4bigapl )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "app1_5.bin", 0x0000, 0x010000, CRC(ebe6e65f) SHA1(aae70efc4b7e0ad9125424acef634361439e0594) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "bigapplesnd.p1", 0x000000, 0x080000, CRC(4afba9b0) SHA1(0a60bb5897ed4403c30902ba12290fd61284d0e7) )
	ROM_LOAD( "bigapplesnd.p2", 0x080000, 0x080000, CRC(7de7b74f) SHA1(419998b585e856925d838fe25ecd9f6f41dfc6a8) )
ROM_END

ROM_START( m4bigapla )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "app1_5c", 0x0000, 0x010000, CRC(458e77ce) SHA1(c01f2dd52c67381b4c09051e6a696841ef0777eb) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "bigapplesnd.p1", 0x000000, 0x080000, CRC(4afba9b0) SHA1(0a60bb5897ed4403c30902ba12290fd61284d0e7) )
	ROM_LOAD( "bigapplesnd.p2", 0x080000, 0x080000, CRC(7de7b74f) SHA1(419998b585e856925d838fe25ecd9f6f41dfc6a8) )
ROM_END

ROM_START( m4bigaplb )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "ba2_0.bin", 0x0000, 0x010000, CRC(2d3db68d) SHA1(9bed97820a527b83f515dca06f032c332cbc11d2) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "bigapplesnd.p1", 0x000000, 0x080000, CRC(4afba9b0) SHA1(0a60bb5897ed4403c30902ba12290fd61284d0e7) )
	ROM_LOAD( "bigapplesnd.p2", 0x080000, 0x080000, CRC(7de7b74f) SHA1(419998b585e856925d838fe25ecd9f6f41dfc6a8) )
ROM_END

ROM_START( m4bigaplc )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "ba2_0d.bin", 0x0000, 0x010000, CRC(aa9d93ee) SHA1(08c58bcffdf943873a713a006ecb104423b9bb93) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "bigapplesnd.p1", 0x000000, 0x080000, CRC(4afba9b0) SHA1(0a60bb5897ed4403c30902ba12290fd61284d0e7) )
	ROM_LOAD( "bigapplesnd.p2", 0x080000, 0x080000, CRC(7de7b74f) SHA1(419998b585e856925d838fe25ecd9f6f41dfc6a8) )
ROM_END

ROM_START( m4bigapld )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "ba6_0x.bin", 0x0000, 0x010000, CRC(7c67032f) SHA1(38f44ec527995a850a0b8fe1d9eab4ee9cae06fa) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "bigapplesnd.p1", 0x000000, 0x080000, CRC(4afba9b0) SHA1(0a60bb5897ed4403c30902ba12290fd61284d0e7) )
	ROM_LOAD( "bigapplesnd.p2", 0x080000, 0x080000, CRC(7de7b74f) SHA1(419998b585e856925d838fe25ecd9f6f41dfc6a8) )
ROM_END

ROM_START( m4bigaple )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "ba7_0x.bin", 0x0000, 0x010000, CRC(f393490a) SHA1(48d23e9deb60fe99f9cbe5601053ee6df2b9bf8b) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "bigapplesnd.p1", 0x000000, 0x080000, CRC(4afba9b0) SHA1(0a60bb5897ed4403c30902ba12290fd61284d0e7) )
	ROM_LOAD( "bigapplesnd.p2", 0x080000, 0x080000, CRC(7de7b74f) SHA1(419998b585e856925d838fe25ecd9f6f41dfc6a8) )
ROM_END


ROM_START( m4blztrl )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "bt1_2.bin", 0x0000, 0x010000, CRC(184f5277) SHA1(52342577a09f787f77f8e026e8f7a11998681fb5) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "blazingtrails.p1", 0x000000, 0x080000, CRC(2b82701b) SHA1(377fed5d378d6d3abdce59ba9e7f0d7edd756337) )
	ROM_LOAD( "blazingtrails.p2", 0x080000, 0x080000, CRC(9338b7fc) SHA1(9252e94feac0d65f40152bb9d049cea85ecd16fa) )
	ROM_LOAD( "blazingtrails.p3", 0x100000, 0x080000, CRC(ef37f3fa) SHA1(4e71296cd8eb61ff6b4cf9136dbecdcd6d167472) )
ROM_END

ROM_START( m4blztrla )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "bt1_3.bin", 0x0000, 0x010000, CRC(6ae5901d) SHA1(0cd2293ca445549e6c0e0a6f9f6adcf5cdc935b0) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "blazingtrails.p1", 0x000000, 0x080000, CRC(2b82701b) SHA1(377fed5d378d6d3abdce59ba9e7f0d7edd756337) )
	ROM_LOAD( "blazingtrails.p2", 0x080000, 0x080000, CRC(9338b7fc) SHA1(9252e94feac0d65f40152bb9d049cea85ecd16fa) )
	ROM_LOAD( "blazingtrails.p3", 0x100000, 0x080000, CRC(ef37f3fa) SHA1(4e71296cd8eb61ff6b4cf9136dbecdcd6d167472) )
ROM_END



ROM_START( m4bodymt )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "bm0-1.bin", 0x0000, 0x020000, CRC(15c379d9) SHA1(04d3c869870a4eda4dfd075b1c1a6efb1cf3bf57) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "bmsnd1.bin", 0x000000, 0x080000, CRC(7f645f66) SHA1(db16e92b6b0c9ac12f7305a4f874a304b93404e6) )
	ROM_LOAD( "bmsnd2.bin", 0x080000, 0x080000, CRC(b8062605) SHA1(570deb41ab8523c5d9b6281a86b915852f6a2305) )
ROM_END


ROM_START( m4coloss )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "col0_6k.bin", 0x0000, 0x010000, CRC(53d2431a) SHA1(44da207ce0ba24d110a1aaf6c0705f9c2245d212) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "colsnd1.bin", 0x000000, 0x080000, CRC(a6fa2c68) SHA1(e5572b37086a28bee9ce4dfe549ed60ddbffe444) )
	ROM_LOAD( "colsnd2.bin", 0x080000, 0x080000, CRC(8b01f0cb) SHA1(990fb0b51ddf4eb3f436e11d01d0e5e3b2465ac5) )
ROM_END

ROM_START( m4colossa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "col0_6s.bin", 0x0000, 0x010000, CRC(e4db7ff7) SHA1(0f8a15b40923ac1cf8780b3b99b0ce070ed1d13d) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "colsnd1.bin", 0x000000, 0x080000, CRC(a6fa2c68) SHA1(e5572b37086a28bee9ce4dfe549ed60ddbffe444) )
	ROM_LOAD( "colsnd2.bin", 0x080000, 0x080000, CRC(8b01f0cb) SHA1(990fb0b51ddf4eb3f436e11d01d0e5e3b2465ac5) )
ROM_END

ROM_START( m4colossb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "col2_0x.bin", 0x0000, 0x010000, CRC(fae24132) SHA1(91bdafe8bfba2b6b350c783fd46963846ca481c8) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "colsnd1.bin", 0x000000, 0x080000, CRC(a6fa2c68) SHA1(e5572b37086a28bee9ce4dfe549ed60ddbffe444) )
	ROM_LOAD( "colsnd2.bin", 0x080000, 0x080000, CRC(8b01f0cb) SHA1(990fb0b51ddf4eb3f436e11d01d0e5e3b2465ac5) )
ROM_END

ROM_START( m4colossc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "col3_0x.bin", 0x0000, 0x010000, CRC(40e0ae7f) SHA1(e6a40c07efbde324091f8a52e615e367ccbb4eaf) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "colsnd1.bin", 0x000000, 0x080000, CRC(a6fa2c68) SHA1(e5572b37086a28bee9ce4dfe549ed60ddbffe444) )
	ROM_LOAD( "colsnd2.bin", 0x080000, 0x080000, CRC(8b01f0cb) SHA1(990fb0b51ddf4eb3f436e11d01d0e5e3b2465ac5) )
ROM_END

ROM_START( m4colossd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "col3_1.bin", 0x0000, 0x010000, CRC(79d7f4fb) SHA1(8d4a19fbde135d95b5f0e6978a8b89baa4dfe139) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "colsnd1.bin", 0x000000, 0x080000, CRC(a6fa2c68) SHA1(e5572b37086a28bee9ce4dfe549ed60ddbffe444) )
	ROM_LOAD( "colsnd2.bin", 0x080000, 0x080000, CRC(8b01f0cb) SHA1(990fb0b51ddf4eb3f436e11d01d0e5e3b2465ac5) )
ROM_END

ROM_START( m4colosse )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "coll10.bin", 0x0000, 0x010000, CRC(a2468607) SHA1(e926025548e4c0ad1e97b35215b2d28c058126dd) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "colsnd1.bin", 0x000000, 0x080000, CRC(a6fa2c68) SHA1(e5572b37086a28bee9ce4dfe549ed60ddbffe444) )
	ROM_LOAD( "colsnd2.bin", 0x080000, 0x080000, CRC(8b01f0cb) SHA1(990fb0b51ddf4eb3f436e11d01d0e5e3b2465ac5) )
ROM_END

ROM_START( m4colossf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "colossus-v1.0.bin", 0x0000, 0x010000, CRC(4fc41c62) SHA1(1c088dd278e414081e98689a49b8305c3d3d4db3) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "colsnd1.bin", 0x000000, 0x080000, CRC(a6fa2c68) SHA1(e5572b37086a28bee9ce4dfe549ed60ddbffe444) )
	ROM_LOAD( "colsnd2.bin", 0x080000, 0x080000, CRC(8b01f0cb) SHA1(990fb0b51ddf4eb3f436e11d01d0e5e3b2465ac5) )
ROM_END

ROM_START( m4colossg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "colossus_8.bin", 0x0000, 0x010000, CRC(4ab3ee66) SHA1(2b61b6f9b43592826f7cb755898fcbc4a381f9b3) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "colsnd1.bin", 0x000000, 0x080000, CRC(a6fa2c68) SHA1(e5572b37086a28bee9ce4dfe549ed60ddbffe444) )
	ROM_LOAD( "colsnd2.bin", 0x080000, 0x080000, CRC(8b01f0cb) SHA1(990fb0b51ddf4eb3f436e11d01d0e5e3b2465ac5) )
ROM_END


ROM_START( m4firebl )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "fb2_0x.bin", 0x0000, 0x010000, CRC(78b6d22f) SHA1(cb23ac9985a39a6052156732a9be1588207834ce) )
ROM_END

ROM_START( m4firebla )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "fb3_1.bin", 0x0000, 0x010000, CRC(baa497e1) SHA1(5732fbb9b590fc389cad21a40ac264d01461f6e5) )
ROM_END

ROM_START( m4fireblb )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "fb3_3x.bin", 0x0000, 0x010000, CRC(bc4f65cb) SHA1(8397437f6098b246568666e06e8bf40a3f2ea51c) )
ROM_END

ROM_START( m4fireblc )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "fba2_6c", 0x0000, 0x010000, CRC(f2cdbb1f) SHA1(fa12e8992d8af6ded01ab54eedff56ab050e8ee1) )
ROM_END

ROM_START( m4firebld ) // looks weird, bad dump?
	ROM_REGION( 0x080000, "temp", 0 )
	ROM_LOAD( "fba2_6", 0x0000, 0x080000, CRC(cf16b2d0) SHA1(32249fb15708dfa408c4642be7a41fba7aeda657) )

	ROM_REGION( 0x010000, "maincpu", 0 )
	// reports src/mame/drivers/mpu4.c: m4firebld has ROM fba2_6 extending past the defined memory region (bug)
	//ROM_LOAD( "fba2_6", 0x0000, 0x010000, CRC(cf16b2d0) SHA1(32249fb15708dfa408c4642be7a41fba7aeda657) )
	//ROM_IGNORE(0x070000)
	ROM_COPY( "temp", nullptr, 0x0000, 0x010000 )
ROM_END



ROM_START( m4mayhem )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "may4_0.bin", 0x0000, 0x010000, CRC(fbbe89eb) SHA1(e5e2e4adabfa41d130dbb7a77c147105ef20ac79) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "mayhemsnd.p1", 0x000000, 0x080000, CRC(f61650ec) SHA1(946fd801ea5f4dd09a911460a709f3942fa412af) )
	ROM_LOAD( "mayhemsnd.p2", 0x080000, 0x080000, CRC(637a6b41) SHA1(d342309b78af21a35f50fb23dd2c7ed737abfdb9) )
ROM_END

ROM_START( m4mayhema )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mayhemv2.bin", 0x0000, 0x010000, CRC(e7c79dd0) SHA1(b7af1fd4853a6ff33d2f3960737caece91714681) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "mayhemsnd.p1", 0x000000, 0x080000, CRC(f61650ec) SHA1(946fd801ea5f4dd09a911460a709f3942fa412af) )
	ROM_LOAD( "mayhemsnd.p2", 0x080000, 0x080000, CRC(637a6b41) SHA1(d342309b78af21a35f50fb23dd2c7ed737abfdb9) )
ROM_END


ROM_START( m4themob )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mob1_5.bin", 0x0000, 0x010000, CRC(adbdbf43) SHA1(650d9af466a258d55d9f6703968501a6eebddfef) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "mobsnd1.bin", 0x000000, 0x080000, CRC(b15c0eb4) SHA1(5d06853a1218f10741ea43bd72a48a727b62ddb1) )
	ROM_LOAD( "mobsnd2.bin", 0x080000, 0x080000, CRC(6ef3b72c) SHA1(47bf1edacd9da7249e19342234985c746ebf1c4b) )
	ROM_LOAD( "mobsnd3.bin", 0x100000, 0x080000, CRC(62a2bf65) SHA1(155536dc29eecc34351c5726d26f02ee8cb0d014) )
ROM_END

ROM_START( m4themoba )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mob1_5d.bin", 0x0000, 0x010000, CRC(2a1d9a20) SHA1(1cd07edcf75f17a98b6377fbfdf88fd7c0abd864) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "mobsnd1.bin", 0x000000, 0x080000, CRC(b15c0eb4) SHA1(5d06853a1218f10741ea43bd72a48a727b62ddb1) )
	ROM_LOAD( "mobsnd2.bin", 0x080000, 0x080000, CRC(6ef3b72c) SHA1(47bf1edacd9da7249e19342234985c746ebf1c4b) )
	ROM_LOAD( "mobsnd3.bin", 0x100000, 0x080000, CRC(62a2bf65) SHA1(155536dc29eecc34351c5726d26f02ee8cb0d014) )
ROM_END


ROM_START( m4themobb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mob1_6.bin", 0x0000, 0x010000, CRC(9c718009) SHA1(99b259d5a93f4657ad2b4ae6cd0b2e1324178022) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "mobsnd1.bin", 0x000000, 0x080000, CRC(b15c0eb4) SHA1(5d06853a1218f10741ea43bd72a48a727b62ddb1) )
	ROM_LOAD( "mobsnd2.bin", 0x080000, 0x080000, CRC(6ef3b72c) SHA1(47bf1edacd9da7249e19342234985c746ebf1c4b) )
	ROM_LOAD( "mobsnd3.bin", 0x100000, 0x080000, CRC(62a2bf65) SHA1(155536dc29eecc34351c5726d26f02ee8cb0d014) )
ROM_END

ROM_START( m4nudbon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nb4_0.bin", 0x0000, 0x010000, CRC(c7802b0f) SHA1(6ebff8ccbd2baf6de55d764d2e7cb34f2ff2384f) )
ROM_END

ROM_START( m4nudbona )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nubonza.bin", 0x0000, 0x010000, CRC(ce8b1c9b) SHA1(3f4e019256ddbd668c8cadecf86015b78b5eaf8c) )
ROM_END


ROM_START( m4nudgem )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gem2_0.bin", 0x0000, 0x010000, CRC(bbab66b8) SHA1(a3a7d40d0ca41e57cd0d6965c0306edca372da1d) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "nudgejems.p1", 0x0000, 0x080000, CRC(e875d82e) SHA1(50fb941ad801397ef3dee651be126c01c9423386) )
ROM_END











ROM_START( m4smshgb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sm2_0x.bin", 0x0000, 0x010000, CRC(52042750) SHA1(2fb5ece50aef457bdbdc1fb880b1a87638551545) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "sngsnd.p1", 0x000000, 0x080000, CRC(0bff940e) SHA1(e53df95cd33d759f89f0278312e6e5f9b8abe341) )
	ROM_LOAD( "sngsnd.p2", 0x080000, 0x080000, CRC(414aa5a5) SHA1(c6e6bba8c4655dc761dd040c4d615f9d4a739663) )
ROM_END

ROM_START( m4smshgba )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sm3_0x.bin", 0x0000, 0x010000, CRC(9a4bb2bd) SHA1(8642b15f658e855c0d682fe84b024ddd85eb527e) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "sngsnd.p1", 0x000000, 0x080000, CRC(0bff940e) SHA1(e53df95cd33d759f89f0278312e6e5f9b8abe341) )
	ROM_LOAD( "sngsnd.p2", 0x080000, 0x080000, CRC(414aa5a5) SHA1(c6e6bba8c4655dc761dd040c4d615f9d4a739663) )
ROM_END

ROM_START( m4smshgbb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sm6_0.bin", 0x0000, 0x010000, CRC(7a58d60f) SHA1(989c816dadf01500be01e2c333befef0c3e12054) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "sngsnd.p1", 0x000000, 0x080000, CRC(0bff940e) SHA1(e53df95cd33d759f89f0278312e6e5f9b8abe341) )
	ROM_LOAD( "sngsnd.p2", 0x080000, 0x080000, CRC(414aa5a5) SHA1(c6e6bba8c4655dc761dd040c4d615f9d4a739663) )
ROM_END

ROM_START( m4smshgbc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sma5_8c", 0x0000, 0x010000, CRC(79e12ac0) SHA1(9e8d4ea8f97d1f73ccab079cbf57aa89a12d2e7d) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "sngsnd.p1", 0x000000, 0x080000, CRC(0bff940e) SHA1(e53df95cd33d759f89f0278312e6e5f9b8abe341) )
	ROM_LOAD( "sngsnd.p2", 0x080000, 0x080000, CRC(414aa5a5) SHA1(c6e6bba8c4655dc761dd040c4d615f9d4a739663) )
ROM_END


ROM_START( m4snklad )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "snakesnladders.bin", 0x0000, 0x010000, CRC(52bdc684) SHA1(00d052629b214b48a9a1d68622ba188206276166) )
ROM_END


ROM_START( m4excam )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ex1_4.bin", 0x0000, 0x010000, CRC(34c4aee2) SHA1(c5487c5b0144ca188bc2e3926a0343fd4c9c565a) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "ex1_0d.bin", 0x0000, 0x010000, CRC(490c510e) SHA1(21a03d8e2dd4d2c7760acbff5705f925fe9f31be) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "mdmexcalibsnd.p1", 0x000000, 0x080000, CRC(8ea73366) SHA1(3ee45ad98e03177eeef97521df7b3d1945242076) )
	ROM_LOAD( "mdmexcalibsnd.p2", 0x080000, 0x080000, CRC(0fca6ca2) SHA1(2029d15e3b51069f5847ab3846bf6c064f0a3381) )
	ROM_LOAD( "mdmexcalibsnd.p3", 0x100000, 0x080000, CRC(43be816a) SHA1(a95f702ec1bb20f3e0f18984948963b56769f5ba) )
	ROM_LOAD( "mdmexcalibsnd.p4", 0x180000, 0x080000, CRC(ef8a718c) SHA1(093a5fff5bab61fc9276a7f9f3c5b728a50603b3) )
ROM_END


ROM_START( m4front )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ff2_1.bin", 0x0000, 0x010000, CRC(3519cba1) SHA1(d83a5370ee82e258024d20ffacec7050950b1326) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "ffsnd2_1.bin", 0x000000, 0x080000, CRC(9b3cdf12) SHA1(1a209985493f686dd37e91693361ecbf32096f66) )
	ROM_LOAD( "ffsnd2_2.bin", 0x080000, 0x080000, CRC(0fc33bdf) SHA1(6de715e33411050ee1d2a0f08bf1c9a8001ffb4f) )
ROM_END


ROM_START( m4safar )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "saf4_1.bin", 0x0000, 0x010000, CRC(ad726457) SHA1(4104be61d179024fae9fb9c631677b1ba56d3f00) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )

ROM_END

ROM_START( m4snowbl )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "sb_3.p2", 0x8000, 0x004000, CRC(c22fe04e) SHA1(233ee0795b0029389247a9550ef39af95f671870) )
	ROM_LOAD( "sb_3.p1", 0xc000, 0x004000, CRC(98fdcaba) SHA1(f4a74d5550dd9fc8bff35a583b3289e1bb0be9d5) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
ROM_END

ROM_START( m4ewshft )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "each_way_shifter(mdm)_v1-0.bin", 0x0000, 0x010000, CRC(506b6cf0) SHA1(870e356b9785e51c5be5d6bc6af9ea7640b51ee8) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "each_way_shifter-snd1.bin", 0x000000, 0x080000, CRC(b21f9b09) SHA1(69ac3ca2874fc3aebd34dd225a195ad1c0305d00) )
	ROM_LOAD( "each_way_shifter-snd2.bin", 0x080000, 0x080000, CRC(e3ce5ec5) SHA1(9c7eefa4042b1b1aca3d0fbefcad10db34992c43) )
ROM_END


/* MDM
   most of these boot and act similar to the Empire games (ie bad text, but run OK) */
GAME(199?, m42punlm,     0,     mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm","2p Unlimited (Mdm) (MPU4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4silnud,     0,     mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm?","Silver Nudger (Mdm?) (MPU4)",                     GAME_FLAGS|MACHINE_NO_SOUND ) // code is close to 2p Unlimited, same sound rom
GAME(199?, m4nud2p,     0,      mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm?","2p Nudger (Mdm?) (MPU4)",                     GAME_FLAGS|MACHINE_NO_SOUND ) // code is close to 2p Unlimited, same sound rom
GAME(199?, m4ctn,     0,        mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm?","Tuppenny Nudger Classic (Mdm?) (MPU4)",                       GAME_FLAGS|MACHINE_NO_SOUND ) // code is close to 2p Unlimited, same sound rom
GAME(199?, m4bigapl,  0,        mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm","The Big Apple (Mdm) (MPU4, set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4bigapla, m4bigapl, mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm","The Big Apple (Mdm) (MPU4, set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4bigaplb, m4bigapl, mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm","The Big Apple (Mdm) (MPU4, set 3)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4bigaplc, m4bigapl, mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm","The Big Apple (Mdm) (MPU4, set 4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4bigapld, m4bigapl, mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm","The Big Apple (Mdm) (MPU4, set 5)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4bigaple, m4bigapl, mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm","The Big Apple (Mdm) (MPU4, set 6)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4blztrl,  0,        mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm","Blazing Trails (Mdm) (MPU4, set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4blztrla, m4blztrl, mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm","Blazing Trails (Mdm) (MPU4, set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4bodymt,  0,        mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm","Body Match (Mdm) (MPU4)",   GAME_FLAGS|MACHINE_NO_SOUND ) // doesn't boot, various alarms
GAME(199?, m4coloss,  0,        mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm","Colossus (Mdm) (MPU4, set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4colossa, m4coloss, mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm","Colossus (Mdm) (MPU4, set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4colossb, m4coloss, mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm","Colossus (Mdm) (MPU4, set 3)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4colossc, m4coloss, mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm","Colossus (Mdm) (MPU4, set 4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4colossd, m4coloss, mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm","Colossus (Mdm) (MPU4, set 5)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4colosse, m4coloss, mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm","Colossus (Mdm) (MPU4, set 6)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4colossf, m4coloss, mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm","Colossus (Mdm) (MPU4, set 7)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4colossg, m4coloss, mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm","Colossus (Mdm) (MPU4, set 8)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4firebl,  0,        mod2    ,mpu4, mpu4_state, m4default, ROT0,   "Mdm","Fireball (Mdm) (MPU4, set 1)",   GAME_FLAGS|MACHINE_NO_SOUND ) // hangs after spin (sound status?)
GAME(199?, m4firebla, m4firebl, mod2    ,mpu4, mpu4_state, m4default, ROT0,   "Mdm","Fireball (Mdm) (MPU4, set 2)",   GAME_FLAGS|MACHINE_NO_SOUND ) // hangs after spin (sound status?)
GAME(199?, m4fireblb, m4firebl, mod2    ,mpu4, mpu4_state, m4default, ROT0,   "Mdm","Fireball (Mdm) (MPU4, set 3)",   GAME_FLAGS|MACHINE_NO_SOUND ) // hangs after spin (sound status?)
GAME(199?, m4fireblc, m4firebl, mod2    ,mpu4, mpu4_state, m4default, ROT0,   "Mdm","Fireball (Mdm) (MPU4, set 4)",   GAME_FLAGS|MACHINE_NO_SOUND ) // hangs after spin (sound status?)
GAME(199?, m4firebld, m4firebl, mod2    ,mpu4, mpu4_state, m4default, ROT0,   "Mdm","Fireball (Mdm) (MPU4, set 5)",   GAME_FLAGS|MACHINE_NO_SOUND ) // hangs after spin (sound status?)
GAME(199?, m4mayhem,  0,        mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm","Mayhem (Mdm) (MPU4, set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4mayhema, m4mayhem, mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm","Mayhem (Mdm) (MPU4, set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4themob,  0,        mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm","The Mob (Mdm) (MPU4, set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4themoba, m4themob, mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm","The Mob (Mdm) (MPU4, set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4themobb, m4themob, mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm","The Mob (Mdm) (MPU4, set 3)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4nudbon,  0,        mod2    ,mpu4, mpu4_state, m4default, ROT0,   "Mdm","Nudge Bonanza (Mdm) (MPU4, set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4nudbona, m4nudbon, mod2    ,mpu4, mpu4_state, m4default, ROT0,   "Mdm","Nudge Bonanza (Mdm) (MPU4, set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4nudgem,  0,        mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm","Nudge Gems (Mdm) (MPU4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4smshgb, 0,         mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm","Smash 'n' Grab (Mdm) (MPU4, set 1)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4smshgba,m4smshgb,  mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm","Smash 'n' Grab (Mdm) (MPU4, set 2)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4smshgbb,m4smshgb,  mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm","Smash 'n' Grab (Mdm) (MPU4, set 3)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4smshgbc,m4smshgb,  mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Mdm","Smash 'n' Grab (Mdm) (MPU4, set 4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4snklad, 0,         mod2    ,mpu4, mpu4_state, m4default, ROT0,   "Mdm","Snakes & Ladders (Mdm) (MPU4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4excam, 0,          mod4oki ,mpu4, mpu4_state, m4default, ROT0,   "Mdm","Excalibur (Mdm) (MPU4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4front, 0,          mod4oki ,mpu4, mpu4_state, m4default, ROT0,   "Mdm","Final Frontier (Mdm) (MPU4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4safar, 0,          mod4oki ,mpu4, mpu4_state, m4default, ROT0,   "Mdm","Safari Club (Mdm) (MPU4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4snowbl, 0,         mod2    ,mpu4, mpu4_state, m4default, ROT0,   "Mdm","Snowball Bingo (Mdm) (MPU4)",   GAME_FLAGS|MACHINE_NO_SOUND )
GAME(199?, m4ewshft, 0,         mod4oki ,mpu4, mpu4_state, m4default, ROT0,   "Mdm","Each Way Shifter (Mdm) (MPU4)",   GAME_FLAGS|MACHINE_NO_SOUND )
