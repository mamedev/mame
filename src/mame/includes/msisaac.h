/* Disabled because the mcu dump is currently unavailable. -AS */
//#define USE_MCU

class msisaac_state : public driver_device
{
public:
	msisaac_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *     videoram;
	UINT8 *     videoram2;
	UINT8 *     videoram3;
	UINT8 *     spriteram;
	size_t      videoram_size;
	size_t      spriteram_size;
	size_t      spriteram2_size;

	/* video-related */
	bitmap_t    *tmp_bitmap1, *tmp_bitmap2;
	tilemap_t     *bg_tilemap;
	tilemap_t     *fg_tilemap, *bg2_tilemap;
	int         bg2_textbank;

	/* sound-related */
	int         sound_nmi_enable, pending_nmi;

	/* fake mcu (in msisaac.c) */
#ifndef USE_MCU
	UINT8       mcu_val;
	UINT8       direction;
#endif

	int         vol_ctrl[16];
	UINT8       snd_ctrl0, snd_ctrl1, snd_ctrl2, snd_ctrl3;

	/* devices */
	device_t *audiocpu;
};


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
