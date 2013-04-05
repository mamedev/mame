#include "emu.h"
#include "hashfile.h"
#include "machine/nes_slot.h"

#define NES_BATTERY_SIZE 0x2000

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type NES_CART_SLOT = &device_creator<nes_cart_slot_device>;


//**************************************************************************
//    NES cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  device_nes_cart_interface - constructor
//-------------------------------------------------

device_nes_cart_interface::device_nes_cart_interface(const machine_config &mconfig, device_t &device)
						: device_slot_card_interface(mconfig, device),
						m_prg(NULL),
						m_prgram(NULL),
						m_vrom(NULL),
						m_vram(NULL),
						m_battery(NULL),
						m_mapper_ram(NULL),
						m_mapper_bram(NULL),
						m_prg_size(0),
						m_prgram_size(0),
						m_vrom_size(0),
						m_vram_size(0),
						m_battery_size(0),
						m_mapper_ram_size(0),
						m_mapper_bram_size(0)
{
}


//-------------------------------------------------
//  ~device_nes_cart_interface - destructor
//-------------------------------------------------

device_nes_cart_interface::~device_nes_cart_interface()
{
}

//-------------------------------------------------
//  pointer allocators
//-------------------------------------------------

void device_nes_cart_interface::prg_alloc(running_machine &machine, size_t size)
{
	if (m_prg == NULL)
	{
		m_prg = auto_alloc_array(machine, UINT8, size);
		m_prg_size = size;
	}
}

void device_nes_cart_interface::prgram_alloc(running_machine &machine, size_t size)
{
	if (m_prgram == NULL)
	{
		m_prgram = auto_alloc_array(machine, UINT8, size);
		m_prgram_size = size;
	}
}

void device_nes_cart_interface::vrom_alloc(running_machine &machine, size_t size)
{
	if (m_vrom == NULL)
	{
		m_vrom = auto_alloc_array(machine, UINT8, size);
		m_vrom_size = size;
	}
}

void device_nes_cart_interface::vram_alloc(running_machine &machine, size_t size)
{
	if (m_vram == NULL)
	{
		m_vram = auto_alloc_array(machine, UINT8, size);
		m_vram_size = size;
	}
}

void device_nes_cart_interface::battery_alloc(running_machine &machine, size_t size)
{
	if (m_battery == NULL)
	{
		m_battery = auto_alloc_array(machine, UINT8, size);
		m_battery_size = size;
	}
}

void device_nes_cart_interface::mapper_ram_alloc(running_machine &machine, size_t size)
{
	if (m_mapper_ram == NULL)
	{
		m_mapper_ram = auto_alloc_array(machine, UINT8, size);
		m_mapper_ram_size = size;
	}
}

void device_nes_cart_interface::mapper_bram_alloc(running_machine &machine, size_t size)
{
	if (m_mapper_bram == NULL)
	{
		m_mapper_bram = auto_alloc_array(machine, UINT8, size);
		m_mapper_bram_size = size;
	}
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nes_cart_slot_device - constructor
//-------------------------------------------------
nes_cart_slot_device::nes_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						device_t(mconfig, NES_CART_SLOT, "NES Cartridge Slot", tag, owner, clock),
						device_image_interface(mconfig, *this),
						device_slot_interface(mconfig, *this),
						m_chr_open_bus(0),
						m_ce_mask(0),
						m_ce_state(0),
						m_vrc_ls_prg_a(0),
						m_vrc_ls_prg_b(0),
						m_vrc_ls_chr(0),
						m_crc_hack(0),
						m_must_be_loaded(1)
{
}

//-------------------------------------------------
//  nes_cart_slot_device - destructor
//-------------------------------------------------

nes_cart_slot_device::~nes_cart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nes_cart_slot_device::device_start()
{
	m_cart = dynamic_cast<device_nes_cart_interface *>(get_card_device());
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void nes_cart_slot_device::device_config_complete()
{
	// inherit a copy of the static data
//  const nes_cart_interface *intf = reinterpret_cast<const nes_cart_interface *>(static_config());
//  if (intf != NULL)
//  {
//      *static_cast<nes_cart_interface *>(this) = *intf;
//  }

	// set brief and instance name
	update_names();
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

/*-------------------------------------------------
 call load
 -------------------------------------------------*/


struct nes_cart_lines
{
	const char *tag;
	int line;
};

static const struct nes_cart_lines nes_cart_lines_table[] =
{
	{ "PRG A0",    0 },
	{ "PRG A1",    1 },
	{ "PRG A2",    2 },
	{ "PRG A3",    3 },
	{ "PRG A4",    4 },
	{ "PRG A5",    5 },
	{ "PRG A6",    6 },
	{ "PRG A7",    7 },
	{ "CHR A10",  10 },
	{ "CHR A11",  11 },
	{ "CHR A12",  12 },
	{ "CHR A13",  13 },
	{ "CHR A14",  14 },
	{ "CHR A15",  15 },
	{ "CHR A16",  16 },
	{ "CHR A17",  17 },
	{ "NC",      127 },
	{ 0 }
};

static int nes_cart_get_line( const char *feature )
{
	const struct nes_cart_lines *nes_line = &nes_cart_lines_table[0];

	if (feature == NULL)
		return 128;

	while (nes_line->tag)
	{
		if (strcmp(nes_line->tag, feature) == 0)
			break;

		nes_line++;
	}

	return nes_line->line;
}

/* Set to generate prg & chr files when the cart is loaded */
#define SPLIT_PRG   0
#define SPLIT_CHR   0

bool nes_cart_slot_device::call_load()
{
	if (m_cart)
	{
		UINT32 vram_size = 0, prgram_size = 0, battery_size = 0, mapper_ram_size = 0, mapper_bram_size = 0; // helper for regions to alloc at the end

		if (software_entry() == NULL)
		{
			const char *mapinfo = NULL;
			int mapint1 = 0, mapint2 = 0, mapint3 = 0, mapint4 = 0;
			char magic[4];

			/* Check first 4 bytes of the image to decide if it is UNIF or iNES */
			/* Unfortunately, many .unf files have been released as .nes, so we cannot rely on extensions only */
			memset(magic, '\0', sizeof(magic));
			fread(magic, 4);

			if ((magic[0] == 'N') && (magic[1] == 'E') && (magic[2] == 'S'))    /* If header starts with 'NES' it is iNES */
			{
				UINT32 prg_size, vrom_size;
				UINT8 header[0x10];
				UINT8 mapper, local_options;
				bool ines20 = FALSE, has_trainer = FALSE, prg16k;

				// check if the image is recognized by nes.hsi
				mapinfo = hashfile_extrainfo(*this);

				// image_extrainfo() resets the file position back to start.
				fseek(0, SEEK_SET);
				// read out the header
				fread(&header, 0x10);

				// SETUP step 1: getting PRG, VROM, VRAM sizes
				prg16k = (header[4] == 1);
				prg_size = prg16k ? 2 * 0x4000 : header[4] * 0x4000;
				vrom_size = header[5] * 0x2000;
				vram_size = 0x4000;

				// SETUP step 2: getting PCB and other settings
				mapper = (header[6] & 0xf0) >> 4;
				local_options = header[6] & 0x0f;

				switch (header[7] & 0xc)
				{
					case 0x4:
					case 0xc:
						// probably the header got corrupted: don't trust upper bits for mapper
						break;

					case 0x8:   // it's iNES 2.0 format
						ines20 = TRUE;
					case 0x0:
					default:
						mapper |= header[7] & 0xf0;
						break;
				}

				// use info from nes.hsi if available!
				if (mapinfo)
				{
					if (4 == sscanf(mapinfo,"%d %d %d %d", &mapint1, &mapint2, &mapint3, &mapint4))
					{
						/* image is present in nes.hsi: overwrite the header settings with these */
						mapper = mapint1;
						local_options = mapint2 & 0x0f;
						m_crc_hack = (mapint2 & 0xf0) >> 4; // this is used to differentiate among variants of the same Mapper (see below)
						prg16k = (mapint3 == 1);
						prg_size = prg16k ? 2 * 0x4000 : mapint3 * 0x4000;
						vrom_size = mapint4 * 0x2000;
						logerror("NES.HSI info: %d %d %d %d\n", mapint1, mapint2, mapint3, mapint4);
					}
					else
					{
						logerror("NES: [%s], Invalid mapinfo found\n", mapinfo);
					}
				}
				else
				{
					logerror("NES: No extrainfo found\n");
				}

				// use extended iNES2.0 info if available!
				if (ines20)
				{
					mapper |= (header[8] & 0x0f) << 8;
					// header[8] & 0xf0 is used for submappers, but I haven't found any specific image to implement this
					prg_size += ((header[9] & 0x0f) << 8) * 0x4000;
					vrom_size += ((header[9] & 0xf0) << 4) * 0x2000;
				}

				// SETUP step 3: storing the info needed for emulation
				m_pcb_id = nes_get_mmc_id(machine(), mapper);
				m_cart->set_mirroring(BIT(local_options, 0) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
				if (BIT(local_options, 1))
					battery_size = NES_BATTERY_SIZE; // with original iNES format we can only support 8K WRAM battery
				has_trainer = BIT(local_options, 2) ? TRUE : FALSE;
				m_cart->set_four_screen_vram(BIT(local_options, 3));

				if (ines20)
				{
					// PRGRAM/BWRAM (not fully supported, also due to lack of 2.0 files)
					if ((header[10] & 0x0f) > 0)
						prgram_size = 0x80 << ((header[10] & 0x0f) - 1);
					if ((header[10] & 0xf0) > 0)
						battery_size = 0x80 << ((header[10] & 0xf0) - 5);
					// VRAM
					vram_size = 0;
					if ((header[11] & 0x0f) > 0)
						vram_size = 0x80 << ((header[11] & 0x0f) - 1);
					// header[11] & 0xf0 is the size of battery backed VRAM, found so far in Racermate II only and not supported yet
				}
				else
				{
					// always map PRGRAM/WRAM in bank5 (eventually, this should be enabled only for some mappers)
					// and save it depending on has_battery

					// PRGRAM size is 8k for most games, but pirate carts often use different sizes,
					// so its size has been added recently to the iNES format spec, but almost no image uses it
					prgram_size = header[8] ? header[8] * 0x2000 : 0x2000;
				}

				// a few mappers correspond to multiple PCBs, so we need a few additional checks and tweaks
				switch (m_pcb_id)
				{
					case STD_CNROM:
						if (mapper == 185)
						{
							switch (m_crc_hack)
							{
								case 0x0: // pin26: CE, pin27: CE (B-Wings, Bird Week)
									m_ce_mask = 0x03;
									m_ce_state = 0x03;
									break;
								case 0x4: // pin26: CE, pin27: /CE (Mighty Bomb Jack, Spy Vs. Spy)
									m_ce_mask = 0x03;
									m_ce_state = 0x01;
									break;
								case 0x8: // pin26: /CE, pin27: CE (Sansu 1, 2, 3 Nen)
									m_ce_mask = 0x03;
									m_ce_state = 0x02;
									break;
								case 0xc: // pin26: /CE, pin27: /CE (Seicross v2.0)
									m_ce_mask = 0x03;
									m_ce_state = 0x00;
									break;
							}
						}
						break;
					case KONAMI_VRC2:
						if (mapper == 22)
						{
							m_vrc_ls_prg_a = 0;
							m_vrc_ls_prg_b = 1;
							m_vrc_ls_chr = 1;
						}
						if (mapper == 23 && !m_crc_hack)
						{
							m_vrc_ls_prg_a = 1;
							m_vrc_ls_prg_b = 0;
							m_vrc_ls_chr = 0;
						}
						if (mapper == 23 && m_crc_hack)
						{
							// here there are also Akumajou Special, Crisis Force, Parodius da!, Tiny Toons which are VRC-4
							m_vrc_ls_prg_a = 3;
							m_vrc_ls_prg_b = 2;
							m_pcb_id = KONAMI_VRC4; // this allows for konami_irq to be installed at reset
						}
						break;
					case KONAMI_VRC4:
						if (mapper == 21)
						{
							// Wai Wai World 2 & Ganbare Goemon Gaiden 2 (the latter with crc_hack)
							m_vrc_ls_prg_a = m_crc_hack ? 7 : 2;
							m_vrc_ls_prg_b = m_crc_hack ? 6 : 1;
						}
						if (mapper == 25)   // here there is also Ganbare Goemon Gaiden which is VRC-2
						{
							m_vrc_ls_prg_a = m_crc_hack ? 2 : 0;
							m_vrc_ls_prg_b = m_crc_hack ? 3 : 1;
						}
						break;
					case KONAMI_VRC6:
						if (mapper == 24)
						{
							m_vrc_ls_prg_a = 1;
							m_vrc_ls_prg_b = 0;
						}
						if (mapper == 26)
						{
							m_vrc_ls_prg_a = 0;
							m_vrc_ls_prg_b = 1;
						}
						break;
					case IREM_G101:
						if (m_crc_hack)
							m_cart->set_mirroring(PPU_MIRROR_HIGH); // Major League has hardwired mirroring
						break;
					case DIS_74X161X161X32:
						if (mapper == 70)
							m_cart->set_mirroring(PPU_MIRROR_VERT); // only hardwired mirroring makes different mappers 70 & 152
						break;
					case SUNSOFT_2:
						if (mapper == 93)
							m_cart->set_mirroring(PPU_MIRROR_VERT); // only hardwired mirroring makes different mappers 89 & 93
						break;
					case STD_BXROM:
						if (m_crc_hack)
							m_pcb_id = AVE_NINA01; // Mapper 34 is used for 2 diff boards
						break;
					case BANDAI_LZ93:
						if (m_crc_hack)
							m_pcb_id = BANDAI_JUMP2;   // Mapper 153 is used for 2 diff boards
						break;
					case IREM_HOLYDIV:
						if (m_crc_hack)
							m_pcb_id = JALECO_JF16;    // Mapper 78 is used for 2 diff boards
						break;
					case CAMERICA_BF9093:
						if (m_crc_hack)
							m_pcb_id = CAMERICA_BF9097;    // Mapper 71 is used for 2 diff boards
						break;
					case HES_BOARD:
						if (m_crc_hack)
							m_pcb_id = HES6IN1_BOARD;  // Mapper 113 is used for 2 diff boards
						break;
					case WAIXING_ZS:
						if (m_crc_hack)
							m_pcb_id = WAIXING_DQ8;    // Mapper 242 is used for 2 diff boards
						break;
					case BMC_GOLD_7IN1:
						if (m_crc_hack)
							m_pcb_id = BMC_MARIOPARTY_7IN1;    // Mapper 52 is used for 2 diff boards
						break;
					case STD_EXROM:
						mapper_ram_size = 0x400;
						break;
					case TAITO_X1_017:
						mapper_ram_size = 0x1400;
						break;
					case TAITO_X1_005:
					case TAITO_X1_005_A:
						mapper_ram_size = 0x80;
						break;
					case NAMCOT_163:
						mapper_ram_size = 0x2000;
						break;
					case FUKUTAKE_BOARD:
						mapper_ram_size = 2816;
						break;
						//FIXME: we also have to fix Action 52 PRG loading somewhere...
				}

				// SETUP step 4: logging what we have found
				if (!ines20)
				{
					logerror("Loaded game in iNES format:\n");
					logerror("-- Mapper %d\n", mapper);
					logerror("-- PRG 0x%x (%d x 16k chunks)\n", prg_size, prg_size / 0x4000);
					logerror("-- VROM 0x%x (%d x 8k chunks)\n", vrom_size, vrom_size / 0x2000);
					logerror("-- VRAM 0x%x (%d x 8k chunks)\n", vram_size, vram_size / 0x2000);
					if (battery_size)
						logerror("-- Battery found\n");
					if (has_trainer)
						logerror("-- Trainer found\n");
					if (m_cart->get_four_screen_vram())
						logerror("-- 4-screen VRAM\n");
					logerror("-- TV System: %s\n", ((header[10] & 3) == 0) ? "NTSC" : (header[10] & 1) ? "Both NTSC and PAL" : "PAL");
				}
				else
				{
					logerror("Loaded game in Extended iNES format:\n");
					logerror("-- Mapper: %d\n", mapper);
					logerror("-- Submapper: %d\n", (header[8] & 0xf0) >> 4);
					logerror("-- PRG 0x%x (%d x 16k chunks)\n", prg_size, prg_size / 0x4000);
					logerror("-- VROM 0x%x (%d x 8k chunks)\n", vrom_size, vrom_size / 0x2000);
					logerror("-- VRAM 0x%x (%d x 8k chunks)\n", vram_size, vram_size / 0x2000);
					logerror("-- PRG NVWRAM: %d\n", header[10] & 0x0f);
					logerror("-- PRG WRAM: %d\n", (header[10] & 0xf0) >> 4);
					logerror("-- CHR NVWRAM: %d\n", header[11] & 0x0f);
					logerror("-- CHR WRAM: %d\n", (header[11] & 0xf0) >> 4);
					logerror("-- TV System: %s\n", (header[12] & 2) ? "Both NTSC and PAL" : (header[12] & 1) ? "PAL" : "NTSC");
				}

				// SETUP step 5: allocate pointers for PRG/VROM
				if (prg_size)
					m_cart->prg_alloc(machine(), prg_size);
				if (vrom_size)
					m_cart->vrom_alloc(machine(), vrom_size);

				if (has_trainer)
					fread(&m_cart->m_prgram[0x1000], 0x200);


				// SETUP step 6: at last load the data!
				// Read in the program chunks
				if (prg16k)
				{
					fread(m_cart->get_prg_base(), 0x4000);
					memcpy(m_cart->get_prg_base() + 0x4000, m_cart->get_prg_base(), 0x4000);
				}
				else
					fread(m_cart->get_prg_base(), m_cart->get_prg_size());
#if SPLIT_PRG
				{
					FILE *prgout;
					char outname[255];

					sprintf(outname, "%s.prg", filename());
					prgout = fopen(outname, "wb");
					if (prgout)
					{
						fwrite(m_cart->get_prg_base(), 1, 0x4000 * m_cart->get_prg_size(), prgout);
						mame_printf_error("Created PRG chunk\n");
					}

					fclose(prgout);
				}
#endif

				// Read in any chr chunks
				if (m_cart->get_vrom_size())
					fread(m_cart->get_vrom_base(), m_cart->get_vrom_size());

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
						mame_printf_error("Created CHR chunk\n");
					}
					fclose(chrout);
				}
#endif
			}
			else if ((magic[0] == 'U') && (magic[1] == 'N') && (magic[2] == 'I') && (magic[3] == 'F')) /* If header starts with 'UNIF' it is UNIF */
			{
				// SETUP step 1: running through the file and getting PRG, VROM sizes
				UINT32 unif_ver = 0, chunk_length = 0, read_length = 0x20;
				UINT32 prg_start = 0, chr_start = 0;
				UINT32 size = length(), prg_size = 0, vrom_size = 0;
				UINT8 buffer[4], mirror = 0;
				char magic2[4];
				char unif_mapr[32]; // here we should store MAPR chunks
				bool mapr_chunk_found = FALSE, small_prg = FALSE;

				// allocate space to temporarily store PRG & CHR banks
				UINT8 *temp_prg = auto_alloc_array(machine(), UINT8, 256 * 0x4000);
				UINT8 *temp_chr = auto_alloc_array(machine(), UINT8, 256 * 0x2000);
				UINT8 temp_byte = 0;

				fread(&buffer, 4);
				unif_ver = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
				logerror("Loaded game in UNIF format, version %d\n", unif_ver);

				if (size <= 0x20)
				{
					logerror("%s only contains the UNIF header and no data.\n", filename());
					return IMAGE_INIT_FAIL;
				}

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
					auto_free(machine(), temp_prg);
					auto_free(machine(), temp_chr);
					fatalerror("UNIF should have a [MAPR] chunk to work. Check if your image has been corrupted\n");
				}

				if (!prg_start)
				{
					auto_free(machine(), temp_prg);
					auto_free(machine(), temp_chr);
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

				m_cart->set_four_screen_vram(0);
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
						m_cart->set_four_screen_vram(1);
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
					m_cart->prg_alloc(machine(), 0x8000);
					memcpy(m_cart->get_prg_base(), temp_prg, 0x4000);
					memcpy(m_cart->get_prg_base() + 0x4000, m_cart->get_prg_base(), 0x4000);
				}
				else
				{
					m_cart->prg_alloc(machine(), prg_size);
					memcpy(m_cart->get_prg_base(), temp_prg, prg_size);
				}

				if (small_prg)  // This is not supported yet, so warn users about this
					mame_printf_error("Loaded UNIF file with non-16k PRG chunk. This is not supported in MESS yet.");

				if (vrom_size)
				{
					m_cart->vrom_alloc(machine(), vrom_size);
					memcpy(m_cart->get_vrom_base(), temp_chr, vrom_size);
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
						mame_printf_error("Created PRG chunk\n");
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
						mame_printf_error("Created CHR chunk\n");
					}
					fclose(chrout);
				}
#endif
				// free the temporary copy of PRG/CHR
				auto_free(machine(), temp_prg);
				auto_free(machine(), temp_chr);
				logerror("UNIF support is only very preliminary.\n");
			}
			else
			{
				logerror("%s is NOT a file in either iNES or UNIF format.\n", filename());
				return IMAGE_INIT_FAIL;
			}
		}
		else
		{
			// SETUP step 1: getting PRG, VROM, VRAM sizes
			UINT32 prg_size = get_software_region_length("prg");
			UINT32 vrom_size = get_software_region_length("chr");
			UINT32 vram_size = get_software_region_length("vram");
			vram_size += get_software_region_length("vram2");

			// validate the xml fields
			if (!prg_size)
				fatalerror("No PRG entry for this software! Please check if the xml list got corrupted\n");
			if (prg_size < 0x8000)
				fatalerror("PRG entry is too small! Please check if the xml list got corrupted\n");

			// SETUP step 2: getting PCB and other settings
			if (get_feature("pcb"))
				m_pcb_id = nes_get_pcb_id(machine(), get_feature("pcb"));
			else
				m_pcb_id = NO_BOARD;

			// SETUP step 3: storing the info needed for emulation
			if (m_pcb_id == STD_TVROM || m_pcb_id == STD_DRROM || m_pcb_id == IREM_LROG017)
				m_cart->set_four_screen_vram(1);
			else
				m_cart->set_four_screen_vram(0);

			if (get_software_region("bwram") != NULL)
				battery_size = get_software_region_length("bwram");

			if (m_pcb_id == BANDAI_LZ93EX)
			{
				// allocate the 24C01 or 24C02 EEPROM
				battery_size += 0x2000;
			}

			if (m_pcb_id == BANDAI_DATACH)
			{
				// allocate the 24C01 and 24C02 EEPROM
				battery_size += 0x4000;
			}

			if (get_software_region("wram") != NULL)
				prgram_size = get_software_region_length("wram");
			if (get_software_region("mapper_ram") != NULL)
				mapper_ram_size = get_software_region_length("mapper_ram");
			if (get_software_region("mapper_bram") != NULL)
				mapper_bram_size = get_software_region_length("mapper_bram");

			if (get_feature("mirroring"))
			{
				const char *mirroring = get_feature("mirroring");
				if (!strcmp(mirroring, "horizontal"))
					m_cart->set_mirroring(PPU_MIRROR_HORZ);
				if (!strcmp(mirroring, "vertical"))
					m_cart->set_mirroring(PPU_MIRROR_VERT);
				if (!strcmp(mirroring, "high"))
					m_cart->set_mirroring(PPU_MIRROR_HIGH);
				if (!strcmp(mirroring, "low"))
					m_cart->set_mirroring(PPU_MIRROR_LOW);
			}
			else
				m_cart->set_mirroring(PPU_MIRROR_NONE);

			/* Check for pins in specific boards which require them */
			if (m_pcb_id == STD_CNROM)
			{
				if (get_feature("chr-pin26") != NULL)
				{
					m_ce_mask |= 0x01;
					m_ce_state |= !strcmp(get_feature("chr-pin26"), "CE") ? 0x01 : 0;
				}
				if (get_feature("chr-pin27") != NULL)
				{
					m_ce_mask |= 0x02;
					m_ce_state |= !strcmp(get_feature("chr-pin27"), "CE") ? 0x02 : 0;
				}
			}

			if (m_pcb_id == TAITO_X1_005 && get_feature("x1-pin17") != NULL && get_feature("x1-pin31") != NULL)
			{
				if (!strcmp(get_feature("x1-pin17"), "CIRAM A10") && !strcmp(get_feature("x1-pin31"), "NC"))
					m_pcb_id = TAITO_X1_005_A;
			}

			if (m_pcb_id == KONAMI_VRC2)
			{
				m_vrc_ls_prg_a = nes_cart_get_line(get_feature("vrc2-pin3"));
				m_vrc_ls_prg_b = nes_cart_get_line(get_feature("vrc2-pin4"));
				m_vrc_ls_chr = (nes_cart_get_line(get_feature("vrc2-pin21")) != 10) ? 1 : 0;
//          mame_printf_error("VRC-2, pin3: A%d, pin4: A%d, pin21: %s\n", state->m_vrc_ls_prg_a, state->m_vrc_ls_prg_b, state->m_vrc_ls_chr ? "NC" : "A10");
			}

			if (m_pcb_id == KONAMI_VRC4)
			{
				m_vrc_ls_prg_a = nes_cart_get_line(get_feature("vrc4-pin3"));
				m_vrc_ls_prg_b = nes_cart_get_line(get_feature("vrc4-pin4"));
//          mame_printf_error("VRC-4, pin3: A%d, pin4: A%d\n", state->m_vrc_ls_prg_a, state->m_vrc_ls_prg_b);
			}

			if (m_pcb_id == KONAMI_VRC6)
			{
				m_vrc_ls_prg_a = nes_cart_get_line(get_feature("vrc6-pin9"));
				m_vrc_ls_prg_b = nes_cart_get_line(get_feature("vrc6-pin10"));
//          mame_printf_error("VRC-6, pin9: A%d, pin10: A%d\n", state->m_vrc_ls_prg_a, state->m_vrc_ls_prg_b);
			}

			/* Check for other misc board variants */
			if (m_pcb_id == STD_SOROM)
			{
				if (get_feature("mmc1_type") != NULL && !strcmp(get_feature("mmc1_type"), "MMC1A"))
					m_pcb_id = STD_SOROM_A;    // in MMC1-A PRG RAM is always enabled
			}

			if (m_pcb_id == STD_SXROM)
			{
				if (get_feature("mmc1_type") != NULL && !strcmp(get_feature("mmc1_type"), "MMC1A"))
					m_pcb_id = STD_SXROM_A;    // in MMC1-A PRG RAM is always enabled
			}

			if (m_pcb_id == STD_NXROM || m_pcb_id == SUNSOFT_DCS)
			{
				if (get_software_region("minicart") != NULL)    // check for dual minicart
				{
					m_pcb_id = SUNSOFT_DCS;
					// we shall load somewhere the minicart, but we still do not support this
				}
			}

			// SETUP step 4: logging what we have found
			logerror("Loaded game from softlist:\n");
			logerror("-- PCB: %s", get_feature("pcb"));
			if (m_pcb_id == UNSUPPORTED_BOARD)
				logerror(" (currently not supported by MESS)");
			logerror("\n-- PRG 0x%x (%d x 16k chunks)\n", prg_size, prg_size / 0x4000);
			logerror("-- VROM 0x%x (%d x 8k chunks)\n", vrom_size, vrom_size / 0x2000);
			logerror("-- VRAM 0x%x (%d x 8k chunks)\n", vram_size, vram_size / 0x2000);
			logerror("-- PRG NVWRAM: %d\n", m_cart->get_battery_size());
			logerror("-- PRG WRAM: %d\n",  m_cart->get_prgram_size());

			// SETUP steps 5/6: allocate pointers for PRG/VROM and load the data!
			m_cart->prg_alloc(machine(), prg_size);
			memcpy(m_cart->get_prg_base(), get_software_region("prg"), prg_size);
			if (vrom_size)
			{
				m_cart->vrom_alloc(machine(), vrom_size);
				memcpy(m_cart->get_vrom_base(), get_software_region("chr"), vrom_size);
			}
		}

		// Allocate the remaining pointer, when needed
		if (vram_size)
			m_cart->vram_alloc(machine(), vram_size);
		if (prgram_size)
			m_cart->prgram_alloc(machine(), prgram_size);
		if (mapper_ram_size)
			m_cart->mapper_ram_alloc(machine(), mapper_ram_size);

		// Attempt to load a battery file for this ROM
		// A few boards have internal RAM with a battery (MMC6, Taito X1-005 & X1-017, etc.)
		if (battery_size || mapper_bram_size)
		{
			UINT32 tot_size = battery_size + mapper_bram_size;
			UINT8 *temp_nvram = auto_alloc_array(machine(), UINT8, tot_size);
			battery_load(temp_nvram, tot_size, 0x00);
			if (battery_size)
			{
				m_cart->battery_alloc(machine(), battery_size);
				memcpy(m_cart->get_battery_base(), temp_nvram, battery_size);
			}
			if (mapper_bram_size)
			{
				m_cart->mapper_bram_alloc(machine(), mapper_bram_size);
				memcpy(m_cart->get_mapper_bram_base(), temp_nvram + battery_size, mapper_bram_size);
			}

			if (temp_nvram)
				auto_free(machine(), temp_nvram);
		}
	}

	return IMAGE_INIT_PASS;
}


/*-------------------------------------------------
 call_unload
 -------------------------------------------------*/

void nes_cart_slot_device::call_unload()
{
	if (m_cart)
	{
		if (m_cart->get_battery_size() || m_cart->get_mapper_bram_size())
		{
			UINT32 tot_size = m_cart->get_battery_size() + m_cart->get_mapper_bram_size();
			UINT8 *temp_nvram = auto_alloc_array(machine(), UINT8, tot_size);
			if (m_cart->get_battery_size())
				memcpy(temp_nvram, m_cart->get_battery_base(), m_cart->get_battery_size());
			if (m_cart->get_mapper_bram_size())
				memcpy(temp_nvram + m_cart->get_battery_size(), m_cart->get_mapper_bram_base(), m_cart->get_mapper_bram_size());

			battery_save(temp_nvram, tot_size);
			if (temp_nvram)
				auto_free(machine(), temp_nvram);
		}
	}
}


/*-------------------------------------------------
 call softlist load
 -------------------------------------------------*/

bool nes_cart_slot_device::call_softlist_load(char *swlist, char *swname, rom_entry *start_entry)
{
	load_software_part_region(this, swlist, swname, start_entry );
	return TRUE;
}

/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

const char * nes_cart_slot_device::get_default_card_software(const machine_config &config, emu_options &options)
{
	return "rom";
}


/*-------------------------------------------------
 read
 -------------------------------------------------*/

READ8_MEMBER(nes_cart_slot_device::read_l)
{
	if (m_cart)
		return m_cart->read_l(space, offset);
	else
		return 0xff;
}

READ8_MEMBER(nes_cart_slot_device::read_m)
{
	if (m_cart)
		return m_cart->read_m(space, offset);
	else
		return 0xff;
}

READ8_MEMBER(nes_cart_slot_device::read_h)
{
	if (m_cart)
		return m_cart->read_h(space, offset);
	else
		return 0xff;
}


/*-------------------------------------------------
 write
 -------------------------------------------------*/

WRITE8_MEMBER(nes_cart_slot_device::write_l)
{
	if (m_cart)
		m_cart->write_l(space, offset, data);
}

WRITE8_MEMBER(nes_cart_slot_device::write_m)
{
	if (m_cart)
		m_cart->write_m(space, offset, data);
}

WRITE8_MEMBER(nes_cart_slot_device::write_h)
{
	if (m_cart)
		m_cart->write_h(space, offset, data);
}


// CART DEVICE [TO BE MOVED TO SEPARATE SOURCE LATER]

//-------------------------------------------------
//  nes_rom_device - constructor
//-------------------------------------------------

const device_type NES_ROM = &device_creator<nes_rom_device>;

nes_rom_device::nes_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, NES_ROM, "NES ROM", tag, owner, clock, "nes_rom", __FILE__),
					device_nes_cart_interface( mconfig, *this )
{
}

nes_rom_device::nes_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
					device_nes_cart_interface( mconfig, *this )
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nes_rom_device::device_start()
{
}
