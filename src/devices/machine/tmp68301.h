// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef MAME_MACHINE_TMP68301_H
#define MAME_MACHINE_TMP68301_H

#pragma once

#include "cpu/m68000/m68000.h"

/* TODO: serial ports, frequency & hook it up with m68k */


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************



class tmp68301_device : public m68000_device
{
public:
	tmp68301_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto in_parallel_callback() { return m_in_parallel_cb.bind(); }
	auto out_parallel_callback() { return m_out_parallel_cb.bind(); }

	// Interrupts
	void external_interrupt_0();
	void external_interrupt_1();
	void external_interrupt_2();

private:
	DECLARE_READ16_MEMBER(imr_r);
	DECLARE_WRITE16_MEMBER(imr_w);
	DECLARE_READ16_MEMBER(ipr_r);
	DECLARE_WRITE16_MEMBER(ipr_w);
	DECLARE_READ16_MEMBER(iisr_r);
	DECLARE_WRITE16_MEMBER(iisr_w);
	DECLARE_READ16_MEMBER(scr_r);
	DECLARE_WRITE16_MEMBER(scr_w);
	DECLARE_READ16_MEMBER(pdr_r);
	DECLARE_WRITE16_MEMBER(pdr_w);
	DECLARE_READ16_MEMBER(pdir_r);
	DECLARE_WRITE16_MEMBER(pdir_w);
	DECLARE_READ8_MEMBER(icr_r);
	DECLARE_WRITE8_MEMBER(icr_w);

	// Hardware Registers
	DECLARE_READ16_MEMBER( regs_r );
	DECLARE_WRITE16_MEMBER( regs_w );

	void tmp68301_regs(address_map &map);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	TIMER_CALLBACK_MEMBER(timer_callback);
	void update_timer(int i);
	void update_ipl();
	uint8_t serial_interrupt_cause(int channel);

	static constexpr uint16_t EXT_IRQ0 = 1 << 0;
	static constexpr uint16_t EXT_IRQ1 = 1 << 1;
	static constexpr uint16_t EXT_IRQ2 = 1 << 2;
	static constexpr uint16_t SERIAL_IRQ_CH0 = 1 << 4;
	static constexpr uint16_t SERIAL_IRQ_CH1 = 1 << 5;
	static constexpr uint16_t SERIAL_IRQ_CH2 = 1 << 6;
	static constexpr uint16_t PARALLEL_IRQ = 1 << 7;
	static constexpr uint16_t TIMER0_IRQ = 1 << 8;
	static constexpr uint16_t TIMER1_IRQ = 1 << 9;
	static constexpr uint16_t TIMER2_IRQ = 1 << 10;

	devcb_read16         m_in_parallel_cb;
	devcb_write16        m_out_parallel_cb;

	// internal state
	uint16_t m_regs[0x400];

	emu_timer *m_tmp68301_timer[3];        // 3 Timers

	uint8_t m_ipl; // internal interrupt level

	uint16_t m_imr;
	uint16_t m_ipr;
	uint16_t m_iisr;
	uint16_t m_scr;
	uint16_t m_pdir;
	uint16_t m_pdr;
	uint8_t m_icr[10];

	void internal_vectors_r(address_map &map);
	uint8_t irq_callback(offs_t offset);
};

DECLARE_DEVICE_TYPE(TMP68301, tmp68301_device)

#endif // MAME_MACHINE_TMP68301_H
