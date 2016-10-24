// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Chips & Technologies CS4031 chipset

    Chipset for 486 based PC/AT compatible systems. Consists of two
    individual chips:

    * F84031
        - DRAM controller
        - ISA-bus controller
        - VESA VL-BUS controller

    * F84035 (82C206 IPC core)
        - 2x 8257 DMA controller
        - 2x 8259 interrupt controller
        - 8254 timer
        - MC14818 RTC

***************************************************************************/

#pragma once

#ifndef __CS4031_H__
#define __CS4031_H__

#include "emu.h"
#include "machine/am9517a.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ds128x.h"
#include "machine/at_keybc.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CS4031_ADD(_tag, _clock, _cputag, _isatag, _biostag, _keybctag) \
	MCFG_DEVICE_ADD(_tag, CS4031, _clock) \
	cs4031_device::static_set_cputag(*device, _cputag); \
	cs4031_device::static_set_isatag(*device, _isatag); \
	cs4031_device::static_set_biostag(*device, _biostag); \
	cs4031_device::static_set_keybctag(*device, _keybctag);

#define MCFG_CS4031_IOR(_ior) \
	downcast<cs4031_device *>(device)->set_ior_callback(DEVCB_##_ior);

#define MCFG_CS4031_IOW(_iow) \
	downcast<cs4031_device *>(device)->set_iow_callback(DEVCB_##_iow);

#define MCFG_CS4031_TC(_tc) \
	downcast<cs4031_device *>(device)->set_tc_callback(DEVCB_##_tc);

#define MCFG_CS4031_HOLD(_hold) \
	downcast<cs4031_device *>(device)->set_hold_callback(DEVCB_##_hold);

#define MCFG_CS4031_NMI(_nmi) \
	downcast<cs4031_device *>(device)->set_nmi_callback(DEVCB_##_nmi);

#define MCFG_CS4031_INTR(_intr) \
	downcast<cs4031_device *>(device)->set_intr_callback(DEVCB_##_intr);

#define MCFG_CS4031_CPURESET(_cpureset) \
	downcast<cs4031_device *>(device)->set_cpureset_callback(DEVCB_##_cpureset);

#define MCFG_CS4031_A20M(_a20m) \
	downcast<cs4031_device *>(device)->set_a20m_callback(DEVCB_##_a20m);

#define MCFG_CS4031_SPKR(_spkr) \
	downcast<cs4031_device *>(device)->set_spkr_callback(DEVCB_##_spkr);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cs4031_device

class cs4031_device : public device_t
{
public:
	// construction/destruction
	cs4031_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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

	// not really public
	uint8_t dma_read_byte(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dma_write_byte(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dma_read_word(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dma_write_word(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dma1_eop_w(int state);
	uint8_t dma1_ior0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { return m_read_ior(0); }
	uint8_t dma1_ior1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { return m_read_ior(1); }
	uint8_t dma1_ior2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { return m_read_ior(2); }
	uint8_t dma1_ior3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { return m_read_ior(3); }
	uint8_t dma2_ior1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { uint16_t result = m_read_ior(5); m_dma_high_byte = result >> 8; return result; }
	uint8_t dma2_ior2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { uint16_t result = m_read_ior(6); m_dma_high_byte = result >> 8; return result; }
	uint8_t dma2_ior3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { uint16_t result = m_read_ior(7); m_dma_high_byte = result >> 8; return result; }
	void dma1_iow0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { m_write_iow(0, data, 0xffff); }
	void dma1_iow1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { m_write_iow(1, data, 0xffff); }
	void dma1_iow2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { m_write_iow(2, data, 0xffff); }
	void dma1_iow3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { m_write_iow(3, data, 0xffff); }
	void dma2_iow1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { m_write_iow(5, (m_dma_high_byte << 8) | data, 0xffff); }
	void dma2_iow2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { m_write_iow(6, (m_dma_high_byte << 8) | data, 0xffff); }
	void dma2_iow3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { m_write_iow(7, (m_dma_high_byte << 8) | data, 0xffff); }
	void dma1_dack0_w(int state) { set_dma_channel(0, state); }
	void dma1_dack1_w(int state) { set_dma_channel(1, state); }
	void dma1_dack2_w(int state) { set_dma_channel(2, state); }
	void dma1_dack3_w(int state) { set_dma_channel(3, state); }
	void dma2_dack0_w(int state);
	void dma2_dack1_w(int state) { set_dma_channel(5, state); }
	void dma2_dack2_w(int state) { set_dma_channel(6, state); }
	void dma2_dack3_w(int state) { set_dma_channel(7, state); }
	void dma2_hreq_w(int state) { m_write_hold(state); }
	void intc1_int_w(int state) { m_write_intr(state); }
	uint8_t intc1_slave_ack_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ctc_out1_w(int state);
	void ctc_out2_w(int state);
	void rtc_irq_w(int state);

	// internal io
	void config_address_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t config_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void config_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t portb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rtc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sysctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sysctrl_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dma_page_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { return m_dma_page[offset]; }
	void dma_page_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { m_dma_page[offset] = data; }
	uint8_t dma2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { return m_dma2->read(space, offset / 2); }
	void dma2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { m_dma2->write(space, offset / 2, data); }
	uint8_t keyb_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void keyb_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t keyb_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void keyb_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void keyb_command_blocked_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// input lines
	void irq01_w(int state) { m_intc1->ir1_w(state); }
	void irq03_w(int state) { m_intc1->ir3_w(state); }
	void irq04_w(int state) { m_intc1->ir4_w(state); }
	void irq05_w(int state) { m_intc1->ir5_w(state); }
	void irq06_w(int state) { m_intc1->ir6_w(state); }
	void irq07_w(int state) { m_intc1->ir7_w(state); }
	void irq09_w(int state) { m_intc2->ir1_w(state); }
	void irq10_w(int state) { m_intc2->ir2_w(state); }
	void irq11_w(int state) { m_intc2->ir3_w(state); }
	void irq12_w(int state) { m_intc2->ir4_w(state); }
	void irq13_w(int state) { m_intc2->ir5_w(state); } // also FERR#
	void irq14_w(int state) { m_intc2->ir6_w(state); }
	void irq15_w(int state) { m_intc2->ir7_w(state); }
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

	int int_ack_r(device_t &device, int irqline) { return m_intc1->acknowledge(); }

	// inline configuration
	static void static_set_cputag(device_t &device, const char *tag);
	static void static_set_isatag(device_t &device, const char *tag);
	static void static_set_biostag(device_t &device, const char *tag);
	static void static_set_keybctag(device_t &device, const char *tag);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_reset_after_children() override;

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

	offs_t page_offset();
	void set_dma_channel(int channel, bool state);
	void update_dma_clock();

	void nmi();
	void a20m();

	void emulated_kbreset(int state);
	void emulated_gatea20(int state);
	void fast_gatea20(int state);
	void keyboard_gatea20(int state);

	void update_read_region(int index, const char *region, offs_t start, offs_t end);
	void update_write_region(int index, const char *region, offs_t start, offs_t end);
	void update_read_regions();
	void update_write_regions();

	// internal state
	const char *m_cputag;
	const char *m_isatag;
	const char *m_biostag;
	const char *m_keybctag;

	address_space *m_space;
	address_space *m_space_io;
	uint8_t *m_isa;
	uint8_t *m_bios;
	uint8_t *m_ram;

	// ipc core devices
	required_device<am9517a_device> m_dma1;
	required_device<am9517a_device> m_dma2;
	required_device<pic8259_device> m_intc1;
	required_device<pic8259_device> m_intc2;
	required_device<pit8254_device> m_ctc;
	required_device<ds12885_device> m_rtc;

	int m_dma_eop;
	uint8_t m_dma_page[0x10];
	uint8_t m_dma_high_byte;
	int m_dma_channel;

	uint8_t m_portb;
	int m_refresh_toggle;
	int m_iochck;
	int m_nmi_mask;

	// keyboard
	at_keyboard_controller_device *m_keybc;
	int m_cpureset;
	int m_kbrst;
	int m_ext_gatea20;
	int m_fast_gatea20;
	int m_emu_gatea20;
	bool m_keybc_d1_written;
	bool m_keybc_data_blocked;

	// chipset configuration
	static const char* m_register_names[];
	static const float m_dma_clock_divider[];

	enum
	{
		DMA_WAIT_STATE = 0x01,
		PERFORMANCE = 0x08,
		F84035_MISC = 0x09,
		DMA_CLOCK = 0x0a,
		SHADOW_READ = 0x19,
		SHADOW_WRITE = 0x1a,
		ROMCS = 0x1b,
		SOFT_RESET_AND_GATEA20 = 0x1c
	};

	uint8_t m_address;
	bool m_address_valid;

	uint8_t m_registers[0x20];
};


// device type definition
extern const device_type CS4031;


#endif  /* __CS4031_H__ */
