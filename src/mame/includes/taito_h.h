/*************************************************************************

    Taito H system

*************************************************************************/

class taitoh_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, taitoh_state(machine)); }

	taitoh_state(running_machine &machine)
		: driver_data_t(machine) { }

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
