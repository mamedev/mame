// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Curt Coder
/*

Omnibyte MSBC-1

PCB Layout
----------

REV B

|-----------------------------------------------------------|
|       CN1     CN2     CN3     CN4         SW1     CN5     |
|                                                           |
|           3.6864MHz   3.6864MHz   ROM1                    |
|                                   ROM0                    |
|       SIO         SIO     SW3 SW2                         |
|                               PIT                         |
|               12.5MHz                 41256 41256 41256   |
|                                       41256 41256 41256   |
|   8MHz                        CPU           41245 41256   |
|                                                           |
|                                               8419        |
|                           SW4                             |
|-|-------------CN6----------------|----|-------CN7-------|-|
  |--------------------------------|    |-----------------|

Notes:
    Relevant IC's shown.

    CPU     - Motorola MC68000R12
    SIO     - Mostek MK68564N-5A Serial Input/Output
    PIT     - Motorola MC68230L10 Parallel Interface/Timer
    8419    - National Semiconductor DP8419N DRAM Controller
    41256   - Fujitsu MB81256-12 256Kx1 RAM
    ROM0    - AMD AM27128ADC 16Kx8 ROM "47-2818-36G1"
    ROM1    - AMD AM27128ADC 16Kx8 ROM "47-2818-36G2"
    SW1     - push button
    SW2     - rotary hex DIP
    SW3     - rotary hex DIP
    SW4     - DIP8
    CN1     -
    CN2     -
    CN3     -
    CN4     -
    CN5     -
    CN6     -
    CN7     -

*/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/terminal.h"
#include "includes/msbc1.h"

#define TERMINAL_TAG "terminal"

static ADDRESS_MAP_START( msbc1_mem, AS_PROGRAM, 16, msbc1_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x03ffff) AM_RAM
	AM_RANGE(0xf80000, 0xf87fff) AM_ROM AM_REGION(MC68000R12_TAG, 0)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( msbc1 )
INPUT_PORTS_END

void msbc1_state::machine_reset()
{
	void *ram = m_maincpu->space(AS_PROGRAM).get_write_ptr(0);
	UINT8 *rom = memregion(MC68000R12_TAG)->base();

	memcpy(ram, rom, 8);

	m_maincpu->reset();
}

static MACHINE_CONFIG_START( msbc1, msbc1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(MC68000R12_TAG, M68000, XTAL_12_5MHz)
	MCFG_CPU_PROGRAM_MAP(msbc1_mem)

	// devices
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( msbc1 )
	ROM_REGION16_BE( 0x8000, MC68000R12_TAG, 0 )
	ROM_LOAD16_BYTE( "47-2818-36g1.u18", 0x0000, 0x4000, CRC(14f25d47) SHA1(964bc49c1dd9e9680c0d3d89ff3794c5044bea62) )
	ROM_LOAD16_BYTE( "47-2818-36g2.u19", 0x0001, 0x4000, CRC(4814b9e1) SHA1(d96cf75084c6588cb33513830c6beeeffc2de853) )

	ROM_REGION( 0x100, "plds", 0 )
	ROM_LOAD( "p21.1.u22", 0x000, 0x100, NO_DUMP )
	ROM_LOAD( "p22.0.u37", 0x000, 0x100, NO_DUMP )
	ROM_LOAD( "p23.0.u38", 0x000, 0x100, NO_DUMP )
	ROM_LOAD( "p24.0.u43", 0x000, 0x100, NO_DUMP )
	ROM_LOAD( "p25.0.u44", 0x000, 0x100, NO_DUMP )
	ROM_LOAD( "p26.0.u45", 0x000, 0x100, NO_DUMP )
	ROM_LOAD( "p27.0.u46", 0x000, 0x100, NO_DUMP )
	ROM_LOAD( "p28.1.u49", 0x000, 0x100, NO_DUMP )
	ROM_LOAD( "p29.0.u58", 0x000, 0x100, NO_DUMP )
	ROM_LOAD( "p30.0.u60", 0x000, 0x100, NO_DUMP )
	ROM_LOAD( "p31.0.u61", 0x000, 0x100, NO_DUMP )
	ROM_LOAD( "p32.0.u76", 0x000, 0x100, NO_DUMP )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1985, msbc1,  0,      0,       msbc1,     msbc1, driver_device,   0,    "Omnibyte", "MSBC-1", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
