/*************************************************************************

    Atari G1 hardware

*************************************************************************/

/*----------- defined in video/atarig1.c -----------*/

WRITE16_HANDLER( atarig1_mo_control_w );

VIDEO_START( atarig1 );
VIDEO_UPDATE( atarig1 );

void atarig1_scanline_update(running_machine *machine, int scrnum, int scanline);

extern UINT8 atarig1_pitfight;
