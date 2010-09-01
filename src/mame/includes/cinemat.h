/*************************************************************************

    Cinematronics vector hardware

*************************************************************************/


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

void cinemat_vector_callback(running_device *device, INT16 sx, INT16 sy, INT16 ex, INT16 ey, UINT8 shift);
WRITE8_HANDLER( cinemat_vector_control_w );

VIDEO_START( cinemat_bilevel );
VIDEO_START( cinemat_16level );
VIDEO_START( cinemat_64level );
VIDEO_START( cinemat_color );
VIDEO_START( cinemat_qb3color );
VIDEO_UPDATE( cinemat );

VIDEO_UPDATE( spacewar );
