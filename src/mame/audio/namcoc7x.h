/*
    namcoc7x.c - sound hardware for mid-90s Namco systems

    System      MCU Synthesizer
    ------------------------------------
    NB-1        C351    C352
    NB-2        C351    C352
    System 22   C74 C352
    FL      C75 C352
    System 11   C76 C352
*/

#include "sound/c352.h"
#include "cpu/m37710/m37710.h"

ADDRESS_MAP_EXTERN(namcoc7x_mcu_map, 16);
ADDRESS_MAP_EXTERN(namcoc7x_mcu_share_map, 16);
ADDRESS_MAP_EXTERN(namcoc7x_mcu_io, 8);

INTERRUPT_GEN( namcoc7x_interrupt );

WRITE16_HANDLER(namcoc7x_soundram16_w);
READ16_HANDLER(namcoc7x_soundram16_r);
WRITE32_HANDLER(namcoc7x_soundram32_w);
READ32_HANDLER(namcoc7x_soundram32_r);

void namcoc7x_sound_write16(UINT16 command, UINT32 offset);
void namcoc7x_on_driver_init(running_machine *machine);
void namcoc7x_set_host_ram(UINT32 *hostram);

/* BIOS from Prop Cycle used as a substitute until we can trojan the real BIOSes for these games */
#define NAMCO_C7X_BIOS	\
	ROM_LOAD( "pr1data.8k", 0x80000, 0x80000, BAD_DUMP CRC(2e5767a4) SHA1(390bf05c90044d841fe2dd4a427177fa1570b9a6) )

#define NAMCO_C7X_MCU(clock)	\
	MCFG_CPU_ADD("mcu", M37702, clock)	\
	MCFG_CPU_PROGRAM_MAP(namcoc7x_mcu_map)	\
	MCFG_CPU_IO_MAP(namcoc7x_mcu_io)	\
	MCFG_CPU_VBLANK_INT_HACK(namcoc7x_interrupt, 2)

#define NAMCO_C7X_MCU_SHARED(clock)	\
	MCFG_CPU_ADD("mcu", M37702, clock)	\
	MCFG_CPU_PROGRAM_MAP(namcoc7x_mcu_share_map)	\
	MCFG_CPU_IO_MAP(namcoc7x_mcu_io)	\
	MCFG_CPU_VBLANK_INT_HACK(namcoc7x_interrupt, 2)

#define NAMCO_C7X_SOUND(clock)	\
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")	\
	MCFG_SOUND_ADD("c352", C352, clock)	\
	MCFG_SOUND_ROUTE(0, "rspeaker", 1.00)	\
	MCFG_SOUND_ROUTE(1, "lspeaker", 1.00)	\
	MCFG_SOUND_ROUTE(2, "rspeaker", 1.00)	\
	MCFG_SOUND_ROUTE(3, "lspeaker", 1.00)

