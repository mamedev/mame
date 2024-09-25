// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**********************************************************************

    General Instruments AY-5-3600 Keyboard Encoder emulation

**********************************************************************
                            _____   _____
                         1 |*    \_/     | 40  X0
                         2 |             | 39  X1
                         3 |             | 38  X2
                         4 |             | 37  X3
                         5 |             | 36  X4
                    B9   6 |             | 35  X5
                    B8   7 |             | 34  X6
                    B7   8 |             | 33  X7
                    B6   9 |             | 32  X8
                    B5  10 |  AY-5-3600  | 31  DELAY NODE
                    B4  11 |             | 30  Vcc
                    B3  12 |             | 29  SHIFT
                    B2  13 |             | 28  CONTROL
                    B1  14 |             | 27  Vgg
                   Vdd  15 |             | 26  Y9
            DATA READY  16 |             | 25  Y8
                    Y0  17 |             | 24  Y7
                    Y1  18 |             | 23  Y6
                    Y2  19 |             | 22  Y5
                    Y3  20 |_____________| 21  Y4

                            _____   _____
                   Vcc   1 |*    \_/     | 40  Vss
                    B9   2 |             | 39  Vgg
                    B8   3 |             | 38  _STCL?
                    B7   4 |             | 37  _MCLR
                  TEST   5 |             | 36  OSC
                    B6   6 |             | 35  CLK OUT
                    B5   7 |             | 34  X7
                    B4   8 |             | 33  X6
                    B3   9 |             | 32  X5
                    B2  10 |  AY-5-3600  | 31  X4
                    B1  11 |   PRO 002   | 30  X3
                    X8  12 |             | 29  X2
                   AKO  13 |             | 28  X1
                  CTRL  14 |             | 27  X0
                 SHIFT  15 |             | 26  Y9
            DATA READY  16 |             | 25  Y8
                    Y0  17 |             | 24  Y7
                    Y1  18 |             | 23  Y6
                    Y2  19 |             | 22  Y5
                    Y3  20 |_____________| 21  Y4

**********************************************************************/

#ifndef MAME_MACHINE_KB3600_H
#define MAME_MACHINE_KB3600_H

#pragma once

class ay3600_device : public device_t
{
public:
	// construction/destruction
	ay3600_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// public interface
	uint16_t b_r();

	auto x0() { return m_read_x0.bind(); }
	auto x1() { return m_read_x1.bind(); }
	auto x2() { return m_read_x2.bind(); }
	auto x3() { return m_read_x3.bind(); }
	auto x4() { return m_read_x4.bind(); }
	auto x5() { return m_read_x5.bind(); }
	auto x6() { return m_read_x6.bind(); }
	auto x7() { return m_read_x7.bind(); }
	auto x8() { return m_read_x8.bind(); }
	auto shift() { return m_read_shift.bind(); }
	auto control() { return m_read_control.bind(); }
	auto data_ready() { return m_write_data_ready.bind(); }
	auto ako() { return m_write_ako.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(perform_scan);

	devcb_read16 m_read_x0, m_read_x1, m_read_x2, m_read_x3, m_read_x4, m_read_x5, m_read_x6, m_read_x7, m_read_x8;
	devcb_read_line m_read_shift, m_read_control;
	devcb_write_line m_write_data_ready, m_write_ako;

private:
	static constexpr int MAX_KEYS_DOWN = 4;

	int m_b;                    // output buffer
	int m_ako;                  // any key down

	int m_x_mask[9];            // mask of what keys are down

	// timers
	emu_timer *m_scan_timer;    // keyboard scan timer
};


// device type definition
DECLARE_DEVICE_TYPE(AY3600, ay3600_device)

#endif // MAME_MACHINE_KB3600_H
