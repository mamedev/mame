/*************************************************************************

    Rastan

*************************************************************************/

class rastan_state : public driver_device
{
public:
	rastan_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
//  UINT16 *    paletteram; // this currently uses generic palette handlers

	/* video-related */
	UINT16      m_sprite_ctrl;
	UINT16      m_sprites_flipscreen;

	/* misc */
	int         m_adpcm_pos;
	int         m_adpcm_data;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_pc090oj;
	device_t *m_pc080sn;
};


/*----------- defined in video/rastan.c -----------*/

WRITE16_HANDLER( rastan_spritectrl_w );

SCREEN_UPDATE( rastan );
