class vastar_state : public driver_device
{
public:
	vastar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_spriteram1;
	UINT8 *m_spriteram2;
	UINT8 *m_spriteram3;

	UINT8 *m_bg1videoram;
	UINT8 *m_bg2videoram;
	UINT8 *m_fgvideoram;
	UINT8 *m_bg1_scroll;
	UINT8 *m_bg2_scroll;
	UINT8 *m_sprite_priority;

	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg1_tilemap;
	tilemap_t *m_bg2_tilemap;

	UINT8 *m_sharedram;

	UINT8 m_nmi_mask;
};


/*----------- defined in video/vastar.c -----------*/

WRITE8_HANDLER( vastar_bg1videoram_w );
WRITE8_HANDLER( vastar_bg2videoram_w );
WRITE8_HANDLER( vastar_fgvideoram_w );
READ8_HANDLER( vastar_bg1videoram_r );
READ8_HANDLER( vastar_bg2videoram_r );

VIDEO_START( vastar );
SCREEN_UPDATE( vastar );
