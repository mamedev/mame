/*
    Bally MPU A084-91786-AH06 (6803)
*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"

class by6803_state : public driver_device
{
public:
	by6803_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};


static ADDRESS_MAP_START( by6803_map, AS_PROGRAM, 8, by6803_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
	AM_RANGE(0x0000, 0x00ff) AM_RAM
	AM_RANGE(0x1000, 0x17ff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( by6803 )
INPUT_PORTS_END

void by6803_state::machine_reset()
{
}

static DRIVER_INIT( by6803 )
{
}

static MACHINE_CONFIG_START( by6803, by6803_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6803, 1000000)
	MCFG_CPU_PROGRAM_MAP(by6803_map)
MACHINE_CONFIG_END

/*-----------------------------------------------------------
/ Atlantis
/-----------------------------------------------------------*/
ROM_START(atlantip)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u26_cpu.rom", 0x8000, 0x4000, CRC(b98491e1) SHA1(b867e2b24e93c4ee19169fe93c0ebfe0c1e2fc25))
	ROM_LOAD( "u27_cpu.rom", 0xc000, 0x4000, CRC(8ea2b4db) SHA1(df55a9fb70d1cabad51dc2b089af7904a823e1d8) )
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASEFF)
	ROM_REGION(0x30000, "sound1", 0)
	ROM_LOAD("u4_snd.rom", 0x00000, 0x8000, CRC(6a48b588) SHA1(c58dbfd920c279d7b9d2de8558d73c687b29ce9c))
	ROM_RELOAD(0x00000+0x8000, 0x8000)
	ROM_LOAD("u19_snd.rom", 0x10000, 0x8000, CRC(1387467c) SHA1(8b3dd6c2fc94cfebc1879795532c651cda202846))
	ROM_RELOAD(0x10000+0x8000, 0x8000)
	ROM_LOAD("u20_snd.rom", 0x20000, 0x8000, CRC(d5a6a773) SHA1(30807e03655d2249c801007350bfb228a2e8a0a4))
	ROM_RELOAD(0x20000+0x8000, 0x8000)
ROM_END

/*------------------------------------
/ Beat the Clock
/------------------------------------*/
ROM_START(beatclck)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "btc_u3.cpu", 0xc000, 0x4000, CRC(9ba822ab) SHA1(f28d38411df3978bcaf24177fa1b47037a586cbb) )
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("btc_u2.snd", 0xc000, 0x1000, CRC(fd22fd2a) SHA1(efad3b94e91d07930ada5366d389f35377dfbd99))
	ROM_LOAD("btc_u3.snd", 0xd000, 0x1000, CRC(22311a4a) SHA1(2c22ba9228e44e68b9308b3bf8803edcd70fa5b9))
	ROM_LOAD("btc_u4.snd", 0xe000, 0x1000, CRC(af1cf23b) SHA1(ebfa3afafd7850dfa2664d3c640fbfa631012455))
	ROM_LOAD("btc_u5.snd", 0xf000, 0x1000, CRC(230cf329) SHA1(45b17a785b81cd5b1d7fdfb720cf1990994b52b7))
ROM_END

/*------------------------------------
/ Karate Fight
/------------------------------------*/

/*------------------------------------
/ Black Belt
/------------------------------------*/
ROM_START(blackblt)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2.cpu", 0x8000, 0x4000, CRC(7c771910) SHA1(1df8ae478c3626a5200215bfca557ca42e064d2b))
	ROM_LOAD( "u3.cpu", 0xc000, 0x4000, CRC(bad0f4c3) SHA1(5e5240fda9f7f7f15f1953f12b132ba1c4fc886e) )
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("blck_u7.snd", 0x8000, 0x8000, CRC(db8bce07) SHA1(6327cfbb2761f4d190e2852f3321cdd0cc1e46a8))
ROM_END

/*------------------------------------
/ Blackwater 100
/------------------------------------*/
ROM_START(black100)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2.cpu", 0x8000, 0x4000, CRC(411fa773) SHA1(9756c7eee0f78792823a0b0379d2baac28cb03e8))
	ROM_LOAD( "u3.cpu", 0xc000, 0x4000, CRC(d6f6f890) SHA1(8fe4dae471f4c89f2fd72c6e647ead5206881c63))
	ROM_REGION(0x01000000, "cpu2", 0)
	ROM_LOAD16_BYTE("u12.bin", 0x00001, 0x10000, CRC(a0ecb282) SHA1(4655e0b85f7e8af8dda853279696718d3adbf7e3))
	ROM_LOAD16_BYTE("u11.bin", 0x00000, 0x10000, CRC(3f117ba3) SHA1(b4cded8fdd90ca030c6ff12c817701402c94baba))
	ROM_LOAD16_BYTE("u14.bin", 0x20001, 0x10000, CRC(b45bf5c4) SHA1(396ddf346e8ebd8cb91777521d93564d029f40b1))
	ROM_LOAD16_BYTE("u13.bin", 0x20000, 0x10000, CRC(f5890443) SHA1(77cd18cf5541ae9f7e2dd1c060a9bf29b242d05d))
ROM_END

ROM_START(black100s)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "sb2.cpu", 0x8000, 0x4000, CRC(b6fdbb0f) SHA1(5b36a725db3a1e023bbb54b8f85300fe99174b6e))
	ROM_LOAD( "sb3.cpu", 0xc000, 0x4000, CRC(ae9930b8) SHA1(1b6c63ce98939ecded300639d872df62548157a4))
	ROM_REGION(0x01000000, "cpu2", 0)
	ROM_LOAD16_BYTE("u12.bin", 0x00001, 0x10000, CRC(a0ecb282) SHA1(4655e0b85f7e8af8dda853279696718d3adbf7e3))
	ROM_LOAD16_BYTE("u11.bin", 0x00000, 0x10000, CRC(3f117ba3) SHA1(b4cded8fdd90ca030c6ff12c817701402c94baba))
	ROM_LOAD16_BYTE("u14.bin", 0x20001, 0x10000, CRC(b45bf5c4) SHA1(396ddf346e8ebd8cb91777521d93564d029f40b1))
	ROM_LOAD16_BYTE("u13.bin", 0x20000, 0x10000, CRC(f5890443) SHA1(77cd18cf5541ae9f7e2dd1c060a9bf29b242d05d))
ROM_END

/*------------------------------------
/ City Slicker
/------------------------------------*/
ROM_START(cityslck)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2.128", 0x8000, 0x4000, CRC(94bcf162) SHA1(1d83592ad2441fc5e4c6fd3ab2373614dfe78b34))
	ROM_LOAD( "u3.128", 0xc000, 0x4000, CRC(97cb2bca) SHA1(0cbd49bbce2ce26c720d8a52bd4d1256f0ac61b3) )
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("u7_snd.512", 0x0000, 0x10000, CRC(6941d68a) SHA1(28de4327f328d16ec4cab59642c185777535efb2))
ROM_END

/*------------------------------------
/ Dungeons & Dragons
/------------------------------------*/
ROM_START(dungdrag)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.128", 0x8000, 0x4000, CRC(cefd4330) SHA1(0bffb2b73229e9908a018e06daeceb736896e5f0))
	ROM_LOAD( "cpu_u3.128", 0xc000, 0x4000, CRC(4bacc7f5) SHA1(71dd898924e0e968c4f3ba8a261e6b382d8ae0f1) )
	ROM_REGION(0x01000000, "cpu2", 0)
	ROM_LOAD16_BYTE("snd_u12.512", 0x00001, 0x10000, CRC(dd95f851) SHA1(6fa46b512bced0d1862b2621e195ef0dfd24f928))
	ROM_LOAD16_BYTE("snd_u11.512", 0x00000, 0x10000, CRC(dcd461b3) SHA1(834000cfb6c6acf5c296db58971251819971f4de))
	ROM_LOAD16_BYTE("snd_u14.512", 0x20001, 0x10000, CRC(dd9e61eb) SHA1(fd1ec58f5708d5abf3d7424954ce054454514283))
	ROM_LOAD16_BYTE("snd_u13.512", 0x20000, 0x10000, CRC(1e2d9211) SHA1(f5fcf1c07f01e7f1a7abff9ac3c481b84471d3a6))
ROM_END

/*------------------------------------
/ Eight Ball Champ
/------------------------------------*/
ROM_START(eballchp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u3_cpu.128", 0xc000, 0x4000, CRC(025f3008) SHA1(25d310f169b92ce6b348330816ddc3b5710e57da) )
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("u3_snd.532", 0xd000, 0x1000, CRC(4836d70d) SHA1(a4acc64609d91a84ba4c8101186d07397b496600))
	ROM_LOAD("u4_snd.532", 0xe000, 0x1000, CRC(4b49d94d) SHA1(52d5f4b7604601cd86f0e80ed7c4fe09d14f5976))
	ROM_LOAD("u5_snd.532", 0xf000, 0x1000, CRC(655441df) SHA1(9da5578856ded3dcdafed67679eb4c4134dc9f81))
ROM_END

ROM_START(eballch2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u3_cpu.128", 0xc000, 0x4000, CRC(025f3008) SHA1(25d310f169b92ce6b348330816ddc3b5710e57da) )
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("ebcu4.snd", 0x8000, 0x2000, NO_DUMP)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_LOAD("ebcu3.snd", 0xc000, 0x2000, NO_DUMP)
	ROM_RELOAD(0xe000, 0x2000)
ROM_END

/*------------------------------------------------
/ Escape from the Lost World
/-----------------------------------------------*/
ROM_START(esclwrld)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2.128", 0x8000, 0x4000, CRC(b11a97ea) SHA1(29339785a67ed7dc9eb39ddc7bb7e6baaf731210))
	ROM_LOAD( "u3.128", 0xc000, 0x4000, CRC(5385a562) SHA1(a6c39532d01db556e4bdf90020a9d9905238e8ef))
	ROM_REGION(0x01000000, "cpu2", 0)
	ROM_LOAD16_BYTE("u12.512", 0x00001, 0x10000, CRC(0c003473) SHA1(8ada2aa546a6499c5e2b5eb45a1975b8285d25f9))
	ROM_LOAD16_BYTE("u11.512", 0x00000, 0x10000, CRC(360f6658) SHA1(c0346952dcd33bbcf4c43c51cde5433a099a7a5d))
	ROM_LOAD16_BYTE("u14.512", 0x20001, 0x10000, CRC(0b92afff) SHA1(78f51989e74ced9e0a81c4e18d5abad71de01faf))
	ROM_LOAD16_BYTE("u13.512", 0x20000, 0x10000, CRC(b056842e) SHA1(7c67e5d69235a784b9c38cb31302d206278a3814))
ROM_END

ROM_START(esclwrldg)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2_ger.128", 0x8000, 0x4000, CRC(0a6ab137) SHA1(0627b7c67d13f305f2287f3cfa023c8dd7721250))
	ROM_LOAD( "u3_ger.128", 0xc000, 0x4000, CRC(26d8bfbb) SHA1(3b81fb0e736d14004bbbbb2edd682fdfc1b2c832))
	ROM_REGION(0x01000000, "cpu2", 0)
	ROM_LOAD16_BYTE("u12.512", 0x00001, 0x10000, CRC(0c003473) SHA1(8ada2aa546a6499c5e2b5eb45a1975b8285d25f9))
	ROM_LOAD16_BYTE("u11.512", 0x00000, 0x10000, CRC(360f6658) SHA1(c0346952dcd33bbcf4c43c51cde5433a099a7a5d))
	ROM_LOAD16_BYTE("u14.512", 0x20001, 0x10000, CRC(0b92afff) SHA1(78f51989e74ced9e0a81c4e18d5abad71de01faf))
	ROM_LOAD16_BYTE("u13.512", 0x20000, 0x10000, CRC(b056842e) SHA1(7c67e5d69235a784b9c38cb31302d206278a3814))
ROM_END
/*------------------------------------
/ Hardbody
/------------------------------------*/
ROM_START(hardbody)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.128", 0x8000, 0x4000, CRC(c9248b47) SHA1(54239bd7d15574ebbb70ed306a804b7b32ed516a))
	ROM_LOAD( "cpu_u3.128", 0xc000, 0x4000, CRC(31c255d0) SHA1(b6ffa2616ae9a4a121585cc402080ec6f26f8472))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound_u7.512", 0x0000, 0x10000, CRC(c96f91af) SHA1(9602a8991ca0cf9a7c68710f55c245d9c675b06f))
ROM_END

ROM_START(hardbodyg)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "hrdbdy-g.u2", 0x8000, 0x4000, CRC(fce357cc) SHA1(f7d13c12aabcb3c5bb5826b1911817bd359f1941))
	ROM_LOAD( "hrdbdy-g.u3", 0xc000, 0x4000, CRC(ccac74b5) SHA1(d55cfc8ee866a9af4567d56890f5a9ecb9c3c02f))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound_u7.512", 0x0000, 0x10000, CRC(c96f91af) SHA1(9602a8991ca0cf9a7c68710f55c245d9c675b06f))
ROM_END

/*-----------------------------------------
/ Heavy Metal Meltdown
/-----------------------------------------*/
ROM_START(hvymetap)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2.rom", 0x8000, 0x4000, CRC(53466e4e) SHA1(af6d0e15821ff707f24bb99b8d9dfb9f929906db))
	ROM_LOAD( "u3.rom", 0xc000, 0x4000, CRC(0a08ae7e) SHA1(04f295fbe3a7bd7b929556338914c0ed94a77d62) )
	ROM_REGION(0x01000000, "cpu2", 0)
	ROM_LOAD16_BYTE("u12.rom", 0x00001, 0x10000, CRC(77933258) SHA1(42a01e97440dbb7d3da92dbfbad2516f4b553a5f))
	ROM_LOAD16_BYTE("u11.rom", 0x00000, 0x10000, CRC(b7e4de7d) SHA1(bcc89e10c368cdbc5137d8f585e109c0be25522d))
ROM_END

/*------------------------------------
/ Lady Luck
/------------------------------------*/
ROM_START(ladyluck)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u3.cpu", 0xc000, 0x4000, CRC(129f41f5) SHA1(0351419814d3f4e98a4572fdec9d53e12fe6b6be) )
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("u4_snd.532", 0x8000, 0x2000, CRC(e9ef01e6) SHA1(79191e776b6683b259cd1a80e9fb3183268bde56))
	ROM_RELOAD(0xa000, 0x2000)
	ROM_LOAD("u3_snd.532", 0xc000, 0x2000, CRC(1bdd6e2b) SHA1(14fc25b5f8eefe8ffab062f83e06ec19403aa00a))
	ROM_RELOAD(0xe000, 0x2000)
ROM_END

/*--------------------------------
/ MotorDome
/-------------------------------*/
ROM_START(motrdome)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "modm_u2.dat", 0x8000, 0x4000, CRC(820ca073) SHA1(0b50712f7d65f629af934deccc52d588f390a05b))
	ROM_LOAD( "modm_u3.dat", 0xc000, 0x4000, CRC(aae7c418) SHA1(9d3ea83ffff0b9696f5113043475c6e9b9a464ae) )
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("modm_u7.snd", 0x8000, 0x8000, CRC(29ce4679) SHA1(f17998198b542dd99a34abd678db7e031bde074b))
ROM_END

/*--------------------------------
/ Party Animal
/-------------------------------*/
ROM_START(prtyanim)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.128", 0x8000, 0x4000, CRC(abdc0b2d) SHA1(b93c7248ea83461101383023bd4e4a50292d8570))
	ROM_LOAD( "cpu_u3.128", 0xc000, 0x4000, CRC(e48b2d63) SHA1(190fc5a805bda9617c08a29c0bde4d94a77279e9) )
	ROM_REGION(0x01000000, "cpu2", 0)
	ROM_LOAD16_BYTE("snd_u12.512", 0x00001, 0x10000, CRC(265a9494) SHA1(3b631f2b1c8c685aef32fb6c5289cd792711ff7e))
	ROM_LOAD16_BYTE("snd_u11.512", 0x00000, 0x10000, CRC(20be998f) SHA1(7f98073d0f559e081b2d6dc8c1f3462e3fe9a713))
	ROM_LOAD16_BYTE("snd_u14.512", 0x20001, 0x10000, CRC(639b3db1) SHA1(e07669c3186c963f2fea29bcf5675ac86eb07c86))
	ROM_LOAD16_BYTE("snd_u13.512", 0x20000, 0x10000, CRC(b652597b) SHA1(8b4074a545d420319712a1fdd77a3bfb282ed9cd))
ROM_END

/*------------------------------------
/ Special Force
/------------------------------------*/
ROM_START(specforc)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2_revc.128", 0x8000, 0x4000, CRC(d042af04) SHA1(0a73ee6d3ce603899fd89de70f90e9efc58b8b42))
	ROM_LOAD( "u3_revc.128", 0xc000, 0x4000, CRC(d48a5eaf) SHA1(90a5d5e928abfec699bae9d0087e90316339058f) )
	ROM_REGION(0x01000000, "cpu2", 0)
	ROM_LOAD16_BYTE("u12_snd.512", 0x00001, 0x10000, CRC(4f48a490) SHA1(6c9a594ecc68adf3b1eda315c4704e1d025a3442))
	ROM_LOAD16_BYTE("u11_snd.512", 0x00000, 0x10000, CRC(b16eb713) SHA1(461e5ed82891d17849984137536bc6d1ab2907c2))
	ROM_LOAD16_BYTE("u14_snd.512", 0x20001, 0x10000, CRC(6911fa51) SHA1(a75f75bb4493b0ea3a423bc033d49022228d79c1))
	ROM_LOAD16_BYTE("u13_snd.512", 0x20000, 0x10000, CRC(3edda92d) SHA1(dbd95bb1c534779f56cc9e30efef159feaf22712))
ROM_END

/*------------------------------------
/ Strange Science
/------------------------------------*/
ROM_START(strngsci)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.128", 0x8000, 0x4000, CRC(2ffcf284) SHA1(27d66806708c983092bab4ed6965c2e91e69acdc))
	ROM_LOAD( "cpu_u3.128", 0xc000, 0x4000, CRC(35257931) SHA1(d3d6b84e50677a4c5f9d5c13c9522ad6d3a1358d) )
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound_u7.256", 0x8000, 0x8000, CRC(bc33901e) SHA1(5231d8f01a107742acee2d13580a461063018a11))
ROM_END

/*-------------------------------------------------------------
/ Truck Stop
/-------------------------------------------------------------*/
ROM_START(trucksp3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2_p3.128", 0x8000, 0x4000, CRC(79b2a5b1) SHA1(d3de91bfadc9684302b2367cfcb30ed0d6faa020))
	ROM_LOAD( "u3_p3.128", 0xc000, 0x4000, CRC(2993373c) SHA1(26490f1dd8a5329a88a2ceb1e6044711a29f1445))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASE00)
	ROM_REGION(0x30000, "sound1", 0)
	ROM_LOAD("u4sndp1.256", 0x00000, 0x8000, CRC(120a386f) SHA1(51b3b45eb7ea63758b21aad404ba12a9607fec44))
	ROM_RELOAD(0x00000 +0x8000, 0x8000)
	ROM_LOAD("u19sndp1.256", 0x10000, 0x8000, CRC(5cd43dda) SHA1(23dd8a52ea1340fc239a246af0d94da905464efb))
	ROM_RELOAD(0x10000 +0x8000, 0x8000)
	ROM_LOAD("u20sndp1.256", 0x20000, 0x8000, CRC(93ac5c33) SHA1(f6dc84eca4678188a58ba3c8ef18975164dd29b0))
	ROM_RELOAD(0x20000 +0x8000, 0x8000)
ROM_END

ROM_START(trucksp2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "u2_p2.128", 0x8000, 0x4000, CRC(3c397dec) SHA1(2fc86ad39c935ce8615eafd67e571ac94c938cd7))
	ROM_LOAD( "u3_p2.128", 0xc000, 0x4000, CRC(d7ac519a) SHA1(612bf9fee0d54e8b1215508bd6c1ea61dcb99951))
	ROM_REGION(0x10000, "cpu2", ROMREGION_ERASE00)
	ROM_REGION(0x30000, "sound1", 0)
	ROM_LOAD("u4sndp1.256", 0x00000, 0x8000, CRC(120a386f) SHA1(51b3b45eb7ea63758b21aad404ba12a9607fec44))
	ROM_RELOAD(0x00000 +0x8000, 0x8000)
	ROM_LOAD("u19sndp1.256", 0x10000, 0x8000, CRC(5cd43dda) SHA1(23dd8a52ea1340fc239a246af0d94da905464efb))
	ROM_RELOAD(0x10000 +0x8000, 0x8000)
	ROM_LOAD("u20sndp1.256", 0x20000, 0x8000, CRC(93ac5c33) SHA1(f6dc84eca4678188a58ba3c8ef18975164dd29b0))
	ROM_RELOAD(0x20000 +0x8000, 0x8000)
ROM_END


GAME( 1989, atlantip, 0,		by6803, by6803, by6803, ROT0, "Bally","Atlantis", GAME_IS_SKELETON_MECHANICAL)
GAME( 1985, beatclck, 0,		by6803, by6803, by6803, ROT0, "Bally","Beat the Clock", GAME_IS_SKELETON_MECHANICAL)
GAME( 1986, blackblt, 0,		by6803, by6803, by6803, ROT0, "Bally","Black Belt", GAME_IS_SKELETON_MECHANICAL)
GAME( 1988, black100, 0,		by6803, by6803, by6803, ROT0, "Bally","Blackwater 100", GAME_IS_SKELETON_MECHANICAL)
GAME( 1988, black100s, black100,	by6803, by6803, by6803, ROT0, "Bally","Blackwater 100 (Single Ball Play)", GAME_IS_SKELETON_MECHANICAL)
GAME( 1987, cityslck, 0,		by6803, by6803, by6803, ROT0, "Bally","City Slicker", GAME_IS_SKELETON_MECHANICAL)
GAME( 1987, dungdrag, 0,		by6803, by6803, by6803, ROT0, "Bally","Dungeons & Dragons", GAME_IS_SKELETON_MECHANICAL)
GAME( 1985, eballchp, 0,		by6803, by6803, by6803, ROT0, "Bally","Eight Ball Champ", GAME_IS_SKELETON_MECHANICAL)
GAME( 1987, esclwrld, 0,		by6803, by6803, by6803, ROT0, "Bally","Escape from the Lost World", GAME_IS_SKELETON_MECHANICAL)
GAME( 1987, esclwrldg, esclwrld,	by6803, by6803, by6803, ROT0, "Bally","Escape from the Lost World (German)", GAME_IS_SKELETON_MECHANICAL)
GAME( 1987, hardbody, 0,		by6803, by6803, by6803, ROT0, "Bally","Hardbody", GAME_IS_SKELETON_MECHANICAL)
GAME( 1987, hardbodyg, hardbody,	by6803, by6803, by6803, ROT0, "Bally","Hardbody (German)", GAME_IS_SKELETON_MECHANICAL)
GAME( 1987, hvymetap, 0,		by6803, by6803, by6803, ROT0, "Bally","Heavy Metal Meltdown", GAME_IS_SKELETON_MECHANICAL)
GAME( 1986, ladyluck, 0,		by6803, by6803, by6803, ROT0, "Bally","Lady Luck", GAME_IS_SKELETON_MECHANICAL)
GAME( 1986, motrdome, 0,		by6803, by6803, by6803, ROT0, "Bally","MotorDome", GAME_IS_SKELETON_MECHANICAL)
GAME( 1987, prtyanim, 0,		by6803, by6803, by6803, ROT0, "Bally","Party Animal", GAME_IS_SKELETON_MECHANICAL)
GAME( 1986, specforc, 0,		by6803, by6803, by6803, ROT0, "Bally","Special Force", GAME_IS_SKELETON_MECHANICAL)
GAME( 1986, strngsci, 0,		by6803, by6803, by6803, ROT0, "Bally","Strange Science", GAME_IS_SKELETON_MECHANICAL)
GAME( 1988, trucksp3, 0,		by6803, by6803, by6803, ROT0, "Bally","Truck Stop (P-3)", GAME_IS_SKELETON_MECHANICAL)
GAME( 1988, trucksp2, trucksp3, by6803, by6803, by6803, ROT0, "Bally","Truck Stop (P-2)", GAME_IS_SKELETON_MECHANICAL)
