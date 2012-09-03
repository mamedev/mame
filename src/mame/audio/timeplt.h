WRITE8_HANDLER( timeplt_sh_irqtrigger_w );

MACHINE_CONFIG_EXTERN( timeplt_sound );
MACHINE_CONFIG_EXTERN( locomotn_sound );

class timeplt_audio_device : public device_t,
                                  public device_sound_interface
{
public:
	timeplt_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~timeplt_audio_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type TIMEPLT_AUDIO;

