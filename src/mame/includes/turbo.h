/*************************************************************************

    Sega Z80-3D system

*************************************************************************/

#include "sound/discrete.h"

/* sprites are scaled in the analog domain; to give a better */
/* rendition of this, we scale in the X direction by this factor */
#define TURBO_X_SCALE		2


typedef struct _i8279_state i8279_state;
struct _i8279_state
{
	UINT8		command;
	UINT8		mode;
	UINT8		prescale;
	UINT8		inhibit;
	UINT8		clear;
	UINT8		ram[16];
};


typedef struct _turbo_state turbo_state;
struct _turbo_state
{
	/* memory pointers */
	UINT8 *		videoram;
	UINT8 *		spriteram;
	UINT8 *		sprite_position;
	UINT8 *		buckrog_bitmap_ram;

	/* machine states */
	i8279_state	i8279;

	/* sound state */
	UINT8		turbo_osel, turbo_bsel;
	UINT8		sound_state[3];

	/* video state */
	tilemap *	fg_tilemap;

	/* Turbo-specific states */
	UINT8		turbo_opa, turbo_opb, turbo_opc;
	UINT8		turbo_ipa, turbo_ipb, turbo_ipc;
	UINT8		turbo_fbpla, turbo_fbcol;
	UINT8		turbo_speed;
	UINT8		turbo_collision;
	UINT8		turbo_last_analog;
	UINT8		turbo_accel;

	/* Subroc-specific states */
	UINT8		subroc3d_col, subroc3d_ply, subroc3d_flip;
	UINT8		subroc3d_mdis, subroc3d_mdir;
	UINT8		subroc3d_tdis, subroc3d_tdir;
	UINT8		subroc3d_fdis, subroc3d_fdir;
	UINT8		subroc3d_hdis, subroc3d_hdir;

	/* Buck Rogers-specific states */
	UINT8		buckrog_fchg, buckrog_mov, buckrog_obch;
	UINT8		buckrog_command;
	UINT8		buckrog_myship;
};


/*----------- defined in audio/turbo.c -----------*/

MACHINE_DRIVER_EXTERN( turbo_samples );
MACHINE_DRIVER_EXTERN( subroc3d_samples );
MACHINE_DRIVER_EXTERN( buckrog_samples );

WRITE8_HANDLER( turbo_sound_a_w );
WRITE8_HANDLER( turbo_sound_b_w );
WRITE8_HANDLER( turbo_sound_c_w );

WRITE8_HANDLER( subroc3d_sound_a_w );
WRITE8_HANDLER( subroc3d_sound_b_w );
WRITE8_HANDLER( subroc3d_sound_c_w );

WRITE8_HANDLER( buckrog_sound_a_w );
WRITE8_HANDLER( buckrog_sound_b_w );


/*----------- defined in video/turbo.c -----------*/

PALETTE_INIT( turbo );
VIDEO_START( turbo );
VIDEO_UPDATE( turbo );

PALETTE_INIT( subroc3d );
VIDEO_UPDATE( subroc3d );

PALETTE_INIT( buckrog );
VIDEO_START( buckrog );
VIDEO_UPDATE( buckrog );

WRITE8_HANDLER( turbo_videoram_w );
WRITE8_HANDLER( buckrog_bitmap_w );
