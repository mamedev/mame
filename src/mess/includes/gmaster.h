#ifndef __GMASTER_H__
#define __GMASTER_H__

typedef struct
{
	UINT8 data[8];
	int index;
	int x, y;
	/*bool*/int mode; // true read does not increase address
	/*bool*/int delayed;
	UINT8 pixels[8][64/*>=62 sure*/];
} GMASTER_VIDEO;

typedef struct
{
	UINT8 ports[5];
} GMASTER_MACHINE;


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
};


/*----------- defined in audio/gmaster.c -----------*/

DECLARE_LEGACY_SOUND_DEVICE(GMASTER, gmaster_sound);

int gmaster_io_callback(device_t *device, int ioline, int state);

#endif /* __GMASTER_H__ */
