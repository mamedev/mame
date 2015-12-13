// license:BSD-3-Clause
// copyright-holders:R. Belmont,Ryan Holtz,Fabio Priuli
/***********************************************************************************************************


    Game Boy Advance cart emulation
    (through slot devices)

 ***********************************************************************************************************/


#include "emu.h"
#include "gba_slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type GBA_CART_SLOT = &device_creator<gba_cart_slot_device>;

//**************************************************************************
//    GBA cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  device_gba_cart_interface - constructor
//-------------------------------------------------

device_gba_cart_interface::device_gba_cart_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_rom(nullptr),
		m_rom_size(0)
{
}


//-------------------------------------------------
//  ~device_gba_cart_interface - destructor
//-------------------------------------------------

device_gba_cart_interface::~device_gba_cart_interface()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_gba_cart_interface::rom_alloc(UINT32 size, const char *tag)
{
	if (m_rom == nullptr)
	{
		// we always alloc 32MB of rom region!
		m_rom = (UINT32 *)device().machine().memory().region_alloc(std::string(tag).append(GBASLOT_ROM_REGION_TAG).c_str(), 0x2000000, 4, ENDIANNESS_LITTLE)->base();
		m_rom_size = size;
	}
}


//-------------------------------------------------
//  nvram_alloc - alloc the space for the ram
//-------------------------------------------------

void device_gba_cart_interface::nvram_alloc(UINT32 size)
{
	m_nvram.resize(size/sizeof(UINT32));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  gba_cart_slot_device - constructor
//-------------------------------------------------
gba_cart_slot_device::gba_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						device_t(mconfig, GBA_CART_SLOT, "Game Boy Advance Cartridge Slot", tag, owner, clock, "gba_cart_slot", __FILE__),
						device_image_interface(mconfig, *this),
						device_slot_interface(mconfig, *this),
						m_type(GBA_STD), m_cart(nullptr)
{
}


//-------------------------------------------------
//  gba_cart_slot_device - destructor
//-------------------------------------------------

gba_cart_slot_device::~gba_cart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void gba_cart_slot_device::device_start()
{
	m_cart = dynamic_cast<device_gba_cart_interface *>(get_card_device());
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void gba_cart_slot_device::device_config_complete()
{
	// set brief and instance name
	update_names();
}


//-------------------------------------------------
//  GBA PCB
//-------------------------------------------------

struct gba_slot
{
	int                     pcb_id;
	const char              *slot_option;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const gba_slot slot_list[] =
{
	{ GBA_STD, "gba_rom" },
	{ GBA_SRAM, "gba_sram" },
	{ GBA_EEPROM, "gba_eeprom" },
	{ GBA_EEPROM4, "gba_eeprom_4k" },
	{ GBA_EEPROM64, "gba_eeprom_64k" },
	{ GBA_FLASH, "gba_flash" },
	{ GBA_FLASH512, "gba_flash_512" },
	{ GBA_FLASH1M, "gba_flash_1m" },
};

static int gba_get_pcb_id(const char *slot)
{
	for (auto & elem : slot_list)
	{
		if (!core_stricmp(elem.slot_option, slot))
			return elem.pcb_id;
	}

	return 0;
}

static const char *gba_get_slot(int type)
{
	for (auto & elem : slot_list)
	{
		if (elem.pcb_id == type)
			return elem.slot_option;
	}

	return "gba_rom";
}


/*-------------------------------------------------
 call load
 -------------------------------------------------*/

bool gba_cart_slot_device::call_load()
{
	if (m_cart)
	{
		UINT8 *ROM;
		UINT32 size = (software_entry() != nullptr) ? get_software_region_length("rom") : length();
		if (size > 0x2000000)
		{
			seterror(IMAGE_ERROR_UNSPECIFIED, "Attempted loading a cart larger than 32MB");
			return IMAGE_INIT_FAIL;
		}

		m_cart->rom_alloc(size, tag());
		ROM = (UINT8 *)m_cart->get_rom_base();

		if (software_entry() == nullptr)
		{
			fread(ROM, size);
			m_type = get_cart_type(ROM, size);
		}
		else
		{
			const char *pcb_name = get_feature("slot");

			memcpy(ROM, get_software_region("rom"), size);

			if (pcb_name)
				m_type = gba_get_pcb_id(pcb_name);

			//printf("Type: %s\n", gba_get_slot(m_type));

			osd_printf_info("GBA: Detected (XML) %s\n", pcb_name ? pcb_name : "NONE");
		}

		if (m_type == GBA_SRAM)
			m_cart->nvram_alloc(0x10000);

		// mirror the ROM
		switch (size)
		{
			case 2 * 1024 * 1024:
				memcpy(ROM + 0x200000, ROM, 0x200000);
				// intentional fall-through
			case 4 * 1024 * 1024:
				memcpy(ROM + 0x400000, ROM, 0x400000);
				// intentional fall-through
			case 8 * 1024 * 1024:
				memcpy(ROM + 0x800000, ROM, 0x800000);
				// intentional fall-through
			case 16 * 1024 * 1024:
				memcpy(ROM + 0x1000000, ROM, 0x1000000);
				break;
		}

		if (m_cart->get_nvram_size())
			battery_load(m_cart->get_nvram_base(), m_cart->get_nvram_size(), 0x00);

		return IMAGE_INIT_PASS;
	}

	return IMAGE_INIT_PASS;
}


/*-------------------------------------------------
 call_unload
 -------------------------------------------------*/

void gba_cart_slot_device::call_unload()
{
	if (m_cart && m_cart->get_nvram_size())
		battery_save(m_cart->get_nvram_base(), m_cart->get_nvram_size());
}



/*-------------------------------------------------
 call softlist load
 -------------------------------------------------*/

bool gba_cart_slot_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	load_software_part_region(*this, swlist, swname, start_entry);
	return TRUE;
}



/*-------------------------------------------------
 get_cart_type - code to detect NVRAM type from
 fullpath
 -------------------------------------------------*/

static inline std::string gba_chip_string( UINT32 chip )
{
	std::string str;
	if (chip == 0) str += "NONE ";
	if (chip & GBA_CHIP_EEPROM) str += "EEPROM ";
	if (chip & GBA_CHIP_EEPROM_64K) str += "EEPROM_64K ";
	if (chip & GBA_CHIP_EEPROM_4K) str += "EEPROM_4K ";
	if (chip & GBA_CHIP_FLASH) str += "FLASH ";
	if (chip & GBA_CHIP_FLASH_1M) str += "FLASH_1M ";
	if (chip & GBA_CHIP_FLASH_512) str += "FLASH_512 ";
	if (chip & GBA_CHIP_SRAM) str += "SRAM ";
	if (chip & GBA_CHIP_RTC) str += "RTC ";
	strtrimspace(str);
	return str;
}


static inline int gba_chip_has_conflict( UINT32 chip )
{
	int count1 = 0, count2 = 0;
	if (chip & GBA_CHIP_EEPROM) count1++;
	if (chip & GBA_CHIP_EEPROM_4K) count1++;
	if (chip & GBA_CHIP_EEPROM_64K) count1++;
	if (chip & GBA_CHIP_FLASH) count2++;
	if (chip & GBA_CHIP_FLASH_1M) count2++;
	if (chip & GBA_CHIP_FLASH_512) count2++;
	if (chip & GBA_CHIP_SRAM) count2++;
	return (count1 + count2) > 1; // if EEPROM + FLASH or EEPROM + SRAM carts exist, change to "(count1 > 1) || (count2 > 1)"
}


int gba_cart_slot_device::get_cart_type(UINT8 *ROM, UINT32 len)
{
	UINT32 chip = 0;
	int type = GBA_STD;

	// first detect nvram type based on strings inside the file
	for (int i = 0; i < len; i++)
	{
		if (!memcmp(&ROM[i], "EEPROM_V", 8))
			chip |= GBA_CHIP_EEPROM; // should be either GBA_CHIP_EEPROM_4K or GBA_CHIP_EEPROM_64K, but it is not yet possible to automatically detect which one
		else if ((!memcmp(&ROM[i], "SRAM_V", 6)) || (!memcmp(&ROM[i], "SRAM_F_V", 8))) // || (!memcmp(&data[i], "ADVANCEWARS", 11))) //advance wars 1 & 2 has SRAM, but no "SRAM_" string can be found inside the ROM space
			chip |= GBA_CHIP_SRAM;
		else if (!memcmp(&ROM[i], "FLASH1M_V", 9))
			chip |= GBA_CHIP_FLASH_1M;
		else if (!memcmp(&ROM[i], "FLASH512_V", 10))
			chip |= GBA_CHIP_FLASH_512;
		else if (!memcmp(&ROM[i], "FLASH_V", 7))
			chip |= GBA_CHIP_FLASH;
		else if (!memcmp(&ROM[i], "SIIRTC_V", 8))
			chip |= GBA_CHIP_RTC;
	}
	osd_printf_info("GBA: Detected (ROM) %s\n", gba_chip_string(chip).c_str());

	// fix for games which return more than one kind of chip: either it is one of the known titles, or we default to no battery
	if (gba_chip_has_conflict(chip))
	{
		char game_code[5] = { 0 };
		bool resolved = FALSE;

		if (len >= 0xac + 4)
			memcpy(game_code, ROM + 0xac, 4);

		osd_printf_info("GBA: Game Code \"%s\"\n", game_code);

		chip &= ~(GBA_CHIP_EEPROM | GBA_CHIP_EEPROM_4K | GBA_CHIP_EEPROM_64K | GBA_CHIP_FLASH | GBA_CHIP_FLASH_1M | GBA_CHIP_FLASH_512 | GBA_CHIP_SRAM);

		// search if it is one of the known titles with NVRAM conflicts
		for (auto & elem : gba_chip_fix_conflict_list)
		{
			const gba_chip_fix_conflict_item *item = &elem;
			if (!strcmp(game_code, item->game_code))
			{
				chip |= item->chip;
				resolved = TRUE;
				break;
			}
		}
		if (!resolved)
			osd_printf_warning("GBA: NVRAM is disabled because multiple NVRAM chips were detected!\n");
	}

	// fix for games which are known to require an eeprom with 14-bit addressing (64 kbit)
	if (chip & GBA_CHIP_EEPROM)
	{
		char game_code[5] = { 0 };

		if (len >= 0xac + 4)
			memcpy(game_code, ROM + 0xac, 4);

		osd_printf_info("GBA: Game Code \"%s\"\n", game_code);

		for (auto & elem : gba_chip_fix_eeprom_list)
		{
			const gba_chip_fix_eeprom_item *item = &elem;
			if (!strcmp(game_code, item->game_code))
			{
				chip = (chip & ~GBA_CHIP_EEPROM) | GBA_CHIP_EEPROM_64K;
				break;
			}
		}
	}

	if (chip & GBA_CHIP_RTC)
	{
		osd_printf_info("game has RTC - not emulated at the moment\n");
		chip &= ~GBA_CHIP_RTC;
	}

	osd_printf_info("GBA: Emulate %s\n", gba_chip_string(chip).c_str());

	switch (chip)
	{
		case GBA_CHIP_SRAM:
			type = GBA_SRAM;
			break;
		case GBA_CHIP_EEPROM:
			type = GBA_EEPROM;
			break;
		case GBA_CHIP_EEPROM_4K:
			type = GBA_EEPROM4;
			break;
		case GBA_CHIP_EEPROM_64K:
			type = GBA_EEPROM64;
			break;
		case GBA_CHIP_FLASH:
			type = GBA_FLASH;
			break;
		case GBA_CHIP_FLASH_512:
			type = GBA_FLASH512;
			break;
		case GBA_CHIP_FLASH_1M:
			type = GBA_FLASH1M;
			break;
		default:
			break;
	}

	return type;
}
/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

void gba_cart_slot_device::get_default_card_software(std::string &result)
{
	if (open_image_file(mconfig().options()))
	{
		const char *slot_string = "gba_rom";
		UINT32 len = core_fsize(m_file);
		dynamic_buffer rom(len);
		int type;

		core_fread(m_file, &rom[0], len);

		type = get_cart_type(&rom[0], len);
		slot_string = gba_get_slot(type);

		//printf("type: %s\n", slot_string);
		clear();

		result.assign(slot_string);
		return;
	}

	software_get_default_slot(result, "gba_rom");
}

/*-------------------------------------------------
 read
 -------------------------------------------------*/

READ32_MEMBER(gba_cart_slot_device::read_rom)
{
	if (m_cart)
		return m_cart->read_rom(space, offset, mem_mask);
	else
		return 0xffffffff;
}

READ32_MEMBER(gba_cart_slot_device::read_ram)
{
	if (m_cart)
		return m_cart->read_ram(space, offset, mem_mask);
	else
		return 0xffffffff;
}


/*-------------------------------------------------
 write
 -------------------------------------------------*/

WRITE32_MEMBER(gba_cart_slot_device::write_ram)
{
	if (m_cart)
		m_cart->write_ram(space, offset, data, mem_mask);
}


/*-------------------------------------------------
 Internal header logging
 -------------------------------------------------*/

void gba_cart_slot_device::internal_header_logging(UINT8 *ROM, UINT32 len)
{
}
