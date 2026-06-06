// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
/***************************************************************************

    h8_intc_base.h

    Formal definition of the interface that H8 family peripherals expect
    an on-board interrupt controller to implement.

***************************************************************************/

#ifndef MAME_CPU_H8_H8_INTC_BASE_H
#define MAME_CPU_H8_H8_INTC_BASE_H

#pragma once


class h8_intc_base : public device_t
{
public:
	// Called to set an IRQ from an internal (to the MCU) source
	virtual void internal_interrupt(int vector) = 0;

protected:
	h8_intc_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, type, tag, owner, clock)
	{
	}
};

#endif // MAME_CPU_H8_H8_INTC_BASE_H
