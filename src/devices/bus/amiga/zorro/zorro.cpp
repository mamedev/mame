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
	m_eint1(*this, true),
	m_eint4(*this, true),
	m_eint5(*this, true),
	m_eint7(*this, true),
	m_int2(*this, true),
	m_int6(*this, true),
	m_ovr(*this, false),
	m_xrdy(*this, false),
	m_autoconfig_device(0)
{
}

void zorro2_bus_device::device_start()
{
	// clear slots
	std::fill(std::begin(m_cards), std::end(m_cards), nullptr);

	// register for save states
	save_item(STRUCT_MEMBER(m_eint1, card_state));
	save_item(STRUCT_MEMBER(m_eint1, bus_state));
	save_item(STRUCT_MEMBER(m_eint4, card_state));
	save_item(STRUCT_MEMBER(m_eint4, bus_state));
	save_item(STRUCT_MEMBER(m_eint5, card_state));
	save_item(STRUCT_MEMBER(m_eint5, bus_state));
	save_item(STRUCT_MEMBER(m_eint7, card_state));
	save_item(STRUCT_MEMBER(m_eint7, bus_state));
	save_item(STRUCT_MEMBER(m_int2, card_state));
	save_item(STRUCT_MEMBER(m_int2, bus_state));
	save_item(STRUCT_MEMBER(m_int6, card_state));
	save_item(STRUCT_MEMBER(m_int6, bus_state));
	save_item(STRUCT_MEMBER(m_ovr, card_state));
	save_item(STRUCT_MEMBER(m_ovr, bus_state));
	save_item(STRUCT_MEMBER(m_xrdy, card_state));
	save_item(STRUCT_MEMBER(m_xrdy, bus_state));
	save_item(NAME(m_autoconfig_device));
}

device_memory_interface::space_config_vector zorro2_bus_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_zorro_space_config)
	};
}

void zorro2_bus_device::add_card(int slot, device_zorro2_card_interface *card)
{
	m_cards[slot] = card;
}

void zorro2_bus_device::update_bus_line(int slot, int state, bus_line &line)
{
	line.card_state &= ~(1U << slot);
	line.card_state |= (state ? 1U : 0U) << slot;

	// bus state is 1 when any line is high (active high) or all of them are (active low)
	bool new_bus_state = line.active_high ? (line.card_state != 0x00) : (line.card_state == 0xff);

	if (new_bus_state != line.bus_state)
		line.handler(new_bus_state);

	line.bus_state = new_bus_state;
}

// from slot device
void zorro2_bus_device::cfgout_w(int slot, int state)
{
	if (state == 1)
		return;

	if (slot != m_autoconfig_device)
		fatalerror("Write to CFGOUT from invalid card!\n");

	if (m_autoconfig_device >= 8)
		return;

	// search for the next card
	while (++m_autoconfig_device < 8)
	{
		if (m_cards[m_autoconfig_device])
		{
			// found a card, tell it to configure itself and exit
			m_cards[m_autoconfig_device]->cfgin_w(0);
			break;
		}
	}
}

void zorro2_bus_device::eint1_w(int slot, int state) { update_bus_line(slot, state, m_eint1); }
void zorro2_bus_device::eint4_w(int slot, int state) { update_bus_line(slot, state, m_eint4); }
void zorro2_bus_device::eint5_w(int slot, int state) { update_bus_line(slot, state, m_eint5); }
void zorro2_bus_device::eint7_w(int slot, int state) { update_bus_line(slot, state, m_eint7); }
void zorro2_bus_device::int2_w(int slot, int state) { update_bus_line(slot, state, m_int2); }
void zorro2_bus_device::int6_w(int slot, int state) { update_bus_line(slot, state, m_int6); }
void zorro2_bus_device::ovr_w(int slot, int state) { update_bus_line(slot, state, m_ovr); }
void zorro2_bus_device::xrdy_w(int slot, int state) { update_bus_line(slot, state, m_xrdy); }

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
	for (auto *card : m_cards)
		if (card)
			card->fc_w(code);
}

void zorro2_bus_device::busrst_w(int state)
{
	for (auto *card : m_cards)
		if (card)
			card->busrst_w(state);

	if (state == 0)
	{
		// reset line states
		m_eint1.reset();
		m_eint4.reset();
		m_eint5.reset();
		m_eint7.reset();
		m_int2.reset();
		m_int6.reset();
		m_ovr.reset();
		m_xrdy.reset();

		// initiate autoconfig
		m_autoconfig_device = 0xff; // no autoconfig device yet

		// reset cfgin for all cards
		for (auto *card : m_cards)
			if (card)
				card->cfgin_w(1);

		// search for the first card and tell it to start autoconfiguration
		for (unsigned i = 0; i < 8; i++)
		{
			if (m_cards[i])
			{
				m_autoconfig_device = i;
				m_cards[i]->cfgin_w(0);
				break;
			}
		}
	}
}


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(ZORRO2_SLOT, zorro2_slot_device, "zorro2_slot", "Zorro-II Slot")

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
	device_zorro2_card_interface *const card = get_card_device();

	if (card)
	{
		zorro2_bus_device *bus = downcast<zorro2_bus_device *>(m_owner);
		card->set_bus(bus, tag());
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
	m_zorro(nullptr),
	m_slot_tag(nullptr),
	m_slot(-1)
{
}

device_zorro2_card_interface::~device_zorro2_card_interface()
{
}

void device_zorro2_card_interface::set_bus(zorro2_bus_device *device, const char *slot_tag)
{
	m_slot_tag = slot_tag;
	m_zorro = device;
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

void device_zorro2_card_interface::interface_pre_start()
{
	if (!m_zorro)
		fatalerror("Zorro-II Bus undefined\n");

	if (m_slot < 0)
	{
		if (!m_zorro->started())
			throw device_missing_dependencies();

		// extract the slot number from the last digit of the slot tag
		size_t const len = strlen(m_slot_tag);

		m_slot = (m_slot_tag[len - 1] - '0') - 1;
		if (m_slot < 0 || m_slot > 7)
			fatalerror("Slot %x out of range for Zorro-II Bus\n", m_slot);

		m_zorro->add_card(m_slot, this);
	}
}
