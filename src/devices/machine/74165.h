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
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_TTL165_DATA_CB(_devcb) \
	devcb = &downcast<ttl165_device &>(*device).set_data_callback(DEVCB_##_devcb);

#define MCFG_TTL165_QH_CB(_devcb) \
	devcb = &downcast<ttl165_device &>(*device).set_qh_callback(DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class ttl165_device : public device_t
{
public:
	// construction/destruction
	ttl165_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// configuration
	template <class Object> devcb_base &set_data_callback(Object &&cb)
		{ return m_data_cb.set_callback(std::forward<Object>(cb)); }

	template <class Object> devcb_base &set_qh_callback(Object &&cb)
		{ return m_qh_cb.set_callback(std::forward<Object>(cb)); }

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
	void update_qh();
};


// device type definition
DECLARE_DEVICE_TYPE(TTL165, ttl165_device)

#endif // MAME_DEVICES_MACHINE_74165_H
