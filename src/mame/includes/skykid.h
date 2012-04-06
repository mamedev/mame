class skykid_state : public driver_device
{
public:
	skykid_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_inputport_selected;
	UINT8 *m_textram;
	UINT8 *m_videoram;
	UINT8 *m_spriteram;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_tx_tilemap;
	UINT8 m_priority;
	UINT16 m_scroll_x;
	UINT16 m_scroll_y;
	UINT8 m_main_irq_mask;
	UINT8 m_mcu_irq_mask;
	DECLARE_WRITE8_MEMBER(inputport_select_w);
	DECLARE_READ8_MEMBER(inputport_r);
	DECLARE_WRITE8_MEMBER(skykid_led_w);
	DECLARE_WRITE8_MEMBER(skykid_subreset_w);
	DECLARE_WRITE8_MEMBER(skykid_bankswitch_w);
	DECLARE_WRITE8_MEMBER(skykid_irq_1_ctrl_w);
	DECLARE_WRITE8_MEMBER(skykid_irq_2_ctrl_w);
	DECLARE_READ8_MEMBER(readFF);
	DECLARE_READ8_MEMBER(skykid_videoram_r);
	DECLARE_WRITE8_MEMBER(skykid_videoram_w);
	DECLARE_READ8_MEMBER(skykid_textram_r);
	DECLARE_WRITE8_MEMBER(skykid_textram_w);
	DECLARE_WRITE8_MEMBER(skykid_scroll_x_w);
	DECLARE_WRITE8_MEMBER(skykid_scroll_y_w);
	DECLARE_WRITE8_MEMBER(skykid_flipscreen_priority_w);
};


/*----------- defined in video/skykid.c -----------*/

VIDEO_START( skykid );
SCREEN_UPDATE_IND16( skykid );
PALETTE_INIT( skykid );

