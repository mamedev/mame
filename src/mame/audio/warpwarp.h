// license:BSD-3-Clause
// copyright-holders:Allard van der Bas
class geebee_sound_device : public device_t,
									public device_sound_interface
{
public:
	geebee_sound_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	enum
	{
		TIMER_VOLUME_DECAY
	};

	DECLARE_WRITE8_MEMBER( sound_w );

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	// internal state
	std::unique_ptr<UINT16[]> m_decay;
	sound_stream *m_channel;
	int m_sound_latch;
	int m_sound_signal;
	int m_volume;
	emu_timer *m_volume_timer;
	int m_noise;
	int m_vcount;
};

extern const device_type GEEBEE;




class warpwarp_sound_device : public device_t,
									public device_sound_interface
{
public:
	warpwarp_sound_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	enum
	{
		TIMER_SOUND_VOLUME_DECAY,
		TIMER_MUSIC_VOLUME_DECAY
	};


	DECLARE_WRITE8_MEMBER( sound_w );
	DECLARE_WRITE8_MEMBER( music1_w );
	DECLARE_WRITE8_MEMBER( music2_w );

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	// internal state
	std::unique_ptr<INT16[]> m_decay;
	sound_stream *m_channel;
	int m_sound_latch;
	int m_music1_latch;
	int m_music2_latch;
	int m_sound_signal;
	int m_sound_volume;
	emu_timer   *m_sound_volume_timer;
	int m_music_signal;
	int m_music_volume;
	emu_timer   *m_music_volume_timer;
	int m_noise;

	int m_vcarry;
	int m_vcount;
	int m_mcarry;
	int m_mcount;
};

extern const device_type WARPWARP;
