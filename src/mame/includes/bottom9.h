/*************************************************************************

    Bottom of the Ninth

*************************************************************************/

class bottom9_state : public driver_device
{
public:
	bottom9_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
//  UINT8 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        m_layer_colorbase[3];
	int        m_sprite_colorbase;
	int        m_zoom_colorbase;

	/* misc */
	int        m_video_enable;
	int        m_zoomreadroms;
	int        m_k052109_selected;
	int        m_nmienable;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_k007232_1;
	device_t *m_k007232_2;
	device_t *m_k052109;
	device_t *m_k051960;
	device_t *m_k051316;
};

/*----------- defined in video/bottom9.c -----------*/

extern void bottom9_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void bottom9_sprite_callback(running_machine &machine, int *code,int *color,int *priority,int *shadow);
extern void bottom9_zoom_callback(running_machine &machine, int *code,int *color,int *flags);

VIDEO_START( bottom9 );
SCREEN_UPDATE( bottom9 );
