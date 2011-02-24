/*************************************************************************

    Cinematronics vector hardware

*************************************************************************/


class cinemat_state : public driver_device
{
public:
	cinemat_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 sound_control;
	void (*sound_handler)(running_machine *,UINT8 sound_val, UINT8 bits_changed);
	UINT32 current_shift;
	UINT32 last_shift;
	UINT32 last_shift2;
	UINT32 current_pitch;
	UINT32 last_frame;
	UINT8 sound_fifo[16];
	UINT8 sound_fifo_in;
	UINT8 sound_fifo_out;
	UINT8 last_portb_write;
	float target_volume;
	float current_volume;
	UINT16 *rambase;
	UINT8 coin_detected;
	UINT8 coin_last_reset;
	UINT8 mux_select;
	int gear;
	int color_mode;
	rgb_t vector_color;
	INT16 lastx;
	INT16 lasty;
	UINT8 last_control;
	int qb3_lastx;
	int qb3_lasty;
};


/*----------- defined in drivers/cinemat.c -----------*/

MACHINE_RESET( cinemat );


/*----------- defined in audio/cinemat.c -----------*/

WRITE8_HANDLER( cinemat_sound_control_w );

MACHINE_CONFIG_EXTERN( spacewar_sound );
MACHINE_CONFIG_EXTERN( barrier_sound );
MACHINE_CONFIG_EXTERN( speedfrk_sound );
MACHINE_CONFIG_EXTERN( starhawk_sound );
MACHINE_CONFIG_EXTERN( sundance_sound );
MACHINE_CONFIG_EXTERN( tailg_sound );
MACHINE_CONFIG_EXTERN( warrior_sound );
MACHINE_CONFIG_EXTERN( armora_sound );
MACHINE_CONFIG_EXTERN( ripoff_sound );
MACHINE_CONFIG_EXTERN( starcas_sound );
MACHINE_CONFIG_EXTERN( solarq_sound );
MACHINE_CONFIG_EXTERN( boxingb_sound );
MACHINE_CONFIG_EXTERN( wotw_sound );
MACHINE_CONFIG_EXTERN( wotwc_sound );
MACHINE_CONFIG_EXTERN( demon_sound );
MACHINE_CONFIG_EXTERN( qb3_sound );


/*----------- defined in video/cinemat.c -----------*/

void cinemat_vector_callback(device_t *device, INT16 sx, INT16 sy, INT16 ex, INT16 ey, UINT8 shift);
WRITE8_HANDLER( cinemat_vector_control_w );

VIDEO_START( cinemat_bilevel );
VIDEO_START( cinemat_16level );
VIDEO_START( cinemat_64level );
VIDEO_START( cinemat_color );
VIDEO_START( cinemat_qb3color );
SCREEN_UPDATE( cinemat );

SCREEN_UPDATE( spacewar );
