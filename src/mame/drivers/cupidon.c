/* Cupidon - Russian Fruit Machines? */


#include "emu.h"
#include "cpu/m68000/m68000.h"

class cupidon_state : public driver_device
{
public:
	cupidon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

};


static ADDRESS_MAP_START( cupidon_map, AS_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x7fffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START(  cupidon )
INPUT_PORTS_END

static MACHINE_CONFIG_START( cupidon, cupidon_state )
	MCFG_CPU_ADD("maincpu", M68000, 16000000)	 // What CPU?
	MCFG_CPU_PROGRAM_MAP(cupidon_map)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	/* unknown sound */
MACHINE_CONFIG_END




ROM_START( tsarevna )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ts_1_29_u2_32m.bin", 0x000000, 0x400000, CRC(e7798a5d) SHA1(5ad876a693c93df79ea5e5672c0a5f3952b2cb36) )
	ROM_LOAD( "ts_1_29_u1_32m.bin", 0x400000, 0x400000, CRC(5a35ca2a) SHA1(b7beac148190b508469f832d370af082f479527c) )
ROM_END

ROM_START( tsarevnaa )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "v0130-2.bin", 0x000000, 0x400000, CRC(36349e13) SHA1(d82c93b7f19e8b75b0d56653aaaf5da44bb302f5) )
	ROM_LOAD( "v0130-1.bin", 0x400000, 0x400000, CRC(f502e677) SHA1(84f89f214aeff8544d526c44634672d972714bf6) )
ROM_END

	

DRIVER_INIT( cupidon )
{

}


GAME( 200?, tsarevna		,0,			cupidon, cupidon, cupidon, ROT0, "Cupidon","Carevna / Tsarevna (v1.29)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 200?, tsarevnaa		,tsarevna,	cupidon, cupidon, cupidon, ROT0, "Cupidon","Carevna / Tsarevna (v1.30)", GAME_NOT_WORKING|GAME_NO_SOUND )
