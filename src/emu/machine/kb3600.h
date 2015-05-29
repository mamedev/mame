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

#pragma once

#ifndef __AY3600__
#define __AY3600__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_AY3600_MATRIX_X0(_cb)  \
	devcb = &ay3600_device::set_x0_cb(*device, DEVCB_##_cb);
#define MCFG_AY3600_MATRIX_X1(_cb)  \
	devcb = &ay3600_device::set_x1_cb(*device, DEVCB_##_cb);
#define MCFG_AY3600_MATRIX_X2(_cb)  \
	devcb = &ay3600_device::set_x2_cb(*device, DEVCB_##_cb);
#define MCFG_AY3600_MATRIX_X3(_cb)  \
	devcb = &ay3600_device::set_x3_cb(*device, DEVCB_##_cb);
#define MCFG_AY3600_MATRIX_X4(_cb)  \
	devcb = &ay3600_device::set_x4_cb(*device, DEVCB_##_cb);
#define MCFG_AY3600_MATRIX_X5(_cb)  \
	devcb = &ay3600_device::set_x5_cb(*device, DEVCB_##_cb);
#define MCFG_AY3600_MATRIX_X6(_cb)  \
	devcb = &ay3600_device::set_x6_cb(*device, DEVCB_##_cb);
#define MCFG_AY3600_MATRIX_X7(_cb)  \
	devcb = &ay3600_device::set_x7_cb(*device, DEVCB_##_cb);
#define MCFG_AY3600_MATRIX_X8(_cb)  \
	devcb = &ay3600_device::set_x8_cb(*device, DEVCB_##_cb);
#define MCFG_AY3600_SHIFT_CB(_cb)   \
	devcb = &ay3600_device::set_shift_cb(*device, DEVCB_##_cb);
#define MCFG_AY3600_CONTROL_CB(_cb) \
	devcb = &ay3600_device::set_control_cb(*device, DEVCB_##_cb);
#define MCFG_AY3600_DATA_READY_CB(_cb)  \
	devcb = &ay3600_device::set_data_ready_cb(*device, DEVCB_##_cb);
#define MCFG_AY3600_AKO_CB(_cb) \
	devcb = &ay3600_device::set_ako_cb(*device, DEVCB_##_cb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ay3600_device

class ay3600_device :   public device_t
{
public:
	// construction/destruction
	ay3600_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// public interface
	UINT16 b_r();

	template<class _Object> static devcb_base &set_x0_cb(device_t &device, _Object rd) { return downcast<ay3600_device &>(device).m_read_x0.set_callback(rd); }
	template<class _Object> static devcb_base &set_x1_cb(device_t &device, _Object rd) { return downcast<ay3600_device &>(device).m_read_x1.set_callback(rd); }
	template<class _Object> static devcb_base &set_x2_cb(device_t &device, _Object rd) { return downcast<ay3600_device &>(device).m_read_x2.set_callback(rd); }
	template<class _Object> static devcb_base &set_x3_cb(device_t &device, _Object rd) { return downcast<ay3600_device &>(device).m_read_x3.set_callback(rd); }
	template<class _Object> static devcb_base &set_x4_cb(device_t &device, _Object rd) { return downcast<ay3600_device &>(device).m_read_x4.set_callback(rd); }
	template<class _Object> static devcb_base &set_x5_cb(device_t &device, _Object rd) { return downcast<ay3600_device &>(device).m_read_x5.set_callback(rd); }
	template<class _Object> static devcb_base &set_x6_cb(device_t &device, _Object rd) { return downcast<ay3600_device &>(device).m_read_x6.set_callback(rd); }
	template<class _Object> static devcb_base &set_x7_cb(device_t &device, _Object rd) { return downcast<ay3600_device &>(device).m_read_x7.set_callback(rd); }
	template<class _Object> static devcb_base &set_x8_cb(device_t &device, _Object rd) { return downcast<ay3600_device &>(device).m_read_x8.set_callback(rd); }
	template<class _Object> static devcb_base &set_shift_cb(device_t &device, _Object rd) { return downcast<ay3600_device &>(device).m_read_shift.set_callback(rd); }
	template<class _Object> static devcb_base &set_control_cb(device_t &device, _Object rd) { return downcast<ay3600_device &>(device).m_read_control.set_callback(rd); }
	template<class _Object> static devcb_base &set_data_ready_cb(device_t &device, _Object wr) { return downcast<ay3600_device &>(device).m_write_data_ready.set_callback(wr); }
	template<class _Object> static devcb_base &set_ako_cb(device_t &device, _Object wr) { return downcast<ay3600_device &>(device).m_write_ako.set_callback(wr); }

	devcb_read16 m_read_x0, m_read_x1, m_read_x2, m_read_x3, m_read_x4, m_read_x5, m_read_x6, m_read_x7, m_read_x8;
	devcb_read_line m_read_shift, m_read_control;
	devcb_write_line m_write_data_ready, m_write_ako;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	static const int MAX_KEYS_DOWN = 4;

	int m_b;                    // output buffer
	int m_ako;                  // any key down

	int m_x_mask[9];            // mask of what keys are down

	// timers
	emu_timer *m_scan_timer;    // keyboard scan timer
};


// device type definition
extern const device_type AY3600;



#endif
