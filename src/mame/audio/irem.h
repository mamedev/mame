WRITE8_HANDLER( irem_sound_cmd_w );

MACHINE_CONFIG_EXTERN( m52_sound_c_audio );
MACHINE_CONFIG_EXTERN( m52_large_audio );
MACHINE_CONFIG_EXTERN( m62_audio );

class irem_audio_device : public device_t,
                                  public device_sound_interface
{
public:
	irem_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~irem_audio_device() { global_free(m_token); }

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

extern const device_type IREM_AUDIO;

