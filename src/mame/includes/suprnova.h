/*----------- defined in drivers/suprnova.c -----------*/

class skns_state : public driver_device
{
public:
	skns_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	sknsspr_device* m_spritegen;
};

extern UINT32 *skns_tilemapA_ram, *skns_tilemapB_ram, *skns_v3slc_ram;
extern UINT32 *skns_palette_ram;
extern UINT32 *skns_pal_regs, *skns_v3_regs, *skns_spc_regs;

/*----------- defined in video/suprnova.c -----------*/

void skns_sprite_kludge(int x, int y);
void skns_draw_sprites(
	running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect,
	UINT32* spriteram_source, size_t spriteram_size,
	UINT8* gfx_source, size_t gfx_length,
	UINT32* sprite_regs );

WRITE32_HANDLER ( skns_tilemapA_w );
WRITE32_HANDLER ( skns_tilemapB_w );
WRITE32_HANDLER ( skns_v3_regs_w );
WRITE32_HANDLER ( skns_pal_regs_w );
WRITE32_HANDLER ( skns_palette_ram_w );
VIDEO_START(skns);
VIDEO_RESET(skns);
SCREEN_EOF(skns);
SCREEN_UPDATE(skns);
