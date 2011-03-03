class wc90b_state : public driver_device
{
public:
	wc90b_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int msm5205next;
	int toggle;
	UINT8 *fgvideoram;
	UINT8 *bgvideoram;
	UINT8 *txvideoram;
	UINT8 *scroll1x;
	UINT8 *scroll2x;
	UINT8 *scroll1y;
	UINT8 *scroll2y;
	UINT8 *scroll_x_lo;
	tilemap_t *tx_tilemap;
	tilemap_t *fg_tilemap;
	tilemap_t *bg_tilemap;
};


/*----------- defined in video/wc90b.c -----------*/

VIDEO_START( wc90b );
SCREEN_UPDATE( wc90b );

WRITE8_HANDLER( wc90b_bgvideoram_w );
WRITE8_HANDLER( wc90b_fgvideoram_w );
WRITE8_HANDLER( wc90b_txvideoram_w );
