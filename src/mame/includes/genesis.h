/* Todo, reorganise, cleanup etc.*/

/*----------- defined in video/genesis.c -----------*/

extern UINT8		genesis_vdp_regs[];
extern UINT16		genesis_bg_pal_lookup[];
extern UINT16		genesis_sp_pal_lookup[];

VIDEO_START( genesis );

VIDEO_UPDATE( genesis );

void system18_vdp_start(running_machine *machine);
void system18_vdp_update(bitmap_t *bitmap, const rectangle *cliprect);

READ16_HANDLER ( genesis_vdp_r );
WRITE16_HANDLER( genesis_vdp_w );
