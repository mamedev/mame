class srmp2_state : public driver_device
{
public:
	srmp2_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int color_bank;
	int gfx_bank;

	int adpcm_bank;
	int adpcm_data;
	UINT32 adpcm_sptr;
	UINT32 adpcm_eptr;

	int port_select;

	union
	{
		UINT8 *u8;
		UINT16 *u16;
	} spriteram1, spriteram2, spriteram3;
	UINT8 iox_data;
	UINT8 iox_mux;
	UINT8 iox_ff;
};


/*----------- defined in video/srmp2.c -----------*/

PALETTE_INIT( srmp2 );
SCREEN_UPDATE( srmp2 );
PALETTE_INIT( srmp3 );
SCREEN_UPDATE( srmp3 );
SCREEN_UPDATE( mjyuugi );
