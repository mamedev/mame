// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

  gio64.cpp - SGI GIO64 slot bus and GIO64 device emulation

***************************************************************************/

#include "emu.h"

// Display boards
#include "newport.h"

#include "gio64.h"

void gio64_cards(device_slot_interface &device)
{
	device.option_add("xl8",    GIO64_XL8);  /* SGI 8-bit XL board */
	device.option_add("xl24",   GIO64_XL24); /* SGI 24-bit XL board */
}

DEFINE_DEVICE_TYPE(GIO64_SLOT, gio64_slot_device, "gio64_slot", "SGI GIO64 Slot")

gio64_slot_device::gio64_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gio64_slot_device(mconfig, GIO64_SLOT, tag, owner, clock)
{
}

gio64_slot_device::gio64_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_slot_interface(mconfig, *this)
	, m_gio64(*this, finder_base::DUMMY_TAG)
	, m_slot_type(GIO64_SLOT_COUNT)
{
}

void gio64_slot_device::device_validity_check(validity_checker &valid) const
{
	if (m_slot_type == GIO64_SLOT_COUNT)
		osd_printf_error("Slot type not defined\n");

	device_t *const card(get_card_device());
	if (card && !dynamic_cast<device_gio64_card_interface *>(card))
		osd_printf_error("Card device %s (%s) does not implement device_gio64_card_interface\n", card->tag(), card->name());
}

void gio64_slot_device::device_resolve_objects()
{
	device_gio64_card_interface *const gio64_card(dynamic_cast<device_gio64_card_interface *>(get_card_device()));
	if (gio64_card)
		gio64_card->set_gio64(m_gio64, m_slot_type);
}

void gio64_slot_device::device_start()
{
	device_t *const card(get_card_device());
	if (card && !dynamic_cast<device_gio64_card_interface *>(card))
		throw emu_fatalerror("gio64_slot_device: card device %s (%s) does not implement device_gio64_card_interface\n", card->tag(), card->name());
}


DEFINE_DEVICE_TYPE(GIO64, gio64_device, "gio64", "SGI GIO64 Bus")

device_memory_interface::space_config_vector gio64_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

gio64_device::gio64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gio64_device(mconfig, GIO64, tag, owner, clock)
{
}

gio64_device::gio64_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_space_config("GIO64 Space", ENDIANNESS_BIG, 64, 32, 0, address_map_constructor())
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_interrupt_cb{{*this}, {*this}, {*this}}
{
}

void gio64_device::device_resolve_objects()
{
	for (auto &cb : m_interrupt_cb)
		cb.resolve_safe();
}

void gio64_device::device_start()
{
	std::fill(std::begin(m_device_list), std::end(m_device_list), nullptr);

	m_space = &space(0);
	m_space->install_readwrite_handler(0x00000000, 0x003fffff, read64_delegate(*this, FUNC(gio64_device::no_gfx_r)), write64_delegate(*this, FUNC(gio64_device::no_gfx_w)));
	m_space->install_readwrite_handler(0x00400000, 0x005fffff, read64_delegate(*this, FUNC(gio64_device::no_exp0_r)), write64_delegate(*this, FUNC(gio64_device::no_exp0_w)));
	m_space->install_readwrite_handler(0x00600000, 0x009fffff, read64_delegate(*this, FUNC(gio64_device::no_exp1_r)), write64_delegate(*this, FUNC(gio64_device::no_exp1_w)));
}

READ64_MEMBER(gio64_device::no_gfx_r)  { return ~0ULL; }
READ64_MEMBER(gio64_device::no_exp0_r) { return ~0ULL; }
READ64_MEMBER(gio64_device::no_exp1_r) { return ~0ULL; }
WRITE64_MEMBER(gio64_device::no_gfx_w)  { }
WRITE64_MEMBER(gio64_device::no_exp0_w) { }
WRITE64_MEMBER(gio64_device::no_exp1_w) { }

READ64_MEMBER(gio64_device::read)
{
	return m_space->read_qword(offset << 3, mem_mask);
}

WRITE64_MEMBER(gio64_device::write)
{
	m_space->write_qword(offset << 3, data, mem_mask);
}

device_gio64_card_interface *gio64_device::get_gio64_card(int slot)
{
	if (slot < 0)
	{
		return nullptr;
	}

	if (m_device_list[slot])
	{
		return m_device_list[slot];
	}

	return nullptr;
}

void gio64_device::add_gio64_card(gio64_slot_device::slot_type_t slot_type, device_gio64_card_interface *card)
{
	assert(slot_type >= 0 && slot_type < gio64_slot_device::GIO64_SLOT_COUNT);
	m_device_list[slot_type] = card;
	card->install_device();
}



device_gio64_card_interface::device_gio64_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "sgigio64")
	, m_gio64(nullptr)
	, m_gio64_slottag(nullptr)
	, m_slot_type(gio64_slot_device::GIO64_SLOT_COUNT)
{
}

device_gio64_card_interface::~device_gio64_card_interface()
{
}

void device_gio64_card_interface::interface_validity_check(validity_checker &valid) const
{
}

void device_gio64_card_interface::interface_pre_start()
{
	if (!m_gio64)
	{
		fatalerror("Can't find SGI GIO64 device\n");
	}

	if (!m_gio64->started())
		throw device_missing_dependencies();
}

void device_gio64_card_interface::interface_post_start()
{
	m_gio64->add_gio64_card(m_slot_type, this);
}

void device_gio64_card_interface::set_gio64(gio64_device *gio64, gio64_slot_device::slot_type_t slot_type)
{
	m_gio64 = gio64;
	m_slot_type = slot_type;
}
