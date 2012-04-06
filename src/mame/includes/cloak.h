/*************************************************************************

    Atari Cloak & Dagger hardware

*************************************************************************/

class cloak_state : public driver_device
{
public:
	cloak_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_spriteram;
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
};


/*----------- defined in video/cloak.c -----------*/



VIDEO_START( cloak );
SCREEN_UPDATE_IND16( cloak );
