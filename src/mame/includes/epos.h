/*************************************************************************

    Epos games

**************************************************************************/

class epos_state : public driver_device
{
public:
	epos_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  videoram;
	size_t   videoram_size;

	/* video-related */
	UINT8    palette;

	/* misc */
	int      counter;
};


/*----------- defined in video/epos.c -----------*/

WRITE8_HANDLER( epos_port_1_w );
VIDEO_UPDATE( epos );
