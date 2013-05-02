/*

    Scorpion 5

    Skeleton Driver - For note keeping.

    This system is not emulated.

*/


#include "emu.h"
#include "includes/bfm_sc5.h"
#include "machine/mcf5206e.h"


static ADDRESS_MAP_START( sc5_map, AS_PROGRAM, 32, bfm_sc5_state )
	AM_RANGE(0x00000000, 0x002fffff) AM_ROM
	AM_RANGE(0x01000000, 0x0100ffff) AM_RAM
	AM_RANGE(0x40000000, 0x4000ffff) AM_RAM

	AM_RANGE(0xffff0000, 0xffff03ff) AM_DEVREADWRITE("maincpu_onboard", mcf5206e_peripheral_device, dev_r, dev_w) // technically this can be moved with MBAR
ADDRESS_MAP_END

INPUT_PORTS_START( bfm_sc5 )
INPUT_PORTS_END



WRITE_LINE_MEMBER(bfm_sc5_state::bfm_sc5_ym_irqhandler)
{
	logerror("YMZ280 is generating an interrupt. State=%08x\n",state);
}

static const ymz280b_interface ymz280b_config =
{
	DEVCB_DRIVER_LINE_MEMBER(bfm_sc5_state,bfm_sc5_ym_irqhandler)
};


MACHINE_CONFIG_START( bfm_sc5, bfm_sc5_state )
	MCFG_CPU_ADD("maincpu", MCF5206E, 40000000) /* MCF5206eFT */
	MCFG_CPU_PROGRAM_MAP(sc5_map)
	MCFG_MCF5206E_PERIPHERAL_ADD("maincpu_onboard")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymz", YMZ280B, 16000000) // ?? Mhz
	MCFG_SOUND_CONFIG(ymz280b_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END
