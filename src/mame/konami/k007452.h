// license:BSD-3-Clause
// copyright-holders:Sean Gonsalves
/***************************************************************************

    Konami 007452 multiplier/divider

***************************************************************************/

#ifndef MAME_KONAMI_K007452_H
#define MAME_KONAMI_K007452_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class k007452_device : public device_t
{
public:
	// construction/destruction
	k007452_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// public interface
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal state
	u8 m_math_regs[6]{};
	u16 m_multiply_result = 0;
	u16 m_divide_quotient = 0;
	u16 m_divide_remainder = 0;
};


// device type declaration
DECLARE_DEVICE_TYPE(KONAMI_007452_MATH, k007452_device)

#endif // MAME_KONAMI_K007452_H
