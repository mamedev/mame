/*

    Scorpion 5

    Skeleton Driver - For note keeping.

    This system is not emulated.

*/


#include "emu.h"
#include "includes/bfm_sc5.h"


static ADDRESS_MAP_START( sc5_map, AS_PROGRAM, 32, bfm_sc5_state )
	AM_RANGE(0x00000000, 0x002fffff) AM_ROM
	AM_RANGE(0x01000000, 0x0100ffff) AM_RAM
	AM_RANGE(0x40000000, 0x40000fff) AM_RAM
ADDRESS_MAP_END

INPUT_PORTS_START( bfm_sc5 )
INPUT_PORTS_END

INTERRUPT_GEN_MEMBER(bfm_sc5_state::sc5_fake_timer_int)
{
	// this should be coming from the Timer / SIM modules of the Coldfire
	//machine().device("maincpu")->execute().set_input_line_and_vector(5, HOLD_LINE, 0x8c);
}

MACHINE_CONFIG_START( bfm_sc5, bfm_sc5_state )
	MCFG_CPU_ADD("maincpu", MCF5206E, 40000000) /* MCF5206eFT */
	MCFG_CPU_PROGRAM_MAP(sc5_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(bfm_sc5_state, sc5_fake_timer_int, 1000)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	/* unknown sound */
MACHINE_CONFIG_END
