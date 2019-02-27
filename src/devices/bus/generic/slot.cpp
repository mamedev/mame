// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

   Generic ROM / RAM Socket and Cartslot device

   This device offers basic RAM / ROM allocation and access

   The available handlers are suited for any situation where a system opens a
   "window" over a ROM or RAM area and we want to access it during emulation.

   This device is not suited whenever the system exposes additional lines to
   the socket/slot: e.g. whenever input/output lines are present, whenever
   there are lines controlling bankswitch / paging, and whenever different
   cart configurations have to be supported (like some PCBs containing ROM
   only, and other containing both ROM and RAM)
   In the latter situations, per-system slot devices have to be created (see
   e.g. APF cart slot device for an example of a simple device with multiple
   pcbs supported)

 ***********************************************************************************************************/


#include "emu.h"
#include "slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(GENERIC_SOCKET, generic_socket_device, "generic_socket", "Generic ROM Socket / RAM Socket")
DEFINE_DEVICE_TYPE(GENERIC_CARTSLOT, generic_cartslot_device, "generic_cartslot", "Generic Cartridge Slot")


//-------------------------------------------------
//  device_generic_cart_interface - constructor
//-------------------------------------------------

device_generic_cart_interface::device_generic_cart_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_rom(nullptr),
		m_rom_size(0),
		m_region(*this, DEVICE_SELF)
{
}


//-------------------------------------------------
//  ~device_generic_cart_interface - destructor
//-------------------------------------------------

device_generic_cart_interface::~device_generic_cart_interface()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_generic_cart_interface::rom_alloc(size_t size, int width, endianness_t endian, const char *tag)
{
	if (m_rom == nullptr)
	{
		m_rom = device().machine().memory().region_alloc(std::string(tag).append(GENERIC_ROM_REGION_TAG).c_str(), size, width, endian)->base();
		m_rom_size = size;
	}
}


//-------------------------------------------------
//  ram_alloc - alloc the space for the ram
//-------------------------------------------------

void device_generic_cart_interface::ram_alloc(uint32_t size)
{
	m_ram.resize(size);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  generic_slot_device - constructor
//-------------------------------------------------
generic_slot_device::generic_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_image_interface(mconfig, *this),
	device_slot_interface(mconfig, *this),
	m_interface(nullptr),
	m_default_card("rom"),
	m_extensions("bin"),
	m_must_be_loaded(false),
	m_width(GENERIC_ROM8_WIDTH),
	m_endianness(ENDIANNESS_LITTLE),
	m_cart(nullptr)
{
}

generic_socket_device::generic_socket_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: generic_slot_device(mconfig, GENERIC_SOCKET, tag, owner, clock)
{
}

generic_cartslot_device::generic_cartslot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: generic_slot_device(mconfig, GENERIC_CARTSLOT, tag, owner, clock)
{
}

//-------------------------------------------------
//  generic_slot_device - destructor
//-------------------------------------------------

generic_slot_device::~generic_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void generic_slot_device::device_start()
{
	m_cart = dynamic_cast<device_generic_cart_interface *>(get_card_device());
}


/*-------------------------------------------------
 call load
 -------------------------------------------------*/

image_init_result generic_slot_device::call_load()
{
	if (m_cart)
	{
		if (!m_device_image_load.isnull())
			return m_device_image_load(*this);
		else
		{
			uint32_t len = common_get_size("rom");

			rom_alloc(len, m_width, m_endianness);
			common_load_rom(get_rom_base(), len, "rom");

			return image_init_result::PASS;
		}
	}

	return image_init_result::PASS;
}


/*-------------------------------------------------
 call_unload
 -------------------------------------------------*/

void generic_slot_device::call_unload()
{
	if (!m_device_image_unload.isnull())
		return m_device_image_unload(*this);
}


/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

std::string generic_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot(m_default_card);
}


/**************************************************

 Implementation

 **************************************************/


/*-------------------------------------------------
 common_get_size - it gets image file size both
 for fullpath and for softlist
 -------------------------------------------------*/

uint32_t generic_slot_device::common_get_size(const char *region)
{
	// if we are loading from softlist, you have to specify a region
	assert(!loaded_through_softlist() || (region != nullptr));

	return !loaded_through_softlist() ? length() : get_software_region_length(region);
}

/*-------------------------------------------------
 common_load_rom - it loads from image file both
 for fullpath and for softlist
 -------------------------------------------------*/

void generic_slot_device::common_load_rom(uint8_t *ROM, uint32_t len, const char *region)
{
	// basic sanity check
	assert((ROM != nullptr) && (len > 0));

	// if we are loading from softlist, you have to specify a region
	assert(!loaded_through_softlist() || (region != nullptr));

	if (!loaded_through_softlist())
		fread(ROM, len);
	else
		memcpy(ROM, get_software_region(region), len);
}

/*-------------------------------------------------
 read_rom
 -------------------------------------------------*/

uint8_t generic_slot_device::read_rom(offs_t offset)
{
	if (m_cart)
		return m_cart->read_rom(offset);
	else
		return 0xff;
}

/*-------------------------------------------------
 read16_rom
 -------------------------------------------------*/

uint16_t generic_slot_device::read16_rom(offs_t offset, uint16_t mem_mask)
{
	if (m_cart)
		return m_cart->read16_rom(offset, mem_mask);
	else
		return 0xffff;
}

/*-------------------------------------------------
 read32_rom
 -------------------------------------------------*/

uint32_t generic_slot_device::read32_rom(offs_t offset, uint32_t mem_mask)
{
	if (m_cart)
		return m_cart->read32_rom(offset, mem_mask);
	else
		return 0xffffffff;
}

/*-------------------------------------------------
 read_ram
 -------------------------------------------------*/

uint8_t generic_slot_device::read_ram(offs_t offset)
{
	if (m_cart)
		return m_cart->read_ram(offset);
	else
		return 0xff;
}

/*-------------------------------------------------
 write_ram
 -------------------------------------------------*/

void generic_slot_device::write_ram(offs_t offset, uint8_t data)
{
	if (m_cart)
		m_cart->write_ram(offset, data);
}
