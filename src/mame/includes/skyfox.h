/*************************************************************************

    Skyfox

*************************************************************************/

class skyfox_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, skyfox_state(machine)); }

	skyfox_state(running_machine &machine)
		: driver_data_t(machine) { }

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

