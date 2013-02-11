/**********************************************************************

    Toshiba T6721A C2MOS Voice Synthesizing LSI emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                   SP3   1 |*    \_/     | 42  Vdd
                  LOSS   2 |             | 41  SP2
                    TS   3 |             | 40  SP1
                   TSN   4 |             | 39  SP0
                     W   5 |             | 38  TEM
                  TDAI   6 |             | 37  FR
                  TFIO   7 |             | 36  BR
                   DAO   8 |             | 35  OD
                   APD   9 |             | 34  REP
                  phi2  10 |             | 33  EXP
                    PD  11 |    T6721A   | 32  CK2
           ROM ADR RST  12 |             | 31  CK1
               ROM RST  13 |             | 30  M-START
                   ALD  14 |             | 29  TPN
                    DI  15 |             | 28  _ACL
                  DTRD  16 |             | 27  CPUM
                    D3  17 |             | 26  _EOS
                    D2  18 |             | 25  _BSY
                    D1  19 |             | 24  _CE
                    D0  20 |             | 23  _RD
                   GND  21 |_____________| 22  _WR

**********************************************************************/

#pragma once

#ifndef __T6721__
#define __T6721__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_T6721A_EOS_HANDLER(_devcb) \
	devcb = &t6721a_device::set_eos_handler(*device, DEVCB2_##_devcb);

#define MCFG_T6721A_DTRD_HANDLER(_devcb) \
	devcb = &t6721a_device::set_dtrd_handler(*device, DEVCB2_##_devcb);

#define MCFG_T6721A_APD_HANDLER(_devcb) \
	devcb = &t6721a_device::set_apd_handler(*device, DEVCB2_##_devcb);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> t6721a_device

class t6721a_device : public device_t,
						public device_sound_interface
{
public:
	t6721a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb2_base &set_eos_handler(device_t &device, _Object object) { return downcast<t6721a_device &>(device).m_eos_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_dtrd_handler(device_t &device, _Object object) { return downcast<t6721a_device &>(device).m_dtrd_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_apd_handler(device_t &device, _Object object) { return downcast<t6721a_device &>(device).m_apd_handler.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( di_w );

protected:
	// device-level overrides
	virtual void device_start();

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

private:
	devcb2_write_line m_eos_handler;
	devcb2_write_line m_dtrd_handler;
	devcb2_write_line m_apd_handler;

	sound_stream *m_stream;
};


// device type definition
extern const device_type T6721A;



#endif
