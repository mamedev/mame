// license:BSD-3-Clause
// copyright-holders:Ed Bernard, Jonathan Gevaryahu, hap
// thanks-to:Kevin Horton
/*
    SSi TSI S14001A speech IC emulator
*/

#ifndef MAME_SOUND_S14001A_H
#define MAME_SOUND_S14001A_H

#pragma once

#include "dirom.h"

class s14001a_device : public device_t, public device_sound_interface, public device_rom_interface<12>
{
public:
	s14001a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto bsy() { return m_bsy_handler.bind(); }

	int busy_r();              // /BUSY (pin 40)
	int romen_r();             // ROM /EN (pin 9)
	void start_w(int state);   // START (pin 10)
	void data_w(u8 data);      // 6-bit word

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override { m_stream->update(); m_stream->set_sample_rate(clock()); }
	virtual void rom_bank_pre_change() override { m_stream->update(); }

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	u8 ReadMem(u16 offset, bool phase);
	bool Clock(); // called once to toggle external clock twice

	enum class states : u8
	{
		IDLE = 0,
		WORDWAIT,
		CWARMSB,    // read 8 CWAR MSBs
		CWARLSB,    // read 4 CWAR LSBs from rom d7-d4
		DARMSB,     // read 8 DAR  MSBs
		CTRLBITS,   // read Stop, Voiced, Silence, Length, XRepeat
		PLAY,
		DELAY
	};

	sound_stream * m_stream;

	devcb_write_line m_bsy_handler;

	// internal state
	bool m_bPhase1;         // 1 bit internal clock

	// registers
	states m_uStateP1;      // 3 bits
	states m_uStateP2;

	u16 m_uDAR13To05P1;     // 9 MSBs of delta address register
	u16 m_uDAR13To05P2;     // incrementing uDAR05To13 advances ROM address by 8 bytes

	u16 m_uDAR04To00P1;     // 5 LSBs of delta address register
	u16 m_uDAR04To00P2;     // 3 address ROM, 2 mux 8 bits of data into 2 bit delta
							// carry indicates end of quarter pitch period (32 cycles)

	u16 m_uCWARP1;          // 12 bits Control Word Address Register (syllable)
	u16 m_uCWARP2;

	bool m_bStopP1;
	bool m_bStopP2;
	bool m_bVoicedP1;
	bool m_bVoicedP2;
	bool m_bSilenceP1;
	bool m_bSilenceP2;
	u8 m_uLengthP1;         // 7 bits, upper three loaded from ROM length
	u8 m_uLengthP2;         // middle two loaded from ROM repeat and/or uXRepeat
							// bit 0 indicates mirror in voiced mode
							// bit 1 indicates internal silence in voiced mode
							// incremented each pitch period quarter

	u8 m_uXRepeatP1;        // 2 bits, loaded from ROM repeat
	u8 m_uXRepeatP2;
	u8 m_uDeltaOldP1;       // 2 bit old delta
	u8 m_uDeltaOldP2;
	u8 m_uOutputP1;         // 4 bits audio output, calculated during phase 1

	// derived signals
	bool m_bDAR04To00CarryP2;
	bool m_bPPQCarryP2;
	bool m_bRepeatCarryP2;
	bool m_bLengthCarryP2;
	u16 m_uRomAddrP1;       // rom address

	// output pins
	u8 m_uOutputP2;         // output changes on phase2
	u16 m_uRomAddrP2;       // address pins change on phase 2
	bool m_bBusyP1;         // busy changes on phase 1

	// input pins
	bool m_bStart;
	u8 m_uWord;             // 6 bit word number to be spoken
};

DECLARE_DEVICE_TYPE(S14001A, s14001a_device)

#endif // MAME_SOUND_S14001A_H
