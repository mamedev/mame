// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/*****************************************************************************************

    NES MMC Emulation - UNIF boards

    Very preliminary support for UNIF boards

    TODO:
    - properly support WRAM, etc.

****************************************************************************************/


/* Set to generate prg & chr files when the cart is loaded */
#define SPLIT_PRG   0
#define SPLIT_CHR   0


/*************************************************************

 unif_list

 Supported UNIF boards and corresponding handlers

 *************************************************************/


struct unif
{
	const char *board; /* UNIF board */

	int nvwram;
	int wram;
	int chrram;
	int board_idx;
};


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

static const unif unif_list[] =
{
/*       UNIF                       NVW  WRAM  CRAM     IDX*/
	{ "DREAMTECH01",                0,    0, CHRRAM_8,  DREAMTECH_BOARD},       //UNIF only!
	{ "NES-ANROM",                  0,    0, CHRRAM_8,  STD_AXROM},
	{ "NES-AOROM",                  0,    0, CHRRAM_8,  STD_AXROM},
	{ "NES-CNROM",                  0,    0, CHRRAM_0,  STD_CNROM},
	{ "NES-NROM",                   0,    0, CHRRAM_0,  STD_NROM},
	{ "NES-NROM-128",               0,    0, CHRRAM_0,  STD_NROM},
	{ "NES-NROM-256",               0,    0, CHRRAM_0,  STD_NROM},
	{ "NES-NTBROM",                 8,    0, CHRRAM_0,  SUNSOFT_DCS},
	{ "NES-SLROM",                  0,    0, CHRRAM_0,  STD_SXROM},
	{ "NES-TBROM",                  0,    0, CHRRAM_0,  STD_TXROM},
	{ "NES-TFROM",                  0,    0, CHRRAM_0,  STD_TXROM},
	{ "NES-TKROM",                  8,    0, CHRRAM_0,  STD_TXROM},
	{ "NES-TLROM",                  0,    0, CHRRAM_0,  STD_TXROM},
	{ "NES-UOROM",                  0,    0, CHRRAM_8,  STD_UXROM},
	{ "UNL-22211",                  0,    0, CHRRAM_0,  TXC_22211},
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
	{ "UNL-SA-016-1M",              0,    0, CHRRAM_0,  AVE_NINA06},    // actually this is Mapper 146, but works like 79!
	{ "UNL-SACHEN-8259D",           0,    0, CHRRAM_0,  SACHEN_8259D},
	{ "UNL-SA-72007",               0,    0, CHRRAM_0,  SACHEN_SA72007},
	{ "UNL-SA-72008",               0,    0, CHRRAM_0,  SACHEN_SA72008},
	{ "UNL-SA-0037",                0,    0, CHRRAM_0,  SACHEN_SA0037},
	{ "UNL-SA-0036",                0,    0, CHRRAM_0,  SACHEN_SA0036},
	{ "UNL-SA-9602B",               0,    0, CHRRAM_0,  SACHEN_SA9602B},
	{ "UNL-SACHEN-8259A",           0,    0, CHRRAM_0,  SACHEN_8259A},
	{ "UNL-SACHEN-8259B",           0,    0, CHRRAM_0,  SACHEN_8259B},
	{ "BMC-190IN1",                 0,    0, CHRRAM_0,  BMC_190IN1},
	{ "BMC-64IN1NOREPEAT",          0,    0, CHRRAM_0,  BMC_64IN1NR},       //UNIF only!
	{ "BMC-A65AS",                  0,    0, CHRRAM_8,  BMC_A65AS},     //UNIF only!
	{ "BMC-GS-2004",                0,    0, CHRRAM_8,  RCM_GS2004},        //UNIF only!
	{ "BMC-GS-2013",                0,    0, CHRRAM_8,  RCM_GS2013},        //UNIF only!
	{ "BMC-NOVELDIAMOND9999999IN1", 0,    0, CHRRAM_0,  BMC_NOVEL1},
	{ "BMC-SUPER24IN1SC03",         8,    0, CHRRAM_8,  BMC_S24IN1SC03},
	{ "BMC-SUPERHIK8IN1",           8,    0, CHRRAM_0,  BMC_HIK8IN1},
	{ "BMC-T-262",                  0,    0, CHRRAM_8,  BMC_T262},      //UNIF only!
	{ "BMC-WS",                     0,    0, CHRRAM_0,  BMC_WS},        //UNIF only!
	{ "BMC-N625092",                0,    0, CHRRAM_0,  UNL_N625092},
	// below are boards which are not yet supported, but are used by some UNIF files. they are here as a reminder to what is missing to be added
	{ "UNL-TEK90",                  0,    0, CHRRAM_0,  JYCOMPANY_A}, // JY Company A (is TEK90 the real PCB name?)
	{ "UNL-KS7017",                 0,    0, CHRRAM_0,  KAISER_KS7017},
	{ "UNL-KS7032",                 0,    0, CHRRAM_0,  KAISER_KS7032}, //  mapper 142
	{ "UNL-603-5052",               0,    0, CHRRAM_0,  UNL_603_5052}, // mapper 238?
	{ "UNL-EDU2000",               32,    0, CHRRAM_8,  UNL_EDU2K},
	{ "UNL-H2288",                  0,    0, CHRRAM_0,  UNL_H2288}, // mapper 123
	{ "UNL-SHERO",                  0,    0, CHRRAM_8,  SACHEN_SHERO},
	{ "UNL-YOKO",                   0,    0, CHRRAM_0,  YOKO_BOARD}, // similar to mapper 83, but not the same
	{ "UNL-FS304",                  0,    8, CHRRAM_8,  WAIXING_FS304}, // used in Zelda 3 by Waixing
	{ "UNL-43272",                  0,    0, CHRRAM_0,  UNL_43272}, // used in Gaau Hok Gwong Cheung
	{ "BTL-MARIO1-MALEE2",          0,    0, CHRRAM_0,  UNL_MMALEE}, // mapper 55?
	{ "BMC-FK23C",                  0,    0, CHRRAM_0,  BMC_FK23C},
	{ "BMC-FK23CA",                 0,    0, CHRRAM_0,  BMC_FK23CA},
	{ "BMC-GHOSTBUSTERS63IN1",      0,    0, CHRRAM_8,  BMC_G63IN1 },
	{ "BMC-BS-5",                   0,    0, CHRRAM_0,  BMC_BENSHIENG},
	{ "BMC-810544-C-A1",            0,    0, CHRRAM_0,  BMC_810544},
	{ "BMC-411120-C",               0,    0, CHRRAM_0,  BMC_411120C},
	{ "BMC-8157",                   0,    0, CHRRAM_8,  BMC_8157},
	{ "BMC-830118C",                0,    0, CHRRAM_0,  BMC_830118C},
	{ "BMC-D1038",                  0,    0, CHRRAM_0,  BMC_VT5201}, // mapper 60?
	{ "BMC-SUPERVISION16IN1",       0,    0, CHRRAM_0,  SVISION16_BOARD}, // mapper 53
	{ "BMC-NTD-03",                 0,    0, CHRRAM_0,  BMC_NTD_03},
	{ "UNL-AC08",                   0,    0, CHRRAM_0,  UNL_AC08},
	{ "UNL-BB",                     0,    0, CHRRAM_0,  UNL_BB},
	{ "UNL-LH32",                   0,    0, CHRRAM_0,  UNL_LH32},
	{ "UNL-LH53",                   0,    0, CHRRAM_0,  UNL_LH53},
	{ "BMC-G-146",                  0,    0, CHRRAM_0,  BMC_G146},
	{ "BMC-11160",                  0,    0, CHRRAM_0,  BMC_11160},
	{ "UNL-MALISB",                 0,    0, CHRRAM_0,  UNL_MALISB},
	{ "UNL-TF1201",                 0,    0, CHRRAM_0,  UNL_TF1201},
	{ "BMC-12-IN-1",                0,    0, CHRRAM_0,  UNSUPPORTED_BOARD},
	{ "BMC-70IN1",                  0,    0, CHRRAM_0,  UNSUPPORTED_BOARD}, // mapper 236?
	{ "BMC-70IN1B",                 0,    0, CHRRAM_0,  UNSUPPORTED_BOARD}, // mapper 236?
	{ "BMC-42IN1RESETSWITCH",       0,    0, CHRRAM_0,  UNSUPPORTED_BOARD}, // mapper 60?
	{ "PEC-586",                     0,    0, CHRRAM_0,  UNSUPPORTED_BOARD},
	{ "UNL-DANCE",                  0,    0, CHRRAM_0,  UNSUPPORTED_BOARD},
	{ "UNL-DRIPGAME",               0,    0, CHRRAM_0,  UNSUPPORTED_BOARD}, // [by Quietust - we need more info]
	{ "UNL-CITYFIGHT",              0,    0, CHRRAM_0,  UNSUPPORTED_BOARD},
	{ "UNL-OneBus",                 0,    0, CHRRAM_0,  UNSUPPORTED_BOARD},
	{ "UNL-DANCE2000",              0,    0, CHRRAM_0,  UNSUPPORTED_BOARD}
};

const unif *nes_unif_lookup( const char *board )
{
	int i;
	for (i = 0; i < ARRAY_LENGTH(unif_list); i++)
	{
		if (!core_stricmp(unif_list[i].board, board))
			return &unif_list[i];
	}
	return NULL;
}

/*************************************************************

 unif_mapr_setup

 setup the board specific variables (wram, nvwram, pcb_id etc.)
 for a given board (after reading the MAPR chunk of the UNIF file)

 *************************************************************/

void unif_mapr_setup( const char *board, int *pcb_id, int *battery, int *prgram, int *vram_chunks )
{
	const unif *unif_board = nes_unif_lookup(board);
	if (unif_board == NULL)
		fatalerror("Unknown UNIF board %s.\n", board);

	*pcb_id = unif_board->board_idx;
	*battery = unif_board->nvwram;  // we should implement battery banks based on the size of this...
	*prgram = unif_board->wram; // we should implement WRAM banks based on the size of this...

	if (unif_board->chrram <= CHRRAM_8)
		*vram_chunks = 1;
	else if (unif_board->chrram == CHRRAM_16)
		*vram_chunks = 2;
	else if (unif_board->chrram == CHRRAM_32)
		*vram_chunks = 4;
}


/*************************************************************

 call_load_unif

 *************************************************************/

void nes_cart_slot_device::call_load_unif()
{
	UINT32 vram_size = 0, prgram_size = 0, battery_size = 0, mapper_sram_size = 0;
	// SETUP step 1: running through the file and getting PRG, VROM sizes
	UINT32 unif_ver = 0, chunk_length = 0, read_length = 0x20;
	UINT32 prg_start = 0, chr_start = 0;
	UINT32 size = length(), prg_size = 0, vrom_size = 0;
	UINT8 buffer[4], mirror = 0;
	char magic2[4];
	char unif_mapr[32]; // here we should store MAPR chunks
	bool mapr_chunk_found = FALSE, small_prg = FALSE;

	// allocate space to temporarily store PRG & CHR banks
	dynamic_buffer temp_prg(256 * 0x4000);
	dynamic_buffer temp_chr(256 * 0x2000);
	UINT8 temp_byte = 0;

	fseek(4, SEEK_SET);
	fread(&buffer, 4);
	unif_ver = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
	logerror("Loaded game in UNIF format, version %d\n", unif_ver);

	do
	{
		fseek(read_length, SEEK_SET);

		memset(magic2, '\0', sizeof(magic2));
		fread(&magic2, 4);

		/* We first run through the whole image to find a [MAPR] chunk. This is needed
		 because, unfortunately, the MAPR chunk is not always the first chunk (see
		 Super 24-in-1). When such a chunk is found, we set mapr_chunk_found=1 and
		 we go back to load other chunks! */
		if (!mapr_chunk_found)
		{
			if ((magic2[0] == 'M') && (magic2[1] == 'A') && (magic2[2] == 'P') && (magic2[3] == 'R'))
			{
				mapr_chunk_found = TRUE;
				logerror("[MAPR] chunk found: ");
				fread(&buffer, 4);
				chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

				if (chunk_length <= 0x20)
					fread(&unif_mapr, chunk_length);
				logerror("%s\n", unif_mapr);

				/* now that we found the MAPR chunk, we can go back to load other chunks */
				fseek(0x20, SEEK_SET);
				read_length = 0x20;
			}
			else
			{
				logerror("Skip this chunk. We need a [MAPR] chunk before anything else.\n");
				fread(&buffer, 4);
				chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

				read_length += (chunk_length + 8);
			}
		}
		else
		{
			/* What kind of chunk do we have here? */
			if ((magic2[0] == 'M') && (magic2[1] == 'A') && (magic2[2] == 'P') && (magic2[3] == 'R'))
			{
				/* The [MAPR] chunk has already been read, so we skip it */
				/* TO DO: it would be nice to check if more than one MAPR chunk is present */
				logerror("[MAPR] chunk found (in the 2nd run). Already loaded.\n");
				fread(&buffer, 4);
				chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

				read_length += (chunk_length + 8);
			}
			else if ((magic2[0] == 'R') && (magic2[1] == 'E') && (magic2[2] == 'A') && (magic2[3] == 'D'))
			{
				logerror("[READ] chunk found. No support yet.\n");
				fread(&buffer, 4);
				chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

				read_length += (chunk_length + 8);
			}
			else if ((magic2[0] == 'N') && (magic2[1] == 'A') && (magic2[2] == 'M') && (magic2[3] == 'E'))
			{
				logerror("[NAME] chunk found. No support yet.\n");
				fread(&buffer, 4);
				chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

				read_length += (chunk_length + 8);
			}
			else if ((magic2[0] == 'W') && (magic2[1] == 'R') && (magic2[2] == 'T') && (magic2[3] == 'R'))
			{
				logerror("[WRTR] chunk found. No support yet.\n");
				fread(&buffer, 4);
				chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

				read_length += (chunk_length + 8);
			}
			else if ((magic2[0] == 'T') && (magic2[1] == 'V') && (magic2[2] == 'C') && (magic2[3] == 'I'))
			{
				logerror("[TVCI] chunk found.\n");
				fread(&buffer, 4);
				chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

				fread(&temp_byte, 1);
				logerror("Television Standard : %s\n", (temp_byte == 0) ? "NTSC" : (temp_byte == 1) ? "PAL" : "Does not matter");

				read_length += (chunk_length + 8);
			}
			else if ((magic2[0] == 'T') && (magic2[1] == 'V') && (magic2[2] == 'S') && (magic2[3] == 'C')) // is this the same as TVCI??
			{
				logerror("[TVSC] chunk found. No support yet.\n");
				fread(&buffer, 4);
				chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

				read_length += (chunk_length + 8);
			}
			else if ((magic2[0] == 'D') && (magic2[1] == 'I') && (magic2[2] == 'N') && (magic2[3] == 'F'))
			{
				logerror("[DINF] chunk found. No support yet.\n");
				fread(&buffer, 4);
				chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

				read_length += (chunk_length + 8);
			}
			else if ((magic2[0] == 'C') && (magic2[1] == 'T') && (magic2[2] == 'R') && (magic2[3] == 'L'))
			{
				logerror("[CTRL] chunk found. No support yet.\n");
				fread(&buffer, 4);
				chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

				read_length += (chunk_length + 8);
			}
			else if ((magic2[0] == 'B') && (magic2[1] == 'A') && (magic2[2] == 'T') && (magic2[3] == 'R'))
			{
				logerror("[BATR] chunk found. No support yet.\n");
				fread(&buffer, 4);
				chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

				read_length += (chunk_length + 8);
			}
			else if ((magic2[0] == 'V') && (magic2[1] == 'R') && (magic2[2] == 'O') && (magic2[3] == 'R'))
			{
				logerror("[VROR] chunk found. No support yet.\n");
				fread(&buffer, 4);
				chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

				read_length += (chunk_length + 8);
			}
			else if ((magic2[0] == 'M') && (magic2[1] == 'I') && (magic2[2] == 'R') && (magic2[3] == 'R'))
			{
				logerror("[MIRR] chunk found.\n");
				fread(&buffer, 4);
				chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

				fread(&mirror, 1);

				read_length += (chunk_length + 8);
			}
			else if ((magic2[0] == 'P') && (magic2[1] == 'C') && (magic2[2] == 'K'))
			{
				logerror("[PCK%c] chunk found. No support yet.\n", magic2[3]);
				fread(&buffer, 4);
				chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

				read_length += (chunk_length + 8);
			}
			else if ((magic2[0] == 'C') && (magic2[1] == 'C') && (magic2[2] == 'K'))
			{
				logerror("[CCK%c] chunk found. No support yet.\n", magic2[3]);
				fread(&buffer, 4);
				chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

				read_length += (chunk_length + 8);
			}
			else if ((magic2[0] == 'P') && (magic2[1] == 'R') && (magic2[2] == 'G'))
			{
				logerror("[PRG%c] chunk found. ", magic2[3]);
				fread(&buffer, 4);
				chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
				prg_size += chunk_length;

				if (chunk_length / 0x4000)
					logerror("It consists of %d 16K-blocks.\n", chunk_length / 0x4000);
				else
				{
					small_prg = TRUE;
					logerror("This chunk is smaller than 16K: the emulation might have issues. Please report this file to the MESS forums.\n");
				}

				/* Read in the program chunks */
				fread(&temp_prg[prg_start], chunk_length);

				prg_start += chunk_length;
				read_length += (chunk_length + 8);
			}
			else if ((magic2[0] == 'C') && (magic2[1] == 'H') && (magic2[2] == 'R'))
			{
				logerror("[CHR%c] chunk found. ", magic2[3]);
				fread(&buffer, 4);
				chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
				vrom_size += chunk_length;

				logerror("It consists of %d 8K-blocks.\n", chunk_length / 0x2000);

				/* Read in the vrom chunks */
				fread(&temp_chr[chr_start], chunk_length);

				chr_start += chunk_length;
				read_length += (chunk_length + 8);
			}
			else
			{
				logerror("Unsupported UNIF chunk or corrupted header. Please report the problem at MESS Board.\n");
				read_length = size;
			}
		}
	} while (size > read_length);

	if (!mapr_chunk_found)
	{
		fatalerror("UNIF should have a [MAPR] chunk to work. Check if your image has been corrupted\n");
	}

	if (!prg_start)
	{
		fatalerror("No PRG found. Please report the problem at MESS Board.\n");
	}

	// SETUP step 2: getting PCB and other settings
	int pcb_id = 0, battery = 0, prgram = 0, vram_chunks = 0;
	unif_mapr_setup(unif_mapr, &pcb_id, &battery, &prgram, &vram_chunks);

	// SETUP step 3: storing the info needed for emulation
	m_pcb_id = pcb_id;
	if (battery)
		battery_size = NES_BATTERY_SIZE; // we should allow for smaller battery!
	prgram_size = prgram * 0x2000;
	vram_size = vram_chunks * 0x2000;

	m_cart->set_four_screen_vram(FALSE);
	switch (mirror)
	{
		case 0: // Horizontal Mirroring (Hard Wired)
			m_cart->set_mirroring(PPU_MIRROR_HORZ);
			break;
		case 1: // Vertical Mirroring (Hard Wired)
			m_cart->set_mirroring(PPU_MIRROR_VERT);
			break;
		case 2: // Mirror All Pages From $2000 (Hard Wired)
			m_cart->set_mirroring(PPU_MIRROR_LOW);
			break;
		case 3: // Mirror All Pages From $2400 (Hard Wired)
			m_cart->set_mirroring(PPU_MIRROR_HIGH);
			break;
		case 4: // Four Screens of VRAM (Hard Wired)
			m_cart->set_four_screen_vram(TRUE);
			m_cart->set_mirroring(PPU_MIRROR_4SCREEN);
			break;
		case 5: // Mirroring Controlled By Mapper Hardware
			logerror("Mirroring handled by the board hardware.\n");
			// default to horizontal at start
			m_cart->set_mirroring(PPU_MIRROR_HORZ);
			break;
		default:
			logerror("Undocumented mirroring value.\n");
			// default to horizontal
			m_cart->set_mirroring(PPU_MIRROR_HORZ);
			break;
	}

	// SETUP step 4: logging what we have found
	logerror("-- Board %s\n", unif_mapr);
	logerror("-- PRG 0x%x (%d x 16k chunks)\n", prg_size, prg_size / 0x4000);
	logerror("-- VROM 0x%x (%d x 8k chunks)\n", vrom_size, vrom_size / 0x2000);
	logerror("-- VRAM 0x%x (%d x 8k chunks)\n", vram_size, vram_size / 0x2000);

	// SETUP steps 5/6: allocate pointers for PRG/VROM and load the data!
	if (prg_size == 0x4000)
	{
		m_cart->prg_alloc(0x8000, tag());
		memcpy(m_cart->get_prg_base(), &temp_prg[0], 0x4000);
		memcpy(m_cart->get_prg_base() + 0x4000, m_cart->get_prg_base(), 0x4000);
	}
	else
	{
		m_cart->prg_alloc(prg_size, tag());
		memcpy(m_cart->get_prg_base(), &temp_prg[0], prg_size);
	}

	if (small_prg)  // This is not supported yet, so warn users about this
		osd_printf_error("Loaded UNIF file with non-16k PRG chunk. This is not supported in MESS yet.");

	if (vrom_size)
	{
		m_cart->vrom_alloc(vrom_size, tag());
		memcpy(m_cart->get_vrom_base(), &temp_chr[0], vrom_size);
	}

#if SPLIT_PRG
	{
		FILE *prgout;
		char outname[255];

		sprintf(outname, "%s.prg", filename());
		prgout = fopen(outname, "wb");
		if (prgout)
		{
			fwrite(m_cart->get_prg_base(), 1, 0x4000 * m_cart->get_prg_size(), prgout);
			osd_printf_error("Created PRG chunk\n");
		}

		fclose(prgout);
	}
#endif

#if SPLIT_CHR
	if (state->m_chr_chunks > 0)
	{
		FILE *chrout;
		char outname[255];

		sprintf(outname, "%s.chr", filename());
		chrout= fopen(outname, "wb");
		if (chrout)
		{
			fwrite(m_cart->get_vrom_base(), 1, m_cart->get_vrom_size(), chrout);
			osd_printf_error("Created CHR chunk\n");
		}
		fclose(chrout);
	}
#endif
	// SETUP steps 7: allocate the remaining pointer, when needed
	if (vram_size)
		m_cart->vram_alloc(vram_size);
	if (prgram_size)
		m_cart->prgram_alloc(prgram_size);

	// Attempt to load a battery file for this ROM
	// A few boards have internal RAM with a battery (MMC6, Taito X1-005 & X1-017, etc.)
	if (battery_size || mapper_sram_size)
	{
		UINT32 tot_size = battery_size + mapper_sram_size;
		dynamic_buffer temp_nvram(tot_size);
		battery_load(&temp_nvram[0], tot_size, 0x00);
		if (battery_size)
		{
			m_cart->battery_alloc(battery_size);
			memcpy(m_cart->get_battery_base(), &temp_nvram[0], battery_size);
		}
		if (mapper_sram_size)
			memcpy(m_cart->get_mapper_sram_base(), &temp_nvram[battery_size], mapper_sram_size);
	}

	logerror("UNIF support is only very preliminary.\n");
}

const char * nes_cart_slot_device::get_default_card_unif(UINT8 *ROM, UINT32 len)
{
	UINT32 chunk_length = 0, read_length = 0x20;
	int pcb_id = 0, battery = 0, prgram = 0, vram_chunks = 0;
	char unif_mapr[32];

	do
	{
		if ((ROM[read_length + 0] == 'M') && (ROM[read_length + 1] == 'A') && (ROM[read_length + 2] == 'P') && (ROM[read_length + 3] == 'R'))
		{
			chunk_length = ROM[read_length + 4] | (ROM[read_length + 5] << 8) | (ROM[read_length + 6] << 16) | (ROM[read_length + 7] << 24);

			if (chunk_length <= 0x20)
				memcpy(unif_mapr, ROM + read_length + 8, chunk_length);

			read_length += (chunk_length + 8);
		}
		else
		{
			chunk_length = ROM[read_length + 4] | (ROM[read_length + 5] << 8) | (ROM[read_length + 6] << 16) | (ROM[read_length + 7] << 24);
			read_length += (chunk_length + 8);
		}
	} while (len > read_length);

	unif_mapr_setup(unif_mapr, &pcb_id, &battery, &prgram, &vram_chunks);

	return nes_get_slot(pcb_id);
}
