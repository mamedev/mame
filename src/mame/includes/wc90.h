class wc90_state : public driver_device
{
public:
	wc90_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *fgvideoram;
	UINT8 *bgvideoram;
	UINT8 *txvideoram;
	UINT8 *scroll0xlo;
	UINT8 *scroll0xhi;
	UINT8 *scroll1xlo;
	UINT8 *scroll1xhi;
	UINT8 *scroll2xlo;
	UINT8 *scroll2xhi;
	UINT8 *scroll0ylo;
	UINT8 *scroll0yhi;
	UINT8 *scroll1ylo;
	UINT8 *scroll1yhi;
	UINT8 *scroll2ylo;
	UINT8 *scroll2yhi;
	tilemap_t *tx_tilemap;
	tilemap_t *fg_tilemap;
	tilemap_t *bg_tilemap;
	UINT8 *spriteram;
	size_t spriteram_size;
};


/*----------- defined in video/wc90.c -----------*/

VIDEO_START( wc90 );
VIDEO_START( wc90t );
WRITE8_HANDLER( wc90_fgvideoram_w );
WRITE8_HANDLER( wc90_bgvideoram_w );
WRITE8_HANDLER( wc90_txvideoram_w );
SCREEN_UPDATE( wc90 );
