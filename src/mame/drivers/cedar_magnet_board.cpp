// license:BSD-3-Clause
// copyright-holders:David Haywood


#include "emu.h"
#include "cedar_magnet_plane.h"


//const device_type CEDAR_MAGNET_BASE = &device_creator<cedar_magnet_board_device>;

cedar_magnet_board_device::cedar_magnet_board_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source)
//  m_ram(*this, "ram")
{
}

void cedar_magnet_board_device::irq(device_t &device)
{
	m_cpu->set_input_line(0, HOLD_LINE);
}

void cedar_magnet_board_device::device_start()
{
}

void cedar_magnet_board_device::write_cpu_bus(int offset, uint8_t data)
{
	device_t* cpu = m_cpu;
	address_space& ap = cpu->memory().space(AS_PROGRAM);
	ap.write_byte(offset, data);
}

uint8_t cedar_magnet_board_device::read_cpu_bus(int offset)
{
	device_t* cpu = m_cpu;
	address_space& ap = cpu->memory().space(AS_PROGRAM);
	return ap.read_byte(offset);
}

bool cedar_magnet_board_device::is_running(void)
{
	return m_is_running;
}

void cedar_magnet_board_device::reset_assert_callback(void *ptr, int32_t param)
{
	m_cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

void cedar_magnet_board_device::reset_clear_callback(void *ptr, int32_t param)
{
	m_cpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
}

void cedar_magnet_board_device::halt_assert_callback(void *ptr, int32_t param)
{
	m_cpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_is_running = false;
}

void cedar_magnet_board_device::halt_clear_callback(void *ptr, int32_t param)
{
	m_cpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	m_is_running = true;
}

void cedar_magnet_board_device::halt_assert(void)
{
	machine().scheduler().timer_set(attotime::from_usec(2), timer_expired_delegate(FUNC(cedar_magnet_board_device::halt_assert_callback),this));
}

void cedar_magnet_board_device::halt_clear(void)
{
	machine().scheduler().timer_set(attotime::from_usec(2), timer_expired_delegate(FUNC(cedar_magnet_board_device::halt_clear_callback),this));
}

void cedar_magnet_board_device::reset_assert(void)
{
	machine().scheduler().timer_set(attotime::from_usec(1), timer_expired_delegate(FUNC(cedar_magnet_board_device::reset_assert_callback),this));
}

void cedar_magnet_board_device::reset_clear(void)
{
	machine().scheduler().timer_set(attotime::from_usec(1), timer_expired_delegate(FUNC(cedar_magnet_board_device::reset_clear_callback),this));
}

void cedar_magnet_board_device::device_reset()
{
	halt_assert();
}
