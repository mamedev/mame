/* helper function to join two 16-bit ROMs and form a 32-bit data stream */
void konami_rom_deinterleave_2(int mem_region);
void konami_rom_deinterleave_2_half(int mem_region);
/* helper function to join four 16-bit ROMs and form a 64-bit data stream */
void konami_rom_deinterleave_4(int mem_region);


#define MAX_K007121 2
extern UINT8 K007121_ctrlram[MAX_K007121][8];

void K007121_ctrl_w(int chip,int offset,int data);
WRITE8_HANDLER( K007121_ctrl_0_w );
WRITE8_HANDLER( K007121_ctrl_1_w );
void K007121_sprites_draw(running_machine *machine,int chip,mame_bitmap *bitmap,const rectangle *cliprect,
		const UINT8 *source,int base_color,int global_x_offset,int bank_base,
		UINT32 pri_mask);


void K007342_vh_start(int gfx_index, void (*callback)(int layer,int bank,int *code,int *color,int *flags));
READ8_HANDLER( K007342_r );
WRITE8_HANDLER( K007342_w );
READ8_HANDLER( K007342_scroll_r );
WRITE8_HANDLER( K007342_scroll_w );
void K007342_tilemap_update(void);
WRITE8_HANDLER( K007342_vreg_w );
void K007342_tilemap_set_enable(int layer, int enable);
void K007342_tilemap_draw(mame_bitmap *bitmap,const rectangle *cliprect,int num,int flags,UINT32 priority);
int K007342_is_INT_enabled(void);


void K007420_vh_start(running_machine *machine, int gfxnum, void (*callback)(int *code,int *color));
READ8_HANDLER( K007420_r );
WRITE8_HANDLER( K007420_w );
void K007420_sprites_draw(mame_bitmap *bitmap,const rectangle *cliprect);
void K007420_set_banklimit(int limit);


/*
You don't have to decode the graphics: the vh_start() routines will do that
for you, using the plane order passed.
Of course the ROM data must be in the correct order. This is a way to ensure
that the ROM test will pass.
The konami_rom_deinterleave() function above will do the reorganization for
you in most cases (but see tmnt.c for additional bit rotations or byte
permutations which may be required).
*/
#define NORMAL_PLANE_ORDER 0,1,2,3
#define REVERSE_PLANE_ORDER 3,2,1,0


/*
The callback is passed:
- layer number (0 = FIX, 1 = A, 2 = B)
- bank (range 0-3, output of the pins CAB1 and CAB2)
- code (range 00-FF, output of the pins VC3-VC10)
  NOTE: code is in the range 0000-FFFF for X-Men, which uses extra RAM
- color (range 00-FF, output of the pins COL0-COL7)
The callback must put:
- in code the resulting tile number
- in color the resulting color index
- if necessary, put flags and/or priority for the TileMap code in the tile_info
  structure (e.g. TILE_FLIPX). Note that TILE_FLIPY is handled internally by the
  chip so it must not be set by the callback.
*/
extern tilemap *K052109_tilemap[3];

void K052109_vh_start(running_machine *machine,int gfx_memory_region,int plane0,int plane1,int plane2,int plane3,
		void (*callback)(int layer,int bank,int *code,int *color,int *flags,int *priority));
/* plain 8-bit access */
READ8_HANDLER( K052109_r );
WRITE8_HANDLER( K052109_w );
READ16_HANDLER( K052109_word_r );
WRITE16_HANDLER( K052109_word_w );
READ16_HANDLER( K052109_lsb_r );
WRITE16_HANDLER( K052109_lsb_w );
void K052109_set_RMRD_line(int state);
void K052109_tilemap_update(void);
int K052109_is_IRQ_enabled(void);
void K052109_set_layer_offsets(int layer, int dx, int dy);


/*
The callback is passed:
- code (range 00-1FFF, output of the pins CA5-CA17)
- color (range 00-FF, output of the pins OC0-OC7). Note that most of the
  time COL7 seems to be "shadow", but not always (e.g. Aliens).
The callback must put:
- in code the resulting sprite number
- in color the resulting color index
- if necessary, in priority the priority of the sprite wrt tilemaps
- if necessary, alter shadow to indicate whether the sprite has shadows enabled.
  shadow is preloaded with color & 0x80 so it doesn't need to be changed unless
  the game has special treatment (Aliens)
*/
void K051960_vh_start(running_machine *machine,int gfx_memory_region,int plane0,int plane1,int plane2,int plane3,
		void (*callback)(int *code,int *color,int *priority,int *shadow));
READ8_HANDLER( K051960_r );
WRITE8_HANDLER( K051960_w );
READ16_HANDLER( K051960_word_r );
WRITE16_HANDLER( K051960_word_w );
READ8_HANDLER( K051937_r );
WRITE8_HANDLER( K051937_w );
READ16_HANDLER( K051937_word_r );
WRITE16_HANDLER( K051937_word_w );
void K051960_sprites_draw(mame_bitmap *bitmap,const rectangle *cliprect,int min_priority,int max_priority);
int K051960_is_IRQ_enabled(void);
int K051960_is_NMI_enabled(void);
void K051960_set_sprite_offsets(int dx, int dy);

/* special handling for the chips sharing address space */
READ8_HANDLER( K052109_051960_r );
WRITE8_HANDLER( K052109_051960_w );


void K053245_vh_start(running_machine *machine,int chip, int gfx_memory_region,int plane0,int plane1,int plane2,int plane3,
		void (*callback)(int *code,int *color,int *priority_mask));
READ16_HANDLER( K053245_word_r );
WRITE16_HANDLER( K053245_word_w );
READ8_HANDLER( K053245_r );
WRITE8_HANDLER( K053245_w );
WRITE8_HANDLER( K053245_1_w );
READ8_HANDLER( K053244_r );
WRITE8_HANDLER( K053244_w );
WRITE8_HANDLER( K053244_1_w );
READ16_HANDLER( K053244_lsb_r );
WRITE16_HANDLER( K053244_lsb_w );
READ16_HANDLER( K053244_word_r );
WRITE16_HANDLER( K053244_word_w );
void K053244_bankselect(int chip, int bank);	/* used by TMNT2, Asterix and Premier Soccer for ROM testing */
void K053245_sprites_draw(int chip, mame_bitmap *bitmap,const rectangle *cliprect);
void K053245_sprites_draw_lethal(running_machine *machine, int chip, mame_bitmap *bitmap,const rectangle *cliprect); /* for lethal enforcers */
void K053245_clear_buffer(int chip);
void K053245_set_SpriteOffset(int chip,int offsx, int offsy);

#define K055673_LAYOUT_GX  0
#define K055673_LAYOUT_RNG 1
#define K055673_LAYOUT_LE2 2
#define K055673_LAYOUT_GX6 3

void K055673_vh_start(running_machine *machine, int gfx_memory_region, int alt_layout, int dx, int dy,
		void (*callback)(int *code,int *color,int *priority));
READ16_HANDLER( K055673_rom_word_r );
READ16_HANDLER( K055673_GX6bpp_rom_word_r );

/*
Callback procedures for non-standard shadows:

1) translate shadow code to the correct 2-bit form (0=off, 1-3=style)
2) shift shadow code left by K053247_SHDSHIFT and add the K053247_CUSTOMSHADOW flag
3) combine the result with sprite color
*/
#define K053247_CUSTOMSHADOW	0x20000000
#define K053247_SHDSHIFT		20

void K053247_vh_start(running_machine *machine, int gfx_memory_region,int dx,int dy,int plane0,int plane1,int plane2,int plane3,
		void (*callback)(int *code,int *color,int *priority_mask));
READ8_HANDLER( K053247_r );
WRITE8_HANDLER( K053247_w );
READ16_HANDLER( K053247_word_r );
WRITE16_HANDLER( K053247_word_w );
READ32_HANDLER( K053247_long_r );
WRITE32_HANDLER( K053247_long_w );
WRITE16_HANDLER( K053247_reg_word_w ); // "OBJSET2" registers
WRITE32_HANDLER( K053247_reg_long_w );
void K053247_sprites_draw(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect);
int K053247_read_register(int regnum);
void K053247_set_SpriteOffset(int offsx, int offsy);
void K053247_wraparound_enable(int status);
void K05324x_set_z_rejection(int zcode); // common to K053245/6/7
void K053247_export_config(UINT16 **ram, gfx_element **gfx, void (**callback)(int *, int *, int *), int *dx, int *dy);

READ8_HANDLER( K053246_r );
WRITE8_HANDLER( K053246_w );
READ16_HANDLER( K053246_word_r );
WRITE16_HANDLER( K053246_word_w );
READ32_HANDLER( K053246_long_r );
WRITE32_HANDLER( K053246_long_w );
void K053246_set_OBJCHA_line(int state);
int K053246_is_IRQ_enabled(void);
int K053246_read_register(int regnum);


/*
The callback is passed:
- code (range 00-FF, contents of the first tilemap RAM byte)
- color (range 00-FF, contents of the first tilemap RAM byte). Note that bit 6
  seems to be hardcoded as flip X.
The callback must put:
- in code the resulting tile number
- in color the resulting color index
- if necessary, put flags for the TileMap code in the tile_info
  structure (e.g. TILE_FLIPX)
*/
void K051316_vh_start_0(running_machine *machine,int gfx_memory_region,int bpp,
		int tilemap_type,int transparent_pen,
		void (*callback)(int *code,int *color,int *flags));
void K051316_vh_start_1(running_machine *machine,int gfx_memory_region,int bpp,
		int tilemap_type,int transparent_pen,
		void (*callback)(int *code,int *color,int *flags));
void K051316_vh_start_2(running_machine *machine,int gfx_memory_region,int bpp,
		int tilemap_type,int transparent_pen,
		void (*callback)(int *code,int *color,int *flags));
READ8_HANDLER( K051316_0_r );
READ8_HANDLER( K051316_1_r );
READ8_HANDLER( K051316_2_r );
WRITE8_HANDLER( K051316_0_w );
WRITE8_HANDLER( K051316_1_w );
WRITE8_HANDLER( K051316_2_w );
READ8_HANDLER( K051316_rom_0_r );
READ8_HANDLER( K051316_rom_1_r );
READ8_HANDLER( K051316_rom_2_r );
WRITE8_HANDLER( K051316_ctrl_0_w );
WRITE8_HANDLER( K051316_ctrl_1_w );
WRITE8_HANDLER( K051316_ctrl_2_w );
void K051316_zoom_draw_0(mame_bitmap *bitmap,const rectangle *cliprect,int flags,UINT32 priority);
void K051316_zoom_draw_1(mame_bitmap *bitmap,const rectangle *cliprect,int flags,UINT32 priority);
void K051316_zoom_draw_2(mame_bitmap *bitmap,const rectangle *cliprect,int flags,UINT32 priority);
void K051316_wraparound_enable(int chip, int status);
void K051316_set_offset(int chip, int xoffs, int yoffs);


extern UINT16 *K053936_0_ctrl,*K053936_0_linectrl;
extern UINT16 *K053936_1_ctrl,*K053936_1_linectrl;
void K053936_0_zoom_draw(mame_bitmap *bitmap,const rectangle *cliprect,tilemap *tmap,int flags,UINT32 priority);
void K053936_1_zoom_draw(mame_bitmap *bitmap,const rectangle *cliprect,tilemap *tmap,int flags,UINT32 priority);
void K053936_wraparound_enable(int chip, int status);
void K053936_set_offset(int chip, int xoffs, int yoffs);


/*
  Note: K053251_w() automatically does a tilemap_mark_all_tiles_dirty(ALL_TILEMAPS)
  when some palette index changes. If ALL_TILEMAPS is too expensive, use
  K053251_set_tilemaps() to indicate which tilemap is associated with each index.
 */
WRITE8_HANDLER( K053251_w );
WRITE16_HANDLER( K053251_lsb_w );
WRITE16_HANDLER( K053251_msb_w );
enum { K053251_CI0=0,K053251_CI1,K053251_CI2,K053251_CI3,K053251_CI4 };
int K053251_get_priority(int ci);
int K053251_get_palette_index(int ci);
void K053251_set_tilemaps(tilemap *ci0,tilemap *ci1,tilemap *ci2,tilemap *ci3,tilemap *ci4);
void K053251_vh_start(void);


WRITE8_HANDLER( K054000_w );
READ8_HANDLER( K054000_r );
WRITE16_HANDLER( K054000_lsb_w );
READ16_HANDLER( K054000_lsb_r );

WRITE8_HANDLER( K051733_w );
READ8_HANDLER( K051733_r );

void K056832_SetExtLinescroll(void);	/* Lethal Enforcers */

void K056832_vh_start(running_machine *machine, int gfx_memory_region, int bpp, int big,
			int (*scrolld)[4][2],
			void (*callback)(int layer, int *code, int *color, int *flags),
			int djmain_hack);
READ16_HANDLER( K056832_ram_word_r );
WRITE16_HANDLER( K056832_ram_word_w );
READ16_HANDLER( K056832_ram_half_word_r );
WRITE16_HANDLER( K056832_ram_half_word_w );
READ16_HANDLER( K056832_5bpp_rom_word_r );
READ32_HANDLER( K056832_5bpp_rom_long_r );
READ32_HANDLER( K056832_6bpp_rom_long_r );
READ16_HANDLER( K056832_rom_word_r );
READ16_HANDLER( K056832_mw_rom_word_r );
READ16_HANDLER( K056832_bishi_rom_word_r );
READ16_HANDLER( K056832_old_rom_word_r );
READ16_HANDLER( K056832_rom_word_8000_r );
WRITE16_HANDLER( K056832_word_w ); // "VRAM" registers
WRITE16_HANDLER( K056832_b_word_w );
READ8_HANDLER( K056832_ram_code_lo_r );
READ8_HANDLER( K056832_ram_code_hi_r );
READ8_HANDLER( K056832_ram_attr_lo_r );
READ8_HANDLER( K056832_ram_attr_hi_r );
WRITE8_HANDLER( K056832_ram_code_lo_w );
WRITE8_HANDLER( K056832_ram_code_hi_w );
WRITE8_HANDLER( K056832_ram_attr_lo_w );
WRITE8_HANDLER( K056832_ram_attr_hi_w );
WRITE8_HANDLER( K056832_w );
WRITE8_HANDLER( K056832_b_w );
void K056832_mark_plane_dirty(int num);
void K056832_MarkAllTilemapsDirty(void);
void K056832_tilemap_draw(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int num, int flags, UINT32 priority);
void K056832_tilemap_draw_dj(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int layer, int flags, UINT32 priority);
void K056832_set_LayerAssociation(int status);
int  K056832_get_LayerAssociation(void);
void K056832_set_LayerOffset(int layer, int offsx, int offsy);
void K056832_set_LSRAMPage(int logical_page, int physical_page, int physical_offset);
void K056832_set_UpdateMode(int mode);
void K056832_linemap_enable(int enable);
int  K056832_is_IRQ_enabled(int irqline);
void K056832_read_AVAC(int *mode, int *data);
int  K056832_read_register(int regnum);
int K056832_get_current_rambank(void);
int K056832_get_lookup(int bits);	/* Asterix */
void K056832_set_tile_bank(int bank);	/* Asterix */

READ32_HANDLER( K056832_ram_long_r );
READ32_HANDLER( K056832_rom_long_r );
WRITE32_HANDLER( K056832_ram_long_w );
WRITE32_HANDLER( K056832_long_w );
WRITE32_HANDLER( K056832_b_long_w );

/* bit depths for the 56832 */
#define K056832_BPP_4	0
#define K056832_BPP_5	1
#define K056832_BPP_6	2
#define K056832_BPP_8	3
#define K056832_BPP_4dj	4
#define K056832_BPP_8LE	5

void K055555_vh_start(void); // "PCU2"
void K055555_write_reg(UINT8 regnum, UINT8 regdat);
WRITE16_HANDLER( K055555_word_w );
WRITE32_HANDLER( K055555_long_w );
int K055555_read_register(int regnum);
int K055555_get_palette_index(int idx);

/* K055555 registers */
/* priority inputs */
#define K55_PALBASE_BG		0	// background palette
#define K55_CONTROL			1	// control register
#define K55_COLSEL_0		2	// layer A, B color depth
#define K55_COLSEL_1		3	// layer C, D color depth
#define K55_COLSEL_2		4	// object, S1 color depth
#define K55_COLSEL_3		5	// S2, S3 color depth

#define K55_PRIINP_0		7	// layer A pri 0
#define K55_PRIINP_1		8	// layer A pri 1
#define K55_PRIINP_2		9	// layer A "COLPRI"
#define K55_PRIINP_3		10	// layer B pri 0
#define K55_PRIINP_4		11	// layer B pri 1
#define K55_PRIINP_5		12	// layer B "COLPRI"
#define K55_PRIINP_6		13	// layer C pri
#define K55_PRIINP_7		14	// layer D pri
#define K55_PRIINP_8		15	// OBJ pri
#define K55_PRIINP_9		16	// sub 1 (GP:PSAC) pri
#define K55_PRIINP_10		17	// sub 2 (GX:PSAC) pri
#define K55_PRIINP_11		18	// sub 3 pri

#define K55_OINPRI_ON 		19	// object priority bits selector

#define K55_PALBASE_A 		23	// layer A palette
#define K55_PALBASE_B 		24	// layer B palette
#define K55_PALBASE_C 		25	// layer C palette
#define K55_PALBASE_D 		26	// layer D palette
#define K55_PALBASE_OBJ		27	// OBJ palette
#define K55_PALBASE_SUB1	28	// SUB1 palette
#define K55_PALBASE_SUB2	29	// SUB2 palette
#define K55_PALBASE_SUB3	30	// SUB3 palette

#define K55_BLEND_ENABLES	33	// blend enables for tilemaps
#define K55_VINMIX_ON		34	// additional blend enables for tilemaps
#define K55_OSBLEND_ENABLES	35	// obj/sub blend enables
#define K55_OSBLEND_ON		36	// not sure, related to obj/sub blend

#define K55_SHAD1_PRI		37	// shadow/highlight 1 priority
#define K55_SHAD2_PRI		38	// shadow/highlight 2 priority
#define K55_SHAD3_PRI		39	// shadow/highlight 3 priority
#define K55_SHD_ON			40	// shadow/highlight
#define K55_SHD_PRI_SEL		41	// shadow/highlight

#define K55_VBRI			42	// VRAM layer brightness enable
#define K55_OSBRI			43	// obj/sub brightness enable, part 1
#define K55_OSBRI_ON		44	// obj/sub brightness enable, part 2
#define K55_INPUT_ENABLES	45	// input enables

/* bit masks for the control register */
#define K55_CTL_GRADDIR		0x01	// 0=vertical, 1=horizontal
#define K55_CTL_GRADENABLE	0x02	// 0=BG is base color only, 1=gradient
#define K55_CTL_FLIPPRI		0x04	// 0=standard Konami priority, 1=reverse
#define K55_CTL_SDSEL		0x08	// 0=normal shadow timing, 1=(not used by GX)

/* bit masks for the input enables */
#define K55_INP_VRAM_A		0x01
#define K55_INP_VRAM_B		0x02
#define K55_INP_VRAM_C		0x04
#define K55_INP_VRAM_D		0x08
#define K55_INP_OBJ			0x10
#define K55_INP_SUB1		0x20
#define K55_INP_SUB2		0x40
#define K55_INP_SUB3		0x80

/* K054338 mixer/alpha blender */
void K054338_vh_start(void);
WRITE16_HANDLER( K054338_word_w ); // "CLCT" registers
WRITE32_HANDLER( K054338_long_w );
int K054338_read_register(int reg);
void K054338_update_all_shadows(running_machine *machine);			// called at the beginning of VIDEO_UPDATE()
void K054338_fill_solid_bg(mame_bitmap *bitmap);				// solid backcolor fill
void K054338_fill_backcolor(running_machine *machine, mame_bitmap *bitmap, int mode);	// unified fill, 0=solid, 1=gradient
int  K054338_set_alpha_level(int pblend);							// blend style 0-2
void K054338_invert_alpha(int invert);								// 0=0x00(invis)-0x1f(solid), 1=0x1f(invis)-0x00(solod)
void K054338_export_config(int **shdRGB);

#define K338_REG_BGC_R		0
#define K338_REG_BGC_GB		1
#define K338_REG_SHAD1R		2
#define K338_REG_BRI3		11
#define K338_REG_PBLEND		13
#define K338_REG_CONTROL	15

#define K338_CTL_KILL		0x01	/* 0 = no video output, 1 = enable */
#define K338_CTL_MIXPRI		0x02
#define K338_CTL_SHDPRI		0x04
#define K338_CTL_BRTPRI		0x08
#define K338_CTL_WAILSL		0x10
#define K338_CTL_CLIPSL		0x20

void K053250_vh_start(int chips, int *region);
WRITE16_HANDLER( K053250_0_w );
READ16_HANDLER( K053250_0_r );
WRITE16_HANDLER( K053250_0_ram_w );
READ16_HANDLER( K053250_0_ram_r );
READ16_HANDLER( K053250_0_rom_r );
WRITE16_HANDLER( K053250_1_w );
READ16_HANDLER( K053250_1_r );
WRITE16_HANDLER( K053250_1_ram_w );
READ16_HANDLER( K053250_1_ram_r );
READ16_HANDLER( K053250_1_rom_r );

// K053250_draw() control flags
#define K053250_WRAP500		0x01
#define K053250_OVERDRIVE	0x02

void K053250_draw(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int chip, int colorbase, int flags, int pri);
void K053250_set_LayerOffset(int chip, int offsx, int offsy);
void K053250_unpack_pixels(int region);
void K053250_dma(int chip, int limiter);


// K053252 CRT and interrupt control unit
READ16_HANDLER( K053252_word_r );	// CCU registers
WRITE16_HANDLER( K053252_word_w );
WRITE32_HANDLER( K053252_long_w );


// debug handlers
READ16_HANDLER( K056832_word_r );		// VACSET
READ16_HANDLER( K056832_b_word_r );		// VSCCS  (board dependent)
READ16_HANDLER( K053246_reg_word_r );	// OBJSET1
READ16_HANDLER( K053247_reg_word_r );	// OBJSET2
READ16_HANDLER( K053251_lsb_r );		// PCU1
READ16_HANDLER( K053251_msb_r );		// PCU1
READ16_HANDLER( K055555_word_r );		// PCU2
READ16_HANDLER( K054338_word_r );		// CLTC

READ32_HANDLER( K056832_long_r );		// VACSET
READ32_HANDLER( K053247_reg_long_r );	// OBJSET2
READ32_HANDLER( K055555_long_r );		// PCU2

READ16_HANDLER( K053244_reg_word_r );	// OBJSET0
