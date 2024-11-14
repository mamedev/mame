// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * wd7600.h
 *
 *  Created on: 5/05/2014
 */

#ifndef MAME_MACHINE_WD7600_H
#define MAME_MACHINE_WD7600_H

#pragma once

#include "machine/am9517a.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ds128x.h"
#include "machine/at_keybc.h"
#include "machine/ram.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wd7600_device

class wd7600_device : public device_t
{
public:
	// construction/destruction
	wd7600_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// callbacks
	auto ior_callback() { return m_read_ior.bind(); }
	auto iow_callback() { return m_write_iow.bind(); }
	auto tc_callback() { return m_write_tc.bind(); }
	auto hold_callback() { return m_write_hold.bind(); }
	auto cpureset_callback() { return m_write_cpureset.bind(); }
	auto nmi_callback() { return m_write_nmi.bind(); }
	auto intr_callback() { return m_write_intr.bind(); }
	auto a20m_callback() { return m_write_a20m.bind(); }
	auto spkr_callback() { return m_write_spkr.bind(); }

	// inline configuration
	template <typename T> void set_cputag(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_isatag(T &&tag) { m_isa.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_biostag(T &&tag) { m_bios.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_keybctag(T &&tag) { m_keybc.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_ramtag(T &&tag) { m_ram.set_tag(std::forward<T>(tag)); }

	// input lines
	void irq01_w(int state) { m_pic1->ir1_w(state); }
	void irq03_w(int state) { m_pic1->ir3_w(state); }
	void irq04_w(int state) { m_pic1->ir4_w(state); }
	void irq05_w(int state) { m_pic1->ir5_w(state); }
	void irq06_w(int state) { m_pic1->ir6_w(state); }
	void irq07_w(int state) { m_pic1->ir7_w(state); }
	void irq09_w(int state) { m_pic2->ir1_w(state); }
	void irq10_w(int state) { m_pic2->ir2_w(state); }
	void irq11_w(int state) { m_pic2->ir3_w(state); }
	void irq12_w(int state) { m_pic2->ir4_w(state); }
	void irq13_w(int state) { m_pic2->ir5_w(state); } // also FERR#
	void irq14_w(int state) { m_pic2->ir6_w(state); }
	void irq15_w(int state) { m_pic2->ir7_w(state); }
	void dreq0_w(int state) { m_dma1->dreq0_w(state); }
	void dreq1_w(int state) { m_dma1->dreq1_w(state); }
	void dreq2_w(int state) { m_dma1->dreq2_w(state); }
	void dreq3_w(int state) { m_dma1->dreq3_w(state); }
	void dreq5_w(int state) { m_dma2->dreq1_w(state); }
	void dreq6_w(int state) { m_dma2->dreq2_w(state); }
	void dreq7_w(int state) { m_dma2->dreq3_w(state); }
	void hlda_w(int state) { m_dma2->hack_w(state); }
	void iochck_w(int state);
	void gatea20_w(int state);
	void kbrst_w(int state);

	uint16_t refresh_r();
	void refresh_w(uint16_t data);
	uint16_t chipsel_r();
	void chipsel_w(uint16_t data);
	uint16_t mem_ctrl_r();
	void mem_ctrl_w(uint16_t data);
	uint16_t bank_01_start_r(offs_t offset, uint16_t mem_mask);
	void bank_01_start_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t bank_23_start_r(offs_t offset, uint16_t mem_mask);
	void bank_23_start_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t split_addr_r();
	void split_addr_w(uint16_t data);
	uint16_t diag_r();
	void diag_w(uint16_t data);

	IRQ_CALLBACK_MEMBER(intack_cb) { return m_pic1->acknowledge(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void pic1_int_w(int state) { m_write_intr(state); }
	uint8_t pic1_slave_ack_r(offs_t offset);
	void ctc_out1_w(int state);
	void ctc_out2_w(int state);
	void rtc_nmi_w(uint8_t data);
	void keyb_cmd_w(uint8_t data);
	void keyb_data_w(uint8_t data);
	uint8_t keyb_data_r();
	uint8_t keyb_status_r();
	void a20_reset_w(uint8_t data);
	uint8_t a20_reset_r();
	uint8_t portb_r();
	void portb_w(uint8_t data);
	void dma_page_w(offs_t offset, uint8_t data) { m_dma_page[offset & 0x0f] = data; }
	uint8_t dma_page_r(offs_t offset) { return m_dma_page[offset & 0x0f]; }
	uint8_t dma_read_byte(offs_t offset);
	void dma_write_byte(offs_t offset, uint8_t data);
	uint8_t dma_read_word(offs_t offset);
	void dma_write_word(offs_t offset, uint8_t data);
	void dma1_eop_w(int state);
	uint8_t dma1_ior0_r() { return m_read_ior(0); }
	uint8_t dma1_ior1_r() { return m_read_ior(1); }
	uint8_t dma1_ior2_r() { return m_read_ior(2); }
	uint8_t dma1_ior3_r() { return m_read_ior(3); }
	uint8_t dma2_ior1_r() { uint16_t result = m_read_ior(5); m_dma_high_byte = result >> 8; return result; }
	uint8_t dma2_ior2_r() { uint16_t result = m_read_ior(6); m_dma_high_byte = result >> 8; return result; }
	uint8_t dma2_ior3_r() { uint16_t result = m_read_ior(7); m_dma_high_byte = result >> 8; return result; }
	void dma1_iow0_w(uint8_t data) { m_write_iow(0, data, 0xffff); }
	void dma1_iow1_w(uint8_t data) { m_write_iow(1, data, 0xffff); }
	void dma1_iow2_w(uint8_t data) { m_write_iow(2, data, 0xffff); }
	void dma1_iow3_w(uint8_t data) { m_write_iow(3, data, 0xffff); }
	void dma2_iow1_w(uint8_t data) { m_write_iow(5, (m_dma_high_byte << 8) | data, 0xffff); }
	void dma2_iow2_w(uint8_t data) { m_write_iow(6, (m_dma_high_byte << 8) | data, 0xffff); }
	void dma2_iow3_w(uint8_t data) { m_write_iow(7, (m_dma_high_byte << 8) | data, 0xffff); }
	void dma1_dack0_w(int state) { set_dma_channel(0, state); }
	void dma1_dack1_w(int state) { set_dma_channel(1, state); }
	void dma1_dack2_w(int state) { set_dma_channel(2, state); }
	void dma1_dack3_w(int state) { set_dma_channel(3, state); }
	void dma2_dack0_w(int state);
	void dma2_dack1_w(int state) { set_dma_channel(5, state); }
	void dma2_dack2_w(int state) { set_dma_channel(6, state); }
	void dma2_dack3_w(int state) { set_dma_channel(7, state); }
	void dma2_hreq_w(int state) { m_write_hold(state); }

	devcb_read16 m_read_ior;
	devcb_write16 m_write_iow;
	devcb_write8 m_write_tc;
	devcb_write_line m_write_hold;
	devcb_write_line m_write_nmi;
	devcb_write_line m_write_intr;
	devcb_write_line m_write_cpureset;
	devcb_write_line m_write_a20m;
	devcb_write_line m_write_spkr;

	required_device<am9517a_device> m_dma1;
	required_device<am9517a_device> m_dma2;
	required_device<pic8259_device> m_pic1;
	required_device<pic8259_device> m_pic2;
	required_device<pit8254_device> m_ctc;
	required_device<ds12885_device> m_rtc;

	required_device<device_memory_interface> m_cpu;
	required_device<at_keyboard_controller_device> m_keybc;
	required_device<ram_device> m_ram;
	required_region_ptr<uint8_t> m_bios;
	required_region_ptr<uint8_t> m_isa;

	offs_t page_offset();
	void set_dma_channel(int channel, bool state);
	void keyboard_gatea20(int state);
	void nmi();
	void a20m();

	// internal state
	uint8_t m_portb;
	int m_iochck;
	int m_nmi_mask;
	int m_alt_a20;
	int m_ext_gatea20;
	int m_kbrst;
	int m_refresh_toggle;
	uint16_t m_refresh_ctrl;
	uint16_t m_memory_ctrl;
	uint16_t m_chip_sel;
	uint16_t m_split_start;
	uint8_t m_bank_start[4];
	uint16_t m_diagnostic;

	int m_dma_eop;
	uint8_t m_dma_page[0x10];
	uint8_t m_dma_high_byte;
	int m_dma_channel;

	address_space *m_space;
	address_space *m_space_io;
};

// device type definition
DECLARE_DEVICE_TYPE(WD7600, wd7600_device)

#endif // MAME_MACHINE_WD7600_H
