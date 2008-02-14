/*----------- defined in video/f1gp.c -----------*/

extern UINT16 *f1gp_spr1vram,*f1gp_spr2vram,*f1gp_spr1cgram,*f1gp_spr2cgram;
extern UINT16 *f1gp_fgvideoram,*f1gp_rozvideoram;
extern UINT16 *f1gp2_sprcgram,*f1gp2_spritelist;
extern UINT16 *f1gpb_rozregs, *f1gpb_fgregs;
extern size_t f1gp_spr1cgram_size,f1gp_spr2cgram_size;


VIDEO_START( f1gp );
VIDEO_START( f1gpb );
VIDEO_START( f1gp2 );
VIDEO_UPDATE( f1gp );
VIDEO_UPDATE( f1gpb );
VIDEO_UPDATE( f1gp2 );

READ16_HANDLER( f1gp_zoomdata_r );
WRITE16_HANDLER( f1gp_zoomdata_w );
READ16_HANDLER( f1gp_rozvideoram_r );
WRITE16_HANDLER( f1gp_rozvideoram_w );
WRITE16_HANDLER( f1gp_fgvideoram_w );
WRITE16_HANDLER( f1gp_fgscroll_w );
WRITE16_HANDLER( f1gp_gfxctrl_w );
WRITE16_HANDLER( f1gp2_gfxctrl_w );
