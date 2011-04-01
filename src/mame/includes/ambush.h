/*************************************************************************

    Ambush

*************************************************************************/

class ambush_state : public driver_device
{
public:
	ambush_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_spriteram;
	UINT8 *    m_colorram;
	UINT8 *    m_scrollram;
	UINT8 *    m_colorbank;

	size_t     m_videoram_size;
	size_t     m_spriteram_size;
};


/*----------- defined in video/ambush.c -----------*/

PALETTE_INIT( ambush );
SCREEN_UPDATE( ambush );
