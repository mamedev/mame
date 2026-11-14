// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy Model II/12/16/6000 card cage emulation

    80-pin J0 backplane bus. Cards install their memory and I/O
    handlers into the Z80 spaces through the bus. Memory at 8000-FFFF
    is a view selected by the port FF page latch on every board, and
    by LOW PAGE on the Model 12 main board.

**********************************************************************/

#ifndef MAME_BUS_TRS80M2_SLOT_H
#define MAME_BUS_TRS80M2_SLOT_H

#pragma once

#include "machine/z80daisy.h"

#include <utility>
#include <vector>


class trs80m2_slot_device;
class device_trs80m2_card_interface;

class trs80m2_bus_device : public device_t
{
public:
	static constexpr u32 PAGE_ENTRIES = 64;

	trs80m2_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <typename T> void set_cpu(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }

	auto int_callback() { return m_write_int.bind(); }
	auto nmi_callback() { return m_write_nmi.bind(); }
	auto kbirq_callback() { return m_write_kbirq.bind(); }
	auto xferrq_callback() { return m_write_xferrq.bind(); }
	auto useirq_callback() { return m_write_useirq.bind(); }
	auto page_callback() { return m_write_page.bind(); }

	void page_map(address_map &map) ATTR_COLD;

	address_space &program() { return m_cpu->space(AS_PROGRAM); }
	address_space &io() { return m_cpu->space(AS_IO); }

	static constexpr u32 entry(int lowp, int ff) { return (lowp << 5) | (BIT(ff, 7) << 4) | (ff & 0x0f); }
	static constexpr int entry_page(u32 entry) { return entry & 0x0f; }
	static constexpr int entry_vram(u32 entry) { return BIT(entry, 4); }
	static constexpr int entry_lowp(u32 entry) { return BIT(entry, 5); }
	memory_view::memory_view_entry &page_view(u32 entry) { return m_page[entry]; }
	void install_page_ram(int page, offs_t end, uint8_t *ram) ATTR_COLD;
	void claim_window(u32 entry) { m_claimed |= u64(1) << entry; }
	bool window_claimed(u32 entry) const { return BIT(m_claimed, entry); }

	void lowp_w(int state);

	u32 add_source() ATTR_COLD { return m_sources++; }
	void int_w(u32 source, int state) { line_w(m_int, source, state, m_write_int); }
	void nmi_w(u32 source, int state) { line_w(m_nmi, source, state, m_write_nmi); }
	void kbirq_w(u32 source, int state) { line_w(m_kbirq, source, state, m_write_kbirq); }
	void xferrq_w(u32 source, int state) { line_w(m_xferrq, source, state, m_write_xferrq); }
	void useirq_w(u32 source, int state) { line_w(m_useirq, source, state, m_write_useirq); }

	bool wait(int cycles, const void *tag);
	attotime local_time() const;

	void add_card(device_trs80m2_card_interface &card) ATTR_COLD { m_cards.push_back(&card); }
	void m68k_map(address_space &space);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void line_w(u32 &line, u32 source, int state, devcb_write_line &cb)
	{
		u32 const prev = line;
		if (state)
			line |= u32(1) << source;
		else
			line &= ~(u32(1) << source);
		if (bool(prev) != bool(line))
			cb(line ? 1 : 0);
	}

	void ff_w(offs_t offset, uint8_t &data, uint8_t mem_mask);
	void update_page();

	required_device<cpu_device> m_cpu;
	memory_view m_page;

	devcb_write_line m_write_int;
	devcb_write_line m_write_nmi;
	devcb_write_line m_write_kbirq;
	devcb_write_line m_write_xferrq;
	devcb_write_line m_write_useirq;
	devcb_write8 m_write_page;

	memory_passthrough_handler m_ff_tap;
	std::vector<device_trs80m2_card_interface *> m_cards;

	bool m_tap_installed;
	u64 m_claimed;
	u32 m_sources;
	u32 m_int;
	u32 m_nmi;
	u32 m_kbirq;
	u32 m_xferrq;
	u32 m_useirq;
	uint8_t m_ff;
	uint8_t m_lowp;
};


class device_trs80m2_card_interface : public device_interface
{
public:
	virtual int z80daisy_irq_state();
	virtual int z80daisy_irq_ack();
	virtual void z80daisy_irq_reti();

	virtual void m68k_map(address_space &space);

protected:
	device_trs80m2_card_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override ATTR_COLD;

	trs80m2_slot_device *m_slot;
};


class trs80m2_slot_device : public device_t,
		public device_single_card_slot_interface<device_trs80m2_card_interface>,
		public device_z80daisy_interface
{
public:
	template <typename T, typename U>
	trs80m2_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&bus, U &&opts, const char *dflt) :
		trs80m2_slot_device(mconfig, tag, owner, clock)
	{
		m_bus.set_tag(std::forward<T>(bus));
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	template <typename T>
	trs80m2_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&opts, const char *dflt) :
		trs80m2_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	trs80m2_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	trs80m2_bus_device &bus() { return *m_bus; }
	auto &bus_finder() { return m_bus; }

	void int_w(int state) { m_bus->int_w(m_source, state); }
	void nmi_w(int state) { m_bus->nmi_w(m_source, state); }
	void kbirq_w(int state) { m_bus->kbirq_w(m_source, state); }
	void xferrq_w(int state) { m_bus->xferrq_w(m_source, state); }
	void useirq_w(int state) { m_bus->useirq_w(m_source, state); }

	// device_z80daisy_interface implementation
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	required_device<trs80m2_bus_device> m_bus;

	device_trs80m2_card_interface *m_card;
	u32 m_source;
};


DECLARE_DEVICE_TYPE(TRS80M2_BUS, trs80m2_bus_device)
DECLARE_DEVICE_TYPE(TRS80M2_SLOT, trs80m2_slot_device)

void trs80m2_cards(device_slot_interface &device);
void trs80m12_cards(device_slot_interface &device);
void trs80m12_j0_cards(device_slot_interface &device);

#endif // MAME_BUS_TRS80M2_SLOT_H
