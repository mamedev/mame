/***************************************************************************

    Sigma Spiders hardware

***************************************************************************/


/*----------- defined in audio/spiders.c -----------*/

WRITE8_HANDLER( spiders_audio_command_w );
WRITE8_HANDLER( spiders_audio_a_w );
WRITE8_HANDLER( spiders_audio_b_w );
WRITE8_HANDLER( spiders_audio_ctrl_w );

MACHINE_DRIVER_EXTERN( spiders_audio );
