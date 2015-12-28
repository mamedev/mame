// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    LMC1992 Digitally-Controlled Stereo Tone and Volume Circuit with
    Four-Channel Input-Selector emulation

**********************************************************************
                            _____   _____
                  Data   1 |*    \_/     | 28  V+
                 Clock   2 |             | 27  Bypass
                Enable   3 |             | 26  Right Input 1
          Left Input 1   4 |             | 25  Right Input 2
          Left Input 2   5 |             | 24  Right Input 3
          Left Input 3   6 |             | 23  Right Input 4
          Left Input 4   7 |   LMC1992   | 22  Right Select Out
       Left Select Out   8 |             | 21  Right Select In
        Left Select In   9 |             | 20  Right Tone In
          Left Tone In  10 |             | 19  Right Tone Out
         Left Tone Out  11 |             | 18  Right Op Amp Out
       Left Op Amp Out  12 |             | 17  Right Rear Out
         Left Rear Out  13 |             | 16  Right Front Out
        Left Front Out  14 |_____________| 15  Ground

**********************************************************************/

#pragma once

#ifndef __LMC1992__
#define __LMC1992__

#include "emu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

enum
{
	LMC1992_LEFT_INPUT_1 = 0,
	LMC1992_LEFT_INPUT_2,
	LMC1992_LEFT_INPUT_3,
	LMC1992_LEFT_INPUT_4,
	LMC1992_RIGHT_INPUT_1,
	LMC1992_RIGHT_INPUT_2,
	LMC1992_RIGHT_INPUT_3,
	LMC1992_RIGHT_INPUT_4
};



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_LMC1992_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, LMC1992, 0)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> lmc1992_device

class lmc1992_device :  public device_t,
						public device_sound_interface
{
public:
	// construction/destruction
	lmc1992_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE_LINE_MEMBER( clock_w );
	DECLARE_WRITE_LINE_MEMBER( data_w );
	DECLARE_WRITE_LINE_MEMBER( enable_w );

protected:
	// device-level overrides
	virtual void device_start() override;

	// internal callbacks
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	inline void execute_command(int addr, int data);

	//sound_stream *m_stream[4];

	int m_enable;                   // enable latch
	int m_data;                     // data latch
	int m_clk;                      // clock latch
	UINT16 m_si;                    // serial in shift register

	int m_input;                    // input select
	int m_bass;                     // bass
	int m_treble;                   // treble
	int m_volume;                   // volume
	int m_fader_rf;                 // right front fader
	int m_fader_lf;                 // left front fader
	int m_fader_rr;                 // right rear fader
	int m_fader_lr;                 // left rear fader
};


// device type definition
extern const device_type LMC1992;



#endif
