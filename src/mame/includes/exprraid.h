/*************************************************************************

    Express Raider

*************************************************************************/


class exprraid_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, exprraid_state(machine)); }

	exprraid_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT8 *        main_ram;
	UINT8 *        videoram;
	UINT8 *        colorram;
	UINT8 *        spriteram;
	size_t         spriteram_size;

	/* video-related */
	tilemap_t        *bg_tilemap, *fg_tilemap;
	int            bg_index[4];

	/* misc */
	//int          coin;    // used in the commented out INTERRUPT_GEN - can this be removed?

	/* devices */
	running_device *maincpu;
	running_device *slave;
};


/*----------- defined in video/exprraid.c -----------*/

extern WRITE8_HANDLER( exprraid_videoram_w );
extern WRITE8_HANDLER( exprraid_colorram_w );
extern WRITE8_HANDLER( exprraid_flipscreen_w );
extern WRITE8_HANDLER( exprraid_bgselect_w );
extern WRITE8_HANDLER( exprraid_scrollx_w );
extern WRITE8_HANDLER( exprraid_scrolly_w );

extern VIDEO_START( exprraid );
extern VIDEO_UPDATE( exprraid );
