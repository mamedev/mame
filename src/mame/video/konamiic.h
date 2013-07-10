

/*
  Note: K053251_w() automatically does a ALL_TILEMAPS->mark_all_dirty()
  when some palette index changes. If ALL_TILEMAPS is too expensive, use
  K053251_set_tilemaps() to indicate which tilemap is associated with each index.
 */
DECLARE_WRITE8_HANDLER( K053251_w );
DECLARE_WRITE16_HANDLER( K053251_lsb_w );
DECLARE_WRITE16_HANDLER( K053251_msb_w );
enum { OLD_K053251_CI0=0,OLD_K053251_CI1,OLD_K053251_CI2,OLD_K053251_CI3,OLD_K053251_CI4 };
int K053251_get_priority(int ci);
int K053251_get_palette_index(int ci);
void K053251_set_tilemaps(tilemap_t *ci0,tilemap_t *ci1,tilemap_t *ci2,tilemap_t *ci3,tilemap_t *ci4);
void K053251_vh_start(running_machine &machine);


DECLARE_WRITE16_HANDLER( K054000_lsb_w );
DECLARE_READ16_HANDLER( K054000_lsb_r );



void K055555_vh_start(running_machine &machine); // "PCU2"
void K055555_write_reg(UINT8 regnum, UINT8 regdat);
DECLARE_WRITE16_HANDLER( K055555_word_w );
DECLARE_WRITE32_HANDLER( K055555_long_w );
int K055555_read_register(int regnum);
int K055555_get_palette_index(int idx);

/* K055555 registers */
/* priority inputs */
#define K55_PALBASE_BG      0   // background palette
#define K55_CONTROL         1   // control register
#define K55_COLSEL_0        2   // layer A, B color depth
#define K55_COLSEL_1        3   // layer C, D color depth
#define K55_COLSEL_2        4   // object, S1 color depth
#define K55_COLSEL_3        5   // S2, S3 color depth

#define K55_PRIINP_0        7   // layer A pri 0
#define K55_PRIINP_1        8   // layer A pri 1
#define K55_PRIINP_2        9   // layer A "COLPRI"
#define K55_PRIINP_3        10  // layer B pri 0
#define K55_PRIINP_4        11  // layer B pri 1
#define K55_PRIINP_5        12  // layer B "COLPRI"
#define K55_PRIINP_6        13  // layer C pri
#define K55_PRIINP_7        14  // layer D pri
#define K55_PRIINP_8        15  // OBJ pri
#define K55_PRIINP_9        16  // sub 1 (GP:PSAC) pri
#define K55_PRIINP_10       17  // sub 2 (GX:PSAC) pri
#define K55_PRIINP_11       18  // sub 3 pri

#define K55_OINPRI_ON       19  // object priority bits selector

#define K55_PALBASE_A       23  // layer A palette
#define K55_PALBASE_B       24  // layer B palette
#define K55_PALBASE_C       25  // layer C palette
#define K55_PALBASE_D       26  // layer D palette
#define K55_PALBASE_OBJ     27  // OBJ palette
#define K55_PALBASE_SUB1    28  // SUB1 palette
#define K55_PALBASE_SUB2    29  // SUB2 palette
#define K55_PALBASE_SUB3    30  // SUB3 palette

#define K55_BLEND_ENABLES   33  // blend enables for tilemaps
#define K55_VINMIX_ON       34  // additional blend enables for tilemaps
#define K55_OSBLEND_ENABLES 35  // obj/sub blend enables
#define K55_OSBLEND_ON      36  // not sure, related to obj/sub blend

#define K55_SHAD1_PRI       37  // shadow/highlight 1 priority
#define K55_SHAD2_PRI       38  // shadow/highlight 2 priority
#define K55_SHAD3_PRI       39  // shadow/highlight 3 priority
#define K55_SHD_ON          40  // shadow/highlight
#define K55_SHD_PRI_SEL     41  // shadow/highlight

#define K55_VBRI            42  // VRAM layer brightness enable
#define K55_OSBRI           43  // obj/sub brightness enable, part 1
#define K55_OSBRI_ON        44  // obj/sub brightness enable, part 2
#define K55_INPUT_ENABLES   45  // input enables

/* bit masks for the control register */
#define K55_CTL_GRADDIR     0x01    // 0=vertical, 1=horizontal
#define K55_CTL_GRADENABLE  0x02    // 0=BG is base color only, 1=gradient
#define K55_CTL_FLIPPRI     0x04    // 0=standard Konami priority, 1=reverse
#define K55_CTL_SDSEL       0x08    // 0=normal shadow timing, 1=(not used by GX)

/* bit masks for the input enables */
#define K55_INP_VRAM_A      0x01
#define K55_INP_VRAM_B      0x02
#define K55_INP_VRAM_C      0x04
#define K55_INP_VRAM_D      0x08
#define K55_INP_OBJ         0x10
#define K55_INP_SUB1        0x20
#define K55_INP_SUB2        0x40
#define K55_INP_SUB3        0x80

/* K054338 mixer/alpha blender */
void K054338_vh_start(running_machine &machine);
DECLARE_WRITE16_HANDLER( K054338_word_w ); // "CLCT" registers
DECLARE_WRITE32_HANDLER( K054338_long_w );
int K054338_read_register(int reg);
void K054338_update_all_shadows(running_machine &machine, int rushingheroes_hack);          // called at the beginning of SCREEN_UPDATE()
void K054338_fill_solid_bg(bitmap_ind16 &bitmap);               // solid backcolor fill
void K054338_fill_backcolor(running_machine &machine, bitmap_rgb32 &bitmap, int mode);  // unified fill, 0=solid, 1=gradient
int  K054338_set_alpha_level(int pblend);                           // blend style 0-2
void K054338_invert_alpha(int invert);                              // 0=0x00(invis)-0x1f(solid), 1=0x1f(invis)-0x00(solod)
void K054338_export_config(int **shdRGB);

#define K338_REG_BGC_R      0
#define K338_REG_BGC_GB     1
#define K338_REG_SHAD1R     2
#define K338_REG_BRI3       11
#define K338_REG_PBLEND     13
#define K338_REG_CONTROL    15

#define K338_CTL_KILL       0x01    /* 0 = no video output, 1 = enable */
#define K338_CTL_MIXPRI     0x02
#define K338_CTL_SHDPRI     0x04
#define K338_CTL_BRTPRI     0x08
#define K338_CTL_WAILSL     0x10
#define K338_CTL_CLIPSL     0x20

// K053252 CRT and interrupt control unit
DECLARE_WRITE16_HANDLER( K053252_word_w );
