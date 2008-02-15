/***************************************************************************

    Mad Alien (c) 1980 Data East Corporation

    Original driver by Norbert Kehrer (February 2004)

***************************************************************************/

#define MADALIEN_MAIN_CLOCK		(10595000)


/*----------- defined in video/madalien.c -----------*/

extern UINT8 *madalien_videoram;
extern UINT8 *madalien_charram;

extern UINT8 *madalien_video_flags;
extern UINT8 *madalien_video_control;
extern UINT8 *madalien_scroll;
extern UINT8 *madalien_edge1_pos;
extern UINT8 *madalien_edge2_pos;
extern UINT8 *madalien_headlight_pos;

WRITE8_HANDLER( madalien_mc6845_address_w );
READ8_HANDLER( madalien_mc6845_register_r );
WRITE8_HANDLER( madalien_mc6845_register_w );

MACHINE_DRIVER_EXTERN( madalien_video );
