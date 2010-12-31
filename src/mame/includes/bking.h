class bking_state : public driver_device
{
public:
	bking_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *     playfield_ram;

	/* video-related */
	bitmap_t    *tmp_bitmap1, *tmp_bitmap2;
	tilemap_t     *bg_tilemap;
	int         pc3259_output[4];
	int         pc3259_mask;
	UINT8       xld1, xld2, xld3;
	UINT8       yld1, yld2, yld3;
	int         ball1_pic, ball2_pic;
	int         crow_pic, crow_flip;
	int         palette_bank, controller, hit;

	/* sound-related */
	int         sound_nmi_enable, pending_nmi;

	/* misc */
	int         addr_h, addr_l;

	/* devices */
	device_t *audiocpu;
};


/*----------- defined in video/bking.c -----------*/

WRITE8_HANDLER( bking_xld1_w );
WRITE8_HANDLER( bking_yld1_w );
WRITE8_HANDLER( bking_xld2_w );
WRITE8_HANDLER( bking_yld2_w );
WRITE8_HANDLER( bking_xld3_w );
WRITE8_HANDLER( bking_yld3_w );
WRITE8_HANDLER( bking_msk_w );
WRITE8_HANDLER( bking_cont1_w );
WRITE8_HANDLER( bking_cont2_w );
WRITE8_HANDLER( bking_cont3_w );
WRITE8_HANDLER( bking_hitclr_w );
WRITE8_HANDLER( bking_playfield_w );

READ8_HANDLER( bking_input_port_5_r );
READ8_HANDLER( bking_input_port_6_r );
READ8_HANDLER( bking_pos_r );

PALETTE_INIT( bking );
VIDEO_START( bking );
VIDEO_UPDATE( bking );
VIDEO_EOF( bking );
