// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_CPU_F2MC16_MB9061X_H
#define MAME_CPU_F2MC16_MB9061X_H

#pragma once

#include "f2mc16.h"

//**************************************************************************
//  CONSTANTS
//**************************************************************************

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mb9061x_device

class mb9061x_device :  public f2mc16_device
{
	friend class mb90610_device;
	friend class mb90611_device;
	friend class mb90641_device;

public:
	const address_space_config m_program_config;

	// interrupts handled by the interrupt controller
	enum
	{
		ICR0 = 0, ICR1, ICR2, ICR3, ICR4, ICR5, ICR6, ICR7, ICR8, ICR9, ICR10, ICR11, ICR12, ICR13, ICR14, ICR15
	};

	// timer external counter tick functions
	DECLARE_WRITE_LINE_MEMBER(tin0_w);
	DECLARE_WRITE_LINE_MEMBER(tin1_w);

protected:
	// construction/destruction
	mb9061x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal_map);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual space_config_vector memory_space_config() const override;

private:
	// TBC
	TIMER_CALLBACK_MEMBER(tbtc_tick);
	u8 tbtc_r();
	void tbtc_w(u8 data);

	// INTC
	u8 intc_r(offs_t offset);
	void intc_w(offs_t offset, u8 data);
	void intc_trigger_irq(int icr, int vector);
	void intc_clear_irq(int icr, int vector);

	// TIMERS
	TIMER_CALLBACK_MEMBER(timer0_tick);
	TIMER_CALLBACK_MEMBER(timer1_tick);
	u8 timer_r(offs_t offset);
	void timer_w(offs_t offset, u8 data);
	void recalc_timer(int tnum);
	void tin_common(int timer, int base, int state);

	u8 m_timer_regs[8];
	u32 m_timer_hz[2];
	emu_timer *m_timer[2];
	int m_event_state[2];
	u16 m_event_count[2];

	u8 m_tbtc;
	emu_timer *m_tbtc_timer;

	u8 m_intc[0x10];
};

class mb90610_device : public mb9061x_device
{
public:
	mb90610_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void mb90610_map(address_map &map);
protected:
	mb90610_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};

class mb90611_device : public mb9061x_device
{
public:
	mb90611_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void mb90611_map(address_map &map);
protected:
	mb90611_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};

class mb90641_device : public mb9061x_device
{
public:
	mb90641_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void mb90641_map(address_map &map);
protected:
	mb90641_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(MB90610A, mb90610_device)
DECLARE_DEVICE_TYPE(MB90611A, mb90611_device)
DECLARE_DEVICE_TYPE(MB90641A, mb90641_device)

#endif // MAME_CPU_F2MC16_MB9061X_H
