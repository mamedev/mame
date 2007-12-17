/***************************************************************************

                            -= Seta Hardware =-

***************************************************************************/

#define	__uPD71054_TIMER	1

/*----------- defined in video/seta.c -----------*/

extern UINT16 *seta_vram_0, *seta_vctrl_0;
extern UINT16 *seta_vram_2, *seta_vctrl_2;
extern UINT16 *seta_vregs;

extern UINT16 *seta_workram; // Needed for zombraid Crosshair hack

extern int seta_tiles_offset;

void seta_coin_lockout_w(int data);

WRITE16_HANDLER( twineagl_tilebank_w );

WRITE16_HANDLER( seta_vram_0_w );
WRITE16_HANDLER( seta_vram_2_w );
WRITE16_HANDLER( seta_vregs_w );

PALETTE_INIT( blandia );
PALETTE_INIT( gundhara );
PALETTE_INIT( jjsquawk );
PALETTE_INIT( usclssic );
PALETTE_INIT( zingzip );

VIDEO_START( seta_no_layers);
VIDEO_START( twineagl_1_layer);
VIDEO_START( seta_1_layer);
VIDEO_START( seta_2_layers);
VIDEO_START( oisipuzl_2_layers );

VIDEO_UPDATE( seta );
VIDEO_UPDATE( seta_no_layers );


/*----------- defined in video/seta2.c -----------*/

extern UINT16 *seta2_vregs;

WRITE16_HANDLER( seta2_vregs_w );

VIDEO_START( seta2 );
VIDEO_START( seta2_offset );
VIDEO_UPDATE( seta2 );
VIDEO_EOF( seta2 );


/*----------- defined in video/ssv.c -----------*/

extern UINT16 *ssv_scroll;

extern int ssv_special;

extern int ssv_tile_code[16];

extern int ssv_sprites_offsx, ssv_sprites_offsy;
extern int ssv_tilemap_offsx, ssv_tilemap_offsy;

extern UINT16 *eaglshot_gfxram, *gdfs_tmapram, *gdfs_tmapscroll;
extern char eaglshot_dirty, *eaglshot_dirty_tile;

READ16_HANDLER( ssv_vblank_r );
WRITE16_HANDLER( ssv_scroll_w );
WRITE16_HANDLER( paletteram16_xrgb_swap_word_w );
WRITE16_HANDLER( gdfs_tmapram_w );
void ssv_enable_video(int enable);

VIDEO_START( ssv );
VIDEO_START( eaglshot );
VIDEO_START( gdfs );

VIDEO_UPDATE( ssv );
VIDEO_UPDATE( eaglshot );
VIDEO_UPDATE( gdfs );
