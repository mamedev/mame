// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Wilbert Pol
/***********************************************************************************************************


    Game Boy cart emulation
    (through slot devices)


 The driver exposes address ranges
 0x0000-0x7fff to read_rom/write_bank
 0xa000-0xbfff to read_ram/write_ram (typically RAM/NVRAM accesses, but megaduck uses the write for bankswitch)

 currently available slot devices:
 gb_rom: standard carts + TAMA5 mapper + pirate carts with protection & bankswitch
 gb_mbc: MBC1-MBC7 carts (more complex bankswitch + RAM + possibly RTC/Rumble/etc.)

 ***********************************************************************************************************/


#include "emu.h"
#include "gb_slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type GB_CART_SLOT = &device_creator<gb_cart_slot_device>;
const device_type MEGADUCK_CART_SLOT = &device_creator<megaduck_cart_slot_device>;

//**************************************************************************
//    GB cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  device_gb_cart_interface - constructor
//-------------------------------------------------

device_gb_cart_interface::device_gb_cart_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_rom(nullptr),
		m_rom_size(0), m_ram_bank(0), m_latch_bank(0), m_latch_bank2(0),
		has_rumble(false),
		has_timer(false),
		has_battery(false)
{
}


//-------------------------------------------------
//  ~device_gb_cart_interface - destructor
//-------------------------------------------------

device_gb_cart_interface::~device_gb_cart_interface()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_gb_cart_interface::rom_alloc(UINT32 size, const char *tag)
{
	if (m_rom == nullptr)
	{
		m_rom = device().machine().memory().region_alloc(std::string(tag).append(GBSLOT_ROM_REGION_TAG).c_str(), size, 1, ENDIANNESS_LITTLE)->base();
		m_rom_size = size;
	}
}


//-------------------------------------------------
//  ram_alloc - alloc the space for the ram
//-------------------------------------------------

void device_gb_cart_interface::ram_alloc(UINT32 size)
{
	m_ram.resize(size);
}


//-------------------------------------------------
//  rom_map_setup - setup map of rom banks in 16K
//  blocks, so to simplify ROM access
//-------------------------------------------------

void device_gb_cart_interface::rom_map_setup(UINT32 size)
{
	int i;
	// setup the rom_bank_map array to faster ROM read
	for (i = 0; i < size / 0x4000; i++)
		rom_bank_map[i] = i;

	// fill up remaining blocks with mirrors
	while (i % 512)
	{
		int j = 0, repeat_banks;
		while ((i % (512 >> j)) && j < 9)
			j++;
		repeat_banks = i % (512 >> (j - 1));
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

//-------------------------------------------------
//  ram_map_setup - setup map of ram banks in 16K
//  blocks, so to simplify ROM access
//-------------------------------------------------

void device_gb_cart_interface::ram_map_setup(UINT8 banks)
{
	int mask = banks - 1;

	for (int i = 0; i < banks; i++)
		ram_bank_map[i] = i;

	// Set up rest of the (mirrored) RAM pages
	for (int i = banks; i < 256; i++)
		ram_bank_map[i] = i & mask;
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  base_gb_cart_slot_device - constructor
//-------------------------------------------------
base_gb_cart_slot_device::base_gb_cart_slot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
						device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_image_interface(mconfig, *this),
						device_slot_interface(mconfig, *this),
						m_sgb_hack(0),
						m_type(GB_MBC_UNKNOWN), m_cart(nullptr)
{
}

gb_cart_slot_device::gb_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						base_gb_cart_slot_device(mconfig, GB_CART_SLOT, "Game Boy Cartridge Slot", tag, owner, clock, "gb_cart_slot", __FILE__)
{
}

megaduck_cart_slot_device::megaduck_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						base_gb_cart_slot_device(mconfig, MEGADUCK_CART_SLOT, "Megaduck Cartridge Slot", tag, owner, clock, "megaduck_cart_slot", __FILE__)
{
}

//-------------------------------------------------
//  base_gb_cart_slot_device - destructor
//-------------------------------------------------

base_gb_cart_slot_device::~base_gb_cart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void base_gb_cart_slot_device::device_start()
{
	m_cart = dynamic_cast<device_gb_cart_interface *>(get_card_device());
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void base_gb_cart_slot_device::device_config_complete()
{
	// set brief and instance name
	update_names();
}


//-------------------------------------------------
//  GB PCB
//-------------------------------------------------


struct gb_slot
{
	int                     pcb_id;
	const char              *slot_option;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const gb_slot slot_list[] =
{
	{ GB_MBC_MBC1, "rom_mbc1" },
	{ GB_MBC_MBC1_COL, "rom_mbc1col" },
	{ GB_MBC_MBC2, "rom_mbc2" },
	{ GB_MBC_MBC3, "rom_mbc3" },
	{ GB_MBC_MBC5, "rom_mbc5" },
	{ GB_MBC_MBC6, "rom_mbc6" },
	{ GB_MBC_MBC7, "rom_mbc7" },
	{ GB_MBC_TAMA5, "rom_tama5" },
	{ GB_MBC_MMM01, "rom_mmm01" },
	{ GB_MBC_M161, "rom_m161_m12" },
	{ GB_MBC_MBC3, "rom_huc1" },    // for now treat this as alias for MBC3
	{ GB_MBC_MBC3, "rom_huc3" },    // for now treat this as alias for MBC3
	{ GB_MBC_SACHEN1, "rom_sachen1" },
	{ GB_MBC_SACHEN2, "rom_sachen2" },
	{ GB_MBC_WISDOM, "rom_wisdom" },
	{ GB_MBC_YONGYONG, "rom_yong" },
	{ GB_MBC_LASAMA, "rom_lasama" },
	{ GB_MBC_ATVRACIN, "rom_atvrac" },
	{ GB_MBC_SINTAX, "rom_sintax" },
	{ GB_MBC_CHONGWU, "rom_chong" },
	{ GB_MBC_LICHENG, "rom_licheng" },
	{ GB_MBC_DIGIMON, "rom_digimon" },
	{ GB_MBC_ROCKMAN8, "rom_rock8" },
	{ GB_MBC_SM3SP, "rom_sm3sp" },
	{ GB_MBC_UNK01, "rom_unk01" },
	{ GB_MBC_DKONG5, "rom_dkong5" },
	{ GB_MBC_CAMERA, "rom_camera" },
	{ GB_MBC_188IN1, "rom_188in1" }
};

static int gb_get_pcb_id(const char *slot)
{
	for (auto & elem : slot_list)
	{
		if (!core_stricmp(elem.slot_option, slot))
			return elem.pcb_id;
	}

	return 0;
}

static const char *gb_get_slot(int type)
{
	for (auto & elem : slot_list)
	{
		if (elem.pcb_id == type)
			return elem.slot_option;
	}

	return "rom";
}


/*-------------------------------------------------
 call load
 -------------------------------------------------*/


bool base_gb_cart_slot_device::call_load()
{
	if (m_cart)
	{
		UINT32 offset;
		UINT32 len = (software_entry() == nullptr) ? length() : get_software_region_length("rom");
		UINT8 *ROM;
		int rambanks = 0;

		// From fullpath, check for presence of a header and skip it + check filesize is valid
		if (software_entry() == nullptr)
		{
			if ((len % 0x4000) == 512)
			{
				logerror("Rom-header found, skipping\n");
				offset = 512;
				len -= offset;
				fseek(offset, SEEK_SET);
			}
			/* Verify that the file contains 16kb blocks */
			if ((len == 0) || ((len % 0x4000) != 0))
			{
				seterror(IMAGE_ERROR_UNSPECIFIED, "Invalid rom file size\n");
				return IMAGE_INIT_FAIL;
			}
		}

		m_cart->rom_alloc(len, tag());
		ROM = m_cart->get_rom_base();

		if (software_entry() == nullptr)
			fread(ROM, len);
		else
			memcpy(ROM, get_software_region("rom"), len);

		// determine cart type
		offset = 0;
		if (get_mmm01_candidate(ROM, len))
			offset = len - 0x8000;

		if (software_entry() != nullptr)
			m_type = gb_get_pcb_id(get_feature("slot") ? get_feature("slot") : "rom");
		else
			m_type = get_cart_type(ROM + offset, len - offset);

		// setup additional mask/shift for MBC1 variants:
		// a few game collections use the same mapper with slightly
		// different lines connection with the ROM / RAM
		if (m_type == GB_MBC_MBC1 || m_type == GB_MBC_188IN1)
			m_cart->set_additional_wirings(0x1f, 0);
		if (m_type == GB_MBC_MBC1_COL)
			m_cart->set_additional_wirings(0x0f, -1);

		// setup RAM/NVRAM/RTC/RUMBLE
		if (software_entry() != nullptr)
		{
			// from softlist we only rely on xml
			if (get_software_region("ram"))
				rambanks = get_software_region_length("ram") / 0x2000;

			if (get_software_region("nvram"))
			{
				m_cart->set_has_battery(true);
				rambanks = get_software_region_length("nvram") / 0x2000;
			}

			if (get_feature("rumble"))
			{
				if (!core_stricmp(get_feature("rumble"), "yes"))
					m_cart->set_has_rumble(true);
			}

			if (get_feature("rtc"))
			{
				if (!core_stricmp(get_feature("rtc"), "yes"))
					m_cart->set_has_timer(true);
			}
		}
		else
		{
			// from fullpath we rely on header
			switch (ROM[0x0147 + offset])
			{
				case 0x03:  case 0x06:  case 0x09:  case 0x0d:  case 0x13:  case 0x17:  case 0x1b:  case 0x22:
					m_cart->set_has_battery(true);
					break;

				case 0x0f:  case 0x10:
					m_cart->set_has_battery(true);
					m_cart->set_has_timer(true);
					break;

				case 0x1c:  case 0x1d:
					m_cart->set_has_rumble(true);
					break;

				case 0x1e:
					m_cart->set_has_battery(true);
					m_cart->set_has_rumble(true);
					break;
			}

			switch (ROM[0x0149 + offset] & 0x07)
			{
				case 0x00: case 0x06: case 0x07:
					rambanks = 0;
					break;
				case 0x01: case 0x02:
					rambanks = 1;
					break;
				case 0x03:
					rambanks = 4;
					break;
				case 0x04:
					rambanks = 16;
					break;
				case 0x05:
				default:
					rambanks = 8;
					break;
			}

			if (m_type == GB_MBC_MBC2 ||  m_type == GB_MBC_MBC7)
				rambanks = 1;
		}

		// setup rom bank map based on real length, not header value
		m_cart->rom_map_setup(len);

		if (rambanks)
			setup_ram(rambanks);

		if (m_cart->get_ram_size() && m_cart->get_has_battery())
			battery_load(m_cart->get_ram_base(), m_cart->get_ram_size(), 0xff);

		//printf("Type: %s\n", gb_get_slot(m_type));

		internal_header_logging(ROM + offset, len);

		// Hack to support Donkey Kong Land 2 + 3 in SGB
		// For some reason, these store the tile data differently. Hacks will go once it's been figured out
		if (strncmp((const char*)(ROM + 0x134), "DONKEYKONGLAND 2", 16) == 0 ||
			strncmp((const char*)(ROM + 0x134), "DONKEYKONGLAND 3", 16) == 0)
			m_sgb_hack = 1;

		return IMAGE_INIT_PASS;
	}

	return IMAGE_INIT_PASS;
}

bool megaduck_cart_slot_device::call_load()
{
	if (m_cart)
	{
		UINT32 len = (software_entry() == nullptr) ? length() : get_software_region_length("rom");

		m_cart->rom_alloc(len, tag());

		if (software_entry() == nullptr)
			fread(m_cart->get_rom_base(), len);
		else
			memcpy(m_cart->get_rom_base(), get_software_region("rom"), len);

		// setup rom bank map based on real length, not header value
		m_cart->rom_map_setup(len);

		return IMAGE_INIT_PASS;
	}

	return IMAGE_INIT_PASS;
}


/*-------------------------------------------------
 call_unload
 -------------------------------------------------*/

void base_gb_cart_slot_device::call_unload()
{
	if (m_cart && m_cart->get_ram_base() && m_cart->get_ram_size() && m_cart->get_has_battery())
		battery_save(m_cart->get_ram_base(), m_cart->get_ram_size());
}

void base_gb_cart_slot_device::setup_ram(UINT8 banks)
{
	m_cart->ram_alloc(banks * 0x2000);
	memset(m_cart->get_ram_base(), 0xff, m_cart->get_ram_size());
	m_cart->ram_map_setup(banks);
}



/*-------------------------------------------------
 call softlist load
 -------------------------------------------------*/

bool base_gb_cart_slot_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	machine().rom_load().load_software_part_region(*this, swlist, swname, start_entry);
	return true;
}

// This fails to catch Mani 4-in-1 carts... even when they match this, then they have MBC1/3 in the internal header instead of MMM01...
bool base_gb_cart_slot_device::get_mmm01_candidate(UINT8 *ROM, UINT32 len)
{
	if (len < 0x8147)
		return false;

	static const UINT8 nintendo_logo[0x18] = {
		0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
		0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
		0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E
	};
	int bytes_matched = 0;
	for (int i = 0; i < 0x18; i++)
	{
		if (ROM[(len - 0x8000) + 0x104 + i] == nintendo_logo[i])
			bytes_matched++;
	}

	if (bytes_matched == 0x18 && ROM[(len - 0x8000) + 0x147] >= 0x0b && ROM[(len - 0x8000) + 0x147] <= 0x0d)
		return true;
	else
		return false;
}

int base_gb_cart_slot_device::get_cart_type(UINT8 *ROM, UINT32 len)
{
	int type = GB_MBC_NONE;

	if (len < 0x014c)
		fatalerror("Checking header of a corrupted image!\n");

	switch(ROM[0x0147])
	{
		case 0x00:  type = GB_MBC_NONE; break;
		case 0x01:  type = GB_MBC_MBC1;    break;
		case 0x02:  type = GB_MBC_MBC1;    break;
		case 0x03:  type = GB_MBC_MBC1;    break;
		case 0x05:  type = GB_MBC_MBC2;    break;
		case 0x06:  type = GB_MBC_MBC2;    break;
		case 0x08:  type = GB_MBC_NONE;    break;
		case 0x09:  type = GB_MBC_NONE;    break;
		case 0x0b:  type = GB_MBC_MMM01;   break;
		case 0x0c:  type = GB_MBC_MMM01;   break;
		case 0x0d:  type = GB_MBC_MMM01;   break;
		case 0x0f:  type = GB_MBC_MBC3;    break;
		case 0x10:  type = GB_MBC_MBC3;    break;
		case 0x11:  type = GB_MBC_MBC3;    break;
		case 0x12:  type = GB_MBC_MBC3;    break;
		case 0x13:  type = GB_MBC_MBC3;    break;
		case 0x15:  type = GB_MBC_MBC4;    break;
		case 0x16:  type = GB_MBC_MBC4;    break;
		case 0x17:  type = GB_MBC_MBC4;    break;
		case 0x19:  type = GB_MBC_MBC5;    break;
		case 0x1a:  type = GB_MBC_MBC5;    break;
		case 0x1b:  type = GB_MBC_MBC5;    break;
		case 0x1c:  type = GB_MBC_MBC5;    break;
		case 0x1d:  type = GB_MBC_MBC5;    break;
		case 0x1e:  type = GB_MBC_MBC5;    break;
		case 0x20:  type = GB_MBC_MBC6;    break;
		case 0x22:  type = GB_MBC_MBC7;    break;
		case 0xbe:  type = GB_MBC_NONE;    break;  /* used in Flash2Advance GB Bridge boot program */
		case 0xea:  type = GB_MBC_YONGYONG; break;  /* Found in Sonic 3D Blast 5 pirate */
		case 0xfc:  type = GB_MBC_CAMERA;   break;
		case 0xfd:  type = GB_MBC_TAMA5;   break;
		case 0xfe:  type = GB_MBC_HUC3;    break;
		case 0xff:  type = GB_MBC_HUC1;    break;
	}

	// Check for special mappers
	if (type == GB_MBC_NONE)
	{
		int count = 0;
		for (int i = 0x0134; i <= 0x014c; i++)
		{
			count += ROM[i];
		}
		if (count == 0)
		{
			type = GB_MBC_WISDOM;
		}
	}

	// Check for some unlicensed games
	//if (type == GB_MBC_MBC5)
	{
		int count = 0;
		for (int i = 0x0184; i < 0x0184 + 0x30; i++)
		{
			count += ROM[i];
		}

		if (count == 4876)
		{
//          printf("Li Cheng %d\n", count);
			type = GB_MBC_LICHENG;
		}
		if (count == 4138 || count == 4125)
		{
			// Zhi Huan Wang uses 4138
			// most sintax use 4125
//          printf("Sintax %d!\n", count);
			type = GB_MBC_SINTAX;
		}
	}

	/* Check if we're dealing with the multigame variant of the MBC1 mapper */
	if (type == GB_MBC_MBC1)
	{   // bomberman collection korea
		if (ROM[0x134] == 0x42 && ROM[0x135] == 0x4f && ROM[0x136] == 0x4d && ROM[0x137] == 0x53)
			type = GB_MBC_MBC1_COL;
//      if (ROM[0x13f] == 0x42 && ROM[0x140] == 0x32 && ROM[0x141] == 0x43 && ROM[0x142] == 0x4B)
//          type = GB_MBC_MBC1_COL;
		// genjin collection
		if (ROM[0x134] == 0x47 && ROM[0x135] == 0x45 && ROM[0x136] == 0x4e && ROM[0x137] == 0x43)
			type = GB_MBC_MBC1_COL;
		// bomberman collection japan
		if (ROM[0x134] == 0x42 && ROM[0x135] == 0x4f && ROM[0x136] == 0x4d && ROM[0x137] == 0x43)
			type = GB_MBC_MBC1_COL;
		// mortal kombat I & II US
		if (ROM[0x140] == 0x49 && ROM[0x141] == 0x26 && ROM[0x142] == 0x49 && ROM[0x143] == 0x49)
			type = GB_MBC_MBC1_COL;
		// mortal kombat I & II japan
		if (ROM[0x140] == 0x20 && ROM[0x141] == 0x44 && ROM[0x142] == 0x55 && ROM[0x143] == 0x4f)
			type = GB_MBC_MBC1_COL;
		// momotarou collection 1 japan
		if (ROM[0x137] == 0x4f && ROM[0x138] == 0x43 && ROM[0x139] == 0x4f && ROM[0x13a] == 0x4c)
			type = GB_MBC_MBC1_COL;
		// super chinese 123 dash japan
		if (ROM[0x142] == 0x32 && ROM[0x143] == 0x33 && ROM[0x144] == 0x42 && ROM[0x145] == 0x41)
			type = GB_MBC_MBC1_COL;
	}

	return type;
}

/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

std::string base_gb_cart_slot_device::get_default_card_software()
{
	if (open_image_file(mconfig().options()))
	{
		const char *slot_string;
		UINT32 len = core_fsize(m_file), offset = 0;
		dynamic_buffer rom(len);
		int type;

		core_fread(m_file, &rom[0], len);

		if ((len % 0x4000) == 512)
			offset = 512;

		if (get_mmm01_candidate(&rom[offset], len - offset))
			offset += (len - 0x8000);

		type = get_cart_type(&rom[offset], len - offset);
		slot_string = gb_get_slot(type);

		//printf("type: %s\n", slot_string);
		clear();

		return std::string(slot_string);
	}

	return software_get_default_slot("rom");
}


std::string megaduck_cart_slot_device::get_default_card_software()
{
	if (open_image_file(mconfig().options()))
		return std::string("rom");

	return software_get_default_slot("rom");
}



/*-------------------------------------------------
 read
 -------------------------------------------------*/

READ8_MEMBER(base_gb_cart_slot_device::read_rom)
{
	if (m_cart)
		return m_cart->read_rom(space, offset);
	else
		return 0xff;
}

READ8_MEMBER(base_gb_cart_slot_device::read_ram)
{
	if (m_cart)
		return m_cart->read_ram(space, offset);
	else
		return 0xff;
}


/*-------------------------------------------------
 write
 -------------------------------------------------*/

WRITE8_MEMBER(base_gb_cart_slot_device::write_bank)
{
	if (m_cart)
		m_cart->write_bank(space, offset, data);
}

WRITE8_MEMBER(base_gb_cart_slot_device::write_ram)
{
	if (m_cart)
		m_cart->write_ram(space, offset, data);
}


/*-------------------------------------------------
 Internal header logging
 -------------------------------------------------*/

void base_gb_cart_slot_device::internal_header_logging(UINT8 *ROM, UINT32 len)
{
	static const char *const cart_types[] =
	{
		"ROM ONLY",                     "ROM+MBC1",                     "ROM+MBC1+RAM",
		"ROM+MBC1+RAM+BATTERY",         "UNKNOWN",                      "ROM+MBC2",
		"ROM+MBC2+BATTERY",             "UNKNOWN",                      "ROM+RAM",
		"ROM+RAM+BATTERY",              "UNKNOWN",                      "ROM+MMM01",
		"ROM+MMM01+SRAM",               "ROM+MMM01+SRAM+BATTERY",       "UNKNOWN",
		"ROM+MBC3+TIMER+BATTERY",       "ROM+MBC3+TIMER+RAM+BATTERY",   "ROM+MBC3",
		"ROM+MBC3+RAM",                 "ROM+MBC3+RAM+BATTERY",         "UNKNOWN",
		"UNKNOWN",                      "UNKNOWN",                      "UNKNOWN",
		"UNKNOWN",                      "ROM+MBC5",                     "ROM+MBC5+RAM",
		"ROM+MBC5+RAM+BATTERY",         "ROM+MBC5+RUMBLE",              "ROM+MBC5+RUMBLE+SRAM",
		"ROM+MBC5+RUMBLE+SRAM+BATTERY", "Pocket Camera",                "Bandai TAMA5",
		/* Need heaps of unknowns here */
		"Hudson HuC-3",                 "Hudson HuC-1"
	};

	// some company codes
	static const struct
	{
		UINT16 code;
		const char *name;
	}
	companies[] =
	{
		{0x3301, "Nintendo"},
		{0x7901, "Accolade"},
		{0xA400, "Konami"},
		{0x6701, "Ocean"},
		{0x5601, "LJN"},
		{0x9900, "ARC?"},
		{0x0101, "Nintendo"},
		{0x0801, "Capcom"},
		{0x0100, "Nintendo"},
		{0xBB01, "SunSoft"},
		{0xA401, "Konami"},
		{0xAF01, "Namcot?"},
		{0x4901, "Irem"},
		{0x9C01, "Imagineer"},
		{0xA600, "Kawada?"},
		{0xB101, "Nexoft"},
		{0x5101, "Acclaim"},
		{0x6001, "Titus"},
		{0xB601, "HAL"},
		{0x3300, "Nintendo"},
		{0x0B00, "Coconuts?"},
		{0x5401, "Gametek"},
		{0x7F01, "Kemco?"},
		{0xC001, "Taito"},
		{0xEB01, "Atlus"},
		{0xE800, "Asmik?"},
		{0xDA00, "Tomy?"},
		{0xB100, "ASCII?"},
		{0xEB00, "Atlus"},
		{0xC000, "Taito"},
		{0x9C00, "Imagineer"},
		{0xC201, "Kemco?"},
		{0xD101, "Sofel?"},
		{0x6101, "Virgin"},
		{0xBB00, "SunSoft"},
		{0xCE01, "FCI?"},
		{0xB400, "Enix?"},
		{0xBD01, "Imagesoft"},
		{0x0A01, "Jaleco?"},
		{0xDF00, "Altron?"},
		{0xA700, "Takara?"},
		{0xEE00, "IGS?"},
		{0x8300, "Lozc?"},
		{0x5001, "Absolute?"},
		{0xDD00, "NCS?"},
		{0xE500, "Epoch?"},
		{0xCB00, "VAP?"},
		{0x8C00, "Vic Tokai"},
		{0xC200, "Kemco?"},
		{0xBF00, "Sammy?"},
		{0x1800, "Hudson Soft"},
		{0xCA01, "Palcom/Ultra"},
		{0xCA00, "Palcom/Ultra"},
		{0xC500, "Data East?"},
		{0xA900, "Technos Japan?"},
		{0xD900, "Banpresto?"},
		{0x7201, "Broderbund?"},
		{0x7A01, "Triffix Entertainment?"},
		{0xE100, "Towachiki?"},
		{0x9300, "Tsuburava?"},
		{0xC600, "Tonkin House?"},
		{0xCE00, "Pony Canyon"},
		{0x7001, "Infogrames?"},
		{0x8B01, "Bullet-Proof Software?"},
		{0x5501, "Park Place?"},
		{0xEA00, "King Records?"},
		{0x5D01, "Tradewest?"},
		{0x6F01, "ElectroBrain?"},
		{0xAA01, "Broderbund?"},
		{0xC301, "SquareSoft"},
		{0x5201, "Activision?"},
		{0x5A01, "Bitmap Brothers/Mindscape"},
		{0x5301, "American Sammy"},
		{0x4701, "Spectrum Holobyte"},
		{0x1801, "Hudson Soft"},
		{0x0000, nullptr}
	};
	static const int ramsize[8] = { 0, 2, 8, 32, 128, 64, 0, 0 };

	char soft[17];
	UINT32 tmp;
	int csum = 0, i = 0;
	int rom_banks;

	switch (ROM[0x0148])
	{
		case 0x52:
			rom_banks = 72;
			break;
		case 0x53:
			rom_banks = 80;
			break;
		case 0x54:
			rom_banks = 96;
			break;
		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x04: case 0x05: case 0x06: case 0x07:
			rom_banks = 2 << ROM[0x0148];
			break;
		default:
			rom_banks = 256;
			break;
	}

	strncpy(soft, (char *)&ROM[0x0134], 16);
	soft[16] = '\0';
	logerror("Cart Information\n");
	logerror("\tName:             %s\n", soft);
	logerror("\tType:             %s [0x%2X]\n", (ROM[0x0147] <= 32) ? cart_types[ROM[0x0147]] : "", ROM[0x0147] );
	logerror("\tGame Boy:         %s\n", (ROM[0x0143] == 0xc0) ? "No" : "Yes" );
	logerror("\tSuper GB:         %s [0x%2X]\n", (ROM[0x0146] == 0x03) ? "Yes" : "No", ROM[0x0146] );
	logerror("\tColor GB:         %s [0x%2X]\n", (ROM[0x0143] == 0x80 || ROM[0x0143] == 0xc0) ? "Yes" : "No", ROM[0x0143] );
	logerror("\tROM Size:         %d 16kB Banks [0x%2X]\n", rom_banks, ROM[0x0148]);
	logerror("\tRAM Size:         %d kB [0x%2X]\n", ramsize[ROM[0x0149] & 0x07], ROM[0x0149]);
	logerror("\tLicense code:     0x%2X%2X\n", ROM[0x0145], ROM[0x0144] );
	tmp = (ROM[0x014b] << 8) + ROM[0x014a];
	for (i = 0; i < ARRAY_LENGTH(companies); i++)
		if (tmp == companies[i].code)
			break;
	logerror("\tManufacturer ID:  0x%2X", tmp);
	logerror(" [%s]\n", (i < ARRAY_LENGTH(companies)) ? companies[i].name : "?");
	logerror("\tVersion Number:   0x%2X\n", ROM[0x014c]);
	logerror("\tComplement Check: 0x%2X\n", ROM[0x014d]);
	logerror("\tChecksum:         0x%2X\n", ((ROM[0x014e] << 8) + ROM[0x014f]));
	tmp = (ROM[0x0103] << 8) + ROM[0x0102];
	logerror("\tStart Address:    0x%2X\n", tmp);

	// Additional checks
	if (rom_banks == 256)
		logerror("\nWarning loading cartridge: Unknown ROM size in header [0x%x].\n", ROM[0x0148]);

	if ((len / 0x4000) != rom_banks)
		logerror("\nWarning loading cartridge: Filesize (0x%x) and reported ROM banks (0x%x) don't match.\n",
					len, rom_banks * 0x4000);
	/* Calculate and check checksum */
	tmp = (ROM[0x014e] << 8) + ROM[0x014f];
	for (int i = 0; i < len; i++)
		csum += ROM[i];
	csum -= (ROM[0x014e] + ROM[0x014f]);
	csum &= 0xffff;

	if (csum != tmp)
		logerror("\nWarning loading cartridge: Checksum is wrong (Actual %X vs Internal %X)\n", csum, tmp);

}
