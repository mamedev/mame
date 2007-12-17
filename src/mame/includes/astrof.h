/***************************************************************************

    Astro Fighter hardware

****************************************************************************/


/*----------- defined in audio/astrof.c -----------*/

MACHINE_START( astrof_audio );
MACHINE_DRIVER_EXTERN( astrof_audio );
WRITE8_HANDLER( astrof_audio_1_w );
WRITE8_HANDLER( astrof_audio_2_w );

MACHINE_DRIVER_EXTERN( tomahawk_audio );
WRITE8_HANDLER( tomahawk_audio_w );
