/*************************************************************************

    Lethal Enforcers

*************************************************************************/

class lethal_state : public driver_device
{
public:
	lethal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
//  UINT8 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        m_layer_colorbase[4];
	int        m_sprite_colorbase;

	/* misc */
	UINT8      m_cur_control2;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_k054539;
	device_t *m_k056832;
	device_t *m_k053244;
	device_t *m_k054000;
};

/*----------- defined in video/lethal.c -----------*/

extern void lethalen_sprite_callback(running_machine &machine, int *code, int *color, int *priority_mask);
extern void lethalen_tile_callback(running_machine &machine, int layer, int *code, int *color, int *flags);

WRITE8_HANDLER(lethalen_palette_control);

VIDEO_START(lethalen);
SCREEN_UPDATE(lethalen);
