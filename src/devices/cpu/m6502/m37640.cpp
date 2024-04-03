// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

//  Mitsubishi M37640 8-bit microcontroller with usb support

#include "emu.h"
#include "m37640.h"

DEFINE_DEVICE_TYPE(M37640, m37640_device, "m37640", "Mitsubishi M37640")

m37640_device::m37640_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, u32 mode) :
	m740_device(mconfig, M37640, tag, owner, clock),
	m_mode(mode)
{
	program_config.m_internal_map = address_map_constructor(FUNC(m37640_device::map), this);
}

void m37640_device::device_start()
{
	m740_device::device_start();
}

void m37640_device::device_reset()
{
	m740_device::device_start();
}

void m37640_device::map(address_map &map)
{
	(void)m_mode;
}
