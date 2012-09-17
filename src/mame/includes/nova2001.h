class nova2001_state : public driver_device
{
public:
	nova2001_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_fg_videoram(*this, "fg_videoram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_spriteram(*this, "spriteram"){ }

	UINT8 m_ninjakun_io_a002_ctrl;
	optional_shared_ptr<UINT8> m_fg_videoram;
	required_shared_ptr<UINT8> m_bg_videoram;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	optional_shared_ptr<UINT8> m_spriteram;
	DECLARE_WRITE8_MEMBER(ninjakun_cpu1_io_A002_w);
	DECLARE_WRITE8_MEMBER(ninjakun_cpu2_io_A002_w);
	DECLARE_WRITE8_MEMBER(ninjakun_paletteram_w);
	DECLARE_WRITE8_MEMBER(nova2001_fg_videoram_w);
	DECLARE_WRITE8_MEMBER(nova2001_bg_videoram_w);
	DECLARE_WRITE8_MEMBER(ninjakun_bg_videoram_w);
	DECLARE_READ8_MEMBER(ninjakun_bg_videoram_r);
	DECLARE_WRITE8_MEMBER(nova2001_scroll_x_w);
	DECLARE_WRITE8_MEMBER(nova2001_scroll_y_w);
	DECLARE_WRITE8_MEMBER(nova2001_flipscreen_w);
	DECLARE_WRITE8_MEMBER(pkunwar_flipscreen_w);
	DECLARE_CUSTOM_INPUT_MEMBER(ninjakun_io_A002_ctrl_r);
	DECLARE_DRIVER_INIT(raiders5);
	DECLARE_DRIVER_INIT(pkunwar);
	TILE_GET_INFO_MEMBER(nova2001_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(nova2001_get_fg_tile_info);
	TILE_GET_INFO_MEMBER(ninjakun_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(ninjakun_get_fg_tile_info);
	TILE_GET_INFO_MEMBER(pkunwar_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(raiders5_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(raiders5_get_fg_tile_info);
	DECLARE_VIDEO_START(nova2001);
	DECLARE_PALETTE_INIT(nova2001);
	DECLARE_MACHINE_START(ninjakun);
	DECLARE_VIDEO_START(ninjakun);
	DECLARE_VIDEO_START(pkunwar);
	DECLARE_VIDEO_START(raiders5);
	UINT32 screen_update_nova2001(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_ninjakun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_pkunwar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_raiders5(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
