/***************************************************************************

    Capcom CPS-3 Hardware

****************************************************************************/

/*----------- defined in audio/cps3.c -----------*/

DEVICE_GET_INFO( cps3_sound );
#define SOUND_CPS3 DEVICE_GET_INFO_NAME(cps3_sound)

WRITE32_HANDLER( cps3_sound_w );
READ32_HANDLER( cps3_sound_r );

