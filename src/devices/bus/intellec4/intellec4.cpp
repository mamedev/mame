// license:BSD-3-Clause
// copyright-holders:Vas Crabb

#include "emu.h"
#include "intellec4.h"

#include <algorithm>


DEFINE_DEVICE_TYPE(INTELLEC4_UNIV_SLOT, bus::intellec4::univ_slot_device, "intlc4univslot", "INTELLEC 4 Universal Slot")
DEFINE_DEVICE_TYPE(INTELLEC4_UNIV_BUS,  bus::intellec4::univ_bus_device,  "intlc4univbus",  "INTELLEC 4 Universal Bus")


namespace bus::intellec4 {

/***********************************************************************
    SLOT DEVICE
***********************************************************************/

univ_slot_device::univ_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, INTELLEC4_UNIV_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_univ_card_interface>(mconfig, *this)
	, m_bus(*this, finder_base::DUMMY_TAG)
{
}


/*----------------------------------
  device_t implementation
----------------------------------*/

void univ_slot_device::device_resolve_objects()
{
	device_univ_card_interface *const univ_card(get_card_device());
	if (univ_card)
		univ_card->set_bus(*m_bus);
}

void univ_slot_device::device_start()
{
}


/***********************************************************************
    BUS DEVICE
***********************************************************************/

univ_bus_device::univ_bus_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, INTELLEC4_UNIV_BUS, tag, owner, clock)
	, m_rom_space(*this, finder_base::DUMMY_TAG, -1)
	, m_rom_ports_space(*this, finder_base::DUMMY_TAG, -1)
	, m_memory_space(*this, finder_base::DUMMY_TAG, -1)
	, m_status_space(*this, finder_base::DUMMY_TAG, -1)
	, m_ram_ports_space(*this, finder_base::DUMMY_TAG, -1)
	, m_test_out_cb(*this)
	, m_stop_out_cb(*this)
	, m_reset_4002_out_cb(*this)
	, m_user_reset_out_cb(*this)
	, m_test(0U)
	, m_stop(0U)
	, m_reset_4002(0U)
	, m_user_reset(0U)
{
	std::fill(std::begin(m_cards), std::end(m_cards), nullptr);
}


/*----------------------------------
  input lines
----------------------------------*/

WRITE_LINE_MEMBER(univ_bus_device::sync_in)
{
	for (device_univ_card_interface *card : m_cards)
	{
		if (card)
			card->sync_in(state);
		else
			break;
	}
}

WRITE_LINE_MEMBER(univ_bus_device::stop_acknowledge_in)
{
	for (device_univ_card_interface *card : m_cards)
	{
		if (card)
			card->stop_acknowledge_in(state);
		else
			break;
	}
}

WRITE_LINE_MEMBER(univ_bus_device::cpu_reset_in)
{
	for (device_univ_card_interface *card : m_cards)
	{
		if (card)
			card->cpu_reset_in(state);
		else
			break;
	}
}


/*----------------------------------
  device_t implementation
----------------------------------*/

void univ_bus_device::device_start()
{
	m_test_out_cb.resolve_safe();
	m_stop_out_cb.resolve_safe();
	m_reset_4002_out_cb.resolve_safe();
	m_user_reset_out_cb.resolve_safe();

	save_item(NAME(m_test));
	save_item(NAME(m_stop));
	save_item(NAME(m_reset_4002));
	save_item(NAME(m_user_reset));
}


/*----------------------------------
  helpers for cards
----------------------------------*/

unsigned univ_bus_device::add_card(device_univ_card_interface &card)
{
	for (unsigned i = 0; std::size(m_cards) > i; ++i)
	{
		if (!m_cards[i])
		{
			m_cards[i] = &card;
			return i;
		}
	}
	throw emu_fatalerror("univ_bus_device: maximum number of cards (%u) exceeded\n", unsigned(std::size(m_cards)));
}

void univ_bus_device::set_test(unsigned index, int state)
{
	assert(std::size(m_cards) >= index);
	bool const changed(bool(state) != !BIT(m_test, index));
	if (changed)
	{
		u16 const other(m_test & ~(u16(1U) << index));
		if (state)
			m_test = other;
		else
			m_test |= u16(1U) << index;

		if ((std::size(m_cards) != index) && !(other & ~(u16(1U) << std::size(m_cards))))
			m_test_out_cb(state);

		for (unsigned card = 0U; (std::size(m_cards) > card) && m_cards[card]; ++card)
		{
			if ((index != card) && !(other & ~(u16(1U) << index)))
				m_cards[card]->test_in(state);
		}
	}
}

void univ_bus_device::set_stop(unsigned index, int state)
{
	assert(std::size(m_cards) >= index);
	bool const changed(bool(state) != !BIT(m_stop, index));
	if (changed)
	{
		u16 const other(m_stop & ~(u16(1U) << index));
		if (state)
			m_stop = other;
		else
			m_stop |= u16(1U) << index;

		if ((std::size(m_cards) != index) && !(other & ~(u16(1U) << std::size(m_cards))))
			m_stop_out_cb(state);

		for (unsigned card = 0U; (std::size(m_cards) > card) && m_cards[card]; ++card)
		{
			if ((index != card) && !(other & ~(u16(1U) << index)))
				m_cards[card]->stop_in(state);
		}
	}
}

void univ_bus_device::set_reset_4002(unsigned index, int state)
{
	assert(std::size(m_cards) >= index);
	bool const changed(bool(state) != !BIT(m_reset_4002, index));
	if (changed)
	{
		u16 const other(m_reset_4002 & ~(u16(1U) << index));
		if (state)
			m_reset_4002 = other;
		else
			m_reset_4002 |= u16(1U) << index;

		if ((std::size(m_cards) != index) && !(other & ~(u16(1U) << std::size(m_cards))))
			m_reset_4002_out_cb(state);

		for (unsigned card = 0U; (std::size(m_cards) > card) && m_cards[card]; ++card)
		{
			if ((index != card) && !(other & ~(u16(1U) << index)))
				m_cards[card]->reset_4002_in(state);
		}
	}
}

void univ_bus_device::set_user_reset(unsigned index, int state)
{
	assert(std::size(m_cards) > index);
	bool const changed(bool(state) != !BIT(m_user_reset, index));
	if (changed)
	{
		u16 const other(m_user_reset & ~(u16(1U) << index));
		if (state)
			m_user_reset = other;
		else
			m_user_reset |= u16(1U) << index;

		if (!other)
			m_user_reset_out_cb(state);

		for (unsigned card = 0U; (std::size(m_cards) > card) && m_cards[card]; ++card)
		{
			if ((index != card) && !(other & ~(u16(1U) << index)))
				m_cards[card]->user_reset_in(state);
		}
	}
}



/***********************************************************************
    CARD INTERFACE
***********************************************************************/

device_univ_card_interface::device_univ_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "intellec4")
	, m_bus(nullptr)
	, m_index(~unsigned(0))
{
}


void device_univ_card_interface::interface_pre_start()
{
	if (!m_bus)
		throw device_missing_dependencies();
}


void device_univ_card_interface::set_bus(univ_bus_device &bus)
{
	m_index = (m_bus = &bus)->add_card(*this);
}

} // namespace bus::intellec4



#include "insdatastor.h"
#include "prommemory.h"
#include "tapereader.h"


// must come after including the headers that declare these extern
template class device_finder<bus::intellec4::device_univ_card_interface, false>;
template class device_finder<bus::intellec4::device_univ_card_interface, true>;


void intellec4_univ_cards(device_slot_interface &device)
{
	device.option_add("imm4_22", INTELLEC4_INST_DATA_STORAGE);
	device.option_add("imm6_26", INTELLEC4_PROM_MEMORY);
	device.option_add("imm4_90", INTELLEC4_TAPE_READER);
}
