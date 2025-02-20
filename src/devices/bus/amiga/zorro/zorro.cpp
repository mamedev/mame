// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Amiga Zorro-II Slot

***************************************************************************/

#include "emu.h"
#include "zorro.h"


//**************************************************************************
//  BUS DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(ZORRO2_BUS, zorro2_bus_device, "zorro2", "Zorro-II Bus")

zorro2_bus_device::zorro2_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ZORRO2_BUS, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_zorro_space_config("zorro", ENDIANNESS_BIG, 16, 24, 0, address_map_constructor()),
	m_eint1_handler(*this),
	m_eint4_handler(*this),
	m_eint5_handler(*this),
	m_eint7_handler(*this),
	m_int2_handler(*this),
	m_int6_handler(*this),
	m_ovr_handler(*this),
	m_autoconfig_device(0)
{
}

void zorro2_bus_device::device_start()
{
	// register for save states
	save_item(NAME(m_autoconfig_device));
}

device_memory_interface::space_config_vector zorro2_bus_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_zorro_space_config)
	};
}

void zorro2_bus_device::add_card(device_zorro2_card_interface &card)
{
	card.set_bus(*this);
	m_cards.emplace_back(card);
}

// from slot device
void zorro2_bus_device::cfgout_w(int state)
{
	// if there is still a device in the chain, tell it to configure itself
	if (m_cards.size() > ++m_autoconfig_device)
		m_cards[m_autoconfig_device].get().cfgin_w(0);
}

void zorro2_bus_device::eint1_w(int state) { m_eint1_handler(state); }
void zorro2_bus_device::eint4_w(int state) { m_eint4_handler(state); }
void zorro2_bus_device::eint5_w(int state) { m_eint5_handler(state); }
void zorro2_bus_device::eint7_w(int state) { m_eint7_handler(state); }
void zorro2_bus_device::int2_w(int state) { m_int2_handler(state); }
void zorro2_bus_device::int6_w(int state) { m_int6_handler(state); }
void zorro2_bus_device::ovr_w(int state) { m_ovr_handler(state); }

// from host
uint16_t zorro2_bus_device::mem_r(offs_t offset, uint16_t mem_mask)
{
	return space().read_word(0x200000 + (offset << 1), mem_mask);
}

void zorro2_bus_device::mem_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	space().write_word(0x200000 + (offset << 1), data, mem_mask);
}

uint16_t zorro2_bus_device::io_r(offs_t offset, uint16_t mem_mask)
{
	return space().read_word(0xe80000 + (offset << 1), mem_mask);
}

void zorro2_bus_device::io_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	space().write_word(0xe80000 + (offset << 1), data, mem_mask);
}

void zorro2_bus_device::fc_w(int code)
{
	for (device_zorro2_card_interface &entry : m_cards)
		entry.fc_w(code);
}

void zorro2_bus_device::busrst_w(int state)
{
	for (device_zorro2_card_interface &card : m_cards)
		card.busrst_w(state);

	if (state == 0)
	{
		// initiate autoconfig
		m_autoconfig_device = 0;

		// if we have a device, start the autoconfig chain
		if (m_cards.size() > m_autoconfig_device)
			m_cards[m_autoconfig_device].get().cfgin_w(0);
	}
}


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(ZORRO2_SLOT, zorro2_slot_device, "zorro2_slot", "Zorro-II slot")

zorro2_slot_device::zorro2_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	zorro2_slot_device(mconfig, ZORRO2_SLOT, tag, owner, clock)
{
}

zorro2_slot_device::zorro2_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_single_card_slot_interface(mconfig, *this)
{
}

void zorro2_slot_device::device_resolve_objects()
{
	device_zorro2_card_interface *dev = get_card_device();

	if (dev)
	{
		zorro2_bus_device *bus = downcast<zorro2_bus_device *>(m_owner);
		bus->add_card(*dev);
	}
}

void zorro2_slot_device::device_start()
{
}


//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

device_zorro2_card_interface::device_zorro2_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "zorro2"),
	m_zorro(nullptr)
{
}

device_zorro2_card_interface::~device_zorro2_card_interface()
{
}

void device_zorro2_card_interface::set_bus(zorro2_bus_device &device)
{
	assert(!m_zorro);
	m_zorro = &device;
}

void device_zorro2_card_interface::fc_w(int code)
{
}

void device_zorro2_card_interface::cfgin_w(int state)
{
}

void device_zorro2_card_interface::busrst_w(int state)
{
}
