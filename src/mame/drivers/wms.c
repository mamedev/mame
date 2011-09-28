/* 

WMS Russian Fruit Machines (Mechanical?)

x86 based

*/

#include "emu.h"
#include "cpu/i386/i386.h"

class wms_state : public driver_device
{
public:
	wms_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

};

static ADDRESS_MAP_START( wms_map, AS_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x000fffff) AM_ROM AM_REGION("maincpu", 0 )

	AM_RANGE(0xfff00000, 0xffffffff) AM_ROM AM_REGION("maincpu", 0 )
ADDRESS_MAP_END

static ADDRESS_MAP_START( wms_io, AS_IO, 32 )
ADDRESS_MAP_END


static INPUT_PORTS_START( wms )
INPUT_PORTS_END



static MACHINE_CONFIG_START( wms, wms_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I486, 40000000 ) // ??
	MCFG_CPU_PROGRAM_MAP(wms_map)
	MCFG_CPU_IO_MAP(wms_io)
MACHINE_CONFIG_END

ROM_START( wms )
	ROM_REGION32_LE(0x100000, "maincpu", 0)
	ROM_LOAD( "10.bin",      0x0e0000, 0x020000, CRC(cf901f7d) SHA1(009a28fede06d2ff7f476ff643bf27cddd2adbab) )
	ROM_REGION(0x100000, "rom", ROMREGION_ERASE00)
ROM_END

ROM_START( wmsa )
	ROM_REGION32_LE(0x100000, "maincpu", 0)
	ROM_LOAD( "10cver4.010", 0x0e0000, 0x020000, CRC(fd310b97) SHA1(5745549258a1cefec4b3dddbe9d9a0d6281278e9) )
	ROM_REGION(0x100000, "rom", ROMREGION_ERASE00)
ROM_END


ROM_START( wmsb )
	ROM_REGION32_LE(0x100000, "maincpu", 0)
	ROM_LOAD( "50cver4.010", 0x0e0000, 0x020000, CRC(eeeeab29) SHA1(898c05c0674a9978caaad4a0fe3650a9d9a56715) )
	ROM_REGION(0x100000, "rom", ROMREGION_ERASE00)
ROM_END


ROM_START( btippers )
	ROM_REGION32_LE(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(96e362e3) SHA1(a0c35e9aa6bcbc5ffbf8750fa728294ef1e21b02) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(5468a57c) SHA1(3cb87c288bef1782b086a9d6d17f5c3a04aca3c8) )
	ROM_LOAD( "xu-4.bin", 0x0000, 0x100000, CRC(460ce5b6) SHA1(a4e22fff508db1e36e30ce0ec2c4fefaee67dcfc) )
	ROM_LOAD( "xu-5.bin", 0x0000, 0x100000, CRC(442ed657) SHA1(e4d33c85c22c44908a016521af53fc234a836b63) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(d4f533a9) SHA1(5ec53fed535fe6409481f99561c13e1fb98385ed) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(c845e18a) SHA1(3e20fbf6ac127a780a7a1517347e3cf7e951e5eb) )
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(2d7a9a0e) SHA1(0ab5752ca3bf360180caec219b7bfd478bb09cf4) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(5d767b66) SHA1(fb0866408657db540b85641ad5624885d7ef58ef) )
ROM_END



static DRIVER_INIT(wms)
{

}

GAME( 200?, wms,	    0,		 wms, wms, wms, ROT0, "WMS", "WMS SetUp/Clear Chips (set 1)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 200?, wmsa,	    wms,	 wms, wms, wms, ROT0, "WMS", "WMS SetUp/Clear Chips (set 2)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 200?, wmsb,	    wms,	 wms, wms, wms, ROT0, "WMS", "WMS SetUp/Clear Chips (set 3)", GAME_NOT_WORKING|GAME_NO_SOUND )

GAME( 200?, btippers,   0,		 wms, wms, wms, ROT0, "WMS", "B Tippers (Russia)", GAME_NOT_WORKING|GAME_NO_SOUND )
