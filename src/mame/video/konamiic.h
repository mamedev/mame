#define K055673_LAYOUT_GX  0
#define K055673_LAYOUT_RNG 1
#define K055673_LAYOUT_LE2 2
#define K055673_LAYOUT_GX6 3

void K055673_vh_start(running_machine &machine, const char *gfx_memory_region, int alt_layout, int dx, int dy,
		void (*callback)(running_machine &machine, int *code,int *color,int *priority));
DECLARE_READ16_HANDLER( K055673_rom_word_r );
DECLARE_READ16_HANDLER( K055673_GX6bpp_rom_word_r );

/*
Callback procedures for non-standard shadows:

1) translate shadow code to the correct 2-bit form (0=off, 1-3=style)
2) shift shadow code left by K053247_SHDSHIFT and add the K053247_CUSTOMSHADOW flag
3) combine the result with sprite color
*/
#define K053247_CUSTOMSHADOW	0x20000000
#define K053247_SHDSHIFT		20

DECLARE_READ16_HANDLER( K053247_word_r );
DECLARE_WRITE16_HANDLER( K053247_word_w );
DECLARE_READ32_HANDLER( K053247_long_r );
DECLARE_WRITE32_HANDLER( K053247_long_w );
DECLARE_WRITE16_HANDLER( K053247_reg_word_w ); // "OBJSET2" registers
DECLARE_WRITE32_HANDLER( K053247_reg_long_w );

int K053247_read_register(int regnum);
void K053247_set_SpriteOffset(int offsx, int offsy);
void K053247_export_config(UINT16 **ram, gfx_element **gfx, void (**callback)(running_machine &, int *, int *, int *), int *dx, int *dy);

DECLARE_WRITE16_HANDLER( K053246_word_w );
DECLARE_WRITE32_HANDLER( K053246_long_w );
void K053246_set_OBJCHA_line(int state);
int K053246_is_IRQ_enabled(void);
int K053246_read_register(int regnum);

extern UINT16 *K053936_0_ctrl,*K053936_0_linectrl;
//extern UINT16 *K053936_1_ctrl,*K053936_1_linectrl;
void K053936_0_zoom_draw(bitmap_ind16 &bitmap,const rectangle &cliprect,tilemap_t *tmap,int flags,UINT32 priority, int glfgreat_hack);
void K053936_wraparound_enable(int chip, int status);
void K053936_set_offset(int chip, int xoffs, int yoffs);


/*
  Note: K053251_w() automatically does a ALL_TILEMAPS->mark_all_dirty()
  when some palette index changes. If ALL_TILEMAPS is too expensive, use
  K053251_set_tilemaps() to indicate which tilemap is associated with each index.
 */
DECLARE_WRITE8_HANDLER( K053251_w );
DECLARE_WRITE16_HANDLER( K053251_lsb_w );
DECLARE_WRITE16_HANDLER( K053251_msb_w );
enum { K053251_CI0=0,K053251_CI1,K053251_CI2,K053251_CI3,K053251_CI4 };
int K053251_get_priority(int ci);
int K053251_get_palette_index(int ci);
void K053251_set_tilemaps(tilemap_t *ci0,tilemap_t *ci1,tilemap_t *ci2,tilemap_t *ci3,tilemap_t *ci4);
void K053251_vh_start(running_machine &machine);


DECLARE_WRITE16_HANDLER( K054000_lsb_w );
DECLARE_READ16_HANDLER( K054000_lsb_r );


#define K056382_DRAW_FLAG_FORCE_XYSCROLL		0x00800000

void K056832_vh_start(running_machine &machine, const char *gfx_memory_region, int bpp, int big,
			int (*scrolld)[4][2],
			void (*callback)(running_machine &machine, int layer, int *code, int *color, int *flags),
			int djmain_hack);
DECLARE_READ16_HANDLER( K056832_ram_word_r );
DECLARE_WRITE16_HANDLER( K056832_ram_word_w );
DECLARE_READ32_HANDLER( K056832_5bpp_rom_long_r );
DECLARE_READ32_HANDLER( K056832_6bpp_rom_long_r );
DECLARE_READ16_HANDLER( K056832_mw_rom_word_r );
DECLARE_WRITE16_HANDLER( K056832_word_w ); // "VRAM" registers
DECLARE_WRITE16_HANDLER( K056832_b_word_w );
void K056832_mark_plane_dirty(int num);
void K056832_MarkAllTilemapsDirty(void);
void K056832_tilemap_draw(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect, int num, UINT32 flags, UINT32 priority);
int  K056832_get_LayerAssociation(void);
void K056832_set_LayerOffset(int layer, int offsx, int offsy);
void K056832_set_UpdateMode(int mode);

DECLARE_READ32_HANDLER( K056832_ram_long_r );
DECLARE_WRITE32_HANDLER( K056832_ram_long_w );
DECLARE_WRITE32_HANDLER( K056832_long_w );
DECLARE_WRITE32_HANDLER( K056832_b_long_w );

/* bit depths for the 56832 */
#define K056832_BPP_4	0
#define K056832_BPP_5	1
#define K056832_BPP_6	2
#define K056832_BPP_8	3
#define K056832_BPP_4dj	4
#define K056832_BPP_8LE	5
#define K056832_BPP_8TASMAN 6

void K055555_vh_start(running_machine &machine); // "PCU2"
void K055555_write_reg(UINT8 regnum, UINT8 regdat);
DECLARE_WRITE16_HANDLER( K055555_word_w );
DECLARE_WRITE32_HANDLER( K055555_long_w );
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

#define K55_OINPRI_ON		19	// object priority bits selector

#define K55_PALBASE_A		23	// layer A palette
#define K55_PALBASE_B		24	// layer B palette
#define K55_PALBASE_C		25	// layer C palette
#define K55_PALBASE_D		26	// layer D palette
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
void K054338_vh_start(running_machine &machine);
DECLARE_WRITE16_HANDLER( K054338_word_w ); // "CLCT" registers
DECLARE_WRITE32_HANDLER( K054338_long_w );
int K054338_read_register(int reg);
void K054338_update_all_shadows(running_machine &machine, int rushingheroes_hack);			// called at the beginning of SCREEN_UPDATE()
void K054338_fill_solid_bg(bitmap_ind16 &bitmap);				// solid backcolor fill
void K054338_fill_backcolor(running_machine &machine, bitmap_rgb32 &bitmap, int mode);	// unified fill, 0=solid, 1=gradient
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

// K053252 CRT and interrupt control unit
DECLARE_WRITE16_HANDLER( K053252_word_w );

