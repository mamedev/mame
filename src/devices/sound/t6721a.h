// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Toshiba T6721A C2MOS Voice Synthesizing LSI emulation

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

#define MCFG_T6721A_EOS_HANDLER(_eos) \
	downcast<t6721a_device *>(device)->set_eos_callback(DEVCB_##_eos);

#define MCFG_T6721A_PHI2_HANDLER(_phi2) \
	downcast<t6721a_device *>(device)->set_phi2_callback(DEVCB_##_phi2);

#define MCFG_T6721A_DTRD_HANDLER(_dtrd) \
	downcast<t6721a_device *>(device)->set_dtrd_callback(DEVCB_##_dtrd);

#define MCFG_T6721A_APD_HANDLER(_apd) \
	downcast<t6721a_device *>(device)->set_apd_callback(DEVCB_##_apd);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> t6721a_device

class t6721a_device : public device_t,
						public device_sound_interface
{
public:
	t6721a_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _eos> void set_eos_callback(_eos eos) { m_write_eos.set_callback(eos); }
	template<class _phi2> void set_phi2_callback(_phi2 phi2) { m_write_phi2.set_callback(phi2); }
	template<class _dtrd> void set_dtrd_callback(_dtrd dtrd) { m_write_dtrd.set_callback(dtrd); }
	template<class _apd> void set_apd_callback(_apd apd) { m_write_apd.set_callback(apd); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( di_w );

	DECLARE_READ_LINE_MEMBER( eos_r );

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	enum
	{
		CMD_NOP = 0,
		CMD_STRT,
		CMD_STOP,
		CMD_ADLD,
		CMD_AAGN,
		CMD_SPLD,
		CMD_CNDT1,
		CMD_CNDT2,
		CMD_RRDM,
		CMD_SPDN,
		CMD_APDN,
		CMD_SAGN
	};

	devcb_write_line m_write_eos;
	devcb_write_line m_write_phi2;
	devcb_write_line m_write_dtrd;
	devcb_write_line m_write_apd;

	sound_stream *m_stream;
};


// device type definition
extern const device_type T6721A;



#endif
