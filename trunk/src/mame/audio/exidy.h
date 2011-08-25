DECLARE_LEGACY_SOUND_DEVICE(EXIDY, exidy_sound);
DECLARE_LEGACY_SOUND_DEVICE(EXIDY_VENTURE, venture_sound);
DECLARE_LEGACY_SOUND_DEVICE(EXIDY_VICTORY, victory_sound);

READ8_DEVICE_HANDLER( exidy_sh6840_r );
WRITE8_DEVICE_HANDLER( exidy_sh6840_w );
WRITE8_DEVICE_HANDLER( exidy_sfxctrl_w );

MACHINE_CONFIG_EXTERN( venture_audio );

MACHINE_CONFIG_EXTERN( mtrap_cvsd_audio );

MACHINE_CONFIG_EXTERN( victory_audio );
READ8_DEVICE_HANDLER( victory_sound_response_r );
READ8_DEVICE_HANDLER( victory_sound_status_r );
WRITE8_DEVICE_HANDLER( victory_sound_command_w );
