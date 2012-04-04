/*************************************************************************

    Crime Fighters

*************************************************************************/

class crimfght_state : public driver_device
{
public:
	crimfght_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
//  UINT8 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        m_layer_colorbase[3];
	int        m_sprite_colorbase;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_k007232;
	device_t *m_k052109;
	device_t *m_k051960;
	DECLARE_WRITE8_MEMBER(crimfght_coin_w);
	DECLARE_WRITE8_MEMBER(crimfght_sh_irqtrigger_w);
	DECLARE_READ8_MEMBER(k052109_051960_r);
	DECLARE_WRITE8_MEMBER(k052109_051960_w);
};

/*----------- defined in video/crimfght.c -----------*/

extern void crimfght_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void crimfght_sprite_callback(running_machine &machine, int *code,int *color,int *priority,int *shadow);

VIDEO_START( crimfght );
SCREEN_UPDATE_IND16( crimfght );
