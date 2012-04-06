class mjkjidai_state : public driver_device
{
public:
	mjkjidai_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_spriteram1;
	UINT8 *m_spriteram2;
	UINT8 *m_spriteram3;

	int m_keyb;
	int m_nvram_init_count;
	UINT8 *m_nvram;
	size_t m_nvram_size;
	int m_display_enable;
	tilemap_t *m_bg_tilemap;

	UINT8 m_nmi_mask;
	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_WRITE8_MEMBER(keyboard_select_w);
	DECLARE_WRITE8_MEMBER(mjkjidai_videoram_w);
	DECLARE_WRITE8_MEMBER(mjkjidai_ctrl_w);
};


/*----------- defined in video/mjkjidai.c -----------*/

VIDEO_START( mjkjidai );
SCREEN_UPDATE_IND16( mjkjidai );


