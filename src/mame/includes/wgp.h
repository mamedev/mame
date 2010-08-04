/*************************************************************************

    World Grand Prix

*************************************************************************/

class wgp_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, wgp_state(machine)); }

	wgp_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT16 *    spritemap;
	UINT16 *    spriteram;
	UINT16 *    pivram;
	UINT16 *    piv_ctrlram;
	UINT16 *    sharedram;
//  UINT16 *    paletteram;    // currently this uses generic palette handling
	size_t      sharedram_size;
	size_t      spritemap_size;
	size_t      spriteram_size;

	/* video-related */
	tilemap_t   *piv_tilemap[3];
	UINT16      piv_ctrl_reg;
	UINT16      piv_zoom[3], piv_scrollx[3], piv_scrolly[3];
	UINT16      rotate_ctrl[8];
	int         piv_xoffs, piv_yoffs;

	/* misc */
	UINT16      cpua_ctrl;
	UINT16      port_sel;
	INT32       banknum;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *subcpu;
	running_device *tc0100scn;
	running_device *tc0140syt;
};


/*----------- defined in video/wgp.c -----------*/

READ16_HANDLER ( wgp_pivram_word_r );
WRITE16_HANDLER( wgp_pivram_word_w );

READ16_HANDLER ( wgp_piv_ctrl_word_r );
WRITE16_HANDLER( wgp_piv_ctrl_word_w );

VIDEO_START( wgp );
VIDEO_START( wgp2 );
VIDEO_UPDATE( wgp );
