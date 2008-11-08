/*----------- defined in drivers/drgnmst.c -----------*/

extern UINT16 *drgnmst_vidregs;

extern UINT16 *drgnmst_fg_videoram;
extern UINT16 *drgnmst_bg_videoram;
extern UINT16 *drgnmst_md_videoram;

extern UINT16 *drgnmst_rowscrollram;
extern UINT16 *drgnmst_vidregs2;


/*----------- defined in video/drgnmst.c -----------*/

WRITE16_HANDLER( drgnmst_fg_videoram_w );
WRITE16_HANDLER( drgnmst_bg_videoram_w );
WRITE16_HANDLER( drgnmst_md_videoram_w );
VIDEO_START(drgnmst);
VIDEO_UPDATE(drgnmst);
