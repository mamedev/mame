// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Microelectronic-Marin E050-16 Real Time Clock emulation

**********************************************************************
                            _____   _____
                  Vdd1   1 |*    \_/     | 16  Vdd2
                OSC IN   2 |             | 15  Clk
               OSC OUT   3 |             | 14  XOUT
                 _STOP   4 |   E05-16    | 13  DI/O
                _RESET   5 |   E050-16   | 12  _SEC
               _OUTSEL   6 |             | 11  _MIN
                  _DAY   7 |             | 10  _HRS
                   Vss   8 |_____________| 9   _CS

**********************************************************************/

#pragma once

#ifndef __E0516__
#define __E0516__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_E0516_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, E0516, _clock)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> e0516_device

class e0516_device :  public device_t,
						public device_rtc_interface
{
public:
	// construction/destruction
	e0516_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE_LINE_MEMBER( cs_w );
	DECLARE_WRITE_LINE_MEMBER( clk_w );
	DECLARE_WRITE_LINE_MEMBER( dio_w );
	DECLARE_READ_LINE_MEMBER( dio_r );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	int m_cs;                       // chip select
	int m_clk;                      // clock
	int m_data_latch;               // data latch
	int m_reg_latch;                // register latch
	int m_read_write;               // read/write data
	int m_state;                    // state
	int m_bits;                     // number of bits transferred
	int m_dio;                      // data pin

	// timers
	emu_timer *m_timer;
};


// device type definition
extern const device_type E0516;



#endif
