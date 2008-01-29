/***************************************************************************

	Pooyan
	
***************************************************************************/

/*----------- defined in video/pooyan.c -----------*/

WRITE8_HANDLER( pooyan_videoram_w );
WRITE8_HANDLER( pooyan_colorram_w );
WRITE8_HANDLER( pooyan_flipscreen_w );

PALETTE_INIT( pooyan );
VIDEO_START( pooyan );
VIDEO_UPDATE( pooyan );
