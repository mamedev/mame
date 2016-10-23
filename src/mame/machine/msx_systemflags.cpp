// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "msx_systemflags.h"


const device_type MSX_SYSTEMFLAGS = &device_creator<msx_systemflags_device>;


msx_systemflags_device::msx_systemflags_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_SYSTEMFLAGS, "MSX System Flags", tag, owner, clock, "msx_systemflags", __FILE__)
	, m_initial_value(0xff)
	, m_system_flags(0xff)
{
}


void msx_systemflags_device::device_start()
{
	m_system_flags = m_initial_value;

	save_item(NAME(m_system_flags));

	// Install IO read/write handlers
	address_space &space = machine().device<cpu_device>("maincpu")->space(AS_IO);
	space.install_write_handler(0xf4, 0xf4, write8_delegate(FUNC(msx_systemflags_device::write), this));
	space.install_read_handler(0xf4, 0xf4, read8_delegate(FUNC(msx_systemflags_device::read), this));
}


uint8_t msx_systemflags_device::read(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return m_system_flags;
}


void msx_systemflags_device::write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_system_flags = data;
}
