// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#pragma once

#ifndef INTERPRO_IOGA_H_
#define INTERPRO_IOGA_H_

#include "emu.h"
#include "machine/upd765.h"

#define MCFG_INTERPRO_IOGA_ADD(_tag, _out_int) \
	MCFG_DEVICE_ADD(_tag, INTERPRO_IOGA, 0) \
	devcb = &interpro_ioga_device::static_set_out_int_callback( *device, DEVCB_##_out_int );

class interpro_ioga_device : public device_t
{
public:
	// construction/destruction
	interpro_ioga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &static_set_out_int_callback(device_t &device, _Object object) { return downcast<interpro_ioga_device &>(device).m_out_int_func.set_callback(object); }

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
	DECLARE_WRITE_LINE_MEMBER(ir9_w) { set_irq_line(11, state); }
	DECLARE_WRITE_LINE_MEMBER(ir10_w) { set_irq_line(12, state); }
	DECLARE_WRITE_LINE_MEMBER(ir11_w) { set_irq_line(17, state); }
	DECLARE_WRITE_LINE_MEMBER(ir12_w) { set_irq_line(18, state); }

	IRQ_CALLBACK_MEMBER(inta_cb);

	DECLARE_WRITE_LINE_MEMBER(drq);

	DECLARE_READ32_MEMBER(read);
	DECLARE_WRITE32_MEMBER(write);

	DECLARE_READ32_MEMBER(timer0_r) { return m_timer[0]; };
	DECLARE_READ32_MEMBER(timer1_r) { return m_timer[1]; };
	DECLARE_READ32_MEMBER(timer3_r) { return m_timer[3]; };

	DECLARE_WRITE32_MEMBER(timer0_w) { set_timer(0, data, IOGA_TIMER_0); }
	DECLARE_WRITE32_MEMBER(timer1_w) { set_timer(1, data, IOGA_TIMER_1); }
	DECLARE_WRITE32_MEMBER(timer3_w) { set_timer(3, data, IOGA_TIMER_3); }

	DECLARE_READ16_MEMBER(icr_r);
	DECLARE_WRITE16_MEMBER(icr_w);

	DECLARE_READ32_MEMBER(fdc_dma_r) { return m_fdc_dma[offset]; };
	DECLARE_WRITE32_MEMBER(fdc_dma_w) { m_fdc_dma[offset] = data; };

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
	void set_timer(int timer, uint32_t value, device_timer_id id);

	devcb_write_line m_out_int_func;

	// a hack to get hold of the dma devices
	upd765_family_device *m_fdc;

	uint32_t m_irq_lines;
	uint8_t m_interrupt;
	uint16_t m_vectors[19];

	uint32_t m_timer[4];

	emu_timer *m_dma_timer;
	uint32_t m_state_drq;
	uint32_t m_fdc_dma[4];
};

// device type definition
extern const device_type INTERPRO_IOGA;

#endif