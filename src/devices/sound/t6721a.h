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

#ifndef MAME_SOUND_T6721A_H
#define MAME_SOUND_T6721A_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> t6721a_device

class t6721a_device : public device_t,
						public device_sound_interface
{
public:
	t6721a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	auto eos_handler() { return m_write_eos.bind(); }
	auto phi2_handler() { return m_write_phi2.bind(); }
	auto dtrd_handler() { return m_write_dtrd.bind(); }
	auto apd_handler() { return m_write_apd.bind(); }

	uint8_t read();
	void write(uint8_t data);

	void di_w(int state);

	int eos_r();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

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
DECLARE_DEVICE_TYPE(T6721A, t6721a_device)

#endif // MAME_SOUND_T6721A_H
