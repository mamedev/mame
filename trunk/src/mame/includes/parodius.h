/*************************************************************************

    Parodius

*************************************************************************/

class parodius_state : public driver_device
{
public:
	parodius_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_ram;
//  UINT8 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        m_layer_colorbase[3];
	int        m_sprite_colorbase;
	int        m_layerpri[3];

	/* misc */
	int        m_videobank;
	//int        m_nmi_enabled;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_k053260;
	device_t *m_k052109;
	device_t *m_k053245;
	device_t *m_k053251;
};

/*----------- defined in video/parodius.c -----------*/

extern void parodius_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void parodius_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask);

SCREEN_UPDATE( parodius );
