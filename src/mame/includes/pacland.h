class pacland_state : public driver_device
{
public:
	pacland_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_videoram2;
	UINT8 *m_spriteram;
	UINT8 m_palette_bank;
	const UINT8 *m_color_prom;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	bitmap_ind16 m_fg_bitmap;
	UINT32 *m_transmask[3];
	UINT16 m_scroll0;
	UINT16 m_scroll1;
	UINT8 m_main_irq_mask;
	UINT8 m_mcu_irq_mask;
	DECLARE_WRITE8_MEMBER(pacland_subreset_w);
	DECLARE_WRITE8_MEMBER(pacland_flipscreen_w);
	DECLARE_READ8_MEMBER(pacland_input_r);
	DECLARE_WRITE8_MEMBER(pacland_coin_w);
	DECLARE_WRITE8_MEMBER(pacland_led_w);
	DECLARE_WRITE8_MEMBER(pacland_irq_1_ctrl_w);
	DECLARE_WRITE8_MEMBER(pacland_irq_2_ctrl_w);
	DECLARE_READ8_MEMBER(readFF);
	DECLARE_WRITE8_MEMBER(pacland_videoram_w);
	DECLARE_WRITE8_MEMBER(pacland_videoram2_w);
	DECLARE_WRITE8_MEMBER(pacland_scroll0_w);
	DECLARE_WRITE8_MEMBER(pacland_scroll1_w);
	DECLARE_WRITE8_MEMBER(pacland_bankswitch_w);
};


/*----------- defined in video/pacland.c -----------*/


PALETTE_INIT( pacland );
VIDEO_START( pacland );
SCREEN_UPDATE_IND16( pacland );
