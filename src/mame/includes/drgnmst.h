
class drgnmst_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, drgnmst_state(machine)); }

	drgnmst_state(running_machine &machine) { }

	/* memory pointers */
	UINT16 *    vidregs;
	UINT16 *    fg_videoram;
	UINT16 *    bg_videoram;
	UINT16 *    md_videoram;
	UINT16 *    rowscrollram;
	UINT16 *    vidregs2;
	UINT16 *    spriteram;
//  UINT16 *    paletteram;     // currently this uses generic palette handling
	size_t      spriteram_size;

	/* video-related */
	tilemap_t     *bg_tilemap,*fg_tilemap, *md_tilemap;

	/* misc */
	UINT16      snd_command;
	UINT16      snd_flag;
	UINT8       oki_control;
	UINT8       oki_command;
	UINT8       pic16c5x_port0;
	UINT8       oki0_bank;
	UINT8       oki1_bank;

	/* devices */
	running_device *oki_1;
	running_device *oki_2;
};


/*----------- defined in video/drgnmst.c -----------*/

WRITE16_HANDLER( drgnmst_fg_videoram_w );
WRITE16_HANDLER( drgnmst_bg_videoram_w );
WRITE16_HANDLER( drgnmst_md_videoram_w );

VIDEO_START(drgnmst);
VIDEO_UPDATE(drgnmst);
