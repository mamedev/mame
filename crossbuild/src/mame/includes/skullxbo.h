/*************************************************************************

    Atari Skull & Crossbones hardware

*************************************************************************/

/*----------- defined in video/skullxbo.c -----------*/

WRITE16_HANDLER( skullxbo_playfieldlatch_w );
WRITE16_HANDLER( skullxbo_xscroll_w );
WRITE16_HANDLER( skullxbo_yscroll_w );
WRITE16_HANDLER( skullxbo_mobmsb_w );

VIDEO_START( skullxbo );
VIDEO_UPDATE( skullxbo );

void skullxbo_scanline_update(int param);
