// license:BSD-3-Clause
// copyright-holders:Grant McNaught
/***************************************************************************

    E-mu Proteus/1 sound generator (custom chip, 32 voices)

    Behavioural model reconstructed from the Proteus/1 XR firmware; see
    proteus_snd.cpp for the register map and which parts are inferred.
    Written as integer/fixed-point arithmetic, so the output is exact and
    can be compared sample for sample with a hardware implementation.

***************************************************************************/

#ifndef MAME_EMUSYS_PROTEUS_SND_H
#define MAME_EMUSYS_PROTEUS_SND_H

#pragma once

class proteus_snd_device : public device_t, public device_sound_interface
{
public:
	proteus_snd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// 0x800000-0x8003ff: 32 voices x 16 words
	u16 read(offs_t offset, u16 mem_mask = ~0);
	void write(offs_t offset, u16 data, u16 mem_mask = ~0);

	// per-voice output/pan byte, offset = voice (CPU 0x500009 + 2 * voice)
	void pan_w(offs_t offset, u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	static constexpr unsigned VOICES = 32;
	static constexpr unsigned WORDS_PER_VOICE = 16;
	static constexpr u32 ADDR_MASK = 0xffffff;

	struct voice
	{
		u32 addr = 0;       // integer ROM word address (register +0x00)
		u16 frac = 0;       // 12-bit fractional position (internal)
		u32 pitch = 0;      // register +0x04
		u32 env = 0;        // register +0x08
		u32 loop_start = 0; // register +0x10
		u32 loop_end = 0;   // NOT of register +0x14
		u16 level = 0;      // current 12-bit float level
		u8 pan = 0x08;      // pan byte, centre
	};

	u32 reg_long(unsigned v, unsigned word) const;
	void step(voice &vc);
	s16 rom_word(u32 addr) const;
	static u32 level_to_gain(u16 level);
	s32 interpolate(const voice &vc) const;

	required_region_ptr<u16> m_waves;
	sound_stream *m_stream;
	u16 m_regs[VOICES * WORDS_PER_VOICE];
	voice m_voice[VOICES];
};

DECLARE_DEVICE_TYPE(PROTEUS_SND, proteus_snd_device)

#endif // MAME_EMUSYS_PROTEUS_SND_H
