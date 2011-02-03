class superqix_state : public driver_device
{
public:
	superqix_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	INT16 *samplebuf;
	UINT8 port1;
	UINT8 port2;
	UINT8 port3;
	UINT8 port3_latch;
	UINT8 from_mcu;
	UINT8 from_z80;
	UINT8 portb;
	int from_mcu_pending;
	int from_z80_pending;
	int invert_coin_lockout;
	int oldpos[2];
	int sign[2];
	UINT8 portA_in;
	UINT8 portB_out;
	UINT8 portC;
	int curr_player;
	UINT8 *videoram;
	UINT8 *bitmapram;
	UINT8 *bitmapram2;
	int pbillian_show_power;
	int gfxbank;
	bitmap_t *fg_bitmap[2];
	int show_bitmap;
	tilemap_t *bg_tilemap;
	int last_power[2];
};


/*----------- defined in video/superqix.c -----------*/

WRITE8_HANDLER( superqix_videoram_w );
WRITE8_HANDLER( superqix_bitmapram_w );
WRITE8_HANDLER( superqix_bitmapram2_w );
WRITE8_HANDLER( pbillian_0410_w );
WRITE8_HANDLER( superqix_0410_w );

VIDEO_START( pbillian );
VIDEO_UPDATE( pbillian );
VIDEO_START( superqix );
VIDEO_UPDATE( superqix );
