// license:BSD-3-Clause
// copyright-holders:Dan Boris, Mirko Buffoni
/*************************************************************************

    Atari Cloak & Dagger hardware

*************************************************************************/

class cloak_state : public driver_device
{
public:
	cloak_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_slave(*this, "slave"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	int m_nvram_enabled;
	UINT8 m_bitmap_videoram_selected;
	UINT8 m_bitmap_videoram_address_x;
	UINT8 m_bitmap_videoram_address_y;
	UINT8 *m_bitmap_videoram1;
	UINT8 *m_bitmap_videoram2;
	UINT8 *m_current_bitmap_videoram_accessed;
	UINT8 *m_current_bitmap_videoram_displayed;
	UINT16 *m_palette_ram;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(cloak_led_w);
	DECLARE_WRITE8_MEMBER(cloak_coin_counter_w);
	DECLARE_WRITE8_MEMBER(cloak_custom_w);
	DECLARE_WRITE8_MEMBER(cloak_irq_reset_0_w);
	DECLARE_WRITE8_MEMBER(cloak_irq_reset_1_w);
	DECLARE_WRITE8_MEMBER(cloak_nvram_enable_w);
	DECLARE_WRITE8_MEMBER(cloak_paletteram_w);
	DECLARE_WRITE8_MEMBER(cloak_clearbmp_w);
	DECLARE_READ8_MEMBER(graph_processor_r);
	DECLARE_WRITE8_MEMBER(graph_processor_w);
	DECLARE_WRITE8_MEMBER(cloak_videoram_w);
	DECLARE_WRITE8_MEMBER(cloak_flipscreen_w);
	void set_current_bitmap_videoram_pointer();
	void adjust_xy(int offset);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start() override;
	UINT32 screen_update_cloak(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void set_pen(int i);
	void draw_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_slave;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};
