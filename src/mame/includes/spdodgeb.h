class spdodgeb_state : public driver_device
{
public:
	spdodgeb_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int toggle;
	int adpcm_pos[2];
	int adpcm_end[2];
	int adpcm_idle[2];
	int adpcm_data[2];
	int mcu63701_command;
	int inputs[4];
	UINT8 tapc[4];
	UINT8 last_port[2];
	UINT8 last_dash[2];
#if 0
	int running[2];
	int jumped[2];
	int prev[2][2];
	int countup[2][2];
	int countdown[2][2];
	int prev[2];
#endif
	UINT8 *videoram;
	int tile_palbank;
	int sprite_palbank;
	tilemap_t *bg_tilemap;
	int lastscroll;
};


/*----------- defined in video/spdodgeb.c -----------*/

PALETTE_INIT( spdodgeb );
VIDEO_START( spdodgeb );
SCREEN_UPDATE( spdodgeb );
INTERRUPT_GEN( spdodgeb_interrupt );
WRITE8_HANDLER( spdodgeb_scrollx_lo_w );
WRITE8_HANDLER( spdodgeb_ctrl_w );
WRITE8_HANDLER( spdodgeb_videoram_w );
