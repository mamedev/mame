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
	~s2636_sound_device() {}

	void soundport_w (int mode, int data);
	
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

private:
	// internal state
	sound_stream *m_channel;
	UINT8 m_reg[1];
	int m_size;
	int m_pos;
	unsigned m_level;
};

extern const device_type S2636_SOUND;


#endif /* S2636_SOUND_H_ */
