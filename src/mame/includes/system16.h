/******************** NEW STUFF *******************/

#include "video/segaic16.h"
#include "sound/upd7759.h"

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


/*----------- defined in machine/s16fd.c -----------*/

void *fd1094_get_decrypted_base(void);
void fd1094_machine_init(running_device *device);
void fd1094_driver_init(running_machine *machine, const char* tag, void (*set_decrypted)(running_machine *, UINT8 *));

/*----------- defined in machine/system16.c -----------*/

/* sound */
extern const upd7759_interface sys16_upd7759_interface;

extern int sys18_sound_info[4*2];

typedef struct _sys16_patch sys16_patch;
struct _sys16_patch
{
	offs_t offset;
	UINT8 data;
};

extern void sys16_patch_code( running_machine *machine, const sys16_patch *patch, int count );

extern MACHINE_RESET( sys16_onetime );

GFXDECODE_EXTERN( sys16 );

/*----------- defined in video/system16.c -----------*/

extern VIDEO_START( system16a_bootleg );
extern VIDEO_START( system16a_bootleg_wb3bl );
extern VIDEO_START( system16a_bootleg_shinobi );
extern VIDEO_START( system16a_bootleg_passsht );
extern VIDEO_UPDATE( system16a_bootleg );
extern VIDEO_UPDATE( system16a_bootleg_passht4b );
extern WRITE16_HANDLER( system16a_bootleg_tilemapselect_w );
extern WRITE16_HANDLER( system16a_bootleg_bgscrolly_w );
extern WRITE16_HANDLER( system16a_bootleg_bgscrollx_w );
extern WRITE16_HANDLER( system16a_bootleg_fgscrolly_w );
extern WRITE16_HANDLER( system16a_bootleg_fgscrollx_w );
extern UINT16* system16a_bootleg_bg0_tileram;
extern UINT16* system16a_bootleg_bg1_tileram;

/* video hardware */
extern WRITE16_HANDLER( sys16_tileram_w );
extern WRITE16_HANDLER( sys16_textram_w );

/* "normal" video hardware */
extern VIDEO_START( system16 );
extern VIDEO_UPDATE( system16 );

/* system18 video hardware */
extern VIDEO_START( system18old );
extern VIDEO_UPDATE( system18old );

extern UINT16 *sys16_tileram;
extern UINT16 *sys16_textram;
extern UINT16 *sys16_spriteram;

extern int sys16_sh_shadowpal;
extern int sys16_MaxShadowColors;

/* video driver constants (vary with game) */
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

extern int sys16_bg2_page[4];
extern int sys16_fg2_page[4];

extern int sys18_bg2_active;
extern int sys18_fg2_active;
extern UINT16 *sys18_splittab_bg_x;
extern UINT16 *sys18_splittab_bg_y;
extern UINT16 *sys18_splittab_fg_x;
extern UINT16 *sys18_splittab_fg_y;

