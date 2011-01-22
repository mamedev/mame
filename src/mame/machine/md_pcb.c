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
	{"SEGA-EEPROM", SEGA_EEPROM},
	{"SEGA-SRAM", SEGA_SRAM},
	{"SEGA-FRAM", SEGA_FRAM},

	{"CM-JCART", CM_JCART},
	{"CM-JCART-SEPROM", CM_JCART_SEPROM},
	{"CM-SEPROM", CODE_MASTERS},

	{"SSF2", SSF2},
	{"NBAJAM", NBA_JAM},
	{"NBAJAMTE", NBA_JAM_TE},
	{"NFLQB96", NFL_QB_96},
	{"CSLAM", C_SLAM},
	{"NHLPA", EA_NHLPA},
	{"WBOY5", WBOY_V},

	{"LIONK3", LIONK3},
	{"SDK99", SDK99},
	{"SKINGKONG", SKINGKONG},
	{"REDCLIFF", REDCL_EN},
	{"RADICA", RADICA},
	{"KOF98", KOF98},
	{"KOF99", KOF99},
	{"SOULBLAD", SOULBLAD},
	{"MJLOVER", MJLOVER},
	{"SQUIRRELK", SQUIRRELK},
	{"SMOUSE", SMOUSE},
	{"SMB", SMB},
	{"SMB2", SMB2},
	{"KAIJU", KAIJU},
	{"CHINFIGHT3", CHINFIGHT3},
	{"LIONK2", LIONK2},
	{"BUGSLIFE", BUGSLIFE},
	{"ELFWOR", ELFWOR},
	{"ROCKMANX3", ROCKMANX3},
	{"SBUBBOB", SBUBBOB},
	{"REALTEC", REALTEC},
	{"MC_SUP19IN1", MC_SUP19IN1},
	{"MC_SUP15IN1", MC_SUP15IN1},
	{"MC_12IN1", MC_12IN1},
	{"TOPFIGHTER", TOPFIGHTER},
	{"POKEMON", POKEMON},
	{"POKEMON2", POKEMON},
	{"MULAN", MULAN}
};

int md_get_pcb_id(const char *pcb)
{
	int	i;

	for (i = 0; i < ARRAY_LENGTH(pcb_list); i++)
	{
		if (!mame_stricmp(pcb_list[i].pcb_name, pcb))
			return pcb_list[i].pcb_id;
	}

	return SEGA_STD;
}
