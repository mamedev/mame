/*----------- defined in drivers/mcatadv.c -----------*/

extern UINT16 *mcatadv_videoram1, *mcatadv_videoram2;
extern UINT16 *mcatadv_scroll, *mcatadv_scroll2;
extern UINT16 *mcatadv_vidregs;


/*----------- defined in video/mcatadv.c -----------*/

VIDEO_UPDATE( mcatadv );
VIDEO_START( mcatadv );
VIDEO_EOF( mcatadv );

WRITE16_HANDLER( mcatadv_videoram1_w );
WRITE16_HANDLER( mcatadv_videoram2_w );
