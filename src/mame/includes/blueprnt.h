/***************************************************************************

    Blue Print

***************************************************************************/

class blueprnt_state : public driver_device
{
public:
	blueprnt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 * m_videoram;
	UINT8 * m_colorram;
	UINT8 * m_spriteram;
	UINT8 * m_scrollram;
	size_t  m_spriteram_size;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	int     m_gfx_bank;

	/* misc */
	int     m_dipsw;

	/* devices */
	device_t *m_audiocpu;
	DECLARE_READ8_MEMBER(blueprnt_sh_dipsw_r);
	DECLARE_WRITE8_MEMBER(blueprnt_sound_command_w);
	DECLARE_WRITE8_MEMBER(blueprnt_coin_counter_w);
	DECLARE_WRITE8_MEMBER(blueprnt_videoram_w);
	DECLARE_WRITE8_MEMBER(blueprnt_colorram_w);
	DECLARE_WRITE8_MEMBER(blueprnt_flipscreen_w);
};


/*----------- defined in video/blueprnt.c -----------*/


PALETTE_INIT( blueprnt );
VIDEO_START( blueprnt );
SCREEN_UPDATE_IND16( blueprnt );
