// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni, James Wallace
#ifndef MAME_SOUND_OKIM6376_H
#define MAME_SOUND_OKIM6376_H

#pragma once

#include "dirom.h"

/* an interface for the OKIM6376 and similar chips (CPU interface only) */

class okim6376_device : public device_t, public device_sound_interface, public device_rom_interface<23, 0, 0, ENDIANNESS_BIG>
{
public:
	okim6376_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(uint8_t data);

	void st_w(int state);
	void ch2_w(int state);

	int busy_r();
	int nar_r();

protected:
	okim6376_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int addrbits);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;
	virtual void device_post_load() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	// device_rom_interface overrides
	virtual void rom_bank_pre_change() override;

	virtual offs_t get_start_position(int channel);
	virtual u32 get_sample_rate();

	/* struct describing a single playing ADPCM voice */
	struct ADPCMVoice
	{
		uint8_t playing;          /* 1 if we are actively playing */

		uint32_t base_offset;     /* pointer to the base memory location */
		uint32_t sample;          /* current sample number */
		uint32_t count;           /* total samples to play */

		uint32_t volume;          /* output volume */
		int32_t signal;
		int32_t step;

		void reset();
		int16_t clock(uint8_t nibble);
	};

	// internal state
	static constexpr unsigned OKIM6376_VOICES = 2;
	struct ADPCMVoice m_voice[OKIM6376_VOICES];
	int32_t m_command[OKIM6376_VOICES];
	int32_t m_latch;            /* Command data is held before transferring to either channel */
	uint8_t m_stage[OKIM6376_VOICES];/* If a sample is playing, flag that we have a command staged */
	sound_stream *m_stream;   /* which stream are we playing on? */
	uint8_t m_divisor;          /* can be 8,10,16, and is read out of ROM data */
	uint8_t m_channel;
	uint8_t m_nar;              /* Next Address Ready */
	uint8_t m_nartimer;
	uint8_t m_busy;
	uint8_t m_ch2;              /* 2CH pin - enables Channel 2 operation */
	uint8_t m_st;               /* STart */
	uint8_t m_st_pulses;        /* Keep track of attenuation */
	uint8_t m_ch2_update;       /* Pulse shape */
	uint8_t m_st_update;

	void oki_process(int channel, int command);
	void generate_adpcm(struct ADPCMVoice *voice, int16_t *buffer, int samples,int channel);
	void okim6376_state_save_register();
	void adpcm_state_save_register(struct ADPCMVoice *voice, int index);
};

class okim6650_device : public okim6376_device
{
public:
	okim6650_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void cmd_w(int state);

protected:
	virtual offs_t get_start_position(int channel) override;
	virtual u32 get_sample_rate() override;
};

DECLARE_DEVICE_TYPE(OKIM6376, okim6376_device)
DECLARE_DEVICE_TYPE(OKIM6650, okim6650_device)

#endif // MAME_SOUND_OKIM6376_H
