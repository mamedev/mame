// license:BSD-3-Clause
// copyright-holders:Ed Bernard, Jonathan Gevaryahu, hap
// thanks-to:Kevin Horton
/*
    SSi TSI S14001A speech IC emulator
*/

#ifndef __S14001A_H__
#define __S14001A_H__

#define MCFG_S14001A_BSY_HANDLER(_devcb) \
	devcb = &s14001a_device::set_bsy_handler(*device, DEVCB_##_devcb);

#define MCFG_S14001A_EXT_READ_HANDLER(_devcb) \
	devcb = &s14001a_device::set_ext_read_handler(*device, DEVCB_##_devcb);


class s14001a_device : public device_t,
						public device_sound_interface
{
public:
	s14001a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~s14001a_device() {}

	// static configuration helpers
	template<class _Object> static devcb_base &set_bsy_handler(device_t &device, _Object object) { return downcast<s14001a_device &>(device).m_bsy_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_ext_read_handler(device_t &device, _Object object) { return downcast<s14001a_device &>(device).m_ext_read_handler.set_callback(object); }

	DECLARE_READ_LINE_MEMBER(busy_r);   // /BUSY (pin 40)
	DECLARE_READ_LINE_MEMBER(romen_r);  // ROM /EN (pin 9)
	DECLARE_WRITE_LINE_MEMBER(start_w); // START (pin 10)
	DECLARE_WRITE8_MEMBER(data_w);      // 6-bit word

	void set_clock(UINT32 clock);       // set new CLK frequency
	void force_update();                // update stream, eg. before external ROM bankswitch

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	required_region_ptr<UINT8> m_SpeechRom;
	sound_stream * m_stream;

	devcb_write_line m_bsy_handler;
	devcb_read8 m_ext_read_handler;

	UINT8 readmem(UINT16 offset, bool phase);
	bool Clock(); // called once to toggle external clock twice

	// emulator helper functions
	UINT8 Mux8To2(bool bVoicedP2, UINT8 uPPQtrP2, UINT8 uDeltaAdrP2, UINT8 uRomDataP2);
	void CalculateIncrement(bool bVoicedP2, UINT8 uPPQtrP2, bool bPPQStartP2, UINT8 uDeltaP2, UINT8 uDeltaOldP2, UINT8 &uDeltaOldP1, UINT8 &uIncrementP2, bool &bAddP2);
	UINT8 CalculateOutput(bool bVoicedP2, bool bXSilenceP2, UINT8 uPPQtrP2, bool bPPQStartP2, UINT8 uLOutputP2, UINT8 uIncrementP2, bool bAddP2);
	void ClearStatistics();
	void GetStatistics(UINT32 &uNPitchPeriods, UINT32 &uNVoiced, UINT32 uNControlWords);
	void SetPrintLevel(UINT32 uPrintLevel) { m_uPrintLevel = uPrintLevel; }

	// internal state
	bool m_bPhase1; // 1 bit internal clock

	enum states
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

	// registers
	states m_uStateP1;          // 3 bits
	states m_uStateP2;

	UINT16 m_uDAR13To05P1;      // 9 MSBs of delta address register
	UINT16 m_uDAR13To05P2;      // incrementing uDAR05To13 advances ROM address by 8 bytes

	UINT16 m_uDAR04To00P1;      // 5 LSBs of delta address register
	UINT16 m_uDAR04To00P2;      // 3 address ROM, 2 mux 8 bits of data into 2 bit delta
								// carry indicates end of quarter pitch period (32 cycles)

	UINT16 m_uCWARP1;           // 12 bits Control Word Address Register (syllable)
	UINT16 m_uCWARP2;

	bool m_bStopP1;
	bool m_bStopP2;
	bool m_bVoicedP1;
	bool m_bVoicedP2;
	bool m_bSilenceP1;
	bool m_bSilenceP2;
	UINT8 m_uLengthP1;          // 7 bits, upper three loaded from ROM length
	UINT8 m_uLengthP2;          // middle two loaded from ROM repeat and/or uXRepeat
								// bit 0 indicates mirror in voiced mode
								// bit 1 indicates internal silence in voiced mode
								// incremented each pitch period quarter

	UINT8 m_uXRepeatP1;         // 2 bits, loaded from ROM repeat
	UINT8 m_uXRepeatP2;
	UINT8 m_uDeltaOldP1;        // 2 bit old delta
	UINT8 m_uDeltaOldP2;
	UINT8 m_uOutputP1;          // 4 bits audio output, calculated during phase 1

	// derived signals
	bool m_bDAR04To00CarryP2;
	bool m_bPPQCarryP2;
	bool m_bRepeatCarryP2;
	bool m_bLengthCarryP2;
	UINT16 m_RomAddrP1;         // rom address

	// output pins
	UINT8 m_uOutputP2;          // output changes on phase2
	UINT16 m_uRomAddrP2;        // address pins change on phase 2
	bool m_bBusyP1;             // busy changes on phase 1

	// input pins
	bool m_bStart;
	UINT8 m_uWord;              // 6 bit word noumber to be spoken

	// emulator variables
	// statistics
	UINT32 m_uNPitchPeriods;
	UINT32 m_uNVoiced;
	UINT32 m_uNControlWords;

	// diagnostic output
	UINT32 m_uPrintLevel;
};

extern const device_type S14001A;


#endif /* __S14001A_H__ */
