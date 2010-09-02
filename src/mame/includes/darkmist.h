/*----------- defined in drivers/darkmist.c -----------*/

extern int darkmist_hw;


/*----------- defined in video/darkmist.c -----------*/

VIDEO_START( darkmist );
VIDEO_UPDATE( darkmist );
PALETTE_INIT( darkmist );

extern UINT8 *darkmist_scroll;
extern UINT8 *darkmist_spritebank;
