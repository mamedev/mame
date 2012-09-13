
class paradise_state : public driver_device
{
public:
	paradise_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_vram_0(*this, "vram_0"),
		m_vram_1(*this, "vram_1"),
		m_vram_2(*this, "vram_2"),
		m_videoram(*this, "videoram"),
		m_paletteram(*this, "paletteram"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_vram_0;
	required_shared_ptr<UINT8> m_vram_1;
	required_shared_ptr<UINT8> m_vram_2;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_paletteram;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t *m_tilemap_0;
	tilemap_t *m_tilemap_1;
	tilemap_t *m_tilemap_2;
	bitmap_ind16 m_tmpbitmap;
	UINT8 m_palbank;
	UINT8 m_priority;
	UINT8 m_pixbank;
	int m_sprite_inc;

	int irq_count;
	DECLARE_WRITE8_MEMBER(paradise_rombank_w);
	DECLARE_WRITE8_MEMBER(torus_coin_counter_w);
	DECLARE_WRITE8_MEMBER(paradise_flipscreen_w);
	DECLARE_WRITE8_MEMBER(tgtball_flipscreen_w);
	DECLARE_WRITE8_MEMBER(paradise_palette_w);
	DECLARE_WRITE8_MEMBER(paradise_vram_0_w);
	DECLARE_WRITE8_MEMBER(paradise_palbank_w);
	DECLARE_WRITE8_MEMBER(paradise_vram_1_w);
	DECLARE_WRITE8_MEMBER(paradise_vram_2_w);
	DECLARE_WRITE8_MEMBER(paradise_pixmap_w);
	DECLARE_WRITE8_MEMBER(paradise_priority_w);
	DECLARE_WRITE8_MEMBER(paradise_okibank_w);
	DECLARE_DRIVER_INIT(torus);
	DECLARE_DRIVER_INIT(paradise);
	DECLARE_DRIVER_INIT(tgtball);
	TILE_GET_INFO_MEMBER(get_tile_info_0);
	TILE_GET_INFO_MEMBER(get_tile_info_1);
	TILE_GET_INFO_MEMBER(get_tile_info_2);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
};

/*----------- defined in video/paradise.c -----------*/






SCREEN_UPDATE_IND16( paradise );
SCREEN_UPDATE_IND16( torus );
SCREEN_UPDATE_IND16( madball );
