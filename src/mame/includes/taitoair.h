/*************************************************************************

    Taito Air System

*************************************************************************/

enum { TAITOAIR_FRAC_SHIFT = 16, TAITOAIR_POLY_MAX_PT = 16 };

struct taitoair_spoint {
	INT32 x, y;
};

struct taitoair_poly {
	struct taitoair_spoint p[TAITOAIR_POLY_MAX_PT];
	int pcount;
	int col;
};


class taitoair_state : public driver_device
{
public:
	taitoair_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *      m68000_mainram;
	UINT16 *      line_ram;
	UINT16 *      dsp_ram;	/* Shared 68000/TMS32025 RAM */
	UINT16 *      paletteram;

	/* video-related */
	taitoair_poly  q;

	/* misc */
	int           dsp_hold_signal;
	INT32         banknum;

	/* devices */
	running_device *audiocpu;
	running_device *dsp;
	running_device *tc0080vco;
};


/*----------- defined in video/taitoair.c -----------*/

VIDEO_UPDATE( taitoair );
