/*************************************************************************

    Atari GT hardware

*************************************************************************/

/*----------- defined in drivers/atarigt.c -----------*/

extern UINT8 atarigt_is_primrage;


/*----------- defined in video/atarigt.c -----------*/

extern UINT16 *atarigt_colorram;

void atarigt_colorram_w(offs_t address, UINT16 data, UINT16 mem_mask);
UINT16 atarigt_colorram_r(offs_t address);

VIDEO_START( atarigt );
VIDEO_UPDATE( atarigt );

void atarigt_scanline_update(running_machine *machine, int scrnum, int scanline);
