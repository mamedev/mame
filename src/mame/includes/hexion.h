/*----------- defined in video/hexion.c -----------*/

VIDEO_START( hexion );
VIDEO_UPDATE( hexion );

WRITE8_HANDLER( hexion_bankswitch_w );
READ8_HANDLER( hexion_bankedram_r );
WRITE8_HANDLER( hexion_bankedram_w );
WRITE8_HANDLER( hexion_bankctrl_w );
WRITE8_HANDLER( hexion_gfxrom_select_w );
