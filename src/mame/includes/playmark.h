
class playmark_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, playmark_state(machine)); }

	playmark_state(running_machine &machine) { }

	/* memory pointers */
	UINT16 *     bgvideoram;
	UINT16 *     videoram1;
	UINT16 *     videoram2;
	UINT16 *     videoram3;
	UINT16 *     rowscroll;
	UINT16 *     spriteram;
//      UINT16 *     paletteram;    // currently this uses generic palette handling
	size_t       spriteram_size;

	/* video-related */
	tilemap_t   *tx_tilemap, *fg_tilemap, *bg_tilemap;
	int         bgscrollx, bgscrolly, bg_enable, bg_full_size;
	int         fgscrollx, fg_rowscroll_enable;

	int         xoffset;
	int         yoffset;
	int         txt_tile_offset;
	int         pri_masks[3];
	UINT16      scroll[7];

	/* powerbal-specific */
	int         tilebank;
	int         bg_yoffset;

	/* misc */
	UINT16      snd_command;
	UINT16      snd_flag;
	UINT8       oki_control;
	UINT8       oki_command;
	int         old_oki_bank;

	/* devices */
	running_device *oki;
	running_device *eeprom;
};

/*----------- defined in video/playmark.c -----------*/

WRITE16_HANDLER( wbeachvl_txvideoram_w );
WRITE16_HANDLER( wbeachvl_fgvideoram_w );
WRITE16_HANDLER( wbeachvl_bgvideoram_w );
WRITE16_HANDLER( hrdtimes_txvideoram_w );
WRITE16_HANDLER( hrdtimes_fgvideoram_w );
WRITE16_HANDLER( hrdtimes_bgvideoram_w );
WRITE16_HANDLER( bigtwin_paletteram_w );
WRITE16_HANDLER( bigtwin_scroll_w );
WRITE16_HANDLER( wbeachvl_scroll_w );
WRITE16_HANDLER( excelsr_scroll_w );
WRITE16_HANDLER( hrdtimes_scroll_w );

VIDEO_START( bigtwin );
VIDEO_START( wbeachvl );
VIDEO_START( excelsr );
VIDEO_START( hotmind );
VIDEO_START( hrdtimes );

VIDEO_UPDATE( bigtwin );
VIDEO_UPDATE( wbeachvl );
VIDEO_UPDATE( excelsr );
VIDEO_UPDATE( hrdtimes );
