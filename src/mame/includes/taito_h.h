/*************************************************************************

    Taito H system

*************************************************************************/

class taitoh_state : public driver_device
{
public:
	taitoh_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *    m68000_mainram;
//  UINT16 *    paletteram;    // currently this uses generic palette handling

	/* misc */
	INT32       banknum;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *tc0080vco;
	running_device *tc0220ioc;
};

/*----------- defined in video/taito_h.c -----------*/

VIDEO_UPDATE( syvalion );
VIDEO_UPDATE( recordbr );
VIDEO_UPDATE( dleague );
