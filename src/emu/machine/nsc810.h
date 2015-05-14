// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * nsc810.h
 *
 *  Created on: 10/03/2014
 */

#ifndef NSC810_H_
#define NSC810_H_

#include "emu.h"

class nsc810_device :  public device_t
{
public:
	// construction/destruction
	nsc810_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_portA_read_callback(device_t &device, _Object object) { return downcast<nsc810_device &>(device).m_portA_r.set_callback(object); }
	template<class _Object> static devcb_base &set_portB_read_callback(device_t &device, _Object object) { return downcast<nsc810_device &>(device).m_portB_r.set_callback(object); }
	template<class _Object> static devcb_base &set_portC_read_callback(device_t &device, _Object object) { return downcast<nsc810_device &>(device).m_portC_r.set_callback(object); }
	template<class _Object> static devcb_base &set_portA_write_callback(device_t &device, _Object object) { return downcast<nsc810_device &>(device).m_portA_w.set_callback(object); }
	template<class _Object> static devcb_base &set_portB_write_callback(device_t &device, _Object object) { return downcast<nsc810_device &>(device).m_portB_w.set_callback(object); }
	template<class _Object> static devcb_base &set_portC_write_callback(device_t &device, _Object object) { return downcast<nsc810_device &>(device).m_portC_w.set_callback(object); }
	template<class _Object> static devcb_base &set_timer0_callback(device_t &device, _Object object) { return downcast<nsc810_device &>(device).m_timer0_out.set_callback(object); }
	template<class _Object> static devcb_base &set_timer1_callback(device_t &device, _Object object) { return downcast<nsc810_device &>(device).m_timer1_out.set_callback(object); }

	void set_timer0_clock(UINT32 clk) { m_timer0_clock = clk; }
	void set_timer1_clock(UINT32 clk) { m_timer1_clock = clk; }

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

protected:
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	UINT8 m_portA_latch;
	UINT8 m_portB_latch;
	UINT8 m_portC_latch;
	UINT8 m_ddrA;
	UINT8 m_ddrB;
	UINT8 m_ddrC;
	UINT8 m_mode;
	emu_timer* m_timer0;
	emu_timer* m_timer1;
	UINT8 m_timer0_mode;
	UINT8 m_timer1_mode;
	UINT16 m_timer0_counter;
	UINT16 m_timer1_counter;
	UINT16 m_timer0_base;
	UINT16 m_timer1_base;
	bool m_timer0_running;
	bool m_timer1_running;
	UINT32 m_timer0_clock;
	UINT32 m_timer1_clock;
	bool m_ramselect;

	devcb_read8 m_portA_r;
	devcb_read8 m_portB_r;
	devcb_read8 m_portC_r;
	devcb_write8 m_portA_w;
	devcb_write8 m_portB_w;
	devcb_write8 m_portC_w;
	devcb_write_line m_timer0_out;
	devcb_write_line m_timer1_out;

	static const device_timer_id TIMER0_CLOCK = 0;
	static const device_timer_id TIMER1_CLOCK = 1;

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

#define MCFG_NSC810_ADD(_tag, _t0clk, _t1clk) \
	MCFG_DEVICE_ADD(_tag, NSC810, 0)           \
	downcast<nsc810_device *>(device)->set_timer0_clock(_t0clk);   \
	downcast<nsc810_device *>(device)->set_timer1_clock(_t1clk);

#define MCFG_NSC810_PORTA_READ(_read) \
	devcb = &nsc810_device::set_portA_read_callback(*device, DEVCB_##_read);

#define MCFG_NSC810_PORTB_READ(_read) \
	devcb = &nsc810_device::set_portB_read_callback(*device, DEVCB_##_read);

#define MCFG_NSC810_PORTC_READ(_read) \
	devcb = &nsc810_device::set_portC_read_callback(*device, DEVCB_##_read);

#define MCFG_NSC810_PORTA_WRITE(_write) \
	devcb = &nsc810_device::set_portA_write_callback(*device, DEVCB_##_write);

#define MCFG_NSC810_PORTB_WRITE(_write) \
	devcb = &nsc810_device::set_portB_write_callback(*device, DEVCB_##_write);

#define MCFG_NSC810_PORTC_WRITE(_write) \
	devcb = &nsc810_device::set_portC_write_callback(*device, DEVCB_##_write);

#define MCFG_NSC810_TIMER0_OUT(_write) \
	devcb = &nsc810_device::set_timer0_callback(*device, DEVCB_##_write);

#define MCFG_NSC810_TIMER1_OUT(_write) \
	devcb = &nsc810_device::set_timer1_callback(*device, DEVCB_##_write);

// device type definition
extern const device_type NSC810;


#endif /* NSC810_H_ */
