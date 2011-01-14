/*****************************************************************************************

    Genesis/MegaDrive Cart PCBs Emulation

****************************************************************************************/

#include "emu.h"
#include "md_pcb.h"

typedef struct _md_pcb  md_pcb;
struct _md_pcb
{
	const char              *pcb_name;
	int                     pcb_id;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const md_pcb pcb_list[] =
{
	/* Standard Sega PCB (one rom) */
	{"171-5703", STD_ROM},
	{"171-5927", STD_ROM},
	{"171-5978B", STD_ROM},
	{"171-5978BA", STD_ROM},
	{"171-6329A", STD_ROM},

	/* Sega PCB with two roms */
	{"171-5841", STD_ROM},

	/* Sega PCB with Serial EEPROM */
	{"171-5878", SEGA_5878},
	{"171-6584A", SEGA_6584A},

	/* Sega PCB with sram */
	{"171-5921", SEGA_5921},
	{"171-6278A", SEGA_6278A},

	/* Sega PCB with fram */
	{"171-6658A", SEGA_6658A},

	/* Namcot PCB (blob epoxy chip) */
	{"837-8861", STD_ROM},

	/* Codemasters PCB */
	{"SRJCV1-2", CM_JCART},
	{"SRJCV2-1", CM_JCART_SEPROM},
	{"SRJCV2-2", CM_JCART_SEPROM}
};

int md_get_pcb_id(const char *pcb)
{
	int	i;

	for (i = 0; i < ARRAY_LENGTH(pcb_list); i++)
	{
		if (!mame_stricmp(pcb_list[i].pcb_name, pcb))
			return pcb_list[i].pcb_id;
	}

	return STD_ROM;
}
