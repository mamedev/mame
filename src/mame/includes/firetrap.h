/***************************************************************************

    Fire Trap

***************************************************************************/

class firetrap_state : public driver_device
{
public:
	firetrap_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *       m_bg1videoram;
	UINT8 *       m_bg2videoram;
	UINT8 *       m_fgvideoram;
	UINT8 *       m_spriteram;
	size_t        m_spriteram_size;

	/* video-related */
	tilemap_t       *m_fg_tilemap;
	tilemap_t       *m_bg1_tilemap;
	tilemap_t       *m_bg2_tilemap;
	UINT8         m_scroll1_x[2];
	UINT8         m_scroll1_y[2];
	UINT8         m_scroll2_x[2];
	UINT8         m_scroll2_y[2];

	/* misc */
	int           m_sound_irq_enable;
	int           m_nmi_enable;
	int           m_i8751_return;
	int           m_i8751_current_command;
	int           m_i8751_init_ptr;
	int           m_msm5205next;
	int           m_adpcm_toggle;
	int           m_coin_command_pending;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_msm;
	DECLARE_WRITE8_MEMBER(firetrap_nmi_disable_w);
	DECLARE_WRITE8_MEMBER(firetrap_bankselect_w);
	DECLARE_READ8_MEMBER(firetrap_8751_bootleg_r);
	DECLARE_READ8_MEMBER(firetrap_8751_r);
	DECLARE_WRITE8_MEMBER(firetrap_8751_w);
	DECLARE_WRITE8_MEMBER(firetrap_sound_command_w);
	DECLARE_WRITE8_MEMBER(firetrap_sound_2400_w);
	DECLARE_WRITE8_MEMBER(firetrap_sound_bankselect_w);
	DECLARE_WRITE8_MEMBER(firetrap_adpcm_data_w);
	DECLARE_WRITE8_MEMBER(flip_screen_w);
	DECLARE_WRITE8_MEMBER(firetrap_fgvideoram_w);
	DECLARE_WRITE8_MEMBER(firetrap_bg1videoram_w);
	DECLARE_WRITE8_MEMBER(firetrap_bg2videoram_w);
	DECLARE_WRITE8_MEMBER(firetrap_bg1_scrollx_w);
	DECLARE_WRITE8_MEMBER(firetrap_bg1_scrolly_w);
	DECLARE_WRITE8_MEMBER(firetrap_bg2_scrollx_w);
	DECLARE_WRITE8_MEMBER(firetrap_bg2_scrolly_w);
};


/*----------- defined in video/firetrap.c -----------*/


PALETTE_INIT( firetrap );
VIDEO_START( firetrap );
SCREEN_UPDATE_IND16( firetrap );
