// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

 Game Boy/Mega Duck cartridge slot interface

 The following ranges are available for use by the cartridge:
 * 0x0000-0x7FFF - Game Boy boot ROM expects to find header at 0x0100.
                   Writes typically used to control memory mapping.
 * 0xA000-0xBFFF - Typically used for additional RAM, sometimes
                   battery-backed.  Sometimes used for I/O.

 **************************************************************************/

#include "emu.h"
#include "slot.h"

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


//**************************************************************************
//  gb_cart_slot_device_base
//**************************************************************************

void gb_cart_slot_device_base::call_unload()
{
	if (m_cart)
		m_cart->unload();
}


gb_cart_slot_device_base::gb_cart_slot_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_cartrom_image_interface(mconfig, *this),
	device_single_card_slot_interface<device_gb_cart_interface>(mconfig, *this),
	m_space(*this, finder_base::DUMMY_TAG, 8),
	m_cart(nullptr)
{
}


void gb_cart_slot_device_base::device_start()
{
	m_cart = get_card_device();
}


image_init_result gb_cart_slot_device_base::call_load()
{
	if (!m_cart)
		return image_init_result::PASS;

	image_init_result result;
	if (!loaded_through_softlist())
	{
		result = load_image_file(image_core_file());
		if (image_init_result::PASS != result)
			return result;
	}
	std::string message;
	result = m_cart->load(message);
	if (image_init_result::PASS != result)
		seterror(image_error::INVALIDIMAGE, message.c_str());
	return result;
}



//**************************************************************************
//  device_gb_cart_interface
//**************************************************************************

void device_gb_cart_interface::unload()
{
}


device_gb_cart_interface::device_gb_cart_interface(machine_config const &mconfig, device_t &device) :
	device_interface(device, "gameboycart"),
	m_slot(dynamic_cast<gb_cart_slot_device_base *>(device.owner()))
{
}
