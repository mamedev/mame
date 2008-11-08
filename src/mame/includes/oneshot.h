/*----------- defined in drivers/oneshot.c -----------*/

extern UINT16 *oneshot_sprites;
extern UINT16 *oneshot_bg_videoram;
extern UINT16 *oneshot_mid_videoram;
extern UINT16 *oneshot_fg_videoram;
extern UINT16 *oneshot_scroll;

extern int gun_x_p1,gun_y_p1,gun_x_p2,gun_y_p2;
extern int gun_x_shift;


/*----------- defined in video/oneshot.c -----------*/

WRITE16_HANDLER( oneshot_bg_videoram_w );
WRITE16_HANDLER( oneshot_mid_videoram_w );
WRITE16_HANDLER( oneshot_fg_videoram_w );
VIDEO_START( oneshot );
VIDEO_UPDATE( oneshot );
VIDEO_UPDATE( maddonna );
