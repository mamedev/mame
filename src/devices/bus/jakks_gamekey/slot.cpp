// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(JAKKS_GAMEKEY_SLOT, jakks_gamekey_slot_device, "jakks_gamekey_slot", "JAKKS Pacific Gamekey Slot")

//**************************************************************************
//    JAKKS GAMEKEY cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  device_jakks_gamekey_interface - constructor
//-------------------------------------------------

device_jakks_gamekey_interface::device_jakks_gamekey_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "jakksgamekey"),
	m_rom(nullptr),
	m_rom_size(0)
{
}


//-------------------------------------------------
//  ~device_jakks_gamekey_interface - destructor
//-------------------------------------------------

device_jakks_gamekey_interface::~device_jakks_gamekey_interface()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_jakks_gamekey_interface::rom_alloc(uint32_t size, const char *tag)
{
	if (m_rom == nullptr)
	{
		m_rom = device().machine().memory().region_alloc(std::string(tag).append(JAKKSSLOT_ROM_REGION_TAG).c_str(), size, 1, ENDIANNESS_BIG)->base();
		m_rom_size = size;
	}
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  jakks_gamekey_slot_device - constructor
//-------------------------------------------------
jakks_gamekey_slot_device::jakks_gamekey_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, JAKKS_GAMEKEY_SLOT, tag, owner, clock),
	device_image_interface(mconfig, *this),
	device_single_card_slot_interface<device_jakks_gamekey_interface>(mconfig, *this),
	m_type(JAKKS_GAMEKEY_PLAIN),
	m_cart(nullptr)
{
}

//-------------------------------------------------
//  jakks_gamekey_slot_device - destructor
//-------------------------------------------------

jakks_gamekey_slot_device::~jakks_gamekey_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void jakks_gamekey_slot_device::device_start()
{
	m_cart = get_card_device();
}

//-------------------------------------------------
//  JAKKS GAMEKEY PCB
//-------------------------------------------------

struct jakks_gamekey_slot
{
	int                     pcb_id;
	const char              *slot_option;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const jakks_gamekey_slot slot_list[] =
{
	{ JAKKS_GAMEKEY_PLAIN,       "plain" },
	{ JAKKS_GAMEKEY_I2C_BASE,    "i2c_base" },
	{ JAKKS_GAMEKEY_I2C_24LC04,  "rom_24lc04" },
};

static int jakks_gamekey_get_pcb_id(const char *slot)
{
	for (auto & elem : slot_list)
	{
		if (!core_stricmp(elem.slot_option, slot))
			return elem.pcb_id;
	}

	return 0;
}

static const char *jakks_gamekey_get_slot(int type)
{
	for (auto & elem : slot_list)
	{
		if (elem.pcb_id == type)
			return elem.slot_option;
	}

	return "plain";
}


/*-------------------------------------------------
 call load
 -------------------------------------------------*/

image_init_result jakks_gamekey_slot_device::call_load()
{
	if (m_cart)
	{
		uint8_t *ROM;
		uint32_t len = !loaded_through_softlist() ? length() : get_software_region_length("rom");

		m_cart->rom_alloc(len, tag());

		ROM = m_cart->get_rom_base();

		if (!loaded_through_softlist())
			fread(ROM, len);
		else
			memcpy(ROM, get_software_region("rom"), len);

		if (!loaded_through_softlist())
		{
			// attempt to detect cart type without softlist assistance
			m_type = get_cart_type(ROM, len);
		}
		else
		{
			// or for softlist loading, use the type specified
			const char *pcb_name = get_feature("slot");
			if (pcb_name)
				m_type = jakks_gamekey_get_pcb_id(pcb_name);
		}

		return image_init_result::PASS;
	}

	return image_init_result::PASS;
}


/*-------------------------------------------------
 get_cart_type - code to detect cart type from
 fullpath
 -------------------------------------------------*/

int jakks_gamekey_slot_device::get_cart_type(const uint8_t *ROM, uint32_t len)
{
	// without code analysis we have no way of knowing.
	int type = JAKKS_GAMEKEY_PLAIN;
	return type;
}


/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

std::string jakks_gamekey_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	if (hook.image_file())
	{
		const char *slot_string;
		uint32_t len = hook.image_file()->size();
		std::vector<uint8_t> rom(len);
		int type;

		hook.image_file()->read(&rom[0], len);

		type = get_cart_type(&rom[0], len);
		slot_string = jakks_gamekey_get_slot(type);

		printf("type: %s\n", slot_string);

		return std::string(slot_string);
	}

	return software_get_default_slot("plain");
}

/*-------------------------------------------------
 read
 -------------------------------------------------*/

uint16_t jakks_gamekey_slot_device::read_cart(offs_t offset)
{
	return m_cart->read_cart(offset);
}

/*-------------------------------------------------
 write
 -------------------------------------------------*/

void jakks_gamekey_slot_device::write_cart(offs_t offset, uint16_t data)
{
	m_cart->write_cart(offset, data);
}

/*-------------------------------------------------
 read seeprom
 -------------------------------------------------*/

uint8_t jakks_gamekey_slot_device::read_cart_seeprom(void)
{
	return m_cart->read_cart_seeprom();
}

/*-------------------------------------------------
 write seeprom
 -------------------------------------------------*/

void jakks_gamekey_slot_device::write_cart_seeprom(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_cart->write_cart_seeprom(offset, data);
}
