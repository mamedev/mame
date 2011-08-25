/*************************************************************************

    Xexex

*************************************************************************/

#include <video/k053250.h>

class xexex_state : public driver_device
{
public:
	xexex_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *    m_workram;
	UINT16 *    m_spriteram;
//  UINT16 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        m_layer_colorbase[4];
	int        m_sprite_colorbase;
	int        m_layerpri[4];
	int        m_cur_alpha;

	/* misc */
	UINT16     m_cur_control2;
	INT32      m_cur_sound_region;
	INT32      m_strip_0x1a;
	int        m_suspension_active;
	int        m_resume_trigger;
	emu_timer  *m_dmadelay_timer;
	int        m_frame;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_k054539;
	device_t *m_filter1l;
	device_t *m_filter1r;
	device_t *m_filter2l;
	device_t *m_filter2r;
	device_t *m_k056832;
	device_t *m_k053246;
	k053250_t *m_k053250;
	device_t *m_k053251;
	device_t *m_k053252;
	device_t *m_k054338;
};


/*----------- defined in video/xexex.c -----------*/

extern void xexex_sprite_callback(running_machine &machine, int *code, int *color, int *priority_mask);
extern void xexex_tile_callback(running_machine &machine, int layer, int *code, int *color, int *flags);

VIDEO_START( xexex );
SCREEN_UPDATE( xexex );
