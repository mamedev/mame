// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    multibus.cpp

    Intel Multibus

*********************************************************************/

#include "emu.h"
#include "multibus.h"
#include "isbc202.h"

// device type definition
DEFINE_DEVICE_TYPE(MULTIBUS_SLOT, multibus_slot_device, "multibus_slot", "Intel Multibus slot")

multibus_slot_device::multibus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig , MULTIBUS_SLOT , tag , owner , clock)
	, device_single_card_slot_interface<device_multibus_interface>(mconfig , *this)
{
	option_reset();
	option_add("isbc202" , ISBC202);
	set_default_option(nullptr);
	set_fixed(false);
}

multibus_slot_device::~multibus_slot_device()
{
}

void multibus_slot_device::install_io_rw(address_space& space)
{
	device_multibus_interface *card = get_card_device();
	if (card) {
		card->install_io_rw(space);
	}
}

void multibus_slot_device::install_mem_rw(address_space& space)
{
	device_multibus_interface *card = get_card_device();
	if (card) {
		card->install_mem_rw(space);
	}
}

void multibus_slot_device::device_start()
{
}

device_multibus_interface::device_multibus_interface(const machine_config &mconfig , device_t &device)
	: device_interface(device , "multibus")
{
}

device_multibus_interface::~device_multibus_interface()
{
}
