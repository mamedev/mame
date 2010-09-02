
class mugsmash_state : public driver_device
{
public:
	mugsmash_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *videoram1;
	UINT16 *videoram2;
	UINT16 *spriteram;
	UINT16 *regs1;
	UINT16 *regs2;

	tilemap_t *tilemap1;
	tilemap_t *tilemap2;

	running_device *maincpu;
	running_device *audiocpu;
};


/*----------- defined in video/mugsmash.c -----------*/

VIDEO_START( mugsmash );
VIDEO_UPDATE( mugsmash );

WRITE16_HANDLER( mugsmash_reg_w );
WRITE16_HANDLER( mugsmash_videoram2_w );
WRITE16_HANDLER( mugsmash_videoram1_w );
