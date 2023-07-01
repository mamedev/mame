// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "oricext.h"
#include "jasmin.h"
#include "microdisc.h"

DEFINE_DEVICE_TYPE(ORICEXT_CONNECTOR, oricext_connector, "oricext_connector", "ORIC extension connector")

oricext_connector::oricext_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ORICEXT_CONNECTOR, tag, owner, clock),
	device_single_card_slot_interface<device_oricext_interface>(mconfig, *this),
	irq_handler(*this),
	reset_handler(*this)
{
}

oricext_connector::~oricext_connector()
{
}

void oricext_connector::device_start()
{
}

void oricext_connector::irq_w(int state)
{
	irq_handler(state);
}

void oricext_connector::reset_w(int state)
{
	reset_handler(state);
}

void oricext_connector::set_view(memory_view &_view)
{
	auto card = get_card_device();
	if(card)
		card->set_view(_view);
}

void oricext_connector::map_io(address_space_installer &space)
{
	auto card = get_card_device();
	if(card)
		card->map_io(space);
}

void oricext_connector::map_rom()
{
	auto card = get_card_device();
	if(card)
		card->map_rom();
}

device_oricext_interface::device_oricext_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "oricext"),
	view(nullptr),
	connector(nullptr)
{
}

void device_oricext_interface::interface_pre_start()
{
	connector = downcast<oricext_connector *>(device().owner());
}

void device_oricext_interface::irq_w(int state)
{
	connector->irq_w(state);
}

void device_oricext_interface::reset_w(int state)
{
	connector->reset_w(state);
}

void device_oricext_interface::set_view(memory_view &_view)
{
	view = &_view;
}

void oricext_intf(device_slot_interface &device)
{
	device.option_add("jasmin", ORIC_JASMIN);
	device.option_add("microdisc", ORIC_MICRODISC);
}
