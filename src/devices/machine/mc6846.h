// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/**********************************************************************

  Copyright (C) Antoine Mine' 2006

  Motorola 6846 timer emulation.

**********************************************************************/

#ifndef MAME_MACHINE_MC6846_H
#define MAME_MACHINE_MC6846_H

#pragma once


class mc6846_device : public device_t
{
public:
	mc6846_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto out_port() { return m_out_port_cb.bind(); }
	auto in_port() { return m_in_port_cb.bind(); }
	auto cp2() { return m_out_cp2_cb.bind(); }
	auto cto() { return m_out_cto_cb.bind(); }
	auto irq() { return m_irq_cb.bind(); }

	/* interface to CPU via address/data bus*/
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	/* asynchronous write from outside world into interrupt-generating pins */
	void set_input_cp1(int data);
	void set_input_cp2(int data);

	/* polling from outside world */
	uint8_t  get_output_port();
	uint8_t  get_output_cto();
	uint8_t  get_output_cp2();

	/* partial access to internal state */
	uint16_t get_preset(); /* timer interval - 1 in us */

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal state

	/* registers */
	uint8_t    m_csr;      /* 0,4: combination status register */
	uint8_t    m_pcr;      /* 1:   peripheral control register */
	uint8_t    m_ddr;      /* 2:   data direction register */
	uint8_t    m_pdr;      /* 3:   peripheral data register (last cpu write) */
	uint8_t    m_tcr;      /* 5:   timer control register */

	/* lines */
	uint8_t m_cp1;         /* 1-bit input */
	uint8_t m_cp2;         /* 1-bit input/output: last external write */
	uint8_t m_cp2_cpu;     /* last cpu write */
	uint8_t m_cto;         /* 1-bit timer output (unmasked) */

	/* internal state */
	uint8_t  m_time_MSB; /* MSB buffer register */
	uint8_t  m_csr0_to_be_cleared;
	uint8_t  m_csr1_to_be_cleared;
	uint8_t  m_csr2_to_be_cleared;
	uint16_t m_latch;   /* timer latch */
	uint16_t m_preset;  /* preset value */
	uint8_t  m_timer_started;

	/* timers */
	emu_timer *m_interval; /* interval programmable timer */
	emu_timer *m_one_shot; /* 1-us x factor one-shot timer */

	/* CPU write to the outside through chip */
	devcb_write8 m_out_port_cb;  /* 8-bit output */
	devcb_write_line m_out_cp2_cb;   /* 1-bit output */

	/* CPU read from the outside through chip */
	devcb_read8 m_in_port_cb; /* 8-bit input */

	/* asynchronous timer output to outside world */
	devcb_write_line m_out_cto_cb; /* 1-bit output */

	/* timer interrupt */
	devcb_write_line m_irq_cb;

	int m_old_cif;
	int m_old_cto;

	inline uint16_t counter();
	inline void update_irq();
	inline void update_cto();
	inline void timer_launch();

	TIMER_CALLBACK_MEMBER(timer_expire);
	TIMER_CALLBACK_MEMBER(timer_one_shot);
};

DECLARE_DEVICE_TYPE(MC6846, mc6846_device)

#endif // MAME_MACHINE_MC6846_H
