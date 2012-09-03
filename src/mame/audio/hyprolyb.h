WRITE8_DEVICE_HANDLER( hyprolyb_adpcm_w );
READ8_DEVICE_HANDLER( hyprolyb_adpcm_busy_r );

MACHINE_CONFIG_EXTERN( hyprolyb_adpcm );

class hyprolyb_adpcm_device : public device_t,
                                  public device_sound_interface
{
public:
	hyprolyb_adpcm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~hyprolyb_adpcm_device() { global_free(m_token); }

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

extern const device_type HYPROLYB_ADPCM;

