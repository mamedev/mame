DECLARE_WRITE8_DEVICE_HANDLER( pleiads_sound_control_a_w );
DECLARE_WRITE8_DEVICE_HANDLER( pleiads_sound_control_b_w );
DECLARE_WRITE8_DEVICE_HANDLER( pleiads_sound_control_c_w );

class pleiads_sound_device : public device_t,
                                  public device_sound_interface
{
public:
	pleiads_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	pleiads_sound_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	~pleiads_sound_device() { global_free(m_token); }

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

extern const device_type PLEIADS;

class naughtyb_sound_device : public pleiads_sound_device
{
public:
	naughtyb_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
};

extern const device_type NAUGHTYB;

class popflame_sound_device : public pleiads_sound_device
{
public:
	popflame_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
};

extern const device_type POPFLAME;

