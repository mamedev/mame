// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

    NEC PC-8801-23 / -24 / -25 "Sound Board II"

    TODO:
    - Irq hangs if there's a PC-8801 OPN/OPNA declared in driver.
      Master OPNA keeps sending acks in the correlated INT4 handler that hampers irq
      signals from here;
    - Confirm there's no DB9 joyport on any variants of this;

**************************************************************************************************/

#include "emu.h"
#include "pc8801_23.h"

DEFINE_DEVICE_TYPE(PC8801_23, pc8801_23_device, "pc8801_23", "NEC PC-8801-23 \"Sound Board II\"")

pc8801_23_device::pc8801_23_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pc8801_exp_device(mconfig, PC8801_23, tag, owner, clock)
	, m_opna(*this, "opna")
{
}

void pc8801_23_device::io_map(address_map &map)
{
	map(0xa8, 0xa9).rw(m_opna, FUNC(ym2608_device::read), FUNC(ym2608_device::write));
	map(0xaa, 0xaa).rw(FUNC(pc8801_23_device::irq_status_r), FUNC(pc8801_23_device::irq_mask_w));
	map(0xac, 0xad).lrw8(
		NAME([this] (offs_t offset) { return m_opna->read(offset | 2); }),
		NAME([this] (offs_t offset, u8 data) { m_opna->write(offset | 2, data); })
	);
}

void pc8801_23_device::opna_map(address_map &map)
{
	// TODO: confirm it really is ROMless
	// TODO: confirm size
	map(0x000000, 0x1fffff).ram();
}

void pc8801_23_device::device_add_mconfig(machine_config &config)
{
	constexpr XTAL sound_clock = XTAL(31'948'800) / 4;

	YM2608(config, m_opna, sound_clock);
	m_opna->set_addrmap(0, &pc8801_23_device::opna_map);
	m_opna->irq_handler().set(FUNC(pc8801_23_device::int4_w));
//  m_opna->port_a_read_callback().set(FUNC(pc8801_23_device::opn_porta_r));
//  m_opna->port_b_read_callback().set_ioport("OPN_PB");
	// TODO: per-channel mixing is unconfirmed
	m_opna->add_route(0, "^^speaker", 0.25, 0);
	m_opna->add_route(0, "^^speaker", 0.25, 1);
	m_opna->add_route(1, "^^speaker", 0.50, 0);
	m_opna->add_route(2, "^^speaker", 0.50, 1);
}

void pc8801_23_device::device_start()
{
	save_item(NAME(m_irq_mask));
	save_item(NAME(m_irq_pending));
}

void pc8801_23_device::device_reset()
{
	m_irq_mask = true;
	m_irq_pending = false;
}

u8 pc8801_23_device::irq_status_r()
{
	return 0x7f | (m_irq_mask << 7);
}

void pc8801_23_device::irq_mask_w(u8 data)
{
	m_irq_mask = bool(BIT(data, 7));

	if (!m_irq_mask && m_irq_pending)
		int4_w(m_irq_pending);
}

void pc8801_23_device::int4_w(int state)
{
	bool irq_state = (!m_irq_mask) & state;
	pc8801_exp_device::int4_w(irq_state);
	m_irq_pending = state;
}
