/*----------- defined in video/spdodgeb.c -----------*/

extern UINT8 *spdodgeb_videoram;

PALETTE_INIT( spdodgeb );
VIDEO_START( spdodgeb );
VIDEO_UPDATE( spdodgeb );
INTERRUPT_GEN( spdodgeb_interrupt );
WRITE8_HANDLER( spdodgeb_scrollx_lo_w );
WRITE8_HANDLER( spdodgeb_ctrl_w );
WRITE8_HANDLER( spdodgeb_videoram_w );
