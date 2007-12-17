/***************************************************************************

    Berzerk hardware

***************************************************************************/


#define BERZERK_MASTER_CLOCK	(10000000)
#define BERZERK_S14001A_CLOCK   (BERZERK_MASTER_CLOCK / 2)


/*----------- defined in audio/berzerk.c -----------*/

MACHINE_DRIVER_EXTERN( berzerk_audio );
WRITE8_HANDLER( berzerk_audio_w );
READ8_HANDLER( berzerk_audio_r );
