/**********************************************************************

    General Instruments AY-5-3600 Keyboard Encoder emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

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

#define MCFG_AY3600_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, AY3600, _clock)   \
	MCFG_DEVICE_CONFIG(_config)


#define AY3600_INTERFACE(name) \
	const ay3600_interface (name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ay3600_interface

struct ay3600_interface
{
	devcb_read16        m_in_x0_cb;
	devcb_read16        m_in_x1_cb;
	devcb_read16        m_in_x2_cb;
	devcb_read16        m_in_x3_cb;
	devcb_read16        m_in_x4_cb;
	devcb_read16        m_in_x5_cb;
	devcb_read16        m_in_x6_cb;
	devcb_read16        m_in_x7_cb;
	devcb_read16        m_in_x8_cb;

	devcb_read_line     m_in_shift_cb;
	devcb_read_line     m_in_control_cb;

	devcb_write_line    m_out_data_ready_cb;
	devcb_write_line    m_out_ako_cb;
};


// ======================> ay3600_device

class ay3600_device :   public device_t,
						public ay3600_interface
{
public:
	// construction/destruction
	ay3600_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

public:
	UINT16 b_r();

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	devcb_resolved_read16       m_in_x_func[9];

	devcb_resolved_read_line    m_in_shift_func;
	devcb_resolved_read_line    m_in_control_func;

	devcb_resolved_write_line   m_out_data_ready_func;
	devcb_resolved_write_line   m_out_ako_func;

	int m_b;                    // output buffer
	int m_ako;                  // any key down

	// timers
	emu_timer *m_scan_timer;    // keyboard scan timer
};


// device type definition
extern const device_type AY3600;



#endif
