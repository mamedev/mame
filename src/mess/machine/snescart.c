/***************************************************************************

  snescart.c

  Machine file to handle cart loading in the Nintendo Super NES emulation.

***************************************************************************/

#include "emu.h"
#include "includes/snes.h"
#include "imagedev/cartslot.h"
#include "snescart.h"

#define SNES_CART_DEBUG 0

/****** Strings for Cart Header Logging ******/

#define UNK "Unknown"

/* Some known type of cart */
static const char *const types[] =
{
	"ROM",
	"ROM, DSP-1",
	"ROM, DSP-2",
	"ROM, DSP-3",
	"ROM, DSP-4",
	"ROM, Super FX / FX2",
	"ROM, SA-1",
	"ROM, S-DD1",
	"ROM, OBC-1",
	"ROM, S-RTC",
	"ROM, Z80GB (Super Game Boy)",
	"ROM, C4",
	"ROM, Seta ST-010",
	"ROM, Seta ST-011",
	"ROM, Seta ST-018",
	"ROM, SPC7110",
	"ROM, SPC7110, RTC",
	UNK,                            // to add: Satellaview BS-X detection
};

/* Some known countries */
static const char *const countries[] =
{
/* 0*/  "Japan (NTSC)", "USA & Canada (NTSC)", "Europe, Oceania & Asia (PAL)", "Sweden (PAL)",
/* 4*/  "Finland (PAL)", "Denmark (PAL)", "France (PAL)", "Holland (PAL)",
/* 8*/  "Spain (PAL)", "Germany, Austria & Switzerland (PAL)", "Italy (PAL)", "Hong Kong & China (PAL)",
/* c*/  "Indonesia (PAL)", "South Korea (NTSC)", UNK, UNK,
};

/* Some known companies (integrations to the list from Snes9x) */
static const char *const companies[] =
{
/* 0*/  "Invalid", "Nintendo", "Ajinomoto", "Imagineer-Zoom", "Chris Gray Enterprises Inc.", "Zamuse", "Falcom", UNK,
/* 8*/  "Capcom", "HOT-B", "Jaleco", "Coconuts", "Rage Software", "Micronet", "Technos", "Mebio Software",
/*10*/  "SHOUEi System", "Starfish", "Gremlin Graphics", "Electronic Arts", "NCS / Masaya", "COBRA Team", "Human/Field", "KOEI",
/*18*/  "Hudson Soft", "Game Village", "Yanoman", UNK, "Tecmo", UNK, "Open System", "Virgin Games",
/*20*/  "KSS", "Sunsoft", "POW", "Micro World", UNK, UNK, "Enix", "Loriciel/Electro Brain",
/*28*/  "Kemco", "Seta Co.,Ltd.", "Culture Brain", "Irem Japan", "Pal Soft", "Visit Co.,Ltd.", "INTEC Inc.", "System Sacom Corp.",
/*30*/  "Viacom New Media", "Carrozzeria", "Dynamic", "Nintendo", "Magifact", "Hect", UNK, UNK,
/*38*/  "Capcom Europe", "Accolade Europe", UNK, "Arcade Zone", "Empire Software", "Loriciel", "Gremlin Graphics", UNK,
/*40*/  "Seika Corp.", "UBI Soft", UNK, UNK, "LifeFitness Exertainment", UNK, "System 3", "Spectrum Holobyte",
/*48*/  UNK, "Irem", UNK, "Raya Systems/Sculptured Software", "Renovation Products", "Malibu Games/Black Pearl", UNK, "U.S. Gold",
/*50*/  "Absolute Entertainment", "Acclaim", "Activision", "American Sammy", "GameTek", "Hi Tech Expressions", "LJN Toys", UNK,
/*58*/  UNK, UNK, "Mindscape", "Romstar, Inc.", UNK, "Tradewest", UNK, "American Softworks Corp.",
/*60*/  "Titus", "Virgin Interactive Entertainment", "Maxis", "Origin/FCI/Pony Canyon", UNK, UNK, UNK, "Ocean",
/*68*/  UNK, "Electronic Arts", UNK, "Laser Beam", UNK, UNK, "Elite", "Electro Brain",
/*70*/  "Infogrames", "Interplay", "LucasArts", "Parker Brothers", "Konami", "STORM", UNK, UNK,
/*78*/  "THQ Software", "Accolade Inc.", "Triffix Entertainment", UNK, "Microprose", UNK, UNK, "Kemco",
/*80*/  "Misawa", "Teichio", "Namco Ltd.", "Lozc", "Koei", UNK, "Tokuma Shoten Intermedia", "Tsukuda Original",
/*88*/  "DATAM-Polystar", UNK, UNK, "Bullet-Proof Software", "Vic Tokai", UNK, "Character Soft", "I\'\'Max",
/*90*/  "Takara", "CHUN Soft", "Video System Co., Ltd.", "BEC", UNK, "Varie", "Yonezawa / S'Pal Corp.", "Kaneco",
/*98*/  UNK, "Pack in Video", "Nichibutsu", "TECMO", "Imagineer Co.", UNK, UNK, UNK,
/*a0*/  "Telenet", "Hori", UNK, UNK, "Konami", "K.Amusement Leasing Co.", UNK, "Takara",
/*a8*/  UNK, "Technos Jap.", "JVC", UNK, "Toei Animation", "Toho", UNK, "Namco Ltd.",
/*b0*/  "Media Rings Corp.", "ASCII Co. Activison", "Bandai", UNK, "Enix America", UNK, "Halken", UNK,
/*b8*/  UNK, UNK, "Culture Brain", "Sunsoft", "Toshiba EMI", "Sony Imagesoft", UNK, "Sammy",
/*c0*/  "Taito", UNK, "Kemco", "Square", "Tokuma Soft", "Data East", "Tonkin House", UNK,
/*c8*/  "KOEI", UNK, "Konami USA", "NTVIC", UNK, "Meldac", "Pony Canyon", "Sotsu Agency/Sunrise",
/*d0*/  "Disco/Taito", "Sofel", "Quest Corp.", "Sigma", "Ask Kodansha Co., Ltd.", UNK, "Naxat", UNK,
/*d8*/  "Capcom Co., Ltd.", "Banpresto", "Tomy", "Acclaim", UNK, "NCS", "Human Entertainment", "Altron",
/*e0*/  "Jaleco", UNK, "Yutaka", UNK, "T&ESoft", "EPOCH Co.,Ltd.", UNK, "Athena",
/*e8*/  "Asmik", "Natsume", "King Records", "Atlus", "Sony Music Entertainment", UNK, "IGS", UNK,
/*f0*/  UNK, "Motown Software", "Left Field Entertainment", "Beam Software", "Tec Magik", UNK, UNK, UNK,
/*f8*/  UNK, "Cybersoft", UNK, "Psygnosis", UNK, UNK, "Davidson", UNK,
};


/* We use this to convert the company_id in the header to int value to be passed in companies[] */
static int char_to_int_conv( char id )
{
	int value;

	if (id == '1') value = 0x01;
	else if (id == '2') value = 0x02;
	else if (id == '3') value = 0x03;
	else if (id == '4') value = 0x04;
	else if (id == '5') value = 0x05;
	else if (id == '6') value = 0x06;
	else if (id == '7') value = 0x07;
	else if (id == '8') value = 0x08;
	else if (id == '9') value = 0x09;
	else if (id == 'A') value = 0x0a;
	else if (id == 'B') value = 0x0b;
	else if (id == 'C') value = 0x0c;
	else if (id == 'D') value = 0x0d;
	else if (id == 'E') value = 0x0e;
	else if (id == 'F') value = 0x0f;
	else value = 0x00;

	return value;
}


/***************************************************************************

  SRAM handling

***************************************************************************/

/* Loads the battery backed RAM into the appropriate memory area */
static void snes_load_sram(running_machine &machine)
{
	snes_state *state = machine.driver_data<snes_state>();
	device_image_interface *image = dynamic_cast<device_image_interface *>(machine.device("cart"));
	image->battery_load(state->m_cart[0].m_nvram, state->m_cart[0].m_nvram_size, 0xff);
}

/* Saves the battery backed RAM from the appropriate memory area */
void snes_machine_stop(running_machine &machine)
{
	snes_state *state = machine.driver_data<snes_state>();

	/* Save SRAM */
	if (state->m_cart[0].m_nvram_size > 0)
	{
		device_image_interface *image = dynamic_cast<device_image_interface *>(machine.device("cart"));
		image->battery_save(state->m_cart[0].m_nvram, state->m_cart[0].m_nvram_size);
	}
}


static void sufami_load_sram(running_machine &machine, const char *cart_tag)
{
	snes_state *state = machine.driver_data<snes_state>();
	int slot_id = 0;
	device_image_interface *image = dynamic_cast<device_image_interface *>(machine.device(cart_tag));

	if (strcmp(cart_tag, ":slot_a") == 0)
		slot_id = 0;

	if (strcmp(cart_tag, ":slot_b") == 0)
		slot_id = 1;

	image->battery_load(state->m_cart[slot_id].m_nvram, state->m_cart[slot_id].m_nvram_size, 0xff);
}

void sufami_machine_stop(running_machine &machine)
{
	snes_state *state = machine.driver_data<snes_state>();

	if (state->m_cart[0].slot_in_use && state->m_cart[0].m_nvram_size)
	{
		device_image_interface *image = dynamic_cast<device_image_interface *>(machine.device("slot_a"));
		image->battery_save(state->m_cart[0].m_nvram, state->m_cart[0].m_nvram_size);
	}

	if (state->m_cart[1].slot_in_use && state->m_cart[1].m_nvram_size)
	{
		device_image_interface *image = dynamic_cast<device_image_interface *>(machine.device("slot_b"));
		image->battery_save(state->m_cart[1].m_nvram, state->m_cart[1].m_nvram_size);
	}
}


/***************************************************************************

  Cart handling

***************************************************************************/

/* This function assign a 'score' to data immediately after 'offset' to measure how valid they are
as information block (to decide if the image is HiRom, LoRom, ExLoRom or ExHiRom) */
/* Code from bsnes, courtesy of byuu - http://byuu.org/ , based on previous code by Cowering */
static int snes_validate_infoblock( UINT8 *infoblock, UINT32 offset )
{
	int score = 0;
	UINT16 reset_vector = infoblock[offset + 0x3c] | (infoblock[offset + 0x3d] << 8);
	UINT16 checksum     = infoblock[offset + 0x1e] | (infoblock[offset + 0x1f] << 8);
	UINT16 ichecksum    = infoblock[offset + 0x1c] | (infoblock[offset + 0x1d] << 8);
	UINT8 reset_opcode  = infoblock[(offset & ~0x7fff) | (reset_vector & 0x7fff)];  //first opcode executed upon reset
	UINT8 mapper        = infoblock[offset + 0x15] & ~0x10;                         //mask off irrelevant FastROM-capable bit

	/* $00:[000-7fff] contains uninitialized RAM and MMIO.
	reset vector must point to ROM at $00:[8000-ffff] to be considered valid. */
	if (reset_vector < 0x8000)
		return 0;

	/* some images duplicate the header in multiple locations, and others have completely
	invalid header information that cannot be relied upon. The code below will analyze
	the first opcode executed at the specified reset vector to determine the probability
	that this is the correct header. Score is assigned accordingly. */

	/* most likely opcodes */
	if (reset_opcode == 0x78        //sei
		|| reset_opcode == 0x18     //clc (clc; xce)
		|| reset_opcode == 0x38     //sec (sec; xce)
		|| reset_opcode == 0x9c     //stz $nnnn (stz $4200)
		|| reset_opcode == 0x4c     //jmp $nnnn
		|| reset_opcode == 0x5c     //jml $nnnnnn
	)
		score += 8;

	/* plausible opcodes */
	if (reset_opcode == 0xc2        //rep #$nn
		|| reset_opcode == 0xe2     //sep #$nn
		|| reset_opcode == 0xad     //lda $nnnn
		|| reset_opcode == 0xae     //ldx $nnnn
		|| reset_opcode == 0xac     //ldy $nnnn
		|| reset_opcode == 0xaf     //lda $nnnnnn
		|| reset_opcode == 0xa9     //lda #$nn
		|| reset_opcode == 0xa2     //ldx #$nn
		|| reset_opcode == 0xa0     //ldy #$nn
		|| reset_opcode == 0x20     //jsr $nnnn
		|| reset_opcode == 0x22     //jsl $nnnnnn
	)
		score += 4;

	/* implausible opcodes */
	if (reset_opcode == 0x40        //rti
		|| reset_opcode == 0x60     //rts
		|| reset_opcode == 0x6b     //rtl
		|| reset_opcode == 0xcd     //cmp $nnnn
		|| reset_opcode == 0xec     //cpx $nnnn
		|| reset_opcode == 0xcc     //cpy $nnnn
	)
		score -= 4;

	/* least likely opcodes */
	if (reset_opcode == 0x00        //brk #$nn
		|| reset_opcode == 0x02     //cop #$nn
		|| reset_opcode == 0xdb     //stp
		|| reset_opcode == 0x42     //wdm
		|| reset_opcode == 0xff     //sbc $nnnnnn,x
	)
		score -= 8;

	/* Sometimes, both the header and reset vector's first opcode will match ...
	fallback and rely on info validity in these cases to determine more likely header. */

	/* a valid checksum is the biggest indicator of a valid header. */
	if ((checksum + ichecksum) == 0xffff && (checksum != 0) && (ichecksum != 0))
		score += 4;

	/* then there are the expected mapper values */
	if (offset == 0x007fc0 && mapper == 0x20)   // 0x20 is usually LoROM
		score += 2;

	if (offset == 0x00ffc0 && mapper == 0x21)   // 0x21 is usually HiROM
		score += 2;

	if (offset == 0x007fc0 && mapper == 0x22)   // 0x22 is usually ExLoROM
		score += 2;

	if (offset == 0x40ffc0 && mapper == 0x25)   // 0x25 is usually ExHiROM
		score += 2;

	/* finally, there are valid values in the Company, Region etc. fields */
	if (infoblock[offset + 0x1a] == 0x33)           // Company field: 0x33 indicates extended header
		score += 2;

	if (infoblock[offset + 0x16] < 0x08)            // ROM Type field
		score++;

	if (infoblock[offset + 0x17] < 0x10)            // ROM Size field
		score++;

	if (infoblock[offset + 0x18] < 0x08)            // SRAM Size field
		score++;

	if (infoblock[offset + 0x19] < 14)              // Region field
		score++;

	/* do we still have a positive score? */
	if (score < 0)
		score = 0;

	return score;
}

/* Here we add a couple of cart utilities, to avoid duplicating the code in each DEVICE_IMAGE_LOAD */
static UINT32 snes_skip_header( device_image_interface &image, UINT32 snes_rom_size )
{
	UINT8 header[512];
	UINT32 offset = 512;

	/* Check for a header (512 bytes) */
	if (image.software_entry() == NULL)
		image.fread(header, 512);
	else
		memcpy(header, image.get_software_region("rom"), 512);

	if ((header[8] == 0xaa) && (header[9] == 0xbb) && (header[10] == 0x04))
	{
		/* Found an SWC identifier */
		logerror("Found header (SWC) - Skipped\n");
	}
	else if ((header[0] | (header[1] << 8)) == (((snes_rom_size - 512) / 1024) / 8))
	{
		/* Some headers have the rom size at the start, if this matches with the actual rom size, we probably have a header */
		logerror("Found header (size) - Skipped\n");
	}
	else if ((snes_rom_size % 0x8000) == 512)
	{
		/* As a last check we'll see if there's exactly 512 bytes extra to this image. */
		logerror("Found header (extra) - Skipped\n");
	}
	else
	{
		/* No header found so go back to the start of the file */
		logerror("No header found.\n");
		offset = 0;
	}

	return offset;
}


/* This determines if a cart is in Mode 20, 21, 22 or 25; sets state->m_cart[0].mode and
 state->m_cart[0].sram_max accordingly; and returns the offset of the internal header
 (needed to detect BSX and ST carts) */
static UINT32 snes_find_hilo_mode( device_image_interface &image, UINT8 *buffer, UINT32 len, UINT32 offset, int cartid )
{
	snes_state *state = image.device().machine().driver_data<snes_state>();
	UINT8 valid_mode20, valid_mode21, valid_mode25;
	UINT32 retvalue;

	/* Now to determine if this is a lo-ROM, a hi-ROM or an extended lo/hi-ROM */
	valid_mode20 = snes_validate_infoblock(buffer, 0x007fc0);
	valid_mode21 = snes_validate_infoblock(buffer, 0x00ffc0);
	valid_mode25 = (len > 0x40ffc0) ? snes_validate_infoblock(buffer, 0x40ffc0) : 0;

	/* Images larger than 32mbits are likely ExHiRom */
	if (valid_mode25)
		valid_mode25 += 4;

	if ((valid_mode20 >= valid_mode21) && (valid_mode20 >= valid_mode25))
	{
		if ((buffer[0x007fd5] == 0x32) || ((state->m_cart_size - offset) > 0x401000))
			state->m_cart[cartid].mode = SNES_MODE_22;  // ExLoRom
		else
			state->m_cart[cartid].mode = SNES_MODE_20;  // LoRom

		retvalue = 0x007fc0;

		/* a few games require 512k, however we store twice as much to be sure to cover the various mirrors */
		state->m_cart[cartid].sram_max = 0x100000;
	}
	else if (valid_mode21 >= valid_mode25)
	{
		state->m_cart[cartid].mode = SNES_MODE_21;  // HiRom
		retvalue = 0x00ffc0;
		state->m_cart[cartid].sram_max = 0x20000;
	}
	else
	{
		state->m_cart[cartid].mode = SNES_MODE_25;  // ExHiRom
		retvalue = 0x40ffc0;
		state->m_cart[cartid].sram_max = 0x20000;
	}

	logerror( "\t HiROM/LoROM id: %s (LoROM: %d , HiROM: %d, ExHiROM: %d)\n",
				(state->m_cart[cartid].mode == SNES_MODE_20) ? "LoROM" :
				(state->m_cart[cartid].mode == SNES_MODE_21) ? "HiROM" :
				(state->m_cart[cartid].mode == SNES_MODE_22) ? "ExLoROM" :
				(state->m_cart[cartid].mode == SNES_MODE_25) ? "ExHiROM" : "Other (BSX or ST)",
				valid_mode20, valid_mode21, valid_mode25);

	return retvalue;
}

static int snes_find_addon_chip( running_machine &machine, UINT8 *buffer, UINT32 start_offs )
{
	snes_state *state = machine.driver_data<snes_state>();
	int supported_type = 1;
	int dsp_prg_offset = 0;

	/* Info mostly taken from http://snesemu.black-ship.net/misc/-from%20nsrt.edgeemu.com-chipinfo.htm */
	switch (buffer[start_offs + 0x16])
	{
		case 0x00:
		case 0x01:
		case 0x02:
			state->m_has_addon_chip = HAS_NONE;
			break;

		case 0x03:
			if (buffer[start_offs + 0x15] == 0x30)
			{
				state->m_has_addon_chip = HAS_DSP4;
				dsp_prg_offset = SNES_DSP4_OFFSET;
			}
			else
			{
				state->m_has_addon_chip = HAS_DSP1;
				dsp_prg_offset = SNES_DSP1B_OFFSET;
			}
			break;

		case 0x04:
			state->m_has_addon_chip = HAS_DSP1;
			dsp_prg_offset = SNES_DSP1B_OFFSET;
			break;

		case 0x05:
			if (buffer[start_offs + 0x15] == 0x20)
			{
				state->m_has_addon_chip = HAS_DSP2;
				dsp_prg_offset = SNES_DSP2_OFFSET;
			}
			/* DSP-3 is hard to detect. We exploit the fact that the only game has been manufactured by Bandai */
			else if ((buffer[start_offs + 0x15] == 0x30) && (buffer[start_offs + 0x1a] == 0xb2))
			{
				state->m_has_addon_chip = HAS_DSP3;
				dsp_prg_offset = SNES_DSP3_OFFSET;
			}
			else
			{
				state->m_has_addon_chip = HAS_DSP1;
				dsp_prg_offset = SNES_DSP1B_OFFSET;
			}
			break;

		case 0x13:  // Mario Chip 1
		case 0x14:  // GSU-x
		case 0x15:  // GSU-x
		case 0x1a:  // GSU-1 (21 MHz at start)
			if (buffer[start_offs + 0x15] == 0x20)
				state->m_has_addon_chip = HAS_SUPERFX;
			break;

		case 0x25:
			state->m_has_addon_chip = HAS_OBC1;
			break;

		case 0x32:  // needed by a Sample game (according to ZSNES)
		case 0x34:
		case 0x35:
			if (buffer[start_offs + 0x15] == 0x23)
			{
				state->m_has_addon_chip = HAS_SA1;
				supported_type = 0;
				mame_printf_error("This is a SA-1 type game, currently unsupported by the driver\n");
			}
			break;

		case 0x43:
		case 0x45:
			if (buffer[start_offs + 0x15] == 0x32)
			{
				state->m_has_addon_chip = HAS_SDD1;
			}
			break;

		case 0x55:
			if (buffer[start_offs + 0x15] == 0x35)
			{
				state->m_has_addon_chip = HAS_RTC;
			}
			break;

		case 0xe3:
			state->m_has_addon_chip = HAS_Z80GB;
			supported_type = 0;
			break;

		case 0xf3:
			state->m_has_addon_chip = HAS_CX4;
			break;

		case 0xf5:
			if (buffer[start_offs + 0x15] == 0x30)
			{
				state->m_has_addon_chip = HAS_ST018;
				supported_type = 0;
			}
			else if (buffer[start_offs + 0x15] == 0x3a)
			{
				state->m_has_addon_chip = HAS_SPC7110;
			}
			break;

		case 0xf6:
			/* These Seta ST-01X chips have both 0x30 at 0x00ffd5,
			 they only differ for the 'size' at 0x00ffd7 */
			if (buffer[start_offs + 0x17] < 0x0a)
				state->m_has_addon_chip = HAS_ST011;
			else
				state->m_has_addon_chip = HAS_ST010;

			// if we are loading the game in a driver without the ST01X DSP, revert to HAS_NONE to avoid crash
			if (!state->m_upd96050)
				state->m_has_addon_chip = HAS_NONE;
			break;

		case 0xf9:
			if (buffer[start_offs + 0x15] == 0x3a)
			{
				state->m_has_addon_chip = HAS_SPC7110_RTC;
				supported_type = 0;
			}
			break;

		default:
			state->m_has_addon_chip = HAS_UNK;
			supported_type = 0;
			break;
	}

	if ((state->m_has_addon_chip >= HAS_DSP1) && (state->m_has_addon_chip <= HAS_DSP4))
	{
		UINT8 *dspsrc = (UINT8 *)(*machine.root_device().memregion("addons"));
		UINT32 *dspprg = (UINT32 *)(*machine.root_device().memregion("dspprg"));
		UINT16 *dspdata = (UINT16 *)(*machine.root_device().memregion("dspdata"));

		// copy DSP program
		for (int i = 0; i < 0x2000; i+= 4)
		{
			*dspprg = dspsrc[dsp_prg_offset+0+i]<<24 | dspsrc[dsp_prg_offset+1+i]<<16 | dspsrc[dsp_prg_offset+2+i]<<8;
			dspprg++;
		}

		// copy DSP data
		for (int i = 0; i < 0x800; i+= 2)
		{
			*dspdata++ = dspsrc[dsp_prg_offset+0x2000+i]<<8 | dspsrc[dsp_prg_offset+0x2001+i];
		}
	}

	if ((state->m_has_addon_chip == HAS_ST010) || (state->m_has_addon_chip == HAS_ST011))
	{
		UINT8 *dspsrc = (UINT8 *)(*machine.root_device().memregion("addons"));
		UINT32 *dspprg = (UINT32 *)(*machine.root_device().memregion("dspprg"));
		UINT16 *dspdata = (UINT16 *)(*machine.root_device().memregion("dspdata"));

		// copy DSP program
		for (int i = 0; i < 0x10000; i+= 4)
		{
			*dspprg = dspsrc[0+i]<<24 | dspsrc[1+i]<<16 | dspsrc[2+i]<<8;
			dspprg++;
		}

		// copy DSP data
		for (int i = 0; i < 0x1000; i+= 2)
		{
			*dspdata++ = dspsrc[0x10000+i]<<8 | dspsrc[0x10001+i];
		}
	}

	return supported_type;
}

static void snes_cart_log_info( running_machine &machine, UINT8* ROM, UINT32 len, int supported )
{
	snes_state *state = machine.driver_data<snes_state>();
	device_image_interface *image = dynamic_cast<device_image_interface *>(machine.device("cart"));
	char title[21], rom_id[4], company_id[2];
	int i, company, has_ram = 0, has_sram = 0;
	UINT32 offset = snes_skip_header(*image, state->m_cart_size);
	UINT32 hilo_mode = snes_find_hilo_mode(*image, ROM, len, offset, 0);

	/* Company */
	for (i = 0; i < 2; i++)
		company_id[i] = ROM[hilo_mode - 0x10 + i];
	company = (char_to_int_conv(company_id[0]) << 4) + char_to_int_conv(company_id[1]);
	if (company == 0)
		company = ROM[hilo_mode + 0x1a];

	/* ROM ID */
	for (i = 0; i < 4; i++)
		rom_id[i] = ROM[hilo_mode - 0x0e + i];

	/* Title */
	for (i = 0; i < 21; i++)
		title[i] = ROM[hilo_mode + i];

	/* RAM */
	if (((ROM[hilo_mode + 0x16] & 0xf) == 1) ||
		((ROM[hilo_mode + 0x16] & 0xf) == 2) ||
		((ROM[hilo_mode + 0x16] & 0xf) == 4) ||
		((ROM[hilo_mode + 0x16] & 0xf) == 5))
		has_ram = 1;

	/* SRAM */
	if (((ROM[hilo_mode + 0x16] & 0xf) == 2) ||
		((ROM[hilo_mode + 0x16] & 0xf) == 5) ||
		((ROM[hilo_mode + 0x16] & 0xf) == 6))
		has_sram = 1;

	int total_blocks = (state->m_cart[0].m_rom_size - offset) / (state->m_cart[0].mode & 0xa5 ? 0x8000 : 0x10000);

	logerror( "ROM DETAILS\n" );
	logerror( "===========\n\n" );
	logerror( "\tTotal blocks:  %d (%dmb)\n", total_blocks, total_blocks / (state->m_cart[0].mode & 5 ? 32 : 16) );
	logerror( "\tROM bank size: %s \n",
				(state->m_cart[0].mode == SNES_MODE_20) ? "LoROM" :
				(state->m_cart[0].mode == SNES_MODE_21) ? "HiROM" :
				(state->m_cart[0].mode == SNES_MODE_22) ? "ExLoROM" :
				(state->m_cart[0].mode == SNES_MODE_25) ? "ExHiROM" : "Other (BSX or ST)" );
	logerror( "\tCompany:       %s [%.2s]\n", companies[company], company_id );
	logerror( "\tROM ID:        %.4s\n\n", rom_id );

	logerror( "HEADER DETAILS\n" );
	logerror( "==============\n\n" );
	logerror( "\tName:          %.21s\n", title );
	logerror( "\tSpeed:         %s [%d]\n", ((ROM[hilo_mode + 0x15] & 0xf0)) ? "FastROM" : "SlowROM", (ROM[hilo_mode + 0x15] & 0xf0) >> 4 );
	logerror( "\tBank size:     %s [%d]\n", (ROM[hilo_mode + 0x15] & 0xf) ? "HiROM" : "LoROM", ROM[hilo_mode + 0x15] & 0xf );

	logerror( "\tType:          %s", types[state->m_has_addon_chip]);
	if (has_ram)
		logerror( ", RAM");
	if (has_sram)
		logerror( ", SRAM");
	logerror( " [%d]\n", ROM[hilo_mode + 0x16] );

	logerror( "\tSize:          %d megabits [%d]\n", 1 << (ROM[hilo_mode + 0x17] - 7), ROM[hilo_mode + 0x17] );
	logerror( "\tSRAM:          %d kilobits [%d]\n", state->m_cart[0].m_nvram_size * 8, ROM[hilo_mode + 0x18] );
	logerror( "\tCountry:       %s [%d]\n", countries[ROM[hilo_mode + 0x19]], ROM[hilo_mode + 0x19] );
	logerror( "\tLicense:       %s [%X]\n", companies[ROM[hilo_mode + 0x1a]], ROM[hilo_mode + 0x1a] );
	logerror( "\tVersion:       1.%d\n", ROM[hilo_mode + 0x1b] );
	logerror( "\tInv Checksum:  %X %X\n", ROM[hilo_mode + 0x1d], ROM[hilo_mode + 0x1c] );
	logerror( "\tChecksum:      %X %X\n", ROM[hilo_mode + 0x1f], ROM[hilo_mode + 0x1e] );
	logerror( "\tNMI Address:   %2X%2Xh\n", ROM[hilo_mode + 0x3b], ROM[hilo_mode + 0x3a] );
	logerror( "\tStart Address: %2X%2Xh\n\n", ROM[hilo_mode + 0x3d], ROM[hilo_mode + 0x3c] );

	logerror( "\tMode: %d\n", state->m_cart[0].mode);

	if (!supported)
		logerror("WARNING: This cart type \"%s\" is not supported yet!\n", types[state->m_has_addon_chip]);
}

DEVICE_IMAGE_LOAD_MEMBER( snes_state,snes_cart )
{
	int supported_type = 1;
	int has_bsx_slot = 0, st_bios = 0;
	UINT32 offset, int_header_offs;

	if (image.software_entry() == NULL)
		m_cart_size = image.length();
	else
		m_cart_size = image.get_software_region_length("rom");

	// Check for a header (512 bytes), and skip it if found
	offset = snes_skip_header(image, m_cart_size);

	// Allocate rom pointer
	m_cart[0].m_rom_size = m_cart_size - offset;
	m_cart[0].m_rom = auto_alloc_array_clear(machine(), UINT8, m_cart[0].m_rom_size);

	if (image.software_entry() == NULL)
	{
		image.fseek(offset, SEEK_SET);
		image.fread(m_cart[0].m_rom, m_cart_size - offset);
	}
	else
		memcpy(m_cart[0].m_rom, image.get_software_region("rom") + offset, m_cart_size - offset);

	if (SNES_CART_DEBUG) mame_printf_error("size %08X\n", m_cart_size - offset);

	// Setup the bank map to handle mirroring of ROM up to 8MB of accessible memory
	rom_map_setup(m_cart[0].m_rom_size);

	// Check if the cart is HiROM or LoROM (and set variables accordingly)
	int_header_offs = snes_find_hilo_mode(image, m_cart[0].m_rom, m_cart[0].m_rom_size, offset, 0);

	// Detect BS-X carts:
	// 1. Detect BS-X Flash Cart
	if ((m_cart[0].m_rom[int_header_offs + 0x13] == 0x00 || m_cart[0].m_rom[int_header_offs + 0x13] == 0xff) &&
			m_cart[0].m_rom[int_header_offs + 0x14] == 0x00)
	{
		UINT8 n15 = m_cart[0].m_rom[int_header_offs + 0x15];
		if (n15 == 0x00 || n15 == 0x80 || n15 == 0x84 || n15 == 0x9c || n15 == 0xbc || n15 == 0xfc)
		{
			if (m_cart[0].m_rom[int_header_offs + 0x1a] == 0x33 || m_cart[0].m_rom[int_header_offs + 0x1a] == 0xff)
			{
				// BS-X Flash Cart
				mame_printf_error("This is a game with BS-X slot: MESS does not support these yet, sorry.\n");
				m_cart[0].mode = SNES_MODE_BSX;
			}
		}
	}

	// 2. Detect presence of BS-X flash cartridge connector
	if ((m_cart[0].m_rom[int_header_offs - 14] == 'Z') && (m_cart[0].m_rom[int_header_offs - 11] == 'J'))
	{
		UINT8 n13 = m_cart[0].m_rom[int_header_offs - 13];
		if ((n13 >= 'A' && n13 <= 'Z') || (n13 >= '0' && n13 <= '9'))
		{
			if (m_cart[0].m_rom[int_header_offs + 0x1a] == 0x33 ||
				(m_cart[0].m_rom[int_header_offs - 10] == 0x00 && m_cart[0].m_rom[int_header_offs - 4] == 0x00))
			{
				has_bsx_slot = 1;
			}
		}
	}

	// If there is a BS-X connector, detect if it is the Base Cart or a compatible slotted cart
	if (has_bsx_slot)
	{
		mame_printf_error("This is a game with BS-X slot: MESS does not support these yet, sorry.\n");
		if (!memcmp(m_cart[0].m_rom + int_header_offs, "Satellaview BS-X     ", 21))
		{
			//BS-X Base Cart
			m_cart[0].mode = SNES_MODE_20; //SNES_MODE_BSX;
		}
		else
		{
			m_cart[0].mode = (int_header_offs == 0x007fc0) ? SNES_MODE_20 : SNES_MODE_21; //SNES_MODE_BSLO : SNES_MODE_BSHI;
		}
	}

	/* Then, detect Sufami Turbo carts */
	if (!memcmp(m_cart[0].m_rom, "BANDAI SFC-ADX", 14))
	{
		m_cart[0].mode = SNES_MODE_ST;
		if (!memcmp(m_cart[0].m_rom + 16, "SFC-ADX BACKUP", 14))
			st_bios = 1;
	}
	if (st_bios)
		mame_printf_error("This is the Sufami Turbo base cart. MESS does not fully support this game in snes/snespal yet, sorry.\nYou might want to try the snesst driver.\n");
	else if (m_cart[0].mode == SNES_MODE_ST)
	{
		mame_printf_error("This is a Sufami Turbo data cart and cannot be loaded for snes/snespal in MESS.\n");
		mame_printf_error("Please use snesst driver to load it, instead.\n");
		return IMAGE_INIT_FAIL;
	}

	
	if (SNES_CART_DEBUG) mame_printf_error("mode %d\n", m_cart[0].mode);

	/* Detect special chips */
	supported_type = snes_find_addon_chip(machine(), m_cart[0].m_rom, int_header_offs);

	/* Find the amount of cart ram */
	m_cart[0].m_nvram_size = 0;
	if (image.software_entry() == NULL)
	{
		UINT32 nvram_size;
		if ((m_has_addon_chip != HAS_SUPERFX))
			nvram_size = m_cart[0].m_rom[int_header_offs + 0x18];
		else
			nvram_size = (m_cart[0].m_rom[0x007fbd] & 0x07);

		if (nvram_size > 0)
		{
			nvram_size = (1024 << nvram_size);
			if (nvram_size > m_cart[0].sram_max)
				nvram_size = m_cart[0].sram_max;

			m_cart[0].m_nvram_size = nvram_size;
		}
//      printf("size %x\n", m_cart[0].m_nvram_size);
	}
	else
	{
		// if we are loading from softlist, take memory length from the xml
		m_cart[0].m_nvram_size = image.get_software_region("nvram") ? image.get_software_region_length("nvram") : 0;

		if (m_cart[0].m_nvram_size > 0)
		{
			if (m_cart[0].m_nvram_size > m_cart[0].sram_max)
				fatalerror("Found more memory than max allowed (found: %x, max: %x), check xml file!\n", m_cart[0].m_nvram_size, m_cart[0].sram_max);
		}
		// TODO: Eventually sram handlers should point to the allocated cart:sram region!
		// For now, we only use the region as a placeholder to carry size info...
//      printf("size %x\n", m_cart[0].m_nvram_size);
	}

	if (m_cart[0].m_nvram_size > 0)
		m_cart[0].m_nvram = auto_alloc_array_clear(machine(), UINT8, m_cart[0].m_nvram_size);

	/* Log snes_cart information */
	snes_cart_log_info(machine(), m_cart[0].m_rom, m_cart[0].m_rom_size, supported_type);

	/* Load SRAM */
	if (m_cart[0].m_nvram_size > 0)
		snes_load_sram(machine());

	/* All done */
	return IMAGE_INIT_PASS;
}

DEVICE_IMAGE_LOAD_MEMBER( snes_state,sufami_cart )
{
	int st_bios = 0, slot_id = 0;
	UINT32 offset;
	UINT8 *ROM = machine().root_device().memregion(image.device().tag())->base();

	if (strcmp(image.device().tag(), ":slot_a") == 0)
		slot_id = 0;

	if (strcmp(image.device().tag(), ":slot_b") == 0)
		slot_id = 1;

	if (image.software_entry() == NULL)
		m_cart_size = image.length();
	else
		m_cart_size = image.get_software_region_length("rom");

	/* Check for a header (512 bytes), and skip it if found */
	offset = snes_skip_header(image, m_cart_size);

	if (image.software_entry() == NULL)
	{
		image.fseek(offset, SEEK_SET);
		image.fread(ROM, m_cart_size - offset);
	}
	else
		memcpy(ROM, image.get_software_region("rom") + offset, m_cart_size - offset);

	if (SNES_CART_DEBUG) mame_printf_error("size %08X\n", m_cart_size - offset);

	/* Detect Sufami Turbo carts */
	if (!memcmp(ROM, "BANDAI SFC-ADX", 14))
	{
		m_cart[slot_id].mode = SNES_MODE_ST;
		if (!memcmp(ROM + 16, "SFC-ADX BACKUP", 14))
			st_bios = 1;
	}
	else
	{
		mame_printf_error("This is not a Sufami Turbo data pack.\n");
		mame_printf_error("This image cannot be loaded in snesst (Use snes or snespal drivers, instead).\n");
		return IMAGE_INIT_FAIL;
	}

	if (st_bios == 1)
	{
		mame_printf_error("This is the Sufami Turbo BIOS and not a Sufami Turbo data pack.\n");
		mame_printf_error("This image cannot be loaded in snesst.\n");
		return IMAGE_INIT_FAIL;
	}


	m_cart[slot_id].m_rom_size = m_cart_size;
	m_cart[slot_id].m_rom = auto_alloc_array_clear(machine(), UINT8, m_cart[0].m_rom_size);
	memcpy(m_cart[slot_id].m_rom, ROM, m_cart[slot_id].m_rom_size - offset);
	rom_map_setup(m_cart[slot_id].m_rom_size);

	m_cart[slot_id].m_nvram_size = 0x20000;
	m_cart[slot_id].m_nvram = auto_alloc_array_clear(machine(), UINT8, m_cart[slot_id].m_nvram_size);

	sufami_load_sram(machine(), image.device().tag());

	m_cart[slot_id].slot_in_use = 1; // aknowledge the cart in this slot, for saving sram at exit

	return IMAGE_INIT_PASS;
}

MACHINE_CONFIG_FRAGMENT( snes_cartslot )
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("sfc,smc,fig,swc,bin")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("snes_cart")
	MCFG_CARTSLOT_LOAD(snes_state,snes_cart)

	MCFG_SOFTWARE_LIST_ADD("cart_list","snes")
	MCFG_SOFTWARE_LIST_FILTER("cart_list","NTSC")
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( snesp_cartslot )
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("sfc,smc,fig,swc,bin")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("snes_cart")
	MCFG_CARTSLOT_LOAD(snes_state,snes_cart)

	MCFG_SOFTWARE_LIST_ADD("cart_list","snes")
	MCFG_SOFTWARE_LIST_FILTER("cart_list","PAL")
MACHINE_CONFIG_END

// This (hackily) emulates a SNES unit with a Sufami Turbo Unit cart inserted:
// hence, the user can mount two data cart in the two slots available on the ST Unit
MACHINE_CONFIG_FRAGMENT( sufami_cartslot )
	MCFG_CARTSLOT_ADD("slot_a")
	MCFG_CARTSLOT_EXTENSION_LIST("st,sfc")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("sufami_cart")
	MCFG_CARTSLOT_LOAD(snes_state,sufami_cart)

	MCFG_CARTSLOT_ADD("slot_b")
	MCFG_CARTSLOT_EXTENSION_LIST("st,sfc")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("sufami_cart")
	MCFG_CARTSLOT_LOAD(snes_state,sufami_cart)

//  MCFG_SOFTWARE_LIST_ADD("cart_list","snes")
MACHINE_CONFIG_END


DRIVER_INIT_MEMBER(snes_state,snes_mess)
{
	snes_ram = machine().root_device().memregion("maincpu")->base();
	memset(snes_ram, 0, 0x1000000);
}

DRIVER_INIT_MEMBER(snes_state,snesst)
{
	m_cart[0].slot_in_use = 0;
	m_cart[1].slot_in_use = 0;

	DRIVER_INIT_CALL(snes_mess);
}

// add-on chip emulators
#include "machine/snesobc1.c"
#include "machine/snescx4.c"
#include "machine/snesrtc.c"
#include "machine/snessdd1.c"
#include "machine/snes7110.c"
