/************************************************************************************************************

	Maygay EPOCH hardware
	 mechanical fruit machine HW

	H8 CPU

	ZYMZ280B sound
	
	2x PICs for security


	--------------------------------------------
	MOST GAMES DO NOT HAVE ANY SOUND ROMS DUMPED
	--------------------------------------------

************************************************************************************************************/

#include "emu.h"
#include "cpu/h83002/h8.h"
#include "sound/ymz280b.h"


class maygayep_state : public driver_device
{
public:
	maygayep_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }


};


static ADDRESS_MAP_START( ep_simp_map, AS_PROGRAM, 16 )
	ADDRESS_MAP_GLOBAL_MASK(0xffffff)
	AM_RANGE( 0x000000, 0x07ffff ) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static INPUT_PORTS_START( maygayep )
INPUT_PORTS_END


static void irqhandler(device_t *device, int state)
{

}

static const ymz280b_interface ymz280b_config =
{
	irqhandler
};


static MACHINE_CONFIG_START( maygayep, maygayep_state )
	MCFG_CPU_ADD("maincpu", H83044, 10000000 )
	MCFG_CPU_PROGRAM_MAP( ep_simp_map )
//	MCFG_CPU_VBLANK_INT( "screen", irq0_line_hold )

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymz", YMZ280B, 10000000 )
	MCFG_SOUND_CONFIG(ymz280b_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

#define MISSING_SOUND \
	ROM_REGION( 0x100000, "ymz", ROMREGION_ERASE00 ) \
	ROM_LOAD( "sound_roms", 0x000000, 0x080000, NO_DUMP ) \


#define EP_SIMP_SOUND \
	ROM_REGION( 0x100000, "ymz", 0 ) \
	ROM_LOAD( "simpsnd0", 0x000000, 0x080000, CRC(d58d16cc) SHA1(75eb2ab106855156831399a08eed66295c0c288f) ) \
	ROM_LOAD( "simpsnd1", 0x080000, 0x080000, CRC(26d12470) SHA1(4ea92b9d80c73d784534185313a6d5534cb6f3b2) ) \


ROM_START( ep_simp )
	ROM_REGION( 0x080000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sim256g0", 0x00000, 0x020000, CRC(8b5b266a) SHA1(358a98f1cd4fc65a4dfe7e9436eeca5f0649de15) )
	ROM_LOAD16_BYTE( "sim256g1", 0x00001, 0x020000, CRC(a4ec81bb) SHA1(bac8475def260f53e0fd25037752295ba04c88de) )
	EP_SIMP_SOUND
ROM_END


GAME( 1999, ep_simp,  0,        maygayep,  maygayep,  0,  ROT0, "Maygay",        "The Simpsons (Maygay) (EPOCH)",        GAME_NO_SOUND | GAME_NOT_WORKING | GAME_MECHANICAL )
