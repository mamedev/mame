// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * wd7600.h
 *
 *  Created on: 5/05/2014
 */

#ifndef WD7600_H_
#define WD7600_H_

#include "emu.h"
#include "machine/am9517a.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ds128x.h"
#include "machine/at_keybc.h"
#include "machine/ram.h"


#define MCFG_WD7600_ADD(_tag, _clock, _cputag, _isatag, _biostag, _keybctag) \
	MCFG_DEVICE_ADD(_tag, WD7600, _clock) \
	wd7600_device::static_set_cputag(*device, _cputag); \
	wd7600_device::static_set_isatag(*device, _isatag); \
	wd7600_device::static_set_biostag(*device, _biostag); \
	wd7600_device::static_set_keybctag(*device, _keybctag);

#define MCFG_WD7600_IOR(_ior) \
	downcast<wd7600_device *>(device)->set_ior_callback(DEVCB_##_ior);

#define MCFG_WD7600_IOW(_iow) \
	downcast<wd7600_device *>(device)->set_iow_callback(DEVCB_##_iow);

#define MCFG_WD7600_TC(_tc) \
	downcast<wd7600_device *>(device)->set_tc_callback(DEVCB_##_tc);

#define MCFG_WD7600_HOLD(_hold) \
	downcast<wd7600_device *>(device)->set_hold_callback(DEVCB_##_hold);

#define MCFG_WD7600_NMI(_nmi) \
	downcast<wd7600_device *>(device)->set_nmi_callback(DEVCB_##_nmi);

#define MCFG_WD7600_INTR(_intr) \
	downcast<wd7600_device *>(device)->set_intr_callback(DEVCB_##_intr);

#define MCFG_WD7600_CPURESET(_cpureset) \
	downcast<wd7600_device *>(device)->set_cpureset_callback(DEVCB_##_cpureset);

#define MCFG_WD7600_A20M(_a20m) \
	downcast<wd7600_device *>(device)->set_a20m_callback(DEVCB_##_a20m);

#define MCFG_WD7600_SPKR(_spkr) \
	downcast<wd7600_device *>(device)->set_spkr_callback(DEVCB_##_spkr);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wd7600_device

class wd7600_device : public device_t
{
public:
	// construction/destruction
	wd7600_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	// callbacks
	template<class _ior> void set_ior_callback(_ior ior) { m_read_ior.set_callback(ior); }
	template<class _iow> void set_iow_callback(_iow iow) { m_write_iow.set_callback(iow); }
	template<class _tc> void set_tc_callback(_tc tc) { m_write_tc.set_callback(tc); }
	template<class _hold> void set_hold_callback(_hold hold) { m_write_hold.set_callback(hold); }
	template<class _cpureset> void set_cpureset_callback(_cpureset cpureset) { m_write_cpureset.set_callback(cpureset); }
	template<class _nmi> void set_nmi_callback(_nmi nmi) { m_write_nmi.set_callback(nmi); }
	template<class _intr> void set_intr_callback(_intr intr) { m_write_intr.set_callback(intr); }
	template<class _a20m> void set_a20m_callback(_a20m a20m) { m_write_a20m.set_callback(a20m); }
	template<class _spkr> void set_spkr_callback(_spkr spkr) { m_write_spkr.set_callback(spkr); }

	// inline configuration
	static void static_set_cputag(device_t &device, std::string tag);
	static void static_set_isatag(device_t &device, std::string tag);
	static void static_set_biostag(device_t &device, std::string tag);
	static void static_set_keybctag(device_t &device, std::string tag);

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
	DECLARE_READ8_MEMBER( dma2_ior1_r ) { UINT16 result = m_read_ior(5); m_dma_high_byte = result >> 8; return result; }
	DECLARE_READ8_MEMBER( dma2_ior2_r ) { UINT16 result = m_read_ior(6); m_dma_high_byte = result >> 8; return result; }
	DECLARE_READ8_MEMBER( dma2_ior3_r ) { UINT16 result = m_read_ior(7); m_dma_high_byte = result >> 8; return result; }
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

private:
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
	std::string m_cputag;
	std::string m_isatag;
	std::string m_biostag;
	std::string m_keybctag;
	UINT8 m_portb;
	int m_iochck;
	int m_nmi_mask;
	int m_alt_a20;
	int m_ext_gatea20;
	int m_kbrst;
	int m_refresh_toggle;
	UINT16 m_refresh_ctrl;
	UINT16 m_memory_ctrl;
	UINT16 m_chip_sel;
	UINT16 m_split_start;
	UINT8 m_bank_start[4];
	UINT16 m_diagnostic;

	int m_dma_eop;
	UINT8 m_dma_page[0x10];
	UINT8 m_dma_high_byte;
	int m_dma_channel;

	address_space *m_space;
	address_space *m_space_io;
	UINT8 *m_isa;
	UINT8 *m_bios;
	UINT8 *m_ram;
	at_keyboard_controller_device *m_keybc;
};

// device type definition
extern const device_type WD7600;

#endif /* WD7600_H_ */
