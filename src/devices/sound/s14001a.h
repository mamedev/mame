// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu,R. Belmont,Zsolt Vasvari
#pragma once
/*
    Copyright (C) 2006-2013 Jonathan Gevaryahu AKA Lord Nightmare

*/
#ifndef __S14001A_H__
#define __S14001A_H__


class s14001a_device : public device_t,
									public device_sound_interface
{
public:
	s14001a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~s14001a_device() {}

	int bsy_r();        /* read BUSY pin */
	void reg_w(int data);     /* write to input latch */
	void rst_w(int data);     /* write to RESET pin */
	void set_clock(int clock);     /* set VSU-1000 clock */
	void set_volume(int volume);    /* set VSU-1000 volume control */

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// internal state
	required_region_ptr<UINT8> m_SpeechRom;
	sound_stream * m_stream;

	UINT8 m_WordInput; // value on word input bus
	UINT8 m_LatchedWord; // value latched from input bus
	UINT16 m_SyllableAddress; // address read from word table
	UINT16 m_PhoneAddress; // starting/current phone address from syllable table
	UINT8 m_PlayParams; // playback parameters from syllable table
	UINT8 m_PhoneOffset; // offset within phone
	UINT8 m_LengthCounter; // 4-bit counter which holds the inverted length of the word in phones, leftshifted by 1
	UINT8 m_RepeatCounter; // 3-bit counter which holds the inverted number of repeats per phone, leftshifted by 1
	UINT8 m_OutputCounter; // 2-bit counter to determine forward/backward and output/silence state.
	UINT8 m_machineState; // chip state machine state
	UINT8 m_nextstate; // chip state machine's new state
	UINT8 m_laststate; // chip state machine's previous state, needed for mirror increment masking
	UINT8 m_resetState; // reset line state
	UINT8 m_oddeven; // odd versus even cycle toggle
	UINT8 m_GlobalSilenceState; // same as above but for silent syllables instead of silent portions of mirrored syllables
	UINT8 m_OldDelta; // 2-bit old delta value
	UINT8 m_DACOutput; // 4-bit DAC Accumulator/output
	UINT8 m_audioout; // filtered audio output
	INT16 m_filtervals[8];
	UINT8 m_VSU1000_amp; // amplitude setting on VSU-1000 board

	void PostPhoneme();
	void s14001a_clock();
};

extern const device_type S14001A;


#endif /* __S14001A_H__ */
