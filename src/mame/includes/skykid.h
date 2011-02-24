class skykid_state : public driver_device
{
public:
	skykid_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 inputport_selected;
	UINT8 *textram;
	UINT8 *videoram;
	UINT8 *spriteram;
	tilemap_t *bg_tilemap;
	tilemap_t *tx_tilemap;
	UINT8 priority;
	UINT16 scroll_x;
	UINT16 scroll_y;
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
