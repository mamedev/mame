// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony PlayStation 2 EE interrupt controller device skeleton
*
*   To Do:
*     Everything
*
*/

#ifndef MAME_MACHINE_PS2INTC_H
#define MAME_MACHINE_PS2INTC_H

#pragma once

class ps2_intc_device;

#include "cpu/mips/mips3.h"

class ps2_intc_device : public device_t
{
public:
	template <typename T>
	ps2_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&ee_tag)
		: ps2_intc_device(mconfig, tag, owner, (uint32_t)0)
	{
		m_ee.set_tag(std::forward<T>(ee_tag));
	}

	ps2_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~ps2_intc_device() override;

	uint32_t read(offs_t offset, uint32_t mem_mask = ~0);
	void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void raise_interrupt(uint32_t line);

	enum
	{
		INT_GS = 0,
		INT_SBUS,
		INT_VB_ON,
		INT_VB_OFF,
		INT_VIF0,
		INT_VIF1,
		INT_VU0,
		INT_VU1,
		INT_IPU,
		INT_TIMER0,
		INT_TIMER1,
		INT_TIMER2,
		INT_TIMER3,
		INT_SFIFO,
		INT_VU0WD
	};

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void update_interrupts();

	required_device<cpu_device> m_ee;

	uint32_t m_status;
	uint32_t m_mask;
};

DECLARE_DEVICE_TYPE(SONYPS2_INTC, ps2_intc_device)

#endif // MAME_MACHINE_PS2INTC_H
