/*----------- defined in video/deco16ic.c -----------*/

extern UINT16 *deco16_pf1_data,*deco16_pf2_data;
extern UINT16 *deco16_pf3_data,*deco16_pf4_data;
extern UINT16 *deco16_pf1_rowscroll,*deco16_pf2_rowscroll;
extern UINT16 *deco16_pf3_rowscroll,*deco16_pf4_rowscroll;

extern UINT16 *deco16_pf12_control,*deco16_pf34_control;
extern UINT16 deco16_priority;

extern UINT16 *deco16_raster_display_list;
extern int deco16_raster_display_position;

extern int deco16_pf1_colour_bank,deco16_pf2_colour_bank,deco16_pf3_colour_bank,deco16_pf4_colour_bank;

void deco16_1_video_init(void);
void deco16_2_video_init(int split);
void deco16_2_video_init_half_width(void);
void deco_allocate_sprite_bitmap(void);

void deco16_pf12_update(const UINT16 *rowscroll_1_ptr, const UINT16 *rowscroll_2_ptr);
void deco16_pf34_update(const UINT16 *rowscroll_1_ptr, const UINT16 *rowscroll_2_ptr);

void deco16_pf12_set_gfxbank(int small, int big);
void deco16_pf34_set_gfxbank(int small, int big);

void deco16_set_tilemap_bank_callback(int tmap, int (*callback)(const int bank));
void deco16_set_tilemap_colour_base(int tmap, int base);
void deco16_set_tilemap_colour_mask(int tmap, int mask);
void deco16_set_tilemap_transparency_mask(int tmap, int mask);

void deco16_tilemap_1_draw(mame_bitmap *bitmap, const rectangle *cliprect, int flags, UINT32 priority);
void deco16_tilemap_2_draw(mame_bitmap *bitmap, const rectangle *cliprect, int flags, UINT32 priority);
void deco16_tilemap_3_draw(mame_bitmap *bitmap, const rectangle *cliprect, int flags, UINT32 priority);
void deco16_tilemap_4_draw(mame_bitmap *bitmap, const rectangle *cliprect, int flags, UINT32 priority);

void deco16_tilemap_34_combine_draw(mame_bitmap *bitmap, const rectangle *cliprect, int flags, UINT32 priority);

void deco16_clear_sprite_priority_bitmap(void);
void deco16_pdrawgfx(mame_bitmap *dest,const gfx_element *gfx,
		UINT32 code,UINT32 color,int flipx,int flipy,int sx,int sy,
		const rectangle *clip,int transparency,int transparent_color,UINT32 pri_mask,UINT32 sprite_mask,UINT8 write_pri);

tilemap *deco16_get_tilemap(int pf, int size);

WRITE16_HANDLER( deco16_pf1_data_w );
WRITE16_HANDLER( deco16_pf2_data_w );
WRITE16_HANDLER( deco16_pf3_data_w );
WRITE16_HANDLER( deco16_pf4_data_w );

WRITE16_HANDLER( deco16_nonbuffered_palette_w );
WRITE16_HANDLER( deco16_buffered_palette_w );
WRITE16_HANDLER( deco16_palette_dma_w );

WRITE16_HANDLER( deco16_priority_w );

READ16_HANDLER( deco16_71_r );

void deco16_print_debug_info(mame_bitmap *bitmap);
