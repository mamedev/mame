// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_VL82C420_H
#define MAME_MACHINE_VL82C420_H

#pragma once

#include "bus/isa/isa.h"
#include "machine/am9517a.h"
#include "machine/at_keybc.h"
#include "machine/ds128x.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"

class vl82c420_device : public device_t,
				       public device_memory_interface
{
public:
	template <typename T, typename U, typename V, typename W, typename X>
	vl82c420_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cputag, U &&biostag, V &&keybctag, W &&ramtag, X &&isatag)
		: vl82c420_device(mconfig, tag, owner, clock)
	{
		set_cputag(std::forward<T>(cputag));
		set_biostag(std::forward<U>(biostag));
		set_keybctag(std::forward<V>(keybctag));
		set_ramtag(std::forward<W>(ramtag));
		set_isatag(std::forward<X>(isatag));
	}

	vl82c420_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto ior() { return m_read_ior.bind(); }
	auto iow() { return m_write_iow.bind(); }
	auto tc() { return m_write_tc.bind(); }
	auto hold() { return m_write_hold.bind(); }
	auto cpureset() { return m_write_cpureset.bind(); }
	auto nmi() { return m_write_nmi.bind(); }
	auto intr() { return m_write_intr.bind(); }
	auto a20m() { return m_write_a20m.bind(); }
	auto spkr() { return m_write_spkr.bind(); }

	// inline configuration
	template <typename T> void set_cputag(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_biostag(T &&tag) { m_bios.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_keybctag(T &&tag) { m_keybc.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_ramtag(T &&tag) { m_ram_dev.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_isatag(T &&tag) { m_isabus.set_tag(std::forward<T>(tag)); }

	IRQ_CALLBACK_MEMBER(int_ack_r) { return m_intc[0]->acknowledge(); }

	void fast_gatea20(int state) {
		m_fast_gatea20 = state;
		m_write_a20m(m_fast_gatea20 | m_ext_gatea20);
	}

	void keyboard_gatea20(int state) {
		m_ext_gatea20 = state;
		m_write_a20m(m_fast_gatea20 | m_ext_gatea20);
	}

	void irq01_w(int state) { m_intc[0]->ir1_w(state); }
	void irq03_w(int state) { m_intc[0]->ir3_w(state); }
	void irq04_w(int state) { m_intc[0]->ir4_w(state); }
	void irq05_w(int state) { m_intc[0]->ir5_w(state); }
	void irq06_w(int state) { m_intc[0]->ir6_w(state); }
	void irq07_w(int state) { m_intc[0]->ir7_w(state); }
	void irq09_w(int state) { m_intc[1]->ir1_w(state); }
	void irq10_w(int state) { m_intc[1]->ir2_w(state); }
	void irq11_w(int state) { m_intc[1]->ir3_w(state); }
	void irq12_w(int state) { m_intc[1]->ir4_w(state); }
	void irq13_w(int state) { m_intc[1]->ir5_w(state); } // also FERR#
	void irq14_w(int state) { m_intc[1]->ir6_w(state); }
	void irq15_w(int state) { m_intc[1]->ir7_w(state); }
	void dreq0_w(int state) { m_dma[0]->dreq0_w(state); }
	void dreq1_w(int state) { m_dma[0]->dreq1_w(state); }
	void dreq2_w(int state) { m_dma[0]->dreq2_w(state); }
	void dreq3_w(int state) { m_dma[0]->dreq3_w(state); }
	void dreq5_w(int state) { m_dma[1]->dreq1_w(state); }
	void dreq6_w(int state) { m_dma[1]->dreq2_w(state); }
	void dreq7_w(int state) { m_dma[1]->dreq3_w(state); }
	void hlda_w(int state)  { m_dma[1]->hack_w(state); }

	void iochck_w(int state)
	{
		if (BIT(m_portb, 3) == 0)
		{
			if (m_iochck && state == 0)
			{
				// set channel check latch
				m_portb |= 1 << 6;
				trigger_nmi();
			}

			m_iochck = state;
		}
	}
	void gatea20_w(int state) { keyboard_gatea20(state); }
	void kbrst_w(int state) {
		// convert to active low signal (gets inverted in at_keybc.c)
		state = (state == ASSERT_LINE ? 0 : 1);

		//if (?)
		//{
		//	// detect transition
		if (m_kbrst == 1 && state == 0)
		{
			m_write_cpureset(1);
			m_write_cpureset(0);
		}
		//}

		m_kbrst = state;

	}


protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_reset_after_children() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override ATTR_COLD;

	void io_map(address_map &map) ATTR_COLD;
	void config_map(address_map &map) ATTR_COLD;
private:
	const address_space_config m_space_config;

	required_device<device_memory_interface> m_cpu;
	required_device<at_kbc_device_base> m_keybc;
	required_region_ptr<u32> m_bios;

	address_space *m_space_mem;
	address_space *m_space_io;
	u8 *m_ram;

	required_device_array<am9517a_device, 2> m_dma;
	required_device_array<pic8259_device, 2> m_intc;
	required_device<pit8254_device> m_pit;
	required_device<ds12885_device> m_rtc;
	required_device<ram_device> m_ram_dev;
	required_device<isa16_device> m_isabus;

	devcb_read16 m_read_ior;
	devcb_write16 m_write_iow;
	devcb_write8 m_write_tc;
	devcb_write_line m_write_hold;
	devcb_write_line m_write_nmi;
	devcb_write_line m_write_intr;
	devcb_write_line m_write_cpureset;
	devcb_write_line m_write_a20m;
	devcb_write_line m_write_spkr;

	int m_dma_eop;
	u8 m_dma_page[0x10];
	u8 m_dma_high_byte;
	int m_dma_channel;

	u8 m_portb;
	int m_refresh_toggle;
	int m_iochck;
	int m_nmi_mask;

	int m_cpureset;
	int m_kbrst;
	int m_ext_gatea20;
	int m_fast_gatea20;

	u8 portb_r();
	void portb_w(u8 data);

	u8 m_config_address;
	bool m_config_unlock;
	u8 m_ramtmg;
	u8 m_ramcfg[2];
	u8 m_ramset;
	u8 m_ntbref;
	u8 m_clkctl;
	u8 m_miscset;
	u8 m_dmactl;
	u8 m_busctl;
	u8 m_fbcr;
	u8 m_romset;
	u8 m_segment_access[6];
	u8 m_segment_cache[6];
	u8 m_cachctl;
	u8 m_pmra[2];
	u8 m_pmre[2];
	u8 m_xctl;

	void config_address_w(offs_t offset, u8 data);
	u8 config_data_r(offs_t offset);
	void config_data_w(offs_t offset, u8 data);

//	std::vector<u8> m_shadow_ram;

	void update_segment_settings();
//	void update_dma_clock();

	offs_t page_offset();
	void set_dma_channel(int channel, bool state);

	u8 dma_read_byte(offs_t offset);
	void dma_write_byte(offs_t offset, u8 data);
	u8 dma_read_word(offs_t offset);
	void dma_write_word(offs_t offset, u8 data);
	void dma1_eop_w(int state);
	u8 dma1_ior0_r() { return m_read_ior(0); }
	u8 dma1_ior1_r() { return m_read_ior(1); }
	u8 dma1_ior2_r() { return m_read_ior(2); }
	u8 dma1_ior3_r() { return m_read_ior(3); }
	u8 dma2_ior1_r() { u16 const result = m_read_ior(5); m_dma_high_byte = result >> 8; return result; }
	u8 dma2_ior2_r() { u16 const result = m_read_ior(6); m_dma_high_byte = result >> 8; return result; }
	u8 dma2_ior3_r() { u16 const result = m_read_ior(7); m_dma_high_byte = result >> 8; return result; }
	void dma1_iow0_w(u8 data) { m_write_iow(0, data, 0xffff); }
	void dma1_iow1_w(u8 data) { m_write_iow(1, data, 0xffff); }
	void dma1_iow2_w(u8 data) { m_write_iow(2, data, 0xffff); }
	void dma1_iow3_w(u8 data) { m_write_iow(3, data, 0xffff); }
	void dma2_iow1_w(u8 data) { m_write_iow(5, (m_dma_high_byte << 8) | data, 0xffff); }
	void dma2_iow2_w(u8 data) { m_write_iow(6, (m_dma_high_byte << 8) | data, 0xffff); }
	void dma2_iow3_w(u8 data) { m_write_iow(7, (m_dma_high_byte << 8) | data, 0xffff); }
	void dma1_dack0_w(int state) { set_dma_channel(0, state); }
	void dma1_dack1_w(int state) { set_dma_channel(1, state); }
	void dma1_dack2_w(int state) { set_dma_channel(2, state); }
	void dma1_dack3_w(int state) { set_dma_channel(3, state); }
	void dma2_dack0_w(int state);
	void dma2_dack1_w(int state) { set_dma_channel(5, state); }
	void dma2_dack2_w(int state) { set_dma_channel(6, state); }
	void dma2_dack3_w(int state) { set_dma_channel(7, state); }
	void dma2_hreq_w(int state) { m_write_hold(state); }

	void trigger_nmi()
	{
		if (m_nmi_mask & BIT(m_portb, 6))
		{
			m_write_nmi(1);
			m_write_nmi(0);
		}
	}

//	void emulated_kbreset(int state);
//	void emulated_gatea20(int state);
};

DECLARE_DEVICE_TYPE(VL82C420, vl82c420_device)

#endif // MAME_MACHINE_VL82C420_H