// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 Sega 8-bit cart emulation
 (through slot devices)

 Master System (Mark III) and Game Gear memory map can access 3 x 16K banks of ROM in
 0x0000-0xbfff memory range. These banks can however point to different ROM or RAM area
 of the cart (or to BIOS banks, but these are handled directly in SMS emulation).

 Hence, carts can interface with the main system through the following handlers
 * read_cart : to read from ROM/RAM in memory range [0000-bfff]
 * write_cart : to write to ROM/RAM in memory range [0000-bfff]
 * write_mapper : to write to range [fffc-ffff] (called by the handler accessing those
 same addresses in sms.c)

 Note about Sega Card / MyCard: the data contained in these matches the data in carts, it's only
 the connector to be different. We emulate this with a variant of the slot having different media
 switch and different interface (the latter not implemented yet)

 TODO:
 - investigate SG-1000 carts so to reduce duplicated code and to add full .sg support to sg1000m3

 ***********************************************************************************************************/


#include "emu.h"
#include "sega8_slot.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type SEGA8_CART_SLOT = &device_creator<sega8_cart_slot_device>;
const device_type SEGA8_CARD_SLOT = &device_creator<sega8_card_slot_device>;


//**************************************************************************
//    SMS cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  device_sega8_cart_interface - constructor
//-------------------------------------------------

device_sega8_cart_interface::device_sega8_cart_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_rom(nullptr),
		m_rom_size(0),
		m_rom_page_count(0),
		has_battery(FALSE),
		m_late_battery_enable(FALSE),
		m_lphaser_xoffs(-1),
		m_sms_mode(0)
{
}


//-------------------------------------------------
//  ~device_sega8_cart_interface - destructor
//-------------------------------------------------

device_sega8_cart_interface::~device_sega8_cart_interface()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_sega8_cart_interface::rom_alloc(UINT32 size, const char *tag)
{
	if (m_rom == nullptr)
	{
		m_rom = device().machine().memory().region_alloc(std::string(tag).append(S8SLOT_ROM_REGION_TAG).c_str(), size, 1, ENDIANNESS_LITTLE)->base();
		m_rom_size = size;
		m_rom_page_count = size / 0x4000;
		if (!m_rom_page_count)
			m_rom_page_count = 1;   // we compute rom pages through (XXX % m_rom_page_count)!
		late_bank_setup();
	}
}


//-------------------------------------------------
//  ram_alloc - alloc the space for the ram
//-------------------------------------------------

void device_sega8_cart_interface::ram_alloc(UINT32 size)
{
	m_ram.resize(size);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sega8_cart_slot_device - constructor
//-------------------------------------------------

sega8_cart_slot_device::sega8_cart_slot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, bool is_card, const char *shortname, const char *source) :
						device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_image_interface(mconfig, *this),
						device_slot_interface(mconfig, *this),
						m_type(SEGA8_BASE_ROM),
						m_must_be_loaded(FALSE),
						m_interface("sms_cart"),
						m_extensions("bin"), m_cart(nullptr)
{
	m_is_card = is_card;
}

sega8_cart_slot_device::sega8_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						device_t(mconfig, SEGA8_CART_SLOT, "Sega Master System / Game Gear / SG1000 Cartridge Slot", tag, owner, clock, "sega8_cart_slot", __FILE__),
						device_image_interface(mconfig, *this),
						device_slot_interface(mconfig, *this),
						m_type(SEGA8_BASE_ROM),
						m_must_be_loaded(FALSE),
						m_is_card(FALSE),
						m_interface("sms_cart"),
						m_extensions("bin"), m_cart(nullptr)
{
}

sega8_card_slot_device::sega8_card_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						sega8_cart_slot_device(mconfig, SEGA8_CARD_SLOT, "Sega Master System / Game Gear / SG1000 Card Slot", tag, owner, clock, TRUE, "sega8_card_slot", __FILE__)
{
}


//-------------------------------------------------
//  sega8_cart_slot_device - destructor
//-------------------------------------------------

sega8_cart_slot_device::~sega8_cart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sega8_cart_slot_device::device_start()
{
	m_cart = dynamic_cast<device_sega8_cart_interface *>(get_card_device());
}

//-------------------------------------------------
//  SMS PCB
//-------------------------------------------------


struct sega8_slot
{
	int                     pcb_id;
	const char              *slot_option;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const sega8_slot slot_list[] =
{
	{ SEGA8_BASE_ROM, "rom" },
	{ SEGA8_EEPROM, "eeprom" },
	{ SEGA8_TEREBIOEKAKI, "terebi" },
	{ SEGA8_4PAK, "4pak" },
	{ SEGA8_CODEMASTERS, "codemasters" },
	{ SEGA8_ZEMINA, "zemina" },
	{ SEGA8_NEMESIS, "nemesis" },
	{ SEGA8_JANGGUN, "janggun" },
	{ SEGA8_KOREAN, "korean" },
	{ SEGA8_KOREAN_NOBANK, "korean_nb" },
	{ SEGA8_OTHELLO, "othello" },
	{ SEGA8_CASTLE, "castle" },
	{ SEGA8_BASIC_L3, "level3" },
	{ SEGA8_MUSIC_EDITOR, "music_editor" },
	{ SEGA8_DAHJEE_TYPEA, "dahjee_typea" },
	{ SEGA8_DAHJEE_TYPEB, "dahjee_typeb" }
};

static int sega8_get_pcb_id(const char *slot)
{
	for (auto & elem : slot_list)
	{
		if (!core_stricmp(elem.slot_option, slot))
			return elem.pcb_id;
	}

	return 0;
}

static const char *sega8_get_slot(int type)
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

int sega8_cart_slot_device::verify_cart( UINT8 *magic, int size )
{
	int retval = IMAGE_VERIFY_FAIL;

	// Verify the file is a valid image - check $7ff0 for "TMR SEGA"
	if (size >= 0x8000)
	{
		if (!strncmp((char*)&magic[0x7ff0], "TMR SEGA", 8))
			retval = IMAGE_VERIFY_PASS;
	}

	return retval;
}

void sega8_cart_slot_device::set_lphaser_xoffset( UINT8 *rom, int size )
{
	static const UINT8 signatures[7][16] =
	{
		/* Spacegun */
		{ 0x54, 0x4d, 0x52, 0x20, 0x53, 0x45, 0x47, 0x41, 0xff, 0xff, 0x9d, 0x99, 0x10, 0x90, 0x00, 0x40 },
		/* Gangster Town */
		{ 0x54, 0x4d, 0x52, 0x20, 0x53, 0x45, 0x47, 0x41, 0x19, 0x87, 0x1b, 0xc9, 0x74, 0x50, 0x00, 0x4f },
		/* Shooting Gallery */
		{ 0x54, 0x4d, 0x52, 0x20, 0x53, 0x45, 0x47, 0x41, 0x20, 0x20, 0x8a, 0x3a, 0x72, 0x50, 0x00, 0x4f },
		/* Rescue Mission */
		{ 0x54, 0x4d, 0x52, 0x20, 0x53, 0x45, 0x47, 0x41, 0x20, 0x20, 0xfb, 0xd3, 0x06, 0x51, 0x00, 0x4f },
		/* Laser Ghost */
		{ 0x54, 0x4d, 0x52, 0x20, 0x53, 0x45, 0x47, 0x41, 0x00, 0x00, 0xb7, 0x55, 0x74, 0x70, 0x00, 0x40 },
		/* Assault City */
		{ 0x54, 0x4d, 0x52, 0x20, 0x53, 0x45, 0x47, 0x41, 0xff, 0xff, 0x9f, 0x74, 0x34, 0x70, 0x00, 0x40 },
		/* Missile Defense 3-D */
		{ 0x54, 0x4d, 0x52, 0x20, 0x53, 0x45, 0x47, 0x41, 0x41, 0x4c, 0x15, 0x4a, 0x01, 0x80, 0x00, 0x4f }
	};

	int xoff = -1;

	if (size >= 0x8000)
	{
		if (!memcmp(&rom[0x7ff0], signatures[0], 16) || !memcmp(&rom[0x7ff0], signatures[1], 16))
			xoff = 26;
		else if (!memcmp(&rom[0x7ff0], signatures[2], 16))
			xoff = 36;
		else if (!memcmp(&rom[0x7ff0], signatures[3], 16))
			xoff = 32;
		else if (!memcmp(&rom[0x7ff0], signatures[4], 16))
			xoff = 30;
		else if (!memcmp(&rom[0x7ff0], signatures[5], 16))
			xoff = 39;
		else if (!memcmp(&rom[0x7ff0], signatures[6], 16))
			xoff = 38;
	}

	m_cart->set_lphaser_xoffs(xoff);
}

void sega8_cart_slot_device::setup_ram()
{
	if (software_entry() == nullptr)
	{
		if (m_type == SEGA8_CASTLE)
		{
			m_cart->ram_alloc(0x2000);
			m_cart->set_has_battery(FALSE);
		}
		else if (m_type == SEGA8_OTHELLO)
		{
			m_cart->ram_alloc(0x800);
			m_cart->set_has_battery(FALSE);
		}
		else if (m_type == SEGA8_BASIC_L3)
		{
			m_cart->ram_alloc(0x8000);
			m_cart->set_has_battery(FALSE);
		}
		else if (m_type == SEGA8_MUSIC_EDITOR)
		{
			m_cart->ram_alloc(0x2800);
			m_cart->set_has_battery(FALSE);
		}
		else if (m_type == SEGA8_DAHJEE_TYPEA)
		{
			m_cart->ram_alloc(0x2400);
			m_cart->set_has_battery(FALSE);
		}
		else if (m_type == SEGA8_DAHJEE_TYPEB)
		{
			m_cart->ram_alloc(0x2000);
			m_cart->set_has_battery(FALSE);
		}
		else if (m_type == SEGA8_CODEMASTERS)
		{
			// Codemasters cart can have 64KB of RAM (Ernie Els Golf? or 8KB?) and no battery
			m_cart->ram_alloc(0x10000);
			m_cart->set_has_battery(FALSE);
		}
		else
		{
			// for generic carts loaded from fullpath we have no way to know exactly if there was RAM,
			// how much RAM was in the cart and if there was a battery so we always alloc 32KB and
			// we save its content only if the game enable the RAM
			m_cart->set_late_battery(TRUE);
			m_cart->ram_alloc(0x08000);
		}
	}
	else
	{
		// from softlist we rely on the xml to only allocate the correct amount of RAM and to save it only if a battery was present
		const char *battery = get_feature("battery");
		m_cart->set_late_battery(FALSE);

		if (get_software_region_length("ram"))
			m_cart->ram_alloc(get_software_region_length("ram"));

		if (battery && !strcmp(battery, "yes"))
			m_cart->set_has_battery(TRUE);
	}
}

bool sega8_cart_slot_device::call_load()
{
	if (m_cart)
	{
		UINT32 len = (software_entry() == nullptr) ? length() : get_software_region_length("rom");
		UINT32 offset = 0;
		UINT8 *ROM;

		if (m_is_card && len > 0x8000)
		{
			seterror(IMAGE_ERROR_UNSPECIFIED, "Attempted loading a card larger than 32KB");
			return IMAGE_INIT_FAIL;
		}

		// check for header
		if ((len % 0x4000) == 512)
		{
			offset = 512;
			len -= 512;
		}

		// make sure that we only get complete (0x4000) rom banks
		if (len & 0x3fff)
			len = ((len >> 14) + 1) << 14;

		m_cart->rom_alloc(len, tag());
		ROM = m_cart->get_rom_base();

		if (software_entry() == nullptr)
		{
			fseek(offset, SEEK_SET);
			fread(ROM, len);
		}
		else
			memcpy(ROM, get_software_region("rom"), get_software_region_length("rom"));

		/* check the image */
		if (verify_cart(ROM, len) == IMAGE_VERIFY_FAIL)
			logerror("Warning loading image: verify_cart failed\n");

		if (software_entry() != nullptr)
			m_type = sega8_get_pcb_id(get_feature("slot") ? get_feature("slot") : "rom");
		else
			m_type = get_cart_type(ROM, len);

		set_lphaser_xoffset(ROM, len);

		setup_ram();

		// Check for gamegear cartridges with PIN 42 set to SMS mode
		if (software_entry() != nullptr)
		{
			const char *pin_42 = get_feature("pin_42");
			if (pin_42 && !strcmp(pin_42, "sms_mode"))
				m_cart->set_sms_mode(1);
		}

		// when loading from fullpath m_late_battery_enable can be TRUE and in that case
		// we attempt to load a battery because the game might have it!
		if (m_cart->get_ram_size() && (m_cart->get_has_battery() || m_cart->get_late_battery()))
			battery_load(m_cart->get_ram_base(), m_cart->get_ram_size(), 0x00);

		//printf("Type: %s\n", sega8_get_slot(type));

		internal_header_logging(ROM + offset, len, m_cart->get_ram_size());

		return IMAGE_INIT_PASS;
	}

	return IMAGE_INIT_PASS;
}


/*-------------------------------------------------
 call_unload
 -------------------------------------------------*/

void sega8_cart_slot_device::call_unload()
{
	if (m_cart && m_cart->get_ram_base() && m_cart->get_ram_size() && m_cart->get_has_battery())
		battery_save(m_cart->get_ram_base(), m_cart->get_ram_size());
}


/*-------------------------------------------------
 call softlist load
 -------------------------------------------------*/

bool sega8_cart_slot_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	load_software_part_region(*this, swlist, swname, start_entry);
	return TRUE;
}


#ifdef UNUSED_FUNCTION
// For the moment we switch to a different detection routine which allows to detect
// in a single run Codemasters mapper, Korean mapper (including Jang Pung 3 which
// uses a diff signature then the one below here) and Zemina mapper (used by Wonsiin, etc.).
// I leave these here to document alt. detection routines and in the case these functions
// can be updated

/* Check for Codemasters mapper
 0x7FE3 - 93 - sms Cosmis Spacehead
 - sms Dinobasher
 - sms The Excellent Dizzy Collection
 - sms Fantastic Dizzy
 - sms Micro Machines
 - gamegear Cosmic Spacehead
 - gamegear Micro Machines
 - 94 - gamegear Dropzone
 - gamegear Ernie Els Golf (also has 64KB additional RAM on the cartridge)
 - gamegear Pete Sampras Tennis
 - gamegear S.S. Lucifer
 - 95 - gamegear Micro Machines 2 - Turbo Tournament

 The Korean game Jang Pung II also seems to use a codemasters style mapper.
 */
int sms_state::detect_codemasters_mapper( UINT8 *rom )
{
	static const UINT8 jang_pung2[16] = { 0x00, 0xba, 0x38, 0x0d, 0x00, 0xb8, 0x38, 0x0c, 0x00, 0xb6, 0x38, 0x0b, 0x00, 0xb4, 0x38, 0x0a };

	if (((rom[0x7fe0] & 0x0f ) <= 9) && (rom[0x7fe3] == 0x93 || rom[0x7fe3] == 0x94 || rom[0x7fe3] == 0x95) &&  rom[0x7fef] == 0x00)
		return 1;

	if (!memcmp(&rom[0x7ff0], jang_pung2, 16))
		return 1;

	return 0;
}


int sms_state::detect_korean_mapper( UINT8 *rom )
{
	static const UINT8 signatures[2][16] =
	{
		{ 0x3e, 0x11, 0x32, 0x00, 0xa0, 0x78, 0xcd, 0x84, 0x85, 0x3e, 0x02, 0x32, 0x00, 0xa0, 0xc9, 0xff }, /* Dodgeball King */
		{ 0x41, 0x48, 0x37, 0x37, 0x44, 0x37, 0x4e, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20 }, /* Sangokushi 3 */
	};
	int i;

	for (i = 0; i < 2; i++)
	{
		if (!memcmp(&rom[0x7ff0], signatures[i], 16))
		{
			return 1;
		}
	}
	return 0;
}
#endif

int sega8_cart_slot_device::get_cart_type(UINT8 *ROM, UINT32 len)
{
	int type = SEGA8_BASE_ROM;

	// Check for special cartridge features (new routine, courtesy of Omar Cornut, from MEKA)
	if (len >= 0x8000)
	{
		int _0002 = 0, _8000 = 0, _a000 = 0, _ffff = 0, _3ffe = 0, _4000 = 0, _6000 = 0;
		for (int i = 0; i < 0x8000; i++)
		{
			if (ROM[i] == 0x32) // Z80 opcode for: LD (xxxx), A
			{
				UINT16 addr = (ROM[i + 2] << 8) | ROM[i + 1];
				if (addr == 0xffff)
				{ i += 2; _ffff++; continue; }
				if (addr == 0x0002 || addr == 0x0003 || addr == 0x0004)
				{ i += 2; _0002++; continue; }
				if (addr == 0x8000)
				{ i += 2; _8000++; continue; }
				if (addr == 0xa000)
				{ i += 2; _a000++; continue; }
				if ( addr == 0x3ffe)
				{ i += 2; _3ffe++; continue; }
				if ( addr == 0x4000 )
				{ i += 2; _4000++; continue; }
				if ( addr == 0x6000 )
				{ i += 2; _6000++; continue; }
			}
		}

		LOG(("Mapper test: _0002 = %d, _8000 = %d, _a000 = %d, _ffff = %d\n", _0002, _8000, _a000, _ffff));

		// 2 is a security measure, although tests on existing ROM showed it was not needed
		if (len > 0x10000 && (_0002 > _ffff + 2 || (_0002 > 0 && _ffff == 0)))
		{
			type = SEGA8_ZEMINA;

			// Check for special bank 0 signature
			if (len == 0x20000 && ROM[0] == 0x00 && ROM[1] == 0x00 && ROM[2] == 0x00 &&
				ROM[0x1e000] == 0xF3 && ROM[0x1e001] == 0xed && ROM[0x1e002] == 0x56)
				type = SEGA8_NEMESIS;
		}
		else if (_8000 > _ffff + 2 || (_8000 > 0 && _ffff == 0))
			type = SEGA8_CODEMASTERS;
		else if (_a000 > _ffff + 2 || (_a000 > 0 && _ffff == 0))
			type = SEGA8_KOREAN;
		else if (_3ffe > _ffff + 2 || _3ffe > 0)
			type = SEGA8_4PAK;
		else if (_4000 > 0 && _6000 > 0 && _8000 > 0 && _a000 > 0)
			type = SEGA8_JANGGUN;
	}

	// Try to detect Dahjee RAM Expansions
	if (len >= 0x8000)
	{
		int x2000_3000 = 0, xd000_e000_f000 = 0, x2000_ff = 0;

		for (int i = 0; i < 0x8000; i++)
		{
			if (ROM[i] == 0x32)
			{
				UINT16 addr = ROM[i + 1] | (ROM[i + 2] << 8);

				switch (addr & 0xf000)
				{
					case 0x2000:
					case 0x3000:
						i += 2;
						x2000_3000++;
						break;

					case 0xd000:
					case 0xe000:
					case 0xf000:
						i += 2;
						xd000_e000_f000++;
						break;
				}
			}
		}
		for (int i = 0x2000; i < 0x4000; i++)
		{
			if (ROM[i] == 0xff)
				x2000_ff++;
		}
		if (x2000_ff == 0x2000 && (xd000_e000_f000 > 10 || x2000_3000 > 10))
		{
			if (xd000_e000_f000 > x2000_3000)
				type = SEGA8_DAHJEE_TYPEB;
			else
				type = SEGA8_DAHJEE_TYPEA;
		}
	}

	// Terebi Oekaki (TV Draw)
	if (len >= 0x13b3 + 7 && !strncmp((const char *)&ROM[0x13b3], "annakmn", 7))
		type = SEGA8_TEREBIOEKAKI;

	// The Castle (ROM+RAM)
	if (len >= 0x1cc3 + 10 && !strncmp((const char *)&ROM[0x1cc3], "ASCII 1986", 10))
		type = SEGA8_CASTLE;

	// BASIC Level 3
	if (len >= 0x6a20 + 29 && !strncmp((const char *)&ROM[0x6a20], "SC-3000 BASIC Level 3 ver 1.0", 29))
		type = SEGA8_BASIC_L3;

	// Music Editor
	if (len >= 0x0841 + 5)
	{
		if (!strncmp((const char *)&ROM[0x0841], "PIANO", 5) || !strncmp((const char *)&ROM[0x0841], "music", 5))
			type = SEGA8_MUSIC_EDITOR;
	}


	return type;
}
/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

std::string sega8_cart_slot_device::get_default_card_software()
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

		type = get_cart_type(&rom[offset], len - offset);
		slot_string = sega8_get_slot(type);

		//printf("type: %s\n", slot_string);
		clear();

		return std::string(slot_string);
	}

	return software_get_default_slot("rom");
}



/*-------------------------------------------------
 read
 -------------------------------------------------*/

READ8_MEMBER(sega8_cart_slot_device::read_cart)
{
	if (m_cart)
		return m_cart->read_cart(space, offset);
	else
		return 0xff;
}

READ8_MEMBER(sega8_cart_slot_device::read_ram)
{
	if (m_cart)
		return m_cart->read_ram(space, offset);
	else
		return 0xff;
}


/*-------------------------------------------------
 write
 -------------------------------------------------*/

WRITE8_MEMBER(sega8_cart_slot_device::write_mapper)
{
	if (m_cart)
		m_cart->write_mapper(space, offset, data);
}

WRITE8_MEMBER(sega8_cart_slot_device::write_cart)
{
	if (m_cart)
		m_cart->write_cart(space, offset, data);
}

WRITE8_MEMBER(sega8_cart_slot_device::write_ram)
{
	if (m_cart)
		m_cart->write_ram(space, offset, data);
}


/*-------------------------------------------------
 Internal header logging
 -------------------------------------------------*/

void sega8_cart_slot_device::internal_header_logging(UINT8 *ROM, UINT32 len, UINT32 nvram_len)
{
	static const char *const system_region[] =
	{
		"",
		"",
		"",
		"Master System Japan",
		"Master System Export",
		"Game Gear Japan",
		"Game Gear Export",
		"Game Gear International",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		""
	};

	static int csum_length[] =
	{
		0x40000,
		0x80000,
		0x100000,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0x1ff0,
		0x3ff0,
		0x7ff0,
		0xcff0,
		0x10000,
		0x20000,
	};

	char reserved[10];
	UINT8 version, csum_size, region, serial[3];
	UINT16 checksum, csum = 0;
	UINT32 csum_end;

	// LOG FILE DETAILS
	logerror("FILE DETAILS\n" );
	logerror("============\n" );
	logerror("Name: %s\n", basename());
	logerror("File Size: 0x%08x\n", (software_entry() == nullptr) ? (int)length() : (int)get_software_region_length("rom"));
	logerror("Detected type: %s\n", sega8_get_slot(m_type));
	logerror("ROM (Allocated) Size: 0x%X\n", len);
	logerror("RAM: %s\n", nvram_len ? "Yes" : "No");
	if (nvram_len)
		logerror("RAM (Allocated) Size: 0x%X - Battery: %s\n", nvram_len, m_cart->get_has_battery() ? "Yes" : "No");
	logerror("\n" );


	// LOG HEADER DETAILS
	if (len < 0x8000)
		return;

	for (int i = 0; i < 10; i++)
		reserved[i] = ROM[0x7ff0 + i];

	checksum = ROM[0x7ffa] | (ROM[0x7ffb] << 8);

	for (int i = 0; i < 3; i++)
		serial[i] = ROM[0x7ffc + i];
	serial[2] &= 0x0f;

	version = (ROM[0x7ffe] & 0xf0) >> 4;

	csum_size = ROM[0x7fff] & 0x0f;
	csum_end = csum_length[csum_size];
	if (!csum_end || csum_end > len)
		csum_end = len;

	region = (ROM[0x7fff] & 0xf0) >> 4;

	// compute cart checksum to compare with expected one
	for (int i = 0; i < csum_end; i++)
	{
		if (i < 0x7ff0 || i >= 0x8000)
		{
			csum += ROM[i];
			csum &= 0xffff;
		}
	}

	logerror("INTERNAL HEADER\n" );
	logerror("===============\n" );
	logerror("Reserved String: %.10s\n", reserved);
	logerror("Region: %s\n", system_region[region]);
	logerror("Checksum: (Expected) 0x%x - (Computed) 0x%x\n", checksum, csum);
	logerror("   [checksum over 0x%X bytes]\n", csum_length[csum_size]);
	logerror("Serial String: %X\n", serial[0] | (serial[1] << 8) | (serial[2] << 16));
	logerror("Software Revision: %x\n", version);
	logerror("\n" );


	if (m_type == SEGA8_CODEMASTERS)
	{
		UINT8 day, month, year, hour, minute;
		csum = 0;

		day = ROM[0x7fe1];
		month = ROM[0x7fe2];
		year = ROM[0x7fe3];
		hour = ROM[0x7fe4];
		minute = ROM[0x7fe5];
		checksum = ROM[0x7fe6] | (ROM[0x7fe7] << 8);
		csum_size = ROM[0x7fe0];

		// compute cart checksum to compare with expected one
		for (int i = 0; i < len; i += 2)
		{
			if (i < 0x7ff0 || i >= 0x8000)
			{
				csum += (ROM[i] | (ROM[i + 1] << 8));
				csum &= 0xffff;
			}
		}

		logerror("CODEMASTERS HEADER\n" );
		logerror("==================\n" );
		logerror("Build date & time: %x/%x/%x %.2x:%.2x\n", day, month, year, hour, minute);
		logerror("Checksum: (Expected) 0x%x - (Computed) 0x%x\n", checksum, csum);
		logerror("   [checksum over 0x%X bytes]\n", csum_size * 0x4000);
		logerror("\n" );
	}
}

// slot interfaces
#include "rom.h"
#include "ccatch.h"
#include "mgear.h"

SLOT_INTERFACE_START(sg1000_cart)
	SLOT_INTERFACE_INTERNAL("rom",  SEGA8_ROM_STD)
	SLOT_INTERFACE_INTERNAL("othello",  SEGA8_ROM_OTHELLO)
	SLOT_INTERFACE_INTERNAL("castle",  SEGA8_ROM_CASTLE)
	SLOT_INTERFACE_INTERNAL("terebi",  SEGA8_ROM_TEREBI)
	SLOT_INTERFACE_INTERNAL("level3",  SEGA8_ROM_BASIC_L3)
	SLOT_INTERFACE_INTERNAL("music_editor",  SEGA8_ROM_MUSIC_EDITOR)
	SLOT_INTERFACE_INTERNAL("dahjee_typea",  SEGA8_ROM_DAHJEE_TYPEA)
	SLOT_INTERFACE_INTERNAL("dahjee_typeb",  SEGA8_ROM_DAHJEE_TYPEB)
	SLOT_INTERFACE_INTERNAL("cardcatcher",  SEGA8_ROM_CARDCATCH)
SLOT_INTERFACE_END

SLOT_INTERFACE_START(sg1000mk3_cart)
	SLOT_INTERFACE_INTERNAL("rom",  SEGA8_ROM_STD)
	SLOT_INTERFACE_INTERNAL("terebi",  SEGA8_ROM_TEREBI)
	SLOT_INTERFACE_INTERNAL("codemasters",  SEGA8_ROM_CODEMASTERS)
	SLOT_INTERFACE_INTERNAL("4pak",  SEGA8_ROM_4PAK)
	SLOT_INTERFACE_INTERNAL("zemina",  SEGA8_ROM_ZEMINA)
	SLOT_INTERFACE_INTERNAL("nemesis",  SEGA8_ROM_NEMESIS)
	SLOT_INTERFACE_INTERNAL("janggun",  SEGA8_ROM_JANGGUN)
	SLOT_INTERFACE_INTERNAL("hicom",  SEGA8_ROM_HICOM)
	SLOT_INTERFACE_INTERNAL("korean",  SEGA8_ROM_KOREAN)
	SLOT_INTERFACE_INTERNAL("korean_nb",  SEGA8_ROM_KOREAN_NB)
	SLOT_INTERFACE_INTERNAL("othello",  SEGA8_ROM_OTHELLO)
	SLOT_INTERFACE_INTERNAL("castle",  SEGA8_ROM_CASTLE)
	SLOT_INTERFACE_INTERNAL("dahjee_typea",  SEGA8_ROM_DAHJEE_TYPEA)
	SLOT_INTERFACE_INTERNAL("dahjee_typeb",  SEGA8_ROM_DAHJEE_TYPEB)
	// are these SC-3000 carts below actually compatible or not? remove if not!
	SLOT_INTERFACE_INTERNAL("level3",  SEGA8_ROM_BASIC_L3)
	SLOT_INTERFACE_INTERNAL("music_editor",  SEGA8_ROM_MUSIC_EDITOR)
SLOT_INTERFACE_END

SLOT_INTERFACE_START(sms_cart)
	SLOT_INTERFACE_INTERNAL("rom",  SEGA8_ROM_STD)
	SLOT_INTERFACE_INTERNAL("codemasters",  SEGA8_ROM_CODEMASTERS)
	SLOT_INTERFACE_INTERNAL("4pak",  SEGA8_ROM_4PAK)
	SLOT_INTERFACE_INTERNAL("zemina",  SEGA8_ROM_ZEMINA)
	SLOT_INTERFACE_INTERNAL("nemesis",  SEGA8_ROM_NEMESIS)
	SLOT_INTERFACE_INTERNAL("janggun",  SEGA8_ROM_JANGGUN)
	SLOT_INTERFACE_INTERNAL("hicom",  SEGA8_ROM_HICOM)
	SLOT_INTERFACE_INTERNAL("korean",  SEGA8_ROM_KOREAN)
	SLOT_INTERFACE_INTERNAL("korean_nb",  SEGA8_ROM_KOREAN_NB)
SLOT_INTERFACE_END

SLOT_INTERFACE_START(gg_cart)
	SLOT_INTERFACE_INTERNAL("rom",  SEGA8_ROM_STD)
	SLOT_INTERFACE_INTERNAL("eeprom",  SEGA8_ROM_EEPROM)
	SLOT_INTERFACE_INTERNAL("codemasters",  SEGA8_ROM_CODEMASTERS)
	SLOT_INTERFACE_INTERNAL("mgear",  SEGA8_ROM_MGEAR)
SLOT_INTERFACE_END
