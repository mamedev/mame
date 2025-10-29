// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

FDD 2D 320KB C-Bus bridge interface

For original 1st gen PC-98 only (VM already won't check the range), connects to the back thru
the 5" FLOPPY DISK port (not the 2DD option)

**************************************************************************************************/

#include "emu.h"
#include "fdd_2d.h"

DEFINE_DEVICE_TYPE(FDD_2D_BRIDGE, fdd_2d_bridge_device, "fdd_2d", "NEC FDD 320KB 2D bridge interface")

fdd_2d_bridge_device::fdd_2d_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, FDD_2D_BRIDGE, tag, owner, clock)
	, m_bus(*this, DEVICE_SELF_OWNER)
	, m_fdd_if(*this, "fdd_if")
{
}

void fdd_2d_bridge_device::device_add_mconfig(machine_config &config)
{
	PC80S31K(config, m_fdd_if, XTAL(31'948'800) / 8);
}

void fdd_2d_bridge_device::device_start()
{
}

void fdd_2d_bridge_device::device_reset()
{
	m_bus->install_device(0x0050, 0x0057, *this, &fdd_2d_bridge_device::io_map);
}

void fdd_2d_bridge_device::io_map(address_map &map)
{
	map(0, 7).m(m_fdd_if, FUNC(pc80s31k_device::host_map)).umask16(0xff00);
}
