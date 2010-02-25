/*************************************************************************

    Pocket Gal Deluxe

*************************************************************************/

typedef struct _pktgaldx_state pktgaldx_state;
struct _pktgaldx_state
{
	/* memory pointers */
	UINT16 *  pf1_rowscroll;
	UINT16 *  pf2_rowscroll;
	UINT16 *  spriteram;
//  UINT16 *  paletteram;    // currently this uses generic palette handling (in decodev.c)
	size_t    spriteram_size;

	UINT16*   pktgaldb_fgram;
	UINT16*   pktgaldb_sprites;

	/* devices */
	running_device *maincpu;
	running_device *deco16ic;
};



/*----------- defined in video/pktgaldx.c -----------*/

VIDEO_UPDATE( pktgaldx );
VIDEO_UPDATE( pktgaldb );
