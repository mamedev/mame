/*************************************************************************

    Pocket Gal Deluxe

*************************************************************************/

class pktgaldx_state : public driver_device
{
public:
	pktgaldx_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *  pf1_rowscroll;
	UINT16 *  pf2_rowscroll;
	UINT16 *  spriteram;
//  UINT16 *  paletteram;    // currently this uses generic palette handling (in deco16ic.c)
	size_t    spriteram_size;

	UINT16*   pktgaldb_fgram;
	UINT16*   pktgaldb_sprites;

	/* devices */
	device_t *maincpu;
	device_t *deco16ic;
};



/*----------- defined in video/pktgaldx.c -----------*/

VIDEO_UPDATE( pktgaldx );
VIDEO_UPDATE( pktgaldb );
