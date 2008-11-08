/*----------- defined in drivers/xmen.c -----------*/

extern UINT16 xmen_current_frame;
extern UINT16 *xmen6p_spriteramleft;
extern UINT16 *xmen6p_spriteramright;
extern UINT16 *xmen6p_tilemapleft;
extern UINT16 *xmen6p_tilemapright;


/*----------- defined in video/xmen.c -----------*/

VIDEO_START( xmen );
VIDEO_START( xmen6p );
VIDEO_UPDATE( xmen );
VIDEO_UPDATE( xmen6p );
VIDEO_EOF( xmen6p );
