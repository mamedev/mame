// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    multibus.cpp

    Intel Multibus

*********************************************************************/

/*
 * TODO:
 *  - shared interrupts
 *  - vectored and non-vectored irq acknowledge
 *  - inhibit lines
 *  - lock/unlock
 *  - bus arbitration/priority
 */

#include "emu.h"
#include "multibus.h"

// device type definition
DEFINE_DEVICE_TYPE(MULTIBUS, multibus_device, "multibus", "Intel Multibus")
DEFINE_DEVICE_TYPE(MULTIBUS_SLOT, multibus_slot_device, "multibus_slot", "Intel Multibus slot")

multibus_device::multibus_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MULTIBUS, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_mem_config("mem", ENDIANNESS_LITTLE, 16, 24, 0, address_map_constructor(FUNC(multibus_device::mem_map), this))
	, m_pio_config("pio", ENDIANNESS_LITTLE, 16, 16, 0, address_map_constructor(FUNC(multibus_device::pio_map), this))
	, m_int_cb(*this)
	, m_xack_cb(*this)
{
}

device_memory_interface::space_config_vector multibus_device::memory_space_config() const
{
	return space_config_vector { std::make_pair(AS_PROGRAM, &m_mem_config), std::make_pair(AS_IO, &m_pio_config) };
}

void multibus_device::device_start()
{
}

void multibus_device::mem_map(address_map &map)
{
	map(0x00'0000, 0xff'ffff).unmaprw().flags(FLAG_UNMAPPED);
}

void multibus_device::pio_map(address_map &map)
{
	map(0x0000, 0xffff).unmaprw().flags(FLAG_UNMAPPED);
}

multibus_slot_device::multibus_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MULTIBUS_SLOT, tag, owner, clock)
	, device_slot_interface(mconfig, *this)
	, m_bus(*this, finder_base::DUMMY_TAG)
{
}

void multibus_slot_device::device_start()
{
}

device_multibus_interface::device_multibus_interface(machine_config const &mconfig, device_t &device)
	: device_interface(device, "multibus")
	, m_bus(*this, finder_base::DUMMY_TAG)
	, m_int(*this)
{
}

void device_multibus_interface::interface_config_complete()
{
	// FIXME: avoid listxml crash caused by owner device not being a slot
	if (device().owner() && device().owner()->owner())
	{
		m_bus.set_tag(downcast<multibus_slot_device &>(*device().owner()).bus());
		multibus_device &bus(*m_bus.lookup());

		// route incoming interrupt requests to all cards
		bus.int_callback<0>().append([this](int state) { m_int[0](state); });
		bus.int_callback<1>().append([this](int state) { m_int[1](state); });
		bus.int_callback<2>().append([this](int state) { m_int[2](state); });
		bus.int_callback<3>().append([this](int state) { m_int[3](state); });
		bus.int_callback<4>().append([this](int state) { m_int[4](state); });
		bus.int_callback<5>().append([this](int state) { m_int[5](state); });
		bus.int_callback<6>().append([this](int state) { m_int[6](state); });
		bus.int_callback<7>().append([this](int state) { m_int[7](state); });
	}
}

void device_multibus_interface::int_w(unsigned number, int state)
{
	switch (number)
	{
	case 0: int_w<0>(state); break;
	case 1: int_w<1>(state); break;
	case 2: int_w<2>(state); break;
	case 3: int_w<3>(state); break;
	case 4: int_w<4>(state); break;
	case 5: int_w<5>(state); break;
	case 6: int_w<6>(state); break;
	case 7: int_w<7>(state); break;
	}
}

void device_multibus_interface::unmap(int spacenum, offs_t addrstart, offs_t addrend, offs_t addrmirror)
{
	m_bus->space(spacenum).unmap_readwrite(addrstart, addrend, addrmirror, multibus_device::FLAG_UNMAPPED);
}
