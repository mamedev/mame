// license:BSD-3-Clause
// copyright-holders:Wouter van Nifterick

// Yamaha YMP706-F (XT329A00) FS tone generator.
//
// 0x400 byte window, A0-A9, D0-D7. 
// Register 0x3FF selects the channel (0..15). 
// Per-channel registers are then byte addresses. 
// A 16-bit value is the high byte at `reg` and the low byte at `reg + 8`. 
// Operator registers are 8-byte stripes. 
// Pitch, portamento and Fseq are computed by the host CPU and stored here.
//
// 0xFC/0xFD and 0xFA/0xFB both force envelope segment 4 on a channel mask 
// (even address high byte, applied when the odd low byte arrives).
// The attack starts when the level stripe is written after that.
// 0x270 is 10, or 11 to insert the external filter on the channel loop.
//
// Outputs are four stereo buses (BUS_*): 
// dry, insertion, variation send and reverb send, for the effect VOP3. 

#ifndef MAME_SOUND_YMP706_H
#define MAME_SOUND_YMP706_H

#pragma once

class yss236_device;

class ymp706_device : public device_t, public device_sound_interface
{
public:
	enum { BUS_DRY = 0, BUS_INS = 2, BUS_VAR = 4, BUS_REV = 6, BUSES = 8 };

	ymp706_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 24'576'000);

	void map(address_map &map) ATTR_COLD;

	// Mix the bus inputs (an upstream chip) into the output.
	void set_mix_input(bool mix) { m_mix_input = mix; }

	// voice_base is 0 or 16, so the two chips do not share filter state.
	void set_filter(yss236_device &filter, int voice_base);

	// Render pending samples before the filter changes state.
	void sync() { m_stream->update(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	static constexpr int NOTES = 16;
	static constexpr int OPS = 8;
	static constexpr int MEM = 0x400;

	struct note
	{
		u8 mem[MEM];
		u32 phase[OPS];
		u32 nphase[OPS];
		// Formant carrier, and the carrier of the grain one period behind.
		u32 cphase[OPS];
		u32 pphase[OPS];
		u8 grain[OPS];
		float eg[OPS];
		float ueg[OPS];
		float nz[OPS][2];
		float prev;
		// 0 is idle. 1..4 are the level segments. 3 holds, 4 releases.
		u8 stage[OPS];
		u8 ustage[OPS];
	};

	sound_stream *m_stream;
	yss236_device *m_filter;
	int m_voice_base;
	bool m_mix_input;
	u8 m_bus[MEM];
	int m_sel;
	note m_note[NOTES];
	u32 m_noise;

	u32 sample_rate() const { return clock() / 512; }

	u8 reg_r(offs_t offset);
	void reg_w(offs_t offset, u8 data);
	static bool chip_wide(offs_t offset);
	void release_mask(u16 mask);
	static void begin_attack(note &n, int op, bool unvoiced);
	float render_note(note &n);
};

DECLARE_DEVICE_TYPE(YMP706, ymp706_device)

#endif // MAME_SOUND_YMP706_H
