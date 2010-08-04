/*************************************************************************

    Metal Clash

*************************************************************************/

class metlclsh_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, metlclsh_state(machine)); }

	metlclsh_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT8 *        bgram;
	UINT8 *        fgram;
	UINT8 *        scrollx;
	UINT8 *        otherram;
//      UINT8 *        paletteram;    // currently this uses generic palette handling
//      UINT8 *        paletteram2;    // currently this uses generic palette handling
	UINT8 *        spriteram;
	size_t         spriteram_size;

	/* video-related */
	tilemap_t      *bg_tilemap,*fg_tilemap;
	UINT8          write_mask, gfxbank;

	/* devices */
	running_device *maincpu;
	running_device *subcpu;
};


/*----------- defined in video/metlclsh.c -----------*/

WRITE8_HANDLER( metlclsh_bgram_w );
WRITE8_HANDLER( metlclsh_fgram_w );
WRITE8_HANDLER( metlclsh_gfxbank_w );
WRITE8_HANDLER( metlclsh_rambank_w );

VIDEO_START( metlclsh );
VIDEO_UPDATE( metlclsh );
