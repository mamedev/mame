// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Nokia MikroMikko 2 expansion bus

***************************************************************************/

#include "emu.h"
#include "exp.h"
#include "crtc186.h"
#include "meme186.h"
#include "mmc186.h"


DEFINE_DEVICE_TYPE(MIKROMIKKO2_EXPANSION_BUS, mikromikko2_expansion_bus_device, "mikromikko2_expansion_bus", "Nokia MikroMikko 2 expansion bus")
DEFINE_DEVICE_TYPE(MIKROMIKKO2_EXPANSION_BUS_SLOT, mikromikko2_expansion_bus_slot_device, "mikromikko2_expansion_bus_slot", "Nokia MikroMikko 2 expansion bus slot")


// ======================> mikromikko2_expansion_bus_device

mikromikko2_expansion_bus_device::mikromikko2_expansion_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mikromikko2_expansion_bus_device(mconfig, MIKROMIKKO2_EXPANSION_BUS, tag, owner, clock)
{
}

mikromikko2_expansion_bus_device::mikromikko2_expansion_bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_memspace(*this, finder_base::DUMMY_TAG, -1),
	m_iospace(*this, finder_base::DUMMY_TAG, -1),
	m_out_nmi_cb(*this),
	m_out_ir2_cb(*this),
	m_out_ir3_cb(*this),
	m_out_ir4_cb(*this),
	m_out_ir5_cb(*this),
	m_out_ir6_cb(*this),
	m_out_hold1_cb(*this),
	m_out_hold2_cb(*this),
	m_out_hold3_cb(*this),
	m_out_hold4_cb(*this),
	m_out_hold5_cb(*this)
{
}

void mikromikko2_expansion_bus_device::device_start()
{
}

void mikromikko2_expansion_bus_device::add_slot(const char *tag)
{
	device_t *dev = subdevice(tag);
	auto slot = dynamic_cast<device_slot_interface *>(dev);
	m_slot_list.push_front(slot);
}

void mikromikko2_expansion_bus_device::inta_w(int state)
{
	for (device_slot_interface *slot : m_slot_list)
	{
		device_t *dev = slot->get_card_device();
		if (dev) {
			auto *card = dynamic_cast<device_mikromikko2_expansion_bus_card_interface *>(dev);
			card->inta_w(state);
		}
	}
}

void mikromikko2_expansion_bus_device::bhlda_w(int state, int bcas)
{
	for (device_slot_interface *slot : m_slot_list)
	{
		device_t *dev = slot->get_card_device();
		if (dev) {
			auto *card = dynamic_cast<device_mikromikko2_expansion_bus_card_interface *>(dev);
			card->bhlda_w(state, bcas);
		}
	}
}


// ======================> mikromikko2_expansion_bus_slot_device

mikromikko2_expansion_bus_slot_device::mikromikko2_expansion_bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mikromikko2_expansion_bus_slot_device(mconfig, MIKROMIKKO2_EXPANSION_BUS_SLOT, tag, owner, clock)
{
}

mikromikko2_expansion_bus_slot_device::mikromikko2_expansion_bus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_bus(*this, finder_base::DUMMY_TAG)
{
}

void mikromikko2_expansion_bus_slot_device::device_start()
{
	auto *const dev = dynamic_cast<device_mikromikko2_expansion_bus_card_interface *>(get_card_device());

	if (dev) dev->set_bus(m_bus);

	m_bus->add_slot(tag());
}


// ======================> device_mikromikko2_expansion_bus_card_interface

device_mikromikko2_expansion_bus_card_interface::device_mikromikko2_expansion_bus_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "mikromikko2_expansion_bus"),
	m_bus(nullptr),
	m_card(&device)
{
}


// ======================> mikromikko2_expansion_bus_cards

void mikromikko2_expansion_bus_cards(device_slot_interface &device)
{
	device.option_add("crtc186", NOKIA_CRTC186);
	device.option_add("meme186", NOKIA_MEME186);
	device.option_add("mmc186", NOKIA_MMC186);
}
