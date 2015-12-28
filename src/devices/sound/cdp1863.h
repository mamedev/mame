// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA CDP1863 CMOS 8-Bit Programmable Frequency Generator emulation

**********************************************************************
                            _____   _____
                _RESET   1 |*    \_/     | 16  Vdd
                 CLK 2   2 |             | 15  OE
                 CLK 1   3 |             | 14  OUT
                   STR   4 |   CDP1863   | 13  DO7
                   DI0   5 |             | 12  DI6
                   DI1   6 |             | 11  DI5
                   DI2   7 |             | 10  DI4
                   Vss   8 |_____________| 9   DI3

**********************************************************************/

#pragma once

#ifndef __CDP1863__
#define __CDP1863__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CDP1863_ADD(_tag, _clock, _clock2) \
	MCFG_DEVICE_ADD(_tag, CDP1863, _clock) \
	cdp1863_device::static_set_clock2(*device, _clock2);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cdp1863_device

class cdp1863_device :  public device_t,
						public device_sound_interface
{
public:
	// construction/destruction
	cdp1863_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// inline configuration helpers
	static void static_set_clock2(device_t &device, int clock2);

	DECLARE_WRITE8_MEMBER( str_w );
	void str_w(UINT8 data);

	DECLARE_WRITE_LINE_MEMBER( oe_w );

	void set_clk1(int clock);
	void set_clk2(int clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// internal callbacks
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	sound_stream *m_stream;

	int m_clock1;                   // clock 1
	int m_clock2;                   // clock 2

	// sound state
	int m_oe;                       // output enable
	int m_latch;                    // sound latch
	INT16 m_signal;                 // current signal
	int m_incr;                     // initial wave state
};


// device type definition
extern const device_type CDP1863;



#endif
