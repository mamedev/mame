// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony PlayStation 2 IOP interrupt controller device skeleton
*
*   To Do:
*     Everything
*
*/

#ifndef MAME_MACHINE_IOPINTC_H
#define MAME_MACHINE_IOPINTC_H

#pragma once

#include "cpu/mips/mips1.h"

class iop_intc_device : public device_t
{
public:
	template <typename T>
	iop_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&iop_tag)
		: iop_intc_device(mconfig, tag, owner, (uint32_t)0)
	{
		m_iop.set_tag(std::forward<T>(iop_tag));
	}

	iop_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~iop_intc_device() override;

	uint32_t read(offs_t offset, uint32_t mem_mask = ~0);
	void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void raise_interrupt(uint32_t line);

	enum
	{
		INT_VB_ON = 0,
		INT_DMA = 3,
		INT_SPU = 9,
		INT_VB_OFF = 11,
		INT_TIMER = 16,
		INT_SIO2 = 17
	};

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void update_interrupts();

	required_device<iop_device> m_iop;
	uint32_t m_status;
	uint32_t m_mask;
	bool m_enabled;
};

DECLARE_DEVICE_TYPE(SONYIOP_INTC, iop_intc_device)

#endif // MAME_MACHINE_IOPINTC_H
