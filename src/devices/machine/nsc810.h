// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * nsc810.h
 *
 *  Created on: 10/03/2014
 */

#ifndef MAME_MACHINE_NSC810_H
#define MAME_MACHINE_NSC810_H

#pragma once


class nsc810_device :  public device_t
{
public:
	// construction/destruction
	nsc810_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t clk0, uint32_t clk1)
		: nsc810_device(mconfig, tag, owner, clock)
	{
		set_timer0_clock(clk0);
		set_timer1_clock(clk1);
	}

	nsc810_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, const XTAL &clk0, const XTAL &clk1)
		: nsc810_device(mconfig, tag, owner, clock)
	{
		set_timer0_clock(clk0.value());
		set_timer1_clock(clk1.value());
	}

	nsc810_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto portA_read_callback() { return m_portA_r.bind(); }
	auto portB_read_callback() { return m_portB_r.bind(); }
	auto portC_read_callback() { return m_portC_r.bind(); }
	auto portA_write_callback() { return m_portA_w.bind(); }
	auto portB_write_callback() { return m_portB_w.bind(); }
	auto portC_write_callback() { return m_portC_w.bind(); }
	auto timer0_callback() { return m_timer0_out.bind(); }
	auto timer1_callback() { return m_timer1_out.bind(); }

	void set_timer0_clock(uint32_t clk) { m_timer0_clock = clk; }
	void set_timer0_clock(const XTAL &clk) { set_timer0_clock(clk.value()); }
	void set_timer1_clock(uint32_t clk) { m_timer1_clock = clk; }
	void set_timer1_clock(const XTAL &clk) { set_timer1_clock(clk.value()); }

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	uint8_t m_portA_latch;
	uint8_t m_portB_latch;
	uint8_t m_portC_latch;
	uint8_t m_ddrA;
	uint8_t m_ddrB;
	uint8_t m_ddrC;
	uint8_t m_mode;
	emu_timer* m_timer0;
	emu_timer* m_timer1;
	uint8_t m_timer0_mode;
	uint8_t m_timer1_mode;
	uint16_t m_timer0_counter;
	uint16_t m_timer1_counter;
	uint16_t m_timer0_base;
	uint16_t m_timer1_base;
	bool m_timer0_running;
	bool m_timer1_running;
	uint32_t m_timer0_clock;
	uint32_t m_timer1_clock;
	bool m_ramselect;

	devcb_read8 m_portA_r;
	devcb_read8 m_portB_r;
	devcb_read8 m_portC_r;
	devcb_write8 m_portA_w;
	devcb_write8 m_portB_w;
	devcb_write8 m_portC_w;
	devcb_write_line m_timer0_out;
	devcb_write_line m_timer1_out;

	static constexpr device_timer_id TIMER0_CLOCK = 0;
	static constexpr device_timer_id TIMER1_CLOCK = 1;

	enum
	{
		REG_PORTA = 0x00,
		REG_PORTB,
		REG_PORTC,
		REG_DDRA = 0x04,
		REG_DDRB,
		REG_DDRC,
		REG_MODE_DEF,
		REG_PORTA_BITCLR,
		REG_PORTB_BITCLR,
		REG_PORTC_BITCLR,
		REG_PORTA_BITSET = 0x0c,
		REG_PORTB_BITSET,
		REG_PORTC_BITSET,
		REG_TIMER0_LOW = 0x10,
		REG_TIMER0_HIGH,
		REG_TIMER1_LOW,
		REG_TIMER1_HIGH,
		REG_TIMER0_STOP,
		REG_TIMER0_START,
		REG_TIMER1_STOP,
		REG_TIMER1_START,
		REG_MODE_TIMER0,
		REG_MODE_TIMER1
	};
};

// device type definition
DECLARE_DEVICE_TYPE(NSC810, nsc810_device)

#endif // MAME_MACHINE_NSC810_H
