/*************************************************************************

    Gangbusters

*************************************************************************/

class gbusters_state : public driver_device
{
public:
	gbusters_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_ram;
//  UINT8 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        m_layer_colorbase[3];
	int        m_sprite_colorbase;

	/* misc */
	int        m_palette_selected;
	int        m_priority;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_k007232;
	device_t *m_k052109;
	device_t *m_k051960;
};

/*----------- defined in video/gbusters.c -----------*/

extern void gbusters_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags, int *priority);
extern void gbusters_sprite_callback(running_machine &machine, int *code,int *color,int *priority,int *shadow);

VIDEO_START( gbusters );
SCREEN_UPDATE( gbusters );
