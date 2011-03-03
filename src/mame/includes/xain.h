class xain_state : public driver_device
{
public:
	xain_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int vblank;
	int from_main;
	int from_mcu;
	UINT8 ddr_a;
	UINT8 ddr_b;
	UINT8 ddr_c;
	UINT8 port_a_out;
	UINT8 port_b_out;
	UINT8 port_c_out;
	UINT8 port_a_in;
	UINT8 port_b_in;
	UINT8 port_c_in;
	int _mcu_ready;
	int _mcu_accept;
	UINT8 *charram;
	UINT8 *bgram0;
	UINT8 *bgram1;
	UINT8 pri;
	tilemap_t *char_tilemap;
	tilemap_t *bgram0_tilemap;
	tilemap_t *bgram1_tilemap;
	UINT8 scrollxP0[2];
	UINT8 scrollyP0[2];
	UINT8 scrollxP1[2];
	UINT8 scrollyP1[2];
};


/*----------- defined in video/xain.c -----------*/

SCREEN_UPDATE( xain );
VIDEO_START( xain );
WRITE8_HANDLER( xain_scrollxP0_w );
WRITE8_HANDLER( xain_scrollyP0_w );
WRITE8_HANDLER( xain_scrollxP1_w );
WRITE8_HANDLER( xain_scrollyP1_w );
WRITE8_HANDLER( xain_charram_w );
WRITE8_HANDLER( xain_bgram0_w );
WRITE8_HANDLER( xain_bgram1_w );
WRITE8_HANDLER( xain_flipscreen_w );
