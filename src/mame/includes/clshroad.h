class clshroad_state : public driver_device
{
public:
	clshroad_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"),
		m_vram_1(*this, "vram_1"),
		m_vregs(*this, "vregs"),
		m_vram_0(*this, "vram_0"){ }

	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_vram_1;
	required_shared_ptr<UINT8> m_vregs;
	required_shared_ptr<UINT8> m_vram_0;
	tilemap_t *m_tilemap_0a;
	tilemap_t *m_tilemap_0b;
	tilemap_t *m_tilemap_1;
	DECLARE_READ8_MEMBER(clshroad_input_r);
	DECLARE_WRITE8_MEMBER(clshroad_flipscreen_w);
	DECLARE_WRITE8_MEMBER(clshroad_vram_0_w);
	DECLARE_WRITE8_MEMBER(clshroad_vram_1_w);
	DECLARE_DRIVER_INIT(firebatl);
	TILE_GET_INFO_MEMBER(get_tile_info_0a);
	TILE_GET_INFO_MEMBER(get_tile_info_0b);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_rows_extra);
	TILE_GET_INFO_MEMBER(get_tile_info_fb1);
	TILE_GET_INFO_MEMBER(get_tile_info_1);
};


/*----------- defined in video/clshroad.c -----------*/


PALETTE_INIT( firebatl );
PALETTE_INIT( clshroad );
VIDEO_START( firebatl );
VIDEO_START( clshroad );
SCREEN_UPDATE_IND16( clshroad );
