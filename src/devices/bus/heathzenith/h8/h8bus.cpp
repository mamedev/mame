// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  h8bus.cpp - Heath/Zenith H-8 bus

  Also known as the "Benton Harbor Bus" BHBus based on city where the
  corporate headquarters were located.

***************************************************************************/

#include "emu.h"

#include "h8bus.h"

#define LOG_INIT (1U << 1)    // Shows register setup

//#define VERBOSE (LOG_INIT)

#include "logmacro.h"

#define LOGINIT(...)        LOGMASKED(LOG_INIT, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif


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

device_p201_p1_card_interface::device_p201_p1_card_interface(device_t &device, const char *tag)
	: device_interface(device, tag)
	, m_p201_reset(*this)
	, m_p201_int1(*this)
	, m_p201_int2(*this)
{
}

device_p201_p1_card_interface::~device_p201_p1_card_interface()
{
}

device_p201_p2_card_interface::device_p201_p2_card_interface(device_t &device, const char *tag)
	: device_interface(device, tag)
	, m_p201_inte(*this)
{
}

device_p201_p2_card_interface::~device_p201_p2_card_interface()
{
}

DEFINE_DEVICE_TYPE(H8BUS_SLOT, h8bus_slot_device, "h8bus_slot", "Heath H-8 slot")

h8bus_slot_device::h8bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: h8bus_slot_device(mconfig, H8BUS_SLOT, tag, owner, clock)
{
}

h8bus_slot_device::h8bus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_single_card_slot_interface(mconfig, *this)
	, m_h8bus(*this, finder_base::DUMMY_TAG)
	, m_h8bus_slottag(nullptr)
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
		unsigned index = m_h8bus->add_h8bus_card(*dev);
		dev->set_index(index);
	}
}


DEFINE_DEVICE_TYPE(H8BUS, h8bus_device, "h8bus", "Heathkit H8 bus (Benton Harbor Bus)")

h8bus_device::h8bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: h8bus_device(mconfig, H8BUS, tag, owner, clock)
{
}

h8bus_device::h8bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_mem_config("mem", ENDIANNESS_LITTLE, 8, 16, 0, address_map_constructor(FUNC(h8bus_device::mem_map), this))
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(h8bus_device::io_map), this))
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
	m_int1_slot_states = 0;
	m_int2_slot_states = 0;
	m_int3_slot_states = 0;
	m_int4_slot_states = 0;
	m_int5_slot_states = 0;
	m_int6_slot_states = 0;
	m_int7_slot_states = 0;
	m_reset_slot_states = 0;
	m_hold_slot_states = 0;
	m_m1_slot_states = 0;
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

u8 h8bus_device::add_h8bus_card(device_h8bus_card_interface &card)
{
	m_device_list.emplace_back(card);
	int index = m_device_list.size() - 1;

	if (index > 31)
	{
		// ensure index is in range.
		index = 31;
	}

	LOGINIT("%s: added card to index: %d, tag: %s\n", FUNCNAME, index, card.m_h8bus_slottag);

	return index;
}

bool h8bus_device::update_line_states(u32 &states, unsigned index, int state)
{
	bool const changed(bool(state) != bool(BIT(states, index)));

	if (changed)
	{
		int old_state = states ? 1 : 0;

		if (state)
		{
			states |= (1 << index);
		}
		else
		{
			states &= ~(1 << index);
		}

		int new_state = states ? 1 : 0;

		return (old_state != new_state);
	}

	return false;
}

void h8bus_device::set_int1_line(unsigned index, int state)
{
	bool const changed = update_line_states(m_int1_slot_states, index, state);

	if (changed)
	{
		for (device_h8bus_card_interface &entry : m_device_list)
		{
			entry.int1_w(m_int1_slot_states ? 1 : 0);
		}
	}
}

void h8bus_device::set_int2_line(unsigned index, int state)
{
	bool const changed = update_line_states(m_int2_slot_states, index, state);

	if (changed)
	{
		for (device_h8bus_card_interface &entry : m_device_list)
		{
			entry.int2_w(m_int2_slot_states ? 1 : 0);
		}
	}
}

void h8bus_device::set_int3_line(unsigned index, int state)
{
	bool const changed = update_line_states(m_int3_slot_states, index, state);

	if (changed)
	{
		for (device_h8bus_card_interface &entry : m_device_list)
		{
			entry.int3_w(m_int3_slot_states ? 1 : 0);
		}
	}
}

void h8bus_device::set_int4_line(unsigned index, int state)
{
	bool const changed = update_line_states(m_int4_slot_states, index, state);

	if (changed)
	{
		for (device_h8bus_card_interface &entry : m_device_list)
		{
			entry.int4_w(m_int4_slot_states ? 1 : 0);
		}
	}
}

void h8bus_device::set_int5_line(unsigned index, int state)
{
	bool const changed = update_line_states(m_int5_slot_states, index, state);

	if (changed)
	{
		for (device_h8bus_card_interface &entry : m_device_list)
		{
			entry.int5_w(m_int5_slot_states ? 1 : 0);
		}
	}
}

void h8bus_device::set_int6_line(unsigned index, int state)
{
	bool const changed = update_line_states(m_int6_slot_states, index, state);

	if (changed)
	{
		for (device_h8bus_card_interface &entry : m_device_list)
		{
			entry.int6_w(m_int6_slot_states ? 1 : 0);
		}
	}
}

void h8bus_device::set_int7_line(unsigned index, int state)
{
	bool const changed = update_line_states(m_int7_slot_states, index, state);

	if (changed)
	{
		for (device_h8bus_card_interface &entry : m_device_list)
		{
			entry.int7_w(m_int7_slot_states ? 1 : 0);
		}
	}
}

void h8bus_device::set_reset_line(unsigned index, int state)
{
	bool const changed = update_line_states(m_reset_slot_states, index, state);

	if (changed)
	{
		for (device_h8bus_card_interface &entry : m_device_list)
		{
			entry.reset_w(m_reset_slot_states ? 1 : 0);
		}
	}
}

void h8bus_device::set_hold_line(unsigned index, int state)
{
	bool const changed = update_line_states(m_hold_slot_states, index, state);

	if (changed)
	{
		for (device_h8bus_card_interface &entry : m_device_list)
		{
			entry.hold_w(m_hold_slot_states ? 1 : 0);
		}
	}
}

void h8bus_device::set_m1_line(unsigned index, int state)
{
	bool const changed = update_line_states(m_m1_slot_states, index, state);

	if (changed)
	{
		for (device_h8bus_card_interface &entry : m_device_list)
		{
			entry.m1_w(m_m1_slot_states ? 1 : 0);
		}
	}
}

void h8bus_device::set_disable_rom_line(unsigned index, int state)
{
	bool const changed = update_line_states(m_disable_rom_slot_states, index, state);

	if (changed)
	{
		for (device_h8bus_card_interface &entry : m_device_list)
		{
			entry.rom_disable_w(m_disable_rom_slot_states ? 1 : 0);
		}
	}
}
