class tceptor_state : public driver_device
{
public:
	tceptor_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_m6809_irq_enable;
	UINT8 m_m68k_irq_enable;
	UINT8 m_mcu_irq_enable;
	UINT8 *m_m68k_shared_ram;
	UINT8 *m_tile_ram;
	UINT8 *m_tile_attr;
	UINT8 *m_bg_ram;
	UINT16 *m_sprite_ram;
	int m_sprite16;
	int m_sprite32;
	int m_bg;
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_bg1_tilemap;
	tilemap_t *m_bg2_tilemap;
	INT32 m_bg1_scroll_x;
	INT32 m_bg1_scroll_y;
	INT32 m_bg2_scroll_x;
	INT32 m_bg2_scroll_y;
	bitmap_ind16 m_temp_bitmap;
	UINT16 *m_sprite_ram_buffered;
	int m_is_mask_spr[1024/16];
	DECLARE_READ16_MEMBER(m68k_shared_word_r);
	DECLARE_WRITE16_MEMBER(m68k_shared_word_w);
	DECLARE_WRITE8_MEMBER(m6809_irq_enable_w);
	DECLARE_WRITE8_MEMBER(m6809_irq_disable_w);
	DECLARE_WRITE16_MEMBER(m68k_irq_enable_w);
	DECLARE_WRITE8_MEMBER(mcu_irq_enable_w);
	DECLARE_WRITE8_MEMBER(mcu_irq_disable_w);
	DECLARE_READ8_MEMBER(dsw0_r);
	DECLARE_READ8_MEMBER(dsw1_r);
	DECLARE_READ8_MEMBER(input0_r);
	DECLARE_READ8_MEMBER(input1_r);
	DECLARE_READ8_MEMBER(readFF);
	DECLARE_WRITE8_MEMBER(tceptor_tile_ram_w);
	DECLARE_WRITE8_MEMBER(tceptor_tile_attr_w);
	DECLARE_WRITE8_MEMBER(tceptor_bg_ram_w);
	DECLARE_WRITE8_MEMBER(tceptor_bg_scroll_w);
	void tile_mark_dirty(int offset);
};


/*----------- defined in video/tceptor.c -----------*/

PALETTE_INIT( tceptor );
VIDEO_START( tceptor );
SCREEN_UPDATE_IND16( tceptor_2d );
SCREEN_UPDATE_IND16( tceptor_3d_left );
SCREEN_UPDATE_IND16( tceptor_3d_right );
SCREEN_VBLANK( tceptor );


