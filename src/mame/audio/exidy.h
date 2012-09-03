class exidy_sound_device : public device_t,
                                  public device_sound_interface
{
public:
	exidy_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	exidy_sound_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	~exidy_sound_device() { global_free(m_token); }

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

extern const device_type EXIDY;

class venture_sound_device : public exidy_sound_device
{
public:
	venture_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
};

extern const device_type EXIDY_VENTURE;

class victory_sound_device : public exidy_sound_device
{
public:
	victory_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
};

extern const device_type EXIDY_VICTORY;


READ8_DEVICE_HANDLER( exidy_sh6840_r );
WRITE8_DEVICE_HANDLER( exidy_sh6840_w );
WRITE8_DEVICE_HANDLER( exidy_sfxctrl_w );

MACHINE_CONFIG_EXTERN( venture_audio );

MACHINE_CONFIG_EXTERN( mtrap_cvsd_audio );

MACHINE_CONFIG_EXTERN( victory_audio );
READ8_DEVICE_HANDLER( victory_sound_response_r );
READ8_DEVICE_HANDLER( victory_sound_status_r );
WRITE8_DEVICE_HANDLER( victory_sound_command_w );
