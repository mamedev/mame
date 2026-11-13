// license:GPL2+
// copyright-holders:Felipe Sanches
/***************************************************************************

    Technics SX-WSA1R -- IC4, the tone generator (TC183C230002),
    PLACEHOLDER SINE backend.

    The real IC4 turns IC3 (the L7A1429 modelling LSI)'s 13-bit stream and the
    six wave mask ROMs (IC43-45, IC47-49) into audio.  THOSE ROMS ARE UNDUMPED,
    so no faithful sample can be produced.  Every gated voice is instead a plain
    sine at its own pitch, which makes the machine audible while the ROMs are
    missing.

    Nothing here is synthesis.  The sine is driven only by the register writes
    the firmware issues to 0x0010C000: PITCH (chan + 0x0400, 1/256 semitone)
    sets the frequency, OUTPUT LEVEL (chan + 0x0080) scales it, and the block-0
    lifecycle latch (0x8100 gate / 0x7E00 free) gates the voice.  When the wave
    ROMs are dumped this device is replaced behind the same two hooks.

    fs = 44100 Hz: IC4's crystal X4 is 33.8688 MHz = 768 x 44100.

***************************************************************************/

#ifndef MAME_MATSUSHITA_WSA1_TONEGEN_H
#define MAME_MATSUSHITA_WSA1_TONEGEN_H

#pragma once


class wsa1_tonegen_device : public device_t, public device_sound_interface
{
public:
	wsa1_tonegen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// Driven by the real 0x0010C000 register writes in wsa1_state::tg_data_w.
	void set_pitch(unsigned ch, uint16_t reg0400);  // chan + 0x0400, 1/256 semitone
	void set_gate(unsigned ch, bool on);            // block-0 lifecycle latch (0x8100/0x7E00)
	void set_level(unsigned ch, uint16_t reg0080);  // chan + 0x0080, log amplitude (256/octave)
	void set_env0(unsigned ch, uint16_t reg0800);   // chan + 0x0800, half of the idle marker
	void set_env1(unsigned ch, uint16_t reg0840);   // chan + 0x0840, the other half

	// The firmware's note-OFF release: it re-stages the envelope registers with a
	// release profile (Voice_Retire_Mode20 -> Dev10C_WriteSixChanRegs_FromD78A,
	// Dev10C_WriteSixChanRegs_FromD78A) WITHOUT re-gating -- the block-0 gate stays until the chip's
	// own envelope decays and the poll frees the voice.  set_release() puts the
	// voice into that decay while it is still gated.  amplitude() lets the driver
	// see when the decay has finished, so it can drop the busy bit the firmware's
	// retire path polls (wsa1.cpp tg_status_r, QUERY 1).
	void set_release(unsigned ch);                  // begin note-off decay (still gated)
	float amplitude(unsigned ch);                   // current voice amplitude, 0 == silent

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	static constexpr int      NUM_VOICES  = 64;      // `ld D,0x40`, prom_c 0xFB8116
	static constexpr uint32_t STREAM_RATE = 44100;   // IC4 crystal 33.8688 MHz = 768 x 44100
	static constexpr unsigned SINE_BITS   = 12;
	static constexpr unsigned SINE_SIZE   = 1u << SINE_BITS;

	uint32_t inc_for(uint16_t reg0400) const;

	struct voice
	{
		uint32_t phase   = 0;      // 32-bit phase accumulator; index = phase >> (32 - SINE_BITS)
		uint32_t inc     = 0;      // phase increment per output sample
		uint16_t pitch   = 0;      // last chan+0x0400 written, for save-state legibility
		uint16_t level   = 0;      // chan+0x0080: log amplitude, 0x0FF4 = unity, 0 = silent
		uint16_t env0800 = 0xff80; // chan+0x0800 \  the two together are this channel's
		uint16_t env0840 = 0xff00; // chan+0x0840 /  idle marker (0xFF80 / 0xFF00)
		bool     gate    = false;  // the block-0 lifecycle latch
		bool     released = false; // note-off decay armed while still gated (see set_release)
		float    env     = 0.0f;   // current amplitude, ramped so gate/level edges do not click
	};

	float amp_of(const voice &v) const;   // target amplitude from gate, idle marker and level

	sound_stream    *m_stream = nullptr;
	voice            m_voice[NUM_VOICES];
	float            m_sine[SINE_SIZE];
};

DECLARE_DEVICE_TYPE(WSA1_TONEGEN, wsa1_tonegen_device)

#endif // MAME_MATSUSHITA_WSA1_TONEGEN_H
