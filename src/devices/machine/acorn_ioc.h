// license:LGPL-2.1+
// copyright-holders:Angelo Salese, R. Belmont, Juergen Buchmueller, Sandro Ronco
/**************************************************************************************************

        Acorn RISC Machine Input/Output Controller (IOC)

**************************************************************************************************/

#ifndef MAME_MACHINE_ACORN_IOC_H
#define MAME_MACHINE_ACORN_IOC_H

#pragma once

#include "diserial.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> acorn_ioc_device

class acorn_ioc_device : public device_t, public device_serial_interface
{
public:
	acorn_ioc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <unsigned N> auto peripheral_r() { static_assert(N >= 1 && N <= 7); return m_peripherals_r[N - 1].bind(); }
	template <unsigned N> auto peripheral_w() { static_assert(N >= 1 && N <= 7); return m_peripherals_w[N - 1].bind(); }
	template <unsigned N> auto gpio_r()       { static_assert(N <= 5);           return m_giop_r[N].bind();            }
	template <unsigned N> auto gpio_w()       { static_assert(N <= 5);           return m_giop_w[N].bind();            }
	auto irq_w()                              { return m_irq_w.bind();  }
	auto fiq_w()                              { return m_fiq_w.bind();  }
	auto baud_w()                             { return m_baud_w.bind(); }
	auto kout_w()                             { return m_kout_w.bind(); }

	void map(address_map &map) ATTR_COLD;

	uint32_t registers_r(offs_t offset, uint32_t mem_mask = ~0);
	void registers_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template<unsigned N> uint32_t periph_r(offs_t offset, uint32_t mem_mask = ~0)                { return m_peripherals_r[N - 1](offset, mem_mask); }
	template<unsigned N> void periph_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0)     { m_peripherals_w[N - 1](offset, data, mem_mask);  }

	void il0_w(int state)        { change_interrupt(IRQ_STATUS_B, 0x01, state); change_interrupt(FIQ_STATUS, 0x40, state); }
	void il1_w(int state)        { change_interrupt(IRQ_STATUS_B, 0x02, state); }
	void il2_w(int state)        { change_interrupt(IRQ_STATUS_B, 0x04, state); }
	void il3_w(int state)        { change_interrupt(IRQ_STATUS_B, 0x08, state); }
	void il4_w(int state)        { change_interrupt(IRQ_STATUS_B, 0x10, state); }
	void il5_w(int state)        { change_interrupt(IRQ_STATUS_B, 0x20, state); }
	void il6_w(int state)        { change_interrupt(IRQ_STATUS_A, 0x01, state); }
	void il7_w(int state)        { change_interrupt(IRQ_STATUS_A, 0x02, state); }
	void fh0_w(int state)        { change_interrupt(FIQ_STATUS  , 0x01, state); }
	void fh1_w(int state)        { change_interrupt(FIQ_STATUS  , 0x02, state); }
	void fl_w(int state)         { change_interrupt(FIQ_STATUS  , 0x04, !state); }
	void por_w(int state)        { if (state) change_interrupt(IRQ_STATUS_A, 0x10, state); }
	void kin_w(int state)        { rx_w(state); }
	void if_w(int state);
	void ir_w(int state);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_serial_interface implementation
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_complete() override;

private:
	void update_interrups();
	void change_interrupt(int reg, uint8_t mask, int state);
	void set_timer(int tmr);
	void latch_timer_cnt(int tmr);
	TIMER_CALLBACK_MEMBER(timer_tick);

	enum    // registers
	{
		CONTROL       = 0x00 / 4,
		KART          = 0x04 / 4,
		IRQ_STATUS_A  = 0x10 / 4,
		IRQ_REQUEST_A = 0x14 / 4,
		IRQ_MASK_A    = 0x18 / 4,
		IRQ_STATUS_B  = 0x20 / 4,
		IRQ_REQUEST_B = 0x24 / 4,
		IRQ_MASK_B    = 0x28 / 4,
		FIQ_STATUS    = 0x30 / 4,
		FIQ_REQUEST   = 0x34 / 4,
		FIQ_MASK      = 0x38 / 4,
		T0_LATCH_LO   = 0x40 / 4,
		T0_LATCH_HI   = 0x44 / 4,
		T0_GO         = 0x48 / 4,
		T0_LATCH      = 0x4c / 4,
		T1_LATCH_LO   = 0x50 / 4,
		T1_LATCH_HI   = 0x54 / 4,
		T1_GO         = 0x58 / 4,
		T1_LATCH      = 0x5c / 4,
		T2_LATCH_LO   = 0x60 / 4,
		T2_LATCH_HI   = 0x64 / 4,
		T2_GO         = 0x68 / 4,
		T2_LATCH      = 0x6c / 4,
		T3_LATCH_LO   = 0x70 / 4,
		T3_LATCH_HI   = 0x74 / 4,
		T3_GO         = 0x78 / 4,
		T3_LATCH      = 0x7c / 4,
	};

	devcb_read32::array<7>      m_peripherals_r;
	devcb_write32::array<7>     m_peripherals_w;
	devcb_read_line::array<6>   m_giop_r;
	devcb_write_line::array<6>  m_giop_w;
	devcb_write_line            m_irq_w;
	devcb_write_line            m_fiq_w;
	devcb_write_line            m_kout_w;
	devcb_write_line            m_baud_w;
	emu_timer *                 m_timers[4];

	int                         m_ir;
	int                         m_if;
	int                         m_baud;
	uint32_t                    m_timercnt[4];
	uint32_t                    m_timerout[4];
	uint8_t                     m_regs[0x20];
};


// device type definition
DECLARE_DEVICE_TYPE(ACORN_IOC, acorn_ioc_device)

#endif // MAME_MACHINE_ACORN_IOC_H
