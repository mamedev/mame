// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni, James Wallace
#pragma once

#ifndef __OKIM6376_H__
#define __OKIM6376_H__

/* an interface for the OKIM6376 and similar chips (CPU interface only) */

/* struct describing a single playing ADPCM voice */
struct ADPCMVoice
{
	UINT8 playing;          /* 1 if we are actively playing */

	UINT32 base_offset;     /* pointer to the base memory location */
	UINT32 sample;          /* current sample number */
	UINT32 count;           /* total samples to play */

	UINT32 volume;          /* output volume */
	INT32 signal;
	INT32 step;
};

class okim6376_device : public device_t,
									public device_sound_interface
{
public:
	okim6376_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~okim6376_device() {}

	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( st_w );
	DECLARE_WRITE_LINE_MEMBER( ch2_w );

	DECLARE_READ_LINE_MEMBER( busy_r );
	DECLARE_READ_LINE_MEMBER( nar_r );

	void set_frequency(int frequency);


protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// internal state
	required_region_ptr<UINT8> m_region_base;     /* pointer to the base of the region */

	#define OKIM6376_VOICES     2
	struct ADPCMVoice m_voice[OKIM6376_VOICES];
	INT32 m_command[OKIM6376_VOICES];
	INT32 m_latch;            /* Command data is held before transferring to either channel */
	UINT8 m_stage[OKIM6376_VOICES];/* If a sample is playing, flag that we have a command staged */
	sound_stream *m_stream;   /* which stream are we playing on? */
	UINT32 m_master_clock;    /* master clock frequency */
	UINT8 m_divisor;          /* can be 8,10,16, and is read out of ROM data */
	UINT8 m_channel;
	UINT8 m_nar;              /* Next Address Ready */
	UINT8 m_nartimer;
	UINT8 m_busy;
	UINT8 m_ch2;              /* 2CH pin - enables Channel 2 operation */
	UINT8 m_st;               /* STart */
	UINT8 m_st_pulses;        /* Keep track of attenuation */
	UINT8 m_ch2_update;       /* Pulse shape */
	UINT8 m_st_update;

	void oki_process(int channel, int command);
	void generate_adpcm(struct ADPCMVoice *voice, INT16 *buffer, int samples,int channel);
	void postload();
	void okim6376_state_save_register();
	void adpcm_state_save_register(struct ADPCMVoice *voice, int index);
};

extern const device_type OKIM6376;

#endif /* __OKIM6376_H__ */
