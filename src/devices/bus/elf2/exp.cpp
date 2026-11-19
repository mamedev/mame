// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Netronics Elf II 86-pin expansion bus emulation

**********************************************************************/

#include "emu.h"
#include "exp.h"

#include "giant.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ELF2_EXPANSION_SLOT, elf2_expansion_slot_device, "elf2_expansion_slot", "Netronics Elf II expansion slot")
DEFINE_DEVICE_TYPE(ELF2_EXPANSION_BUS, elf2_expansion_bus_device, "elf2_expansion_bus", "Netronics Elf II expansion bus")



//**************************************************************************
//  SLOT
//**************************************************************************

elf2_expansion_slot_device::elf2_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ELF2_EXPANSION_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_elf2_expansion_card_interface>(mconfig, *this),
	m_bus(*this, finder_base::DUMMY_TAG)
{
}

void elf2_expansion_slot_device::device_start()
{
	device_elf2_expansion_card_interface *const card = get_card_device();

	if (card)
		m_bus->add_card(*card);
}



//**************************************************************************
//  BUS
//**************************************************************************

elf2_expansion_bus_device::elf2_expansion_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ELF2_EXPANSION_BUS, tag, owner, clock),
	m_program(*this, finder_base::DUMMY_TAG, -1),
	m_io(*this, finder_base::DUMMY_TAG, -1)
{
}

void elf2_expansion_bus_device::device_start()
{
}

void elf2_expansion_bus_device::add_card(device_elf2_expansion_card_interface &card)
{
	m_cards.push_front(&card);
	card.m_bus = this;
}

int elf2_expansion_bus_device::ef1_r()
{
	int state = 0;

	for (device_elf2_expansion_card_interface *card : m_cards)
		state |= card->elf2_ef1_r();

	return state;
}

int elf2_expansion_bus_device::ef2_r()
{
	int state = 0;

	for (device_elf2_expansion_card_interface *card : m_cards)
		state |= card->elf2_ef2_r();

	return state;
}

int elf2_expansion_bus_device::ef3_r()
{
	int state = 0;

	for (device_elf2_expansion_card_interface *card : m_cards)
		state |= card->elf2_ef3_r();

	return state;
}

int elf2_expansion_bus_device::ef4_r()
{
	int state = 0;

	for (device_elf2_expansion_card_interface *card : m_cards)
		state |= card->elf2_ef4_r();

	return state;
}

void elf2_expansion_bus_device::q_w(int state)
{
	for (device_elf2_expansion_card_interface *card : m_cards)
		card->elf2_q_w(state);
}



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

device_elf2_expansion_card_interface::device_elf2_expansion_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "elf2exp"),
	m_bus(nullptr)
{
}

void device_elf2_expansion_card_interface::interface_pre_start()
{
	if (!m_bus)
		throw device_missing_dependencies();
}



//**************************************************************************
//  SLOT OPTIONS
//**************************************************************************

void elf2_expansion_cards(device_slot_interface &device)
{
	device.option_add("giant", ELF2_GIANT_BOARD);
}
