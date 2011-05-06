/*****************************************************************************************

    NES MMC Emulation - UNIF boards

    Very preliminary support for UNIF boards

    TODO:
    - properly support WRAM, etc.

****************************************************************************************/

typedef struct __unif
{
	const char *board; /* UNIF board */

	int nvwram;
	int wram;
	int chrram;
	int board_idx;
} unif;


/*************************************************************

    Constants

*************************************************************/

/* CHRRAM sizes */
enum
{
	CHRRAM_0 = 0,
	CHRRAM_1,
	CHRRAM_2,
	CHRRAM_4,
	CHRRAM_6,
	CHRRAM_8,
	CHRRAM_16,
	CHRRAM_32
};

/*************************************************************

    unif_list

    Supported UNIF boards and corresponding handlers

*************************************************************/

static const unif unif_list[] =
{
/*       UNIF                       NVW  WRAM  CRAM     IDX*/
	{ "DREAMTECH01",                0,    0, CHRRAM_8,  DREAMTECH_BOARD},		//UNIF only!
	{ "NES-ANROM",                  0,    0, CHRRAM_8,  STD_AXROM},
	{ "NES-AOROM",                  0,    0, CHRRAM_8,  STD_AXROM},
	{ "NES-CNROM",                  0,    0, CHRRAM_0,  STD_CNROM},
	{ "NES-NROM",                   0,    0, CHRRAM_0,  STD_NROM},
	{ "NES-NROM-128",               0,    0, CHRRAM_0,  STD_NROM},
	{ "NES-NROM-256",               0,    0, CHRRAM_0,  STD_NROM},
	{ "NES-NTBROM",                 8,    0, CHRRAM_0,  STD_NXROM},
	{ "NES-SLROM",                  0,    0, CHRRAM_0,  STD_SXROM},
	{ "NES-TBROM",                  0,    0, CHRRAM_0,  STD_TXROM},
	{ "NES-TFROM",                  0,    0, CHRRAM_0,  STD_TXROM},
	{ "NES-TKROM",                  8,    0, CHRRAM_0,  STD_TXROM},
	{ "NES-TLROM",                  0,    0, CHRRAM_0,  STD_TXROM},
	{ "NES-UOROM",                  0,    0, CHRRAM_8,  STD_UXROM},
	{ "UNL-22211",                  0,    0, CHRRAM_0,  TXC_22211A},
	// mapper 172 & 173 are variant of this one... no UNIF?
	{ "UNL-KOF97",                  0,    0, CHRRAM_0,  UNL_KOF97},
	{ "UNL-SA-NROM",                0,    0, CHRRAM_0,  SACHEN_TCA01},
	{ "UNL-VRC7",                   0,    0, CHRRAM_0,  KONAMI_VRC7},
	{ "UNL-T-230",                  0,    0, CHRRAM_8,  UNL_T230},
	{ "UNL-CC-21",                  0,    0, CHRRAM_0,  UNL_CC21},
	{ "UNL-AX5705",                 0,    0, CHRRAM_0,  UNL_AX5705},
	{ "UNL-SMB2J",                  8,    8, CHRRAM_0,  UNL_SMB2J},
	{ "UNL-8237",                   0,    0, CHRRAM_0,  UNL_8237},
	{ "UNL-SL1632",                 0,    0, CHRRAM_0,  REXSOFT_SL1632},
	{ "UNL-SACHEN-74LS374N",        0,    0, CHRRAM_0,  SACHEN_74LS374},
	// mapper 243 variant exists! how to distinguish?!?  mapper243_l_w, NULL, NULL, NULL, NULL, NULL, NULL (also uses NT_VERT!)
	{ "UNL-TC-U01-1.5M",            0,    0, CHRRAM_0,  SACHEN_TCU01},
	{ "UNL-SACHEN-8259C",           0,    0, CHRRAM_0,  SACHEN_8259C},
	{ "UNL-SA-016-1M",              0,    0, CHRRAM_0,  AVE_NINA06},	// actually this is Mapper 146, but works like 79!
	{ "UNL-SACHEN-8259D",           0,    0, CHRRAM_0,  SACHEN_8259D},
	{ "UNL-SA-72007",               0,    0, CHRRAM_0,  SACHEN_SA72007},
	{ "UNL-SA-72008",               0,    0, CHRRAM_0,  SACHEN_SA72008},
	{ "UNL-SA-0037",                0,    0, CHRRAM_0,  SACHEN_SA0037},
	{ "UNL-SA-0036",                0,    0, CHRRAM_0,  SACHEN_SA0036},
	{ "UNL-SACHEN-8259A",           0,    0, CHRRAM_0,  SACHEN_8259A},
	{ "UNL-SACHEN-8259B",           0,    0, CHRRAM_0,  SACHEN_8259B},
	{ "BMC-190IN1",                 0,    0, CHRRAM_0,  BMC_190IN1},
	{ "BMC-64IN1NOREPEAT",          0,    0, CHRRAM_0,  BMC_64IN1NR},		//UNIF only!
	{ "BMC-A65AS",                  0,    0, CHRRAM_8,  BMC_A65AS},		//UNIF only!
	{ "BMC-GS-2004",                0,    0, CHRRAM_8,  BMC_GS2004},		//UNIF only!
	{ "BMC-GS-2013",                0,    0, CHRRAM_8,  BMC_GS2013},		//UNIF only!
	{ "BMC-NOVELDIAMOND9999999IN1", 0,    0, CHRRAM_0,  BMC_NOVELDIAMOND},
	{ "BMC-SUPER24IN1SC03",         8,    0, CHRRAM_8,  BMC_S24IN1SC03},
	{ "BMC-SUPERHIK8IN1",           8,    0, CHRRAM_0,  BMC_HIK8IN1},
	{ "BMC-T-262",                  0,    0, CHRRAM_8,  BMC_T262},		//UNIF only!
	{ "BMC-WS",                     0,    0, CHRRAM_0,  BMC_WS},		//UNIF only!
	{ "BMC-N625092",                0,    0, CHRRAM_0,  UNL_N625092},
	// below are boards which are not yet supported, but are used by some UNIF files. they are here as a reminder to what is missing to be added
	{ "UNL-TEK90",                  0,    0, CHRRAM_0,  UNSUPPORTED_BOARD},	// related to JY Company? (i.e. mappers 90, 209, 211?)
	{ "UNL-KS7017",                 0,    0, CHRRAM_0,  UNSUPPORTED_BOARD},
	{ "UNL-KS7032",                 0,    0, CHRRAM_0,  KAISER_KS7032}, //  mapper 142
	{ "UNL-DANCE",                  0,    0, CHRRAM_0,  UNSUPPORTED_BOARD},
	{ "UNL-603-5052",               0,    0, CHRRAM_0,  UNL_603_5052}, // mapper 238?
	{ "UNL-EDU2000",               32,    0, CHRRAM_8,  UNL_EDU2K},
	{ "UNL-H2288",                  0,    0, CHRRAM_0,  UNL_H2288},	// mapper 123
	{ "UNL-SHERO",                  0,    0, CHRRAM_8,  UNSUPPORTED_BOARD /*SACHEN_SHERO*/},
	{ "UNL-TF1201",                 0,    0, CHRRAM_0,  UNSUPPORTED_BOARD /*UNL_TF1201*/},
	{ "UNL-DRIPGAME",               0,    0, CHRRAM_0,  UNSUPPORTED_BOARD}, // [by Quietust - we need more info]}
	{ "UNL-OneBus",                 0,    0, CHRRAM_0,  UNSUPPORTED_BOARD},
	{ "BTL-MARIO1-MALEE2",          0,    0, CHRRAM_0,  UNSUPPORTED_BOARD},	// mapper 55?
	{ "BMC-FK23C",                  0,    0, CHRRAM_0,  BMC_FK23C},
	{ "BMC-FK23CA",                 0,    0, CHRRAM_0,  BMC_FK23CA},
	{ "BMC-GHOSTBUSTERS63IN1",      0,    0, CHRRAM_8,  BMC_G63IN1 },
	{ "BMC-BS-5",                   0,    0, CHRRAM_0,  BMC_BENSHENG_BS5},
	{ "BMC-810544-C-A1",            0,    0, CHRRAM_0,  BMC_810544},
	{ "BMC-411120-C",               0,    0, CHRRAM_0,  UNSUPPORTED_BOARD},
	{ "BMC-8157",                   0,    0, CHRRAM_8,  UNSUPPORTED_BOARD /*BMC_8157*/},
	{ "BMC-42IN1RESETSWITCH",       0,    0, CHRRAM_0,  UNSUPPORTED_BOARD},	// mapper 60?
	{ "BMC-830118C",                0,    0, CHRRAM_0,  UNSUPPORTED_BOARD},
	{ "BMC-D1038",                  0,    0, CHRRAM_0,  BMC_VT5201}, // mapper 60?
	{ "BMC-12-IN-1",                0,    0, CHRRAM_0,  UNSUPPORTED_BOARD},
	{ "BMC-70IN1",                  0,    0, CHRRAM_0,  UNSUPPORTED_BOARD},	// mapper 236?
	{ "BMC-70IN1B",                 0,    0, CHRRAM_0,  UNSUPPORTED_BOARD},	// mapper 236?
	{ "BMC-SUPERVISION16IN1",       0,    0, CHRRAM_0,  UNSUPPORTED_BOARD},	// mapper 53
	{ "BMC-NTD-03",                 0,    0, CHRRAM_0,  BMC_NTD_03}
};

const unif *nes_unif_lookup( const char *board )
{
	int i;
	for (i = 0; i < ARRAY_LENGTH(unif_list); i++)
	{
		if (!mame_stricmp(unif_list[i].board, board))
			return &unif_list[i];
	}
	return NULL;
}

/*************************************************************

 unif_mapr_setup

 setup the board specific variables (wram, nvwram, pcb_id etc.)
 for a given board (after reading the MAPR chunk of the UNIF file)

 *************************************************************/

void unif_mapr_setup( running_machine &machine, const char *board )
{
	nes_state *state = machine.driver_data<nes_state>();
	const unif *unif_board = nes_unif_lookup(board);

	logerror("%s\n", board);

	if (unif_board == NULL)
		fatalerror("Unknown UNIF board %s.", board);

	state->m_pcb_id = unif_board->board_idx;
	state->m_battery = unif_board->nvwram;	// we should implement battery banks based on the size of this...
	state->m_battery_size = NES_BATTERY_SIZE; // FIXME: we should allow for smaller battery!
	state->m_prg_ram = unif_board->wram;	// we should implement WRAM banks based on the size of this...

	if (unif_board->chrram <= CHRRAM_8)
		state->m_vram_chunks = 1;
	else if (unif_board->chrram == CHRRAM_16)
		state->m_vram_chunks = 2;
	else if (unif_board->chrram == CHRRAM_32)
		state->m_vram_chunks = 4;
}
