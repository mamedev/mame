/*************************************************************************

    3dfx Voodoo Graphics SST-1 emulator

    driver by Aaron Giles

**************************************************************************/

#define MAX_VOODOO			2

/* enumeration specifying which model of Voodoo we are emulating */
enum
{
	VOODOO_1,
	VOODOO_2,
	VOODOO_BANSHEE,
	VOODOO_3,
	MAX_VOODOO_TYPES
};

void voodoo_start(int which, const device_config *screen, int type, int fbmem_in_mb, int tmem0_in_mb, int tmem1_in_mb);
int voodoo_update(int which, bitmap_t *bitmap, const rectangle *cliprect);
void voodoo_reset(int which);
void voodoo_exit(int which);
int voodoo_get_type(int which);
int voodoo_is_stalled(int which);
void voodoo_set_init_enable(int which, UINT32 newval);
void voodoo_set_vblank_callback(int which, void (*vblank)(running_machine *, int));
void voodoo_set_stall_callback(int which, void (*stall)(running_machine *, int));

READ32_HANDLER( voodoo_0_r );
WRITE32_HANDLER( voodoo_0_w );

READ32_HANDLER( voodoo_1_r );
WRITE32_HANDLER( voodoo_1_w );

READ32_HANDLER( banshee_0_r );
WRITE32_HANDLER( banshee_0_w );
READ32_HANDLER( banshee_fb_0_r );
WRITE32_HANDLER( banshee_fb_0_w );
READ32_HANDLER( banshee_io_0_r );
WRITE32_HANDLER( banshee_io_0_w );
READ32_HANDLER( banshee_rom_0_r );

READ32_HANDLER( banshee_1_r );
WRITE32_HANDLER( banshee_1_w );
READ32_HANDLER( banshee_fb_1_r );
WRITE32_HANDLER( banshee_fb_1_w );
READ32_HANDLER( banshee_io_1_r );
WRITE32_HANDLER( banshee_io_1_w );
READ32_HANDLER( banshee_rom_1_r );
