/*************************************************************************

    Atari Food Fight hardware

*************************************************************************/

/*----------- defined in video/foodf.c -----------*/

WRITE16_HANDLER( foodf_paletteram_w );

void foodf_set_flip(int flip);
VIDEO_START( foodf );
VIDEO_UPDATE( foodf );
