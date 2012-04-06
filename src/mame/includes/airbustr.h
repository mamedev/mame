/*************************************************************************

    Air Buster

*************************************************************************/

class airbustr_state : public driver_device
{
public:
	airbustr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_videoram2;
	UINT8 *    m_colorram;
	UINT8 *    m_colorram2;
	UINT8 *    m_paletteram;
	UINT8 *    m_devram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	tilemap_t    *m_fg_tilemap;
	bitmap_ind16 m_sprites_bitmap;
	int        m_bg_scrollx;
	int        m_bg_scrolly;
	int        m_fg_scrollx;
	int        m_fg_scrolly;
	int        m_highbits;

	/* misc */
	int        m_soundlatch_status;
	int        m_soundlatch2_status;

	/* devices */
	device_t *m_master;
	device_t *m_slave;
	device_t *m_audiocpu;
	device_t *m_pandora;
	DECLARE_READ8_MEMBER(devram_r);
	DECLARE_WRITE8_MEMBER(master_nmi_trigger_w);
	DECLARE_WRITE8_MEMBER(master_bankswitch_w);
	DECLARE_WRITE8_MEMBER(slave_bankswitch_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_READ8_MEMBER(soundcommand_status_r);
	DECLARE_READ8_MEMBER(soundcommand_r);
	DECLARE_READ8_MEMBER(soundcommand2_r);
	DECLARE_WRITE8_MEMBER(soundcommand_w);
	DECLARE_WRITE8_MEMBER(soundcommand2_w);
	DECLARE_WRITE8_MEMBER(airbustr_paletteram_w);
	DECLARE_WRITE8_MEMBER(airbustr_coin_counter_w);
	DECLARE_WRITE8_MEMBER(airbustr_videoram_w);
	DECLARE_WRITE8_MEMBER(airbustr_colorram_w);
	DECLARE_WRITE8_MEMBER(airbustr_videoram2_w);
	DECLARE_WRITE8_MEMBER(airbustr_colorram2_w);
	DECLARE_WRITE8_MEMBER(airbustr_scrollregs_w);
};


/*----------- defined in video/airbustr.c -----------*/


VIDEO_START( airbustr );
SCREEN_UPDATE_IND16( airbustr );
SCREEN_VBLANK( airbustr );
