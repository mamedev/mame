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
	DECLARE_WRITE8_MEMBER(vastar_hold_cpu2_w);
	DECLARE_READ8_MEMBER(vastar_sharedram_r);
	DECLARE_WRITE8_MEMBER(vastar_sharedram_w);
	DECLARE_WRITE8_MEMBER(flip_screen_w);
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(vastar_fgvideoram_w);
	DECLARE_WRITE8_MEMBER(vastar_bg1videoram_w);
	DECLARE_WRITE8_MEMBER(vastar_bg2videoram_w);
	DECLARE_READ8_MEMBER(vastar_bg1videoram_r);
	DECLARE_READ8_MEMBER(vastar_bg2videoram_r);
};


/*----------- defined in video/vastar.c -----------*/


VIDEO_START( vastar );
SCREEN_UPDATE_IND16( vastar );
