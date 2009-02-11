/*************************************************************************

    Atari Battle Zone hardware

*************************************************************************/


/*----------- defined in drivers/bzone.c -----------*/

extern UINT8 rb_input_select;


/*----------- defined in audio/bzone.c -----------*/

WRITE8_HANDLER( bzone_sounds_w );

DEVICE_GET_INFO( bzone_sound );
#define SOUND_BZONE DEVICE_GET_INFO_NAME(bzone_sound)


/*----------- defined in audio/redbaron.c -----------*/

WRITE8_HANDLER( redbaron_sounds_w );
WRITE8_DEVICE_HANDLER( redbaron_pokey_w );

DEVICE_GET_INFO( redbaron_sound );
#define SOUND_REDBARON DEVICE_GET_INFO_NAME(redbaron_sound)
