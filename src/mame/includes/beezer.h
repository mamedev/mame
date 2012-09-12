#include "machine/6522via.h"

class beezer_state : public driver_device
{
public:
	beezer_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"){ }

	required_shared_ptr<UINT8> m_videoram;
	int m_pbus;
	int m_banklatch;

	cpu_device *m_maincpu;
	DECLARE_WRITE8_MEMBER(beezer_bankswitch_w);
	DECLARE_WRITE8_MEMBER(beezer_map_w);
	DECLARE_READ8_MEMBER(beezer_line_r);
	DECLARE_DRIVER_INIT(beezer);
};


/*----------- defined in machine/beezer.c -----------*/

extern const via6522_interface b_via_0_interface;
extern const via6522_interface b_via_1_interface;


/*----------- defined in audio/beezer.c -----------*/

class beezer_sound_device : public device_t,
                                  public device_sound_interface
{
public:
	beezer_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~beezer_sound_device() { global_free(m_token); }

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

extern const device_type BEEZER;


READ8_DEVICE_HANDLER( beezer_sh6840_r );
WRITE8_DEVICE_HANDLER( beezer_sh6840_w );
WRITE8_DEVICE_HANDLER( beezer_sfxctrl_w );
WRITE8_DEVICE_HANDLER( beezer_timer1_w );
READ8_DEVICE_HANDLER( beezer_noise_r );

/*----------- defined in video/beezer.c -----------*/

TIMER_DEVICE_CALLBACK( beezer_interrupt );
SCREEN_UPDATE_IND16( beezer );
