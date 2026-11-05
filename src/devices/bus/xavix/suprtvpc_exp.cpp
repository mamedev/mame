// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Super TV-PC rear expansion-port interface. Connects the console's data,
    clock and ready lines to an optional peripheral, with the dedicated thermal
    printer provided as a slot option.

    TODO:
    - Establish the physical connector pin numbering.

***************************************************************************/

#include "emu.h"
#include "suprtvpc_exp.h"

#include "suprtvpc_printer.h"

DEFINE_DEVICE_TYPE(SUPRTVPC_EXP_PORT, suprtvpc_exp_port_device, "suprtvpc_exp", "Super TV-PC Expansion Port")

suprtvpc_exp_port_device::suprtvpc_exp_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SUPRTVPC_EXP_PORT, tag, owner, clock)
	, device_single_card_slot_interface<device_suprtvpc_exp_interface>(mconfig, *this)
{
	set_options([] (device_slot_interface &slot) { slot.option_add("printer", SUPRTVPC_PRINTER); }, nullptr, false);
}

device_suprtvpc_exp_interface::device_suprtvpc_exp_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "suprtvpcexp")
{
}

void suprtvpc_exp_port_device::device_start()
{
	m_peripheral = get_card_device();
}

void device_suprtvpc_exp_interface::data_w(int state)
{
}

void device_suprtvpc_exp_interface::clock_w(int state)
{
}

int device_suprtvpc_exp_interface::ready_r()
{
	return 0;
}

void suprtvpc_exp_port_device::data_w(int state)
{
	if (m_peripheral)
		m_peripheral->data_w(state);
}

void suprtvpc_exp_port_device::clock_w(int state)
{
	if (m_peripheral)
		m_peripheral->clock_w(state);
}

int suprtvpc_exp_port_device::ready_r()
{
	return m_peripheral ? m_peripheral->ready_r() : 0;
}
