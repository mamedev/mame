
class f1gp_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, f1gp_state(machine)); }

	f1gp_state(running_machine &machine) { }

	/* memory pointers */
	UINT16 *  sharedram;
	UINT16 *  spr1vram;
	UINT16 *  spr2vram;
	UINT16 *  spr1cgram;
	UINT16 *  spr2cgram;
	UINT16 *  fgvideoram;
	UINT16 *  rozvideoram;
	UINT16 *  sprcgram;
	UINT16 *  spritelist;
	UINT16 *  spriteram;
	UINT16 *  fgregs;
	UINT16 *  rozregs;
	UINT16 *  zoomdata;
//      UINT16 *  paletteram;    // currently this uses generic palette handling
	size_t    spr1cgram_size, spr2cgram_size;
	size_t    spriteram_size;

	/* video-related */
	tilemap_t   *fg_tilemap, *roz_tilemap;
	int       roz_bank, flipscreen, gfxctrl;
	int       scroll[2];

	/* misc */
	int       pending_command;

	/* devices */
	running_device *audiocpu;
	running_device *k053936;
};

/*----------- defined in video/f1gp.c -----------*/

READ16_HANDLER( f1gp_zoomdata_r );
WRITE16_HANDLER( f1gp_zoomdata_w );
READ16_HANDLER( f1gp_rozvideoram_r );
WRITE16_HANDLER( f1gp_rozvideoram_w );
WRITE16_HANDLER( f1gp_fgvideoram_w );
WRITE16_HANDLER( f1gp_fgscroll_w );
WRITE16_HANDLER( f1gp_gfxctrl_w );
WRITE16_HANDLER( f1gp2_gfxctrl_w );

VIDEO_START( f1gp );
VIDEO_START( f1gpb );
VIDEO_START( f1gp2 );
VIDEO_UPDATE( f1gp );
VIDEO_UPDATE( f1gpb );
VIDEO_UPDATE( f1gp2 );
