// license:BSD-3-Clause
// copyright-holders:Ed Bernard, Jonathan Gevaryahu, hap
// thanks-to:Kevin Horton
/*
    SSi TSI S14001A speech IC emulator
*/

#ifndef MAME_SOUND_S14001A_H
#define MAME_SOUND_S14001A_H

class s14001a_device : public device_t, public device_sound_interface
{
public:
	s14001a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto bsy() { return m_bsy_handler.bind(); }
	auto ext_read() { return m_ext_read_handler.bind(); }

	DECLARE_READ_LINE_MEMBER(busy_r);   // /BUSY (pin 40)
	DECLARE_READ_LINE_MEMBER(romen_r);  // ROM /EN (pin 9)
	DECLARE_WRITE_LINE_MEMBER(start_w); // START (pin 10)
	DECLARE_WRITE8_MEMBER(data_w);      // 6-bit word

	void set_clock(uint32_t clock);       // set new CLK frequency
	void set_clock(const XTAL &xtal) { set_clock(xtal.value()); }
	void force_update();                // update stream, eg. before external ROM bankswitch

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	uint8_t readmem(uint16_t offset, bool phase);
	bool Clock(); // called once to toggle external clock twice

	// emulator helper functions
	void ClearStatistics();
	void GetStatistics(uint32_t &uNPitchPeriods, uint32_t &uNVoiced, uint32_t &uNControlWords);
	void SetPrintLevel(uint32_t uPrintLevel) { m_uPrintLevel = uPrintLevel; }

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

	required_region_ptr<uint8_t> m_SpeechRom;
	sound_stream * m_stream;

	devcb_write_line m_bsy_handler;
	devcb_read8 m_ext_read_handler;

	// internal state
	bool m_bPhase1; // 1 bit internal clock

	// registers
	states m_uStateP1;          // 3 bits
	states m_uStateP2;

	uint16_t m_uDAR13To05P1;      // 9 MSBs of delta address register
	uint16_t m_uDAR13To05P2;      // incrementing uDAR05To13 advances ROM address by 8 bytes

	uint16_t m_uDAR04To00P1;      // 5 LSBs of delta address register
	uint16_t m_uDAR04To00P2;      // 3 address ROM, 2 mux 8 bits of data into 2 bit delta
								// carry indicates end of quarter pitch period (32 cycles)

	uint16_t m_uCWARP1;           // 12 bits Control Word Address Register (syllable)
	uint16_t m_uCWARP2;

	bool m_bStopP1;
	bool m_bStopP2;
	bool m_bVoicedP1;
	bool m_bVoicedP2;
	bool m_bSilenceP1;
	bool m_bSilenceP2;
	uint8_t m_uLengthP1;          // 7 bits, upper three loaded from ROM length
	uint8_t m_uLengthP2;          // middle two loaded from ROM repeat and/or uXRepeat
								// bit 0 indicates mirror in voiced mode
								// bit 1 indicates internal silence in voiced mode
								// incremented each pitch period quarter

	uint8_t m_uXRepeatP1;         // 2 bits, loaded from ROM repeat
	uint8_t m_uXRepeatP2;
	uint8_t m_uDeltaOldP1;        // 2 bit old delta
	uint8_t m_uDeltaOldP2;
	uint8_t m_uOutputP1;          // 4 bits audio output, calculated during phase 1

	// derived signals
	bool m_bDAR04To00CarryP2;
	bool m_bPPQCarryP2;
	bool m_bRepeatCarryP2;
	bool m_bLengthCarryP2;
	uint16_t m_RomAddrP1;         // rom address

	// output pins
	uint8_t m_uOutputP2;          // output changes on phase2
	uint16_t m_uRomAddrP2;        // address pins change on phase 2
	bool m_bBusyP1;             // busy changes on phase 1

	// input pins
	bool m_bStart;
	uint8_t m_uWord;              // 6 bit word number to be spoken

	// emulator variables
	// statistics
	uint32_t m_uNPitchPeriods;
	uint32_t m_uNVoiced;
	uint32_t m_uNControlWords;

	// diagnostic output
	uint32_t m_uPrintLevel;
};

DECLARE_DEVICE_TYPE(S14001A, s14001a_device)

#endif // MAME_SOUND_S14001A_H
