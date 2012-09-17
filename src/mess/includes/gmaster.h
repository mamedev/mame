#ifndef __GMASTER_H__
#define __GMASTER_H__

struct GMASTER_VIDEO
{
	UINT8 data[8];
	int index;
	int x, y;
	/*bool*/int mode; // true read does not increase address
	/*bool*/int delayed;
	UINT8 pixels[8][64/*>=62 sure*/];
};

struct GMASTER_MACHINE
{
	UINT8 ports[5];
};


class gmaster_state : public driver_device
{
public:
	gmaster_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	GMASTER_VIDEO m_video;
	GMASTER_MACHINE m_gmachine;
	DECLARE_READ8_MEMBER(gmaster_io_r);
	DECLARE_WRITE8_MEMBER(gmaster_io_w);
	DECLARE_READ8_MEMBER(gmaster_port_r);
	DECLARE_WRITE8_MEMBER(gmaster_port_w);
	DECLARE_DRIVER_INIT(gmaster);
	virtual void palette_init();
	UINT32 screen_update_gmaster(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in audio/gmaster.c -----------*/

class gmaster_sound_device : public device_t,
                                  public device_sound_interface
{
public:
	gmaster_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~gmaster_sound_device() { global_free(m_token); }

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

extern const device_type GMASTER;


int gmaster_io_callback(device_t *device, int ioline, int state);

#endif /* __GMASTER_H__ */
