// license:BSD-3-Clause
// copyright-holders:Vas Crabb

#include "emu.h"
#include "intellec4.h"

#include <algorithm>


DEFINE_DEVICE_TYPE_NS(INTELLEC4_UNIV_SLOT, bus::intellec4, univ_slot_device, "intlc4univslot", "INTELLEC 4/MOD 40 Universal Slot")
DEFINE_DEVICE_TYPE_NS(INTELLEC4_UNIV_BUS,  bus::intellec4, univ_bus_device,  "intlc4univbus",  "INTELLEC 4/MOD 40 Universal Bus")


namespace bus { namespace intellec4 {

univ_slot_device::univ_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, INTELLEC4_UNIV_SLOT, tag, owner, clock)
	, device_slot_interface(mconfig, *this)
{
}


void univ_slot_device::set_bus_tag(device_t &device, char const *tag)
{
}


void univ_slot_device::device_start()
{
}



univ_bus_device::univ_bus_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, INTELLEC4_UNIV_BUS, tag, owner, clock)
	, m_stop_out_cb(*this)
	, m_test_out_cb(*this)
	, m_reset_4002_out_cb(*this)
	, m_user_reset_out_cb(*this)
	, m_stop(0U)
	, m_test(0U)
	, m_reset_4002(0U)
	, m_user_reset(0U)
{
	std::fill(std::begin(m_cards), std::end(m_cards), nullptr);
}


WRITE_LINE_MEMBER(univ_bus_device::sync_in)
{
	// FIXME: distribute to cards
}

WRITE_LINE_MEMBER(univ_bus_device::test_in)
{
	if (state)
		m_test &= ~u16(1U);
	else
		m_test |= 1U;
	// FIXME: distribute to cards
}

WRITE_LINE_MEMBER(univ_bus_device::stop_in)
{
	if (state)
		m_stop &= ~u16(1U);
	else
		m_stop |= 1U;
	// FIXME: distribute to cards
}

WRITE_LINE_MEMBER(univ_bus_device::stop_acknowledge_in)
{
	// FIXME: distribute to cards
}

WRITE_LINE_MEMBER(univ_bus_device::cpu_reset_in)
{
	// FIXME: distribute to cards
}

WRITE_LINE_MEMBER(univ_bus_device::reset_4002_in)
{
	if (state)
		m_reset_4002 &= ~u16(1U);
	else
		m_reset_4002 |= 1U;
	// FIXME: distribute to cards
}


void univ_bus_device::device_start()
{
	m_stop_out_cb.resolve_safe();
	m_test_out_cb.resolve_safe();
	m_reset_4002_out_cb.resolve_safe();
	m_user_reset_out_cb.resolve_safe();
}

} } // namespace bus::intellec4



SLOT_INTERFACE_START(intellec4_univ_cards)
SLOT_INTERFACE_END
