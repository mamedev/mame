// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_CPU_M6502_M5074X_H
#define MAME_CPU_M6502_M5074X_H

#pragma once

#include "m740.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> m5074x_device

class m5074x_device :  public m740_device
{
	friend class m50740_device;
	friend class m50741_device;
	friend class m50753_device;

	enum
	{
		M5074X_INT1_LINE = INPUT_LINE_IRQ0
	};

	enum
	{
		TIMER_1 = 0,
		TIMER_2,
		TIMER_X,

		TIMER_ADC,

		NUM_TIMERS
	};

public:
	const address_space_config m_program_config;

	template <std::size_t Bit> auto read_p() { return m_read_p[Bit].bind(); }
	template <std::size_t Bit> auto write_p() { return m_write_p[Bit].bind(); }
	template <std::size_t Bit> void set_pullups(u8 mask) { m_pullups[Bit] = mask; }

	uint8_t ports_r(offs_t offset);
	void ports_w(offs_t offset, uint8_t data);
	uint8_t tmrirq_r(offs_t offset);
	void tmrirq_w(offs_t offset, uint8_t data);

	bool are_port_bits_output(uint8_t port, uint8_t mask) { return ((m_ddrs[port] & mask) == mask) ? true : false; }

protected:
	// construction/destruction
	m5074x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int addrbits, address_map_constructor internal_map);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

	// device_execute_interface overrides (TODO: /8 in M50740A/41/52/57/58 SLW mode)
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return (clocks + 4 - 1) / 4; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return (cycles * 4); }
	virtual void execute_set_input(int inputnum, int state) override;

	TIMER_CALLBACK_MEMBER(timer1_tick);
	TIMER_CALLBACK_MEMBER(timer2_tick);
	TIMER_CALLBACK_MEMBER(timerx_tick);
	virtual TIMER_CALLBACK_MEMBER(adc_complete) { }

	void send_port(uint8_t offset, uint8_t data);
	uint8_t read_port(uint8_t offset);

	void recalc_irqs();
	void recalc_timer(int timer);

	devcb_read8::array<5>  m_read_p;
	devcb_write8::array<5> m_write_p;

	uint8_t m_ports[5], m_ddrs[5], m_pullups[5];
	uint8_t m_intctrl, m_tmrctrl;
	uint8_t m_tmr12pre, m_tmr1, m_tmr2, m_tmrxpre, m_tmrx;
	uint8_t m_tmr1latch, m_tmr2latch, m_tmrxlatch;
	uint8_t m_last_all_ints;

private:
	emu_timer *m_timers[NUM_TIMERS];
};

class m50740_device : public m5074x_device
{
public:
	m50740_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	m50740_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

private:
	void m50740_map(address_map &map) ATTR_COLD;
};

class m50741_device : public m5074x_device
{
public:
	m50741_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	m50741_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

private:
	void m50741_map(address_map &map) ATTR_COLD;
};

class m50753_device : public m5074x_device
{
public:
	m50753_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	enum
	{
		M50753_INT1_LINE = INPUT_LINE_IRQ0,
		M50753_INT2_LINE = INPUT_LINE_IRQ1
	};

	template <std::size_t Bit> auto ad_in() { return m_ad_in[Bit].bind(); }

	auto read_in_p() { return m_in_p.bind(); }

protected:
	m50753_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void execute_set_input(int inputnum, int state) override;

	virtual TIMER_CALLBACK_MEMBER(adc_complete) override;

private:
	void m50753_map(address_map &map) ATTR_COLD;

	uint8_t ad_r();
	uint8_t in_r();
	void ad_start_w(uint8_t data);
	uint8_t ad_control_r();
	void ad_control_w(uint8_t data);
	void ad_trigger_w(uint8_t data);
	uint8_t pwm_control_r();
	void pwm_control_w(uint8_t data);

	devcb_read8::array<8> m_ad_in;
	devcb_read8 m_in_p;

	uint8_t m_ad_control;
	bool m_pwm_enabled;
};

DECLARE_DEVICE_TYPE(M50740, m50740_device)
DECLARE_DEVICE_TYPE(M50741, m50741_device)
DECLARE_DEVICE_TYPE(M50753, m50753_device)

#endif // MAME_CPU_M6502_M5074X_H
