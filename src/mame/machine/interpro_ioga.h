// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#pragma once

#ifndef INTERPRO_IOGA_H_
#define INTERPRO_IOGA_H_

#include "emu.h"

#define MCFG_INTERPRO_IOGA_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, INTERPRO_IOGA, 0)

#define MCFG_INTERPRO_IOGA_NMI_CB(_out_nmi) \
	devcb = &interpro_ioga_device::static_set_out_int_callback(*device, DEVCB_##_out_nmi);

#define MCFG_INTERPRO_IOGA_IRQ_CB(_out_int) \
	devcb = &interpro_ioga_device::static_set_out_int_callback(*device, DEVCB_##_out_int);

#define MCFG_INTERPRO_IOGA_DMA_CB(_channel, _dma_r, _dma_w) \
	devcb = &interpro_ioga_device::static_set_dma_r_callback(*device, _channel, DEVCB_##_dma_r); \
	devcb = &interpro_ioga_device::static_set_dma_w_callback(*device, _channel, DEVCB_##_dma_w);

// timer 0 seem to be a 60Hz cycle
#define IOGA_TIMER0_IRQ     14

// best guess for timer 1 is 10MHz based on typical prescaler value of 1000 and timer value of 100 for a delay of 100ms
#define IOGA_TIMER1_IRQ     15
#define IOGA_TIMER1_VMASK   0xffff
#define IOGA_TIMER1_START   0x10000
#define IOGA_TIMER1_EXPIRED 0x20000

// best guess for timer 3 is 12.5MHz based on typical value of 12500 for a delay of 1ms
#define IOGA_TIMER3_CLOCK	XTAL_12_5MHz
#define IOGA_TIMER3_IRQ     1
#define IOGA_TIMER3_VMASK   0x3fffffff
#define IOGA_TIMER3_START	0x40000000
#define IOGA_TIMER3_EXPIRED 0x80000000

#define IOGA_INTERRUPT_PENDING         0x0100
#define IOGA_INTERRUPT_ENABLE_EXTERNAL 0x0200
#define IOGA_INTERRUPT_EDGE            0x0400
#define IOGA_INTERRUPT_NEGPOL          0x0800
#define IOGA_INTERRUPT_ENABLE_INTERNAL 0x1000
// FIXME: hack for forced interrupts
#define IOGA_INTERRUPT_FORCED          0x8000

#define IOGA_DMA_CHANNELS 4
#define IOGA_DMA_CHANNEL_PLOTTER 0
#define IOGA_DMA_CHANNEL_SCSI    1
#define IOGA_DMA_CHANNEL_FLOPPY  2
#define IOGA_DMA_CHANNEL_SERIAL  3

class interpro_ioga_device : public device_t
{
public:
	// construction/destruction
	interpro_ioga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &static_set_out_nmi_callback(device_t &device, _Object object) { return downcast<interpro_ioga_device &>(device).m_out_nmi_func.set_callback(object); }
	template<class _Object> static devcb_base &static_set_out_int_callback(device_t &device, _Object object) { return downcast<interpro_ioga_device &>(device).m_out_int_func.set_callback(object); }

	template<class _Object> static devcb_base &static_set_dma_r_callback(device_t &device, int channel, _Object object) { return downcast<interpro_ioga_device &>(device).m_dma_r_func[channel].set_callback(object); }
	template<class _Object> static devcb_base &static_set_dma_w_callback(device_t &device, int channel, _Object object) { return downcast<interpro_ioga_device &>(device).m_dma_w_func[channel].set_callback(object); }

	virtual DECLARE_ADDRESS_MAP(map, 8);

	// external interrupt lines
	DECLARE_WRITE_LINE_MEMBER(ir0_w) { set_irq_line(2, state); }
	DECLARE_WRITE_LINE_MEMBER(ir1_w) { set_irq_line(3, state); }
	DECLARE_WRITE_LINE_MEMBER(ir2_w) { set_irq_line(4, state); }
	DECLARE_WRITE_LINE_MEMBER(ir3_w) { set_irq_line(5, state); }
	DECLARE_WRITE_LINE_MEMBER(ir4_w) { set_irq_line(6, state); }
	DECLARE_WRITE_LINE_MEMBER(ir5_w) { set_irq_line(7, state); }
	DECLARE_WRITE_LINE_MEMBER(ir6_w) { set_irq_line(8, state); }
	DECLARE_WRITE_LINE_MEMBER(ir7_w) { set_irq_line(9, state); }
	DECLARE_WRITE_LINE_MEMBER(ir8_w) { set_irq_line(10, state); }
	// FIXME: this is a workaround for the mc146818 code which inverts the normal irq state convention
	DECLARE_WRITE_LINE_MEMBER(ir9_w) { set_irq_line(11, !state); }
	DECLARE_WRITE_LINE_MEMBER(ir10_w) { set_irq_line(12, state); }
	DECLARE_WRITE_LINE_MEMBER(ir11_w) { set_irq_line(17, state); }
	DECLARE_WRITE_LINE_MEMBER(ir12_w) { set_irq_line(18, state); }

	IRQ_CALLBACK_MEMBER(inta_cb);

	DECLARE_WRITE_LINE_MEMBER(drq);

	DECLARE_READ32_MEMBER(timer_prescaler_r) { return m_prescaler; }
	DECLARE_READ32_MEMBER(timer0_r) { return m_timer_reg[0]; }
	DECLARE_READ32_MEMBER(timer1_r) { return m_timer_reg[1]; }
	DECLARE_READ32_MEMBER(timer2_r) { return m_timer_reg[2]; }
	DECLARE_READ32_MEMBER(timer3_r) { return m_timer_reg[3]; }

	DECLARE_WRITE32_MEMBER(timer_prescaler_w) { m_prescaler = data; }
	DECLARE_WRITE32_MEMBER(timer0_w) { write_timer(0, data, IOGA_TIMER_0); }
	DECLARE_WRITE32_MEMBER(timer1_w) { write_timer(1, data, IOGA_TIMER_1); }
	DECLARE_WRITE32_MEMBER(timer2_w) { write_timer(2, data, IOGA_TIMER_2); }
	DECLARE_WRITE32_MEMBER(timer3_w) { write_timer(3, data, IOGA_TIMER_3); }

	DECLARE_READ16_MEMBER(icr_r);
	DECLARE_WRITE16_MEMBER(icr_w);
	DECLARE_READ16_MEMBER(icr18_r) { return icr_r(space, 18, mem_mask); }
	DECLARE_WRITE16_MEMBER(icr18_w) { icr_w(space, 18, data, mem_mask); }

	DECLARE_READ8_MEMBER(softint_r) { return m_softint; }
	DECLARE_WRITE8_MEMBER(softint_w);
	DECLARE_READ8_MEMBER(nmictrl_r) { return m_nmictrl; }
	DECLARE_WRITE8_MEMBER(nmictrl_w) { m_nmictrl = data; }
	
	DECLARE_READ32_MEMBER(fdc_dma_r) { return m_fdc_dma[offset]; }
	DECLARE_WRITE32_MEMBER(fdc_dma_w) { m_fdc_dma[offset] = data; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	cpu_device *m_cpu;

private:
	static const device_timer_id IOGA_TIMER_0 = 0;
	static const device_timer_id IOGA_TIMER_1 = 1;
	static const device_timer_id IOGA_TIMER_2 = 2;
	static const device_timer_id IOGA_TIMER_3 = 3;

	static const device_timer_id IOGA_TIMER_DMA = 4;

	void set_irq_line(int irq, int state);
	void write_timer(int timer, uint32_t value, device_timer_id id);

	void update_irq(int state);

	devcb_write_line m_out_nmi_func;
	devcb_write_line m_out_int_func;

	devcb_read8 m_dma_r_func[IOGA_DMA_CHANNELS];
	devcb_write8 m_dma_w_func[IOGA_DMA_CHANNELS];

	bool m_irq_active;
	uint32_t m_irq_current;

	uint16_t m_vectors[19];
	uint8_t m_softint;
	uint8_t m_nmictrl;

	uint32_t m_prescaler;
	uint32_t m_timer_reg[4];
	emu_timer *m_timer[4];

	emu_timer *m_dma_timer;
	uint32_t m_state_drq;
	uint32_t m_fdc_dma[4];
};

// device type definition
extern const device_type INTERPRO_IOGA;

#endif