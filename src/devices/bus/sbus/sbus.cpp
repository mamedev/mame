// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

  sbus.c - Sun SBus slot bus and card emulation

***************************************************************************/

#include "emu.h"

// Display boards
#include "bwtwo.h"
#include "cgthree.h"
#include "cgsix.h"

// Accelerator boards
#include "sunpc.h"

// Peripheral boards
#include "artecon.h"
#include "hme.h"

#include "sbus.h"

void sbus_cards(device_slot_interface &device)
{
	device.option_add("bwtwo",    SBUS_BWTWO);    /* Sun bwtwo monochrome display board */
	device.option_add("cgthree",  SBUS_CGTHREE);  /* Sun cgthree color display board */
	device.option_add("turbogx",  SBUS_TURBOGX);  /* Sun TurboGX 8-bit color display board */
	device.option_add("turbogxp", SBUS_TURBOGXP); /* Sun TurboGX+ 8-bit color display board */
	device.option_add("sunpc",    SBUS_SUNPC);    /* Sun SunPC 5x86 Accelerator board */
	device.option_add("hme",      SBUS_HME);      /* Sun SunSwift 10/100 + Fast Wide SCSI "Colossus" board */
	device.option_add("sb300p",   SBUS_SB300P);   /* Artecon CB300P 3-serial/1-parallel board */
}

DEFINE_DEVICE_TYPE(SBUS_SLOT, sbus_slot_device, "sbus_slot", "Sun SBus Slot")

sbus_slot_device::sbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sbus_slot_device(mconfig, SBUS_SLOT, tag, owner, clock)
{
}

sbus_slot_device::sbus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_slot_interface(mconfig, *this)
	, m_sbus(*this, finder_base::DUMMY_TAG)
	, m_slot(-1)
{
}

void sbus_slot_device::device_validity_check(validity_checker &valid) const
{
	if (m_slot < 0 || m_slot > 2)
		osd_printf_error("Slot %d out of range for Sun SBus\n", m_slot);

	device_t *const card(get_card_device());
	if (card && !dynamic_cast<device_sbus_card_interface *>(card))
		osd_printf_error("Card device %s (%s) does not implement device_sbus_card_interface\n", card->tag(), card->name());
}

void sbus_slot_device::device_resolve_objects()
{
	device_sbus_card_interface *const sbus_card(dynamic_cast<device_sbus_card_interface *>(get_card_device()));
	if (sbus_card)
		sbus_card->set_sbus(m_sbus, m_slot);
}

void sbus_slot_device::device_start()
{
	device_t *const card(get_card_device());
	if (card && !dynamic_cast<device_sbus_card_interface *>(card))
		throw emu_fatalerror("sbus_slot_device: card device %s (%s) does not implement device_sbus_card_interface\n", card->tag(), card->name());
}


DEFINE_DEVICE_TYPE(SBUS, sbus_device, "sbus", "Sun SBus")

device_memory_interface::space_config_vector sbus_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

sbus_device::sbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sbus_device(mconfig, SBUS, tag, owner, clock)
{
}

sbus_device::sbus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_space_config("SBus Space", ENDIANNESS_BIG, 32, 32, 0, address_map_constructor())
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_type1space(*this, finder_base::DUMMY_TAG)
	, m_irq_cb{{*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}}
	, m_buserr(*this)
{
}

void sbus_device::device_resolve_objects()
{
	// resolve callbacks
	for (int i = 0; i < 7; i++)
		m_irq_cb[i].resolve_safe();
	m_buserr.resolve_safe();
}

void sbus_device::device_start()
{
	std::fill(std::begin(m_device_list), std::end(m_device_list), nullptr);

	m_space = &space(0);
	m_space->install_readwrite_handler(0x00000000, 0x01ffffff, read32_delegate(FUNC(sbus_device::slot_timeout_r<0>), this), write32_delegate(FUNC(sbus_device::slot_timeout_w<0>), this));
	m_space->install_readwrite_handler(0x02000000, 0x03ffffff, read32_delegate(FUNC(sbus_device::slot_timeout_r<1>), this), write32_delegate(FUNC(sbus_device::slot_timeout_w<1>), this));
	m_space->install_readwrite_handler(0x04000000, 0x05ffffff, read32_delegate(FUNC(sbus_device::slot_timeout_r<2>), this), write32_delegate(FUNC(sbus_device::slot_timeout_w<2>), this));
}

template <unsigned Slot> READ32_MEMBER(sbus_device::slot_timeout_r)
{
	m_maincpu->set_mae();
	m_buserr(0, 0x20);
	m_buserr(1, 0xffa00000 + (Slot << 21));
	return 0;
}

template <unsigned Slot> WRITE32_MEMBER(sbus_device::slot_timeout_w)
{
	m_maincpu->set_mae();
	m_buserr(0, 0x8020);
	m_buserr(1, 0xffa00000 + (Slot << 21));
}

READ32_MEMBER(sbus_device::read)
{
	return m_space->read_dword(offset << 2, mem_mask);
}

WRITE32_MEMBER(sbus_device::write)
{
	m_space->write_dword(offset << 2, data, mem_mask);
}

device_sbus_card_interface *sbus_device::get_sbus_card(int slot)
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

void sbus_device::add_sbus_card(int slot, device_sbus_card_interface *card)
{
	m_device_list[slot] = card;
	card->install_device();
}

void sbus_device::set_irq_line(int state, int line)
{
	m_irq_cb[line](state);
}



device_sbus_card_interface::device_sbus_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
	, m_sbus_finder(device, finder_base::DUMMY_TAG)
	, m_sbus(nullptr)
	, m_slot(-1)
	, m_base(0)
{
}

device_sbus_card_interface::~device_sbus_card_interface()
{
}

void device_sbus_card_interface::interface_validity_check(validity_checker &valid) const
{
	if (m_sbus_finder && m_sbus && (m_sbus != m_sbus_finder))
		osd_printf_error("Contradictory buses configured (%s and %s)\n", m_sbus_finder->tag(), m_sbus->tag());
}

void device_sbus_card_interface::interface_pre_start()
{
	device_slot_card_interface::interface_pre_start();

	if (!m_sbus)
	{
		m_sbus = m_sbus_finder;
		if (!m_sbus)
			fatalerror("Can't find Sun SBus device %s\n", m_sbus_finder.finder_tag());
	}

	if (!m_sbus->started())
		throw device_missing_dependencies();
}

void device_sbus_card_interface::interface_post_start()
{
	m_base = m_slot << 25;
	m_sbus->add_sbus_card(m_slot, this);
}

void device_sbus_card_interface::set_sbus(sbus_device *sbus, int slot)
{
	m_sbus = sbus;
	m_slot = slot;
}
