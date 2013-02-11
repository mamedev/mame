/***********************************************************************************************************


    MegaDrive cart emulation
    (through slot devices)

    The driver exposes address ranges
    0x000000-0x7fffff to read/write (typically cart data, possibly banked, and some protection)
    0x130000-0x1300ff to read_a13/write_a13 (typically SRAM toggling or protection)
    0x150000-0x1500ff to read_a15/write_a15 (for SVP registers, not converted yet to slots)

    currently available slot devices:
    md_rom: standard carts + carts with NVRAM (SRAM/FRAM) + pirate carts with protection & bankswitch
    md_eeprom: carts + I2C EEPROM (EEPROM device hooked up only, support to be completed)
    md_jcart: Codemasters J-Cart & J-Cart+SEPROM (again, EEPROM device hooked up only, support to be completed)
    md_sk: Sonic & Knuckles pass-thorugh cart (enables a second slot to mount any other cart)
    md_stm95: cart + STM95 EEPROM (e.g. Pier Solar)


    Cart Mirroring (based Eke's research)

    MD Cartridge area is mapped to $000000-$3fffff: when accessing ROM, 68k address lines A1 to A21 can be
    used by the internal cartridge hardware to decode full 4MB address range.
    Depending on ROM total size and additional decoding hardware, some address lines might be ignored,
    resulting in ROM mirroring.

    Cartridges typically use either 8-bits (x2) or 16-bits (x1, x2) Mask ROM chips, each chip size is a
    factor of 2 bytes.
    When one chip ROM1 of size 2^N is present, it is generally mirrored each 2^N bytes so that read access
    to cart area sees the sequence ROM1,ROM1,ROM1,... (up to 4MB)
    When two chips ROM1 & ROM2 are present and the whole size is 2^N, then the block ROM1+ROM2 is mirrored
    in the cart area, and reads see the sequence ROM1+ROM2,ROM1+ROM2,... (up to 4MB)
    When two chips ROM1 & ROM2 are present and the whole size is not 2^N (e.g. because ROM1 and ROM2 have
    different sizes), then the area between the end of ROM2 and next power 2^N is generally ignored, and
    reads see the sequence ROM1,ROM2,XXXX,ROM1,ROM2,XXXX... (up to 4MB)

    At loading time we first compute first power 2^N larger than cart size (see get_padded_size function),
    we allocate such a size for ROM and we fill of 0xff the area between end of dump and 2^N.
    Then we handle mirroring by creating a rom_bank_map[] (see rom_map_setup function) which points each
    access in 0x000000-0x400000 to the correct 64K ROM bank.

 ***********************************************************************************************************/


#include "emu.h"
#include "machine/md_slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type MD_CART_SLOT = &device_creator<md_cart_slot_device>;
const device_type PICO_CART_SLOT = &device_creator<pico_cart_slot_device>;
const device_type MD_SUBCART_SLOT = &device_creator<md_subcart_slot_device>;


//**************************************************************************
//    MD cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  device_md_cart_interface - constructor
//-------------------------------------------------

device_md_cart_interface::device_md_cart_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_rom(NULL),
		m_nvram(NULL),
		m_rom_size(0),
		m_nvram_size(0)
{
}


//-------------------------------------------------
//  ~device_md_cart_interface - destructor
//-------------------------------------------------

device_md_cart_interface::~device_md_cart_interface()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_md_cart_interface::rom_alloc(running_machine &machine, size_t size)
{
	if (m_rom == NULL)
	{
		m_rom = auto_alloc_array_clear(machine, UINT16, size/sizeof(UINT16));
		m_rom_size = size;
	}
}


//-------------------------------------------------
//  nvram_alloc - alloc the space for the nvram
//-------------------------------------------------

void device_md_cart_interface::nvram_alloc(running_machine &machine, size_t size)
{
	if (m_nvram == NULL)
	{
		m_nvram = auto_alloc_array_clear(machine, UINT16, size/sizeof(UINT16));
		m_nvram_size = size;
	}
}

//-------------------------------------------------
//  rom_map_setup - setup map of rom banks in 64K
//  blocks, so to simplify ROM mirroring
//-------------------------------------------------

void device_md_cart_interface::rom_map_setup(UINT32 size)
{
	int i;
	// setup the rom_bank_map array to faster ROM read
	for (i = 0; i < size / 0x10000; i++)
		rom_bank_map[i] = i;

	// fill up remaining blocks with mirrors
	while (i % 64)
	{
		int j = 0, repeat_banks;
		while ((i % (64 >> j)) && j < 7)
			j++;
		repeat_banks = i % (64 >> (j - 1));
		for (int k = 0; k < repeat_banks; k++)
			rom_bank_map[i + k] = rom_bank_map[i + k - repeat_banks];
		i += repeat_banks;
	}

// check bank map!
//  for (i = 0; i < 64; i++)
//  {
//      printf("bank %3d = %3d\t", i, rom_bank_map[i]);
//      if ((i%8) == 7)
//          printf("\n");
//  }
}

//-------------------------------------------------
//
//
//-------------------------------------------------

UINT32 device_md_cart_interface::get_padded_size(UINT32 size)
{
	UINT32 pad_size = 0x10000;
	while (size > pad_size)
		pad_size <<= 1;

	if (pad_size < 0x800000 && size < pad_size)
		return pad_size;
	else
		return size;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  base_md_cart_slot_device - constructor
//-------------------------------------------------
base_md_cart_slot_device::base_md_cart_slot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
						device_t(mconfig, type, name, tag, owner, clock),
						device_image_interface(mconfig, *this),
						device_slot_interface(mconfig, *this),
						m_type(SEGA_STD)
{
}

md_cart_slot_device::md_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						base_md_cart_slot_device(mconfig, MD_CART_SLOT, "MD Cartridge Slot", tag, owner, clock)
{
}

md_subcart_slot_device::md_subcart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						base_md_cart_slot_device(mconfig, MD_SUBCART_SLOT, "MD Cartridge SubSlot", tag, owner, clock)
{
}

pico_cart_slot_device::pico_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						base_md_cart_slot_device(mconfig, PICO_CART_SLOT, "Pico Cartridge Slot", tag, owner, clock)
{
}

//-------------------------------------------------
//  base_md_cart_slot_device - destructor
//-------------------------------------------------

base_md_cart_slot_device::~base_md_cart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void base_md_cart_slot_device::device_start()
{
	m_cart = dynamic_cast<device_md_cart_interface *>(get_card_device());
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void base_md_cart_slot_device::device_config_complete()
{
	// set brief and instance name
	update_names();
}


//-------------------------------------------------
//  MD PCB
//-------------------------------------------------


struct md_slot
{
	int                     pcb_id;
	const char              *slot_option;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const md_slot slot_list[] =
{
	{ SEGA_SK, "rom_sk" },
	{ SEGA_STD, "rom_svp"},
//  { SEGA_SVP, "rom_svp"},     // not ready yet...

	{ SEGA_SRAM, "rom_sram" },
	{ SEGA_FRAM, "rom_fram" },
	{ HARDBALL95, "rom_hardbl95" },
	{ BEGGAR, "rom_beggar"},

	{ SEGA_EEPROM, "rom_eeprom" },
	{ NBA_JAM, "rom_nbajam" },
	{ NBA_JAM_TE, "rom_nbajamte" },
	{ NFL_QB_96, "rom_nflqb" },
	{ C_SLAM, "rom_cslam" },
	{ EA_NHLPA, "rom_nhlpa" },
	{ BRIAN_LARA, "rom_blara" },

	{ CM_JCART, "rom_jcart" },
	{ CODE_MASTERS, "rom_codemast" },
	{ CM_MM96, "rom_mm96" },

	{ PSOLAR, "rom_stm95"},

	{ SSF2, "rom_ssf2" },
	{ RADICA, "rom_radica" },
//  { GAME_KANDUME, "rom_gkand" },  // what's needed by this?

	{ BUGSLIFE, "rom_bugs" },
	{ CHINFIGHT3, "rom_chinf3" },
	{ ELFWOR, "rom_elfwor" },
	{ KAIJU, "rom_pokestad" },
	{ KOF98, "rom_kof98" },
	{ KOF99, "rom_kof99" },
	{ LIONK2, "rom_lion2" },
	{ LIONK3, "rom_lion3" },
	{ MC_PIRATE, "rom_mcpir" },
	{ MJLOVER, "rom_mjlov" },
	{ MULAN, "rom_mulan"},
	{ POKEMON2, "rom_poke2"},
	{ REALTEC, "rom_realtec" },
	{ REDCL_EN, "rom_redcl" },
	{ REDCLIFF, "rom_redcl" },
	{ ROCKMANX3, "rom_rx3" },
	{ SBUBBOB, "rom_sbubl" },
	{ SMB, "rom_smb" },
	{ SMB2, "rom_smb2" },
	{ SMOUSE, "rom_smouse" },
	{ SOULBLAD, "rom_soulblad" },
	{ SQUIRRELK, "rom_squir" },
	{ TOPFIGHTER, "rom_topf" },

	{ SEGA_SRAM_FULLPATH, "rom_sram" },
	{ SEGA_SRAM_FALLBACK, "rom_sramsafe" }
};

static int md_get_pcb_id(const char *slot)
{
	for (int i = 0; i < ARRAY_LENGTH(slot_list); i++)
	{
		if (!mame_stricmp(slot_list[i].slot_option, slot))
			return slot_list[i].pcb_id;
	}

	return SEGA_STD;
}

static const char *md_get_slot(int type)
{
	for (int i = 0; i < ARRAY_LENGTH(slot_list); i++)
	{
		if (slot_list[i].pcb_id == type)
			return slot_list[i].slot_option;
	}

	return "rom";
}


/*-------------------------------------------------
 SRAM handling
 -------------------------------------------------*/

/*-------------------------------------------------
 call load
 -------------------------------------------------*/


bool base_md_cart_slot_device::call_load()
{
	if (m_cart)
	{
		m_type = SEGA_STD;
		int res;

		// STEP 1: load the file image and keep a copy for later banking
		// STEP 2: identify the cart type
		// The two steps are carried out differently if we are loading from a list or not
		if (software_entry() == NULL)
			res = load_nonlist();
		else
			res = load_list();

		//printf("cart type: %d\n", m_type);

		if (res == IMAGE_INIT_PASS)
		{
			// STEP 3: install memory handlers for this type of cart
			setup_custom_mappers();

			// STEP 4: take care of SRAM.
			setup_nvram();

			if (m_cart->get_nvram_size())
				battery_load(m_cart->get_nvram_base(), m_cart->get_nvram_size(), 0xff);
		}

		return res;

	}

	return IMAGE_INIT_PASS;
}


int base_md_cart_slot_device::load_list()
{
	UINT16 *ROM;
	UINT32 length = get_software_region_length("rom");
	const char  *slot_name;

	// if cart size is not (2^n * 64K), the system will see anyway that size so we need to alloc a bit more space
	length = m_cart->get_padded_size(length);

	m_cart->rom_alloc(machine(), length);
	ROM = m_cart->get_rom_base();
	memcpy(ROM, get_software_region("rom"), get_software_region_length("rom"));

	// if we allocated a ROM larger that the file (e.g. due to uneven cart size), set remaining space to 0xff
	if (length > get_software_region_length("rom"))
		memset(ROM + get_software_region_length("rom")/2, 0xffff, (length - get_software_region_length("rom"))/2);

	if ((slot_name = get_feature("slot")) == NULL)
		m_type = SEGA_STD;
	else
		m_type = md_get_pcb_id(slot_name);

	// handle mirroring of ROM, unless it's SSF2 or Pier Solar
	if (m_type != SSF2 && m_type != PSOLAR)
		m_cart->rom_map_setup(length);

	return IMAGE_INIT_PASS;
}


/*************************************
 *  Helper function: Detect SMD file
 *************************************/

/* code taken directly from GoodGEN by Cowering */
static int genesis_is_SMD(unsigned char *buf,unsigned int len)
{
	if (buf[0x2080] == 'S' && buf[0x80] == 'E' && buf[0x2081] == 'G' && buf[0x81] == 'A')
		return 1;

	/* aq quiz */
	if (!strncmp("UZ(-01  ", (const char *) &buf[0xf0], 8))
		return 1;

	/* Phelios USA redump */
	/* target earth */
	/* klax (namcot) */
	if (buf[0x2080] == ' ' && buf[0x0080] == 'S' && buf[0x2081] == 'E' && buf[0x0081] == 'G')
		return 1;

	/* jap baseball 94 */
	if (!strncmp("OL R-AEAL", (const char *) &buf[0xf0], 9))
		return 1;

	/* devilish Mahjong Tower */
	if (!strncmp("optrEtranet", (const char *) &buf[0xf3], 11))
		return 1;

	/* golden axe 2 beta */
	if (buf[0x0100] == 0x3c && buf[0x0101] == 0 && buf[0x0102] == 0 && buf[0x0103] == 0x3c)
		return 1;

	/* omega race */
	if (!strncmp("OEARC   ", (const char *) &buf[0x90], 8))
		return 1;

	/* budokan beta */
	if ((len >= 0x6708 + 8) && !strncmp(" NTEBDKN", (const char *) &buf[0x6708], 8))
		return 1;

	/* cdx pro 1.8 bios */
	if (!strncmp("so fCXP", (const char *) &buf[0x2c0], 7))
		return 1;

	/* ishido (hacked) */
	if (!strncmp("sio-Wyo ", (const char *) &buf[0x0090], 8))
		return 1;

	/* onslaught */
	if (!strncmp("SS  CAL ", (const char *) &buf[0x0088], 8))
		return 1;

	/* tram terror pirate */
	if ((len >= 0x3648 + 8) && !strncmp("SG NEPIE", (const char *) &buf[0x3648], 8))
		return 1;

	/* breath of fire 3 chinese */
	if (buf[0x0007] == 0x1c && buf[0x0008] == 0x0a && buf[0x0009] == 0xb8 && buf[0x000a] == 0x0a)
		return 1;

	/*tetris pirate */
	if ((len >= 0x1cbe + 5) && !strncmp("@TTI>", (const char *) &buf[0x1cbe], 5))
		return 1;

	return 0;
}

/*************************************
 *  Loading a cart image *not* from
 *  softlist
 *************************************/

int base_md_cart_slot_device::load_nonlist()
{
	unsigned char *ROM, *tmpROM;
	UINT32 len = m_cart->get_padded_size(length()); // if cart size is not (2^n * 64K), the system will see anyway that size so we need to alloc a bit more space

	// this contains an hack for SSF2: its current bankswitch code needs larger rom space to work
	m_cart->rom_alloc(machine(), (len == 0x500000) ? 0x900000 : len);

	ROM = (unsigned char *)m_cart->get_rom_base();
	tmpROM = global_alloc_array(unsigned char, len);

	// STEP 1: determine the file type (SMD? MD? BIN?)
	fread(tmpROM, len);

	/* is this a SMD file? */
	if (genesis_is_SMD(&tmpROM[0x200], len))
	{
		mame_printf_debug("SMD!\n");

		for (int ptr = 0; ptr < (len - 0x200) / 0x2000; ptr += 2)
		{
			for (int x = 0; x < 0x2000; x++)
			{
				*ROM++ = *(tmpROM + 0x200 + ((ptr + 1) * 0x2000) + x);
				*ROM++ = *(tmpROM + 0x200 + ((ptr + 0) * 0x2000) + x);
			}
		}

		len -= 0x200;
	}
	/* is this a MD file? */
	else if ((tmpROM[0x80] == 'E') && (tmpROM[0x81] == 'A') && (tmpROM[0x82] == 'M' || tmpROM[0x82] == 'G'))
	{
		mame_printf_debug("MD!\n");

		for (int ptr = 0; ptr < len; ptr += 2)
		{
			ROM[ptr] = tmpROM[(len >> 1) + (ptr >> 1)];
			ROM[ptr + 1] = tmpROM[(ptr >> 1)];
		}
	}
	/* BIN it is, then */
	else
	{
		fseek(0, SEEK_SET);
		fread(ROM, len);
	}

	global_free(tmpROM);

	// if we allocated a ROM larger that the file (e.g. due to uneven cart size), set remaining space to 0xff
	if (len > length())
		memset(m_cart->get_rom_base() + length()/2, 0xffff, (len - length())/2);

	// STEP 2: determine the cart type (to deal with pirate mappers & eeprom)
	m_type = get_cart_type(ROM, length());

	// handle mirroring of ROM, unless it's SSF2 or Pier Solar
	if (m_type != SSF2 && m_type != PSOLAR)
		m_cart->rom_map_setup(len);

#ifdef LSB_FIRST
	unsigned char fliptemp;
	// is this really needed nowadays?
	for (int ptr = 0; ptr < len; ptr += 2)
	{
		fliptemp = ROM[ptr];
		ROM[ptr] = ROM[ptr+1];
		ROM[ptr+1] = fliptemp;
	}
#endif

	return IMAGE_INIT_PASS;
}

/*-------------------------------------------------
 call_unloadload
 -------------------------------------------------*/

void base_md_cart_slot_device::call_unload()
{
	if (m_cart->get_nvram_size())
		battery_save(m_cart->get_nvram_base(), m_cart->get_nvram_size());
}


void base_md_cart_slot_device::setup_custom_mappers()
{
	UINT16 *ROM16 = m_cart->get_rom_base();

	switch (m_type)
	{
		case SSF2:
			// copy the image in 0x400000-0x900000 and keep the beginning for bankswitch
			for (int x = 0x500000/2; x > 0; x--)
				ROM16[x + 0x400000/2 - 1] = ROM16[x - 1];
			for (int x = 0; x < 0x400000/2; x++)
				ROM16[x] = ROM16[x + 0x400000/2];
			break;
		case REDCL_EN:
			// decrypt
			for (int x = 0; x < 0x200000/2; x++)
				ROM16[x] = ROM16[x + 2] ^ 0x4040;
			break;

		// patch out protection in a bunch of titles...
		case POKEMON2:
			/*todo: emulate protection instead
			 006036:e000
			 002540:6026
			 001ed0:6026
			 002476:6022
			 */
			ROM16[0x06036/2] = 0xe000;
			ROM16[0x02540/2] = 0x6026;
			ROM16[0x01ed0/2] = 0x6026;
			ROM16[0x02476/2] = 0x6022;
			ROM16[0x7e300/2] = 0x60fe;
			break;
		case MULAN:
			/*todo: emulate protection instead
			 006036:e000
			 +more?
			 */
			//  ROM16[0x01ed0/2] = 0xe000;
			//  ROM16[0x02540/2] = 0xe000;
			ROM16[0x06036/2] = 0xe000;
			break;
	}
}

void base_md_cart_slot_device::setup_nvram()
{
	UINT8 *ROM = (UINT8 *)m_cart->get_rom_base();
	m_cart->m_nvram_readonly = 0;
	m_cart->m_nvram_active = 0;
	m_cart->m_nvram_handlers_installed = 0;

	/* install SRAM & i2c handlers for the specific type of cart */
	switch (m_type)
	{
		// These types only come from fullpath loading
		case SEGA_SRAM_FULLPATH:
			m_cart->m_nvram_start = (ROM[0x1b5] << 24 | ROM[0x1b4] << 16 | ROM[0x1b7] << 8 | ROM[0x1b6]);
			m_cart->m_nvram_end = (ROM[0x1b9] << 24 | ROM[0x1b8] << 16 | ROM[0x1bb] << 8 | ROM[0x1ba]);
			logerror("SRAM detected from header: starting location %X - SRAM Length %X\n", m_cart->m_nvram_start, m_cart->m_nvram_end - m_cart->m_nvram_start + 1);

			// We assume at most 64k of SRAM. is this correct?
			if ((m_cart->m_nvram_start > m_cart->m_nvram_end) || ((m_cart->m_nvram_end - m_cart->m_nvram_start) >= 0x10000))
				m_cart->m_nvram_end = m_cart->m_nvram_start + 0xffff;

			if (m_cart->m_nvram_start & 1)
				m_cart->m_nvram_start -= 1;

			if (!(m_cart->m_nvram_end & 1))
				m_cart->m_nvram_end += 1;

			m_cart->nvram_alloc(machine(), m_cart->m_nvram_end - m_cart->m_nvram_start + 1);
			if (m_cart->m_rom_size <= m_cart->m_nvram_start)
				m_cart->m_nvram_active = 1;
			m_cart->m_nvram_handlers_installed = 1;
			// don't trust too much header?
			m_cart->m_nvram_start &= 0x3fffff;
			m_cart->m_nvram_end &= 0x3fffff;
			break;
		case SEGA_SRAM_FALLBACK:
			m_cart->m_nvram_start = 0x200000;
			m_cart->m_nvram_end = m_cart->m_nvram_start + 0xffff;
			m_cart->nvram_alloc(machine(), m_cart->m_nvram_end - m_cart->m_nvram_start + 1);
			if (m_cart->m_rom_size <= m_cart->m_nvram_start)
				m_cart->m_nvram_active = 1;
			break;

		// These types only come from softlist loading
		case SEGA_SRAM:
			m_cart->m_nvram_start = 0x200000;
			m_cart->m_nvram_end = m_cart->m_nvram_start + get_software_region_length("sram") - 1;
			m_cart->nvram_alloc(machine(), m_cart->m_nvram_end - m_cart->m_nvram_start + 1);
			if (m_cart->m_rom_size <= m_cart->m_nvram_start)
				m_cart->m_nvram_active = 1;
			m_cart->m_nvram_handlers_installed = 1;
			break;
		case SEGA_FRAM:
			m_cart->m_nvram_start = 0x200000;
			m_cart->m_nvram_end = m_cart->m_nvram_start + get_software_region_length("fram") - 1;
			m_cart->nvram_alloc(machine(), m_cart->m_nvram_end - m_cart->m_nvram_start + 1);
			m_cart->m_nvram_active = 1;
			m_cart->m_nvram_handlers_installed = 1;
			break;

		// These types might come from both (pending proper id routines)
		case HARDBALL95:
			m_cart->m_nvram_start = 0x300000;
			m_cart->m_nvram_end = m_cart->m_nvram_start + get_software_region_length("sram") - 1;
			m_cart->nvram_alloc(machine(), m_cart->m_nvram_end - m_cart->m_nvram_start + 1);
			m_cart->m_nvram_active = 1;
			m_cart->m_nvram_handlers_installed = 1;
			break;
		case BEGGAR:
			m_cart->m_nvram_start = 0x400000;
			m_cart->m_nvram_end = m_cart->m_nvram_start + 0xffff;
			m_cart->nvram_alloc(machine(), m_cart->m_nvram_end - m_cart->m_nvram_start + 1);
			m_cart->m_nvram_active = 1;
			m_cart->m_nvram_handlers_installed = 1;
			break;
	}
}



/*-------------------------------------------------
 call softlist load
 -------------------------------------------------*/

bool base_md_cart_slot_device::call_softlist_load(char *swlist, char *swname, rom_entry *start_entry)
{
	load_software_part_region(this, swlist, swname, start_entry );
	return TRUE;
}

int base_md_cart_slot_device::get_cart_type(UINT8 *ROM, UINT32 len)
{
	int type = SEGA_STD;

	/* Detect carts which need additional handlers */
	static const unsigned char smouse_sig[] = { 0x4d, 0xf9, 0x00, 0x40, 0x00, 0x02 },
	mjlover_sig[]   = { 0x13, 0xf9, 0x00, 0x40, 0x00, 0x00 }, // move.b  ($400000).l,($FFFF0C).l (partial)
	squir_sig[]     = { 0x26, 0x79, 0x00, 0xff, 0x00, 0xfa },
	bugsl_sig[]     = { 0x20, 0x12, 0x13, 0xc0, 0x00, 0xff },
	sbub_sig[]      = { 0x0c, 0x39, 0x00, 0x55, 0x00, 0x40 }, // cmpi.b  #$55,($400000).l
	lk3_sig[]       = { 0x0c, 0x01, 0x00, 0x30, 0x66, 0xe4 },
	sdk_sig[]       = { 0x48, 0xe7, 0xff, 0xfe, 0x52, 0x79 },
	redcliff_sig[]  = { 0x10, 0x39, 0x00, 0x40, 0x00, 0x04 }, // move.b  ($400004).l,d0
	redcl_en_sig[]  = { 0x50, 0x79, 0x40, 0x00, 0x40, 0x44 }, // move.b  ($400004).l,d0
	smb_sig[]       = { 0x20, 0x4d, 0x41, 0x52, 0x49, 0x4f },
	smb2_sig[]      = { 0x4e, 0xb9, 0x00, 0x0f, 0x25, 0x84 },
	kaiju_sig[]     = { 0x19, 0x7c, 0x00, 0x01, 0x00, 0x00 },
	chifi3_sig[]    = { 0xb6, 0x16, 0x66, 0x00, 0x00, 0x4a },
	lionk2_sig[]    = { 0x26, 0x79, 0x00, 0xff, 0x00, 0xf4 },
	rx3_sig[]       = { 0x66, 0x00, 0x00, 0x0e, 0x30, 0x3c },
	kof98_sig[]     = { 0x9b, 0xfc, 0x00, 0x00, 0x4a, 0x00 },
	s15in1_sig[]    = { 0x22, 0x3c, 0x00, 0xa1, 0x30, 0x00 },
	kof99_sig[]     = { 0x20, 0x3c, 0x30, 0x00, 0x00, 0xa1 }, // move.l  #$300000A1,d0
	radica_sig[]    = { 0x4e, 0xd0, 0x30, 0x39, 0x00, 0xa1 }, // jmp (a0) move.w ($a130xx),d0
	soulb_sig[]     = { 0x33, 0xfc, 0x00, 0x0c, 0x00, 0xff }, // move.w  #$C,($FF020A).l (what happens if check fails)
	s19in1_sig[]    = { 0x13, 0xc0, 0x00, 0xa1, 0x30, 0x38 },
	rockman_sig[]   = { 0xea, 0x80 };

	switch (len)
	{
		case 0x80000:
			if (!memcmp(&ROM[0x08c8], smouse_sig, sizeof(smouse_sig)))
				type = SMOUSE;

			if (!memcmp((char *)&ROM[0x7e30e], "SEGA", 4) ||
				!memcmp((char *)&ROM[0x7e100], "SEGA", 4) ||
				!memcmp((char *)&ROM[0x7e1e6], "SEGA", 4))
				type = REALTEC;

			if (!memcmp((char *)&ROM[0x0180], "GM T-50396", 10)) // NHLPA Hockey 93
				type = EA_NHLPA;

			if (!memcmp((char *)&ROM[0x0180], "GM MK-1215", 10)) // Evander Holyfield
				type = SEGA_EEPROM;
			break;

		case 0xc0000:

			if (!memcmp((char *)&ROM[0x0180], "GM G-4060 ", 8)) // Wonder Boy V
				type = SEGA_EEPROM;
			break;

		case 0x100000:
			if (!memcmp(&ROM[0x01b24], mjlover_sig, sizeof(mjlover_sig)))
				type = MJLOVER;

			if (!memcmp(&ROM[0x03b4], squir_sig, sizeof(squir_sig)))
				type = SQUIRRELK;

			if (!memcmp(&ROM[0xee0d0], bugsl_sig, sizeof(bugsl_sig)))
				type = BUGSLIFE;

			if (!memcmp((char *)&ROM[0x0172], "GAME : ELF WOR", 14))
				type = ELFWOR;

			if (!memcmp(&ROM[0x123e4], sbub_sig, sizeof(sbub_sig)))
				type = SBUBBOB;

			if (!memcmp((char *)&ROM[0x0180], "GM T-50176", 10)) // Rings of Power
				type = EA_NHLPA;

			if (!memcmp((char *)&ROM[0x0180], "MK 00001211-00", 14)) // Sports Talk Baseball
				type = SEGA_EEPROM;

			if (!memcmp((char *)&ROM[0x0180], "GM T-120096-", 12)) // Micro Machines 2
				type = CODE_MASTERS;

			if (!memcmp((char *)&ROM[0x0180], "GM T-120146-", 12)) // Brian Lara Cricket 96 / Shane Wayne Cricket 96
				type = BRIAN_LARA;

			if (!memcmp((char *)&ROM[0x0190], "OJKRPTBVFCA     ", 0x10)) // Micro Machines '96 / Military TODO: better way to recognize these?
				type = CODE_MASTERS;
			break;

		case 0x200000:
			if (!memcmp(&ROM[0x18c6], lk3_sig, sizeof(lk3_sig)))
				type = LIONK3;

			if (!memcmp(&ROM[0x220], sdk_sig, sizeof(sdk_sig)))
				type = LIONK3;

			if (!memcmp(&ROM[0xce560], redcliff_sig, sizeof(redcliff_sig)))
				type = REDCLIFF;

			if (!memcmp(&ROM[0xc8cb0], smb_sig, sizeof(smb_sig)))
				type = SMB;

			if (!memcmp(&ROM[0xf24d6], smb2_sig, sizeof(smb2_sig)))
				type = SMB2;

			if (!memcmp(&ROM[0x674e], kaiju_sig, sizeof(kaiju_sig)))
				type = KAIJU;

			if (!memcmp(&ROM[0x1780], chifi3_sig, sizeof(chifi3_sig)))
				type = CHINFIGHT3;

			if (!memcmp(&ROM[0x03c2], lionk2_sig, sizeof(lionk2_sig)))
				type = LIONK2;

			if (!memcmp(&ROM[0xc8b90], rx3_sig, sizeof(rx3_sig)))
				type = ROCKMANX3;

			if (!memcmp(&ROM[0x56ae2], kof98_sig, sizeof(kof98_sig)))
				type = KOF98;

			if (!memcmp(&ROM[0x17bb2], s15in1_sig, sizeof(s15in1_sig)))
				type = MC_PIRATE;

			if (!memcmp((char *)&ROM[0x0180], "GM T-081326 ", 12)) // NBA Jam
				type = NBA_JAM;

			if (!memcmp((char *)&ROM[0x0180], "GM MK-1228", 10)) // Greatest Heavyweight of the Ring
				type = SEGA_EEPROM;

			if ((!memcmp((char *)&ROM[0x0180], "GM T-12046", 10)) || // Mega Man
				(!memcmp((char *)&ROM[0x0180], "GM T-12053", 10) && !memcmp(&ROM[0x18e], rockman_sig, sizeof(rockman_sig)))) // / Rock Man (EEPROM version)
				type = SEGA_EEPROM;

			break;

		case 0x200005:
			if (!memcmp(&ROM[0xce564], redcl_en_sig, sizeof(redcliff_sig)))
				type = REDCL_EN;
			break;

		case 0x300000:
			if (!memcmp(&ROM[0x220], sdk_sig, sizeof(sdk_sig)))
				type = LIONK3;

			if (!memcmp(&ROM[0x1fd0d2], kof99_sig, sizeof(kof99_sig)))
				type = KOF99;

			if (!memcmp((char *)&ROM[0x0180], "GM T-81406", 10)) // NBA Jam TE
				type = NBA_JAM_TE;

			if (!memcmp((char *)&ROM[0x0180], "GM T-081276 ", 12)) // NFL Quarterback Club
				type = NBA_JAM_TE;

			break;

		case 0x400000:
			if (!memcmp(&ROM[0x3c031c], radica_sig, sizeof(radica_sig)) ||
				!memcmp(&ROM[0x3f031c], radica_sig, sizeof(radica_sig))) // ssf+gng + radica vol1
				type = RADICA;

			if (!memcmp(&ROM[0x028460], soulb_sig, sizeof(soulb_sig)))
				type = SOULBLAD;

			if (!memcmp(&ROM[0x1e700], s19in1_sig, sizeof(s19in1_sig)))
				type = MC_PIRATE;

			if (!memcmp((char *)&ROM[0x0180], "GM T-081586-", 12)) // NFL Quarterback Club 96
				type = NFL_QB_96;

			if (!memcmp((char *)&ROM[0x0180], "GM T-081576 ", 12)) // College Slam
				type = C_SLAM;

			if (!memcmp((char *)&ROM[0x0180], "GM T-81476", 10)) // Big Hurt Baseball
				type = C_SLAM;

			break;

		case 0x500000:
			if (!memcmp((char *)&ROM[0x0120], "SUPER STREET FIGHTER2 ", 22))
				type = SSF2;
			break;

		case 0x800000:
			if (!memcmp((char *)&ROM[0x0180], "GM T-574023-", 12)) // Pier Solar
				type = PSOLAR;
			break;

		default:
			break;
	}

	//check for SRAM among the general carts
	if (type == SEGA_STD)
	{
		// If the cart is not of a special type, we check the header for SRAM.
		if (ROM[0x1b1] == 'R' && ROM[0x1b0] == 'A')
		{
			UINT32 start, end;
			start = (ROM[0x1b5] << 24 | ROM[0x1b4] << 16 | ROM[0x1b7] << 8 | ROM[0x1b6]);
			end = (ROM[0x1b9] << 24 | ROM[0x1b8] << 16 | ROM[0x1bb] << 8 | ROM[0x1ba]);
			logerror("SRAM detected from header: starting location %X - SRAM Length %X\n", start, end - start + 1);

			// For some games using serial EEPROM, difference between SRAM end to start is 0 or 1.
			// Carts with EEPROM should have been already detected above, but better safe than sorry
			if (end - start < 2)
				type = SEGA_EEPROM;
			else
				type = SEGA_SRAM_FULLPATH;
		}
		else
		{
			// Unfortunately, there are ROMs without correct info in the header,
			// Hence, when loading from fullpath we do the SRAM mapping anyway...
			// but treat it in a custom way
			type = SEGA_SRAM_FALLBACK;
		}
	}

	return type;
}
/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

const char * base_md_cart_slot_device::get_default_card_software(const machine_config &config, emu_options &options)
{
	const char *slot_string = "rom";

	if (open_image_file(options))
	{
		UINT32 len = core_fsize(m_file);
		UINT8 *ROM = global_alloc_array(UINT8, len);
		int type;

		core_fread(m_file, ROM, len);

		type = get_cart_type(ROM, len);
		slot_string = md_get_slot(type);

		global_free(ROM);
		clear();

		return slot_string;
	}
	else
		return software_get_default_slot(config, options, this, "rom");
}



/*-------------------------------------------------
 read
 -------------------------------------------------*/

READ16_MEMBER(base_md_cart_slot_device::read)
{
	if (m_cart)
		return m_cart->read(space, offset);
	else
		return 0xffff;
}

READ16_MEMBER(base_md_cart_slot_device::read_a13)
{
	if (m_cart)
		return m_cart->read_a13(space, offset);
	else
		return 0xffff;
}

READ16_MEMBER(base_md_cart_slot_device::read_a15)
{
	if (m_cart)
		return m_cart->read_a15(space, offset);
	else
		return 0xffff;
}


/*-------------------------------------------------
 write
 -------------------------------------------------*/

WRITE16_MEMBER(base_md_cart_slot_device::write)
{
	if (m_cart)
		m_cart->write(space, offset, data);
}

WRITE16_MEMBER(base_md_cart_slot_device::write_a13)
{
	if (m_cart)
		m_cart->write_a13(space, offset, data);
}

WRITE16_MEMBER(base_md_cart_slot_device::write_a15)
{
	if (m_cart)
		m_cart->write_a15(space, offset, data);
}
