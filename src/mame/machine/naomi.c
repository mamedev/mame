/***************************************************************************

Per-game specific JVS settings / idle loop skips for the MAME Naomi driver.

***************************************************************************/

#include "emu.h"
#include "includes/naomi.h"

UINT64 *naomi_ram64;
int jvsboard_type;

static READ64_HANDLER( naomi_bios_idle_skip_r )
{
	if (cpu_get_pc(space->cpu)==0xc04173c)
		cpu_spinuntil_time(space->cpu, ATTOTIME_IN_USEC(500));
		//cpu_spinuntil_int(space->cpu);
//  else
//      printf("%08x\n", cpu_get_pc(space->cpu));

	return naomi_ram64[0x2ad238/8];
}

DRIVER_INIT(naomi)
{
	memory_install_read64_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc2ad238, 0xc2ad23f, 0, 0, naomi_bios_idle_skip_r); // rev e bios
	jvsboard_type = JVSBD_DEFAULT;
}

DRIVER_INIT(naomi_mp)
{
	memory_install_read64_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc2ad238, 0xc2ad23f, 0, 0, naomi_bios_idle_skip_r); // rev e bios
	jvsboard_type = JVSBD_MAHJONG;
}

static READ64_HANDLER( naomigd_ggxxsla_idle_skip_r )
{
	if (cpu_get_pc(space->cpu)==0x0c0c9adc)
		cpu_spinuntil_time(space->cpu, ATTOTIME_IN_USEC(500));

	return naomi_ram64[0x1aae18/8];
}

DRIVER_INIT( ggxxsla )
{
	memory_install_read64_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc1aae18, 0xc1aae1f, 0, 0, naomigd_ggxxsla_idle_skip_r);
	DRIVER_INIT_CALL(naomi);
}

static READ64_HANDLER( naomigd_ggxx_idle_skip_r )
{
	if (cpu_get_pc(space->cpu)==0xc0b5c3c) // or 0xc0bab0c
		cpu_spinuntil_time(space->cpu, ATTOTIME_IN_USEC(500));

	return naomi_ram64[0x1837b8/8];
}


DRIVER_INIT( ggxx )
{
	memory_install_read64_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc1837b8, 0xc1837bf, 0, 0, naomigd_ggxx_idle_skip_r);
	DRIVER_INIT_CALL(naomi);
}

static READ64_HANDLER( naomigd_ggxxrl_idle_skip_r )
{
	if (cpu_get_pc(space->cpu)==0xc0b84bc) // or 0xc0bab0c
		cpu_spinuntil_time(space->cpu, ATTOTIME_IN_USEC(500));

	//printf("%08x\n", cpu_get_pc(space->cpu));

	return naomi_ram64[0x18d6c8/8];
}


DRIVER_INIT( ggxxrl )
{
	memory_install_read64_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc18d6c8, 0xc18d6cf, 0, 0, naomigd_ggxxrl_idle_skip_r);
	DRIVER_INIT_CALL(naomi);
}
