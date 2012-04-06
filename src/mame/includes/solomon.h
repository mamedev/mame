class solomon_state : public driver_device
{
public:
	solomon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	size_t m_spriteram_size;
	UINT8 *m_spriteram;
	UINT8 *m_videoram;
	UINT8 *m_colorram;
	UINT8 *m_videoram2;
	UINT8 *m_colorram2;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;

	UINT8 m_nmi_mask;
	DECLARE_WRITE8_MEMBER(solomon_sh_command_w);
	DECLARE_READ8_MEMBER(solomon_0xe603_r);
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(solomon_videoram_w);
	DECLARE_WRITE8_MEMBER(solomon_colorram_w);
	DECLARE_WRITE8_MEMBER(solomon_videoram2_w);
	DECLARE_WRITE8_MEMBER(solomon_colorram2_w);
	DECLARE_WRITE8_MEMBER(solomon_flipscreen_w);
};


/*----------- defined in video/solomon.c -----------*/


VIDEO_START( solomon );
SCREEN_UPDATE_IND16( solomon );
