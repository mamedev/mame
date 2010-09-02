/*----------- defined in video/dcon.c -----------*/

extern UINT16 *dcon_back_data,*dcon_fore_data,*dcon_mid_data,*dcon_scroll_ram,*dcon_textram;

WRITE16_HANDLER( dcon_gfxbank_w );
WRITE16_HANDLER( dcon_background_w );
WRITE16_HANDLER( dcon_foreground_w );
WRITE16_HANDLER( dcon_midground_w );
WRITE16_HANDLER( dcon_text_w );
WRITE16_HANDLER( dcon_control_w );
READ16_HANDLER( dcon_control_r );

VIDEO_START( dcon );
VIDEO_UPDATE( dcon );
VIDEO_UPDATE( sdgndmps );
