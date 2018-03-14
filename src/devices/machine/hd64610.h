// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**********************************************************************

    Hitachi HD64610 Real Time Clock

**********************************************************************
                            _____   _____
                   GND   1 |*    \_/     | 24  Vcc
          H-Start/Stop   2 |             | 23  OSC2
                  _IRQ   3 |             | 22  OSC1
                   1Hz   4 |   HD64610   | 21  GND
                    A3   5 |             | 20  _WE
                    A2   6 |             | 19  _OE
                    A1   7 |             | 18  _CS
                    A0   8 |             | 17  I/O8
                  I/O1   9 |             | 16  I/O7
                  I/O2  10 |             | 15  I/O6
                  I/O3  11 |             | 14  I/O5
                   GND  12 |_____________| 13  I/O4

**********************************************************************/

#ifndef MAME_MACHINE_HD64610_H
#define MAME_MACHINE_HD64610_H

#pragma once

#include "dirtc.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_HD64610_OUT_IRQ_CB(_devcb) \
	devcb = &downcast<hd64610_device &>(*device).set_out_irq_callback(DEVCB_##_devcb);

#define MCFG_HD64610_OUT_1HZ_CB(_devcb) \
	devcb = &downcast<hd64610_device &>(*device).set_out_1hz_callback(DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> hd64610_device

class hd64610_device :  public device_t,
						public device_rtc_interface,
						public device_nvram_interface
{
public:
	// construction/destruction
	hd64610_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> devcb_base &set_out_irq_callback(Object &&cb) { return m_out_irq_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_1hz_callback(Object &&cb) { return m_out_1hz_cb.set_callback(std::forward<Object>(cb)); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	// hardware start/stop line
	DECLARE_WRITE_LINE_MEMBER( h_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_rtc_interface overrides
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;

private:
	inline void set_irq_line();
	inline uint8_t read_counter(int counter);
	inline void write_counter(int counter, uint8_t value);
	inline void check_alarm();

	static const device_timer_id TIMER_UPDATE_COUNTER = 0;

	devcb_write_line        m_out_irq_cb;
	devcb_write_line        m_out_1hz_cb;

	uint8_t   m_regs[0x10];       // Internal registers
	int     m_hline_state;      // H-Start/Stop line
	int     m_irq_out;          // alarm output

	// timers
	emu_timer *m_counter_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(HD64610, hd64610_device)

#endif // MAME_MACHINE_HD64610_H
