// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "msx_switched.h"


msx_switched_device::msx_switched_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source)
{
}


void msx_switched_device::device_start()
{
	address_space &space = dynamic_cast<device_memory_interface*>(owner())->space();

	// Install IO read/write handlers
	uint16_t start = ( get_id() << 8 ) | 0x00;
	uint16_t end = ( get_id() << 8 ) | 0x0f;
	space.install_read_handler(start, end, read8_delegate(FUNC(msx_switched_device::io_read), this));
	space.install_write_handler(start, end, write8_delegate(FUNC(msx_switched_device::io_write), this));
}
