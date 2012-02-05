/*
    Zaccaria Generation 1
*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/s2650/s2650.h"

class zac_1_state : public driver_device
{
public:
	zac_1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};


static ADDRESS_MAP_START( zac_1_map, AS_PROGRAM, 8, zac_1_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( zac_1 )
INPUT_PORTS_END

void zac_1_state::machine_reset()
{
}

static DRIVER_INIT( zac_1 )
{
}

static MACHINE_CONFIG_START( zac_1, zac_1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", S2650, 6000000/2)
	MCFG_CPU_PROGRAM_MAP(zac_1_map)
MACHINE_CONFIG_END


/*--------------------------------
/ Earth Wind Fire (04/81)
/-------------------------------*/
ROM_START(ewf)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "zac_boot.lgc", 0x0000, 0x0800, CRC(62a3da59) SHA1(db571139aff61757f6c0fda6fa0d1fea8257cb15))
	ROM_LOAD ( "ewf_2.lgc", 0x1c00, 0x0400, CRC(aa67e0b4) SHA1(4491eff7081fd5e397974fac1156992ce2012d0b))
	ROM_LOAD ( "ewf_3.lgc", 0x0800, 0x0400, CRC(b21bf015) SHA1(ecddfe1d6797c39e094a7f86efabf0abea0fa4af))
	ROM_LOAD ( "ewf_4.lgc", 0x0c00, 0x0400, CRC(d110da3f) SHA1(88e27347d209fab5be924f95b0a001476ea92c1f))
	ROM_LOAD ( "ewf_5.lgc", 0x1000, 0x0400, CRC(f695dab6) SHA1(48ca60718cea40baa5052f690c8d69eb7ab32b0e))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("ewf.snd", 0x0000, 0x0800, CRC(5079e493) SHA1(51d366cdd09ad00b8b016b0ea1c85ac95ef94d71))
ROM_END

/*--------------------------------
/ Fire Mountain (01/80)
/-------------------------------*/
ROM_START(firemntn)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "zac_boot.lgc", 0x0000, 0x0800, CRC(62a3da59) SHA1(db571139aff61757f6c0fda6fa0d1fea8257cb15))
	ROM_LOAD ( "firemt_2.lgc", 0x1c00, 0x0400, CRC(d146253f) SHA1(69910ddd1b7f1a0a0db689e750a0288d10e92951))
	ROM_LOAD ( "firemt_3.lgc", 0x0800, 0x0400, CRC(d9faae07) SHA1(9883be01e2d359a111528029407141c9792c3583))
	ROM_LOAD ( "firemt_4.lgc", 0x0c00, 0x0400, CRC(b5cac3da) SHA1(94f1153571a099574d041a5168854056a692a03d))
	ROM_LOAD ( "firemt_5.lgc", 0x1000, 0x0400, CRC(13f11d84) SHA1(031f43467a4a01810297e3bfe0762ed2eed4e251))
ROM_END

/*--------------------------------
/ Future World (10/78)
/-------------------------------*/
// Game ROMs #2 and #3 have to be swapped!
ROM_START(futurwld)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "futwld_1.lgc", 0x0000, 0x0400, CRC(d83b8793) SHA1(3bb04d8395191ecf324b6da0bcddcf7bd8d41867))
	ROM_LOAD ( "futwld_3.lgc", 0x0400, 0x0400, CRC(bdcb7e1d) SHA1(e6c0c7e8188df87937f0b22dbb0639872e03e948))
	ROM_LOAD ( "futwld_2.lgc", 0x0800, 0x0400, CRC(48e3d293) SHA1(0029f30c4a94067e7782e22499b11db86f051934))
	ROM_LOAD ( "futwld_4.lgc", 0x0c00, 0x0400, CRC(b1de2120) SHA1(970e1c4eadb7ace1398684accac289a434d13d84))
	ROM_LOAD ( "futwld_5.lgc", 0x1000, 0x0400, CRC(6b7965f2) SHA1(31314bc63f01717004c5c2448b5db7d292145b60))
ROM_END

/*--------------------------------
/ Horror
/-------------------------------*/

/*--------------------------------
/ Hot Wheels (09/79)
/-------------------------------*/
ROM_START(hotwheel)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "zac_boot.lgc", 0x0000, 0x0800, CRC(62a3da59) SHA1(db571139aff61757f6c0fda6fa0d1fea8257cb15))
	ROM_LOAD ( "htwhls_2.lgc", 0x1c00, 0x0400, CRC(7ff870ae) SHA1(274ee7c2cb92b6710c546058e7277f06720b5e37))
	ROM_LOAD ( "htwhls_3.lgc", 0x0800, 0x0400, CRC(7c1fba91) SHA1(d514e9b3128dfe7999e414fd9044dc20c0d76c66))
	ROM_LOAD ( "htwhls_4.lgc", 0x0c00, 0x0400, CRC(974804ba) SHA1(f35c1b52327b2d3170a9a28dbee4d1437f1f594a))
	ROM_LOAD ( "htwhls_5.lgc", 0x1000, 0x0400, CRC(e28f3c60) SHA1(eb780be60b41017d105288cef71906d15474b8fa))
ROM_END

/*--------------------------------
// House of Diamonds (07/78)
/-------------------------------*/
ROM_START(hod)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "hod_1.bin", 0x0000, 0x0400, CRC(b666af0e) SHA1(e6a96ed30733e7b011ba35d1a628cefd073f29a1))
	ROM_LOAD ( "hod_2.bin", 0x0400, 0x0400, CRC(956aac25) SHA1(2a59c3589d14e36ab2c61c6fbc9e8212410a385b))
	ROM_LOAD ( "hod_3.bin", 0x0800, 0x0400, CRC(88b05360) SHA1(44992a01eaa8f58296d6fb003da8dad528f2b937))
	ROM_LOAD ( "hod_4.bin", 0x0c00, 0x0400, CRC(25b6be1f) SHA1(351138404865d69ccb3ad450deda0776e987fdd2))
	ROM_LOAD ( "hod_5.bin", 0x1000, 0x0400, CRC(81b73c40) SHA1(21b80cff132becdb028e6ee895231da635189ef4))
ROM_END

/*--------------------------------
/ Locomotion (09/81)
/-------------------------------*/
ROM_START(locomotp)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "loc-1.fil", 0x0000, 0x0800, CRC(8d0252a2) SHA1(964dca642fb26eef2c132eca354a0ffce32e25df))
	ROM_LOAD ( "loc-2.fil", 0x1c00, 0x0400, CRC(9dbd8601) SHA1(10bc37d2691c7237a14e0718febed2aa7822db23))
	ROM_LOAD ( "loc-3.fil", 0x0800, 0x0400, CRC(8cadea7b) SHA1(e712add828dd22a2b495f0479f949748db21fbf7))
	ROM_CONTINUE(0x1400, 0x0400)
	ROM_LOAD ( "loc-4.fil", 0x0c00, 0x0400, CRC(177c89b6) SHA1(23de8208dbbf141952a974514fc752ed2eb6b202))
	ROM_LOAD ( "loc-5.fil", 0x1000, 0x0400, CRC(cad4122a) SHA1(df29914adeb9675abbd9f43dbef23adf2fe96c81))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("loc-snd.fil", 0x0000, 0x0800, CRC(51ea9d2a) SHA1(9a68687af2c1cad2a261f61a67a625d906c502e1))
ROM_END

/*--------------------------------
/ Shooting the Rapids (04/79)
/-------------------------------*/
ROM_START(strapids)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "rapids_1.lgc", 0x0000, 0x0400, CRC(2a30cef3) SHA1(1af0ad08316fca565a6de1d308ed0495907656e7))
	ROM_LOAD ( "rapids_2.lgc", 0x0400, 0x0400, CRC(04adaa14) SHA1(7819de53cee669b7e42624cd577ed1e3b771d2a9))
	ROM_LOAD ( "rapids_3.lgc", 0x0800, 0x0400, CRC(397992fb) SHA1(46e4f293fc8d8094eb16030261342504694fbf8f))
	ROM_LOAD ( "rapids_4.lgc", 0x0c00, 0x0400, CRC(3319fa21) SHA1(b384a7347e0d6ca3bec53f356312b66d66b5b03f))
	ROM_LOAD ( "rapids_5.lgc", 0x1000, 0x0400, CRC(0dd67110) SHA1(0c32e400ef07d7243148ae280e145a3e050313e8))
ROM_END

/*--------------------------------
/ Space Shuttle (09/80)
/-------------------------------*/
ROM_START(sshtlzac)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "zac_boot.lgc", 0x0000, 0x0800, CRC(62a3da59) SHA1(db571139aff61757f6c0fda6fa0d1fea8257cb15))
	ROM_LOAD ( "spcshtl2.lgc", 0x1c00, 0x0400, CRC(0e06771b) SHA1(f30f3727f24219e5047c871fe81c2e172a17cd38))
	ROM_LOAD ( "spcshtl3.lgc", 0x0800, 0x0400, CRC(a302e5a9) SHA1(1585f4000d105a7a2be5638ade9ab8668e6c8a5e))
	ROM_LOAD ( "spcshtl4.lgc", 0x0c00, 0x0400, CRC(a02ee0b5) SHA1(50532bdc347ecfdbd4cc43403ff2cb1dcb1fe1ac))
	ROM_LOAD ( "spcshtl5.lgc", 0x1000, 0x0400, CRC(d1dabd9b) SHA1(0d28336764f43fa4d1b23d849b6ec0f60b2b4ecf))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("spcshtl.snd", 0x0000, 0x0800, CRC(9a61781c) SHA1(0293640653d8cc9532debd31bbb70f025b4e6d03))
ROM_END

/*--------------------------------
/ Star God (05/80)
/-------------------------------*/
ROM_START(stargod)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "zac_boot.lgc", 0x0000, 0x0800, CRC(62a3da59) SHA1(db571139aff61757f6c0fda6fa0d1fea8257cb15))
	ROM_LOAD ( "stargod2.lgc", 0x1c00, 0x0400, CRC(7a784b03) SHA1(bc3490b69913f52e3e9db5c3de5617ab89efe073))
	ROM_LOAD ( "stargod3.lgc", 0x0800, 0x0400, CRC(95492ac0) SHA1(992ad53efc5b53020e3939dfca5431fd50b6571c))
	ROM_LOAD ( "stargod4.lgc", 0x0c00, 0x0400, CRC(09e5682a) SHA1(c9fcad4f55ee005e204a49fa65e7d77ecfde9680))
	ROM_LOAD ( "stargod5.lgc", 0x1000, 0x0400, CRC(43ba2462) SHA1(6749bdceca4a1dc2bc90d7ee3b671f52219e1af4))
ROM_END

ROM_START(stargoda)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "zac_boot.lgc", 0x0000, 0x0800, CRC(62a3da59) SHA1(db571139aff61757f6c0fda6fa0d1fea8257cb15))
	ROM_LOAD ( "stargod2.lgc", 0x1c00, 0x0400, CRC(7a784b03) SHA1(bc3490b69913f52e3e9db5c3de5617ab89efe073))
	ROM_LOAD ( "stargod3.lgc", 0x0800, 0x0400, CRC(95492ac0) SHA1(992ad53efc5b53020e3939dfca5431fd50b6571c))
	ROM_LOAD ( "stargod4.lgc", 0x0c00, 0x0400, CRC(09e5682a) SHA1(c9fcad4f55ee005e204a49fa65e7d77ecfde9680))
	ROM_LOAD ( "stargod5.lgc", 0x1000, 0x0400, CRC(43ba2462) SHA1(6749bdceca4a1dc2bc90d7ee3b671f52219e1af4))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("stargod.snd", 0x7800, 0x0800, CRC(c9103a68) SHA1(cc77af54fdb192f0b334d9d1028210618c3f1d95))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Winter Sports (01/78)
/-------------------------------*/
ROM_START(wsports)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "ws1.bin", 0x0000, 0x0400, CRC(58feb058) SHA1(50216bba5be28284e63d826543297d1b6b609325))
	ROM_LOAD ( "ws2.bin", 0x0400, 0x0400, CRC(ece702cb) SHA1(84cf0976b33bd7cf25976de9c66cc85808f1cd50))
	ROM_LOAD ( "ws3.bin", 0x0800, 0x0400, CRC(ff7f6824) SHA1(0eef4aca51c0e823f7634d7fc22c96c590239269))
	ROM_LOAD ( "ws4.bin", 0x0c00, 0x0400, CRC(74460cf2) SHA1(4afa612af1eff8eae686ceba7c117bc7962272c7))
	ROM_LOAD ( "ws5.bin", 0x1000, 0x0400, CRC(5ef51ced) SHA1(390579d0482ceabf87924f7718ef33e336726d92))
ROM_END

GAME(1981, ewf,       0,       zac_1,  zac_1,  zac_1,  ROT0,  "Zaccaria",    "Earth Wind Fire",     GAME_IS_SKELETON_MECHANICAL)
GAME(1980, firemntn,  0,       zac_1,  zac_1,  zac_1,  ROT0,  "Zaccaria",    "Fire Mountain",       GAME_IS_SKELETON_MECHANICAL)
GAME(1978, futurwld,  0,       zac_1,  zac_1,  zac_1,  ROT0,  "Zaccaria",    "Future World",        GAME_IS_SKELETON_MECHANICAL)
GAME(1979, hotwheel,  0,       zac_1,  zac_1,  zac_1,  ROT0,  "Zaccaria",    "Hot Wheels",          GAME_IS_SKELETON_MECHANICAL)
GAME(1978, hod,       0,       zac_1,  zac_1,  zac_1,  ROT0,  "Zaccaria",    "House of Diamonds",   GAME_IS_SKELETON_MECHANICAL)
GAME(1981, locomotp,  0,       zac_1,  zac_1,  zac_1,  ROT0,  "Zaccaria",    "Locomotion",          GAME_IS_SKELETON_MECHANICAL)
GAME(1979, strapids,  0,       zac_1,  zac_1,  zac_1,  ROT0,  "Zaccaria",    "Shooting the Rapids", GAME_IS_SKELETON_MECHANICAL)
GAME(1980, sshtlzac,  0,       zac_1,  zac_1,  zac_1,  ROT0,  "Zaccaria",    "Space Shuttle (Zaccaria)",   GAME_IS_SKELETON_MECHANICAL)
GAME(1980, stargod,   0,       zac_1,  zac_1,  zac_1,  ROT0,  "Zaccaria",    "Star God",            GAME_IS_SKELETON_MECHANICAL)
GAME(1980, stargoda,  stargod, zac_1,  zac_1,  zac_1,  ROT0,  "Zaccaria",    "Star God (alternate sound)", GAME_IS_SKELETON_MECHANICAL)
GAME(1978, wsports,   0,       zac_1,  zac_1,  zac_1,  ROT0,  "Zaccaria",    "Winter Sports",       GAME_IS_SKELETON_MECHANICAL)
