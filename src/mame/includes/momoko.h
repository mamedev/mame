/*************************************************************************

    Momoko 120%

*************************************************************************/

class momoko_state : public driver_device
{
public:
	momoko_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *        bg_scrollx;
	UINT8 *        bg_scrolly;
	UINT8 *        videoram;
	UINT8 *        spriteram;
//  UINT8 *        paletteram;    // currently this uses generic palette handling
	size_t         spriteram_size;
	size_t         videoram_size;

	/* video-related */
	UINT8          fg_scrollx;
	UINT8          fg_scrolly;
	UINT8          fg_select;
	UINT8          text_scrolly;
	UINT8          text_mode;
	UINT8          bg_select;
	UINT8          bg_priority;
	UINT8          bg_mask;
	UINT8          fg_mask;
	UINT8          flipscreen;
};


/*----------- defined in video/momoko.c -----------*/

WRITE8_HANDLER( momoko_fg_scrollx_w );
WRITE8_HANDLER( momoko_fg_scrolly_w );
WRITE8_HANDLER( momoko_text_scrolly_w );
WRITE8_HANDLER( momoko_text_mode_w );
WRITE8_HANDLER( momoko_bg_scrollx_w );
WRITE8_HANDLER( momoko_bg_scrolly_w );
WRITE8_HANDLER( momoko_flipscreen_w );
WRITE8_HANDLER( momoko_fg_select_w);
WRITE8_HANDLER( momoko_bg_select_w);
WRITE8_HANDLER( momoko_bg_priority_w);

VIDEO_UPDATE( momoko );
