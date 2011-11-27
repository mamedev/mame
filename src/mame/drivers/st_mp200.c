/*
    Stern MP-200 MPU
    (almost identical to Bally MPU-35)
*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m6800/m6800.h"

class st_mp200_state : public driver_device
{
public:
	st_mp200_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};


static ADDRESS_MAP_START( st_mp200_map, AS_PROGRAM, 8, st_mp200_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( st_mp200 )
INPUT_PORTS_END

void st_mp200_state::machine_reset()
{
}

static DRIVER_INIT( st_mp200 )
{
}

static MACHINE_CONFIG_START( st_mp200, st_mp200_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 1000000)
	MCFG_CPU_PROGRAM_MAP(st_mp200_map)
MACHINE_CONFIG_END

/*--------------------------------
/ Ali
/-------------------------------*/
ROM_START(ali)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(92e75b40) SHA1(bace68db0ea12d50a546157d11084f3b00949136))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(119a4300) SHA1(e913d9bd399b90502efe110c8bf7f23ae07df276))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(9c91d08f) SHA1(a3e8c8e8c2c8b03d86b36eea8c84e5c0a27b8444))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(7629db56) SHA1(f922d31ec4dd1755da0a24bec4e3fa3a7a9b22fc))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END


/*--------------------------------
/ Big Game
/-------------------------------*/
ROM_START(biggame)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(f59c7514) SHA1(49ab034a21e70956f63327aec4cbae115cd66a66))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(57df1dc5) SHA1(283f45879b76d56ba0db0fb3d9d9771f91a70d02))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(0251039b) SHA1(0a0e662788cf012dfb773d200c542a2a363748a8))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(801e9a66) SHA1(8634d6bd4af3e5ec3b736679393462961b76ede1))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Catacomb
/-------------------------------*/
ROM_START(catacomp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(d445dd40) SHA1(9ff5896977d7e2a0cf788c77dcfd7c010e17d2fb))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(d717a545) SHA1(a183f3b1f766c3a82ae52defc38d84328fb7b31a))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(bc504409) SHA1(cd3e948d34a8db71fc841261e683988c9df31ef8))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(da61b5a2) SHA1(ec4a914cd57b37921578699bc427f12a3670c7eb))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("snd_u9.716", 0x0000, 0x0800, CRC(a13cb591) SHA1(b64a2dc3429803095dc05cdd1718db2404b13eb8))
	ROM_LOAD("snd_u10.716",0x0800, 0x0800, CRC(2b31f8be) SHA1(05b394bd8b6c04e34fe2bab19cbd0f06d9e4b90d))
ROM_END

/*--------------------------------
/ Cheetah
/-------------------------------*/
ROM_START(cheetah)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(6a845d94) SHA1(c272d5895edf2270f5f06fc33345bb4911abbee4))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(e7bdbe6c) SHA1(8b213c2271dbd5157e0d34a33672130b935d76be))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(a827a1a1) SHA1(723ebf193b5ce7b19df70e83caa9bb80f2e3fa66))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(ed33c227) SHA1(a96ba2814cef7663728bb5fdea2dc6ecfa219038))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Cue (Proto - Never released)
/-------------------------------*/
ROM_START(cue)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, NO_DUMP)
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, NO_DUMP)
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, NO_DUMP)
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, NO_DUMP)
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Dragonfist
/-------------------------------*/
ROM_START(dragfist)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(4cbd1a38) SHA1(73b7291f38cd0a3300107605db26d474ecfc3101))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(1783269a) SHA1(75151b79844d26d9e8ecf00dec96643ee2fedc5b))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(9ac8292b) SHA1(99ad3ad6e1d1b19695ce1b5b76f6bd85c9c6530d))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(a374c8f9) SHA1(481116025a52353f298f3d93dfe33b3ad9f86d18))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Flight 2000
/-------------------------------*/
ROM_START(flight2k)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(df9efed9) SHA1(47727664e745e77ca1c221a32bd56d936f5b31bc))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(38c13649) SHA1(bcdbd17b48edd41ec7d38261595ac06eb8fc6a4d))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(425fae6a) SHA1(fde8d23e6ebb176ba72f763d66c2e17e51237fa1))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(dc243186) SHA1(046ce51b8a8218214088c4264548c753bd880e19))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("snd_u9.716", 0x0000, 0x0800, CRC(d816573c) SHA1(75134a017c34abbb149159ca001d35464a3f5128))
ROM_END

/*--------------------------------
/ Freefall
/-------------------------------*/
ROM_START(freefall)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(d13891ad) SHA1(afb40c51f2d5695c74ce9979c0a818845f95edd4))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(77bc7759) SHA1(3f739757180b3dcce5426935a51e4b615f157199))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(82bda054) SHA1(32772e878d2a4bba8f67e419a68a81fec2a5f6d7))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(68168b97) SHA1(defa4bba465182db22debddb4070c40c048c95e2))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("snd_u9.716", 0x0000, 0x0800, CRC(ea8cf062) SHA1(55c840a9bea363fd436c00a115cb61d15a9f8c47))
	ROM_LOAD("snd_u10.716",0x0800, 0x0800, CRC(dd681a79) SHA1(d954cae375fb0145e10536e43d1cb03902de2ea3))
ROM_END

/*--------------------------------
/ Galaxy
/-------------------------------*/
ROM_START(galaxypi)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(35656b67) SHA1(e1ad9456c561d19220f8607576cb505588512179))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(12be0601) SHA1(d651b834348c071dda660f37b4e359bf01cbd8d3))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(08bdb285) SHA1(7984835ac151e5dac05628f3d5146d20e3623c38))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(ad846a42) SHA1(303c9cb933ca60d35e12793a4ac0cf7ef11bc92e))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Hypnox
/-------------------------------*/

/*--------------------------------
/ Iron Maiden
/-------------------------------*/
ROM_START(ironmaid)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(e15371a4) SHA1(fe441ed8abd325190d8eee6d907e17c7fc02be64))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(84a29c01) SHA1(0e0ff8821c7028ce690328cd08a77bb51c0993c9))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(981ac0dd) SHA1(c585907b74695812f333867cf359a01a5ea6ed81))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(4e6f9c25) SHA1(9053e1d335a29f7acade7752adffe69f42032959))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*----------------------------------------
/ Lazer Lord (Proto - Never released)
/---------------------------------------*/
ROM_START(lazrlord)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(32a6f341) SHA1(75922c6831463d240fe057a0f72280d417899fa4))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(17583ba4) SHA1(4807e3ab18c2e40a292b499fe038975bb4b9fc17))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(669f3a8e) SHA1(4beb0e4c75f4e3c1788808b57081612d4774d130))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(395327a3) SHA1(e2a3a8ea696bcc4b5e11b08b6c7a6d9a991aa4af))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Lightning
/-------------------------------*/
ROM_START(lightnin)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(d3469d0a) SHA1(18565f5c85694da8eaf850146d3d9a90a17b7816))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(cd52262d) SHA1(099aeda2183822046cce907b265b42319007ac32))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(e0933419) SHA1(1f7cad915496f34473dffde7e320d51838acd0fd))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(df221c6b) SHA1(5935020d3a24d829fbeaa8cf764daff48a151a81))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("snd_u9.716", 0x0000, 0x0800, CRC(00ffa77c) SHA1(242efd800731a7f84369c6ce54298d0a227dd8ba))
	ROM_LOAD("snd_u10.716",0x0800, 0x0800, CRC(80fe9158) SHA1(20fcdb4c09b25e494f02bbfb20c07ff2870d5798))
ROM_END

/*--------------------------------
/ Meteor
/-------------------------------*/
ROM_START(meteorp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(e0fd8452) SHA1(a13215378a678e26a565742d81fdadd2e161ba7a))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(43a46997) SHA1(2c74ca10cf9091db10542960f499f39f3da277ee))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(fd396792) SHA1(b5d051a7ce7e7c2f9c4a0d900cef4f9ef2089476))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(03fa346c) SHA1(51c04123cb433e90920c241e2d1f89db4643427b))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Nine Ball
/-------------------------------*/
ROM_START(nineball)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(fcb58f97) SHA1(6510a6d0b466bd27ade50992260cea716d79fda2))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(c7c62161) SHA1(624eab2fdf7bafbf4af012df521bd09f9b2da8d8))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(bdd7f258) SHA1(2a38de09827100cbbd4e79be50aad03a3f2b63b4))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(7e831499) SHA1(8d3c148b91c21938b1b5fca85ecd8f6d7f1e76b0))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Orbitor 1
/-------------------------------*/
ROM_START(orbitor1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(575520e3) SHA1(9d52b065a14d4f95cebd48f60f628f2c246385fa))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(d31f27a8) SHA1(0442260db42192a95f6292e6b57000c127871d28))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(4421d827) SHA1(9b617215f2d92ef2c69104eb4e63a924704665aa))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(8861155a) SHA1(81a1b3434d4f80dee5704454f8359200faea173d))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("snd_u9.716", 0x0000, 0x0800, CRC(2ba24569) SHA1(da2f4a4eeed9ae7ff8a342f4d630e12dcb2decf5))
	ROM_LOAD("snd_u10.716",0x0800, 0x0800, CRC(8e5b4a38) SHA1(de3f59363553f5f0d6098401734436930e64fbbd))
ROM_END

/*--------------------------------
/ Quicksilver
/-------------------------------*/
ROM_START(quicksil)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(fc1bd20a) SHA1(e3c547f996dfc5d1567223d234443cf31d648ef6))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(0bcaceb4) SHA1(461d2fe5772a5ac84d31a4a186b9f639c683ca8a))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(8cb01165) SHA1(b42e2ccce2c20ad570cdcdb63c9d12e414f9b255))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(8c0e336a) SHA1(8d3a5b7c07d03c7e2945ea60c72f9181d3ee2a14))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Seawitch
/-------------------------------*/
ROM_START(seawitch)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(c214140b) SHA1(4d68ddd3b0f051c5f601ea5b9d5d5195d6017304))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(ab2eab3a) SHA1(80a8c1ccd554be279720a26466bd6c59e1e56df0))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(b8844174) SHA1(6e01321196fd6fce7b5526efc402044c87fe96a6))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(6c296d8f) SHA1(8cdb77f382ef1214ef45579213cf8f19141366ad))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Split Second
/-------------------------------*/
ROM_START(splitsec)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(c6ff9aa9) SHA1(39f80faca16c869ac14df7c5fc3dfa80b47dad95))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(fda74efc) SHA1(31becc243ada23e2f4d17927985772c9fcf8a3c3))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(81b9f784) SHA1(43cf71b51eda70a3c126340ea658c03c438e4f18))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(ecbedb0a) SHA1(8cc7281dd2bd300ab95a08761c12733d98599ebd))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("snd_u9.716", 0x0000, 0x0800, CRC(e6ed5f48) SHA1(ea2bbc607acb2b816667cd54f3d07605110c252e))
	ROM_LOAD("snd_u10.716",0x0800, 0x0800, CRC(36e6ee70) SHA1(61bd89d69627bea89b7f31af63ff90ace6db3c85))
ROM_END

/*--------------------------------
/ Stargazer
/-------------------------------*/
ROM_START(stargzr)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(83606fd4) SHA1(7f6448bc0dabe50de40fd47a7242c1be4a93e84d))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(c54ae389) SHA1(062e64e8ced723adb7f4040539ba6400fc4a9c9a))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(1a4c7dcb) SHA1(54888a8867b8d60f215b7e683ae4966f14ddca15))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(4e1f4dc6) SHA1(1f63a0b71af84fb6e1168ff77cbcbabcaa1323f3))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Viper
/-------------------------------*/
ROM_START(viperp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u1.716", 0x1000, 0x0800, CRC(d0ea0aeb) SHA1(28f4df9f45807abd1528aa6e5a80933156e6d692))
	ROM_LOAD( "cpu_u5.716", 0x1800, 0x0800, CRC(d26c7273) SHA1(303c18861941463932fdf47e9606159936b28dc1))
	ROM_LOAD( "cpu_u2.716", 0x5000, 0x0800, CRC(d03f1612) SHA1(d390ec1e953148ac26bf218701117855c941fc65))
	ROM_LOAD( "cpu_u6.716", 0x5800, 0x0800, CRC(96ff5f60) SHA1(a9df887ca338db208a684540f6c9fc07722c3aa5))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Gamatron (Pinstar game, 1985)
/-------------------------------*/
ROM_START(gamatron)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "gamatron.764", 0x1000, 0x0800, CRC(fa9f7676) SHA1(8c56868eb6af7bb8ad73523ab6583100fcadc3c1))
	ROM_CONTINUE( 0x5000, 0x0800)
	ROM_CONTINUE( 0x1800, 0x0800)
	ROM_CONTINUE( 0x5800, 0x0800)
	ROM_RELOAD( 0xe000, 0x2000)
ROM_END

/*----------------------------------
/ Black Sheep Squadron (Astro game)
/---------------------------------*/
ROM_START(blkshpsq)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, CRC(23d6cd54) SHA1(301ba10f3f333109630dd8abd13a6b4063f805a9))
	ROM_RELOAD( 0x5000, 0x0800)
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, CRC(ea68b9f7) SHA1(ebb69f4faadf457454939e47d8ae6e79eb0e1a11))
	ROM_RELOAD( 0x5800, 0x0800)
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*----------------------------------
/ Unknown game and manufacturer
/---------------------------------*/
ROM_START(st_game)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.716", 0x1000, 0x0800, CRC(b9ac5204) SHA1(1ac4e336eb62c091e61e9b6b21a858e70ac9ab38))
	ROM_RELOAD( 0x5000, 0x0800)
	ROM_LOAD( "cpu_u6.716", 0x1800, 0x0800, CRC(e16fbde1) SHA1(f7fe2f2ef9251792af1227f82dcc95239dd8baa1))
	ROM_RELOAD( 0x5800, 0x0800)
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END


//GAME(1982,    cue,        0,      st_mp200,   st_mp200,   st_mp200,   ROT0,   "Stern",                "Cue",              GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1980,	ali,		0,			st_mp200,	st_mp200,	st_mp200,	ROT0,	"Stern",				"Ali",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1980,	biggame,	0,			st_mp200,	st_mp200,	st_mp200,	ROT0,	"Stern",				"Big Game",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1981,	catacomp,	0,			st_mp200,	st_mp200,	st_mp200,	ROT0,	"Stern",				"Catacomb (Pinball)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1980,	cheetah,	0,			st_mp200,	st_mp200,	st_mp200,	ROT0,	"Stern",				"Cheetah",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1982,	dragfist,	0,			st_mp200,	st_mp200,	st_mp200,	ROT0,	"Stern",				"Dragonfist",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1980,	flight2k,	0,			st_mp200,	st_mp200,	st_mp200,	ROT0,	"Stern",				"Flight 2000",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1981,	freefall,	0,			st_mp200,	st_mp200,	st_mp200,	ROT0,	"Stern",				"Freefall",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1980,	galaxypi,	0,			st_mp200,	st_mp200,	st_mp200,	ROT0,	"Stern",				"Galaxy",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1981,	ironmaid,	0,			st_mp200,	st_mp200,	st_mp200,	ROT0,	"Stern",				"Iron Maiden",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1984,	lazrlord,	0,			st_mp200,	st_mp200,	st_mp200,	ROT0,	"Stern",				"Lazer Lord",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1981,	lightnin,	0,			st_mp200,	st_mp200,	st_mp200,	ROT0,	"Stern",				"Lightning",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1979,	meteorp,	0,			st_mp200,	st_mp200,	st_mp200,	ROT0,	"Stern",				"Meteor (Stern)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1980,	nineball,	0,			st_mp200,	st_mp200,	st_mp200,	ROT0,	"Stern",				"Nine Ball",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1982,	orbitor1,	0,			st_mp200,	st_mp200,	st_mp200,	ROT0,	"Stern",				"Orbitor 1",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1980,	quicksil,	0,			st_mp200,	st_mp200,	st_mp200,	ROT0,	"Stern",				"Quicksilver",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1980,	seawitch,	0,			st_mp200,	st_mp200,	st_mp200,	ROT0,	"Stern",				"Seawitch",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1981,	splitsec,	0,			st_mp200,	st_mp200,	st_mp200,	ROT0,	"Stern",				"Split Second",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1980,	stargzr,	0,			st_mp200,	st_mp200,	st_mp200,	ROT0,	"Stern",				"Stargazer",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1981,	viperp,		0,			st_mp200,	st_mp200,	st_mp200,	ROT0,	"Stern",				"Viper (Pinball)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1985,	gamatron,	flight2k,	st_mp200,	st_mp200,	st_mp200,	ROT0,	"Pinstar",				"Gamatron",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1978,	blkshpsq,	0,			st_mp200,	st_mp200,	st_mp200,	ROT0,	"Astro",				"Black Sheep Squadron",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(198?,	st_game,	0,			st_mp200,	st_mp200,	st_mp200,	ROT0,	"<unknown>",			"unknown pinball game",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
