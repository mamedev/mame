/*************************************************************************

    Model Racing Dribbling hardware

*************************************************************************/



class dribling_state : public driver_device
{
public:
	dribling_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  videoram;
	UINT8 *  colorram;

	/* misc */
	UINT8    abca;
	UINT8    dr, ds, sh;
	UINT8    input_mux;
	UINT8    di;

	/* devices */
	running_device *maincpu;
	running_device *ppi_0;
	running_device *ppi_1;
};


/*----------- defined in video/dribling.c -----------*/

PALETTE_INIT( dribling );
WRITE8_HANDLER( dribling_colorram_w );
VIDEO_UPDATE( dribling );
