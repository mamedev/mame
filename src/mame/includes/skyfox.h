/*************************************************************************

    Skyfox

*************************************************************************/

typedef struct _skyfox_state skyfox_state;
struct _skyfox_state
{
	/* memory pointers */
	UINT8 *    spriteram;
	size_t     spriteram_size;

	/* video-related */
	UINT8      vreg[8];
	int        bg_pos, bg_ctrl;

	/* misc */
	int        palette_selected;

	/* devices */
	running_device *maincpu;
};

/*----------- defined in video/skyfox.c -----------*/

WRITE8_HANDLER( skyfox_vregs_w );

PALETTE_INIT( skyfox );

VIDEO_UPDATE( skyfox );

