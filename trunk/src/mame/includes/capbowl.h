/*************************************************************************

    Coors Light Bowling/Bowl-O-Rama hardware

*************************************************************************/

#include "machine/nvram.h"

class capbowl_state : public driver_device
{
public:
	capbowl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	void init_nvram(nvram_device &nvram, void *base, size_t size);

	/* memory pointers */
	UINT8 *  m_rowaddress;

	/* video-related */
	offs_t m_blitter_addr;

	/* input-related */
	UINT8 m_last_trackball_val[2];

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
};

/*----------- defined in video/capbowl.c -----------*/

VIDEO_START( capbowl );
SCREEN_UPDATE( capbowl );

WRITE8_HANDLER( bowlrama_blitter_w );
READ8_HANDLER( bowlrama_blitter_r );

WRITE8_HANDLER( capbowl_tms34061_w );
READ8_HANDLER( capbowl_tms34061_r );
