/***************************************************************************

	Tutankham
	
***************************************************************************/

/*----------- defined in video/tutankhm.c -----------*/

extern UINT8 *tutankhm_scroll;

WRITE8_HANDLER( tutankhm_flip_screen_x_w );
WRITE8_HANDLER( tutankhm_flip_screen_y_w );

VIDEO_START( tutankhm );
VIDEO_UPDATE( tutankhm );

WRITE8_HANDLER( junofrst_blitter_w );
