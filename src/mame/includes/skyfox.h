/*************************************************************************

    Skyfox

*************************************************************************/

class skyfox_state : public driver_device
{
public:
	skyfox_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    spriteram;
	size_t     spriteram_size;

	/* video-related */
	UINT8      vreg[8];
	int        bg_pos, bg_ctrl;

	/* misc */
	int        palette_selected;

	/* devices */
	device_t *maincpu;
};

/*----------- defined in video/skyfox.c -----------*/

WRITE8_HANDLER( skyfox_vregs_w );

PALETTE_INIT( skyfox );

VIDEO_UPDATE( skyfox );

