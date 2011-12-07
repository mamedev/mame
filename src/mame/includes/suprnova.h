typedef struct
{
	UINT16 x1p, y1p, z1p, x1s, y1s, z1s;
	UINT16 x2p, y2p, z2p, x2s, y2s, z2s;
	UINT16 org;

	UINT16 x1_p1, x1_p2, y1_p1, y1_p2, z1_p1, z1_p2;
	UINT16 x2_p1, x2_p2, y2_p1, y2_p2, z2_p1, z2_p2;
	UINT16 x1tox2, y1toy2, z1toz2;
	INT16 x_in, y_in, z_in;
	UINT16 flag;

	UINT8 disconnect;
} hit_t;


class skns_state : public driver_device
{
public:
	skns_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu")
		{ }

	sknsspr_device* m_spritegen;
	UINT32 *m_tilemapA_ram;
	UINT32 *m_tilemapB_ram;
	UINT32 *m_v3slc_ram;
	UINT32 *m_palette_ram;
	UINT32 *m_pal_regs;
	UINT32 *m_v3_regs;
	UINT32 *m_spc_regs;
	UINT32 *m_v3t_ram;
	UINT32 *m_main_ram;
	UINT32 *m_cache_ram;
	hit_t m_hit;
	UINT32 m_timer_0_temp[4];
	bitmap_t *m_sprite_bitmap;
	bitmap_t *m_tilemap_bitmap_lower;
	bitmap_t *m_tilemap_bitmapflags_lower;
	bitmap_t *m_tilemap_bitmap_higher;
	bitmap_t *m_tilemap_bitmapflags_higher;
	int m_depthA;
	int m_depthB;
	int m_use_spc_bright;
	int m_use_v3_bright;
	UINT8 m_bright_spc_b;
	UINT8 m_bright_spc_g;
	UINT8 m_bright_spc_r;
	UINT8 m_bright_spc_b_trans;
	UINT8 m_bright_spc_g_trans;
	UINT8 m_bright_spc_r_trans;
	UINT8 m_bright_v3_b;
	UINT8 m_bright_v3_g;
	UINT8 m_bright_v3_r;
	UINT8 m_bright_v3_b_trans;
	UINT8 m_bright_v3_g_trans;
	UINT8 m_bright_v3_r_trans;
	int m_spc_changed;
	int m_v3_changed;
	int m_palette_updated;
	int m_alt_enable_background;
	int m_alt_enable_sprites;
	tilemap_t *m_tilemap_A;
	tilemap_t *m_tilemap_B;

	UINT8 m_region;

	required_device<cpu_device> m_maincpu;
};


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
