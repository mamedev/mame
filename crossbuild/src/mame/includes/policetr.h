/*************************************************************************

    P&P Marketing Police Trainer hardware

**************************************************************************/

/*----------- defined in drivers/policetr.c -----------*/

extern UINT32 *	policetr_rambase;


/*----------- defined in video/policetr.c -----------*/

WRITE32_HANDLER( policetr_video_w );
READ32_HANDLER( policetr_video_r );

WRITE32_HANDLER( policetr_palette_offset_w );
WRITE32_HANDLER( policetr_palette_data_w );

VIDEO_START( policetr );
VIDEO_UPDATE( policetr );
