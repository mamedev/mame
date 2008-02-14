/*----------- defined in video/deco32.c -----------*/

VIDEO_START( captaven );
VIDEO_START( fghthist );
VIDEO_START( dragngun );
VIDEO_START( lockload );
VIDEO_START( nslasher );

VIDEO_EOF( captaven );
VIDEO_EOF( dragngun );

VIDEO_UPDATE( captaven );
VIDEO_UPDATE( fghthist );
VIDEO_UPDATE( dragngun );
VIDEO_UPDATE( nslasher );

extern UINT32 *deco32_pf1_data,*deco32_pf2_data,*deco32_pf3_data,*deco32_pf4_data;
extern UINT32 *deco32_pf12_control,*deco32_pf34_control;
extern UINT32 *deco32_pf1_rowscroll,*deco32_pf2_rowscroll,*deco32_pf3_rowscroll,*deco32_pf4_rowscroll;
extern UINT32 *dragngun_sprite_layout_0_ram, *dragngun_sprite_layout_1_ram;
extern UINT32 *dragngun_sprite_lookup_0_ram, *dragngun_sprite_lookup_1_ram;
extern UINT32 *deco32_ace_ram;
extern UINT16 *deco32_raster_display_list;
extern int deco32_raster_display_position;

WRITE32_HANDLER( deco32_pf1_data_w );
WRITE32_HANDLER( deco32_pf2_data_w );
WRITE32_HANDLER( deco32_pf3_data_w );
WRITE32_HANDLER( deco32_pf4_data_w );

WRITE32_HANDLER( deco32_nonbuffered_palette_w );
WRITE32_HANDLER( deco32_buffered_palette_w );
WRITE32_HANDLER( deco32_palette_dma_w );

WRITE32_HANDLER( deco32_pri_w );
WRITE32_HANDLER( dragngun_sprite_control_w );
WRITE32_HANDLER( dragngun_spriteram_dma_w );
WRITE32_HANDLER( deco32_ace_ram_w );
