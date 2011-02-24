class realbrk_state : public driver_device
{
public:
	realbrk_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *dsw_select;
	UINT16 *backup_ram;
	UINT16 *vram_0;
	UINT16 *vram_1;
	UINT16 *vram_2;
	UINT16 *vregs;
	UINT16 *vram_0ras;
	UINT16 *vram_1ras;
	bitmap_t *tmpbitmap0;
	bitmap_t *tmpbitmap1;
	int disable_video;
	tilemap_t *tilemap_0;
	tilemap_t *tilemap_1;
	tilemap_t *tilemap_2;
};


/*----------- defined in video/realbrk.c -----------*/

#ifndef REALBRK_H
#define REALBRK_H

VIDEO_START(realbrk);
SCREEN_UPDATE(realbrk);
SCREEN_UPDATE(dai2kaku);

WRITE16_HANDLER( realbrk_vram_0_w );
WRITE16_HANDLER( realbrk_vram_1_w );
WRITE16_HANDLER( realbrk_vram_2_w );
WRITE16_HANDLER( realbrk_vregs_w );
WRITE16_HANDLER( realbrk_flipscreen_w );
WRITE16_HANDLER( dai2kaku_flipscreen_w );


#endif

