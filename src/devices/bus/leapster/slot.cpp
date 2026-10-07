// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(LEAPSTER_SLOT, leapster_slot_device, "leapster_slot", "LeapFrog Leapster Cartridge Slot")

//**************************************************************************
//    LeapFrog Leapster cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  device_leapster_interface - constructor
//-------------------------------------------------

device_leapster_interface::device_leapster_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "leapstercart"),
	m_rom(nullptr),
	m_rom_size(0)
{
}


//-------------------------------------------------
//  ~device_leapster_interface - destructor
//-------------------------------------------------

device_leapster_interface::~device_leapster_interface()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_leapster_interface::rom_alloc(uint32_t size, const char *tag)
{
	if (m_rom == nullptr)
	{
		m_rom = device().machine().memory().region_alloc(std::string(tag).append(LEAPSTER_ROM_REGION_TAG), size, 1, ENDIANNESS_BIG)->base();
		m_rom_size = size;
	}
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  leapster_slot_device - constructor
//-------------------------------------------------
leapster_slot_device::leapster_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, LEAPSTER_SLOT, tag, owner, clock),
	device_cartrom_image_interface(mconfig, *this),
	device_single_card_slot_interface<device_leapster_interface>(mconfig, *this),
	m_type(LEAPSTER_PLAIN),
	m_cart(nullptr)
{
}

//-------------------------------------------------
//  leapster_slot_device - destructor
//-------------------------------------------------

leapster_slot_device::~leapster_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void leapster_slot_device::device_start()
{
	m_cart = get_card_device();
}


/*-------------------------------------------------
 call load
 -------------------------------------------------*/

std::pair<std::error_condition, std::string> leapster_slot_device::call_load()
{
	if (!m_cart)
		return std::make_pair(std::error_condition(), std::string());

	uint32_t const len = !loaded_through_softlist() ? length() : get_software_region_length("rom");

	if (len > 0x100'0000)
		return std::make_pair(image_error::INVALIDLENGTH, "Cartridges larger than 16MB are not supported");

	m_cart->rom_alloc(len, tag());

	uint8_t *const ROM = m_cart->get_rom_base();

	if (!loaded_through_softlist())
	{
		const u32 cnt = fread(ROM, len);
		if (cnt != len)
			return std::make_pair(std::errc::io_error, "Error reading cartridge file");
	}
	else
	{
		memcpy(ROM, get_software_region("rom"), len);
	}

	if (!loaded_through_softlist())
	{
		// for now we assume a non-softlisted ROM is a 'plain' cartridge, no NVRAM
		m_type = LEAPSTER_PLAIN;
	}
	else
	{
		// or for softlist loading, use the type specified
		const char *pcb_name = get_feature("slot");
		if (pcb_name)
			m_type = leapster_get_pcb_id(pcb_name);
	}
	
	return std::make_pair(std::error_condition(), std::string());
}

/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

std::string leapster_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("plain");
}

/*-------------------------------------------------
 read
 -------------------------------------------------*/

uint16_t leapster_slot_device::read_cart(offs_t offset)
{
	return m_cart->read_cart(offset);
}

/*-------------------------------------------------
 write
 -------------------------------------------------*/

void leapster_slot_device::write_cart(offs_t offset, uint16_t data)
{
	m_cart->write_cart(offset, data);
}

/*-------------------------------------------------
 read seeprom
 -------------------------------------------------*/

uint8_t leapster_slot_device::read_nvram(uint16_t offset)
{
	return m_cart->read_nvram(offset);
}

void leapster_slot_device::write_nvram(uint16_t offset, uint8_t data)
{
	m_cart->write_nvram(offset, data);
}


