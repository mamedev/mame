// license:BSD-3-Clause
// copyright-holders:Allard van der Bas
/* 8 voices max */
#define MAX_VOICES 8

/* this structure defines the parameters for a channel */
struct wp_sound_channel
{
	int frequency;
	int counter;
	int volume;
	const UINT8 *wave;
	int oneshot;
	int oneshotplaying;
};

class wiping_sound_device : public device_t,
									public device_sound_interface
{
public:
	wiping_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~wiping_sound_device() {}

	DECLARE_WRITE8_MEMBER( sound_w );

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// internal state

	/* data about the sound system */
	wp_sound_channel m_channel_list[MAX_VOICES];
	wp_sound_channel *m_last_channel;

	/* global sound parameters */
	const UINT8 *m_sound_prom;
	const UINT8 *m_sound_rom;
	int m_num_voices;
	int m_sound_enable;
	sound_stream *m_stream;

	/* mixer tables and internal buffers */
	INT16 *m_mixer_table;
	INT16 *m_mixer_lookup;
	short *m_mixer_buffer;
	short *m_mixer_buffer_2;

	UINT8 m_soundregs[0x4000];

	void make_mixer_table(int voices, int gain);
};

extern const device_type WIPING;
