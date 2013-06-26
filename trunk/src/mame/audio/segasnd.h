/*************************************************************************

    Sega g80 common sound hardware

*************************************************************************/

MACHINE_CONFIG_EXTERN( sega_speech_board );

DECLARE_WRITE8_DEVICE_HANDLER( sega_speech_data_w );
DECLARE_WRITE8_DEVICE_HANDLER( sega_speech_control_w );



MACHINE_CONFIG_EXTERN( sega_universal_sound_board );
MACHINE_CONFIG_EXTERN( sega_universal_sound_board_rom );

DECLARE_READ8_DEVICE_HANDLER( sega_usb_status_r );
DECLARE_WRITE8_DEVICE_HANDLER( sega_usb_data_w );
DECLARE_READ8_DEVICE_HANDLER( sega_usb_ram_r );
DECLARE_WRITE8_DEVICE_HANDLER( sega_usb_ram_w );
