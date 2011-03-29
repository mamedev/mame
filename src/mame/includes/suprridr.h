/*************************************************************************

    Venture Line Super Rider driver

**************************************************************************/

class suprridr_state : public driver_device
{
public:
	suprridr_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 nmi_enable;
	UINT8 sound_data;
	UINT8 *fgram;
	UINT8 *bgram;
	tilemap_t *fg_tilemap;
	tilemap_t *bg_tilemap;
	tilemap_t *bg_tilemap_noscroll;
	UINT8 flipx;
	UINT8 flipy;
	UINT8 *spriteram;
};


/*----------- defined in video/suprridr.c -----------*/

VIDEO_START( suprridr );
PALETTE_INIT( suprridr );

WRITE8_HANDLER( suprridr_flipx_w );
WRITE8_HANDLER( suprridr_flipy_w );
WRITE8_HANDLER( suprridr_fgdisable_w );
WRITE8_HANDLER( suprridr_fgscrolly_w );
WRITE8_HANDLER( suprridr_bgscrolly_w );
int suprridr_is_screen_flipped(running_machine &machine);

WRITE8_HANDLER( suprridr_fgram_w );
WRITE8_HANDLER( suprridr_bgram_w );

SCREEN_UPDATE( suprridr );
