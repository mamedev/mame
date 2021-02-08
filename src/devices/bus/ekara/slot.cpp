// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(EKARA_CART_SLOT, ekara_cart_slot_device, "ekara_cart_slot", "e-kara Cartridge Slot")

//**************************************************************************
//    EKARA cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  device_ekara_cart_interface - constructor
//-------------------------------------------------

device_ekara_cart_interface::device_ekara_cart_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "ekaracart"),
	m_rom(nullptr),
	m_rom_size(0)
{
}


//-------------------------------------------------
//  ~device_ekara_cart_interface - destructor
//-------------------------------------------------

device_ekara_cart_interface::~device_ekara_cart_interface()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_ekara_cart_interface::rom_alloc(uint32_t size, const char *tag)
{
	if (m_rom == nullptr)
	{
		m_rom = device().machine().memory().region_alloc(std::string(tag).append(EKARASLOT_ROM_REGION_TAG).c_str(), size, 1, ENDIANNESS_BIG)->base();
		m_rom_size = size;
	}
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ekara_cart_slot_device - constructor
//-------------------------------------------------
ekara_cart_slot_device::ekara_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, EKARA_CART_SLOT, tag, owner, clock),
	device_image_interface(mconfig, *this),
	device_single_card_slot_interface<device_ekara_cart_interface>(mconfig, *this),
	m_type(EKARA_PLAIN),
	m_cart(nullptr)
{
}

//-------------------------------------------------
//  ekara_cart_slot_device - destructor
//-------------------------------------------------

ekara_cart_slot_device::~ekara_cart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ekara_cart_slot_device::device_start()
{
	m_cart = get_card_device();
}

//-------------------------------------------------
//  EKARA PCB
//-------------------------------------------------

struct ekara_slot
{
	int                     pcb_id;
	const char              *slot_option;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const ekara_slot slot_list[] =
{
	{ EKARA_PLAIN,       "plain" },
	{ EKARA_I2C_BASE,    "i2c_base" },
	{ EKARA_I2C_24C08,   "rom_24c08" },
	{ EKARA_I2C_24LC04,  "rom_24lc04" },
	{ EKARA_I2C_24LC02,  "rom_24lc02" },
};

static int ekara_get_pcb_id(const char *slot)
{
	for (auto & elem : slot_list)
	{
		if (!strcmp(elem.slot_option, slot))
			return elem.pcb_id;
	}

	return 0;
}

static const char *ekara_get_slot(int type)
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

image_init_result ekara_cart_slot_device::call_load()
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
				m_type = ekara_get_pcb_id(pcb_name);
		}

		return image_init_result::PASS;
	}

	return image_init_result::PASS;
}


/*-------------------------------------------------
 get_cart_type - code to detect cart type from
 fullpath
 -------------------------------------------------*/

int ekara_cart_slot_device::get_cart_type(const uint8_t *ROM, uint32_t len)
{
	// without code analysis we have no way of knowing.
	int type = EKARA_PLAIN;
	return type;
}


/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

std::string ekara_cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	if (hook.image_file())
	{
		const char *slot_string;
		uint32_t len = hook.image_file()->size();
		std::vector<uint8_t> rom(len);
		int type;

		hook.image_file()->read(&rom[0], len);

		type = get_cart_type(&rom[0], len);
		slot_string = ekara_get_slot(type);

		printf("type: %s\n", slot_string);

		return std::string(slot_string);
	}

	return software_get_default_slot("plain");
}

/*-------------------------------------------------
 read
 -------------------------------------------------*/

uint8_t ekara_cart_slot_device::read_cart(offs_t offset)
{
	return m_cart->read_cart(offset);
}

/*-------------------------------------------------
 write
 -------------------------------------------------*/

void ekara_cart_slot_device::write_cart(offs_t offset, uint8_t data)
{
	m_cart->write_cart(offset, data);
}

/*-------------------------------------------------
 read extra
 -------------------------------------------------*/

uint8_t ekara_cart_slot_device::read_extra(offs_t offset)
{
	return m_cart->read_extra(offset);
}

/*-------------------------------------------------
 write extra
 -------------------------------------------------*/

void ekara_cart_slot_device::write_extra(offs_t offset, uint8_t data)
{
	m_cart->write_extra(offset, data);
}

/*-------------------------------------------------
 write control
 -------------------------------------------------*/

void ekara_cart_slot_device::write_bus_control(offs_t offset, uint8_t data)
{
	m_cart->write_bus_control(offset, data);
}

bool ekara_cart_slot_device::is_read_access_not_rom(void)
{
	return m_cart->is_read_access_not_rom();
}

bool ekara_cart_slot_device::is_write_access_not_rom(void)
{
	return m_cart->is_write_access_not_rom();
}

/*-------------------------------------------------
 direct seeprom access (popira2, gc0010)
 -------------------------------------------------*/

WRITE_LINE_MEMBER(ekara_cart_slot_device::write_sda)
{
	m_cart->write_sda(state);
}

WRITE_LINE_MEMBER(ekara_cart_slot_device::write_scl)
{
	m_cart->write_scl(state);
}

READ_LINE_MEMBER(ekara_cart_slot_device::read_sda )
{
	return  m_cart->read_sda();
}
