class fortyl_state : public driver_device
{
public:
	fortyl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *     m_videoram;
	UINT8 *     m_colorram;
	UINT8 *     m_spriteram;
	UINT8 *     m_spriteram2;
	UINT8 *     m_video_ctrl;
	size_t      m_spriteram_size;
	size_t      m_spriteram2_size;

	/* video-related */
	bitmap_t    *m_tmp_bitmap1;
	bitmap_t    *m_tmp_bitmap2;
	tilemap_t     *m_bg_tilemap;
	UINT8       m_flipscreen;
	UINT8		m_pix_redraw;
	UINT8       m_xoffset;
	UINT8       *m_pixram1;
	UINT8       *m_pixram2;
	bitmap_t    *m_pixel_bitmap1;
	bitmap_t    *m_pixel_bitmap2;
	int         m_pixram_sel;

	/* sound-related */
	int         m_sound_nmi_enable;
	int			m_pending_nmi;

	/* fake mcu */
	UINT8 *     m_mcu_ram;
	UINT8       m_from_mcu;
	int         m_mcu_sent;
	int			m_main_sent;
	UINT8       m_mcu_in[2][16];
	UINT8		m_mcu_out[2][16];
	int         m_mcu_cmd;

	/* misc */
	int         m_pix_color[4];
	UINT8       m_pix1;
	UINT8		m_pix2[2];
	UINT8       m_snd_data;
	UINT8		m_snd_flag;
	int         m_vol_ctrl[16];
	UINT8       m_snd_ctrl0;
	UINT8		m_snd_ctrl1;
	UINT8		m_snd_ctrl2;
	UINT8		m_snd_ctrl3;

	/* devices */
	device_t *m_audiocpu;
};


/*----------- defined in video/40love.c -----------*/

WRITE8_HANDLER( fortyl_bg_videoram_w );
WRITE8_HANDLER( fortyl_bg_colorram_w );
READ8_HANDLER ( fortyl_bg_videoram_r );
READ8_HANDLER ( fortyl_bg_colorram_r );
WRITE8_HANDLER( fortyl_pixram_sel_w );
READ8_HANDLER( fortyl_pixram_r );
WRITE8_HANDLER( fortyl_pixram_w );

VIDEO_START( fortyl );
SCREEN_UPDATE( fortyl );
PALETTE_INIT( fortyl );
