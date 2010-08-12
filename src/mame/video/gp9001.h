/* GP9001 Video Controller */

extern bitmap_t* gp9001_custom_priority_bitmap;

class gp9001vdp_device_config : public device_config,
								 public device_config_memory_interface
{
	friend class gp9001vdp_device;
	gp9001vdp_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
public:
	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
	virtual device_t *alloc_device(running_machine &machine) const;
protected:
	virtual void device_config_complete();
	virtual bool device_validity_check(const game_driver &driver) const;
	virtual const address_space_config *memory_space_config(int spacenum = 0) const;
	address_space_config		m_space_config;
	UINT8						m_gfxregion;

};

class gp9001vdp_device : public device_t,
						  public device_memory_interface
{
	friend class gp9001vdp_device_config;
	gp9001vdp_device(running_machine &_machine, const gp9001vdp_device_config &config);
public:
	UINT16 gp9001_voffs;
	UINT16 *bgvideoram16;
	UINT16 *fgvideoram16;
	UINT16 *topvideoram16;

	UINT16 *spriteram16_now;	/* Sprites to draw this frame */
	UINT16 *spriteram16_new;	/* Sprites to add to next frame */
	UINT16 *spriteram16_n;

	UINT16 gp9001_scroll_reg;
	UINT16 bg_scrollx;
	UINT16 bg_scrolly;
	UINT16 fg_scrollx;
	UINT16 fg_scrolly;
	UINT16 top_scrollx;
	UINT16 top_scrolly;
	UINT16 sprite_scrollx;
	UINT16 sprite_scrolly;

	UINT8 bg_flip;
	UINT8 fg_flip;
	UINT8 top_flip;
	UINT8 sprite_flip;

	UINT16 tile_limit; // prevent bad tile in Batsugun, might be something like the CPS1 tile addressing limits?
	int	   tile_region; // we also use this to figure out which vdp we're using in some debug logging features
	tilemap_t *top_tilemap, *fg_tilemap, *bg_tilemap;

	// debug
	int display_bg;
	int display_fg;
	int display_top;
	int display_sp;

	// technically this is just rom banking, allowing the chip to see more graphic ROM, however it's easier to handle it
	// in the chip implementation than externally for now (which would require dynamic decoding of the entire charsets every
	// time the bank was changed)
	int gp9001_gfxrom_is_banked;
	int gp9001_gfxrom_bank_dirty;		/* dirty flag of object bank (for Batrider) */
	UINT16 gp9001_gfxrom_bank[8];		/* Batrider object bank */


	void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, const UINT8* primap );
	void gp9001_draw_custom_tilemap(running_machine* machine, bitmap_t* bitmap, tilemap_t* tilemap, const UINT8* priremap, const UINT8* pri_enable );
	void gp9001_render_vdp(running_machine* machine, bitmap_t* bitmap, const rectangle* cliprect);
	void gp9001_video_eof(void);

	// offset kludges, needed by fixeight bootleg
	int extra_xoffset[4];
	int extra_yoffset[4];

protected:
	virtual void device_start();
	virtual void device_reset();
	const gp9001vdp_device_config &m_config;
	UINT8						m_gfxregion;

};

const device_type gp9001vdp_ = gp9001vdp_device_config::static_alloc_device_config;

#define GP9001_BG_VRAM_SIZE   0x1000	/* Background RAM size */
#define GP9001_FG_VRAM_SIZE   0x1000	/* Foreground RAM size */
#define GP9001_TOP_VRAM_SIZE  0x1000	/* Top Layer  RAM size */
#define GP9001_SPRITERAM_SIZE 0x800	/* Sprite     RAM size */
#define GP9001_SPRITE_FLIPX 0x1000	/* Sprite flip flags */
#define GP9001_SPRITE_FLIPY 0x2000


ADDRESS_MAP_EXTERN( gp9001vdp0_map, 16 );
ADDRESS_MAP_EXTERN( gp9001vdp1_map, 16 );
extern int gp9001_displog;


/* vdp map 0, gfx region 0 */
#define MDRV_DEVICE_ADD_VDP0 \
	MDRV_DEVICE_ADD("gp9001vdp0", gp9001vdp_, 0) \
	MDRV_DEVICE_ADDRESS_MAP(0, gp9001vdp0_map) MDRV_DEVICE_INLINE_DATA16(0, 0) \

/* vdp map 1, gfx region 2 */
#define MDRV_DEVICE_ADD_VDP1 \
	MDRV_DEVICE_ADD("gp9001vdp1", gp9001vdp_, 0) \
	MDRV_DEVICE_ADDRESS_MAP(0, gp9001vdp1_map) MDRV_DEVICE_INLINE_DATA16(0, 2) \


// access to VDP
READ16_DEVICE_HANDLER( gp9001_vdp_r );
WRITE16_DEVICE_HANDLER( gp9001_vdp_w );
READ16_DEVICE_HANDLER( gp9001_vdp_alt_r );
WRITE16_DEVICE_HANDLER( gp9001_vdp_alt_w );
// this bootleg has strange access
READ16_DEVICE_HANDLER ( pipibibi_bootleg_videoram16_r );
WRITE16_DEVICE_HANDLER( pipibibi_bootleg_videoram16_w  );
READ16_DEVICE_HANDLER ( pipibibi_bootleg_spriteram16_r );
WRITE16_DEVICE_HANDLER( pipibibi_bootleg_spriteram16_w );
WRITE16_DEVICE_HANDLER( pipibibi_bootleg_scroll_w );

void gp9001_log_vram(gp9001vdp_device* vdp, running_machine *machine);

