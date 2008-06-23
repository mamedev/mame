/******************** NEW STUFF *******************/

#include "video/segaic16.h"

/*----------- defined in video/segahang.c -----------*/

VIDEO_START( hangon );
VIDEO_START( sharrier );
VIDEO_UPDATE( hangon );

/*----------- defined in video/segas16a.c -----------*/

VIDEO_START( system16a );
VIDEO_UPDATE( system16a );

/*----------- defined in video/segas16b.c -----------*/

VIDEO_START( system16b );
VIDEO_START( timscanr );
VIDEO_UPDATE( system16b );

/*----------- defined in video/segas18.c -----------*/

VIDEO_START( system18 );
VIDEO_UPDATE( system18 );

void system18_set_grayscale(running_machine *machine, int enable);
void system18_set_vdp_enable(running_machine *machine, int eanble);
void system18_set_vdp_mixing(running_machine *machine, int mixing);

/*----------- defined in video/segaorun.c -----------*/

VIDEO_START( outrun );
VIDEO_START( shangon );
VIDEO_UPDATE( outrun );
VIDEO_UPDATE( shangon );

/*----------- defined in video/segaxbd.c -----------*/

VIDEO_START( xboard );
VIDEO_UPDATE( xboard );
void xboard_set_road_priority(int priority);

/*----------- defined in video/segaybd.c -----------*/

VIDEO_START( yboard );
VIDEO_UPDATE( yboard );



/******************** OLD STUFF *******************/

#define SYS16_SPR_FLIPX						0x01
#define SYS16_SPR_VISIBLE					0x04
#define SYS16_SPR_DRAW_TO_LEFT				0x08
#define SYS16_SPR_SPECIAL					0x10
#define SYS16_SPR_SHADOW					0x20 /* all pixels */
#define SYS16_SPR_PARTIAL_SHADOW			0x40 /* pen #10 */
#define SYS16_SPR_DRAW_TO_TOP				0x80

struct sys16_sprite_attributes {
	int priority, flags;
	int gfx, color;
	UINT8 pitch;
	int zoomx, zoomy;
	int x,y, screen_height;	/* in screen coordinates */
	int shadow_pen;
};

extern int (*sys16_spritesystem)(
	struct sys16_sprite_attributes *sprite,
	const UINT16 *source,
	int bJustGetColor );

/*----------- defined in video/sys16spr.c -----------*/

extern int sys16_sprite_shinobi( struct sys16_sprite_attributes *sprite, const UINT16 *source, int bJustGetColor );
extern int sys16_sprite_passshot( struct sys16_sprite_attributes *sprite, const UINT16 *source, int bJustGetColor );
extern int sys16_sprite_quartet2( struct sys16_sprite_attributes *sprite, const UINT16 *source, int bJustGetColor );

#define NumOfShadowColors 32
#define ShadowColorsMultiplier 2

/*----------- defined in machine/s16fd.c -----------*/

void *fd1094_get_decrypted_base(void);
void fd1094_machine_init(void);
void fd1094_driver_init(running_machine *machine, void (*set_decrypted)(UINT8 *));

/*----------- defined in machine/system16.c -----------*/

extern UINT16 *sys16_workingram;
extern UINT16 *sys16_workingram2;
extern UINT16 *sys16_extraram;
extern UINT16 *sys16_extraram2;
extern UINT16 *sys16_extraram3;

/* sound */
extern const struct upd7759_interface sys16_upd7759_interface;

extern int sys18_sound_info[4*2];

extern void sys16_patch_code( int offset, int data );

#define SYS16_MWA16_PALETTERAM	sys16_paletteram_w
#define SYS16_MRA16_PALETTERAM	SMH_RAM

#define SYS16_MRA16_WORKINGRAM	SMH_RAM
#define SYS16_MWA16_WORKINGRAM	SMH_RAM

#define SYS16_MRA16_WORKINGRAM2	SMH_RAM
#define SYS16_MWA16_WORKINGRAM2	SMH_RAM

extern MACHINE_RESET( sys16_onetime );

#define SYS16_MRA16_SPRITERAM		SMH_RAM
#define SYS16_MWA16_SPRITERAM		SMH_RAM

#define SYS16_MRA16_TILERAM		sys16_tileram_r
#define SYS16_MWA16_TILERAM		sys16_tileram_w

#define SYS16_MRA16_TEXTRAM		sys16_textram_r
#define SYS16_MWA16_TEXTRAM		sys16_textram_w

#define SYS16_MRA16_EXTRAM		SMH_RAM
#define SYS16_MWA16_EXTRAM		SMH_RAM

#define SYS16_MRA16_EXTRAM2		SMH_RAM
#define SYS16_MWA16_EXTRAM2		SMH_RAM

#define SYS16_MRA16_EXTRAM3		SMH_RAM
#define SYS16_MWA16_EXTRAM3		SMH_RAM

#define SYS16_MRA16_EXTRAM4		SMH_RAM
#define SYS16_MWA16_EXTRAM4		SMH_RAM

#define SYS16_MRA16_ROADRAM		SMH_RAM
#define SYS16_MWA16_ROADRAM		SMH_RAM

GFXDECODE_EXTERN( sys16 );


#define SYS16_JOY1 PORT_START_TAG("IN0")\
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY

#define SYS16_JOY2 PORT_START_TAG("IN1") \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL

#define SYS16_SERVICE PORT_START_TAG("IN2") \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) \
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW) \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 ) \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 ) \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 ) \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

#define SYS16_COINAGE PORT_START_TAG("DSW1") \
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3,4") \
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x05, "2 Coins/1 Credit 5/3 6/4" ) \
	PORT_DIPSETTING(    0x04, "2 Coins/1 Credit 4/3" ) \
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x01, "1 Coin/1 Credit 2/3" ) \
	PORT_DIPSETTING(    0x02, "1 Coin/1 Credit 4/5" ) \
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit 5/6" ) \
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) ) \
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) ) \
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) ) \
	PORT_DIPSETTING(    0x00, "Free Play (if Coin B too) or 1/1" ) \
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,6,7,8") \
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x50, "2 Coins/1 Credit 5/3 6/4" ) \
	PORT_DIPSETTING(    0x40, "2 Coins/1 Credit 4/3" ) \
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x10, "1 Coin/1 Credit 2/3" ) \
	PORT_DIPSETTING(    0x20, "1 Coin/1 Credit 4/5" ) \
	PORT_DIPSETTING(    0x30, "1 Coin/1 Credit 5/6" ) \
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) ) \
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) ) \
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) ) \
	PORT_DIPSETTING(    0x00, "Free Play (if Coin A too) or 1/1" )


/*----------- defined in video/system16.c -----------*/

/* video hardware */
extern READ16_HANDLER( sys16_tileram_r );
extern WRITE16_HANDLER( sys16_tileram_w );
extern READ16_HANDLER( sys16_textram_r );
extern WRITE16_HANDLER( sys16_textram_w );
extern WRITE16_HANDLER( sys16_paletteram_w );

/* "normal" video hardware */
extern VIDEO_START( system16 );
extern VIDEO_UPDATE( system16 );

/* system18 video hardware */
extern VIDEO_START( system18old );
extern VIDEO_UPDATE( system18old );

/* callback to poll video registers */
extern void (* sys16_update_proc)( void );

extern UINT16 *sys16_tileram;
extern UINT16 *sys16_textram;
extern UINT16 *sys16_spriteram;

extern int sys16_sh_shadowpal;
extern int sys16_MaxShadowColors;

/* video driver constants (vary with game) */
extern int sys16_gr_bitmap_width;
extern int sys16_sprxoffset;
extern int sys16_bgxoffset;
extern int sys16_fgxoffset;
extern const int *sys16_obj_bank;
extern int sys16_textlayer_lo_min;
extern int sys16_textlayer_lo_max;
extern int sys16_textlayer_hi_min;
extern int sys16_textlayer_hi_max;
extern int sys16_bg1_trans;
extern int sys16_bg_priority_mode;
extern int sys16_fg_priority_mode;
extern int sys16_bg_priority_value;
extern int sys16_fg_priority_value;
extern int sys16_tilebank_switch;
extern int sys16_rowscroll_scroll;
extern int shinobl_kludge;

/* video driver registers */
extern int sys16_refreshenable;
extern int sys16_tile_bank0;
extern int sys16_tile_bank1;
extern int sys16_bg_scrollx, sys16_bg_scrolly;
extern int sys16_bg_page[4];
extern int sys16_fg_scrollx, sys16_fg_scrolly;
extern int sys16_fg_page[4];

extern int sys16_bg2_scrollx, sys16_bg2_scrolly;
extern int sys16_bg2_page[4];
extern int sys16_fg2_scrollx, sys16_fg2_scrolly;
extern int sys16_fg2_page[4];

extern int sys18_bg2_active;
extern int sys18_fg2_active;
extern UINT16 *sys18_splittab_bg_x;
extern UINT16 *sys18_splittab_bg_y;
extern UINT16 *sys18_splittab_fg_x;
extern UINT16 *sys18_splittab_fg_y;

extern UINT16 *sys16_gr_ver;
extern UINT16 *sys16_gr_hor;
extern UINT16 *sys16_gr_pal;
extern UINT16 *sys16_gr_flip;
extern int sys16_gr_palette;
extern int sys16_gr_palette_default;
extern UINT8 sys16_gr_colorflip[2][4];
extern UINT16 *sys16_gr_second_road;

