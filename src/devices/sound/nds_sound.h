// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    nds_sound.h
	Nintendo DS sound: 16 channels of PCM, ADPCM, PSG, or noise

***************************************************************************/
#ifndef MAME_SOUND_NDS_SOUND_H
#define MAME_SOUND_NDS_SOUND_H

#pragma once


class nds_sound_device : public device_t, public device_sound_interface
{
public:
	nds_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// we DMA from the ARM7's address space
	void set_hostspace(address_space &space) { m_space = &space; }

	uint32_t read(offs_t offset, uint32_t mem_mask = ~0);
	void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	// stop all channels, used on entry to GBA mode
	void mute_all();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream) override;

private:
	static constexpr int OUTPUT_RATE = 32768;
	static constexpr int NUM_CHANNELS = 16;

	enum : uint8_t
	{
		FORMAT_PCM8 = 0,
		FORMAT_PCM16,
		FORMAT_ADPCM,
		FORMAT_PSG
	};

	struct channel_t
	{
		// latched from the registers at key-on
		bool active;
		uint8_t format;
		uint8_t repeat;         // 1 = loop, otherwise one-shot
		uint8_t volume;         // 0-127
		uint8_t vol_shift;      // 0, 1, 2 or 4
		uint8_t pan;            // 0 (left) - 127 (right)
		uint8_t duty;           // PSG square duty
		bool hold;

		uint32_t sad;           // source address
		uint32_t loopstart;     // byte offset of the loop point
		uint32_t length;        // byte length of loop start + loop

		uint32_t addr;          // next byte to read
		uint32_t frac;          // 16.16 resampling accumulator
		uint32_t step;          // 16.16 source samples per output sample
		int16_t sample;         // current source sample

		// ADPCM state
		int32_t adpcm_val;
		int32_t adpcm_index;
		int32_t adpcm_loop_val;
		int32_t adpcm_loop_index;
		bool adpcm_loop_saved;
		bool adpcm_low_nibble;

		// PSG
		uint32_t psg_phase;
		uint16_t noise_lfsr;
	};

	uint32_t chan_freq(int ch) const;
	void key_on(int ch);
	void advance_sample(channel_t &c, int ch);

	sound_stream *m_stream;
	address_space *m_space;

	uint32_t m_regs[0x120 / 4];
	channel_t m_chan[NUM_CHANNELS];
};

DECLARE_DEVICE_TYPE(NDS_SOUND, nds_sound_device)

#endif // MAME_SOUND_NDS_SOUND_H
