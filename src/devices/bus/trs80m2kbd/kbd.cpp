// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy Model II/12/16/6000 keyboard connector emulation

**********************************************************************/

#include "emu.h"
#include "kbd.h"

#include "tandy6k_kbd.h"
#include "trs80m2_kbd.h"


DEFINE_DEVICE_TYPE(TRS80M2_KEYBOARD_PORT, trs80m2_keyboard_port_device, "trs80m2_kbd_port", "Tandy Model II keyboard port")


trs80m2_keyboard_port_device::trs80m2_keyboard_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TRS80M2_KEYBOARD_PORT, tag, owner, clock),
	device_single_card_slot_interface<device_trs80m2_keyboard_interface>(mconfig, *this),
	m_write_clock(*this),
	m_keyboard(nullptr)
{
}

void trs80m2_keyboard_port_device::device_start()
{
	m_keyboard = get_card_device();
}

int trs80m2_keyboard_port_device::data_r()
{
	return m_keyboard ? m_keyboard->data_r() : 0;
}

void trs80m2_keyboard_port_device::busy_w(int state)
{
	if (m_keyboard)
		m_keyboard->busy_w(state);
}


device_trs80m2_keyboard_interface::device_trs80m2_keyboard_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "trs80m2kbd"),
	m_port(dynamic_cast<trs80m2_keyboard_port_device *>(device.owner()))
{
}


void trs80m2_keyboards(device_slot_interface &device)
{
	device.option_add("trs80m2_kbd", TRS80M2_KEYBOARD);
	device.option_add("tandy6k_kbd", TANDY6K_KEYBOARD);
}
