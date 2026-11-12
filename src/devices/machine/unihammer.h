// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Seikosha uni-hammer dot matrix print mechanism

    A single hammer strikes through a 7-dot column as the platen turns,
    while a jaw clutch couples the carrier to the same motor shaft to
    advance it one dot column per platen ridge. The mechanism is driven
    by four active low control lines from the host CPU and reports back
    with the dot sensor and the home sensor:

        _MOT    motor on
        _HC     H solenoid: released engages the jaw clutch, asserted
                declutches and the recovery spring pulls the carrier home
        _LFC    line feed solenoid, one 1/18" pulse per assertion
        _PIN    hammer solenoid, fires one dot

        DOT     rotation detector, a burst of pulses per dot column
        HOME    high while the carrier sits at the home position

**********************************************************************/

#ifndef MAME_MACHINE_UNIHAMMER_H
#define MAME_MACHINE_UNIHAMMER_H

#pragma once

#include "machine/bitmap_printer.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> unihammer_printer_device

class unihammer_printer_device : public device_t
{
public:
	// construction/destruction
	unihammer_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// configuration
	auto dot_handler() { return m_dot_cb.bind(); }
	auto home_handler() { return m_home_cb.bind(); }

	// paper geometry, in inches (defaults to 8" x 11")
	void set_paper_size(int width_in, int height_in) { m_paper_width_in = width_in; m_paper_height_in = height_in; }

	// control inputs, taking the pin level as seen on the CPU port
	void mot_w(int state);
	void hc_w(int state);
	void lfc_w(int state);
	void pin_w(int state);

	// sensor outputs, for hosts that poll rather than take the callbacks
	int dot_r() const { return m_dot; }
	int home_r() const { return m_home; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(dot_tick);
	TIMER_CALLBACK_MEMBER(return_done);

	void start_slot();
	void set_dot(int state);
	void set_home(int state);
	void print_dot();
	void do_linefeed();

	required_device<bitmap_printer_device> m_bitmap_printer;

	devcb_write_line m_dot_cb;
	devcb_write_line m_home_cb;

	emu_timer *m_dot_timer = nullptr;
	emu_timer *m_return_timer = nullptr;

	int m_paper_width_in = 8;
	int m_paper_height_in = 11;

	// control line levels, as last written (idle high)
	uint8_t m_mot = 1;
	uint8_t m_hc = 1;
	uint8_t m_lfc = 1;
	uint8_t m_pin = 1;

	bool m_motor_on = false;
	bool m_clutch_engaged = false;

	int m_dot = 0;          // DOT sensor output
	int m_home = 1;         // HOME sensor output
	int m_slot = 0;         // pulse slot within the 30-slot group
	int m_column = 0;       // carrier position, in dot columns from home
};


// device type definition
DECLARE_DEVICE_TYPE(UNIHAMMER_PRINTER, unihammer_printer_device)


#endif // MAME_MACHINE_UNIHAMMER_H
