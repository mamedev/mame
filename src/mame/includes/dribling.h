/*************************************************************************

    Model Racing Dribbling hardware

*************************************************************************/

/*----------- defined in drivers/dribling.c -----------*/

extern UINT8 dribling_abca;


/*----------- defined in video/dribling.c -----------*/

PALETTE_INIT( dribling );
WRITE8_HANDLER( dribling_colorram_w );
VIDEO_UPDATE( dribling );
