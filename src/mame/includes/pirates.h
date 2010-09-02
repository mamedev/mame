/*----------- defined in video/pirates.c -----------*/

extern UINT16 *pirates_tx_tileram, *pirates_spriteram;
extern UINT16 *pirates_fg_tileram,  *pirates_bg_tileram;
extern UINT16 *pirates_scroll;

WRITE16_HANDLER( pirates_tx_tileram_w );
WRITE16_HANDLER( pirates_fg_tileram_w );
WRITE16_HANDLER( pirates_bg_tileram_w );

VIDEO_START( pirates );
VIDEO_UPDATE( pirates );
