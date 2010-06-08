/*************************************************************************

    Gottlieb Exterminator hardware

*************************************************************************/

/*----------- defined in video/exterm.c -----------*/

extern UINT16 *exterm_master_videoram;
extern UINT16 *exterm_slave_videoram;

PALETTE_INIT( exterm );
void exterm_scanline_update(screen_device &screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params);

void exterm_to_shiftreg_master(const address_space *space, UINT32 address, UINT16* shiftreg);
void exterm_from_shiftreg_master(const address_space *space, UINT32 address, UINT16* shiftreg);
void exterm_to_shiftreg_slave(const address_space *space, UINT32 address, UINT16* shiftreg);
void exterm_from_shiftreg_slave(const address_space *space, UINT32 address, UINT16* shiftreg);
