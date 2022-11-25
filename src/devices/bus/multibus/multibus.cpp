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
	, m_mem_config("mem", ENDIANNESS_LITTLE, 16, 24)
	, m_pio_config("pio", ENDIANNESS_LITTLE, 16, 16)
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
	m_int_cb.resolve_all_safe();
	m_xack_cb.resolve_safe();
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

void multibus_slot_device::device_resolve_objects()
{
	device_multibus_interface *const card(dynamic_cast<device_multibus_interface *>(get_card_device()));

	if (card)
		card->set_bus_device(*m_bus);
}

device_multibus_interface::device_multibus_interface(machine_config const &mconfig, device_t &device)
	: device_interface(device, "multibus")
	, m_bus(nullptr)
{
}

void device_multibus_interface::set_bus_device(multibus_device &bus_device)
{
	m_bus = &bus_device;
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
