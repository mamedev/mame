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
		: driver_device(mconfig, type, tag) ,
		m_char_ram(*this, "char_ram"),
		m_hud_ram(*this, "hud_ram"),
		m_scene_ram(*this, "scene_ram"),
		m_ground_ram(*this, "ground_ram"),
		m_object_ram(*this, "object_ram"){ }

	/* memory pointers */
	required_shared_ptr<UINT16> m_char_ram;
	required_shared_ptr<UINT16> m_hud_ram;
	required_shared_ptr<UINT16> m_scene_ram;
	required_shared_ptr<UINT16> m_ground_ram;
	required_shared_ptr<UINT16> m_object_ram;

	/* video-related */
	tilemap_t		*m_tilemap;
	UINT8			m_ground_ctrl;
	UINT16			m_scroll_h;
	UINT16			m_scroll_v;
	bitmap_ind16    *m_front_buffer;
	bitmap_ind16    *m_back_buffer;
	emu_timer		*m_bufend_timer;
	emu_timer		*m_cursor_timer;

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
	UINT8		*m_obj_pal_ram;
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
	DECLARE_WRITE16_MEMBER(adrst_w);
	DECLARE_READ16_MEMBER(main_gnd_r);
	DECLARE_WRITE16_MEMBER(main_gnd_w);
	DECLARE_READ16_MEMBER(main_obj_r);
	DECLARE_WRITE16_MEMBER(main_obj_w);
	DECLARE_WRITE16_MEMBER(tst_w);
	DECLARE_READ16_MEMBER(main_z80_r);
	DECLARE_WRITE16_MEMBER(main_z80_w);
	DECLARE_WRITE16_MEMBER(inten_w);
	DECLARE_WRITE16_MEMBER(emres_w);
	DECLARE_READ8_MEMBER(adc_r);
	DECLARE_WRITE8_MEMBER(sound_vol);
	DECLARE_WRITE8_MEMBER(ym2203_out_b);
	TILE_GET_INFO_MEMBER(get_lockon_tile_info);
};


/*----------- defined in video/lockon.c -----------*/

PALETTE_INIT( lockon );
VIDEO_START( lockon );
SCREEN_UPDATE_IND16( lockon );
SCREEN_VBLANK( lockon );

