// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

    Fidelity Electronics 68000 based board driver

******************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"

#include "includes/fidelz80.h"

// internal artwork
#include "fidel_eag.lh"


class fidel68k_state : public fidelz80base_state
{
public:
	fidel68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: fidelz80base_state(mconfig, type, tag)
	{ }

	// EAG
	//..
};



// Devices, I/O

/******************************************************************************
    EAG
******************************************************************************/



/******************************************************************************
    Address Maps
******************************************************************************/

// EAG

static ADDRESS_MAP_START( eag_map, AS_PROGRAM, 16, fidel68k_state )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
ADDRESS_MAP_END



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( eag )

INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

static MACHINE_CONFIG_START( eag, fidel68k_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(eag_map)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", fidelz80base_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_fidel_eag)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( feagv2 )
	ROM_REGION16_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("V2_6114_E5.bin", 0x00000, 0x10000, CRC(f9c7bada) SHA1(60e545f829121b9a4f1100d9e85ac83797715e80) )
	ROM_LOAD16_BYTE("V2_6114_O5.bin", 0x00001, 0x10000, CRC(04f97b22) SHA1(8b2845dd115498f7b385e8948eca6a5893c223d1) )
ROM_END


/******************************************************************************
    Drivers
******************************************************************************/

/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    INIT              COMPANY, FULLNAME, FLAGS */
COMP( 198?, feagv2,     0,      0,      eag,     eag,     driver_device, 0, "Fidelity Electronics", "Elite Avant Garde V2", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
