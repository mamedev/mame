/*************************************************************************

    Skyfox

*************************************************************************/

class skyfox_state : public driver_device
{
public:
	skyfox_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    m_spriteram;
	size_t     m_spriteram_size;

	/* video-related */
	UINT8      m_vreg[8];
	int        m_bg_pos;
	int        m_bg_ctrl;

	/* misc */
	int        m_palette_selected;

	/* devices */
	device_t *m_maincpu;
};

/*----------- defined in video/skyfox.c -----------*/

WRITE8_HANDLER( skyfox_vregs_w );

PALETTE_INIT( skyfox );

SCREEN_UPDATE( skyfox );

