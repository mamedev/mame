// license:BSD-3-Clause
// copyright-holders:R. Belmont

/*
 * An emulation of the Digital Equipment Corporation DC7085 (also called "DZ") quad-UART
 *
 * Used in:
 *
 *   Several models of MIPS DECstation
 *   Some VAXstations
 *
 * Sources:
 *
 *   http://www.vanade.com/~blc/DS3100/pmax/DS3100.func.spec.pdf
 *
 */

#include "emu.h"
#include "dc7085.h"

#define LOG_GENERAL (1U << 0)
#define LOG_REG     (1U << 1)

//#define VERBOSE (LOG_GENERAL|LOG_REG)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(DC7085, dc7085_device, "dc7085", "Digital Equipment Corporation DC7085 Quad UART")

dc7085_device::dc7085_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DC7085, tag, owner, clock),
	m_int_cb(*this)
{
}

void dc7085_device::map(address_map &map)
{
	map(0x00, 0x01).rw(FUNC(dc7085_device::status_r), FUNC(dc7085_device::control_w));
	map(0x04, 0x05).rw(FUNC(dc7085_device::rxbuffer_r), FUNC(dc7085_device::lineparams_w));
	map(0x10, 0x11).rw(FUNC(dc7085_device::txparams_r), FUNC(dc7085_device::txparams_w));
	map(0x14, 0x15).rw(FUNC(dc7085_device::modem_status_r), FUNC(dc7085_device::txdata_w));
}

void dc7085_device::device_start()
{
	m_int_cb.resolve_safe();
}

void dc7085_device::device_reset()
{
}

u16 dc7085_device::status_r()
{
	return 0x8000;
}

u16 dc7085_device::rxbuffer_r()
{
	return 0;
}

u16 dc7085_device::txparams_r()
{
	return 0;
}

u16 dc7085_device::modem_status_r()
{
	return 0;
}

void dc7085_device::control_w(u16 data)
{
}

void dc7085_device::lineparams_w(u16 data)
{
}

void dc7085_device::txparams_w(u16 data)
{
}

void dc7085_device::txdata_w(u16 data)
{
}
