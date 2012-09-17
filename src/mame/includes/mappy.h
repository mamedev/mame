class mappy_state : public driver_device
{
public:
	mappy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"){ }

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	tilemap_t *m_bg_tilemap;
	bitmap_ind16 m_sprite_bitmap;

	UINT8 m_scroll;
	int m_mux;

	UINT8 m_main_irq_mask;
	UINT8 m_sub_irq_mask;
	UINT8 m_sub2_irq_mask;
	DECLARE_WRITE8_MEMBER(superpac_latch_w);
	DECLARE_WRITE8_MEMBER(phozon_latch_w);
	DECLARE_WRITE8_MEMBER(mappy_latch_w);
	DECLARE_WRITE8_MEMBER(superpac_videoram_w);
	DECLARE_WRITE8_MEMBER(mappy_videoram_w);
	DECLARE_WRITE8_MEMBER(superpac_flipscreen_w);
	DECLARE_READ8_MEMBER(superpac_flipscreen_r);
	DECLARE_WRITE8_MEMBER(mappy_scroll_w);
	DECLARE_READ8_MEMBER(dipA_l);
	DECLARE_READ8_MEMBER(dipA_h);
	DECLARE_READ8_MEMBER(dipB_mux);
	DECLARE_READ8_MEMBER(dipB_muxi);
	DECLARE_WRITE8_MEMBER(out_mux);
	DECLARE_WRITE8_MEMBER(out_lamps);
	DECLARE_WRITE8_MEMBER(grobda_DAC_w);
	DECLARE_DRIVER_INIT(digdug2);
	DECLARE_DRIVER_INIT(grobda);
	TILEMAP_MAPPER_MEMBER(superpac_tilemap_scan);
	TILEMAP_MAPPER_MEMBER(mappy_tilemap_scan);
	TILE_GET_INFO_MEMBER(superpac_get_tile_info);
	TILE_GET_INFO_MEMBER(phozon_get_tile_info);
	TILE_GET_INFO_MEMBER(mappy_get_tile_info);
	DECLARE_MACHINE_START(mappy);
	DECLARE_MACHINE_RESET(superpac);
	DECLARE_VIDEO_START(superpac);
	DECLARE_PALETTE_INIT(superpac);
	DECLARE_MACHINE_RESET(phozon);
	DECLARE_VIDEO_START(phozon);
	DECLARE_PALETTE_INIT(phozon);
	DECLARE_MACHINE_RESET(mappy);
	DECLARE_VIDEO_START(mappy);
	DECLARE_PALETTE_INIT(mappy);
	UINT32 screen_update_superpac(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_phozon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_mappy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in video/mappy.c -----------*/











