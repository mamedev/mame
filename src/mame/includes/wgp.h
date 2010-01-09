/*************************************************************************

    World Grand Prix

*************************************************************************/

typedef struct _wgp_state wgp_state;
struct _wgp_state
{
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
	const device_config *maincpu;
	const device_config *audiocpu;
	const device_config *subcpu;
	const device_config *tc0100scn;
	const device_config *tc0140syt;
};


/*----------- defined in video/wgp.c -----------*/

READ16_HANDLER ( wgp_pivram_word_r );
WRITE16_HANDLER( wgp_pivram_word_w );

READ16_HANDLER ( wgp_piv_ctrl_word_r );
WRITE16_HANDLER( wgp_piv_ctrl_word_w );

VIDEO_START( wgp );
VIDEO_START( wgp2 );
VIDEO_UPDATE( wgp );
