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
};


/*----------- defined in video/skykid.c -----------*/

VIDEO_START( skykid );
SCREEN_UPDATE( skykid );
PALETTE_INIT( skykid );

READ8_HANDLER( skykid_videoram_r );
WRITE8_HANDLER( skykid_videoram_w );
READ8_HANDLER( skykid_textram_r );
WRITE8_HANDLER( skykid_textram_w );
WRITE8_HANDLER( skykid_scroll_x_w );
WRITE8_HANDLER( skykid_scroll_y_w );
WRITE8_HANDLER( skykid_flipscreen_priority_w );
