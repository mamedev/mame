// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(GAMATE_CART_SLOT, gamate_cart_slot_device, "gamate_cart_slot", "Gamate Cartridge Slot")

//**************************************************************************
//    GAMATE cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  device_gamate_cart_interface - constructor
//-------------------------------------------------

device_gamate_cart_interface::device_gamate_cart_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "gamatecart"),
	m_rom(nullptr),
	m_rom_size(0)
{
}


//-------------------------------------------------
//  ~device_gamate_cart_interface - destructor
//-------------------------------------------------

device_gamate_cart_interface::~device_gamate_cart_interface()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_gamate_cart_interface::rom_alloc(uint32_t size, const char *tag)
{
	if (m_rom == nullptr)
	{
		m_rom = device().machine().memory().region_alloc(std::string(tag).append(GAMATESLOT_ROM_REGION_TAG).c_str(), size, 1, ENDIANNESS_BIG)->base();
		m_rom_size = size;
	}
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  gamate_cart_slot_device - constructor
//-------------------------------------------------
gamate_cart_slot_device::gamate_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, GAMATE_CART_SLOT, tag, owner, clock),
	device_image_interface(mconfig, *this),
	device_single_card_slot_interface(mconfig, *this),
	m_type(GAMATE_PLAIN),
	m_cart(nullptr)
{
}

//-------------------------------------------------
//  gamate_cart_slot_device - destructor
//-------------------------------------------------

gamate_cart_slot_device::~gamate_cart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void gamate_cart_slot_device::device_start()
{
	m_cart = get_card_device();
}

//-------------------------------------------------
//  GAMATE PCB
//-------------------------------------------------

struct gamate_slot
{
	int                     pcb_id;
	const char              *slot_option;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const gamate_slot slot_list[] =
{
	{ GAMATE_PLAIN,       "plain" },
	{ GAMATE_BANKED,      "banked" },
	{ GAMATE_4IN1,        "4in1" },
};

static int gamate_get_pcb_id(const char *slot)
{
	for (auto & elem : slot_list)
	{
		if (!core_stricmp(elem.slot_option, slot))
			return elem.pcb_id;
	}

	return 0;
}

static const char *gamate_get_slot(int type)
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

image_init_result gamate_cart_slot_device::call_load()
{
	if (m_cart)
	{
		uint8_t *ROM;
		uint32_t len = !loaded_through_softlist() ? length() : get_software_region_length("rom");

		if (len > 0x80000)
		{
			seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
			return image_init_result::FAIL;
		}

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
				m_type = gamate_get_pcb_id(pcb_name);
		}

		return image_init_result::PASS;
	}

	return image_init_result::PASS;
}


/*-------------------------------------------------
 get_cart_type - code to detect cart type from
 fullpath
 -------------------------------------------------*/

int gamate_cart_slot_device::get_cart_type(const uint8_t *ROM, uint32_t len)
{
	int type = GAMATE_PLAIN;

	switch (len)
	{
		case 0x4000:
		case 0x8000:
			type = GAMATE_PLAIN;
			break;

		case 0x10000:
		case 0x20000:
		case 0x40000:
		case 0x80000:
			type = GAMATE_BANKED;
			break;
	}

	return type;
}


/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

std::string gamate_cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	if (hook.image_file())
	{
		const char *slot_string;
		uint32_t len = hook.image_file()->size();
		std::vector<uint8_t> rom(len);
		int type;

		hook.image_file()->read(&rom[0], len);

		type = get_cart_type(&rom[0], len);
		slot_string = gamate_get_slot(type);

		//printf("type: %s\n", slot_string);

		return std::string(slot_string);
	}

	return software_get_default_slot("plain");
}

/*-------------------------------------------------
 read
 -------------------------------------------------*/

uint8_t gamate_cart_slot_device::read_cart(offs_t offset)
{
	if (m_cart)
	{
		return m_cart->read_cart(offset);
	}
	else
		return 0xff;
}

/*-------------------------------------------------
 write
 -------------------------------------------------*/

void gamate_cart_slot_device::write_cart(offs_t offset, uint8_t data)
{
	if (m_cart)
	{
		m_cart->write_cart(offset, data);
	}
}
