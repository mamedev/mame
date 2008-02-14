/*************************************************************************

    Atari Cops'n Robbers hardware

*************************************************************************/

/*----------- defined in machine/copsnrob.c -----------*/

READ8_HANDLER( copsnrob_gun_position_r );


/*----------- defined in video/copsnrob.c -----------*/

extern UINT8 *copsnrob_bulletsram;
extern UINT8 *copsnrob_carimage;
extern UINT8 *copsnrob_cary;
extern UINT8 *copsnrob_trucky;
extern UINT8 *copsnrob_truckram;

VIDEO_UPDATE( copsnrob );
