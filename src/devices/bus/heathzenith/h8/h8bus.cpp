// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  h8bus.cpp - Heath/Zenith H-8 bus

  Also known as the "Benton Harbor Bus" BHBus based on city where the
  corporate headquarters were located.

***************************************************************************/

#include "emu.h"

#include "h8bus.h"

#include <cstring>

device_h8bus_card_interface::device_h8bus_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "h8bus"),
	m_h8bus(nullptr)
{
}

device_h8bus_card_interface::~device_h8bus_card_interface()
{
}

void device_h8bus_card_interface::interface_pre_start()
{
	if (!m_h8bus)
	{
		throw device_missing_dependencies();
	}
}

device_h8bus_p1_card_interface::device_h8bus_p1_card_interface(const machine_config &mconfig, device_t &device):
	device_h8bus_card_interface(mconfig, device)
{
}

device_h8bus_p1_card_interface::~device_h8bus_p1_card_interface()
{
}

device_h8bus_p2_card_interface::device_h8bus_p2_card_interface(const machine_config &mconfig, device_t &device) :
	device_h8bus_card_interface(mconfig, device)
{
}

device_h8bus_p2_card_interface::~device_h8bus_p2_card_interface()
{
}

DEFINE_DEVICE_TYPE(H8BUS_SLOT, h8bus_slot_device, "h8bus_slot", "Heath H-8 slot")

h8bus_slot_device::h8bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h8bus_slot_device(mconfig, H8BUS_SLOT, tag, owner, clock)
{
}

h8bus_slot_device::h8bus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_single_card_slot_interface(mconfig, *this),
	m_h8bus(*this, finder_base::DUMMY_TAG),
	m_h8bus_slottag(nullptr)
{
}

void h8bus_slot_device::device_start()
{
}

void h8bus_slot_device::device_resolve_objects()
{
	device_h8bus_card_interface *dev = get_card_device();

	if (dev)
	{
		dev->set_h8bus_tag(m_h8bus.target(), m_h8bus_slottag);
		m_h8bus->add_h8bus_card(*dev);
	}
}


DEFINE_DEVICE_TYPE(H8BUS, h8bus_device, "h8bus", "Heathkit H8 bus (Benton Harbor Bus)")

h8bus_device::h8bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h8bus_device(mconfig, H8BUS, tag, owner, clock)
{
}

h8bus_device::h8bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_mem_config("mem", ENDIANNESS_LITTLE, 16, 16, 0, address_map_constructor(FUNC(h8bus_device::mem_map), this)),
	m_io_config("io", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(h8bus_device::io_map), this)),
	m_p1_card(nullptr),
	m_p2_card(nullptr)
{
}

h8bus_device::~h8bus_device()
{
}

void h8bus_device::device_start()
{
}

void h8bus_device::device_reset()
{
}

void h8bus_device::mem_map(address_map &map)
{
	map.unmap_value_high();
}

void h8bus_device::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
}

device_memory_interface::space_config_vector h8bus_device::memory_space_config() const
{
	return space_config_vector { std::make_pair(AS_PROGRAM, &m_mem_config), std::make_pair(AS_IO, &m_io_config) };
}

void h8bus_device::add_h8bus_card(device_h8bus_card_interface &card)
{
	m_device_list.emplace_back(card);
	if (strcmp(card.m_h8bus_slottag, "p1") == 0)
	{
		m_p1_card = (device_h8bus_p1_card_interface*) &card;
	}
	else if (strcmp(card.m_h8bus_slottag, "p2") == 0)
	{
		m_p2_card = (device_h8bus_p2_card_interface*) &card;
	}
}

// TODO properly handle multiple cards sharing the same interrupt line.
void h8bus_device::set_int1_line(int state)
{
	for (device_h8bus_card_interface &entry : m_device_list)
	{
		entry.int1_w(state);
	}
}

void h8bus_device::set_int2_line(int state)
{
	for (device_h8bus_card_interface &entry : m_device_list)
	{
		entry.int2_w(state);
	}
}

void h8bus_device::set_int3_line(int state)
{
	for (device_h8bus_card_interface &entry : m_device_list)
	{
		entry.int3_w(state);
	}
}

void h8bus_device::set_int4_line(int state)
{
	for (device_h8bus_card_interface &entry : m_device_list)
	{
		entry.int4_w(state);
	}
}

void h8bus_device::set_int5_line(int state)
{
	for (device_h8bus_card_interface &entry : m_device_list)
	{
		entry.int5_w(state);
	}
}

void h8bus_device::set_int6_line(int state)
{
	for (device_h8bus_card_interface &entry : m_device_list)
	{
		entry.int6_w(state);
	}
}

void h8bus_device::set_int7_line(int state)
{
	for (device_h8bus_card_interface &entry : m_device_list)
	{
		entry.int7_w(state);
	}
}

void h8bus_device::set_reset_line(int state)
{
	for (device_h8bus_card_interface &entry : m_device_list)
	{
		entry.reset_w(state);
	}
}

void h8bus_device::set_hold_line(int state)
{
	for (device_h8bus_card_interface &entry : m_device_list)
	{
		entry.hold_w(state);
	}
}

void h8bus_device::set_m1_line(int state)
{
	for (device_h8bus_card_interface &entry : m_device_list)
	{
		entry.m1_w(state);
	}
}

void h8bus_device::set_p201_inte(int state)
{
	m_p1_card->p201_inte(state);
}

void h8bus_device::set_p201_reset(int state)
{
	m_p2_card->p201_reset_w(state);
}

void h8bus_device::set_p201_int1(int state)
{
	m_p2_card->p201_int1_w(state);
}

void h8bus_device::set_p201_int2(int state)
{
	m_p2_card->p201_int2_w(state);
}

void h8bus_device::set_disable_rom_line(int state)
{
	for (device_h8bus_card_interface &entry : m_device_list)
	{
		entry.rom_disable_w(state);
	}
}
