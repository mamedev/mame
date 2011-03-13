class lucky74_state : public driver_device
{
public:
	lucky74_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 ym2149_portb;
	UINT8 usart_8251;
	UINT8 copro_sm7831;
	int adpcm_pos;
	int adpcm_end;
	int adpcm_data;
	UINT8 adpcm_reg[6];
	UINT8 adpcm_busy_line;
	UINT8 *fg_videoram;
	UINT8 *fg_colorram;
	UINT8 *bg_videoram;
	UINT8 *bg_colorram;
	tilemap_t *fg_tilemap;
	tilemap_t *bg_tilemap;
};


/*----------- defined in video/lucky74.c -----------*/

WRITE8_HANDLER( lucky74_fg_videoram_w );
WRITE8_HANDLER( lucky74_fg_colorram_w );
WRITE8_HANDLER( lucky74_bg_videoram_w );
WRITE8_HANDLER( lucky74_bg_colorram_w );
PALETTE_INIT( lucky74 );
VIDEO_START( lucky74 );
SCREEN_UPDATE( lucky74 );
