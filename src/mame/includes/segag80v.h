/*************************************************************************

    Sega vector hardware

*************************************************************************/


class segag80v_state : public driver_device
{
public:
	segag80v_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *mainram;
	UINT8 has_usb;
	UINT8 mult_data[2];
	UINT16 mult_result;
	UINT8 spinner_select;
	UINT8 spinner_sign;
	UINT8 spinner_count;
	UINT8 *vectorram;
	size_t vectorram_size;
	int min_x;
	int min_y;
};


/*----------- defined in audio/segag80v.c -----------*/

WRITE8_HANDLER( elim1_sh_w );
WRITE8_HANDLER( elim2_sh_w );
WRITE8_HANDLER( spacfury1_sh_w );
WRITE8_HANDLER( spacfury2_sh_w );
WRITE8_HANDLER( zektor1_sh_w );
WRITE8_HANDLER( zektor2_sh_w );


/*----------- defined in video/segag80v.c -----------*/

VIDEO_START( segag80v );
VIDEO_UPDATE( segag80v );
