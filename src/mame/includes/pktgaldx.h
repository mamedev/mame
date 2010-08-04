/*************************************************************************

    Pocket Gal Deluxe

*************************************************************************/

class pktgaldx_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, pktgaldx_state(machine)); }

	pktgaldx_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT16 *  pf1_rowscroll;
	UINT16 *  pf2_rowscroll;
	UINT16 *  spriteram;
//  UINT16 *  paletteram;    // currently this uses generic palette handling (in deco16ic.c)
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
