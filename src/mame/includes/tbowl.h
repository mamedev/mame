/*----------- defined in video/tbowl.c -----------*/

extern UINT8 *tbowl_txvideoram, *tbowl_bgvideoram, *tbowl_bg2videoram;
extern UINT8 *tbowl_spriteram;

WRITE8_HANDLER( tbowl_bg2videoram_w );
WRITE8_HANDLER( tbowl_bgvideoram_w );
WRITE8_HANDLER( tbowl_txvideoram_w );

WRITE8_HANDLER( tbowl_bg2xscroll_lo );
WRITE8_HANDLER( tbowl_bg2xscroll_hi );
WRITE8_HANDLER( tbowl_bg2yscroll_lo );
WRITE8_HANDLER( tbowl_bg2yscroll_hi );
WRITE8_HANDLER( tbowl_bgxscroll_lo );
WRITE8_HANDLER( tbowl_bgxscroll_hi );
WRITE8_HANDLER( tbowl_bgyscroll_lo );
WRITE8_HANDLER( tbowl_bgyscroll_hi );

VIDEO_START( tbowl );
VIDEO_UPDATE( tbowl );
