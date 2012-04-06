/***************************************************************************

 Lasso and similar hardware

***************************************************************************/

class lasso_state : public driver_device
{
public:
	lasso_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *  m_videoram;
	UINT8 *  m_colorram;
	UINT8 *  m_spriteram;
	UINT8 *  m_bitmap_ram;	/* 0x2000 bytes for a 256 x 256 x 1 bitmap */
	UINT8 *  m_back_color;
	UINT8 *  m_chip_data;
	UINT8 *  m_track_scroll;
	UINT8 *  m_last_colors;
	size_t   m_spriteram_size;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_track_tilemap;
	UINT8    m_gfxbank;		/* used by lasso, chameleo, wwjgtin and pinbo */
	UINT8    m_track_enable;	/* used by wwjgtin */

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_sn_1;
	device_t *m_sn_2;
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(pinbo_sound_command_w);
	DECLARE_READ8_MEMBER(sound_status_r);
	DECLARE_WRITE8_MEMBER(sound_select_w);
	DECLARE_WRITE8_MEMBER(lasso_videoram_w);
	DECLARE_WRITE8_MEMBER(lasso_colorram_w);
	DECLARE_WRITE8_MEMBER(lasso_flip_screen_w);
	DECLARE_WRITE8_MEMBER(lasso_video_control_w);
	DECLARE_WRITE8_MEMBER(wwjgtin_video_control_w);
	DECLARE_WRITE8_MEMBER(pinbo_video_control_w);
};


/*----------- defined in video/lasso.c -----------*/


PALETTE_INIT( lasso );
PALETTE_INIT( wwjgtin );

VIDEO_START( lasso );
VIDEO_START( wwjgtin );
VIDEO_START( pinbo );

SCREEN_UPDATE_IND16( lasso );
SCREEN_UPDATE_IND16( chameleo );
SCREEN_UPDATE_IND16( wwjgtin );
