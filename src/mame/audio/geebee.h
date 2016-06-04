// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller

class geebee_sound_device : public device_t,
									public device_sound_interface
{
public:
	geebee_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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
