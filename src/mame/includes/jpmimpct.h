/***************************************************************************

    JPM IMPACT with Video hardware

****************************************************************************/

/*----------- defined in video/jpmimpct.c -----------*/

extern UINT16 *tms_vram;

READ16_HANDLER( bt477_r );
WRITE16_HANDLER( bt477_w );

void jpmimpct_to_shiftreg(const address_space *space, UINT32 address, UINT16 *shiftreg);
void jpmimpct_from_shiftreg(const address_space *space, UINT32 address, UINT16 *shiftreg);
void jpmimpct_scanline_update(const device_config *screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params);

VIDEO_START( jpmimpct );
