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
	UINT16 *      m_m68000_mainram;
	UINT16 *      m_line_ram;
	UINT16 *      m_dsp_ram;	/* Shared 68000/TMS32025 RAM */
	UINT16 *      m_paletteram;

	/* video-related */
	taitoair_poly  m_q;

	/* misc */
	int           m_dsp_hold_signal;
	INT32         m_banknum;

	/* devices */
	device_t *m_audiocpu;
	device_t *m_dsp;
	device_t *m_tc0080vco;
};


/*----------- defined in video/taitoair.c -----------*/

SCREEN_UPDATE( taitoair );
