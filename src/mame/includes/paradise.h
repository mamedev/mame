
class paradise_state : public driver_device
{
public:
	paradise_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *  m_vram_0;
	UINT8 *  m_vram_1;
	UINT8 *  m_vram_2;
	UINT8 *  m_videoram;
	UINT8 *  m_paletteram;
	UINT8 *  m_spriteram;
	size_t   m_spriteram_size;

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
};

/*----------- defined in video/paradise.c -----------*/




VIDEO_START( paradise );

SCREEN_UPDATE_IND16( paradise );
SCREEN_UPDATE_IND16( torus );
SCREEN_UPDATE_IND16( madball );
