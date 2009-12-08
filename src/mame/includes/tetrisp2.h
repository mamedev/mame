/*----------- defined in drivers/tetrisp2.c -----------*/

extern UINT16 tetrisp2_systemregs[0x10];


/*----------- defined in video/tetrisp2.c -----------*/

extern UINT16 *tetrisp2_vram_bg, *tetrisp2_scroll_bg;
extern UINT16 *tetrisp2_vram_fg, *tetrisp2_scroll_fg;
extern UINT16 *tetrisp2_vram_rot, *tetrisp2_rotregs;

extern UINT8 *tetrisp2_priority;

extern UINT16 *rocknms_sub_vram_bg, *rocknms_sub_scroll_bg;
extern UINT16 *rocknms_sub_vram_fg, *rocknms_sub_scroll_fg;
extern UINT16 *rocknms_sub_vram_rot, *rocknms_sub_rotregs;

extern UINT16 *rocknms_sub_priority;

WRITE16_HANDLER( tetrisp2_palette_w );
WRITE16_HANDLER( rocknms_sub_palette_w );
WRITE8_HANDLER( tetrisp2_priority_w );
WRITE8_HANDLER( rockn_priority_w );
READ8_HANDLER( tetrisp2_priority_r );
WRITE16_HANDLER( rocknms_sub_priority_w );
READ16_HANDLER( nndmseal_priority_r );

WRITE16_HANDLER( tetrisp2_vram_bg_w );
WRITE16_HANDLER( tetrisp2_vram_fg_w );
WRITE16_HANDLER( tetrisp2_vram_rot_w );

WRITE16_HANDLER( rocknms_sub_vram_bg_w );
WRITE16_HANDLER( rocknms_sub_vram_fg_w );
WRITE16_HANDLER( rocknms_sub_vram_rot_w );

VIDEO_START( tetrisp2 );
VIDEO_UPDATE( tetrisp2 );

VIDEO_START( rockntread );
VIDEO_UPDATE( rockntread );

VIDEO_START( rocknms );
VIDEO_UPDATE( rocknms );

VIDEO_START( nndmseal );
void tetrisp2_draw_sprites(running_machine *machine, bitmap_t *bitmap, bitmap_t *bitmap_pri, const rectangle *cliprect, UINT8* priram, UINT16 *sprram_top, size_t sprram_size, int gfxnum, int reverseorder, int flip, int allowzoom);
