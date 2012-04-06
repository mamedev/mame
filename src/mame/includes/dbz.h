/*************************************************************************

    Dragonball Z

*************************************************************************/

class dbz_state : public driver_device
{
public:
	dbz_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *      m_bg1_videoram;
	UINT16 *      m_bg2_videoram;
//  UINT16 *      m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t    *m_bg1_tilemap;
	tilemap_t    *m_bg2_tilemap;
	int          m_layer_colorbase[6];
	int          m_layerpri[5];
	int          m_sprite_colorbase;

	/* misc */
	int           m_control;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_k053246;
	device_t *m_k053251;
	device_t *m_k056832;
	device_t *m_k053936_1;
	device_t *m_k053936_2;
	DECLARE_READ16_MEMBER(dbzcontrol_r);
	DECLARE_WRITE16_MEMBER(dbzcontrol_w);
	DECLARE_WRITE16_MEMBER(dbz_sound_command_w);
	DECLARE_WRITE16_MEMBER(dbz_sound_cause_nmi);
	DECLARE_WRITE16_MEMBER(dbz_bg2_videoram_w);
	DECLARE_WRITE16_MEMBER(dbz_bg1_videoram_w);
};


/*----------- defined in video/dbz.c -----------*/

extern void dbz_sprite_callback(running_machine &machine, int *code, int *color, int *priority_mask);
extern void dbz_tile_callback(running_machine &machine, int layer, int *code, int *color, int *flags);


VIDEO_START(dbz);
SCREEN_UPDATE_IND16(dbz);
