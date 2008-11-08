/*----------- defined in drivers/suprnova.c -----------*/

extern UINT32 *skns_tilemapA_ram, *skns_tilemapB_ram, *skns_v3slc_ram;
extern UINT32 *skns_palette_ram;
extern UINT32 *skns_pal_regs, *skns_v3_regs, *skns_spc_regs;
extern UINT32 skns_v3t_dirty[0x4000]; // allocate this elsewhere?
extern UINT32 skns_v3t_4bppdirty[0x8000]; // allocate this elsewhere?
extern int skns_v3t_somedirty,skns_v3t_4bpp_somedirty;

/*----------- defined in video/suprnova.c -----------*/

void skns_sprite_kludge(int x, int y);
void skns_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);

WRITE32_HANDLER ( skns_tilemapA_w );
WRITE32_HANDLER ( skns_tilemapB_w );
WRITE32_HANDLER ( skns_v3_regs_w );
WRITE32_HANDLER ( skns_pal_regs_w );
WRITE32_HANDLER ( skns_palette_ram_w );
VIDEO_START(skns);
VIDEO_RESET(skns);
VIDEO_EOF(skns);
VIDEO_UPDATE(skns);
