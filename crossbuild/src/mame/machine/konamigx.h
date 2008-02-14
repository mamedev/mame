#ifndef __MACH_KONAMIGX_H
#define __MACH_KONAMIGX_H

void tms57002_init(void);

READ8_HANDLER( tms57002_data_r );
READ8_HANDLER( tms57002_status_r );
WRITE8_HANDLER( tms57002_control_w );
WRITE8_HANDLER( tms57002_data_w );

READ16_HANDLER( tms57002_data_word_r );
READ16_HANDLER( tms57002_status_word_r );
WRITE16_HANDLER( tms57002_control_word_w );
WRITE16_HANDLER( tms57002_data_word_w );



// 2nd-Tier GX/MW Hardware Functions
void K053247GP_set_SpriteOffset(int offsx, int offsy);

void K053936GP_set_offset(int chip, int xoffs, int yoffs);
void K053936GP_clip_enable(int chip, int status);
void K053936GP_set_cliprect(int chip, int minx, int maxx, int miny, int maxy);
void K053936GP_0_zoom_draw(mame_bitmap *bitmap, const rectangle *cliprect, tilemap *tmap, int tilebpp, int blend);
void K053936GP_1_zoom_draw(mame_bitmap *bitmap, const rectangle *cliprect, tilemap *tmap, int tilebpp, int blend);



// 1st-Tier GX/MW Variables and Functions
extern UINT8  konamigx_wrport1_0, konamigx_wrport1_1;
extern UINT16 konamigx_wrport2;


// Tile and Sprite Mixcode Decoders
int K055555GX_decode_vmixcolor(int layer, int *color);
int K055555GX_decode_osmixcolor(int layer, int *color);


// Sprite Callbacks

/* callbacks should return color codes in this format:
    fedcba9876543210fedcba9876543210
    ----------------xxxxxxxxxxxxxxxx (bit 00-15: color)
    --------------xx---------------- (bit 16-17: blend code)
    ------------xx------------------ (bit 18-19: brightness code)
    -x------------------------------ (bit 30   : skip shadow)
    x------------------------------- (bit 31   : full shadow)
*/
#define K055555_COLORMASK	0x0000ffff
#define K055555_MIXSHIFT 	16
#define K055555_BRTSHIFT 	18
#define K055555_SKIPSHADOW	0x40000000
#define K055555_FULLSHADOW	0x80000000

void konamigx_type2_sprite_callback(int *code, int *color, int *priority);

void konamigx_dragoonj_sprite_callback(int *code, int *color, int *priority);
void konamigx_salmndr2_sprite_callback(int *code, int *color, int *priority);
void konamigx_le2_sprite_callback(int *code, int *color, int *priority);


// Centralized Sprites and Layer Blitter

/* Mixer Flags
    fedcba9876543210fedcba9876543210
    --------------------FFEEDDCCBBAA (layer A-F blend modes)
    ----------------DCBA------------ (layer A-D line/row scroll disables)
    ----FFEEDDCCBBAA---------------- (layer A-F mix codes in forced blending)
    ---x---------------------------- (disable shadows)
    --x----------------------------- (disable z-buffering)
*/
#define GXMIX_BLEND_AUTO	0			// emulate all blend effects
#define GXMIX_BLEND_NONE	1			// disable all blend effects
#define GXMIX_BLEND_FAST	2			// simulate translucency
#define GXMIX_BLEND_FORCE	3			// force mix code on selected layer(s)
#define GXMIX_NOLINESCROLL	0x1000		// disable linescroll on selected layer(s)
#define GXMIX_NOSHADOW		0x10000000	// disable all shadows (shadow pens will be skipped)
#define GXMIX_NOZBUF		0x20000000	// disable z-buffering (shadow pens will be drawn as solid)

// Sub Layer Flags
#define GXSUB_K053250	0x10	// chip type: 0=K053936 ROZ+, 1=K053250 LVC
#define GXSUB_4BPP 		0x04	//  16 colors
#define GXSUB_5BPP 		0x05	//  32 colors
#define GXSUB_8BPP 		0x08	// 256 colors

void konamigx_mixer(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect,
					tilemap *sub1, int sub1flags,
					tilemap *sub2, int sub2flags,
					int mixerflags);

void konamigx_mixer_init(int objdma);
void konamigx_mixer_primode(int mode);


void konamigx_objdma(void);


// K055550/K053990/ESC protection devices handlers
READ16_HANDLER ( K055550_word_r );
WRITE16_HANDLER( K055550_word_w );
WRITE16_HANDLER( K053990_martchmp_word_w );
void konamigx_esc_alert(UINT32 *srcbase, int srcoffs, int count, int mode);
#endif
