/*
 Ace System 1 Hardware
 Fruit Machines

 skeleton driver!

sets placed here on
total rom size 0x8000
ram at 0x8000-0x87ff
lots of reads from 0xe000 at the start

*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/z80/z80.h"

class aces1_state : public driver_device
{
public:
	aces1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;
};



static ADDRESS_MAP_START( aces1_map, AS_PROGRAM, 8, aces1_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( aces1_portmap, AS_IO, 8, aces1_state )
ADDRESS_MAP_END


static INPUT_PORTS_START( aces1 )
INPUT_PORTS_END


static MACHINE_CONFIG_START( aces1, aces1_state )

	MCFG_CPU_ADD("maincpu", Z80, 4000000) /* ?? Mhz */
	MCFG_CPU_PROGRAM_MAP(aces1_map)
	MCFG_CPU_IO_MAP(aces1_portmap)

MACHINE_CONFIG_END



ROM_START( ac1cshtw )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ctp1.bin", 0x0000, 0x008000, CRC(2fabb08f) SHA1(b737930e428f9258ab22394229c2b5039edf8f97) )
ROM_END


ROM_START( ac1clbmn )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cmoney.bin", 0x0000, 0x008000, CRC(db1ce4df) SHA1(315a94d8ddad96e86e082e5ded1fbcdb7ef41908) )

	ROM_REGION( 0x80000, "alt", 0 ) // same thing but in smaller roms? should they all really be like this?
	ROM_LOAD( "cm388p71.bin", 0x0000, 0x002000, CRC(c686c1e3) SHA1(c60bf1616b30413f8ecd4a2c8b75d1b37f456c1f) )
	ROM_LOAD( "cm388p72.bin", 0x2000, 0x002000, CRC(f2cb5fdf) SHA1(fb87865b366cde88b5b4b8cec64d52d2c15a3ee5) )
	ROM_LOAD( "cm388p73.bin", 0x4000, 0x002000, CRC(6ee672c3) SHA1(2258088ae4224f06a250040d7c0b8fd964a9e56c) )
	ROM_LOAD( "cm388p74.bin", 0x6000, 0x002000, CRC(db3e2581) SHA1(26d1b58318f126e88190b67d87ba5bbb802d45ba) )
ROM_END



ROM_START( ac1gogld )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "goforgold.bin", 0x0000, 0x008000, CRC(2ce24d23) SHA1(72807c37fee5853ffe21df2cdf2802cc3c96f1c6) )

	ROM_REGION( 0x80000, "alt", 0 ) // same thing but in smaller roms? should they all really be like this?
	ROM_LOAD( "370gg_1.bin", 0x0000, 0x002000, CRC(c1337c4c) SHA1(e8d372e2faeb84eec50e297b183f2416891bd2ec) )
	ROM_LOAD( "370gg_2.bin", 0x2000, 0x002000, CRC(2c31bfb1) SHA1(9b95be6839a25c906d4ea9cea70bb641ecaf77e5) )
	ROM_LOAD( "370gg_3.bin", 0x4000, 0x002000, CRC(f6a6dd6e) SHA1(adaae3a15d03e41f192bea33b3d2acca0488b0c6) )
	ROM_LOAD( "370gg_4.bin", 0x6000, 0x002000, CRC(0af03fb2) SHA1(2966f5954635d96287a9bca8ea33bd0b55ad51ce) )
ROM_END



ROM_START( ac1hotpf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ace_hot_profit_systemone.bin", 0x0000, 0x008000, CRC(951a750d) SHA1(feff32617321c5403f8e38a3aca1b49d065d5616) )
ROM_END


ROM_START( ac1pster )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pstrling.bin", 0x0000, 0x008000, CRC(ae46e199) SHA1(4cbe1205fa22e54b730f1fa5c01151368f35ed5f) )
ROM_END

ROM_START( ac1pstrt )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pst.bin", 0x0000, 0x008000, CRC(8e2ee921) SHA1(100de5ab0420d6c0196d90da4412a7d2c24a0912) )
ROM_END


ROM_START( ac1primt )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "403pt_p21.bin", 0x0000, 0x008000, CRC(83a87319) SHA1(e51cd1a3e5261140c9390c52ed7ae81b097ed2e8) )

	ROM_REGION( 0x80000, "alt", 0 ) // same thing but in smaller roms? should they all really be like this?
	ROM_LOAD( "403ptp21.bin", 0x0000, 0x002000, CRC(5437973c) SHA1(cd42fe09a75ea8bf8efd25c1ca7b12c4db029f31) )
	ROM_LOAD( "403ptp22.bin", 0x2000, 0x002000, CRC(a0a800a1) SHA1(348726be2e0161f2bfe63ca80e5193609c4f4211) )
	ROM_LOAD( "403ptp23.bin", 0x4000, 0x002000, CRC(69b6df2a) SHA1(2ecfce178b4fa22e2b8a3855171cf7e06ac0dc6d) )
	ROM_LOAD( "403ptp24.bin", 0x6000, 0x002000, CRC(12578388) SHA1(7e16dad8bc19c34c23f7fa3e627a1c85f669a19e) )
ROM_END


ROM_START( ac1taklv )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "430tl.bin", 0x0000, 0x008000, CRC(2fabb08f) SHA1(b737930e428f9258ab22394229c2b5039edf8f97) )

	ROM_REGION( 0x80000, "alt", 0 ) // same thing but in smaller roms? should they all really be like this?
	ROM_LOAD( "430tlp11.bin", 0x0000, 0x002000, CRC(32241ccd) SHA1(5aa46b0f45ab92ad8d1b9d500a6f8416888e4094) )
	ROM_LOAD( "430tlp12.bin", 0x2000, 0x002000, CRC(017479b6) SHA1(6ea72c1cd1866b6469eef51a841ca12720af0121) )
	ROM_LOAD( "430tlp13.bin", 0x4000, 0x002000, CRC(c70082c2) SHA1(2b7e901a6eb31871f83d835288025d256775c11b) )
	ROM_LOAD( "430tlp14.bin", 0x6000, 0x002000, CRC(09008e12) SHA1(f3f6dd3bafdcf7187148fed914d7c43caf53d48a) )
ROM_END


ROM_START( ac1bbclb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bb587p7a", 0x0000, 0x002000, CRC(906142a6) SHA1(395bfa7277a7953d47eb77a6689964e79e075b9c) )
	ROM_LOAD( "bb587p7c", 0x2000, 0x002000, CRC(53b462e4) SHA1(ead45992ab8d32ff931d6a262696395e03af0181) )
	ROM_LOAD( "bb587p7d", 0x4000, 0x002000, CRC(daff7083) SHA1(9a5dd2153b3e3b3c2b4edf0a52a9c07504961b4a) )
	ROM_LOAD( "bb587p7e", 0x6000, 0x002000, CRC(edf56e20) SHA1(7790851d8ddf599694846b4ddcfe2669a0ef05cc) )

	ROM_REGION( 0x80000, "alt", 0 )
	ROM_LOAD( "bb587p7b", 0x0000, 0x002000, CRC(e229c32d) SHA1(945f9492f642979009fa0dd309a5c1b3e43a671b) ) // alt rom at 0x0000
ROM_END


ROM_START( ac1clbsv )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "411cs_1.bin", 0x0000, 0x002000, CRC(0f1df1ea) SHA1(2d908838de301c62c4e334626d8ada5ad544d518) )
	ROM_LOAD( "411cs_2.bin", 0x2000, 0x002000, CRC(9bfd03d3) SHA1(ee83783bfaeca685427d5d674148df49af2b4647) )
	ROM_LOAD( "411cs_3.bin", 0x4000, 0x002000, CRC(a105c0c5) SHA1(0e9de822e8e6707a00f321e9664d50c117713abf) )
	ROM_LOAD( "411cs_4.bin", 0x6000, 0x002000, CRC(750a1ab9) SHA1(8c6e7aa3526ee807ce4edc4b64a9c21de67d7985) )
ROM_END



ROM_START( ac1clbxt )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "xt545p2c", 0x0000, 0x002000, CRC(78d7335a) SHA1(d06a67c7074f264940314de089341df0de73f3cc) )
	ROM_LOAD( "xt545p2e", 0x2000, 0x002000, CRC(97681816) SHA1(e456420b88676b32721921dc9e43279b006e66ec) )
	ROM_LOAD( "xt545p2f", 0x4000, 0x002000, CRC(cc7d9052) SHA1(d64ad5106adaaeb5665462c3ec8a2985693ab5ff) )
	ROM_LOAD( "xt545p2g", 0x6000, 0x002000, CRC(af26cdd8) SHA1(c4011f54bf669a4ac8e04691303c9d593399f9e5) )

	ROM_REGION( 0x80000, "alt", 0 )
	ROM_LOAD( "xt545p2d", 0x0000, 0x002000, CRC(a7fe418d) SHA1(96c1dfb878ed0ca585fdfc5555255f334d8b5f99) ) // alt rom at 0x0000
ROM_END




ROM_START( ac1piaca )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pia casino 5p 5-1.bin", 0x0000, 0x002000, CRC(216b0719) SHA1(11a1a04edd981922e2a6756709168c33e1d437c7) ) // aka pd566p5a
	ROM_LOAD( "pia casino 5p 5-2.bin", 0x2000, 0x002000, CRC(5baad962) SHA1(3afd8e0f76e82cd7fd028d9a627a9ca5a7704bd7) ) // aka pd566p5b
	ROM_LOAD( "pia casino 5p 5-3.bin", 0x4000, 0x002000, CRC(d6670a6a) SHA1(30de7ba1534351ec96c803530fcdacc234ee7454) ) // aka pd566p5c
	ROM_LOAD( "pia casino 5p 5-4.bin", 0x6000, 0x002000, CRC(7987db2d) SHA1(ee6ed7617f64faaa12182a8f12a96adb8b0ce32d) ) // aka pd566p5d
ROM_END



ROM_START( ac1piacl )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cpia7_1.bin", 0x0000, 0x002000, CRC(b8f4e481) SHA1(296aebd8ead9d87b050e6af4535eb81b7dc0f574) )
	ROM_LOAD( "cpia7_2.bin", 0x2000, 0x002000, CRC(09520caa) SHA1(cb2f0f88e47a2bb5d23817c1c757925ef4278ec9) )
	ROM_LOAD( "cpia7_3.bin", 0x4000, 0x002000, CRC(8b606921) SHA1(2309536ce500f8a42ef2eb2e634211875f0c97fe) )
	ROM_LOAD( "cpia7_4.bin", 0x6000, 0x002000, CRC(20e403a0) SHA1(6703459d3354ee6069e10c60498943912316cd62) )

	ROM_REGION( 0x80000, "alt", 0 ) // merged files
	ROM_LOAD( "cp513p7a", 0x0000, 0x008000, CRC(89b7d808) SHA1(26ad587e5d5f788c2c4e075e27c385ff67453c53) )
	ROM_LOAD( "cp513p7b", 0x0000, 0x008000, CRC(05c5241b) SHA1(eeb2bcda1a3a203c749667fb374c5d32ace585c7) )
	ROM_LOAD( "cpia7.bin", 0x0000, 0x008000, CRC(718b10f0) SHA1(0fe8ee1d14e5f22d27057f53ef2a2690cf02cab2) )
ROM_END





ROM_START( ac1prmcl )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pr535p7a", 0x0000, 0x002000, CRC(52167b25) SHA1(59081de590a72adbc826c16067920d145d5cf1ae) )
	ROM_LOAD( "pr535p7e", 0x2000, 0x002000, CRC(3cc34b9a) SHA1(597c75814593ec60cfb840d98dcd76df68815245) )
	ROM_LOAD( "pr535p7f", 0x4000, 0x002000, CRC(0346857f) SHA1(58cf5a9bcb2f4e51977c0c7ec732a5458f5dac5d) )
	ROM_LOAD( "pr535p7g", 0x6000, 0x002000, CRC(848888e3) SHA1(c7d1ed2153981cf45c68bb58b0220d0238edca49) )

	ROM_REGION( 0x80000, "alt", 0 )
	ROM_LOAD( "pr535p7b", 0x0000, 0x002000, CRC(b656cd89) SHA1(a26cb18ad23d25d0d631244f3a7d32914ba08750) ) // alt rom at 0x0000
	ROM_LOAD( "pr535p7c", 0x0000, 0x002000, CRC(c01f02ea) SHA1(e879c0a62229f8adb3ae389c07d27b3f568f87e3) ) // alt rom at 0x0000
	ROM_LOAD( "pr535p7d", 0x0000, 0x002000, CRC(245fb446) SHA1(6152e89df3707fca3e576f238420f9790ab47cff) ) // alt rom at 0x0000
ROM_END




ROM_START( ac1rundx )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "rd560d3b", 0x0000, 0x008000, CRC(e86b735d) SHA1(10ed7ffd60bfd8218007b8a2a5ebacbc5b241aaa) )

	ROM_REGION( 0x80000, "alt", 0 )
	ROM_LOAD( "rd560d3d", 0x0000, 0x008000, CRC(a8ec49ab) SHA1(503e7b0a5404f0ea06b95cdce372c7cb01b2a309) )
ROM_END


ROM_START( ac1totb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tbp3.1", 0x0000, 0x002000, CRC(abf58311) SHA1(26e2b9d8041f048050e23a8c1e6f6a7ac5660597) )
	ROM_LOAD( "tbp3.2", 0x2000, 0x002000, CRC(04d04f0a) SHA1(d6cdbc6306b956de47c599b1c2c3df529d372538) )
	ROM_LOAD( "tbp3.3", 0x4000, 0x002000, CRC(c5c669d3) SHA1(a54014f2f50fc7dda8cf7a8b40f9451b3fe317ae) )
	ROM_LOAD( "tbp3.4", 0x6000, 0x002000, CRC(f2d31ff4) SHA1(445d21ab0d413c23e0ef7bc00f940cdff5142cdd) )
ROM_END

ROM_START( ac1sptb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "simplythebest091a20pn.bin", 0x0000, 0x010000, CRC(8402d11f) SHA1(bc10f29c546fda03e18238811956c56546fa8bef) )
ROM_END

ROM_START( ac1shid )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "sh551d4a", 0x0000, 0x010000, CRC(5cf109df) SHA1(dd52ed897417cf4eb7b0ba60c8f0a6692c5f5e76) ) // overdump?

	ROM_REGION( 0x80000, "alt", 0 )
	ROM_LOAD( "sh564p4a", 0x0000, 0x008000, CRC(6e6fe7d8) SHA1(7bd21eabcef5c5c74f6d20c9b43dd0e975bb958f) )
ROM_END


DRIVER_INIT( aces1 )
{
}

GAME( 199?, ac1cshtw		,0			,aces1	,aces1	,aces1	,ROT0	,"Ace", "Cash Towers (Ace) (ACESYS1)",GAME_IS_SKELETON_MECHANICAL )
GAME( 199?, ac1clbmn		,0			,aces1	,aces1	,aces1	,ROT0	,"Ace", "Club Money (Ace) (ACESYS1)",GAME_IS_SKELETON_MECHANICAL )
GAME( 199?, ac1gogld		,0			,aces1	,aces1	,aces1	,ROT0	,"Ace", "Go For Gold (Ace) (ACESYS1)",GAME_IS_SKELETON_MECHANICAL )
GAME( 199?, ac1hotpf		,0			,aces1	,aces1	,aces1	,ROT0	,"Ace", "Hot Profit (Ace) (ACESYS1)",GAME_IS_SKELETON_MECHANICAL )
GAME( 199?, ac1pster		,0			,aces1	,aces1	,aces1	,ROT0	,"Ace", "Pound Sterling (Ace) (ACESYS1)",GAME_IS_SKELETON_MECHANICAL )
GAME( 199?, ac1pstrt		,0			,aces1	,aces1	,aces1	,ROT0	,"Pcp", "Pound Stretcher (Pcp) (ACESYS1)",GAME_IS_SKELETON_MECHANICAL )
GAME( 199?, ac1primt		,0			,aces1	,aces1	,aces1	,ROT0	,"Ace", "Primetime (Ace) (ACESYS1)",GAME_IS_SKELETON_MECHANICAL )
GAME( 199?, ac1taklv		,0			,aces1	,aces1	,aces1	,ROT0	,"Ace", "Take It Or Leave It (Ace) (ACESYS1)",GAME_IS_SKELETON_MECHANICAL )
// guessed hw
GAME( 199?, ac1bbclb		,0			,aces1	,aces1	,aces1	,ROT0	,"Ace", "Big Break Club (Ace) (ACESYS1)",GAME_IS_SKELETON_MECHANICAL )
GAME( 199?, ac1clbsv		,0			,aces1	,aces1	,aces1	,ROT0	,"Ace", "Club Sovereign (Ace) (ACESYS1)",GAME_IS_SKELETON_MECHANICAL )
GAME( 199?, ac1clbxt		,0			,aces1	,aces1	,aces1	,ROT0	,"Ace", "Club Xtra (Ace) (ACESYS1)",GAME_IS_SKELETON_MECHANICAL )
GAME( 199?, ac1piaca		,0			,aces1	,aces1	,aces1	,ROT0	,"Ace", "Play It Again Casino (Ace) (ACESYS1)",GAME_IS_SKELETON_MECHANICAL ) // Same ROMs were in 'Play It Again Deluxe'
GAME( 199?, ac1piacl		,0			,aces1	,aces1	,aces1	,ROT0	,"Ace", "Play It Again Club (Ace) (ACESYS1)",GAME_IS_SKELETON_MECHANICAL )
GAME( 199?, ac1prmcl		,0			,aces1	,aces1	,aces1	,ROT0	,"Ace", "Premier Club (Ace) (ACESYS1)",GAME_IS_SKELETON_MECHANICAL )
GAME( 199?, ac1rundx		,0			,aces1	,aces1	,aces1	,ROT0	,"Ace", "Runner Deluxe Club (Ace) (ACESYS1)",GAME_IS_SKELETON_MECHANICAL )
GAME( 199?, ac1totb			,0			,aces1	,aces1	,aces1	,ROT0	,"Ace", "Top Of The Bill (Ace) (ACESYS1)",GAME_IS_SKELETON_MECHANICAL )
GAME( 199?, ac1sptb			,0			,aces1	,aces1	,aces1	,ROT0	,"Pcp", "Simply the Best (Pcp) (ACESYS1)",GAME_IS_SKELETON_MECHANICAL )
GAME( 199?, ac1shid			,0			,aces1	,aces1	,aces1	,ROT0	,"Ace", "Super Hi De Hi (Ace) (ACESYS1)",GAME_IS_SKELETON_MECHANICAL )
