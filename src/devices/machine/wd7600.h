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


#define MCFG_WD7600_ADD(_tag, _clock, _cputag, _isatag, _biostag, _keybctag) \
	MCFG_DEVICE_ADD(_tag, WD7600, _clock) \
	downcast<wd7600_device &>(*device).set_cputag(_cputag); \
	downcast<wd7600_device &>(*device).set_isatag(_isatag); \
	downcast<wd7600_device &>(*device).set_biostag(_biostag); \
	downcast<wd7600_device &>(*device).set_keybctag(_keybctag);

#define MCFG_WD7600_IOR(_ior) \
	devcb = &downcast<wd7600_device *>(device)->set_ior_callback(DEVCB_##_ior);

#define MCFG_WD7600_IOW(_iow) \
	devcb = &downcast<wd7600_device *>(device)->set_iow_callback(DEVCB_##_iow);

#define MCFG_WD7600_TC(_tc) \
	devcb = &downcast<wd7600_device *>(device)->set_tc_callback(DEVCB_##_tc);

#define MCFG_WD7600_HOLD(_hold) \
	devcb = &downcast<wd7600_device *>(device)->set_hold_callback(DEVCB_##_hold);

#define MCFG_WD7600_NMI(_nmi) \
	devcb = &downcast<wd7600_device *>(device)->set_nmi_callback(DEVCB_##_nmi);

#define MCFG_WD7600_INTR(_intr) \
	devcb = &downcast<wd7600_device *>(device)->set_intr_callback(DEVCB_##_intr);

#define MCFG_WD7600_CPURESET(_cpureset) \
	devcb = &downcast<wd7600_device *>(device)->set_cpureset_callback(DEVCB_##_cpureset);

#define MCFG_WD7600_A20M(_a20m) \
	devcb = &downcast<wd7600_device *>(device)->set_a20m_callback(DEVCB_##_a20m);

#define MCFG_WD7600_SPKR(_spkr) \
	devcb = &downcast<wd7600_device *>(device)->set_spkr_callback(DEVCB_##_spkr);


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
	template <class Object> devcb_base &set_ior_callback(Object &&ior) { return m_read_ior.set_callback(std::forward<Object>(ior)); }
	template <class Object> devcb_base &set_iow_callback(Object &&iow) { return m_write_iow.set_callback(std::forward<Object>(iow)); }
	template <class Object> devcb_base &set_tc_callback(Object &&tc) { return m_write_tc.set_callback(std::forward<Object>(tc)); }
	template <class Object> devcb_base &set_hold_callback(Object &&hold) { return m_write_hold.set_callback(std::forward<Object>(hold)); }
	template <class Object> devcb_base &set_cpureset_callback(Object &&cpureset) { return m_write_cpureset.set_callback(std::forward<Object>(cpureset)); }
	template <class Object> devcb_base &set_nmi_callback(Object &&nmi) { return m_write_nmi.set_callback(std::forward<Object>(nmi)); }
	template <class Object> devcb_base &set_intr_callback(Object &&intr) { return m_write_intr.set_callback(std::forward<Object>(intr)); }
	template <class Object> devcb_base &set_a20m_callback(Object &&a20m) { return m_write_a20m.set_callback(std::forward<Object>(a20m)); }
	template <class Object> devcb_base &set_spkr_callback(Object &&spkr) { return m_write_spkr.set_callback(std::forward<Object>(spkr)); }

	// inline configuration
	void set_cputag(const char *tag) { m_cputag = tag; }
	void set_isatag(const char *tag) { m_isatag = tag; }
	void set_biostag(const char *tag) { m_biostag = tag; }
	void set_keybctag(const char *tag) { m_keybctag = tag; }

	// input lines
	DECLARE_WRITE_LINE_MEMBER( irq01_w ) { m_pic1->ir1_w(state); }
	DECLARE_WRITE_LINE_MEMBER( irq03_w ) { m_pic1->ir3_w(state); }
	DECLARE_WRITE_LINE_MEMBER( irq04_w ) { m_pic1->ir4_w(state); }
	DECLARE_WRITE_LINE_MEMBER( irq05_w ) { m_pic1->ir5_w(state); }
	DECLARE_WRITE_LINE_MEMBER( irq06_w ) { m_pic1->ir6_w(state); }
	DECLARE_WRITE_LINE_MEMBER( irq07_w ) { m_pic1->ir7_w(state); }
	DECLARE_WRITE_LINE_MEMBER( irq09_w ) { m_pic2->ir1_w(state); }
	DECLARE_WRITE_LINE_MEMBER( irq10_w ) { m_pic2->ir2_w(state); }
	DECLARE_WRITE_LINE_MEMBER( irq11_w ) { m_pic2->ir3_w(state); }
	DECLARE_WRITE_LINE_MEMBER( irq12_w ) { m_pic2->ir4_w(state); }
	DECLARE_WRITE_LINE_MEMBER( irq13_w ) { m_pic2->ir5_w(state); } // also FERR#
	DECLARE_WRITE_LINE_MEMBER( irq14_w ) { m_pic2->ir6_w(state); }
	DECLARE_WRITE_LINE_MEMBER( irq15_w ) { m_pic2->ir7_w(state); }
	DECLARE_WRITE_LINE_MEMBER( dreq0_w ) { m_dma1->dreq0_w(state); }
	DECLARE_WRITE_LINE_MEMBER( dreq1_w ) { m_dma1->dreq1_w(state); }
	DECLARE_WRITE_LINE_MEMBER( dreq2_w ) { m_dma1->dreq2_w(state); }
	DECLARE_WRITE_LINE_MEMBER( dreq3_w ) { m_dma1->dreq3_w(state); }
	DECLARE_WRITE_LINE_MEMBER( dreq5_w ) { m_dma2->dreq1_w(state); }
	DECLARE_WRITE_LINE_MEMBER( dreq6_w ) { m_dma2->dreq2_w(state); }
	DECLARE_WRITE_LINE_MEMBER( dreq7_w ) { m_dma2->dreq3_w(state); }
	DECLARE_WRITE_LINE_MEMBER( hlda_w ) { m_dma2->hack_w(state); }
	DECLARE_WRITE_LINE_MEMBER( iochck_w );
	DECLARE_WRITE_LINE_MEMBER( gatea20_w );
	DECLARE_WRITE_LINE_MEMBER( kbrst_w );

	DECLARE_READ16_MEMBER(refresh_r);
	DECLARE_WRITE16_MEMBER(refresh_w);
	DECLARE_READ16_MEMBER(chipsel_r);
	DECLARE_WRITE16_MEMBER(chipsel_w);
	DECLARE_READ16_MEMBER(mem_ctrl_r);
	DECLARE_WRITE16_MEMBER(mem_ctrl_w);
	DECLARE_READ16_MEMBER(bank_01_start_r);
	DECLARE_WRITE16_MEMBER(bank_01_start_w);
	DECLARE_READ16_MEMBER(bank_23_start_r);
	DECLARE_WRITE16_MEMBER(bank_23_start_w);
	DECLARE_READ16_MEMBER(split_addr_r);
	DECLARE_WRITE16_MEMBER(split_addr_w);
	DECLARE_READ16_MEMBER(diag_r);
	DECLARE_WRITE16_MEMBER(diag_w);

	IRQ_CALLBACK_MEMBER(intack_cb) { return m_pic1->acknowledge(); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

private:
	DECLARE_WRITE_LINE_MEMBER(rtc_irq_w);
	DECLARE_WRITE_LINE_MEMBER( pic1_int_w ) { m_write_intr(state); }
	DECLARE_READ8_MEMBER( pic1_slave_ack_r );
	DECLARE_WRITE_LINE_MEMBER( ctc_out1_w );
	DECLARE_WRITE_LINE_MEMBER( ctc_out2_w );
	DECLARE_WRITE8_MEMBER( rtc_w );
	DECLARE_WRITE8_MEMBER( keyb_cmd_w );
	DECLARE_WRITE8_MEMBER( keyb_data_w );
	DECLARE_READ8_MEMBER( keyb_data_r );
	DECLARE_READ8_MEMBER( keyb_status_r );
	DECLARE_WRITE8_MEMBER( a20_reset_w );
	DECLARE_READ8_MEMBER( a20_reset_r );
	DECLARE_READ8_MEMBER( portb_r );
	DECLARE_WRITE8_MEMBER( portb_w );
	DECLARE_WRITE8_MEMBER( dma_page_w ) { m_dma_page[offset & 0x0f] = data; }
	DECLARE_READ8_MEMBER( dma_page_r ) { return m_dma_page[offset & 0x0f]; }
	DECLARE_READ8_MEMBER( dma_read_byte );
	DECLARE_WRITE8_MEMBER( dma_write_byte );
	DECLARE_READ8_MEMBER( dma_read_word );
	DECLARE_WRITE8_MEMBER( dma_write_word );
	DECLARE_WRITE_LINE_MEMBER( dma1_eop_w );
	DECLARE_READ8_MEMBER( dma1_ior0_r ) { return m_read_ior(0); }
	DECLARE_READ8_MEMBER( dma1_ior1_r ) { return m_read_ior(1); }
	DECLARE_READ8_MEMBER( dma1_ior2_r ) { return m_read_ior(2); }
	DECLARE_READ8_MEMBER( dma1_ior3_r ) { return m_read_ior(3); }
	DECLARE_READ8_MEMBER( dma2_ior1_r ) { uint16_t result = m_read_ior(5); m_dma_high_byte = result >> 8; return result; }
	DECLARE_READ8_MEMBER( dma2_ior2_r ) { uint16_t result = m_read_ior(6); m_dma_high_byte = result >> 8; return result; }
	DECLARE_READ8_MEMBER( dma2_ior3_r ) { uint16_t result = m_read_ior(7); m_dma_high_byte = result >> 8; return result; }
	DECLARE_WRITE8_MEMBER( dma1_iow0_w ) { m_write_iow(0, data, 0xffff); }
	DECLARE_WRITE8_MEMBER( dma1_iow1_w ) { m_write_iow(1, data, 0xffff); }
	DECLARE_WRITE8_MEMBER( dma1_iow2_w ) { m_write_iow(2, data, 0xffff); }
	DECLARE_WRITE8_MEMBER( dma1_iow3_w ) { m_write_iow(3, data, 0xffff); }
	DECLARE_WRITE8_MEMBER( dma2_iow1_w ) { m_write_iow(5, (m_dma_high_byte << 8) | data, 0xffff); }
	DECLARE_WRITE8_MEMBER( dma2_iow2_w ) { m_write_iow(6, (m_dma_high_byte << 8) | data, 0xffff); }
	DECLARE_WRITE8_MEMBER( dma2_iow3_w ) { m_write_iow(7, (m_dma_high_byte << 8) | data, 0xffff); }
	DECLARE_WRITE_LINE_MEMBER( dma1_dack0_w ) { set_dma_channel(0, state); }
	DECLARE_WRITE_LINE_MEMBER( dma1_dack1_w ) { set_dma_channel(1, state); }
	DECLARE_WRITE_LINE_MEMBER( dma1_dack2_w ) { set_dma_channel(2, state); }
	DECLARE_WRITE_LINE_MEMBER( dma1_dack3_w ) { set_dma_channel(3, state); }
	DECLARE_WRITE_LINE_MEMBER( dma2_dack0_w );
	DECLARE_WRITE_LINE_MEMBER( dma2_dack1_w ) { set_dma_channel(5, state); }
	DECLARE_WRITE_LINE_MEMBER( dma2_dack2_w ) { set_dma_channel(6, state); }
	DECLARE_WRITE_LINE_MEMBER( dma2_dack3_w ) { set_dma_channel(7, state); }
	DECLARE_WRITE_LINE_MEMBER( dma2_hreq_w ) { m_write_hold(state); }

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

	offs_t page_offset();
	void set_dma_channel(int channel, bool state);
	void keyboard_gatea20(int state);
	void nmi();
	void a20m();

	// internal state
	const char *m_cputag;
	const char *m_isatag;
	const char *m_biostag;
	const char *m_keybctag;
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
	uint8_t *m_isa;
	uint8_t *m_bios;
	uint8_t *m_ram;
	at_keyboard_controller_device *m_keybc;
};

// device type definition
DECLARE_DEVICE_TYPE(WD7600, wd7600_device)

#endif // MAME_MACHINE_WD7600_H
