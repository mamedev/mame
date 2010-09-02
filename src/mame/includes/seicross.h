class seicross_state : public driver_device
{
public:
	seicross_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *nvram;
	size_t nvram_size;

	UINT8 portb;

	size_t spriteram_size;
	size_t spriteram2_size;
	UINT8 *spriteram;
	UINT8 *spriteram2;
	UINT8 *videoram;
	UINT8 *colorram;
	tilemap_t *bg_tilemap;
	UINT8 *row_scroll;
};


/*----------- defined in video/seicross.c -----------*/

extern UINT8 *seicross_videoram;
extern UINT8 *seicross_colorram;
extern UINT8 *seicross_row_scroll;

WRITE8_HANDLER( seicross_videoram_w );
WRITE8_HANDLER( seicross_colorram_w );

PALETTE_INIT( seicross );
VIDEO_START( seicross );
VIDEO_UPDATE( seicross );
