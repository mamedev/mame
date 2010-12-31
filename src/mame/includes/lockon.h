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
	lockon_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16	*char_ram;
	UINT16	*hud_ram;
	UINT16	*scene_ram;
	UINT16	*ground_ram;
	UINT16	*object_ram;

	size_t	hudram_size;
	size_t	objectram_size;
	size_t	groundram_size;

	/* video-related */
	tilemap_t   *tilemap;
	UINT8	      ground_ctrl;
	UINT16      scroll_h;
	UINT16      scroll_v;
	bitmap_t    *front_buffer;
	bitmap_t    *back_buffer;
	emu_timer   *bufend_timer;
	emu_timer   *cursor_timer;

	/* Rotation Control */
	UINT16      xsal;
	UINT16      x0ll;
	UINT16      dx0ll;
	UINT16      dxll;
	UINT16      ysal;
	UINT16      y0ll;
	UINT16      dy0ll;
	UINT16      dyll;

	/* Object palette RAM control */
	UINT32      iden;
	UINT8	*     obj_pal_ram;
	UINT32      obj_pal_latch;
	UINT32      obj_pal_addr;

	/* misc */
	UINT8       ctrl_reg;
	UINT32      main_inten;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
	device_t *ground;
	device_t *object;
	device_t *f2203_1l;
	device_t *f2203_2l;
	device_t *f2203_3l;
	device_t *f2203_1r;
	device_t *f2203_2r;
	device_t *f2203_3r;
};


/*----------- defined in video/lockon.c -----------*/

PALETTE_INIT( lockon );
VIDEO_START( lockon );
VIDEO_UPDATE( lockon );
VIDEO_EOF( lockon );
READ16_HANDLER( lockon_crtc_r );
WRITE16_HANDLER( lockon_crtc_w );
WRITE16_HANDLER( lockon_rotate_w );
WRITE16_HANDLER( lockon_fb_clut_w );
WRITE16_HANDLER( lockon_scene_h_scr_w );
WRITE16_HANDLER( lockon_scene_v_scr_w );
WRITE16_HANDLER( lockon_ground_ctrl_w );
WRITE16_HANDLER( lockon_char_w );

WRITE16_HANDLER( lockon_tza112_w );
READ16_HANDLER( lockon_obj_4000_r );
WRITE16_HANDLER( lockon_obj_4000_w );
