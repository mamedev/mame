// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#include "emu.h"
#include "vsmile_ctrl.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(VSMILE_CTRL_PORT, vsmile_ctrl_port_device, "vsmile_ctrl_port", "V.Smile Controller Port")


//**************************************************************************
//    V.Smile controller interface
//**************************************************************************

device_vsmile_ctrl_interface::device_vsmile_ctrl_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
	, m_port(dynamic_cast<vsmile_ctrl_port_device *>(device.owner()))
{
}

device_vsmile_ctrl_interface::~device_vsmile_ctrl_interface()
{
}

void device_vsmile_ctrl_interface::interface_validity_check(validity_checker &valid) const
{
	device_slot_card_interface::interface_validity_check(valid);

	if (device().owner() && !m_port)
	{
		osd_printf_error(
				"Owner device %s (%s) is not a vsmile_ctrl_port_device\n",
				device().owner()->tag(),
				device().owner()->name());
	}
}

void device_vsmile_ctrl_interface::interface_pre_start()
{
	device_slot_card_interface::interface_pre_start();

	if (m_port && !m_port->started())
		throw device_missing_dependencies();
}


//**************************************************************************
//    V.Smile controller port
//**************************************************************************

vsmile_ctrl_port_device::vsmile_ctrl_port_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: device_t(mconfig, VSMILE_CTRL_PORT, tag, owner, clock)
	, device_slot_interface(mconfig, *this)
	, m_rts_cb(*this)
	, m_data_cb(*this)
{
}

vsmile_ctrl_port_device::~vsmile_ctrl_port_device()
{
}

void vsmile_ctrl_port_device::device_validity_check(validity_checker &valid) const
{
	device_t *const card(get_card_device());
	if (card && !dynamic_cast<device_vsmile_ctrl_interface *>(card))
	{
		osd_printf_error(
				"Card device %s (%s) does not implement device_vsmile_ctrl_interface\n",
				card->tag(),
				card->name());
	}
}

void vsmile_ctrl_port_device::device_resolve_objects()
{
	device_vsmile_ctrl_interface *const card(dynamic_cast<device_vsmile_ctrl_interface *>(get_card_device()));
	if (card)
		m_device = card;

	m_rts_cb.resolve_safe();
	m_data_cb.resolve_safe();
}

void vsmile_ctrl_port_device::device_start()
{
	device_t *const card(get_card_device());
	if (card && !m_device)
	{
		throw emu_fatalerror(
				"vsmile_ctrl_port_device: card device %s (%s) does not implement device_vsmile_ctrl_interface\n",
				card->tag(),
				card->name());
	}
}


#include "pad.h"

void vsmile_controllers(device_slot_interface &device)
{
	device.option_add("pad", VSMILE_PAD);
}
