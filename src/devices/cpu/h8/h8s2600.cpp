// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "h8s2600.h"
#include "h8s2600d.h"

h8s2600_device::h8s2600_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor map_delegate) :
	h8s2000_device(mconfig, type, tag, owner, clock, map_delegate),
	m_mac_saturating(false)
{
	m_has_mac = true;
}

std::unique_ptr<util::disasm_interface> h8s2600_device::create_disassembler()
{
	return std::make_unique<h8s2600_disassembler>();
}

void h8s2600_device::device_start()
{
	h8s2000_device::device_start();
	save_item(NAME(m_mac_saturating));
}

void h8s2600_device::device_reset()
{
	h8s2000_device::device_reset();
	m_mac_saturating = false;
}

#include "cpu/h8/h8s2600.hxx"
