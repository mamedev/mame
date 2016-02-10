// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

    Adder 5
     (Scorpion 5 Video Board)

    Skeleton Driver - For note keeping.

    This system is not emulated.

*/

#include "emu.h"
#include "includes/bfm_ad5.h"
#include "machine/mcf5206e.h"


extern int find_project_string(running_machine &machine, int addrxor, int mode);


DRIVER_INIT_MEMBER(adder5_state,ad5)
{
	// sc5 roms always start with SC5
	UINT8 *src = memregion( "maincpu" )->base();
//  printf("%02x %02x %02x %02x\n", src[0], src[1], src[2], src[3]);
	if (((src[0] == 0x20) && (src[2] == 0x43)) || ((src[1] == 0x35) && (src[3] == 0x53)))
	{
		printf("Confirmed SC5 ROM\n");
	}
	else
	{
		printf("NOT AN SC5 ROM!!!!!\n");
	}


	// there is usually a string in the rom with identification info, often also saying which sound rom should be used!
	// find it.
	int found = find_project_string(machine(), 3, 0);
	if (!found)
	{
		printf("Normal rom pair string not found, checking mismatched / missing rom string\n");
	}

	// help identify roms where one of the pair is missing too
	if (!found)
	{
		found = find_project_string(machine(), 3, 1);
	}

	if (!found)
	{
		found = find_project_string(machine(), 3, 2);
	}

	if (!found)
	{
		printf("No suitable string found\n");
	}

}

static ADDRESS_MAP_START( ad5_map, AS_PROGRAM, 32, adder5_state )
	AM_RANGE(0x00000000, 0x00ffffff) AM_ROM
	AM_RANGE(0x01000000, 0x0100ffff) AM_RAM
	AM_RANGE(0x40000000, 0x40000fff) AM_RAM
	AM_RANGE(0x80000000, 0x8000ffff) AM_RAM
	AM_RANGE(0x80800000, 0x8080ffff) AM_RAM

	AM_RANGE(0xffff0000, 0xffff03ff) AM_DEVREADWRITE("maincpu_onboard", mcf5206e_peripheral_device, dev_r, dev_w) // technically this can be moved with MBAR
ADDRESS_MAP_END

INPUT_PORTS_START( bfm_ad5 )
INPUT_PORTS_END

INTERRUPT_GEN_MEMBER(adder5_state::ad5_fake_timer_int)
{
	// this should be coming from the Timer / SIM modules of the Coldfire
//  m_maincpu->set_input_line_and_vector(5, HOLD_LINE, 0x8c);
}

MACHINE_CONFIG_START( bfm_ad5, adder5_state )
	MCFG_CPU_ADD("maincpu", MCF5206E, 40000000) /* MCF5206eFT */
	MCFG_CPU_PROGRAM_MAP(ad5_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(adder5_state, ad5_fake_timer_int, 1000)
	MCFG_MCF5206E_PERIPHERAL_ADD("maincpu_onboard")

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	/* unknown sound */
MACHINE_CONFIG_END
