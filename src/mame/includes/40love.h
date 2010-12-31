class fortyl_state : public driver_device
{
public:
	fortyl_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *     videoram;
	UINT8 *     colorram;
	UINT8 *     spriteram;
	UINT8 *     spriteram2;
	UINT8 *     video_ctrl;
	size_t      spriteram_size;
	size_t      spriteram2_size;

	/* video-related */
	bitmap_t    *tmp_bitmap1, *tmp_bitmap2;
	tilemap_t     *bg_tilemap;
	UINT8       flipscreen, pix_redraw;
	UINT8       xoffset;
	UINT8       *pixram1;
	UINT8       *pixram2;
	bitmap_t    *pixel_bitmap1;
	bitmap_t    *pixel_bitmap2;
	int         pixram_sel;

	/* sound-related */
	int         sound_nmi_enable, pending_nmi;

	/* fake mcu */
	UINT8 *     mcu_ram;
	UINT8       from_mcu;
	int         mcu_sent, main_sent;
	UINT8       mcu_in[2][16], mcu_out[2][16];
	int         mcu_cmd;

	/* misc */
	int         pix_color[4];
	UINT8       pix1, pix2[2];
	UINT8       snd_data, snd_flag;
	int         vol_ctrl[16];
	UINT8       snd_ctrl0, snd_ctrl1, snd_ctrl2, snd_ctrl3;

	/* devices */
	device_t *audiocpu;
};


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
