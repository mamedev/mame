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
	DECLARE_WRITE8_MEMBER(capbowl_rom_select_w);
	DECLARE_READ8_MEMBER(track_0_r);
	DECLARE_READ8_MEMBER(track_1_r);
	DECLARE_WRITE8_MEMBER(track_reset_w);
	DECLARE_WRITE8_MEMBER(capbowl_sndcmd_w);
	DECLARE_WRITE8_MEMBER(capbowl_tms34061_w);
	DECLARE_READ8_MEMBER(capbowl_tms34061_r);
	DECLARE_WRITE8_MEMBER(bowlrama_blitter_w);
	DECLARE_READ8_MEMBER(bowlrama_blitter_r);
};

/*----------- defined in video/capbowl.c -----------*/

VIDEO_START( capbowl );
SCREEN_UPDATE_RGB32( capbowl );


