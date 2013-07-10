

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
