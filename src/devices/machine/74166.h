// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    SN54/74166

    8-Bit Parallel-In/Serial-Out Shift Register

               ___ ___
       SER  1 |*  u   | 16  VCC
         A  2 |       | 15  SH//LD
         B  3 |       | 14  H
         C  4 |       | 13  QH
         D  5 |       | 12  G
   CLK INH  6 |       | 11  F
       CLK  7 |       | 10  E
       GND  8 |_______|  9  /CLR

***************************************************************************/

#ifndef MAME_DEVICES_MACHINE_74166_H
#define MAME_DEVICES_MACHINE_74166_H

#pragma once

#include "machine/timer.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class ttl166_device : public device_t
{
public:
	// construction/destruction
	ttl166_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto data_callback() { return m_data_cb.bind(); }
	auto qh_callback() { return m_qh_cb.bind(); }

	DECLARE_WRITE_LINE_MEMBER(serial_w);
	DECLARE_WRITE_LINE_MEMBER(clock_w);
	DECLARE_WRITE_LINE_MEMBER(shift_load_w);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<timer_device> m_timer;

	// callbacks
	devcb_read8 m_data_cb;
	devcb_write_line m_qh_cb;

	// state
	uint8_t m_data;
	int m_ser;
	int m_clk;
	int m_shld;

	TIMER_DEVICE_CALLBACK_MEMBER(qh_output);
};


// device type definition
DECLARE_DEVICE_TYPE(TTL166, ttl166_device)

#endif // MAME_DEVICES_MACHINE_74166_H
