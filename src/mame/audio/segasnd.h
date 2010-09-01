/*************************************************************************

    Sega g80 common sound hardware

*************************************************************************/

MACHINE_CONFIG_EXTERN( sega_speech_board );

WRITE8_HANDLER( sega_speech_data_w );
WRITE8_HANDLER( sega_speech_control_w );



MACHINE_CONFIG_EXTERN( sega_universal_sound_board );
MACHINE_CONFIG_EXTERN( sega_universal_sound_board_rom );

void sega_usb_reset(running_machine *machine, UINT8 t1_clock_mask);

READ8_HANDLER( sega_usb_status_r );
WRITE8_HANDLER( sega_usb_data_w );
READ8_HANDLER( sega_usb_ram_r );
WRITE8_HANDLER( sega_usb_ram_w );
