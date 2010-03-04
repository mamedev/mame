
class fitfight_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, fitfight_state(machine)); }

	fitfight_state(running_machine &machine) { }

	/* memory pointers */
	UINT16 *  fof_100000;
	UINT16 *  fof_600000;
	UINT16 *  fof_700000;
	UINT16 *  fof_800000;
	UINT16 *  fof_900000;
	UINT16 *  fof_a00000;
	UINT16 *  fof_bak_tileram;
	UINT16 *  fof_mid_tileram;
	UINT16 *  fof_txt_tileram;
	UINT16 *  spriteram;
//  UINT16 *  paletteram;   // currently this uses generic palette handling

	/* video-related */
	tilemap_t  *fof_bak_tilemap, *fof_mid_tilemap, *fof_txt_tilemap;

	/* misc */
	int      bbprot_kludge;
	UINT16   fof_700000_data;
};


/*----------- defined in video/fitfight.c -----------*/

WRITE16_HANDLER( fof_bak_tileram_w );
WRITE16_HANDLER( fof_mid_tileram_w );
WRITE16_HANDLER( fof_txt_tileram_w );

VIDEO_START(fitfight);
VIDEO_UPDATE(fitfight);
