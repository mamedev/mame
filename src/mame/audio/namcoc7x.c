/*
    namcoc7x.c - sound hardware for mid-90s Namco systems

    System      MCU Synthesizer
    ------------------------------------
    NB-1        C351    C352
    NB-2        C351    C352
    System 22   C74 C352
    FL      C75 C352
    System 11   C76 C352

    All of these MCUs are Mitsubishi M377xx family (65c816 based)
    with 16k of internal BIOS ROM.  The BIOS is the same for all
    chips of the same number.  We currently use the BIOS from a
    Super System 22 game (Prop Cycle) to substitute for the
    BIOS in these chips.  This is compatible enough to play
    the sound and music, but not enough to read the inputs or
    handle coinage (the other task these MCUs have).

    These systems have 48k of shared RAM with the host.  Due to
    the MCU's work variables being moved around we cannot enable
    full sharing or else the main CPU can overwrite important
    MCU work variables and crash it.

    Emulation by R. Belmont.
    Thanks to Cync and Cap for invaluable assistance.
*/

#include "emu.h"
#include "cpu/m37710/m37710.h"
#include "deprecat.h"
#include "audio/namcoc7x.h"

static UINT16 *namcoc7x_mcuram;
static UINT16 su_82;
static UINT32 *namcoc7x_hostram;

static READ16_HANDLER( speedup_r )
{
	if ((cpu_get_pc(space->cpu) == 0xc12d) && (!(su_82 & 0xff00)))
	{
		cpu_spinuntil_int(space->cpu);
	}

	return su_82;
}

static WRITE16_HANDLER( speedup_w )
{
	COMBINE_DATA(&su_82);
}

WRITE16_HANDLER(namcoc7x_soundram16_w)
{
	COMBINE_DATA(namcoc7x_mcuram+offset);
}

READ16_HANDLER(namcoc7x_soundram16_r)
{
	return namcoc7x_mcuram[offset];
}

WRITE32_HANDLER(namcoc7x_soundram32_w)
{
	namcoc7x_soundram16_w(space, offset*2, data >> 16, mem_mask >> 16);
	namcoc7x_soundram16_w(space, offset*2+1, data, mem_mask);
}

READ32_HANDLER(namcoc7x_soundram32_r)
{
	return (namcoc7x_soundram16_r(space, offset*2, mem_mask >> 16) << 16) |
		namcoc7x_soundram16_r(space, offset*2+1, mem_mask);
}

void namcoc7x_sound_write16(UINT16 command, UINT32 offset)
{
	namcoc7x_mcuram[offset] = command;
}

void namcoc7x_on_driver_init(running_machine *machine)
{
	UINT8 *pROM = (UINT8 *)machine->region("c7x")->base();
	device_t *cpu;

	// clear the two 16-bits magic values at the start of the rom
	// (prevents external init routines from getting called - they assume a
	//  ROM layout for a different BIOS and crash ours)
	memset(pROM, 0, 4);

	// install speedup cheat
	for (cpu = machine->device("maincpu"); cpu != NULL; cpu = cpu->typenext())
		if (cpu->type() == M37702)
			memory_install_readwrite16_handler(cpu_get_address_space(cpu, ADDRESS_SPACE_PROGRAM), 0x82, 0x83, 0, 0, speedup_r, speedup_w);
}

void namcoc7x_set_host_ram(UINT32 *hostram)
{
	namcoc7x_hostram = hostram;
}

// Only share the sound work area
static READ16_HANDLER( c7x_shared_r )
{
	UINT16 *share16 = (UINT16 *)namcoc7x_hostram;

	if (offset >= 0x400/2)
	{
		return namcoc7x_mcuram[offset];
	}

	return share16[BYTE_XOR_LE(offset)];
}

static WRITE16_HANDLER( c7x_shared_w )
{
	UINT16 *share16 = (UINT16 *)namcoc7x_hostram;

	if (offset >= 0x400/2)
	{
		COMBINE_DATA(&namcoc7x_mcuram[offset]);
	}
	else
	{
		COMBINE_DATA(&share16[BYTE_XOR_LE(offset)]);
	}
}

ADDRESS_MAP_START( namcoc7x_mcu_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x002000, 0x002fff) AM_DEVREADWRITE("c352", c352_r, c352_w)
	AM_RANGE(0x004000, 0x00bfff) AM_RAM AM_BASE(&namcoc7x_mcuram)
	AM_RANGE(0x00c000, 0x00ffff) AM_ROM AM_REGION("c7x", 0x8c000)
	AM_RANGE(0x080000, 0x0fffff) AM_ROM AM_REGION("c7x", 0)
	AM_RANGE(0x200000, 0x27ffff) AM_ROM AM_REGION("c7x", 0)
	AM_RANGE(0x280000, 0x2fffff) AM_ROM AM_REGION("c7x", 0)
	AM_RANGE(0x301000, 0x301001) AM_NOP	// watchdog? LEDs?
	AM_RANGE(0x308000, 0x308003) AM_NOP	// volume control IC?
ADDRESS_MAP_END

ADDRESS_MAP_START( namcoc7x_mcu_share_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x002000, 0x002fff) AM_DEVREADWRITE("c352", c352_r, c352_w )
	AM_RANGE(0x004000, 0x00bfff) AM_READWRITE( c7x_shared_r, c7x_shared_w ) AM_BASE(&namcoc7x_mcuram)
	AM_RANGE(0x00c000, 0x00ffff) AM_ROM AM_REGION("c7x", 0x8c000)
	AM_RANGE(0x080000, 0x0fffff) AM_ROM AM_REGION("c7x", 0)
	AM_RANGE(0x200000, 0x27ffff) AM_ROM AM_REGION("c7x", 0)
	AM_RANGE(0x280000, 0x2fffff) AM_ROM AM_REGION("c7x", 0)
	AM_RANGE(0x301000, 0x301001) AM_NOP	// watchdog? LEDs?
	AM_RANGE(0x308000, 0x308003) AM_NOP	// volume control IC?
ADDRESS_MAP_END

static int p4;

static READ8_HANDLER( mcu_port5_r )
{
	return 0xff;
}

static WRITE8_HANDLER( mcu_port4_w )
{
	p4 = data;
}

static READ8_HANDLER( mcu_port4_r )
{
	return p4;
}

static WRITE8_HANDLER( mcu_port5_w )
{
}

static READ8_HANDLER( mcu_port6_r )
{
	return 0;
}

static READ8_HANDLER( mcu_port7_r )
{
	return 0;
}

ADDRESS_MAP_START( namcoc7x_mcu_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(M37710_PORT4, M37710_PORT4) AM_READ( mcu_port4_r ) AM_WRITE( mcu_port4_w )
	AM_RANGE(M37710_PORT5, M37710_PORT5) AM_READ( mcu_port5_r ) AM_WRITE( mcu_port5_w )
	AM_RANGE(M37710_PORT6, M37710_PORT6) AM_READ( mcu_port6_r ) AM_WRITENOP
	AM_RANGE(M37710_PORT7, M37710_PORT7) AM_READ( mcu_port7_r )
	AM_RANGE(0x10, 0x1f) AM_NOP
ADDRESS_MAP_END

INTERRUPT_GEN( namcoc7x_interrupt )
{
	if (cpu_getiloops(device) == 0)
		cpu_set_input_line(device, M37710_LINE_IRQ0, HOLD_LINE);
	else
		cpu_set_input_line(device, M37710_LINE_IRQ2, HOLD_LINE);
}

