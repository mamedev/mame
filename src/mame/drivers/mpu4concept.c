// license:BSD-3-Clause
// copyright-holders:David Haywood
/* MPU4 games by 'Concept' */

#include "emu.h"
#include "includes/mpu4.h"

MACHINE_CONFIG_EXTERN( mod4oki );
INPUT_PORTS_EXTERN( mpu4 );

#define GAME_FLAGS (MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL)


#define M4RHFEVC_EXTRA_ROMS \
	ROM_REGION( 0x080000, "msm6376", 0 ) \
	ROM_LOAD( "rhfs1.bin", 0x0000, 0x03de5e, CRC(0dddd05f) SHA1(908a58752fb1cf76667695a40bcaa7778201c3a2) )
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4RHFEVC_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default_big ,ROT0,company,title,GAME_FLAGS )


GAME_CUSTOM( 199?, m4rhfevc,       0,          "rhb6a58e.bin", 0x0000, 0x010000, CRC(c5a1ec02) SHA1(3a4dc552fffc34673e590e903a5c15a409f9aeec), "Concept Games Ltd","Red Hot Fever (Concept Games Ltd) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4rhfevc__a,    m4rhfevc,   "rhf1a101.bin", 0x0000, 0x010000, CRC(e3a5caf2) SHA1(228fc1c7a6fa5029fe7c5a5feb6de1de6d703bc1), "Concept Games Ltd","Red Hot Fever (Concept Games Ltd) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4rhfevc__b,    m4rhfevc,   "rhf1a102.bin", 0x0000, 0x010000, CRC(cd5b8788) SHA1(40d0b956291d3f858fe968d0bdf11929fe191f0a), "Concept Games Ltd","Red Hot Fever (Concept Games Ltd) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4rhfevc__c,    m4rhfevc,   "rhf1a103.bin", 0x0000, 0x010000, CRC(7ad8b857) SHA1(840aa944e3da9ef4c5e724d7d22a8fe04f1b0d35), "Concept Games Ltd","Red Hot Fever (Concept Games Ltd) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4rhfevc__d,    m4rhfevc,   "rhf1a104.bin", 0x0000, 0x010000, CRC(0ce94f67) SHA1(2a4869ef38bf00fc21eba5583e8a23eb6db5c574), "Concept Games Ltd","Red Hot Fever (Concept Games Ltd) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4rhfevc__e,    m4rhfevc,   "rhf1a106.bin", 0x0000, 0x010000, CRC(fb5f5346) SHA1(1863e93bd0221d7851b9d7ebd9c802c8435ad490), "Concept Games Ltd","Red Hot Fever (Concept Games Ltd) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4rhfevc__f,    m4rhfevc,   "rhf1a107.bin", 0x0000, 0x010000, CRC(f37860d1) SHA1(8986e0112bdc30f4ac338a8e42e8f32e16dd2902), "Concept Games Ltd","Red Hot Fever (Concept Games Ltd) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4rhfevc__g,    m4rhfevc,   "rhf1a108.bin", 0x0000, 0x010000, CRC(5b4fd8cd) SHA1(5d70c7cb71b57c6d808e84d42219d7aa4b2ab858), "Concept Games Ltd","Red Hot Fever (Concept Games Ltd) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4rhfevc__h,    m4rhfevc,   "rhf1a109.bin", 0x0000, 0x010000, CRC(24c0b660) SHA1(d50fd13c225362fd72e51de9a3159d1a78ee7442), "Concept Games Ltd","Red Hot Fever (Concept Games Ltd) (MPU4) (set 9)" )
GAME_CUSTOM( 199?, m4rhfevc__i,    m4rhfevc,   "rhf1a115.bin", 0x0000, 0x010000, CRC(244694b7) SHA1(ad1cc41d6a43bddeb641dfa0099a9bcc2c16a07c), "Concept Games Ltd","Red Hot Fever (Concept Games Ltd) (MPU4) (set 10)" )
GAME_CUSTOM( 199?, m4rhfevc__j,    m4rhfevc,   "rhf1a125.bin", 0x0000, 0x010000, CRC(060cfc77) SHA1(4d65665763fb4987a9c6aa4ba7f3feed4387e3fc), "Concept Games Ltd","Red Hot Fever (Concept Games Ltd) (MPU4) (set 11)" )
GAME_CUSTOM( 199?, m4rhfevc__k,    m4rhfevc,   "rhf1a126.bin", 0x0000, 0x010000, CRC(6901e28d) SHA1(e0cf4beb97a359df22aa89576793bab3f89e2f71), "Concept Games Ltd","Red Hot Fever (Concept Games Ltd) (MPU4) (set 12)" )
GAME_CUSTOM( 199?, m4rhfevc__l,    m4rhfevc,   "rhf1a128.bin", 0x0000, 0x010000, CRC(bd82a28d) SHA1(e6a5bb5595e3561ba7d21e6da108239545baa1ae), "Concept Games Ltd","Red Hot Fever (Concept Games Ltd) (MPU4) (set 13)" )
GAME_CUSTOM( 199?, m4rhfevc__m,    m4rhfevc,   "rhf1a129.bin", 0x0000, 0x010000, CRC(228725d3) SHA1(aaac6d001f1568821549cdfd44a56c407f0391ee), "Concept Games Ltd","Red Hot Fever (Concept Games Ltd) (MPU4) (set 14)" )
GAME_CUSTOM( 199?, m4rhfevc__n,    m4rhfevc,   "rhf1a130.bin", 0x0000, 0x010000, CRC(f576770c) SHA1(d0e44768de4673c38bd17a504dafb7df40ac8bd3), "Concept Games Ltd","Red Hot Fever (Concept Games Ltd) (MPU4) (set 15)" )
GAME_CUSTOM( 199?, m4rhfevc__o,    m4rhfevc,   "rhf1a131.bin", 0x0000, 0x010000, CRC(9fa11675) SHA1(02d5d3f8feebee898f34b68b29ac369df8e49ca4), "Concept Games Ltd","Red Hot Fever (Concept Games Ltd) (MPU4) (set 16)" )
GAME_CUSTOM( 199?, m4rhfevc__p,    m4rhfevc,   "rhf1a132.bin", 0x0000, 0x010000, CRC(3baa2ee4) SHA1(e18533e50533ed59f6bad26c4085bbd91a71c024), "Concept Games Ltd","Red Hot Fever (Concept Games Ltd) (MPU4) (set 17)" )
GAME_CUSTOM( 199?, m4rhfevc__q,    m4rhfevc,   "rhf1g127.bin", 0x0000, 0x010000, CRC(63de982a) SHA1(e1f77b7c5afb9c7d4682312a0864dbaff72ed7a9), "Concept Games Ltd","Red Hot Fever (Concept Games Ltd) (MPU4) (set 18)" )
GAME_CUSTOM( 199?, m4rhfevc__r,    m4rhfevc,   "rhf1g128.bin", 0x0000, 0x010000, CRC(88119fd0) SHA1(96a80af400522890cb912244f1c26ff60d6f1d87), "Concept Games Ltd","Red Hot Fever (Concept Games Ltd) (MPU4) (set 19)" )
GAME_CUSTOM( 199?, m4rhfevc__s,    m4rhfevc,   "rhf1g129.bin", 0x0000, 0x010000, CRC(afd31bc6) SHA1(738643ac9956738708d423e43ae0d14b20560ed2), "Concept Games Ltd","Red Hot Fever (Concept Games Ltd) (MPU4) (set 20)" )
GAME_CUSTOM( 199?, m4rhfevc__t,    m4rhfevc,   "rhf1g130.bin", 0x0000, 0x010000, CRC(8de45d11) SHA1(0839cbd4954c6587761d2b654487bc2236992313), "Concept Games Ltd","Red Hot Fever (Concept Games Ltd) (MPU4) (set 21)" )
GAME_CUSTOM( 199?, m4rhfevc__u,    m4rhfevc,   "rhf1g131.bin", 0x0000, 0x010000, CRC(60ea3303) SHA1(c9946047b7833626fba493fcd1cd6fbe8214bf73), "Concept Games Ltd","Red Hot Fever (Concept Games Ltd) (MPU4) (set 22)" )
GAME_CUSTOM( 199?, m4rhfevc__v,    m4rhfevc,   "rhf1g132.bin", 0x0000, 0x010000, CRC(2645c98b) SHA1(27ed2433ab9111b99c4bd375f3926c8d44e519ed), "Concept Games Ltd","Red Hot Fever (Concept Games Ltd) (MPU4) (set 23)" )
GAME_CUSTOM( 199?, m4rhfevc__w,    m4rhfevc,   "rhf2a101.bin", 0x0000, 0x010000, CRC(206c9ab6) SHA1(01acc4ce9731f273d0eb68a4cdbe323c13384b30), "Concept Games Ltd","Red Hot Fever (Concept Games Ltd) (MPU4) (set 24)" )
GAME_CUSTOM( 199?, m4rhfevc__x,    m4rhfevc,   "rhf2g101.bin", 0x0000, 0x010000, CRC(bae39dae) SHA1(d34f0506415f681d145f3c1b3949667a6dfdde95), "Concept Games Ltd","Red Hot Fever (Concept Games Ltd) (MPU4) (set 25)" )
GAME_CUSTOM( 199?, m4rhfevc__y,    m4rhfevc,   "lxled1.bin",   0x0000, 0x010000, CRC(84a652b7) SHA1(0efb11480a0435140b43f77a7c7bca5e51e3aefb), "Concept Games Ltd","Red Hot Fever (Concept Games Ltd) (MPU4) (set 26)" )



#define M4PULWNC_EXTRA_ROMS \
	ROM_REGION( 0x080000, "msm6376", 0 ) \
	ROM_LOAD( "paws1.bin", 0x0000, 0x03e72f, CRC(124b14ba) SHA1(b6b84c306c83b3159d88992ad88a10aff161fce8) )
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4PULWNC_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default_big ,ROT0,company,title,GAME_FLAGS )



GAME_CUSTOM( 199?, m4pulwnc,       0,          "pawhc02.bin",  0x0000, 0x010000, CRC(599bfe96) SHA1(5960e60b52b1b965c76e5df40ef255e6d79c9ac2), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4pulwnc__a,    m4pulwnc,   "pawhc03.bin",  0x0000, 0x010000, CRC(78262206) SHA1(8d8d9ca982d2d69bdb750be6654a7253f539ee31), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4pulwnc__b,    m4pulwnc,   "pawhc05.bin",  0x0000, 0x010000, CRC(7683b547) SHA1(71ffdc8e6952c0dad652b67691ed88f3674ce80d), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4pulwnc__c,    m4pulwnc,   "pawhm01.bin",  0x0000, 0x010000, CRC(076b8162) SHA1(1a13f07a57a6cc1486b710012c29e5a45ab6e258), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4pulwnc__d,    m4pulwnc,   "pawhs09.bin",  0x0000, 0x010000, CRC(a51c9f45) SHA1(74675dcf4299f10fffd998dfbfa59f541deff3cc), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4pulwnc__e,    m4pulwnc,   "pawhs10.bin",  0x0000, 0x010000, CRC(7e4c27bf) SHA1(aea811790bf505d3a60dc4145225eb441c4733f5), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4pulwnc__f,    m4pulwnc,   "pawuc06.bin",  0x0000, 0x010000, CRC(e4a97fe0) SHA1(ae59f6637866d7f8d9ebd08e1bf72f1e99c67bbe), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4pulwnc__g,    m4pulwnc,   "pawuc08.bin",  0x0000, 0x010000, CRC(f5500f6a) SHA1(0ec680589edcbee86e85f1d2454ce11efaab416c), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4pulwnc__h,    m4pulwnc,   "pawus09.bin",  0x0000, 0x010000, CRC(650ed3df) SHA1(995ac45346b925c823b393361dcd7d77d413278d), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 9)" )
GAME_CUSTOM( 199?, m4pulwnc__i,    m4pulwnc,   "pawus10.bin",  0x0000, 0x010000, CRC(dcaeb369) SHA1(65604c0c77f6887c9967a9b1ef0fbcce5535127b), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 10)" )
GAME_CUSTOM( 199?, m4pulwnc__j,    m4pulwnc,   "pw2ds14.bin",  0x0000, 0x010000, CRC(12327bfd) SHA1(b0f4ee8eef61fdb203384883de06715455c12907), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 11)" )
GAME_CUSTOM( 199?, m4pulwnc__k,    m4pulwnc,   "pw2ds15.bin",  0x0000, 0x010000, CRC(8f094db8) SHA1(a2a323bd917770b3be364fcaca08aef155c2f934), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 12)" )
GAME_CUSTOM( 199?, m4pulwnc__l,    m4pulwnc,   "pw2ds16.bin",  0x0000, 0x010000, CRC(19b3de66) SHA1(80ffba6eff992fbfea4f7c31f434192250970a78), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 13)" )
GAME_CUSTOM( 199?, m4pulwnc__m,    m4pulwnc,   "pw2ds17.bin",  0x0000, 0x010000, CRC(a105e0b4) SHA1(10c0abd4c82c99da2126961fb39981c036725e02), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 14)" )
GAME_CUSTOM( 199?, m4pulwnc__n,    m4pulwnc,   "pw2hc05.bin",  0x0000, 0x010000, CRC(46381370) SHA1(39a3cc2d8ad0b1799f89db380a3362139f96a2b7), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 15)" )
GAME_CUSTOM( 199?, m4pulwnc__o,    m4pulwnc,   "pw2hc06.bin",  0x0000, 0x010000, CRC(12139fdd) SHA1(77cc4ca1ce5659129b0f4b3caaed76a905840d03), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 16)" )
GAME_CUSTOM( 199?, m4pulwnc__p,    m4pulwnc,   "pw2hc07.bin",  0x0000, 0x010000, CRC(f4883446) SHA1(9eba19b74d1182be6ec9ec2778d93ac83aa9fed1), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 17)" )
GAME_CUSTOM( 199?, m4pulwnc__q,    m4pulwnc,   "pw2hc08.bin",  0x0000, 0x010000, CRC(62075be2) SHA1(ea5d3c82a1e0d6f914d86dce156453f47bf1d635), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 18)" )
GAME_CUSTOM( 199?, m4pulwnc__r,    m4pulwnc,   "pw2hc10.bin",  0x0000, 0x010000, CRC(9918d018) SHA1(3a51a5b04a749f5ec49dd7a25fc1c50509f3ab7c), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 19)" )
GAME_CUSTOM( 199?, m4pulwnc__s,    m4pulwnc,   "pw2hm01.bin",  0x0000, 0x010000, CRC(330a0961) SHA1(99d831c90d9b7f336ad2d27e024ccf896f87f0c7), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 20)" )
GAME_CUSTOM( 199?, m4pulwnc__t,    m4pulwnc,   "pw2hm02.bin",  0x0000, 0x010000, CRC(43ebb988) SHA1(3eec2db9731c2bdf6e768ca0ff2c21aa02758c36), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 21)" )
GAME_CUSTOM( 199?, m4pulwnc__u,    m4pulwnc,   "pw2hm04.bin",  0x0000, 0x010000, CRC(83d7cfa8) SHA1(7a1ee63ab1860cdf6edd6a7945d41a7229ac114a), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 22)" )
GAME_CUSTOM( 199?, m4pulwnc__v,    m4pulwnc,   "pw2hs11.bin",  0x0000, 0x010000, CRC(c3fb0cea) SHA1(53b6c8b0a9ccf037b988616f789ab0175ca1482d), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 23)" )
GAME_CUSTOM( 199?, m4pulwnc__w,    m4pulwnc,   "pw2hs15.bin",  0x0000, 0x010000, CRC(227cd67e) SHA1(37eb5605ef37cb9ce2dd3be0d61b1f74a90cb686), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 24)" )
GAME_CUSTOM( 199?, m4pulwnc__x,    m4pulwnc,   "pw2hs16.bin",  0x0000, 0x010000, CRC(70d368fe) SHA1(560f967107c20ca6910ebd20be0d48db5739e53d), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 25)" )
GAME_CUSTOM( 199?, m4pulwnc__y,    m4pulwnc,   "pw2hs17.bin",  0x0000, 0x010000, CRC(ef7a5ded) SHA1(f34852db80e49016ff53bad4d975606cfdb06493), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 26)" )
GAME_CUSTOM( 199?, m4pulwnc__z,    m4pulwnc,   "pw2mh03.bin",  0x0000, 0x010000, CRC(9d4859c7) SHA1(90bed8937c581726852e911c56827108ae983f1e), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 27)" )
GAME_CUSTOM( 199?, m4pulwnc__0,    m4pulwnc,   "pw3cs01.bin",  0x0000, 0x010000, CRC(7e4e07f9) SHA1(52b9a086da53621c92571c0209171595cb78c479), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 28)" )
GAME_CUSTOM( 199?, m4pulwnc__1,    m4pulwnc,   "pw3cs02.bin",  0x0000, 0x010000, CRC(c85736a9) SHA1(147535fc56b4d40cca076200a17a696251c4ec90), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 29)" )
GAME_CUSTOM( 199?, m4pulwnc__2,    m4pulwnc,   "show03.bin",   0x0000, 0x010000, CRC(d89ae7fc) SHA1(1887b2066447c6e90d3fb9a4259e87e857793c7d), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 30)" )
GAME_CUSTOM( 199?, m4pulwnc__3,    m4pulwnc,   "show04.bin",   0x0000, 0x010000, CRC(5a96d640) SHA1(bf36c2327c527583a0c758afdbb329e773151d9c), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 31)" )
GAME_CUSTOM( 199?, m4pulwnc__4,    m4pulwnc,   "tchm02.bin",   0x0000, 0x010000, CRC(09aa9ad6) SHA1(91ae2d677aefde936daf211051719aa49966398f), "Concept Games Ltd","Pull-A-Win (Concept Games Ltd) (MPU4) (set 32)" )



#define M4SPNWNC_EXTRA_ROMS \
	ROM_REGION( 0x080000, "msm6376", 0 ) \
	ROM_LOAD( "sawsnd1", 0x0000, 0x080000, CRC(7957381f) SHA1(8fd45e5bf67248607f7d98032e08516ded493d74) )
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4SPNWNC_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default_big ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4spnwnc,       0,          "saw01.bin", 0x0000, 0x010000, CRC(5350e50e) SHA1(0d7ba3280eddb4400545729c55bcfaff7918d553), "Concept Games Ltd","Spin-A-Win (Concept Games Ltd) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4spnwnc__a,    m4spnwnc,   "saw02.bin", 0x0000, 0x010000, CRC(daf85100) SHA1(ff89adb0d6530bcf5ff0807f48c6008198948d50), "Concept Games Ltd","Spin-A-Win (Concept Games Ltd) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4spnwnc__b,    m4spnwnc,   "saw03.bin", 0x0000, 0x010000, CRC(a891451d) SHA1(2c0a4b6b5c50e234715e103e72986a2bda1d4588), "Concept Games Ltd","Spin-A-Win (Concept Games Ltd) (MPU4) (set 3)" )

#define M4NUDGWC_EXTRA_ROMS \
	ROM_REGION( 0x180000, "msm6376", ROMREGION_ERASE00 ) \
	ROM_LOAD( "naws1.bin", 0x0000, 0x02373f, CRC(b2ea8c50) SHA1(a02181f8f4636e69287073f4ffb8604ff2f14b9c) )
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4NUDGWC_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki ,mpu4 , mpu4_state,m4default_big ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4nudgwc,       0,          "naw02.bin",    0x0000, 0x010000, CRC(eb3ff27d) SHA1(ff0a80a75162380c6cc2d1b31f0bb0579faa1a2c), "Concept Games Ltd","Nudge-A-Win (Concept Games Ltd) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4nudgwc__a,    m4nudgwc,   "nawhc6.bin",   0x0000, 0x010000, CRC(f9389823) SHA1(e1db35200c9ed9d59cf817901cf75bdbb48507b2), "Concept Games Ltd","Nudge-A-Win (Concept Games Ltd) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4nudgwc__b,    m4nudgwc,   "nawhm5.bin",   0x0000, 0x010000, CRC(da365ac1) SHA1(4d8aa3541dcf94a550c815a0ade226a426b0c92d), "Concept Games Ltd","Nudge-A-Win (Concept Games Ltd) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4nudgwc__c,    m4nudgwc,   "nawsl10n.bin", 0x0000, 0x010000, CRC(6d5527b1) SHA1(52cdd413aaf5031dd3b8172bf49df59c3b33c9e7), "Concept Games Ltd","Nudge-A-Win (Concept Games Ltd) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4nudgwc__d,    m4nudgwc,   "nawsl10p.bin", 0x0000, 0x010000, CRC(cfdc953d) SHA1(919c5b52e9853b5896c573649257353e0b28536a), "Concept Games Ltd","Nudge-A-Win (Concept Games Ltd) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4nudgwc__e,    m4nudgwc,   "nawsl13n.bin", 0x0000, 0x010000, CRC(92ee524c) SHA1(88467af5d9e6db69969aaf9d8540828a1c058362), "Concept Games Ltd","Nudge-A-Win (Concept Games Ltd) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4nudgwc__f,    m4nudgwc,   "nawsl14n.bin", 0x0000, 0x010000, CRC(5217e17e) SHA1(449ff0c43bde5b4fecc7e5d31652648f7094e89d), "Concept Games Ltd","Nudge-A-Win (Concept Games Ltd) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4nudgwc__g,    m4nudgwc,   "nawsl7.bin",   0x0000, 0x010000, CRC(261192f6) SHA1(d754c0db8ee3986c33ea903c2efe86f14240afcf), "Concept Games Ltd","Nudge-A-Win (Concept Games Ltd) (MPU4) (set 8)" )
//Strange ROM, 0x20000 in length, but only 0x10000 worth of content, suspect overdump
GAME_CUSTOM( 199?, m4nudgwc__h,    m4nudgwc,   "naw0_4.bin",   0x0000, 0x020000, CRC(0201f6f9) SHA1(48772611db7ae0cda48b8d725fdc8ef50e64d6ad), "Concept Games Ltd","Nudge-A-Win (Concept Games Ltd) (MPU4) (set 9)" ) // rom too big, cut?
