/*
    Gottlieb System 3
*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m6502/m65ce02.h"

class gts3_state : public driver_device
{
public:
	gts3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};


static ADDRESS_MAP_START( gts3_map, AS_PROGRAM, 8, gts3_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( gts3 )
INPUT_PORTS_END

void gts3_state::machine_reset()
{
}

static DRIVER_INIT( gts3 )
{
}

static MACHINE_CONFIG_START( gts3, gts3_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M65C02, 2000000)
	MCFG_CPU_PROGRAM_MAP(gts3_map)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Amazon Hunt III
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Barb Wire
/-------------------------------------------------------------------*/
ROM_START(barbwire)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(2e130835) SHA1(f615eaf1c48851d837c57c17c038cc1d0806f6f7))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(2b9533cd) SHA1(2b154550006e37a9dd1acb0cb832535415a7266b))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(ebde41b0) SHA1(38a132f815a5270dff58a5e34f5c73701d6e214d))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(7171bc86) SHA1(d9b1f54d34400490c219ca3ba566cc40cac517d7))
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(ce83c6c3) SHA1(95a364844525548d28f78d54f9d058728cebf089))
	ROM_RELOAD(0x80000+0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(7c602a35) SHA1(66dbd7679973683c8346836c28c02ff922d17375))
ROM_END

/*-------------------------------------------------------------------
/ Bell Ringer
/-------------------------------------------------------------------*/
ROM_START(bellring)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("br_gprom.bin", 0x0000, 0x10000, CRC(a9a59b36) SHA1(ca6d0e54a5c85ef72485975c632660831a3b8c82))
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("br_drom1.bin", 0x8000, 0x8000, CRC(99f38229) SHA1(f63d743e63e88728e8d53320b21b2fda1b6385f8))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("br_yrom1.bin", 0x8000, 0x8000, CRC(d5aab379) SHA1(b3995f8aa2e54f91f2a0fd010c807fbfbf9ae847))
ROM_END

/*-------------------------------------------------------------------
/ Brooks & Dunn (#749)
/-------------------------------------------------------------------*/
ROM_START(brooks)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(26cebf07) SHA1(14741e2d216528f176dc35ade856baffab0f99a0))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, NO_DUMP)
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, NO_DUMP)
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, NO_DUMP)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, NO_DUMP)
	ROM_RELOAD(0x80000+0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, NO_DUMP)
ROM_END

/*-------------------------------------------------------------------
/ Cactus Jack's (#729)
/-------------------------------------------------------------------*/
ROM_START(cactjack)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(5661ab06) SHA1(12b7066110feab0aef36ff7bdc74690fc8da4ed3))
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(78c099e1) SHA1(953111237fdc3e20562d823eb2b6430e5a4afe4d))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x20000, CRC(c890475f) SHA1(1cf6ed0dbd003a76a5cf889f62b489c0a62e9d25))
	ROM_RELOAD(0x00000+0x40000, 0x20000)
	ROM_RELOAD(0x00000+0x80000, 0x20000)
	ROM_RELOAD(0x00000+0xc0000, 0x20000)
	ROM_LOAD("arom2.bin", 0x20000, 0x20000, CRC(aba8fd98) SHA1(81b8af4d2d8e40b5b44f114c095371afe5539549))
	ROM_RELOAD(0x20000+0x40000, 0x20000)
	ROM_RELOAD(0x20000+0x80000, 0x20000)
	ROM_RELOAD(0x20000+0xc0000, 0x20000)
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(4554ed0d) SHA1(df0a9225f961e0ee876c3e63ad54c6e4eac080ae))
ROM_END

/*-------------------------------------------------------------------
/ Car Hop (#725)
/-------------------------------------------------------------------*/
ROM_START(carhop)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(164b2c9c) SHA1(49cf7e3a3acb5de8dbfd2ad22f8bd9a352ff2899))
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(9dec74e7) SHA1(8234bdca5536d30dc1eabcb3a5505d2fd824ce0f))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(831ee812) SHA1(57056cde36b17cb7d7f34275b1bb5dc3d52bde4e))
ROM_END

/*-------------------------------------------------------------------
/ Caribbean Cruise (#C102)
/-------------------------------------------------------------------*/
ROM_START(ccruise)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(668b5757) SHA1(8ff955e8598ffdc68eab7fd69c6a67c4eed13f0f))
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000,  0x8000, CRC(4480257e) SHA1(50b93d4496816ef7cdf007ac75c72c6aaa956aba))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(f8cec60c) SHA1(e52f3a5890a3bb5eb6c932c3d0ed471ed76909c9))
	ROM_RELOAD(0x40000,  0x40000)
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(6e424e53) SHA1(90a9bf5ce84680972f9d12eb386215494c584b9b))
ROM_END

/*-------------------------------------------------------------------
/ Class of 1812 (#730)
/-------------------------------------------------------------------*/
ROM_START(clas1812)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(564349bf) SHA1(458eb2ece924a20d309dce7117c94e75b4a21fd7))
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(3863a9df) SHA1(1759abbfcb127a6909f70845f41daf3ac8e80cef))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x20000, CRC(357b0069) SHA1(870b0b84c6b3754f89b4e4e0b4594613ef589204))
	ROM_RELOAD(0x00000+0x40000, 0x20000)
	ROM_RELOAD(0x00000+0x80000, 0x20000)
	ROM_RELOAD(0x00000+0xc0000, 0x20000)
	ROM_LOAD("arom2.bin", 0x20000, 0x20000, CRC(5be02ff7) SHA1(51af73a26bbed0915ec57cde8f9cac552978b2dc))
	ROM_RELOAD(0x20000+0x40000, 0x20000)
	ROM_RELOAD(0x20000+0x80000, 0x20000)
	ROM_RELOAD(0x20000+0xc0000, 0x20000)
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(4ecf6ecb) SHA1(92469ccdedcc8e61edcddaedd688ef990a9ad5ad))
ROM_END

/*-------------------------------------------------------------------
/ Cue Ball Wizard
/-------------------------------------------------------------------*/
ROM_START(cueball)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(3437fdd8) SHA1(2a0fc9bc8e3d0c430ce2cf8afad378fc93af609d))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom.bin", 0x00000, 0x40000, CRC(3cc7f470) SHA1(6adf8ac2ff93eb19c7b1dbbcf8fff6cd926dc563))
	ROM_RELOAD( 0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(9fd04109) SHA1(27864fe4e9c248dce6221c9e56861967d089b216))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(476bb11c) SHA1(ce546df59933cc230a6671dec493bbbe71146dee))
	ROM_RELOAD(0x00000+0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(23708ad9) SHA1(156fcb19403f9845404af1a4ac4edfd3fcde601d))
	ROM_RELOAD(0x80000+0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(c22f5cc5) SHA1(a5bfbc1824bc483eecc961851bd411cb0dbcdc4a))
ROM_END

/*-------------------------------------------------------------------
/ Deadly Weapon (#724)
/-------------------------------------------------------------------*/
ROM_START(deadweap)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(07d84b32) SHA1(25d8772a5c8655b3406df94563076719b07129cd))
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(f55dd7ec) SHA1(fe306c40bf3d98e4076d0d8a935c3671469d4cff))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(93369ed3) SHA1(3340478ffc00cf9991beabd4f0ecd89d0c88965e))
ROM_END

/*-------------------------------------------------------------------
/ Frank Thomas' Big Hurt
/-------------------------------------------------------------------*/
ROM_START(bighurt)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(92ce9353) SHA1(479edb2e39fa610eb2854b028d3a039473e52eba))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(bbe96c5e) SHA1(4aaac8d88e739ccb22a7d87a820b14b6d40d3ff8))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(d472210c) SHA1(4607e6f928cb9a5f41175210ba0427b6cd50fb83))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(b3def376) SHA1(94553052cfe80774affebd5b0f99512055552786))
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(59789e66) SHA1(08b7f82f83c53f15cafefb009ab9833457c088cc))
	ROM_RELOAD(0x80000+0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(c58941ed) SHA1(3b3545b1e8986b06238576a0cef69d3e3a59a325))
ROM_END

/*-------------------------------------------------------------------
/ Freddy: A Nightmare on Elm Street
/-------------------------------------------------------------------*/
ROM_START(freddy)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(f0a6f3e6) SHA1(ad9af12260b8adc639fa00de49366b1016df49ed))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(d78d0fa3) SHA1(132c05e71cf5ad53184f044873fb3dd71f6da15f))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(d472210c) SHA1(4607e6f928cb9a5f41175210ba0427b6cd50fb83))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(6bec0567) SHA1(510c0e5a5af7573761a69bad5ab36f0019767c48))
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(f0e9284d) SHA1(6ffe8286e27b0eecab9620ca613e3d72bb7f77ce))
	ROM_RELOAD(0x80000+0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(4a748665) SHA1(9f08b6d0731390c306194808226d2e99fbe9122d))
ROM_END

ROM_START(freddy4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom4.bin", 0x0000, 0x10000, CRC(cd8b46ea) SHA1(3151a9f7b514314dc4989232e1eda444555242c0))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(d78d0fa3) SHA1(132c05e71cf5ad53184f044873fb3dd71f6da15f))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(d472210c) SHA1(4607e6f928cb9a5f41175210ba0427b6cd50fb83))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(6bec0567) SHA1(510c0e5a5af7573761a69bad5ab36f0019767c48))
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(f0e9284d) SHA1(6ffe8286e27b0eecab9620ca613e3d72bb7f77ce))
	ROM_RELOAD(0x80000 +0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(4a748665) SHA1(9f08b6d0731390c306194808226d2e99fbe9122d))
ROM_END
/*-------------------------------------------------------------------
/ Gladiators
/-------------------------------------------------------------------*/
ROM_START(gladiatp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(40386cf5) SHA1(3139e3707971a708ad98c735deec7e4ee7bb36cd))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(fdc8baed) SHA1(d8ad96665cd9d8b2a6ce94653753c692384685ff))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(60779d60) SHA1(2fa09c65ddd6cf638382229062a48163e8972136))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(85cbdda7) SHA1(4eaea8866cb281034e30f425e864419fdb58081f))
	ROM_RELOAD(0x00000+0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(da2c1073) SHA1(faf58099e78dffdce5c15f393ffa3707ec80dd51))
	ROM_RELOAD(0x80000+0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(c5b72153) SHA1(c5d94f3fa815fc33952107c3a3ad698c3c443ce3))
ROM_END

/*-------------------------------------------------------------------
/ Hoops (#727)
/-------------------------------------------------------------------*/
ROM_START(hoops)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(78391273) SHA1(dbf91597ce2910e526fb5e82355ad862706b4975))
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(e72c00eb) SHA1(5b9f85083b38d916afb0f9b72b061501504725ff))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(9718b958) SHA1(bac806267bab4852c0f3fdb48f8d872992f61ace))
ROM_END

/*-------------------------------------------------------------------
/ Lights, Camera, Action (#720)
/-------------------------------------------------------------------*/
ROM_START(lca)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x8000, 0x8000, CRC(52957d70) SHA1(0c24d824b1aa966eb3af3db3ff02870ba463dcd6))
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(a258d72d) SHA1(eeb4768c8b2f57509a088d3ac8d35aa34f2cfc2c))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(20919ebb) SHA1(a2ea79863b41a04aa23ea596932825408cca64e3))
ROM_END

ROM_START(lca2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom2.bin", 0x8000, 0x8000, CRC(937a8426) SHA1(6bc2d1b0c3dc273577376654ba72b60febe32529))
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(a258d72d) SHA1(eeb4768c8b2f57509a088d3ac8d35aa34f2cfc2c))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(20919ebb) SHA1(a2ea79863b41a04aa23ea596932825408cca64e3))
ROM_END

/*-------------------------------------------------------------------
/ Mario Andretti
/-------------------------------------------------------------------*/
ROM_START(andretti)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(cffa788d) SHA1(84646880b09dce73a42a6d87666897f6bd74a8f9))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(1f70baae) SHA1(cf07bb057093b2bd18e6ee45009245ea62094e53))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(d472210c) SHA1(4607e6f928cb9a5f41175210ba0427b6cd50fb83))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(918c3270) SHA1(aa57d3bfba01e701b02ca7e4f0946144cfb7d4b1))
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(3c61a2f7) SHA1(65cfb5d1261a1b0c219e1786b6635d7b0a188040))
	ROM_RELOAD(0x80000+0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(4ffb15b0) SHA1(de4e9b2ccca865deb2595320015a149246795260))
ROM_END

ROM_START(andretti4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gpromt4.bin", 0x0000, 0x10000, CRC(c6f6a23b) SHA1(01ea23a830be1e86f5ecd27d6d56c1c6d5ff3176))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(1f70baae) SHA1(cf07bb057093b2bd18e6ee45009245ea62094e53))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(d472210c) SHA1(4607e6f928cb9a5f41175210ba0427b6cd50fb83))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(918c3270) SHA1(aa57d3bfba01e701b02ca7e4f0946144cfb7d4b1))
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(3c61a2f7) SHA1(65cfb5d1261a1b0c219e1786b6635d7b0a188040))
	ROM_RELOAD(0x80000+0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(4ffb15b0) SHA1(de4e9b2ccca865deb2595320015a149246795260))
ROM_END

/*-------------------------------------------------------------------
/ Nudge-It (N102)
/-------------------------------------------------------------------*/
ROM_START(nudgeit)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(3d9e0309) SHA1(caaa28482e7f260668aa05b39b551acb8e4cc41a))
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(ae0c4b1d) SHA1(c8aa409c9b54fd8ecf70eb2926f4e98fc5eb11fe))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(65fc2e60) SHA1(6377c220753d9e4b5c76d445056409526d95772f))
ROM_END

/*-------------------------------------------------------------------
/ Operation Thunder (#732)
/-------------------------------------------------------------------*/
ROM_START(opthund)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(96a128c2) SHA1(4032c5191b167a0498371207666a1f73155b7a74))
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(db28be69) SHA1(6c505c34c8bdccc43dd8f310f01dd3a6b49e8059))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(0fbb130a) SHA1(a171c20f861dac5918c5b410e2a2bdd6e7c0553b))
	ROM_RELOAD(0x00000+0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(0f7632b3) SHA1(a122a062448139d5c1a9daa7d827c3073aa194f7))
	ROM_RELOAD(0x80000+0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(169816d1) SHA1(d23b1d8d1b841ca065a485e80805ecc6342ce57b))
ROM_END

/*-------------------------------------------------------------------
/ Rescue 911
/-------------------------------------------------------------------*/
ROM_START(rescu911)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(943a7597) SHA1(dcf4151727efa64e8740202b68fc8e76098ff8dd))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(9657ebd5) SHA1(b716daa71f8ec4332bf338f1f976425b6ec781ab))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(034c6bc3) SHA1(c483690a6e4ce533b8939e27547175c301316172))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(f6daa16c) SHA1(be132072b27a94f61653de0a22eecc8b90db3077))
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(59374104) SHA1(8ad7f5f0109771dd5cebe13e80f8e1a9420f4447))
	ROM_RELOAD(0x80000+0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(14f86b56) SHA1(2364c284412eba719f88d50dcf47d5482365dbf3))
ROM_END

/*-------------------------------------------------------------------
/ Shaq Attaq
/-------------------------------------------------------------------*/
ROM_START(shaqattq)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(7a967fd1) SHA1(c06e2aad9452150d92cfd3ba37b8e4a932cf4324))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(d6cca842) SHA1(0498ab558d252e42dee9636e6736d159c7d06275))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(16a03261) SHA1(25f5a3d32d2ec80766381106445fd624360fea78))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(019014ec) SHA1(808a8c3154fca6218fe991b46a2525926d8e51f9))
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(cc5f157d) SHA1(81c3dadff1bbf37a1f091ea77d9061879be7d99c))
	ROM_RELOAD(0x80000+0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(e81e2928) SHA1(4bfe57efa99bb762e4de6c7e88e79b8c5ff57626))
ROM_END

ROM_START(shaqattq2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom2.bin", 0x0000, 0x10000, CRC(494b5cec) SHA1(91511eb9f8b0182ffeff5301fb5bcf4ee9056b3f))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(d6cca842) SHA1(0498ab558d252e42dee9636e6736d159c7d06275))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(16a03261) SHA1(25f5a3d32d2ec80766381106445fd624360fea78))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(019014ec) SHA1(808a8c3154fca6218fe991b46a2525926d8e51f9))
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(cc5f157d) SHA1(81c3dadff1bbf37a1f091ea77d9061879be7d99c))
	ROM_RELOAD(0x80000+0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(e81e2928) SHA1(4bfe57efa99bb762e4de6c7e88e79b8c5ff57626))
ROM_END

/*-------------------------------------------------------------------
/ Silver Slugger (#722)
/-------------------------------------------------------------------*/
ROM_START(silvslug)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(a6c524e2) SHA1(dc12dd8e814a37aada021f84c58475efe72cb846))
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(eac3e1cc) SHA1(2725457231854e4f3d54fbba745b8fc6f55b1688))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(20bc9797) SHA1(5d17b5f0423d8854fb7c4816d53a223ecc7c50c6))
ROM_END

/*-------------------------------------------------------------------
/ Stargate
/-------------------------------------------------------------------*/
ROM_START(stargatp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(837e4354) SHA1(b7d1e270309b3d7965dafeec7b81d2dd41e5700c))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(17b89750) SHA1(927702f88013945cb9f2ea8389800b925182c347))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(781b2b27) SHA1(06decd22b9064ee4859618a043055e0b3e3b9e04))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(a0f62605) SHA1(8c39452367150f66271371ab02be2f5a812cb954))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(53123fd4) SHA1(77fd183a10eea2e04a07edf9da14ef7aadb65f91))
ROM_END

ROM_START(stargatp1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom1.bin", 0x0000, 0x10000, CRC(567ecd88) SHA1(2dc4bfbc971cc873af6ec32e5ddbbed001d2e1d2))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom1.bin", 0x00000, 0x80000, CRC(91c1b01a) SHA1(96eec2e9e52c8278c102f433a554327d420fe131))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(781b2b27) SHA1(06decd22b9064ee4859618a043055e0b3e3b9e04))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(a0f62605) SHA1(8c39452367150f66271371ab02be2f5a812cb954))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(53123fd4) SHA1(77fd183a10eea2e04a07edf9da14ef7aadb65f91))
ROM_END

ROM_START(stargatp2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom2.bin", 0x0000, 0x10000, CRC(862920f8) SHA1(cde77e7937782f2f9fe4b7fe27b56206d6f26f63))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom2.bin", 0x00000, 0x80000, CRC(d0205e03) SHA1(d8dea47f0fa0e46e2bd107a1f57121372fdef0d8))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(781b2b27) SHA1(06decd22b9064ee4859618a043055e0b3e3b9e04))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(a0f62605) SHA1(8c39452367150f66271371ab02be2f5a812cb954))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(53123fd4) SHA1(77fd183a10eea2e04a07edf9da14ef7aadb65f91))
ROM_END

ROM_START(stargatp3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom3.bin", 0x0000, 0x10000, CRC(83f0a2e7) SHA1(5d247a3329a946449e4b333b18c13e351caa230b))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom3.bin", 0x00000, 0x80000, CRC(db483524) SHA1(ea14e8b04c32fc403ce2ff060caed5562104a862))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(781b2b27) SHA1(06decd22b9064ee4859618a043055e0b3e3b9e04))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(a0f62605) SHA1(8c39452367150f66271371ab02be2f5a812cb954))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(53123fd4) SHA1(77fd183a10eea2e04a07edf9da14ef7aadb65f91))
ROM_END

ROM_START(stargatp4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom4.bin", 0x0000, 0x10000, CRC(7b8f6920) SHA1(f354593e13c30e15c25580387ef2eb9b23622c89))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom3.bin", 0x00000, 0x80000, CRC(db483524) SHA1(ea14e8b04c32fc403ce2ff060caed5562104a862))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(781b2b27) SHA1(06decd22b9064ee4859618a043055e0b3e3b9e04))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(a0f62605) SHA1(8c39452367150f66271371ab02be2f5a812cb954))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(53123fd4) SHA1(77fd183a10eea2e04a07edf9da14ef7aadb65f91))
ROM_END

/*-------------------------------------------------------------------
/ Street Fighter 2
/-------------------------------------------------------------------*/
ROM_START(sfight2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(299ad173) SHA1(95cca8c22cfabc55175a49b0439fc7858bdec1bd))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(e565e5e9) SHA1(c37abf28918feb38bbad6ebb610023d52ba96957))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(f5c13e80) SHA1(4dd3d35c25e3cb92d6000e463ddce564e112c108))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(8518ff55) SHA1(b31678aa7c1b1240becf0ae0af05b30f7df4a491))
	ROM_RELOAD(0x00000+0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(85a304d9) SHA1(71141dea44e4117cad66089c7a0806de1be1a96a))
	ROM_RELOAD(0x80000+0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(9009f461) SHA1(589d94a9ae2269175be9f71b1946107bb85620ee))
ROM_END

ROM_START(sfight2a)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom1.bin", 0x0000, 0x10000, CRC(5b42c332) SHA1(958e9fe09e587038dc282fc2f276608ef3744b1d))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom2.bin", 0x00000, 0x80000, CRC(80eb7513) SHA1(d13d44545c7b177e27b596bac6eba173b34a017b))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(f5c13e80) SHA1(4dd3d35c25e3cb92d6000e463ddce564e112c108))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(8518ff55) SHA1(b31678aa7c1b1240becf0ae0af05b30f7df4a491))
	ROM_RELOAD(0x00000+0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(85a304d9) SHA1(71141dea44e4117cad66089c7a0806de1be1a96a))
	ROM_RELOAD(0x80000+0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(9009f461) SHA1(589d94a9ae2269175be9f71b1946107bb85620ee))
ROM_END

ROM_START(sfight2b)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom2.bin", 0x0000, 0x10000, CRC(26d24c06) SHA1(c706bd6b2bd5b9ad6a6fb69178169977a54107b5))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom2.bin", 0x00000, 0x80000, CRC(80eb7513) SHA1(d13d44545c7b177e27b596bac6eba173b34a017b))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(f5c13e80) SHA1(4dd3d35c25e3cb92d6000e463ddce564e112c108))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(8518ff55) SHA1(b31678aa7c1b1240becf0ae0af05b30f7df4a491))
	ROM_RELOAD(0x00000+0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(85a304d9) SHA1(71141dea44e4117cad66089c7a0806de1be1a96a))
	ROM_RELOAD(0x80000+0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(9009f461) SHA1(589d94a9ae2269175be9f71b1946107bb85620ee))
ROM_END

/*-------------------------------------------------------------------
/ Strikes 'N' Spares
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Super Mario Brothers (#733) - Only one dsprom dump seems to work?
/-------------------------------------------------------------------*/
ROM_START(smb)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(fa1f6e52) SHA1(d7ade0e129cb399494967e025d25614bf1650db7))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom2.bin", 0x00000, 0x40000, CRC(181e8234) SHA1(9b22681f61cae401269a88c3cfd783d683390877))
	ROM_RELOAD( 0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(6f1d0a3e) SHA1(c7f665d79b9073f28f90debde16cafa9ab57a47c))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(e9cef116) SHA1(5f710bc24e1a168f296a22417aebecbde3bfaa5c))
	ROM_RELOAD(0x00000+0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(0acdfd49) SHA1(0baabd32b546842bc5c76a61b509b558677b50f9))
	ROM_RELOAD(0x80000+0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(e1379106) SHA1(10c46bad7cbae528716c5ba0709bb1fd3574a0a8))
ROM_END

ROM_START(smb1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom1.bin", 0x0000, 0x10000, CRC(1d8c4df8) SHA1(e301bf3b2a8ed6ef902fe15b890b4c06c4606aa9))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom2.bin", 0x00000, 0x40000, CRC(181e8234) SHA1(9b22681f61cae401269a88c3cfd783d683390877))
	ROM_RELOAD( 0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(6f1d0a3e) SHA1(c7f665d79b9073f28f90debde16cafa9ab57a47c))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(e9cef116) SHA1(5f710bc24e1a168f296a22417aebecbde3bfaa5c))
	ROM_RELOAD(0x00000+0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(0acdfd49) SHA1(0baabd32b546842bc5c76a61b509b558677b50f9))
	ROM_RELOAD(0x80000+0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(e1379106) SHA1(10c46bad7cbae528716c5ba0709bb1fd3574a0a8))
ROM_END

ROM_START(smb2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom2.bin", 0x0000, 0x10000, CRC(5b0f44c4) SHA1(ca9b0cd82c75612c85c956497c8f9c12992f6ad5))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom2.bin", 0x00000, 0x40000, CRC(181e8234) SHA1(9b22681f61cae401269a88c3cfd783d683390877))
	ROM_RELOAD( 0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(6f1d0a3e) SHA1(c7f665d79b9073f28f90debde16cafa9ab57a47c))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(e9cef116) SHA1(5f710bc24e1a168f296a22417aebecbde3bfaa5c))
	ROM_RELOAD(0x00000+0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(0acdfd49) SHA1(0baabd32b546842bc5c76a61b509b558677b50f9))
	ROM_RELOAD(0x80000+0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(e1379106) SHA1(10c46bad7cbae528716c5ba0709bb1fd3574a0a8))
ROM_END

ROM_START(smb3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom3.bin", 0x0000, 0x10000, CRC(5a40822c) SHA1(a87ec6307f848483c76141e47fd67e4549f9c9d3))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom2.bin", 0x00000, 0x40000, CRC(181e8234) SHA1(9b22681f61cae401269a88c3cfd783d683390877))
	ROM_RELOAD( 0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(6f1d0a3e) SHA1(c7f665d79b9073f28f90debde16cafa9ab57a47c))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(e9cef116) SHA1(5f710bc24e1a168f296a22417aebecbde3bfaa5c))
	ROM_RELOAD(0x00000+0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(0acdfd49) SHA1(0baabd32b546842bc5c76a61b509b558677b50f9))
	ROM_RELOAD(0x80000+0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(e1379106) SHA1(10c46bad7cbae528716c5ba0709bb1fd3574a0a8))
ROM_END

/*-------------------------------------------------------------------
/ Super Mario Brothers Mushroom World
/-------------------------------------------------------------------*/
ROM_START(smbmush)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(45f6d0cc) SHA1(a73c71ab64aee293ae46e65c34d70840296778d4))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom.bin", 0x00000, 0x40000, CRC(dda6c8be) SHA1(b64f73b81afe973674f9543a704b498e31d26c12))
	ROM_RELOAD( 0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(6f04a0ac) SHA1(53bbc182a3bd635ad18504692a4454994daef7ef))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(edce7951) SHA1(4a80d6367a5bebf9fee181456280619aa64b441f))
	ROM_RELOAD(0x00000+0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(dd7ea212) SHA1(adaf0262e315c26b1f4d6365e9d465c7afb6984d))
	ROM_RELOAD(0x80000+0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(09712c37) SHA1(e2ee902ea6eac3e6257880949bd07a90de08e7b9))
ROM_END

/*-------------------------------------------------------------------
/ Surf'n Safari (#731)
/-------------------------------------------------------------------*/
ROM_START(surfnsaf)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(ac3393bd) SHA1(f9c533b937b5ca5698b805ed6ed573cb22383d9d))
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(ec8fc963) SHA1(247e76d87beb3339e7d55292f9eadd2351621cfa))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(38b569b2) SHA1(93be47916a92541d097233b60a42eb7ca587ce52))
	ROM_RELOAD(0x00000+0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(224c2021) SHA1(6b426097a2870b3b32d786be6e66ba6be9f54c29))
	ROM_RELOAD(0x80000+0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(a0480418) SHA1(a982564d5dbf52275c2e7223687b07cf4ca0a115))
ROM_END

/*-------------------------------------------------------------------
/ Tee'd Off
/-------------------------------------------------------------------*/
ROM_START(teedoffp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(0620365b) SHA1(18887c49a5d3806b725fa6289e50db82974c0f40))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(340b8a49) SHA1(3ac76faf920b00b77c77023c42595307840ed3a7))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(3868e77a) SHA1(2db91c527803a369ca659eaae6022667a126d2ef))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(9e442b71) SHA1(889023af42a2527a51343ccee7f66b089b6e6d01))
	ROM_RELOAD(0x00000+0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(3dad9508) SHA1(70ed49fa82dbe7586bfca72c5020834f9173d563))
	ROM_RELOAD(0x80000+0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(c51d98d8) SHA1(9387a39a03ca90bc8eaddc0c2df8874067a22dea))
ROM_END

ROM_START(teedoffp1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom1.bin", 0x0000, 0x10000, CRC(95760ab1) SHA1(9342128e2de4e81c4b0cfc482bb0650434a04bee))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom1.bin", 0x00000, 0x80000, CRC(24f10ad2) SHA1(15f44f69d39ca9782410a75070edf348f64dba62))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(3868e77a) SHA1(2db91c527803a369ca659eaae6022667a126d2ef))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(9e442b71) SHA1(889023af42a2527a51343ccee7f66b089b6e6d01))
	ROM_RELOAD(0x00000+0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(3dad9508) SHA1(70ed49fa82dbe7586bfca72c5020834f9173d563))
	ROM_RELOAD(0x80000+0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(c51d98d8) SHA1(9387a39a03ca90bc8eaddc0c2df8874067a22dea))
ROM_END

ROM_START(teedoffp3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom3.bin", 0x0000, 0x10000, CRC(d7008579) SHA1(b7bc9f54340ffb2d684b5df80624e8c01e7fa18b))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom1.bin", 0x00000, 0x80000, CRC(24f10ad2) SHA1(15f44f69d39ca9782410a75070edf348f64dba62))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(3868e77a) SHA1(2db91c527803a369ca659eaae6022667a126d2ef))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(9e442b71) SHA1(889023af42a2527a51343ccee7f66b089b6e6d01))
	ROM_RELOAD(0x00000+0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(3dad9508) SHA1(70ed49fa82dbe7586bfca72c5020834f9173d563))
	ROM_RELOAD(0x80000+0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(c51d98d8) SHA1(9387a39a03ca90bc8eaddc0c2df8874067a22dea))
ROM_END

/*-------------------------------------------------------------------
/ Title Fight (#726)
/-------------------------------------------------------------------*/
ROM_START(tfight)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(43b3193a) SHA1(bd185fe67c147a6acca8e78da4b77c384124fc46))
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(9514739f) SHA1(2794549f549d68e064a9a962a4e91fff7dcf0160))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(8591d421) SHA1(74402cf8b419e0cb05069851b0d5616e66b2f0a9))
ROM_END

/*-------------------------------------------------------------------
/ Vegas (#723)
/-------------------------------------------------------------------*/
ROM_START(vegas)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(48189981) SHA1(95144af4b222158becd4d5748d15b7b6c6021bd2))
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(46eb5755) SHA1(94ec2d0cf41f68a8c3d7505186b11b4abb4803db))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(af1095f1) SHA1(06609085cd74b969e4f2ec962c427c5c42ebc6ff))
ROM_END

/*-------------------------------------------------------------------
/ Waterworld
/-------------------------------------------------------------------*/
ROM_START(waterwld)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(db1fd197) SHA1(caa22f7e3f52be85da496375115933722a414ee0))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(79164099) SHA1(fa048fb7aa91cadd6c0758c570a4c74337bd7cd5))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(2a8c5d04) SHA1(1a6a698fc05a199923721e91e68aaaa8d3c6a3c2))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(3ee37668) SHA1(9ced05b4f060568bf686974bc2472ff7c05a87c6))
	ROM_LOAD("arom2.bin", 0x80000, 0x80000, CRC(a631bf12) SHA1(4784da1fabd2858b2c47af71784eb475cbbb4ab5))
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(6dddce0a) SHA1(6ad9b023ba8632dda0a4e04a4f66aac52ddd3b09))
ROM_END

ROM_START(waterwld2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom2.bin", 0x0000, 0x10000, CRC(c3d64cd7) SHA1(63bfd26fdc7082c2bb60c978508820442ac90f14))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(79164099) SHA1(fa048fb7aa91cadd6c0758c570a4c74337bd7cd5))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(2a8c5d04) SHA1(1a6a698fc05a199923721e91e68aaaa8d3c6a3c2))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(3ee37668) SHA1(9ced05b4f060568bf686974bc2472ff7c05a87c6))
	ROM_LOAD("arom2.bin", 0x80000, 0x80000, CRC(a631bf12) SHA1(4784da1fabd2858b2c47af71784eb475cbbb4ab5))
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(6dddce0a) SHA1(6ad9b023ba8632dda0a4e04a4f66aac52ddd3b09))
ROM_END

/*-------------------------------------------------------------------
/ Wipeout
/-------------------------------------------------------------------*/
ROM_START(wipeout)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(1161cdb7) SHA1(fdf4c0abb70a41149c69bd55c613849a662944d3))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(cbdec3ab) SHA1(2d70d436783830bf074a7a0590d5c48432136595))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(98ae6da4) SHA1(3842c2c4e708a5deae6b5d9407694d337b62384f))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(cccdf23a) SHA1(1b1e31f04cd60d64f0b9b8ab2c6169dacd0bce69))
	ROM_RELOAD(0x00000+0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(d4cc44a1) SHA1(c68264f00efa9f219fc257061ed39cd789e94126))
	ROM_RELOAD(0x80000+0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(f08e6d7f) SHA1(284214ac80735ddd36933ecd60debc7aea18403c))
ROM_END

/*-------------------------------------------------------------------
/ World Challenge Soccer
/-------------------------------------------------------------------*/
ROM_START(wcsoccer)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(6382c32e) SHA1(e212f4a9a77d1cf089acb226a8079ac4cae8a96d))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom.bin", 0x00000, 0x80000, CRC(71ba5263) SHA1(e86c2cc89d31534fb2d9d24fab2fcdb0af7cc73d))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(18d5edf3) SHA1(7d0d46506cf9d4b96b9b93139e3c65643e120c28))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(ece4eebf) SHA1(78f882668967194bd547ace5d22083faeb29ef5e))
	ROM_RELOAD(0x00000+0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(4e466500) SHA1(78c4b41a174d82a7e0e7775713c76e679c8a7e89))
	ROM_RELOAD(0x80000+0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(8b2795b0) SHA1(b838d4e410c815421099c65b0d3b22227dae17c6))
ROM_END

ROM_START(wcsoccerd2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(6382c32e) SHA1(e212f4a9a77d1cf089acb226a8079ac4cae8a96d))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom2.bin", 0x00000, 0x80000, CRC(4c8ea71d) SHA1(ce751b84e2033e4de2f2c57490867ecafd423aaa))
	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(18d5edf3) SHA1(7d0d46506cf9d4b96b9b93139e3c65643e120c28))
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(ece4eebf) SHA1(78f882668967194bd547ace5d22083faeb29ef5e))
	ROM_RELOAD(0x00000+0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(4e466500) SHA1(78c4b41a174d82a7e0e7775713c76e679c8a7e89))
	ROM_RELOAD(0x80000+0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(8b2795b0) SHA1(b838d4e410c815421099c65b0d3b22227dae17c6))
ROM_END

/************************************/
/* NOT OFFICIALY LISTED             */
/************************************/

/*-------------------------------------------------------------------
/ Unnamed game? by Toptronic HGmbH, Germany
/-------------------------------------------------------------------*/
ROM_START(tt_game)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(e7944b75) SHA1(b73f2e0004556c8aa88baef0cddcdefb5b905b8d))
	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, NO_DUMP)
	ROM_REGION(0x100000, "sound1", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x20000, CRC(b0983d90) SHA1(72e6a71f20fd5849543ca13813f062a3fc1d7dcf))
	ROM_RELOAD(0x00000+0x40000, 0x20000)
	ROM_RELOAD(0x00000+0x80000, 0x20000)
	ROM_RELOAD(0x00000+0xc0000, 0x20000)
	ROM_LOAD("arom2.bin", 0x20000, 0x20000, CRC(3e31ce58) SHA1(a2ef72d7b2bb821d1f62dce7212e31a1df3e7791))
	ROM_RELOAD(0x20000+0x40000, 0x20000)
	ROM_RELOAD(0x20000+0x80000, 0x20000)
	ROM_RELOAD(0x20000+0xc0000, 0x20000)
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, NO_DUMP)
ROM_END

/*-------------------------------------------------------------------
/ Strikes n' Spares (#N111)
/-------------------------------------------------------------------*/
ROM_START(snspares)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(9e018496) SHA1(a4995f153ba2179198cfc56b7011707328e4ec89))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom.bin", 0x00000, 0x40000, CRC(5c901899) SHA1(d106561b2e382afdb16e938072c9c8f1d1ccdae6))
	ROM_RELOAD( 0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user2", 0)
	ROM_LOAD("dsprom.bin", 0x00000, 0x40000, CRC(5c901899) SHA1(d106561b2e382afdb16e938072c9c8f1d1ccdae6))
	ROM_RELOAD( 0x40000, 0x40000)
	ROM_REGION(0x100000, "user3", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(e248574a) SHA1(d2bdc2b9a330bb81556d25d464f617e0934995eb))
ROM_END
ROM_START(snspares1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gprom1.bin", 0x0000, 0x10000, CRC(590393f4) SHA1(f52400c620e510253abd1c0719050b9bb09be942))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("dsprom.bin", 0x00000, 0x40000, CRC(5c901899) SHA1(d106561b2e382afdb16e938072c9c8f1d1ccdae6))
	ROM_RELOAD( 0x40000, 0x40000)
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "user2", 0)
	ROM_LOAD("dsprom.bin", 0x00000, 0x40000, CRC(5c901899) SHA1(d106561b2e382afdb16e938072c9c8f1d1ccdae6))
	ROM_RELOAD( 0x40000, 0x40000)
	ROM_REGION(0x100000, "user3", 0)
	ROM_LOAD("arom1.bin", 0x00000, 0x80000, CRC(e248574a) SHA1(d2bdc2b9a330bb81556d25d464f617e0934995eb))
ROM_END


GAME(1996,	barbwire,	0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Barb Wire",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1990,	bellring,	0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Bell Ringer",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1996,	brooks,		0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Brooks & Dunn (rev.T1)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1991,	cactjack,	0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Cactus Jack's",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1991,	carhop,		0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Car Hop",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1989,	ccruise,	0,			gts3,	gts3,	gts3,	ROT0,	"International Concepts","Caribbean Cruise",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1991,	clas1812,	0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Class of 1812",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1992,	cueball,	0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Cue Ball Wizard",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1990,	deadweap,	0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Deadly Weapon",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1995,	bighurt,	0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Frank Thomas' Big Hurt (rev.3)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1994,	freddy,		0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Freddy: A Nightmare on Elm Street (rev.3)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1994,	freddy4,	freddy,		gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Freddy: A Nightmare on Elm Street (rev.4)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1993,	gladiatp,	0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Gladiators",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1991,	hoops,		0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Hoops",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1989,	lca,		0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Lights...Camera...Action!",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1989,	lca2,		lca,		gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Lights...Camera...Action! (rev.2)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1995,	andretti,	0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Mario Andretti",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1995,	andretti4,	andretti,	gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Mario Andretti (rev.T4)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1990,	nudgeit,	0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Nudge-It",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1992,	opthund,	0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Operation Thunder",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1994,	rescu911,	0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Rescue 911 (rev.1)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1995,	shaqattq,	0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Shaq Attaq (rev.5)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1995,	shaqattq2,	shaqattq,	gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Shaq Attaq (rev.2)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1990,	silvslug,	0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Silver Slugger",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1995,	stargatp,	0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Stargate (Pinball)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1995,	stargatp1,	stargatp,	gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Stargate (rev.1)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1995,	stargatp2,	stargatp,	gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Stargate (rev.2)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1995,	stargatp3,	stargatp,	gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Stargate (rev.3)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1995,	stargatp4,	stargatp,	gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Stargate (rev.4)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1993,	sfight2,	0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Street Fighter 2",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1993,	sfight2a,	sfight2,	gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Street Fighter 2 (rev.1)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1993,	sfight2b,	sfight2,	gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Street Fighter 2 (rev.2)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1992,	smb,		0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Super Mario Brothers",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1992,	smb1,		smb,		gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Super Mario Brothers (rev.1)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1992,	smb2,		smb,		gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Super Mario Brothers (rev.2)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1992,	smb3,		smb,		gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Super Mario Brothers (rev.3)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1992,	smbmush,	0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Super Mario Brothers Mushroom World",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1991,	surfnsaf,	0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Surf'n Safari",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1993,	teedoffp,	0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Tee'd Off",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1993,	teedoffp1,	teedoffp,	gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Tee'd Off (rev.1)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1993,	teedoffp3,	teedoffp,	gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Tee'd Off (rev.3)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1990,	tfight,		0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Title Fight",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1990,	vegas,		0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Vegas",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1995,	waterwld,	0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Waterworld (rev.3)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1995,	waterwld2,	waterwld,	gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Waterworld (rev.2)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1993,	wipeout,	0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Wipeout (rev.2)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1994,	wcsoccer,	0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"World Challenge Soccer (rev.1)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1994,	wcsoccerd2,	wcsoccer,	gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"World Challenge Soccer (disp.rev.2)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(19??,	tt_game,	0,			gts3,	gts3,	gts3,	ROT0,	"Toptronic",			"unknown Toptronic pinball game",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1995,	snspares,	0,			gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Strikes n' Spares (rev.6)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1995,	snspares1,	snspares,	gts3,	gts3,	gts3,	ROT0,	"Gottlieb",				"Strikes n' Spares (rev.1)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
