// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

  gio.cpp - SGI GIO slot bus and GIO device emulation

***************************************************************************/

#include "emu.h"

// Display boards
#include "newport.h"

#include "gio.h"

void gio_cards(device_slot_interface &device)
{
	device.option_add("xl8",    GIO_XL8);  /* SGI 8-bit XL board */
	device.option_add("xl24",   GIO_XL24); /* SGI 24-bit XL board */
}

DEFINE_DEVICE_TYPE(GIO_SLOT, gio_slot_device, "gio_slot", "SGI GIO Slot")

gio_slot_device::gio_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gio_slot_device(mconfig, GIO_SLOT, tag, owner, clock)
{
}

gio_slot_device::gio_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_slot_interface(mconfig, *this)
	, m_gio(*this, finder_base::DUMMY_TAG)
{
}

void gio_slot_device::device_validity_check(validity_checker &valid) const
{
	device_t *const card(get_card_device());
	if (card && !dynamic_cast<device_gio_card_interface *>(card))
		osd_printf_error("Card device %s (%s) does not implement device_gio_card_interface\n", card->tag(), card->name());
}

void gio_slot_device::device_resolve_objects()
{
	device_gio_card_interface *const gio_card(dynamic_cast<device_gio_card_interface *>(get_card_device()));
	if (gio_card)
		gio_card->set_gio(m_gio, tag());
}

void gio_slot_device::device_start()
{
	device_t *const card(get_card_device());
	if (card && !dynamic_cast<device_gio_card_interface *>(card))
		throw emu_fatalerror("gio_slot_device: card device %s (%s) does not implement device_gio_card_interface\n", card->tag(), card->name());
}


DEFINE_DEVICE_TYPE(GIO, gio_device, "gio", "SGI GIO Bus")

device_memory_interface::space_config_vector gio_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

gio_device::gio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gio_device(mconfig, GIO, tag, owner, clock)
{
}

gio_device::gio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_space_config("GIO Space", ENDIANNESS_BIG, 64, 32, 0, address_map_constructor())
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_hpc3(*this, finder_base::DUMMY_TAG)
{
}

void gio_device::device_resolve_objects()
{
}

void gio_device::device_start()
{
	std::fill(std::begin(m_device_list), std::end(m_device_list), nullptr);

	m_space = &space(0);
	m_space->install_readwrite_handler(0x00000000, 0x003fffff, read64_delegate(FUNC(gio_device::no_gfx_r), this), write64_delegate(FUNC(gio_device::no_gfx_w), this));
	m_space->install_readwrite_handler(0x00400000, 0x005fffff, read64_delegate(FUNC(gio_device::no_exp0_r), this), write64_delegate(FUNC(gio_device::no_exp0_w), this));
	m_space->install_readwrite_handler(0x00600000, 0x009fffff, read64_delegate(FUNC(gio_device::no_exp1_r), this), write64_delegate(FUNC(gio_device::no_exp1_w), this));
}

READ64_MEMBER(gio_device::no_gfx_r)  { return ~0ULL; }
READ64_MEMBER(gio_device::no_exp0_r) { return ~0ULL; }
READ64_MEMBER(gio_device::no_exp1_r) { return ~0ULL; }
WRITE64_MEMBER(gio_device::no_gfx_w)  { }
WRITE64_MEMBER(gio_device::no_exp0_w) { }
WRITE64_MEMBER(gio_device::no_exp1_w) { }

READ64_MEMBER(gio_device::read)
{
	return m_space->read_qword(offset << 3, mem_mask);
}

WRITE64_MEMBER(gio_device::write)
{
	m_space->write_qword(offset << 3, data, mem_mask);
}

device_gio_card_interface *gio_device::get_gio_card(int slot)
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

void gio_device::add_gio_card(device_gio_card_interface::gio_slot_type_t slot_type, device_gio_card_interface *card)
{
	m_device_list[slot_type] = card;
	card->install_device();
}



device_gio_card_interface::device_gio_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
	, m_gio(nullptr)
	, m_gio_slottag(nullptr)
	, m_slot_type(GIO_SLOT_COUNT)
{
}

device_gio_card_interface::~device_gio_card_interface()
{
}

void device_gio_card_interface::interface_validity_check(validity_checker &valid) const
{
}

void device_gio_card_interface::interface_pre_start()
{
	device_slot_card_interface::interface_pre_start();

	if (!m_gio)
	{
		fatalerror("Can't find SGI GIO device\n");
	}

	if (GIO_SLOT_COUNT == m_slot_type)
	{
		if (!m_gio->started())
			throw device_missing_dependencies();

		if (strcmp(m_gio_slottag, ":gio_gfx") == 0)
			m_slot_type = GIO_SLOT_GFX;
		else if (strcmp(m_gio_slottag, ":gio_exp0") == 0)
			m_slot_type = GIO_SLOT_EXP0;
		else if (strcmp(m_gio_slottag, ":gio_exp1") == 0)
			m_slot_type = GIO_SLOT_EXP1;
		else
			fatalerror("Slot %s incorrectly named for SGI GIO graphics slot\n", m_gio_slottag);

		m_gio->add_gio_card(m_slot_type, this);
	}
}

void device_gio_card_interface::set_gio(gio_device *gio, const char *slottag)
{
	m_gio = gio;
	m_gio_slottag = slottag;
}
