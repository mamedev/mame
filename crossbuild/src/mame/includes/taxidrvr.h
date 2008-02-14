/*----------- defined in video/taxidrvr.c -----------*/

extern UINT8 *taxidrvr_vram0,*taxidrvr_vram1,*taxidrvr_vram2,*taxidrvr_vram3;
extern UINT8 *taxidrvr_vram4,*taxidrvr_vram5,*taxidrvr_vram6,*taxidrvr_vram7;
extern UINT8 *taxidrvr_scroll;
extern int taxidrvr_bghide;

WRITE8_HANDLER( taxidrvr_spritectrl_w );

VIDEO_UPDATE( taxidrvr );
