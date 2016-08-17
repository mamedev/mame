// license:BSD-3-Clause
// copyright-holders:Quench, David Haywood
/* GP9001 Video Controller */

struct gp9001layeroffsets
{
	int normal;
	int flipped;
};

struct gp9001layer
{
	UINT16 flip;
	UINT16 scrollx;
	UINT16 scrolly;

	gp9001layeroffsets extra_xoffset;
	gp9001layeroffsets extra_yoffset;
};

struct gp9001tilemaplayer : gp9001layer
{
	tilemap_t *tmap;
};

struct gp9001spritelayer : gp9001layer
{
	bool use_sprite_buffer;
	std::unique_ptr<UINT16[]> vram16_buffer; // vram buffer for this layer
};


class gp9001vdp_device : public device_t,
							public device_gfx_interface,
							public device_video_interface,
							public device_memory_interface
{
	static const gfx_layout tilelayout, spritelayout;
	DECLARE_GFXDECODE_MEMBER(gfxinfo);

public:
	gp9001vdp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	UINT16 gp9001_voffs;
	UINT16 gp9001_scroll_reg;

	gp9001tilemaplayer bg, top, fg;
	gp9001spritelayer sp;

	// technically this is just rom banking, allowing the chip to see more graphic ROM, however it's easier to handle it
	// in the chip implementation than externally for now (which would require dynamic decoding of the entire charsets every
	// time the bank was changed)
	int gp9001_gfxrom_is_banked;
	int gp9001_gfxrom_bank_dirty;       /* dirty flag of object bank (for Batrider) */
	UINT16 gp9001_gfxrom_bank[8];       /* Batrider object bank */


	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, const UINT8* primap );
	void gp9001_draw_custom_tilemap( bitmap_ind16 &bitmap, tilemap_t* tilemap, const UINT8* priremap, const UINT8* pri_enable );
	void gp9001_render_vdp( bitmap_ind16 &bitmap, const rectangle &cliprect);
	void gp9001_screen_eof(void);
	void create_tilemaps(void);
	void init_scroll_regs(void);

	bitmap_ind8 *custom_priority_bitmap;

	DECLARE_ADDRESS_MAP(map, 16);

	// access to VDP
	DECLARE_READ16_MEMBER( gp9001_vdp_r );
	DECLARE_WRITE16_MEMBER( gp9001_vdp_w );
	DECLARE_READ16_MEMBER( gp9001_vdp_alt_r );
	DECLARE_WRITE16_MEMBER( gp9001_vdp_alt_w );

	// this bootleg has strange access
	DECLARE_READ16_MEMBER( pipibibi_bootleg_videoram16_r );
	DECLARE_WRITE16_MEMBER( pipibibi_bootleg_videoram16_w );
	DECLARE_READ16_MEMBER( pipibibi_bootleg_spriteram16_r );
	DECLARE_WRITE16_MEMBER( pipibibi_bootleg_spriteram16_w );
	DECLARE_WRITE16_MEMBER( pipibibi_bootleg_scroll_w );

	// internal handlers
	DECLARE_WRITE16_MEMBER( gp9001_bg_tmap_w );
	DECLARE_WRITE16_MEMBER( gp9001_fg_tmap_w );
	DECLARE_WRITE16_MEMBER( gp9001_top_tmap_w );

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	address_space_config        m_space_config;

	TILE_GET_INFO_MEMBER(get_top0_tile_info);
	TILE_GET_INFO_MEMBER(get_fg0_tile_info);
	TILE_GET_INFO_MEMBER(get_bg0_tile_info);

private:
	required_shared_ptr<UINT16> m_vram_bg;
	required_shared_ptr<UINT16> m_vram_fg;
	required_shared_ptr<UINT16> m_vram_top;
	required_shared_ptr<UINT16> m_spriteram;

	void gp9001_voffs_w(UINT16 data, UINT16 mem_mask);
	int gp9001_videoram16_r(void);
	void gp9001_videoram16_w(UINT16 data, UINT16 mem_mask);
	UINT16 gp9001_vdpstatus_r(void);
	void gp9001_scroll_reg_select_w(UINT16 data, UINT16 mem_mask);
	void gp9001_scroll_reg_data_w(UINT16 data, UINT16 mem_mask);
};

extern const device_type GP9001_VDP;

#define GP9001_BG_VRAM_SIZE   0x1000    /* Background RAM size */
#define GP9001_FG_VRAM_SIZE   0x1000    /* Foreground RAM size */
#define GP9001_TOP_VRAM_SIZE  0x1000    /* Top Layer  RAM size */
#define GP9001_SPRITERAM_SIZE 0x800 /* Sprite     RAM size */
#define GP9001_SPRITE_FLIPX 0x1000  /* Sprite flip flags */
#define GP9001_SPRITE_FLIPY 0x2000
