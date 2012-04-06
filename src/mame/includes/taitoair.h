/*************************************************************************

    Taito Air System

*************************************************************************/

enum { TAITOAIR_FRAC_SHIFT = 16, TAITOAIR_POLY_MAX_PT = 16 };

struct taitoair_spoint {
	INT32 x, y;
};

struct taitoair_poly {
	struct taitoair_spoint p[TAITOAIR_POLY_MAX_PT];
	int pcount;
	int col;
};


class taitoair_state : public driver_device
{
public:
	taitoair_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *      m_m68000_mainram;
	UINT16 *      m_line_ram;
	UINT16 *      m_dsp_ram;	/* Shared 68000/TMS32025 RAM */
	UINT16 *      m_paletteram;

	/* video-related */
	taitoair_poly  m_q;

	/* misc */
	int           m_dsp_hold_signal;
	INT32         m_banknum;

	/* devices */
	device_t *m_audiocpu;
	device_t *m_dsp;
	device_t *m_tc0080vco;

	UINT16 *      m_gradram;
	UINT16 *      m_backregs;

	bitmap_ind16 *m_framebuffer[2];

    /* 3d info */
    INT16 m_frustumLeft;
    INT16 m_frustumBottom;
    INT16 m_eyecoordBuffer[4];  /* homogeneous */

    //bitmap_ind16 *m_buffer3d;
	DECLARE_WRITE16_MEMBER(system_control_w);
	DECLARE_READ16_MEMBER(lineram_r);
	DECLARE_WRITE16_MEMBER(lineram_w);
	DECLARE_READ16_MEMBER(dspram_r);
	DECLARE_WRITE16_MEMBER(dspram_w);
	DECLARE_READ16_MEMBER(dsp_HOLD_signal_r);
	DECLARE_WRITE16_MEMBER(dsp_HOLDA_signal_w);
	DECLARE_WRITE16_MEMBER(airsys_paletteram16_w);
	DECLARE_WRITE16_MEMBER(airsys_gradram_w);
	DECLARE_READ16_MEMBER(stick_input_r);
	DECLARE_READ16_MEMBER(stick2_input_r);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE16_MEMBER(dsp_flags_w);
	DECLARE_WRITE16_MEMBER(dsp_x_eyecoord_w);
	DECLARE_WRITE16_MEMBER(dsp_y_eyecoord_w);
	DECLARE_WRITE16_MEMBER(dsp_z_eyecoord_w);
	DECLARE_WRITE16_MEMBER(dsp_frustum_left_w);
	DECLARE_WRITE16_MEMBER(dsp_frustum_bottom_w);
	DECLARE_WRITE16_MEMBER(dsp_rasterize_w);
	DECLARE_READ16_MEMBER(dsp_x_return_r);
	DECLARE_READ16_MEMBER(dsp_y_return_r);
};


/*----------- defined in video/taitoair.c -----------*/

SCREEN_UPDATE_IND16( taitoair );
VIDEO_START( taitoair );


