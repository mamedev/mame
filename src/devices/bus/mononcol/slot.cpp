// license:BSD-3-Clause
// copyright-holders:Fabio Priuli

#include "emu.h"
#include "slot.h"

#define VERBOSE 0
#include "logmacro.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(MONONCOL_CARTSLOT, mononcol_cartslot_device, "mononcol_cartslot", "Monon Color Cartridge Slot")


//**************************************************************************
//  device_mononcol_cart_interface
//**************************************************************************

device_mononcol_cart_interface::device_mononcol_cart_interface(machine_config const &mconfig, device_t &device) :
	device_interface(device, "genslot"),
	m_rom(nullptr),
	m_rom_size(0)
{
}

device_mononcol_cart_interface::~device_mononcol_cart_interface()
{
}

void device_mononcol_cart_interface::rom_alloc(u32 size, int width, endianness_t endian)
{
	if (m_rom)
	{
		throw emu_fatalerror(
				"%s: Request to allocate ROM when already allocated (allocated size %u, requested size %u)\n",
				device().tag(),
				m_rom_size,
				size);
	}

	std::string fulltag = device().subtag("^cart:rom");

	device().logerror("Allocating %u byte ROM region with tag '%s' (width %d)\n", size, fulltag, width);
	m_rom = device().machine().memory().region_alloc(fulltag, size, width, endian)->base();
	m_rom_size = size;

	if (!(m_rom_size && !(m_rom_size & (m_rom_size - 1))))
	{
		fatalerror("m_rom_size is NOT a power of 2");
	}
}



//**************************************************************************
//  mononcol_cartslot_device
//**************************************************************************

mononcol_cartslot_device::mononcol_cartslot_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_rom_image_interface(mconfig, *this),
	device_single_card_slot_interface<device_mononcol_cart_interface>(mconfig, *this),
	m_cart(nullptr)
{
}


mononcol_cartslot_device::mononcol_cartslot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mononcol_cartslot_device(mconfig, MONONCOL_CARTSLOT, tag, owner, clock)
{
}

mononcol_cartslot_device::~mononcol_cartslot_device()
{
}

void mononcol_cartslot_device::device_start()
{
	m_cart = get_card_device();
}


/*-------------------------------------------------
 call load
 -------------------------------------------------*/

image_init_result mononcol_cartslot_device::call_load()
{
	if (m_cart)
	{
		u32 len = common_get_size("rom");

		rom_alloc(len, 1, ENDIANNESS_LITTLE);
		common_load_rom(get_rom_base(), len, "rom");

		m_cart->set_spi_region(get_rom_base());
		m_cart->set_spi_size(get_rom_size());

		return image_init_result::PASS;
	}

	return image_init_result::PASS;
}



/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

std::string mononcol_cartslot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("rom");
}


/**************************************************

 Implementation

 **************************************************/


/*-------------------------------------------------
 common_get_size - it gets image file size both
 for fullpath and for softlist
 -------------------------------------------------*/

u32 mononcol_cartslot_device::common_get_size(char const *region)
{
	// if we are loading from softlist, you have to specify a region
	assert(!loaded_through_softlist() || (region != nullptr));

	return !loaded_through_softlist() ? length() : get_software_region_length(region);
}

/*-------------------------------------------------
 common_load_rom - it loads from image file both
 for fullpath and for softlist
 -------------------------------------------------*/

void mononcol_cartslot_device::common_load_rom(u8 *ROM, u32 len, char const *region)
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
