// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_CPU_M6502_M3745X_H
#define MAME_CPU_M6502_M3745X_H

#pragma once

#include "m740.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> m3745x_device

class m3745x_device :  public m740_device
{
	friend class m37450_device;

	enum
	{
		TIMER_1 = 0,
		TIMER_2,
		TIMER_3,

		TIMER_ADC,

		NUM_TIMERS
	};

public:
	enum
	{
		M3745X_INT1_LINE = INPUT_LINE_IRQ0,
		M3745X_INT2_LINE,
		M3745X_INT3_LINE,

		M3745X_SET_OVERFLOW = M740_SET_OVERFLOW
	};

	const address_space_config m_program_config;

	template<std::size_t Bit> auto read_p() { return m_read_p[Bit-3].bind(); }
	template<std::size_t Bit> auto write_p() { return m_write_p[Bit-3].bind(); }
	template<std::size_t Bit> auto read_ad() { return m_read_ad[Bit].bind(); }

	devcb_read8  m_read_p[4];
	devcb_write8 m_write_p[4];
	devcb_read8  m_read_ad[8];

	DECLARE_READ8_MEMBER(ports_r);
	DECLARE_WRITE8_MEMBER(ports_w);
	DECLARE_READ8_MEMBER(adc_r);
	DECLARE_WRITE8_MEMBER(adc_w);
	DECLARE_READ8_MEMBER(intregs_r);
	DECLARE_WRITE8_MEMBER(intregs_w);

	bool are_port_bits_output(uint8_t port, uint8_t mask) { return ((m_ddrs[port] & mask) == mask) ? true : false; }

protected:
	// construction/destruction
	m3745x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal_map);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual space_config_vector memory_space_config() const override;

	void send_port(uint8_t offset, uint8_t data);
	uint8_t read_port(uint8_t offset);

	void recalc_irqs();

	uint8_t m_ports[6], m_ddrs[6];
	uint8_t m_intreq1, m_intreq2, m_intctrl1, m_intctrl2;
	uint8_t m_adctrl;
	uint16_t m_last_all_ints;

private:
	emu_timer *m_timers[NUM_TIMERS];
};

class m37450_device : public m3745x_device
{
public:
	m37450_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void m37450_map(address_map &map);
protected:
	m37450_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(M37450, m37450_device)

#endif // MAME_CPU_M6502_M3745X_H
