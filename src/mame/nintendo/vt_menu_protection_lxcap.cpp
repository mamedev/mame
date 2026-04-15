// license:BSD-3-Clause
// copyright-holders:David Haywood

// some kind of 2-wire protocol (but not SPI?)
// might be doing a calculation rather than returning data from a table

#include "emu.h"

#include "vt_menu_protection_lxcap.h"

DEFINE_DEVICE_TYPE(VT_MENU_PROTECTION_LXCAP, vt_menu_protection_lxcap_device, "vtmenuprot", "VT Menu Protection (lxcap)")

vt_menu_protection_lxcap_device::vt_menu_protection_lxcap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VT_MENU_PROTECTION_LXCAP, tag, owner, clock),
	m_extrarom(*this, DEVICE_SELF)
{
}

uint8_t vt_menu_protection_lxcap_device::read()
{
	return 0x00;
}

void vt_menu_protection_lxcap_device::write_clock(bool state)
{
	m_clock = state;
}

void vt_menu_protection_lxcap_device::write_data(bool state)
{
	m_data = state;
}

void vt_menu_protection_lxcap_device::device_start()
{
	save_item(NAME(m_data));
	save_item(NAME(m_clock));
}

void vt_menu_protection_lxcap_device::device_reset()
{
	m_data = false;
	m_clock = false;
}
