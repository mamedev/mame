/*************************************************************************

    Competition Golf Final Round

*************************************************************************/

class compgolf_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, compgolf_state(machine)); }

	compgolf_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT8 *        videoram;
	UINT8 *        bg_ram;
	UINT8 *        spriteram;

	/* video-related */
	tilemap_t        *text_tilemap, *bg_tilemap;
	int            scrollx_lo, scrollx_hi;
	int            scrolly_lo, scrolly_hi;

	/* misc */
	int            bank;
};


/*----------- defined in video/compgolf.c -----------*/

WRITE8_HANDLER( compgolf_video_w );
WRITE8_HANDLER( compgolf_back_w );
PALETTE_INIT ( compgolf );
VIDEO_START  ( compgolf );
VIDEO_UPDATE ( compgolf );
