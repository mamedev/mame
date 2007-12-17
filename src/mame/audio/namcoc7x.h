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

ADDRESS_MAP_EXTERN(namcoc7x_mcu_map);
ADDRESS_MAP_EXTERN(namcoc7x_mcu_share_map);
ADDRESS_MAP_EXTERN(namcoc7x_mcu_io);

INTERRUPT_GEN( namcoc7x_interrupt );

void namcoc7x_sound_write16(UINT16 command, UINT32 offset);
void namcoc7x_on_driver_init(void);
void namcoc7x_set_host_ram(UINT32 *hostram);

#define NAMCO_C7X_HARDWARE	\
static struct C352interface namcoc7x_c352_interface =	\
{	\
	REGION_SOUND1	\
};

/* BIOS from Prop Cycle used as a substitute until we can trojan the real BIOSes for these games */
#define NAMCO_C7X_BIOS	\
	ROM_LOAD( "pr1data.8k", 0x80000, 0x80000, BAD_DUMP CRC(2e5767a4) SHA1(390bf05c90044d841fe2dd4a427177fa1570b9a6) )

#define NAMCO_C7X_MCU(clock)	\
	MDRV_CPU_ADD_TAG("mcu", M37702, clock)	\
	MDRV_CPU_PROGRAM_MAP(namcoc7x_mcu_map, 0)	\
	MDRV_CPU_IO_MAP(namcoc7x_mcu_io, 0)	\
	MDRV_CPU_VBLANK_INT(namcoc7x_interrupt, 2)

#define NAMCO_C7X_MCU_SHARED(clock)	\
	MDRV_CPU_ADD_TAG("mcu", M37702, clock)	\
	MDRV_CPU_PROGRAM_MAP(namcoc7x_mcu_share_map, 0)	\
	MDRV_CPU_IO_MAP(namcoc7x_mcu_io, 0)	\
	MDRV_CPU_VBLANK_INT(namcoc7x_interrupt, 2)

#define NAMCO_C7X_SOUND(clock)	\
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")	\
	MDRV_SOUND_ADD(C352, clock)	\
	MDRV_SOUND_CONFIG(namcoc7x_c352_interface)	\
	MDRV_SOUND_ROUTE(0, "right", 1.00)	\
	MDRV_SOUND_ROUTE(1, "left", 1.00)	\
	MDRV_SOUND_ROUTE(2, "right", 1.00)	\
	MDRV_SOUND_ROUTE(3, "left", 1.00)

