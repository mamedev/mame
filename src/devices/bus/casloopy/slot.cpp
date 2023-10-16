// license:BSD-3-Clause
// copyright-holders:Phil Bennett
/***********************************************************************************************************

    Casio Loopy cart slot emulation

 ***********************************************************************************************************/

#include "emu.h"
#include "slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(CASLOOPY_CART_SLOT, casloopy_cart_slot_device, "casloopy_cart_slot", "Casio Loopy Cartridge Slot")

//**************************************************************************
//    casloopy Cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  device_casloopy_cart_interface - constructor
//-------------------------------------------------

device_casloopy_cart_interface::device_casloopy_cart_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "casloopy_cart"),
	m_rom(nullptr),
	m_rom_size(0)
{
}


//-------------------------------------------------
//  ~device_casloopy_cart_interface - destructor
//-------------------------------------------------

device_casloopy_cart_interface::~device_casloopy_cart_interface()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_casloopy_cart_interface::rom_alloc(uint32_t size)
{
	if (m_rom == nullptr)
	{
		m_rom = (uint16_t *)device().machine().memory().region_alloc(device().subtag("^cart:rom"), size, 2, ENDIANNESS_BIG)->base();
		m_rom_size = size;
	}
}

void device_casloopy_cart_interface::nvram_alloc(uint32_t size)
{
	m_nvram.resize(size);
	device().save_item(NAME(m_nvram));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  casloopy_cart_slot_device - constructor
//-------------------------------------------------
casloopy_cart_slot_device::casloopy_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CASLOOPY_CART_SLOT, tag, owner, clock),
	device_cartrom_image_interface(mconfig, *this),
	device_single_card_slot_interface<device_casloopy_cart_interface>(mconfig, *this),
	m_cart(nullptr)
{
}


//-------------------------------------------------
//  casloopy_cart_slot_device - destructor
//-------------------------------------------------

casloopy_cart_slot_device::~casloopy_cart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void casloopy_cart_slot_device::device_start()
{
	m_cart = get_card_device();
}



/*-------------------------------------------------
 call load
 -------------------------------------------------*/

std::pair<std::error_condition, std::string> casloopy_cart_slot_device::call_load()
{
	if (!loaded_through_softlist())
	{
		m_cart->rom_alloc(length());
		fread(m_cart->get_rom_base(), length());
	}
	else
	{
		const u32 size = get_software_region_length("rom");
		m_cart->rom_alloc(size);
		memcpy(m_cart->get_rom_base(), get_software_region("rom"), size);
	}

	if (get_software_region("nvram"))
	{
		const u32 nvram_size = get_software_region_length("nvram");
		m_cart->nvram_alloc(nvram_size);
		battery_load(m_cart->get_nvram_base(), nvram_size, 0);
	}

	return std::make_pair(std::error_condition(), std::string());
}


/*-------------------------------------------------
 call unload
 -------------------------------------------------*/

void casloopy_cart_slot_device::call_unload()
{
	if (m_cart && m_cart->get_nvram_base() && m_cart->get_nvram_size())
		battery_save(m_cart->get_nvram_base(), m_cart->get_nvram_size());
}


/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

std::string casloopy_cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("std");
}


/*-------------------------------------------------
 read accessors
 -------------------------------------------------*/

uint16_t casloopy_cart_slot_device::read_rom(offs_t offset)
{
	if (m_cart)
		return m_cart->read_rom(offset);
	else
		return 0xffff;
}

uint8_t casloopy_cart_slot_device::read_ram(offs_t offset)
{
	if (m_cart)
		return m_cart->read_ram(offset);
	else
		return 0xff;
}


/*-------------------------------------------------
 write accessors
 -------------------------------------------------*/

void casloopy_cart_slot_device::write_ram(offs_t offset, u8 data)
{
	if (m_cart)
		m_cart->write_ram(offset, data);
}
