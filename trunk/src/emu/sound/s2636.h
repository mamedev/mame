/*****************************************************************************
 *
 * sound/s2636.h
 *
 ****************************************************************************/

#ifndef S2636_SOUND_H_
#define S2636_SOUND_H_



class s2636_sound_device : public device_t,
									public device_sound_interface
{
public:
	s2636_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~s2636_sound_device() { global_free(m_token); }

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

extern const device_type S2636_SOUND;

void s2636_soundport_w (device_t *device, int mode, int data);


#endif /* VC4000_H_ */
