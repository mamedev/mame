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
	nds_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// we DMA from the ARM7's address space
	void set_hostspace(address_space &space)
	{
		m_space = &space;
	}

	uint32_t read(offs_t offset, uint32_t mem_mask = ~0);
	void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	// stop native sound and disconnect its output on entry to GBA mode
	void mute_all();

	// the sound clock stops while the ARM7 sleeps
	void set_sleep(bool sleeping);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream) override;

private:
	static constexpr int OUTPUT_RATE = 32768;
	static constexpr int NUM_CHANNELS = 16;
	static constexpr int NUM_CAPTURES = 2;

	enum : uint8_t
	{
		FORMAT_PCM8 = 0,
		FORMAT_PCM16,
		FORMAT_ADPCM,
		FORMAT_PSG
	};

	struct channel_t
	{
		bool active;           // source clock running, including the final sample period

		// latched from the registers at key-on
		uint8_t format;
		uint8_t repeat;         // 1 = loop, otherwise one-shot

		// live channel controls
		uint8_t volume;         // 0-127
		uint8_t vol_shift;      // 0, 1, 2 or 4
		uint8_t pan;            // 0 (left) - 127 (right)
		uint8_t duty;           // PSG square duty
		bool hold;

		uint32_t sad;           // source address
		uint32_t loopstart;     // byte offset of the loop point
		uint32_t length;        // byte length of loop start + loop

		uint32_t addr;          // next byte to read
		uint32_t frac;          // elapsed timer clocks in the current source period
		int16_t sample;         // current source sample
		uint8_t startup_delay;  // source periods before PCM/ADPCM decoding begins

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

	struct capture_t
	{
		uint32_t dad;
		uint32_t length;
		uint32_t offset;
	};

	uint32_t timer_clock() const;
	uint32_t chan_period(int ch) const;
	void update();
	void update_sync();
	void advance_channels(uint64_t ticks);
	void update_controls(int ch);
	void key_on(int ch);
	void advance_sample(channel_t &c, int ch);
	void mix(int32_t (&output)[2], int32_t (&capture)[NUM_CAPTURES]) const;
	void capture_start(int cap);
	void capture_sample(int cap, int32_t sample);
	TIMER_CALLBACK_MEMBER(capture_tick);
	TIMER_CALLBACK_MEMBER(sync_tick);

	sound_stream *m_stream;
	address_space *m_space;
	emu_timer *m_sync_timer;
	emu_timer *m_capture_timer[NUM_CAPTURES];
	attotime m_capture_remaining[NUM_CAPTURES];
	uint64_t m_last_update;
	bool m_muted;
	bool m_sleeping;

	uint32_t m_regs[0x120 / 4];
	channel_t m_chan[NUM_CHANNELS];
	capture_t m_capture[NUM_CAPTURES];
};

DECLARE_DEVICE_TYPE(NDS_SOUND, nds_sound_device)

#endif // MAME_SOUND_NDS_SOUND_H
