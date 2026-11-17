// license:BSD-3-Clause
// copyright-holders:Carl
/*************************************************************************
 *
 *      pc_joy.h
 *
 *      joystick port
 *
 *************************************************************************/

#include "emu.h"
#include "pc_joy.h"

#include "options.h"

DEFINE_DEVICE_TYPE(PC_JOY, pc_joy_device, "pc_joy", "PC joystick port")

pc_joy_device::pc_joy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PC_JOY, tag, owner, clock),
	device_single_card_slot_interface<device_pc_joy_interface>(mconfig, *this),
	m_dev(nullptr)
{
	set_options(pc_joysticks, "basic_joy", false);
}

uint8_t pc_joy_device::joy_port_r()
{
	int delta = ((machine().time() - m_stime) * 256 * 2325).seconds();

	if(!m_dev)
		return 0xf0;

	return ((m_dev->btn() & 0xf) << 4) | (m_dev->y2(delta) << 3) | (m_dev->x2(delta) << 2) | (m_dev->y1(delta) << 1) | m_dev->x1(delta);
}

void pc_joy_device::joy_port_w(uint8_t data)
{
	m_stime = machine().time();
	if(!m_dev)
		return;
	m_dev->port_write();
}

void pc_joy_device::device_config_complete()
{
	m_dev = get_card_device();
}

device_pc_joy_interface::device_pc_joy_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "pcjoy")
{
}

device_pc_joy_interface::~device_pc_joy_interface()
{
}
