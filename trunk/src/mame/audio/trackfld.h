DECLARE_WRITE8_HANDLER( konami_sh_irqtrigger_w );
DECLARE_READ8_HANDLER( trackfld_sh_timer_r );
DECLARE_READ8_DEVICE_HANDLER( trackfld_speech_r );
DECLARE_WRITE8_DEVICE_HANDLER( trackfld_sound_w );
DECLARE_READ8_HANDLER( hyperspt_sh_timer_r );
DECLARE_WRITE8_DEVICE_HANDLER( hyperspt_sound_w );
DECLARE_WRITE8_HANDLER( konami_SN76496_latch_w );
DECLARE_WRITE8_DEVICE_HANDLER( konami_SN76496_w );

class trackfld_audio_device : public device_t,
									public device_sound_interface
{
public:
	trackfld_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~trackfld_audio_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type TRACKFLD_AUDIO;
