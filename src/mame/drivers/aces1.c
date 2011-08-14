/*
 Ace System 1 Hardware
 Fruit Machines

 skeleton driver!
*/

#include "emu.h"
#include "cpu/z80/z80.h"

class aces1_state : public driver_device
{
public:
	aces1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }
};



static ADDRESS_MAP_START( aces1_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( aces1_portmap, AS_IO, 8 )
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

DRIVER_INIT( aces1 )
{

}

GAME( 199?, ac1cshtw		,0			,aces1	,aces1	,aces1	,ROT0	,"Ace", "Cash Towers (Ace) (ACESYS1)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, ac1clbmn		,0			,aces1	,aces1	,aces1	,ROT0	,"Ace", "Club Money (Ace) (ACESYS1)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, ac1gogld		,0			,aces1	,aces1	,aces1	,ROT0	,"Ace", "Go For Gold (Ace) (ACESYS1)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, ac1hotpf		,0			,aces1	,aces1	,aces1	,ROT0	,"Ace", "Hot Profit (Ace) (ACESYS1)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, ac1pster		,0			,aces1	,aces1	,aces1	,ROT0	,"Ace", "Pound Sterling (Ace) (ACESYS1)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, ac1pstrt		,0			,aces1	,aces1	,aces1	,ROT0	,"Pcp", "Pound Stretcher (Pcp) (ACESYS1)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, ac1primt		,0			,aces1	,aces1	,aces1	,ROT0	,"Ace", "Primetime (Ace) (ACESYS1)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, ac1taklv		,0			,aces1	,aces1	,aces1	,ROT0	,"Ace", "Take It Or Leave It (Ace) (ACESYS1)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
