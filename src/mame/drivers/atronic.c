/* Atronic Russian Video Fruit Machines */
/*
 From 1999? (documentation is dated August 99)

*/

#include "emu.h"
#include "cpu/z80/z80.h"


class atronic_state : public driver_device
{
public:
	atronic_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

};




static ADDRESS_MAP_START( atronic_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( atronic_portmap, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END


static INPUT_PORTS_START( atronic )
INPUT_PORTS_END


static MACHINE_CONFIG_START( atronic, atronic_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 6000000)
	MCFG_CPU_PROGRAM_MAP(atronic_map)
	MCFG_CPU_IO_MAP(atronic_portmap)
MACHINE_CONFIG_END


ROM_START( atronic )
	ROM_REGION( 0x100000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "atronic u2.bin", 0x0000, 0x080000, CRC(ddcfa9ed) SHA1(008ffaf56ccdb3eb60fa5a0ad2f14d1988c2fa5a) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "atronic u6.bin", 0x0000, 0x020000, CRC(9742b2d8) SHA1(9f5851c78f92055730b834de18f8dc7bd9b29a37) )

	ROM_REGION( 0x800000, "u8u15", ROMREGION_ERASE00 ) // gfx
	ROM_REGION( 0x400000, "u18u21",ROMREGION_ERASE00 ) // sound
	ROM_REGION( 0x400000, "pals",ROMREGION_ERASE00 ) // pal (converted from JED)
ROM_END

ROM_START( atronica )
	ROM_REGION( 0x100000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "atronic u2.bin", 0x0000, 0x080000, CRC(ddcfa9ed) SHA1(008ffaf56ccdb3eb60fa5a0ad2f14d1988c2fa5a) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "atronic u6 std.bin", 0x0000, 0x020000, CRC(9ef7ae79) SHA1(3ed0ea056b23cee8829421c2369ff869b370ee80) )

	ROM_REGION( 0x800000, "u8u15", ROMREGION_ERASE00 ) // gfx
	ROM_REGION( 0x400000, "u18u21",ROMREGION_ERASE00 ) // sound
	ROM_REGION( 0x400000, "pals",ROMREGION_ERASE00 ) // pal (converted from JED)
ROM_END



ROM_START( atlantca )
	ROM_REGION( 0x100000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "u2.8 o-atla01-abaaa-ca-rus", 0x0000, 0x100000, CRC(c3f2aa47) SHA1(eda0088bfaea7a9a341dd63ae587c989742c6630) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "u6.1 atla01-a-zb-std-5-xx-xx-axx", 0x0000, 0x020000, CRC(5d09a4bf) SHA1(94aea5396a968ff659ac9e2f4879262c55eba2fe) )

	ROM_REGION( 0x800000, "u8u15", 0 ) // gfx
	ROM_LOAD( "u8.8 atla01-a-e-std-5",  0x000000, 0x100000, CRC(9e910565) SHA1(78e4e731d94b7e71db7cd9b15f9d0adfdd9f4e4f) )
	ROM_LOAD( "u9.8 atla01-a-e-std-5",  0x100000, 0x100000, CRC(7f8210fa) SHA1(f71faee0d606c6aa06287f6ea31f41727e2a22d9) )
	ROM_LOAD( "u10.8 atla01-a-e-std-5", 0x100000, 0x100000, CRC(e179fb20) SHA1(373f88ad001de48b415d9e2b2ca0b885c39080ac) )
	ROM_LOAD( "u11.8 atla01-a-e-std-5", 0x100000, 0x100000, CRC(af648717) SHA1(8ab57dc9962ed47a8beb03dcfc686c57de326793) )
	ROM_LOAD( "u12.8 atla01-a-e-std-5", 0x100000, 0x100000, CRC(6e37d906) SHA1(a5db448a5846f76ccf7b5297faddf310a1cc9fd6) )
	ROM_LOAD( "u13.8 atla01-a-e-std-5", 0x100000, 0x100000, CRC(6e89bf2b) SHA1(0c3346a5da6c67bf2ef38cf657860dccb03a0461) )
	ROM_LOAD( "u14.8 atla01-a-e-std-5", 0x100000, 0x100000, CRC(50188375) SHA1(511af1a61c5259d4ed99fa7cb26697bc802a5dc6) )
	ROM_LOAD( "u15.8 atla01-a-e-std-5", 0x100000, 0x100000, CRC(157a7615) SHA1(6fbb506c716e99781a73922c98dc9173c5d61353) )

	ROM_REGION( 0x400000, "u18u21", 0 ) // sound
	ROM_LOAD( "u18.8 atla01-aa-a-std", 0x000000, 0x100000, CRC(df8621e6) SHA1(4a91966a577dcc79d0b694482e7600ade1d4cbbc) )
	ROM_LOAD( "u19.8 atla01-aa-a-std", 0x100000, 0x100000, CRC(bb84f63d) SHA1(02990221a17657f2e46e0d42e2670158b4b0a7a6) )
	ROM_LOAD( "u20.8 atla01-aa-a-std", 0x200000, 0x100000, CRC(9f31c533) SHA1(eefa4d547aa5067b76686918564028593bb76c96) )
	ROM_LOAD( "u21.8 atla01-aa-a-std", 0x300000, 0x100000, CRC(a1bcd0a3) SHA1(0fd66c3bda92cead9457c35ce4b39f97293bb119) )

	ROM_REGION( 0x400000, "pals", 0 ) // pal (converted from JED)
	ROM_LOAD( "atlantica.bin", 0x0000, 0x0002dd, CRC(c3fdcd7d) SHA1(b56c859689e44689474142e537951c1cef40e46b) )	
ROM_END


GAME( 1999, atronic,    0,		  atronic, atronic,  0,             ROT0,  "Atronic", "Atronic SetUp/Clear Chips (Russia, set 1)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME( 1999, atronica,   atronic,  atronic, atronic,  0,             ROT0,  "Atronic", "Atronic SetUp/Clear Chips (Russia, set 2)", GAME_NOT_WORKING|GAME_NO_SOUND)

GAME( 2002, atlantca,    0,		  atronic, atronic,  0,             ROT0,  "Atronic", "Atlantica (Russia)", GAME_NOT_WORKING|GAME_NO_SOUND)


