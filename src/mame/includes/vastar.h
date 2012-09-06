class vastar_state : public driver_device
{
public:
	vastar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_bg1videoram(*this, "bg1videoram"),
		m_bg2videoram(*this, "bg2videoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bg1_scroll(*this, "bg1_scroll"),
		m_bg2_scroll(*this, "bg2_scroll"),
		m_sprite_priority(*this, "sprite_priority"),
		m_sharedram(*this, "sharedram"),
		m_spriteram1(*this, "spriteram1"),
		m_spriteram2(*this, "spriteram2"),
		m_spriteram3(*this, "spriteram3"){ }

	required_shared_ptr<UINT8> m_bg1videoram;
	required_shared_ptr<UINT8> m_bg2videoram;
	required_shared_ptr<UINT8> m_fgvideoram;
	required_shared_ptr<UINT8> m_bg1_scroll;
	required_shared_ptr<UINT8> m_bg2_scroll;
	required_shared_ptr<UINT8> m_sprite_priority;
	required_shared_ptr<UINT8> m_sharedram;
	required_shared_ptr<UINT8> m_spriteram1;
	required_shared_ptr<UINT8> m_spriteram2;
	required_shared_ptr<UINT8> m_spriteram3;

	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg1_tilemap;
	tilemap_t *m_bg2_tilemap;


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
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
};


/*----------- defined in video/vastar.c -----------*/


VIDEO_START( vastar );
SCREEN_UPDATE_IND16( vastar );
