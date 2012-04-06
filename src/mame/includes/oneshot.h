
class oneshot_state : public driver_device
{
public:
	oneshot_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *        m_sprites;
	UINT16 *        m_bg_videoram;
	UINT16 *        m_mid_videoram;
	UINT16 *        m_fg_videoram;
	UINT16 *        m_scroll;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_mid_tilemap;
	tilemap_t  *m_fg_tilemap;

	/* misc */
	int m_gun_x_p1;
	int m_gun_y_p1;
	int m_gun_x_p2;
	int m_gun_y_p2;
	int m_gun_x_shift;
	int m_p1_wobble;
	int m_p2_wobble;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	DECLARE_READ16_MEMBER(oneshot_in0_word_r);
	DECLARE_READ16_MEMBER(oneshot_gun_x_p1_r);
	DECLARE_READ16_MEMBER(oneshot_gun_y_p1_r);
	DECLARE_READ16_MEMBER(oneshot_gun_x_p2_r);
	DECLARE_READ16_MEMBER(oneshot_gun_y_p2_r);
	DECLARE_WRITE16_MEMBER(oneshot_bg_videoram_w);
	DECLARE_WRITE16_MEMBER(oneshot_mid_videoram_w);
	DECLARE_WRITE16_MEMBER(oneshot_fg_videoram_w);
};

/*----------- defined in video/oneshot.c -----------*/


VIDEO_START( oneshot );
SCREEN_UPDATE_IND16( oneshot );
SCREEN_UPDATE_IND16( maddonna );
