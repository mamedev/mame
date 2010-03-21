/*************************************************************************

    Metro Games

*************************************************************************/

class metro_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, metro_state(machine)); }

	metro_state(running_machine &machine) { }

	/* memory pointers */
	UINT16 *    vram_0;
	UINT16 *    vram_1;
	UINT16 *    vram_2;
	UINT16 *    spriteram;
	UINT16 *    tiletable;
	UINT16 *    tiletable_old;
	UINT16 *    blitter_regs;
	UINT16 *    scroll;
	UINT16 *    window;
	UINT16 *    irq_enable;
	UINT16 *    irq_levels;
	UINT16 *    irq_vectors;
	UINT16 *    rombank;
	UINT16 *    videoregs;
	UINT16 *    screenctrl;
	UINT16 *    input_sel;
	UINT16 *    k053936_ram;

	size_t      spriteram_size;
	size_t      tiletable_size;

	int         flip_screen;

	/* video-related */
	tilemap_t   *k053936_tilemap;
	int         bg_tilemap_enable[3];
	int         bg_tilemap_enable16[3];
	int         bg_tilemap_scrolldx[3];

	int         support_8bpp, support_16x16;
	int         has_zoom;
	int         sprite_xoffs, sprite_yoffs;

	/* blitter */
	int         blitter_bit;

	/* irq_related */
	int         irq_line;
	UINT8       requested_int[8];
	emu_timer   *mouja_irq_timer;

	/* sound related */
	UINT16      soundstatus;
	int         porta, portb, busy_sndcpu;

	/* misc */
	int         gakusai_oki_bank_lo, gakusai_oki_bank_hi;

	/* used by vmetal.c */
	UINT16 *vmetal_texttileram;
	UINT16 *vmetal_mid1tileram;
	UINT16 *vmetal_mid2tileram;
	UINT16 *vmetal_tlookup;
	UINT16 *vmetal_videoregs;

	tilemap_t *vmetal_texttilemap;
	tilemap_t *vmetal_mid1tilemap;
	tilemap_t *vmetal_mid2tilemap;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *oki;
	running_device *ymsnd;
	running_device *k053936;
};


/*----------- defined in video/metro.c -----------*/

WRITE16_HANDLER( metro_window_w );
WRITE16_HANDLER( metro_vram_0_w );
WRITE16_HANDLER( metro_vram_1_w );
WRITE16_HANDLER( metro_vram_2_w );
WRITE16_HANDLER( metro_k053936_w );

VIDEO_START( metro_14100 );
VIDEO_START( metro_14220 );
VIDEO_START( metro_14300 );
VIDEO_START( blzntrnd );
VIDEO_START( gstrik2 );

VIDEO_UPDATE( metro );

void metro_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);
