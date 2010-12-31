/***************************************************************************

    Game Driver for Video System Mahjong series and Pipe Dream.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2001/02/04 -
    and Bryan McPhail, Nicola Salmoria, Aaron Giles

***************************************************************************/

class fromance_state : public driver_device
{
public:
	fromance_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers (used by pipedrm) */
	UINT8 *  videoram;
	UINT8 *  spriteram;
//  UINT8 *  paletteram;    // currently this uses generic palette handling
	size_t   videoram_size;
	size_t   spriteram_size;

	/* video-related */
	tilemap_t  *bg_tilemap,*fg_tilemap;
	UINT8    *local_videoram[2];
	UINT8    *local_paletteram;
	UINT8    selected_videoram, selected_paletteram;
	UINT32   scrollx[2], scrolly[2];
	UINT8    gfxreg;
	UINT8    flipscreen, flipscreen_old;
	UINT32   scrolly_ofs, scrollx_ofs;

	UINT8    crtc_register;
	UINT8    crtc_data[0x10];
	emu_timer *crtc_timer;

	/* misc */
	UINT8    directionflag, commanddata, portselect;
	UINT8    adpcm_reset, adpcm_data, vclk_left;
	UINT8    pending_command, sound_command;

	/* devices */
	device_t *subcpu;
};


/*----------- defined in video/fromance.c -----------*/

VIDEO_START( fromance );
VIDEO_START( nekkyoku );
VIDEO_START( pipedrm );
VIDEO_START( hatris );
VIDEO_UPDATE( fromance );
VIDEO_UPDATE( pipedrm );

WRITE8_HANDLER( fromance_crtc_data_w );
WRITE8_HANDLER( fromance_crtc_register_w );

WRITE8_HANDLER( fromance_gfxreg_w );

WRITE8_HANDLER( fromance_scroll_w );

READ8_HANDLER( fromance_paletteram_r );
WRITE8_HANDLER( fromance_paletteram_w );

READ8_HANDLER( fromance_videoram_r );
WRITE8_HANDLER( fromance_videoram_w );
