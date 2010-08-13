/*************************************************************************

    Sega vector hardware

*************************************************************************/


/*----------- defined in audio/segag80v.c -----------*/

WRITE8_HANDLER( elim1_sh_w );
WRITE8_HANDLER( elim2_sh_w );
WRITE8_HANDLER( spacfury1_sh_w );
WRITE8_HANDLER( spacfury2_sh_w );
WRITE8_HANDLER( zektor1_sh_w );
WRITE8_HANDLER( zektor2_sh_w );


/*----------- defined in video/segag80v.c -----------*/

extern UINT8 *segag80v_vectorram;
extern size_t segag80v_vectorram_size;

VIDEO_START( segag80v );
VIDEO_UPDATE( segag80v );
