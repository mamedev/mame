// license: BSD-3-Clause
// copyright-holders: Dirk Best, Luca Elia
/***************************************************************************

    SN54/74165

    8-Bit Parallel-In/Serial-Out Shift Register

               ___ ___
    SH//LD  1 |*  u   | 16  VCC
       CLK  2 |       | 15  CLK INH
         E  3 |       | 14  D
         F  4 |       | 13  C
         G  5 |       | 12  B
         H  6 |       | 11  A
       /QH  7 |       | 10  SER
       GND  8 |_______|  9  QH

***************************************************************************/

#ifndef MAME_DEVICES_MACHINE_74165_H
#define MAME_DEVICES_MACHINE_74165_H

#pragma once

#include "machine/timer.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class ttl165_device : public device_t
{
public:
	// construction/destruction
	ttl165_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto data_callback() { return m_data_cb.bind(); }
	auto qh_callback() { return m_qh_cb.bind(); }

	void serial_w(int state);
	void clock_w(int state);
	void shift_load_w(int state);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

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
	void update_qh();
};


// device type definition
DECLARE_DEVICE_TYPE(TTL165, ttl165_device)

#endif // MAME_DEVICES_MACHINE_74165_H
