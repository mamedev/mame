/*************************************************************************

    Lock-On hardware

*************************************************************************/

/* Calculated from CRT controller writes */
#define PIXEL_CLOCK			(XTAL_21MHz / 3)
#define FRAMEBUFFER_CLOCK	XTAL_10MHz
#define HBSTART				320
#define HBEND				0
#define HTOTAL				448
#define VBSTART				240
#define VBEND				0
#define VTOTAL				280

enum
{
	MAIN_CPU = 0,
	GROUND_CPU,
	OBJECT_CPU,
	SOUND_CPU
};

/*----------- defined in drivers/lockon.c -----------*/
extern UINT8  lockon_ctrl_reg;
extern UINT32 lockon_main_inten;

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

extern UINT16 *lockon_char_ram;
extern UINT16 *lockon_scene_ram;
extern UINT16 *lockon_object_ram;
extern UINT16 *lockon_hud_ram;
extern UINT16 *lockon_ground_ram;

extern size_t lockon_hudram_size;
extern size_t lockon_objectram_size;
extern size_t lockon_groundram_size;
