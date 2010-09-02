/*----------- defined in video/quizdna.c -----------*/

VIDEO_START( quizdna );
VIDEO_UPDATE( quizdna );

WRITE8_HANDLER( quizdna_fg_ram_w );
WRITE8_HANDLER( quizdna_bg_ram_w );
WRITE8_HANDLER( quizdna_bg_yscroll_w );
WRITE8_HANDLER( quizdna_bg_xscroll_w );
WRITE8_HANDLER( quizdna_screen_ctrl_w );

WRITE8_HANDLER( paletteram_xBGR_RRRR_GGGG_BBBB_w );
