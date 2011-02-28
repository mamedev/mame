class popeye_state : public driver_device
{
public:
	popeye_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 prot0;
	UINT8 prot1;
	UINT8 prot_shift;
	int dswbit;
	UINT8 *videoram;
	UINT8 *colorram;
	UINT8 *background_pos;
	UINT8 *palettebank;
	UINT8 *bitmapram;
	bitmap_t *tmpbitmap2;
	UINT8 invertmask;
	UINT8 bitmap_type;
	tilemap_t *fg_tilemap;
	UINT8 lastflip;
};


/*----------- defined in video/popeye.c -----------*/

WRITE8_HANDLER( popeye_videoram_w );
WRITE8_HANDLER( popeye_colorram_w );
WRITE8_HANDLER( popeye_bitmap_w );
WRITE8_HANDLER( skyskipr_bitmap_w );

PALETTE_INIT( popeye );
PALETTE_INIT( popeyebl );
VIDEO_START( skyskipr );
VIDEO_START( popeye );
SCREEN_UPDATE( popeye );
