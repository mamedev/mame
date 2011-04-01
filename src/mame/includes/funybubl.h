

class funybubl_state : public driver_device
{
public:
	funybubl_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    m_paletteram;

	/* devices */
	device_t *m_audiocpu;

	/* memory */
	UINT8      m_banked_vram[0x2000];
};



/*----------- defined in video/funybubl.c -----------*/

WRITE8_HANDLER ( funybubl_paldatawrite );

VIDEO_START(funybubl);
SCREEN_UPDATE(funybubl);
