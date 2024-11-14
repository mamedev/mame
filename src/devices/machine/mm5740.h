// license:BSD-3-Clause
// copyright-holders: R. Belmont, Mark Garlanger
/**********************************************************************

    MM5740 Keyboard Encoder emulation

 **********************************************************************
                            _____   _____
                    B3   1 |*    \_/     | 40  B4
                   Vll   2 |             | 39  B9
                 Clock   3 |             | 38  B2
                    X9   4 |             | 37  B1
                    X8   5 |             | 36  B8
                    X7   6 |             | 35  B7
                    X6   7 |             | 34  B6
                    X5   8 |             | 33  B5
                    X4   9 |             | 32  Vss
                    X3  10 |    MM5740   | 31  Y9
                    X2  11 |             | 30  Y8
                    X1  12 |             | 29  Y7
    Data Strobe Output  13 |             | 28  Y6
   Data Strobe Control  14 |             | 27  Y5
         Output Enable  15 |             | 26  Y4
                Repeat  16 |             | 25  Y3
       Key Bounce Mask  17 |             | 24  Y2
                   Vgg  18 |             | 23  Y1
               Control  19 |             | 22  Y10
        Shift Lock I/O  20 |_____________| 21  Shift


Name                 Pin No.     Function
----------------------------------------------------------------------

X1-X9                4-12        Output - Drives the key switch matrix.

Y1-Y10               22-31       Inputs - connect to the X drive lines with
                                 the key switch matrix.

B1-B9                1,33-40     Tri-stated data outputs.

Data Strobe Output   13          Output to indicate key pressed.

Data Strobe Control  14          Input to control data strobe output pulse width.

Output Enable        15          Input to control the chip's TRI-STATE output

Repeat               16          Each cycle of this signal will issue
                                 a new data strobe for the pressed key.

Key-Bounce Mask      17          Use capacitor on this chip to provide
                                 key debouncing

Shift                21          Shift key pressed

Control              19          Control key pressed

Shift Lock I/O       20          Togglable input to signify shift (NOT caps) lock.

Clock                3           A TTL compatible clock signal

Vss                  32          +5.0V

Vll                  2           Ground

Vgg                  18          -12V

**********************************************************************/

/* TODO:
    Support shift lock
    Support additional internal ROMs
*/

#ifndef MAME_MACHINE_MM5740_H
#define MAME_MACHINE_MM5740_H

#pragma once

class mm5740_device : public device_t
{
public:
	mm5740_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// public interface
	u16 b_r();

	template <unsigned N> auto x_cb() { return m_read_x[N - 1].bind(); }
	auto shift_cb() { return m_read_shift.bind(); }
	auto control_cb() { return m_read_control.bind(); }
	auto data_ready_cb() { return m_write_data_ready.bind(); }

	void repeat_line_w(int state);

	static u32 calc_effective_clock_key_debounce(u32 capacitance);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(perform_scan);

private:
	devcb_read16::array<9> m_read_x;
	devcb_read_line m_read_shift;
	devcb_read_line m_read_control;
	devcb_write_line m_write_data_ready;

	required_memory_region m_rom; // Internal ROM

	s32  m_b;                     // output buffer
	u16  m_offset;                // last key pressed (without shift/ctrl modifiers)

	u16  m_x_mask[9];             // mask of what keys are down
	bool m_repeat;                // state of the 'repeat' input.
	bool m_last_repeat;           // state of the repeat input on the last scan.

	// timers
	emu_timer *m_scan_timer;      // keyboard scan timer
};


// device type definition
DECLARE_DEVICE_TYPE(MM5740, mm5740_device)

#endif // MAME_MACHINE_MM5740_H
