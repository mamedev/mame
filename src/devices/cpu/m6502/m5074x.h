// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_CPU_M6502_M5074X_H
#define MAME_CPU_M6502_M5074X_H

#pragma once

#include "m740.h"

//**************************************************************************
//  CONSTANTS
//**************************************************************************

// internal ROM region
#define M5074X_INTERNAL_ROM_REGION "internal"
#define M5074X_INTERNAL_ROM(_tag) (_tag ":" M5074X_INTERNAL_ROM_REGION)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> m5074x_device

class m5074x_device :  public m740_device
{
	friend class m50740_device;
	friend class m50741_device;

	enum
	{
		M5074X_INT1_LINE = INPUT_LINE_IRQ0,

		M5074X_SET_OVERFLOW = M740_SET_OVERFLOW
	};

	enum
	{
		TIMER_1 = 0,
		TIMER_2,
		TIMER_X,

		NUM_TIMERS
	};

public:
	const address_space_config m_program_config;

	template <std::size_t Bit> auto read_p() { return m_read_p[Bit].bind(); }
	template <std::size_t Bit> auto write_p() { return m_write_p[Bit].bind(); }

	devcb_read8  m_read_p[4];
	devcb_write8 m_write_p[4];

	DECLARE_READ8_MEMBER(ports_r);
	DECLARE_WRITE8_MEMBER(ports_w);
	DECLARE_READ8_MEMBER(tmrirq_r);
	DECLARE_WRITE8_MEMBER(tmrirq_w);

	bool are_port_bits_output(uint8_t port, uint8_t mask) { return ((m_ddrs[port] & mask) == mask) ? true : false; }

protected:
	// construction/destruction
	m5074x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal_map);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual space_config_vector memory_space_config() const override;

	void send_port(address_space &space, uint8_t offset, uint8_t data);
	uint8_t read_port(uint8_t offset);

	void recalc_irqs();
	void recalc_timer(int timer);

	uint8_t m_ports[6], m_ddrs[6];
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

	void m50740_map(address_map &map);
protected:
	m50740_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};

class m50741_device : public m5074x_device
{
public:
	m50741_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void m50741_map(address_map &map);
protected:
	m50741_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(M50740, m50740_device)
DECLARE_DEVICE_TYPE(M50741, m50741_device)

#endif // MAME_CPU_M6502_M5074X_H
