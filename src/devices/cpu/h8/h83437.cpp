// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
// copyright-holders:Lubomir Rintel
/***************************************************************************

    h83437.cpp

    H8-3437 family emulation

***************************************************************************/

#include "emu.h"
#include "h83437.h"

DEFINE_DEVICE_TYPE(H83434, h83434_device, "h83434", "Hitachi H8/3434")
DEFINE_DEVICE_TYPE(H83436, h83436_device, "h83436", "Hitachi H8/3436")
DEFINE_DEVICE_TYPE(H83437, h83437_device, "h83437", "Hitachi H8/3437")


h83437_device::h83437_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 start) :
	h83337_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(h83437_device::map), this), start),
	m_porta(*this, "porta"),
	m_portb(*this, "portb")
{
}

h83437_device::h83437_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h83437_device(mconfig, H83437, tag, owner, clock, 0xf780)
{
}

h83434_device::h83434_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h83437_device(mconfig, H83434, tag, owner, clock, 0xfb80)
{
}

h83436_device::h83436_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h83437_device(mconfig, H83436, tag, owner, clock, 0xf780)
{
}

void h83437_device::map(address_map &map)
{
	h83337_device::map(map);
	map(0xffaa, 0xffaa).rw(m_porta, FUNC(h8_port_device::dr_r), FUNC(h8_port_device::dr_w));
	map(0xffab, 0xffab).rw(m_porta, FUNC(h8_port_device::port_r), FUNC(h8_port_device::ddr_w));
	map(0xffbc, 0xffbc).rw(m_portb, FUNC(h8_port_device::dr_r), FUNC(h8_port_device::dr_w));
	map(0xffbd, 0xffbd).r(m_portb, FUNC(h8_port_device::port_r));
	map(0xffbe, 0xffbe).w(m_portb, FUNC(h8_port_device::ddr_w));
}

void h83437_device::device_add_mconfig(machine_config &config)
{
	h83337_device::device_add_mconfig(config);
	H8_PORT(config, m_porta, *this, h8_device::PORT_A, 0x00, 0x00);
	H8_PORT(config, m_portb, *this, h8_device::PORT_B, 0x00, 0x00);
}
