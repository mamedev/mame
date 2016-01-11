// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Cowering
/***********************************************************************************************************


    SNES cart emulation
    (through slot devices)


    Carts can be mapped in memory in several different ways and accesses to carts depend
    on the presence of add-on chips (which map their I/O to diff memory areas)

    Hence, carts can interface with the main system through the following handlers
    * read_l : typically used to read ROM from memory range [00-7f][0000-ffff]
    * read_h : typically used to read ROM from memory range [80-ff][0000-ffff]
    * read_ram : used to read (NV)RAM at the appropriate offset (masks has to be applied
                 *before* calling it, if dealing with >32K RAM)
    * write_ram : used to write (NV)RAM at the appropriate offset
    * read_chip : used to read add-on chip registers
    * write_chip : used to write to add-on chip registers

    Also, we define two additional ROM access handlers, write_l & write_h for carts with
    subslots (e.g. BS-X compatible ones), that need to write to subslot (NV)RAM independently
    to accesses to their own (NV)RAM.

    Notes about add-on detection and handling (useful for future addition of st018, cx4, etc.)
    ===============================================================================================
    When loading from softlist, m_type would be enough to take care of add-on chips, because
    the ones needing a CPU dump have it in the zipfile. However, to support these games also
    from fullpath, both with files having DSP data appended to the .sfc and with older dumps
    missing DSP data, a second variable is present in the SNES slot: m_addon.
    From fullpath, support works as follows
    - get_default_card_software needs to decide whether to use the main devices or the legacy
      ones containing DSP dump as device roms, so it gets m_type as the main device should be
      used and if m_addon is ADDON_DSP* or ADDON_ST*, then it checks if the DSP data is appended
      or if m_type has to be switched to legacy type
    - call_load needs to detect faulty dumps too, to alloc m_addon_bios and copy the data from
      the correct place, so if m_addon is ADDON_DSP* or ADDON_ST* it checks whether DSP data is
      appended or not: if it is, this data is copied to m_addon_bios; if not, then we are in
      the legacy device case and data is copied from the device rom
    After the cart has been loaded and emulation has started, only m_type is needed to later
    handlers installation and cart accesses

    Also notice that, from softlist, DSP1, 1B, 2, 3 are treated as the same device, because they
    all have the same I/O and the only difference (i.e. the DSP data) comes from the zipfile itself.
    OTOH, to support faulty dumps missing DSP content, we need separate legacy devices...


 ***********************************************************************************************************/


#include "emu.h"
#include "snes_slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type SNS_CART_SLOT = &device_creator<sns_cart_slot_device>;
const device_type SNS_SUFAMI_CART_SLOT = &device_creator<sns_sufami_cart_slot_device>;
const device_type SNS_BSX_CART_SLOT = &device_creator<sns_bsx_cart_slot_device>;

//**************************************************************************
//    SNES Cartridge Interface
//**************************************************************************

//-------------------------------------------------
//  device_sns_cart_interface - constructor
//-------------------------------------------------

device_sns_cart_interface::device_sns_cart_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_rom(nullptr),
		m_rom_size(0)
{
}


//-------------------------------------------------
//  ~device_sns_cart_interface - destructor
//-------------------------------------------------

device_sns_cart_interface::~device_sns_cart_interface()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_sns_cart_interface::rom_alloc(UINT32 size, const char *tag)
{
	if (m_rom == nullptr)
	{
		m_rom = device().machine().memory().region_alloc(std::string(tag).append(SNSSLOT_ROM_REGION_TAG).c_str(), size, 1, ENDIANNESS_LITTLE)->base();
		m_rom_size = size;
	}
}


//-------------------------------------------------
//  nvram_alloc - alloc the space for the nvram
//-------------------------------------------------

void device_sns_cart_interface::nvram_alloc(UINT32 size)
{
	m_nvram.resize(size);
}


//-------------------------------------------------
//  rtc_ram_alloc - alloc the space for the rtc_ram
//  (needed to save it to NVRAM, will be removed
//  once the RTCs become devices and NVRAM gets
//  saved by the device itself)
//-------------------------------------------------

void device_sns_cart_interface::rtc_ram_alloc(UINT32 size)
{
	m_rtc_ram.resize(size);
}


//-------------------------------------------------
//  addon_bios_alloc - alloc the space for the
//  (optional) add-on CPU bios
//-------------------------------------------------

void device_sns_cart_interface::addon_bios_alloc(UINT32 size)
{
	m_bios.resize(size);
}


//-------------------------------------------------
//  rom_map_setup - setup map of rom banks in 32K
//  blocks, so to simplify ROM access
//-------------------------------------------------

void device_sns_cart_interface::rom_map_setup(UINT32 size)
{
	int i;
	// setup the rom_bank_map array to faster ROM read
	for (i = 0; i < size / 0x8000; i++)
		rom_bank_map[i] = i;

	// fill up remaining blocks with mirrors
	while (i % 256)
	{
		int j = 0, repeat_banks;
		while ((i % (256 >> j)) && j < 8)
			j++;
		repeat_banks = i % (256 >> (j - 1));
		for (int k = 0; k < repeat_banks; k++)
			rom_bank_map[i + k] = rom_bank_map[i + k - repeat_banks];
		i += repeat_banks;
	}

// check bank map!
//  for (i = 0; i < 256; i++)
//  {
//      printf("bank %3d = %3d\t", i, rom_bank_map[i]);
//      if ((i%8) == 7)
//          printf("\n");
//  }
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  base_sns_cart_slot_device - constructor
//-------------------------------------------------
base_sns_cart_slot_device::base_sns_cart_slot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
						device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_image_interface(mconfig, *this),
						device_slot_interface(mconfig, *this),
						m_addon(ADDON_NONE),
						m_type(SNES_MODE20), m_cart(nullptr)
{
}

sns_cart_slot_device::sns_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						base_sns_cart_slot_device(mconfig, SNS_CART_SLOT, "SNES Cartridge Slot", tag, owner, clock, "sns_cart_slot", __FILE__)
{
}

sns_sufami_cart_slot_device::sns_sufami_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						base_sns_cart_slot_device(mconfig, SNS_SUFAMI_CART_SLOT, "SNES Sufami Turbo Cartridge Slot", tag, owner, clock, "sns_sufami_cart_slot", __FILE__)
{
}

sns_bsx_cart_slot_device::sns_bsx_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						base_sns_cart_slot_device(mconfig, SNS_BSX_CART_SLOT, "SNES BS-X Cartridge Slot", tag, owner, clock, "sns_bsx_cart_slot", __FILE__)
{
}

//-------------------------------------------------
//  base_sns_cart_slot_device - destructor
//-------------------------------------------------

base_sns_cart_slot_device::~base_sns_cart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void base_sns_cart_slot_device::device_start()
{
	m_cart = dynamic_cast<device_sns_cart_interface *>(get_card_device());
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void base_sns_cart_slot_device::device_config_complete()
{
	// set brief and instance name
	update_names();
}


//-------------------------------------------------
//  SNES PCB
//-------------------------------------------------


struct sns_slot
{
	int                     pcb_id;
	const char              *slot_option;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const sns_slot slot_list[] =
{
	{ SNES_MODE20,      "lorom"},
	{ SNES_BSXLO,       "lorom_bsx"},
	{ SNES_CX4,         "lorom_cx4"},
	{ SNES_DSP,         "lorom_dsp"},
	{ SNES_DSP4,        "lorom_dsp4"},
	{ SNES_OBC1,        "lorom_obc1"},
	{ SNES_SA1,         "lorom_sa1"},
	{ SNES_SDD1,        "lorom_sdd1"},
	{ SNES_SFX,         "lorom_sfx"},
	{ SNES_Z80GB,       "lorom_sgb"},
	{ SNES_ST010,       "lorom_st010"},
	{ SNES_ST011,       "lorom_st011"},
	{ SNES_ST018,       "lorom_st018"},
	{ SNES_SUFAMITURBO, "lorom_sufami"},
	{ SNES_MODE21,      "hirom"},
	{ SNES_DSP_MODE21,  "hirom_dsp"},
	{ SNES_BSXHI,       "hirom_bsx"},
	{ SNES_SPC7110,     "hirom_spc7110"},
	{ SNES_SPC7110_RTC, "hirom_spcrtc"},
	{ SNES_SRTC,        "hirom_srtc"},
	{ SNES_BSX,         "bsxrom"},
	// BS-X memory packs
	{ SNES_BSMEMPAK,    "bsmempak"},
	// Sufami Turbo carts
	{ SNES_STROM,       "strom"},
	// Event carts
	{ SNES_PFEST94,     "pfest94" },
	// pirate carts
	{ SNES_POKEMON,      "lorom_poke"},
	{ SNES_TEKKEN2,      "lorom_tekken2"},
	{ SNES_SOULBLAD,     "lorom_sbld"},
	{ SNES_MCPIR1,       "lorom_mcpir1"},
	{ SNES_MCPIR2,       "lorom_mcpir2"},
	{ SNES_20COL,        "lorom_20col"},
	{ SNES_BANANA,       "lorom_pija"},  // wip
	{ SNES_BUGS,         "lorom_bugs"},  // wip
	// legacy slots to support DSPx games from fullpath
	{ SNES_DSP1_LEG,     "lorom_dsp1leg"},
	{ SNES_DSP1B_LEG,    "lorom_dsp1bleg"},
	{ SNES_DSP2_LEG,     "lorom_dsp2leg"},
	{ SNES_DSP3_LEG,     "lorom_dsp3leg"},
	{ SNES_DSP4_LEG,     "lorom_dsp4leg"},
	{ SNES_DSP1_MODE21_LEG, "hirom_dsp1leg"},
	{ SNES_ST010_LEG,     "lorom_st10leg"},
	{ SNES_ST011_LEG,     "lorom_st11leg"}
};

static int sns_get_pcb_id(const char *slot)
{
	for (auto & elem : slot_list)
	{
		if (!core_stricmp(elem.slot_option, slot))
			return elem.pcb_id;
	}

	return 0;
}

static const char *sns_get_slot(int type)
{
	for (auto & elem : slot_list)
	{
		if (elem.pcb_id == type)
			return elem.slot_option;
	}

	return "lorom";
}


/*-------------------------------------------------
 SRAM handling
 -------------------------------------------------*/

/*************************************
 *  Helper functions
 *************************************/


/* Here we add a couple of cart utilities, to avoid duplicating the code in each DEVICE_IMAGE_LOAD */
UINT32 base_sns_cart_slot_device::snes_skip_header( UINT8 *ROM, UINT32 rom_size )
{
	UINT8 header[512];
	UINT32 offset = 512;

	/* Check for a header (512 bytes) */
	memcpy(header, ROM, 512);

	if ((header[8] == 0xaa) && (header[9] == 0xbb) && (header[10] == 0x04))
	{
		/* Found an SWC identifier */
		logerror("Found header (SWC) - Skipped\n");
	}
	else if ((header[0] | (header[1] << 8)) == (((rom_size - 512) / 1024) / 8))
	{
		/* Some headers have the rom size at the start, if this matches with the actual rom size, we probably have a header */
		logerror("Found header (size) - Skipped\n");
	}
	else if ((rom_size % 0x8000) == 512)
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

/* This determines if a cart is in Mode 20, 21, 22 or 25; sets state->m_cart[0].mode and
 state->m_cart[0].sram accordingly; and returns the offset of the internal header (needed to
 detect BSX and ST carts) */
static UINT32 snes_find_hilo_mode(device_t *device, UINT8 *buffer, UINT32 buf_len )
{
	UINT8 valid_mode20 = 0;
	UINT8 valid_mode21 = 0;
	UINT8 valid_mode25 = 0;
	UINT32 retvalue;

	/* Now to determine if this is a lo-ROM, a hi-ROM or an extended lo/hi-ROM */
	if (buf_len > 0x007fc0)
		valid_mode20 = snes_validate_infoblock(buffer, 0x007fc0);
	if (buf_len > 0x00ffc0)
		valid_mode21 = snes_validate_infoblock(buffer, 0x00ffc0);
	if (buf_len > 0x40ffc0)
		valid_mode25 = snes_validate_infoblock(buffer, 0x40ffc0);

	/* Images larger than 32mbits are likely ExHiRom */
	if (valid_mode25)
		valid_mode25 += 4;

	if ((valid_mode20 >= valid_mode21) && (valid_mode20 >= valid_mode25))
		retvalue = 0x007fc0;
	else if (valid_mode21 >= valid_mode25)
		retvalue = 0x00ffc0;
	else
		retvalue = 0x40ffc0;

	device->logerror( "\t HiROM/LoROM id: %s (LoROM: %d , HiROM: %d, ExHiROM: %d)\n",
				(retvalue == 0x007fc0) ? "LoROM" :
				(retvalue == 0x00ffc0) ? "HiROM" :
				(retvalue == 0x40ffc0) ? "ExHiROM" : "Other",
				valid_mode20, valid_mode21, valid_mode25);

	return retvalue;
}


static int snes_find_addon_chip( UINT8 *buffer, UINT32 start_offs )
{
	/* Info mostly taken from http://snesemu.black-ship.net/misc/hardware/-from%20nsrt.edgeemu.com-chipinfo.htm */
	switch (buffer[start_offs + 0x16])
	{
		case 0x00:
		case 0x01:
		case 0x02:
			break;

		case 0x03:
			if (buffer[start_offs + 0x15] == 0x30)
				return ADDON_DSP4;
			else
				return ADDON_DSP1;

		case 0x04:
			return ADDON_DSP1;

		case 0x05:
			// DSP2 can be detected by (buffer[start_offs + 0x15] == 0x20)
			// DSP3 is harder to detect, and one has to rely on the manufacturer (Bandai)
			//      by checking (buffer[start_offs + 0x15] == 0x30) && (buffer[start_offs + 0x1a] == 0xb2)
			// in other cases is DSP1, but we do treat all these together...
			if (buffer[start_offs + 0x15] == 0x20)
				return ADDON_DSP2;
			else if ((buffer[start_offs + 0x15] == 0x30) && (buffer[start_offs + 0x1a] == 0xb2))
				return ADDON_DSP3;
			else
				return ADDON_DSP1;

		case 0x13:  // Mario Chip 1
		case 0x14:  // GSU-x
		case 0x15:  // GSU-x
		case 0x1a:  // GSU-1 (21 MHz at start)
			if (buffer[start_offs + 0x15] == 0x20)
				return ADDON_SFX;
			break;

		case 0x25:
			return ADDON_OBC1;

		case 0x32:  // needed by a Sample game (according to ZSNES)
		case 0x34:
		case 0x35:
			if (buffer[start_offs + 0x15] == 0x23)
				return ADDON_SA1;
			break;

		case 0x43:
		case 0x45:
			if (buffer[start_offs + 0x15] == 0x32)
				return ADDON_SDD1;
			break;

		case 0x55:
			if (buffer[start_offs + 0x15] == 0x35)
				return ADDON_SRTC;
			break;

		case 0xe3:
			return ADDON_Z80GB;

		case 0xf3:
			return ADDON_CX4;

		case 0xf5:
			if (buffer[start_offs + 0x15] == 0x30)
				return ADDON_ST018;
			else if (buffer[start_offs + 0x15] == 0x3a)
				return ADDON_SPC7110;
			break;

		case 0xf6:
			/* These Seta ST-01X chips have both 0x30 at 0xffd5,
			 they only differ for the 'size' at 0xffd7 */
			if (buffer[start_offs + 0x17] < 0x0a)
				return ADDON_ST011;
			else
				return ADDON_ST010;

		case 0xf9:
			if (buffer[start_offs + 0x15] == 0x3a)
				return ADDON_SPC7110_RTC;
			break;

		default:
			break;
	}
	return -1;
}


/*-------------------------------------------------
 call load
 -------------------------------------------------*/


bool base_sns_cart_slot_device::call_load()
{
	if (m_cart)
	{
		UINT8 *ROM;
		UINT32 len, offset = 0;
		const char *slot_name;

		/* Check for a header (512 bytes), and skip it if found */
		if (software_entry() == nullptr)
		{
			UINT32 tmplen = length();
			dynamic_buffer tmpROM(tmplen);
			fread(&tmpROM[0], tmplen);
			offset = snes_skip_header(&tmpROM[0], tmplen);
			fseek(offset, SEEK_SET);
		}

		len = (software_entry() == nullptr) ? (length() - offset) : get_software_region_length("rom");

		m_cart->rom_alloc(len, tag());
		ROM = m_cart->get_rom_base();
		if (software_entry() == nullptr)
			fread(ROM, len);
		else
			memcpy(ROM, get_software_region("rom"), len);

		m_cart->rom_map_setup(len);

		// check for on-cart CPU bios
		if (software_entry() != nullptr)
		{
			if (get_software_region("addon"))
			{
				m_cart->addon_bios_alloc(get_software_region_length("addon"));
				memcpy(m_cart->get_addon_bios_base(), get_software_region("addon"), get_software_region_length("addon"));
			}
		}

		// get pcb type
		if (software_entry() == nullptr)
			get_cart_type_addon(ROM, len, m_type, m_addon);
		else
		{
			if ((slot_name = get_feature("slot")) == nullptr)
				m_type = SNES_MODE20;
			else
				m_type = sns_get_pcb_id(slot_name);

			if (m_type == SNES_DSP && len > 0x100000)
					m_type = SNES_DSP_2MB;
		}

		if (software_entry() == nullptr)
			setup_addon_from_fullpath();

		// in carts with an add-on CPU having internal dump, this speeds up access to the internal rom
		// by installing read_bank in address space and mapping m_bios there
		m_cart->speedup_addon_bios_access();

		setup_nvram();

		if (m_cart->get_nvram_size() || m_cart->get_rtc_ram_size())
		{
			UINT32 tot_size = m_cart->get_nvram_size() + m_cart->get_rtc_ram_size();
			dynamic_buffer temp_nvram(tot_size);
			battery_load(&temp_nvram[0], tot_size, 0xff);
			if (m_cart->get_nvram_size())
				memcpy(m_cart->get_nvram_base(), &temp_nvram[0], m_cart->get_nvram_size());
			if (m_cart->get_rtc_ram_size())
				memcpy(m_cart->get_rtc_ram_base(), &temp_nvram[m_cart->get_nvram_size()], m_cart->get_rtc_ram_size());
		}

		//printf("Type %d\n", m_type);

		internal_header_logging(ROM, len);

		return IMAGE_INIT_PASS;
	}

	return IMAGE_INIT_PASS;
}


/*-------------------------------------------------
 call_unload
 -------------------------------------------------*/

void base_sns_cart_slot_device::call_unload()
{
	if (m_cart)
	{
		if (m_cart->get_nvram_size() || m_cart->get_rtc_ram_size())
		{
			UINT32 tot_size = m_cart->get_nvram_size() + m_cart->get_rtc_ram_size();
			dynamic_buffer temp_nvram(tot_size);
			if (m_cart->get_nvram_size())
				memcpy(&temp_nvram[0], m_cart->get_nvram_base(), m_cart->get_nvram_size());
			if (m_cart->get_rtc_ram_size())
				memcpy(&temp_nvram[m_cart->get_nvram_size()], m_cart->get_rtc_ram_base(), m_cart->get_rtc_ram_size());

			battery_save(&temp_nvram[0], tot_size);
		}
	}
}


void base_sns_cart_slot_device::setup_addon_from_fullpath()
{
	// if we already have an add-on bios or if no addon has been detected, we have nothing to do
	if (m_cart->get_addon_bios_size() || m_addon == ADDON_NONE)
		return;

	// check if the add-on dump is appended to the file
	// if this is the case, copy it in m_bios and refresh
	// the rom_bank_map with correct game rom size
	switch (m_addon)
	{
		case ADDON_DSP1:
		case ADDON_DSP1B:
		case ADDON_DSP2:
		case ADDON_DSP3:
		case ADDON_DSP4:
			// check for add-on dump
			if ((m_cart->get_rom_size() & 0x7fff) == 0x2800)
			{
				logerror("Found NEC DSP dump at the bottom of the ROM.\n");
				m_cart->addon_bios_alloc(0x2800);
				memcpy(m_cart->get_addon_bios_base(), m_cart->get_rom_base() + (m_cart->get_rom_size() - 0x2800), 0x2800);
				m_cart->rom_map_setup(m_cart->get_rom_size() - 0x2800);
			}
			// check for byuu's compressed version (swapped order of bytes and stripped fixed 0x00 bytes)
			if ((m_cart->get_rom_size() & 0x7fff) == 0x2000)
			{
				logerror("Found NEC DSP dump (byuu's version) at the bottom of the ROM.\n");
				m_cart->addon_bios_alloc(0x2800);
				for (int i = 0; i < 0x800; i++)
				{
					memcpy(m_cart->get_addon_bios_base() + i * 4 + 2, m_cart->get_rom_base() + (m_cart->get_rom_size() - 0x2000) + i * 3 + 0, 1);
					memcpy(m_cart->get_addon_bios_base() + i * 4 + 1, m_cart->get_rom_base() + (m_cart->get_rom_size() - 0x2000) + i * 3 + 1, 1);
					memcpy(m_cart->get_addon_bios_base() + i * 4 + 0, m_cart->get_rom_base() + (m_cart->get_rom_size() - 0x2000) + i * 3 + 2, 1);
					memset(m_cart->get_addon_bios_base() + i * 4 + 3, 0xff, 1);
				}
				memcpy(m_cart->get_addon_bios_base() + 0x2000, m_cart->get_rom_base() + (m_cart->get_rom_size() - 0x1800), 0x800);
				m_cart->rom_map_setup(m_cart->get_rom_size() - 0x2000);
			}
			break;
		case ADDON_ST010:
		case ADDON_ST011:
			// check for add-on dump
			if ((m_cart->get_rom_size() & 0x3ffff) == 0x11000)
			{
				logerror("Found Seta DSP dump at the bottom of the ROM.\n");
				m_cart->addon_bios_alloc(0x11000);
				memcpy(m_cart->get_addon_bios_base(), m_cart->get_rom_base() + (m_cart->get_rom_size() - 0x11000), 0x11000);
				m_cart->rom_map_setup(m_cart->get_rom_size() - 0x11000);
			}
			// check for byuu's compressed version (swapped order of bytes and stripped fixed 0x00 bytes)
			if ((m_cart->get_rom_size() & 0xffff) == 0xd000)
			{
				logerror("Found Seta DSP dump (byuu's version) at the bottom of the ROM.\n");
				m_cart->addon_bios_alloc(0x11000);
				for (int i = 0; i < 0x4000; i++)
				{
					memcpy(m_cart->get_addon_bios_base() + i * 4 + 2, m_cart->get_rom_base() + (m_cart->get_rom_size() - 0xd000) + i * 3 + 0, 1);
					memcpy(m_cart->get_addon_bios_base() + i * 4 + 1, m_cart->get_rom_base() + (m_cart->get_rom_size() - 0xd000) + i * 3 + 1, 1);
					memcpy(m_cart->get_addon_bios_base() + i * 4 + 0, m_cart->get_rom_base() + (m_cart->get_rom_size() - 0xd000) + i * 3 + 2, 1);
					memset(m_cart->get_addon_bios_base() + i * 4 + 3, 0xff, 1);
				}
				memcpy(m_cart->get_addon_bios_base() + 0x10000, m_cart->get_rom_base() + (m_cart->get_rom_size() - 0xc000), 0x1000);
				m_cart->rom_map_setup(m_cart->get_rom_size() - 0xd000);
			}
			break;
		case ADDON_CX4:
			if ((m_cart->get_rom_size() & 0x7fff) == 0x0c00)
			{
				logerror("Found CX4 dump at the bottom of the ROM.\n");
				m_cart->addon_bios_alloc(0x0c00);
				memcpy(m_cart->get_addon_bios_base(), m_cart->get_rom_base() + (m_cart->get_rom_size() - 0x0c00), 0x0c00);
				m_cart->rom_map_setup(m_cart->get_rom_size() - 0x0c00);
			}
			break;
		case ADDON_ST018:
			if ((m_cart->get_rom_size() & 0x3ffff) == 0x28000)
			{
				logerror("Found ST018 dump at the bottom of the ROM.\n");
				m_cart->addon_bios_alloc(0x28000);
				memcpy(m_cart->get_addon_bios_base(), m_cart->get_rom_base() + (m_cart->get_rom_size() - 0x28000), 0x28000);
				m_cart->rom_map_setup(m_cart->get_rom_size() - 0x28000);
			}
			break;
	}

	// otherwise, we need to use the legacy versions including DSP dump in device romset
	if (!m_cart->get_addon_bios_size())
	{
		std::string region = std::string(m_cart->device().tag()).append(":addon");
		UINT8 *ROM;

		switch (m_addon)
		{
			case ADDON_DSP1:
				ROM = machine().root_device().memregion(region.c_str())->base();
				m_cart->addon_bios_alloc(0x2800);
				memcpy(m_cart->get_addon_bios_base(), ROM, 0x2800);
				break;
			case ADDON_DSP1B:
				ROM = machine().root_device().memregion(region.c_str())->base();
				m_cart->addon_bios_alloc(0x2800);
				memcpy(m_cart->get_addon_bios_base(), ROM, 0x2800);
				break;
			case ADDON_DSP2:
				ROM = machine().root_device().memregion(region.c_str())->base();
				m_cart->addon_bios_alloc(0x2800);
				memcpy(m_cart->get_addon_bios_base(), ROM, 0x2800);
				break;
			case ADDON_DSP3:
				ROM = machine().root_device().memregion(region.c_str())->base();
				m_cart->addon_bios_alloc(0x2800);
				memcpy(m_cart->get_addon_bios_base(), ROM, 0x2800);
				break;
			case ADDON_DSP4:
				ROM = machine().root_device().memregion(region.c_str())->base();
				m_cart->addon_bios_alloc(0x2800);
				memcpy(m_cart->get_addon_bios_base(), ROM, 0x2800);
				break;
			case ADDON_ST010:
				ROM = machine().root_device().memregion(region.c_str())->base();
				m_cart->addon_bios_alloc(0x11000);
				memcpy(m_cart->get_addon_bios_base(), ROM, 0x11000);
				break;
			case ADDON_ST011:
				ROM = machine().root_device().memregion(region.c_str())->base();
				m_cart->addon_bios_alloc(0x11000);
				memcpy(m_cart->get_addon_bios_base(), ROM, 0x11000);
				break;
		}
	}

}

void base_sns_cart_slot_device::setup_nvram()
{
	UINT8 *ROM = (UINT8 *)m_cart->get_rom_base();
	UINT32 size = 0;
	if (software_entry() == nullptr)
	{
		int hilo_mode = snes_find_hilo_mode(this, ROM, m_cart->get_rom_size());
		UINT8 sram_size = (m_type == SNES_SFX) ? (ROM[0x00ffbd] & 0x07) : (ROM[hilo_mode + 0x18] & 0x07);
		if (sram_size)
		{
			UINT32 max = (hilo_mode == 0x007fc0) ? 0x80000 : 0x20000;   // MODE20 vs MODE21
			size = 1024 << sram_size;
			if (size > max)
				size = max;
		}
	}
	else
	{
		if (get_software_region("nvram"))
			size = get_software_region_length("nvram");
	}

	if (size)
		m_cart->nvram_alloc(size);

	if (m_type == SNES_STROM)
		m_cart->nvram_alloc(0x20000);
	if (m_type == SNES_BSX)
		m_cart->nvram_alloc(0x8000);

	// setup also RTC SRAM, when needed (to be removed when RTCs are converted to devices)
	if (m_type == SNES_SRTC)
		m_cart->rtc_ram_alloc(13);
	if (m_type == SNES_SPC7110_RTC)
		m_cart->rtc_ram_alloc(16);
}



/*-------------------------------------------------
 call softlist load
 -------------------------------------------------*/

bool base_sns_cart_slot_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	load_software_part_region(*this, swlist, swname, start_entry );
	return TRUE;
}

void base_sns_cart_slot_device::get_cart_type_addon(UINT8 *ROM, UINT32 len, int &type, int &addon)
{
	// First, look if the cart is HiROM or LoROM (and set snes_cart accordingly)
	int hilo_mode = snes_find_hilo_mode(this,ROM, len);

	switch (hilo_mode)
	{
		case 0x007fc0: // LoRom & ExLoRom
			type = SNES_MODE20;
			break;
		case 0x00ffc0: // HiRom
		case 0x40ffc0: // ExHiRom
			type = SNES_MODE21;
			break;
		default:
			break;
	}

	// detect Sufami Turbo...
	if (type == SNES_MODE20 && !memcmp(ROM, "BANDAI SFC-ADX", 14))
	{
		if (!memcmp(ROM + 16, "SFC-ADX BACKUP", 14))
			type = SNES_SUFAMITURBO;
		else
			type = SNES_STROM;
	}

	// detect BS-X Base Cart
	if (!memcmp(ROM + hilo_mode, "Satellaview BS-X     ", 21))
		type = SNES_BSX;
	// Detect BS-X Flash Cart
	if ((ROM[hilo_mode + 0x13] == 0x00 || ROM[hilo_mode + 0x13] == 0xff) && ROM[hilo_mode + 0x14] == 0x00)
	{
		UINT8 n15 = ROM[hilo_mode + 0x15];
		if (n15 == 0x00 || n15 == 0x80 || n15 == 0x84 || n15 == 0x9c || n15 == 0xbc || n15 == 0xfc)
		{
			if (ROM[hilo_mode + 0x1a] == 0x33 || ROM[hilo_mode + 0x1a] == 0xff)
				type = SNES_BSMEMPAK;
		}
	}

	// check for add-on chips...
	if (len >= hilo_mode + 0x1a)
	{
		addon = snes_find_addon_chip(ROM, hilo_mode);
		if (addon != -1)
		{
			// m_type handles DSP1,2,3 in the same way, but snes_add requires them to be separate...
			switch (addon)
			{
				case ADDON_CX4:
					type = SNES_CX4;
					break;
				case ADDON_DSP1:
				case ADDON_DSP1B:
				case ADDON_DSP2:
				case ADDON_DSP3:
					if (type == SNES_MODE20 && len > 0x100000)
						type = SNES_DSP_2MB;
					else if (type == SNES_MODE21)
						type = SNES_DSP_MODE21;
					else
						type = SNES_DSP;
					break;
				case ADDON_DSP4:
					type = SNES_DSP4;
					break;
				case ADDON_OBC1:
					type = SNES_OBC1;
					break;
				case ADDON_SA1:
					type = SNES_SA1;
					break;
				case ADDON_SDD1:
					type = SNES_SDD1;
					break;
				case ADDON_SFX:
					type = SNES_SFX;
					break;
				case ADDON_SPC7110:
					type = SNES_SPC7110;
					break;
				case ADDON_SPC7110_RTC:
					type = SNES_SPC7110_RTC;
					break;
				case ADDON_ST010:
					type = SNES_ST010;
					break;
				case ADDON_ST011:
					type = SNES_ST011;
					break;
				case ADDON_ST018:
					type = SNES_ST018;
					break;
				case ADDON_SRTC:
					type = SNES_SRTC;
					break;
				case ADDON_Z80GB:
					type = SNES_Z80GB;
					break;
			}
		}
	}
}

/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

std::string base_sns_cart_slot_device::get_default_card_software()
{
	bool fullpath = open_image_file(mconfig().options());

	if (fullpath)
	{
		const char *slot_string;
		UINT32 offset;
		UINT32 len = core_fsize(m_file);
		dynamic_buffer rom(len);
		int type = 0, addon = 0;

		core_fread(m_file, &rom[0], len);

		offset = snes_skip_header(&rom[0], len);

		get_cart_type_addon(&rom[offset], len - offset, type, addon);
		// here we're from fullpath, so check if it's a DSP game which needs legacy device (i.e. it has no appended DSP dump)
		switch (addon)
		{
			case ADDON_DSP1:
				if ((len & 0x7fff) != 0x2800 && (len & 0x7fff) != 0x2000)
				{
					if (type == SNES_DSP_MODE21)
						type = SNES_DSP1_MODE21_LEG;
					else
						type = SNES_DSP1_LEG;
				}
				break;
			case ADDON_DSP1B:
				if ((len & 0x7fff) != 0x2800 && (len & 0x7fff) != 0x2000)
					type = SNES_DSP1B_LEG;
				break;
			case ADDON_DSP2:
				if ((len & 0x7fff) != 0x2800 && (len & 0x7fff) != 0x2000)
					type = SNES_DSP2_LEG;
				break;
			case ADDON_DSP3:
				if ((len & 0x7fff) != 0x2800 && (len & 0x7fff) != 0x2000)
					type = SNES_DSP3_LEG;
				break;
			case ADDON_DSP4:
				if ((len & 0x7fff) != 0x2800 && (len & 0x7fff) != 0x2000)
					type = SNES_DSP4_LEG;
				break;
			case ADDON_ST010:
				if ((len & 0x3ffff) != 0x11000 && (len & 0xffff) != 0xd000)
					type = SNES_ST010_LEG;
				break;
			case ADDON_ST011:
				if ((len & 0x3ffff) != 0x11000 && (len & 0xffff) != 0xd000)
					type = SNES_ST011_LEG;
				break;
		}

		slot_string = sns_get_slot(type);

		clear();

		return std::string(slot_string);
	}

	return software_get_default_slot("lorom");
}


/*-------------------------------------------------
 read
 -------------------------------------------------*/

READ8_MEMBER(base_sns_cart_slot_device::read_l)
{
	if (m_cart)
		return m_cart->read_l(space, offset);
	else
		return 0xff;
}

READ8_MEMBER(base_sns_cart_slot_device::read_h)
{
	if (m_cart)
		return m_cart->read_h(space, offset);
	else
		return 0xff;
}

READ8_MEMBER(base_sns_cart_slot_device::read_ram)
{
	if (m_cart)
		return m_cart->read_ram(space, offset);
	else
		return 0xff;
}

READ8_MEMBER(base_sns_cart_slot_device::chip_read)
{
	if (m_cart)
		return m_cart->chip_read(space, offset);
	else
		return 0xff;
}

/*-------------------------------------------------
 write
 -------------------------------------------------*/

WRITE8_MEMBER(base_sns_cart_slot_device::write_l)
{
	if (m_cart)
		m_cart->write_l(space, offset, data);
}

WRITE8_MEMBER(base_sns_cart_slot_device::write_h)
{
	if (m_cart)
		m_cart->write_h(space, offset, data);
}

WRITE8_MEMBER(base_sns_cart_slot_device::write_ram)
{
	if (m_cart)
		m_cart->write_ram(space, offset, data);
}

WRITE8_MEMBER(base_sns_cart_slot_device::chip_write)
{
	if (m_cart)
		m_cart->chip_write(space, offset, data);
}


/*-------------------------------------------------
 Internal header logging
 -------------------------------------------------*/

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

#define UNK  "UNKNOWN"

void base_sns_cart_slot_device::internal_header_logging(UINT8 *ROM, UINT32 len)
{
	static const char *const cart_types[] =
	{
		"ROM (LoROM)",
		"ROM (HiROM)",
		"ROM (ExLoROM)",
		"ROM (ExHiROM)",
		"ROM, CX4",
		"ROM, DSP-1,2,3 (LoROM)",
		"ROM, DSP-1 (LoROM 2MB)",
		"ROM, DSP-1 (HiROM)",
		"ROM, DSP-4",
		"ROM, OBC-1",
		"ROM, SA-1",
		"ROM, S-DD1",
		"ROM, Super FX / FX2",
		"ROM, SPC7110",
		"ROM, SPC7110, RTC",
		"ROM, S-RTC",
		"ROM, Seta ST-010",
		"ROM, Seta ST-011",
		"ROM, Seta ST-018",
		"ROM, Z80GB (Super Game Boy)",
		"ROM, BS-X Base cart",
		"ROM, BS-X compatible (LoROM)",
		"ROM, BS-X compatible (HiROM)",
		"BS-X memory pack",
		"ROM, Sufami Turbo",
		"Sufami Turbo cart",
		// pirate cart types which are not recognized from fullpath
		UNK,
		UNK,
		UNK
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

	int hilo_mode = snes_find_hilo_mode(this,ROM, len);
	char title[21], rom_id[4], company_id[2];
	int type = 0, company, addon, has_ram = 0, has_sram = 0;
	switch (hilo_mode)
	{
		case 0x007fc0:
			if ((ROM[0x007fd5] == 0x32) || (len > 0x401000))
				type = SNES_MODE22;  // ExLoRom
			else
				type = SNES_MODE20;  // LoRom
			type = SNES_MODE20;  // LoRom & ExLoRom
			break;
		case 0x00ffc0:
			type = SNES_MODE21;  // HiRom
			break;
		case 0x40ffc0:
			type = SNES_MODE25;  // ExHiRom
			break;
		default:
			break;
	}

	// detect Sufami Turbo...
	if (type == SNES_MODE20 && !memcmp(ROM, "BANDAI SFC-ADX", 14))
	{
		if (!memcmp(ROM + 16, "SFC-ADX BACKUP", 14))
			type = SNES_SUFAMITURBO;
		else
			type = SNES_STROM;
	}

	// detect BS-X Base Cart
	if (!memcmp(ROM + hilo_mode, "Satellaview BS-X     ", 21))
		type = SNES_BSX;
	// Detect BS-X Flash Cart
	if ((ROM[hilo_mode + 0x13] == 0x00 || ROM[hilo_mode + 0x13] == 0xff) && ROM[hilo_mode + 0x14] == 0x00)
	{
		UINT8 n15 = ROM[hilo_mode + 0x15];
		if (n15 == 0x00 || n15 == 0x80 || n15 == 0x84 || n15 == 0x9c || n15 == 0xbc || n15 == 0xfc)
		{
			if (ROM[hilo_mode + 0x1a] == 0x33 || ROM[hilo_mode + 0x1a] == 0xff)
				type = SNES_BSMEMPAK;
		}
	}

	addon = snes_find_addon_chip(ROM, hilo_mode);
	if (addon != -1)
	{
		if (type == SNES_MODE20 && addon == SNES_DSP)
		{
			if (len > 0x100000)
				type = SNES_DSP_2MB;
			else
				type = SNES_DSP;
		}
		else if (type == SNES_MODE21 && addon == SNES_DSP)
			type = SNES_DSP_MODE21;
		else
			type = addon;
	}

	/* Company */
	for (int i = 0; i < 2; i++)
		company_id[i] = ROM[hilo_mode - 0x10 + i];
	company = (char_to_int_conv(company_id[0]) << 4) + char_to_int_conv(company_id[1]);
	if (company == 0)
		company = ROM[hilo_mode + 0x1a];

	/* ROM ID */
	for(int i = 0; i < 4; i++ )
		rom_id[i] = ROM[hilo_mode - 0x0e + i];

	/* Title */
	for(int i = 0; i < 21; i++ )
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

	logerror( "ROM DETAILS\n" );
	logerror( "===========\n\n" );
	logerror( "\tTotal blocks:  0x%x\n", len);
	logerror( "\tROM bank size: %s \n",
				(type == SNES_MODE20) ? "LoROM" :
				(type == SNES_MODE21) ? "HiROM" :
				(type == SNES_MODE22) ? "ExLoROM" :
				(type == SNES_MODE25) ? "ExHiROM" : "Other (BSX or ST)" );
	logerror( "\tCompany:       %s [%.2s]\n", companies[company], company_id );
	logerror( "\tROM ID:        %.4s\n\n", rom_id );

	logerror( "HEADER DETAILS\n" );
	logerror( "==============\n\n" );
	logerror( "\tName:          %.21s\n", title );
	logerror( "\tSpeed:         %s [%d]\n", (ROM[hilo_mode + 0x15] & 0xf0) ? "FastROM" : "SlowROM", (ROM[hilo_mode + 0x15] & 0xf0) >> 4);
	logerror( "\tBank size:     %s [%d]\n", (ROM[hilo_mode + 0x15] & 0xf) ? "HiROM" : "LoROM", ROM[hilo_mode + 0x15] & 0xf);

	logerror( "\tType:          %s", cart_types[type]);
	if (has_ram)
		logerror( ", RAM");
	if (has_sram)
		logerror( ", SRAM");
	logerror( " [%d]\n", ROM[hilo_mode + 0x16]);

	logerror( "\tSize:          %d megabits [%d]\n", 1 << (ROM[hilo_mode + 0x17] - 7), ROM[hilo_mode + 0x17]);
	logerror( "\tSRAM:          %d kilobits [%d]\n", ROM[hilo_mode + 0x18] * 8, ROM[hilo_mode + 0x18] );
	if (ROM[hilo_mode + 0x19] < ARRAY_LENGTH(countries))
		logerror( "\tCountry:       %s [%d]\n", countries[ROM[hilo_mode + 0x19]], ROM[hilo_mode + 0x19]);
	else
		logerror( "\tCountry:       Unknown [%d]\n", ROM[hilo_mode + 0x19]);
	logerror( "\tLicense:       %s [%X]\n", companies[ROM[hilo_mode + 0x1a]], ROM[hilo_mode + 0x1a]);
	logerror( "\tVersion:       1.%d\n", ROM[hilo_mode + 0x1b]);
	logerror( "\tInv Checksum:  %X %X\n", ROM[hilo_mode + 0x1d], ROM[hilo_mode + 0x1c]);
	logerror( "\tChecksum:      %X %X\n", ROM[hilo_mode + 0x1f], ROM[hilo_mode + 0x1e]);
	logerror( "\tNMI Address:   %2X%2Xh\n", ROM[hilo_mode + 0x3b], ROM[hilo_mode + 0x3a]);
	logerror( "\tStart Address: %2X%2Xh\n\n", ROM[hilo_mode + 0x3d], ROM[hilo_mode + 0x3c]);

	logerror( "\tMode: %d\n", type);
}
