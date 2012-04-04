/*************************************************************************

    Laser Battle / Lazarian - Cat and Mouse

*************************************************************************/

#include "machine/6821pia.h"

class laserbat_state : public driver_device
{
public:
	laserbat_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	int        m_video_page;

	/* misc */
	int        m_input_mux;
	int        m_active_8910;
	int        m_port0a;
	int        m_last_port0b;
	int        m_cb1_toggle;

	/* information for the single 32x32 sprite displayed */
	int        m_sprite_x;
	int        m_sprite_y;
	int        m_sprite_code;
	int        m_sprite_color;
	int        m_sprite_enable;

	/* sound-related */
	int        m_csound1;
	int        m_ksound1;
	int        m_ksound2;
	int        m_ksound3;
	int        m_degr;
	int        m_filt;
	int        m_a;
	int        m_us;
	int        m_bit14;

	/* device */
	device_t *m_audiocpu;
	device_t *m_s2636_1;
	device_t *m_s2636_2;
	device_t *m_s2636_3;
	pia6821_device *m_pia;
	device_t *m_sn;
	device_t *m_tms1;
	device_t *m_tms2;
	device_t *m_ay1;
	device_t *m_ay2;

	// memory
	UINT8      m_videoram[0x400];
	UINT8      m_colorram[0x400];
	DECLARE_WRITE8_MEMBER(laserbat_videoram_w);
	DECLARE_WRITE8_MEMBER(video_extra_w);
	DECLARE_WRITE8_MEMBER(sprite_x_y_w);
	DECLARE_WRITE8_MEMBER(laserbat_input_mux_w);
	DECLARE_READ8_MEMBER(laserbat_input_r);
	DECLARE_WRITE8_MEMBER(laserbat_cnteff_w);
	DECLARE_WRITE8_MEMBER(laserbat_cntmov_w);
};


/*----------- defined in audio/laserbat.c -----------*/

WRITE8_HANDLER( laserbat_csound1_w );
WRITE8_HANDLER( laserbat_csound2_w );
