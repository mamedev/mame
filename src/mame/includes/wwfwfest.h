/*----------- defined in video/wwfwfest.c -----------*/

extern UINT16 *wwfwfest_fg0_videoram, *wwfwfest_bg0_videoram, *wwfwfest_bg1_videoram;
extern int wwfwfest_pri;
extern int wwfwfest_bg0_scrollx, wwfwfest_bg0_scrolly, wwfwfest_bg1_scrollx, wwfwfest_bg1_scrolly;

VIDEO_START( wwfwfest );
VIDEO_START( wwfwfstb );
VIDEO_UPDATE( wwfwfest );
WRITE16_HANDLER( wwfwfest_fg0_videoram_w );
WRITE16_HANDLER( wwfwfest_bg0_videoram_w );
WRITE16_HANDLER( wwfwfest_bg1_videoram_w );
