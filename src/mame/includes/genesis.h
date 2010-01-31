/* Todo, reorganise, cleanup etc.*/

/*----------- defined in video/genesis.c -----------*/

void system18_vdp_start(running_machine *machine);
void system18_vdp_update(bitmap_t *bitmap, const rectangle *cliprect);

READ16_HANDLER ( genesis_vdp_r );
WRITE16_HANDLER( genesis_vdp_w );
