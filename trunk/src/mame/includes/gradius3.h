/*************************************************************************

    Gradius 3

*************************************************************************/

class gradius3_state : public driver_device
{
public:
	gradius3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *    m_gfxram;
//  UINT16 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	int         m_layer_colorbase[3];
	int         m_sprite_colorbase;

	/* misc */
	int         m_priority;
	int         m_irqAen;
	int         m_irqBmask;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_subcpu;
	device_t *m_k007232;
	device_t *m_k052109;
	device_t *m_k051960;
};

/*----------- defined in video/gradius3.c -----------*/

extern void gradius3_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask,int *shadow);
extern void gradius3_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);

READ16_HANDLER( gradius3_gfxrom_r );
WRITE16_HANDLER( gradius3_gfxram_w );

VIDEO_START( gradius3 );
SCREEN_UPDATE( gradius3 );
