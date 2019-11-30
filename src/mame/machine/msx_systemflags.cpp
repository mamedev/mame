// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "msx_systemflags.h"


DEFINE_DEVICE_TYPE(MSX_SYSTEMFLAGS, msx_systemflags_device, "msx_systemflags", "MSX System Flags")


msx_systemflags_device::msx_systemflags_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_SYSTEMFLAGS, tag, owner, clock)
	, m_initial_value(0xff)
	, m_system_flags(0xff)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
{
}


void msx_systemflags_device::device_start()
{
	m_system_flags = m_initial_value;

	save_item(NAME(m_system_flags));

	// Install IO read/write handlers
	address_space &space = m_maincpu->space(AS_IO);
	space.install_write_handler(0xf4, 0xf4, write8_delegate(*this, FUNC(msx_systemflags_device::write)));
	space.install_read_handler(0xf4, 0xf4, read8_delegate(*this, FUNC(msx_systemflags_device::read)));
}


READ8_MEMBER(msx_systemflags_device::read)
{
	return m_system_flags;
}


WRITE8_MEMBER(msx_systemflags_device::write)
{
	m_system_flags = data;
}
