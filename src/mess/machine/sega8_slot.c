/***********************************************************************************************************



 ***********************************************************************************************************/


#include "emu.h"
#include "machine/sega8_slot.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type SEGA8_CART_SLOT = &device_creator<sega8_cart_slot_device>;


//**************************************************************************
//    SMS cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  device_sega8_cart_interface - constructor
//-------------------------------------------------

device_sega8_cart_interface::device_sega8_cart_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_rom(NULL),
		m_ram(NULL),
		m_rom_size(0),
		m_ram_size(0),
		m_rom_page_count(0),
		has_battery(FALSE),
		m_lphaser_xoffs(0),
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

void device_sega8_cart_interface::rom_alloc(running_machine &machine, UINT32 size)
{
	if (m_rom == NULL)
	{
		m_rom = auto_alloc_array_clear(machine, UINT8, size);
		m_rom_size = size;
		m_rom_page_count = size / 0x4000;
		late_bank_setup();
	}
}


//-------------------------------------------------
//  ram_alloc - alloc the space for the ram
//-------------------------------------------------

void device_sega8_cart_interface::ram_alloc(running_machine &machine, UINT32 size)
{
	if (m_ram == NULL)
	{
		m_ram = auto_alloc_array_clear(machine, UINT8, size);
		m_ram_size = size;
		state_save_register_item_pointer(machine, "SEGA8_CART", NULL, 0, m_ram, m_ram_size);
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sega8_cart_slot_device - constructor
//-------------------------------------------------

sega8_cart_slot_device::sega8_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						device_t(mconfig, SEGA8_CART_SLOT, "Sega Master System / Game Gear / SG1000 Cartridge Slot", tag, owner, clock),
						device_image_interface(mconfig, *this),
						device_slot_interface(mconfig, *this),
						m_type(SEGA8_BASE_ROM),
						m_must_be_loaded(FALSE),
						m_interface("sms_cart"),
						m_extensions("bin")
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
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void sega8_cart_slot_device::device_config_complete()
{
	// set brief and instance name
	update_names();
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
	{ SEGA8_KOREAN_NOBANK, "korean_nobank" }
};

static int sega8_get_pcb_id(const char *slot)
{
	for (int i = 0; i < ARRAY_LENGTH(slot_list); i++)
	{
		if (!mame_stricmp(slot_list[i].slot_option, slot))
			return slot_list[i].pcb_id;
	}

	return 0;
}

static const char *sega8_get_slot(int type)
{
	for (int i = 0; i < ARRAY_LENGTH(slot_list); i++)
	{
		if (slot_list[i].pcb_id == type)
			return slot_list[i].slot_option;
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
	static const UINT8 signatures[6][16] =
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
	};

	int xoff = 51;

	if (size >= 0x8000)
	{
		if (!memcmp(&rom[0x7ff0], signatures[0], 16) || !memcmp(&rom[0x7ff0], signatures[1], 16))
			xoff = 41;
		
		if (!memcmp(&rom[0x7ff0], signatures[2], 16))
			xoff = 50;
		
		if (!memcmp(&rom[0x7ff0], signatures[3], 16))
			xoff = 48;
		
		if (!memcmp(&rom[0x7ff0], signatures[4], 16))
			xoff = 45;
		
		if (!memcmp(&rom[0x7ff0], signatures[5], 16))
			xoff = 54;
		
	}

	m_cart->set_lphaser_xoffs(xoff);
}


bool sega8_cart_slot_device::call_load()
{
	if (m_cart)
	{
		UINT32 len = (software_entry() == NULL) ? length() : get_software_region_length("rom");
		UINT32 offset = 0;
		UINT8 *ROM;
		
		// check for header
		if ((len % 0x4000) == 512)
		{
			offset = 512;
			len -= 512;
		}
		
		// make sure that we only get complete (0x4000) rom banks
		if (len & 0x3fff)
			len = ((len >> 14) + 1) << 14;
		
		m_cart->rom_alloc(machine(), len);
		ROM = m_cart->get_rom_base();
		
		if (software_entry() == NULL)
		{
			fseek(offset, SEEK_SET);
			fread(ROM, len);
		}
		else
			memcpy(ROM, get_software_region("rom") + offset, len);
		
		/* check the image */
		if (verify_cart(ROM, len) == IMAGE_VERIFY_FAIL)
			logerror("Warning loading image: verify_cart failed\n");

		if (software_entry() != NULL)
			m_type = sega8_get_pcb_id(get_feature("slot") ? get_feature("slot") : "rom");
		else
			m_type = get_cart_type(ROM, len);


		if (m_type == SEGA8_CODEMASTERS)
		{
			m_cart->ram_alloc(machine(), 0x10000);
			m_cart->set_has_battery(FALSE);
		}
		else
		{
			// for now
			m_cart->ram_alloc(machine(), 0x08000);
			m_cart->set_has_battery(TRUE);
		}
		
		set_lphaser_xoffset(ROM, len);

		
		// Check for gamegear cartridges with PIN 42 set to SMS mode
		if (software_entry() != NULL)
		{
			const char *pin_42 = get_feature("pin_42");
			if (pin_42 && !strcmp(pin_42, "sms_mode"))
				m_cart->set_sms_mode(1);
		}

		// for now we always attempt to load a battery, but we only save it if ram is actually accessed
		if (m_cart->get_ram_size() /*&& m_cart->get_has_battery()*/)
			battery_load(m_cart->get_ram_base(), m_cart->get_ram_size(), 0x00);
		
		//printf("Type: %s\n", sega8_get_slot(type));
		
		internal_header_logging(ROM + offset, len);
		
		return IMAGE_INIT_PASS;
	}

	return IMAGE_INIT_PASS;
}


/*-------------------------------------------------
 call_unload
 -------------------------------------------------*/

void sega8_cart_slot_device::call_unload()
{
	if (m_cart && m_cart->get_ram_size() && m_cart->get_has_battery())
		battery_save(m_cart->get_ram_base(), m_cart->get_ram_size());
}


/*-------------------------------------------------
 call softlist load
 -------------------------------------------------*/

bool sega8_cart_slot_device::call_softlist_load(char *swlist, char *swname, rom_entry *start_entry)
{
	load_software_part_region(this, swlist, swname, start_entry);
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
	static const UINT8 terebi_oekaki[7] = { 0x61, 0x6e, 0x6e, 0x61, 0x6b, 0x6d, 0x6e }; // "annakmn"

	/* Check for special cartridge features (new routine, courtesy of Omar Cornut, from MEKA)  */
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
		if (_0002 > _ffff + 2 || (_0002 > 0 && _ffff == 0))
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

	// Terebi Oekaki (TV Draw) is a SG1000 game with special input device which is compatible with SG1000 Mark III
	if (!memcmp(&ROM[0x13b3], terebi_oekaki, 7))
		type = SEGA8_TEREBIOEKAKI;
	
	return type;
}
/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

const char * sega8_cart_slot_device::get_default_card_software(const machine_config &config, emu_options &options)
{
	if (open_image_file(options))
	{
		const char *slot_string = "rom";
		UINT32 len = core_fsize(m_file), offset = 0;
		UINT8 *ROM = global_alloc_array(UINT8, len);
		int type;

		core_fread(m_file, ROM, len);

		if ((len % 0x4000) == 512)
			offset = 512;

		type = get_cart_type(ROM + offset, len - offset);
		slot_string = sega8_get_slot(type);

		//printf("type: %s\n", slot_string);
		global_free(ROM);
		clear();

		return slot_string;
	}

	return software_get_default_slot(config, options, this, "rom");
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


/*-------------------------------------------------
 Internal header logging
 -------------------------------------------------*/

void sega8_cart_slot_device::internal_header_logging(UINT8 *ROM, UINT32 len)
{
}
