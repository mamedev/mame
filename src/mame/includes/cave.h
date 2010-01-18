/***************************************************************************

    Cave hardware

***************************************************************************/

struct sprite_cave
{
	int priority, flags;

	const UINT8 *pen_data;	/* points to top left corner of tile data */
	int line_offset;

	pen_t base_pen;
	int tile_width, tile_height;
	int total_width, total_height;	/* in screen coordinates */
	int x, y, xcount0, ycount0;
	int zoomx_re, zoomy_re;
};

#define MAX_PRIORITY        4
#define MAX_SPRITE_NUM      0x400

typedef struct _cave_state cave_state;
struct _cave_state
{
	/* memory pointers */
	UINT16 *     videoregs;
	UINT16 *     vram_0;
	UINT16 *     vram_1;
	UINT16 *     vram_2;
	UINT16 *     vram_3;
	UINT16 *     vctrl_0;
	UINT16 *     vctrl_1;
	UINT16 *     vctrl_2;
	UINT16 *     vctrl_3;
	UINT16 *     spriteram;
	UINT16 *     spriteram_2;
	UINT16 *     paletteram;
	size_t       spriteram_size;
	size_t       paletteram_size;

	/* video-related */
	struct sprite_cave *sprite;
	struct sprite_cave *sprite_table[MAX_PRIORITY][MAX_SPRITE_NUM + 1];

	struct
	{
		int    clip_left, clip_right, clip_top, clip_bottom;
		UINT8  *baseaddr;
		int    line_offset;
		UINT8  *baseaddr_zbuf;
		int    line_offset_zbuf;
	} blit;


	void (*get_sprite_info)(running_machine *machine);
	void (*sprite_draw)(running_machine *machine, int priority);

	tilemap_t    *tilemap_0, *tilemap_1, *tilemap_2, *tilemap_3;
	int          tiledim_0, old_tiledim_0;
	int          tiledim_1, old_tiledim_1;
	int          tiledim_2, old_tiledim_2;
	int          tiledim_3, old_tiledim_3;

	bitmap_t     *sprite_zbuf;
	UINT16       sprite_zbuf_baseval;

	int          num_sprites;

	int          spriteram_bank;
	int          spriteram_bank_delay;

	UINT16       *palette_map;

	int          layers_offs_x, layers_offs_y;
	int          row_effect_offs_n;
	int          row_effect_offs_f;
	int          background_color;

	int          spritetype[2];
	int          kludge;


	/* misc */
	int          time_vblank_irq;
	UINT8        irq_level;
	UINT8        vblank_irq;
	UINT8        sound_irq;
	UINT8        unknown_irq;
	UINT8        agallet_vblank_irq;

	/* sound related */
	int          soundbuf_len;
	UINT8        soundbuf_data[32];
	//UINT8        sound_flag1, sound_flag2;

	/* eeprom-related */
	int          region_byte;

	/* game specific */
	// sailormn
	int          sailormn_tilebank;
	UINT8        *mirror_ram;
	// korokoro
	UINT16       leds[2];
	int          hopper;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};

/*----------- defined in video/cave.c -----------*/

WRITE16_HANDLER( cave_vram_0_w );
WRITE16_HANDLER( cave_vram_1_w );
WRITE16_HANDLER( cave_vram_2_w );
WRITE16_HANDLER( cave_vram_3_w );

WRITE16_HANDLER( cave_vram_0_8x8_w );
WRITE16_HANDLER( cave_vram_1_8x8_w );
WRITE16_HANDLER( cave_vram_2_8x8_w );
WRITE16_HANDLER( cave_vram_3_8x8_w );

PALETTE_INIT( cave );
PALETTE_INIT( ddonpach );
PALETTE_INIT( dfeveron );
PALETTE_INIT( mazinger );
PALETTE_INIT( sailormn );
PALETTE_INIT( pwrinst2 );
PALETTE_INIT( korokoro );

VIDEO_START( cave_1_layer );
VIDEO_START( cave_2_layers );
VIDEO_START( cave_3_layers );
VIDEO_START( cave_4_layers );

VIDEO_START( sailormn_3_layers );

VIDEO_UPDATE( cave );

void cave_get_sprite_info(running_machine *machine);
void sailormn_tilebank_w(running_machine *machine, int bank);
