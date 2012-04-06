/*************************************************************************

    Lock-On hardware

*************************************************************************/

/* Calculated from CRT controller writes */
#define PIXEL_CLOCK            (XTAL_21MHz / 3)
#define FRAMEBUFFER_CLOCK      XTAL_10MHz
#define HBSTART                320
#define HBEND                  0
#define HTOTAL                 448
#define VBSTART                240
#define VBEND                  0
#define VTOTAL                 280


class lockon_state : public driver_device
{
public:
	lockon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16	*m_char_ram;
	UINT16	*m_hud_ram;
	UINT16	*m_scene_ram;
	UINT16	*m_ground_ram;
	UINT16	*m_object_ram;

	size_t	m_hudram_size;
	size_t	m_objectram_size;
	size_t	m_groundram_size;

	/* video-related */
	tilemap_t   *m_tilemap;
	UINT8	      m_ground_ctrl;
	UINT16      m_scroll_h;
	UINT16      m_scroll_v;
	bitmap_ind16    *m_front_buffer;
	bitmap_ind16    *m_back_buffer;
	emu_timer   *m_bufend_timer;
	emu_timer   *m_cursor_timer;

	/* Rotation Control */
	UINT16      m_xsal;
	UINT16      m_x0ll;
	UINT16      m_dx0ll;
	UINT16      m_dxll;
	UINT16      m_ysal;
	UINT16      m_y0ll;
	UINT16      m_dy0ll;
	UINT16      m_dyll;

	/* Object palette RAM control */
	UINT32      m_iden;
	UINT8	*     m_obj_pal_ram;
	UINT32      m_obj_pal_latch;
	UINT32      m_obj_pal_addr;

	/* misc */
	UINT8       m_ctrl_reg;
	UINT32      m_main_inten;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_ground;
	device_t *m_object;
	device_t *m_f2203_1l;
	device_t *m_f2203_2l;
	device_t *m_f2203_3l;
	device_t *m_f2203_1r;
	device_t *m_f2203_2r;
	device_t *m_f2203_3r;
	DECLARE_READ16_MEMBER(lockon_crtc_r);
	DECLARE_WRITE16_MEMBER(lockon_crtc_w);
	DECLARE_WRITE16_MEMBER(lockon_char_w);
	DECLARE_WRITE16_MEMBER(lockon_scene_h_scr_w);
	DECLARE_WRITE16_MEMBER(lockon_scene_v_scr_w);
	DECLARE_WRITE16_MEMBER(lockon_ground_ctrl_w);
	DECLARE_WRITE16_MEMBER(lockon_tza112_w);
	DECLARE_READ16_MEMBER(lockon_obj_4000_r);
	DECLARE_WRITE16_MEMBER(lockon_obj_4000_w);
	DECLARE_WRITE16_MEMBER(lockon_fb_clut_w);
	DECLARE_WRITE16_MEMBER(lockon_rotate_w);
};


/*----------- defined in video/lockon.c -----------*/

PALETTE_INIT( lockon );
VIDEO_START( lockon );
SCREEN_UPDATE_IND16( lockon );
SCREEN_VBLANK( lockon );

