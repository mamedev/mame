/*
    buggychl mcu emulation is also used by
    40love.c, bking.c and msisaac.c
*/

class buggychl_state : public driver_device
{
public:
	buggychl_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *     videoram;	// buggychl, 40love, msisaac
	UINT8 *     videoram2;	// msisaac
	UINT8 *     videoram3;	// msisaac
	UINT8 *     colorram;	// 40love
	UINT8 *     spriteram;	// buggychl, 40love, msisaac
	UINT8 *     spriteram2;	// 40love
	UINT8 *     scrollv;	// buggychl
	UINT8 *     scrollh;	// buggychl
	UINT8 *     charram;	// buggychl
	UINT8 *     video_ctrl;	// 40love
//      UINT8 *     paletteram; // currently this uses generic palette handling (buggychl & msisaac)
	UINT8 *     mcu_ram;	// 40love (undokai)
	UINT8 *     playfield_ram;	// bking
	size_t      videoram_size;
	size_t      spriteram_size;
	size_t      spriteram2_size;

	/* video-related */
	bitmap_t    *tmp_bitmap1, *tmp_bitmap2;
	tilemap_t     *bg_tilemap;
	tilemap_t     *fg_tilemap, *bg2_tilemap;	// msisaac
	// buggychl
	int         sl_bank, bg_on, sky_on, sprite_color_base, bg_scrollx;
	UINT8       sprite_lookup[0x2000];
	// 40love
	UINT8       flipscreen, pix_redraw;
	UINT8       xoffset;
	UINT8       *pixram1;
	UINT8       *pixram2;
	bitmap_t    *pixel_bitmap1;
	bitmap_t    *pixel_bitmap2;
	tilemap_t     *background;
	int         pixram_sel;
	// bking
	int         pc3259_output[4];
	int         pc3259_mask;
	UINT8       xld1, xld2, xld3;
	UINT8       yld1, yld2, yld3;
	int         ball1_pic, ball2_pic;
	int         crow_pic, crow_flip;
	int         palette_bank, controller, hit;
	// msisaac
	int         bg2_textbank;


	/* sound-related */
	int         sound_nmi_enable, pending_nmi;

	/* mcu */
	UINT8       port_a_in, port_a_out, ddr_a;
	UINT8       port_b_in, port_b_out, ddr_b;
	UINT8       port_c_in, port_c_out, ddr_c;
	UINT8       from_main, from_mcu;
	int         mcu_sent, main_sent;

	/* fake mcu (in 40love.c) - it also uses from_mcu */
	UINT8       mcu_in[2][16], mcu_out[2][16];
	int         mcu_cmd;

	/* fake mcu (in msisaac.c) */
#ifndef USE_MCU
	UINT8       mcu_val;
	UINT8       direction;
#endif


	/* misc */
	// 40love
	int         pix_color[4];
	UINT8       pix1, pix2[2];
	UINT8       snd_data, snd_flag;
	int         vol_ctrl[16];
	UINT8       snd_ctrl0, snd_ctrl1, snd_ctrl2, snd_ctrl3;
	// bking
	int         addr_h, addr_l;

	/* devices */
	running_device *audiocpu;
	running_device *mcu;
};


/*----------- defined in machine/buggychl.c -----------*/

READ8_HANDLER( buggychl_68705_port_a_r );
WRITE8_HANDLER( buggychl_68705_port_a_w );
WRITE8_HANDLER( buggychl_68705_ddr_a_w );
READ8_HANDLER( buggychl_68705_port_b_r );
WRITE8_HANDLER( buggychl_68705_port_b_w );
WRITE8_HANDLER( buggychl_68705_ddr_b_w );
READ8_HANDLER( buggychl_68705_port_c_r );
WRITE8_HANDLER( buggychl_68705_port_c_w );
WRITE8_HANDLER( buggychl_68705_ddr_c_w );
WRITE8_HANDLER( buggychl_mcu_w );
READ8_HANDLER( buggychl_mcu_r );
READ8_HANDLER( buggychl_mcu_status_r );


/*----------- defined in video/buggychl.c -----------*/

WRITE8_HANDLER( buggychl_chargen_w );
WRITE8_HANDLER( buggychl_sprite_lookup_bank_w );
WRITE8_HANDLER( buggychl_sprite_lookup_w );
WRITE8_HANDLER( buggychl_ctrl_w );
WRITE8_HANDLER( buggychl_bg_scrollx_w );

PALETTE_INIT( buggychl );
VIDEO_START( buggychl );
VIDEO_UPDATE( buggychl );

/*----------- defined in video/40love.c -----------*/

WRITE8_HANDLER( fortyl_bg_videoram_w );
WRITE8_HANDLER( fortyl_bg_colorram_w );
READ8_HANDLER ( fortyl_bg_videoram_r );
READ8_HANDLER ( fortyl_bg_colorram_r );
WRITE8_HANDLER( fortyl_pixram_sel_w );
READ8_HANDLER( fortyl_pixram_r );
WRITE8_HANDLER( fortyl_pixram_w );

VIDEO_START( fortyl );
VIDEO_UPDATE( fortyl );
PALETTE_INIT( fortyl );

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

/*----------- defined in video/msisaac.c -----------*/

WRITE8_HANDLER( msisaac_fg_scrolly_w );
WRITE8_HANDLER( msisaac_fg_scrollx_w );
WRITE8_HANDLER( msisaac_bg_scrolly_w );
WRITE8_HANDLER( msisaac_bg_scrollx_w );
WRITE8_HANDLER( msisaac_bg2_scrolly_w );
WRITE8_HANDLER( msisaac_bg2_scrollx_w );
WRITE8_HANDLER( msisaac_bg2_textbank_w );

WRITE8_HANDLER( msisaac_bg_videoram_w );
WRITE8_HANDLER( msisaac_bg2_videoram_w );
WRITE8_HANDLER( msisaac_fg_videoram_w );

VIDEO_UPDATE( msisaac );
VIDEO_START( msisaac );
