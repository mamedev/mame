class realbrk_state : public driver_device
{
public:
	realbrk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_dsw_select;
	UINT16 *m_backup_ram;
	UINT16 *m_vram_0;
	UINT16 *m_vram_1;
	UINT16 *m_vram_2;
	UINT16 *m_vregs;
	UINT16 *m_vram_0ras;
	UINT16 *m_vram_1ras;
	bitmap_t *m_tmpbitmap0;
	bitmap_t *m_tmpbitmap1;
	int m_disable_video;
	tilemap_t *m_tilemap_0;
	tilemap_t *m_tilemap_1;
	tilemap_t *m_tilemap_2;
	UINT16 *m_spriteram;
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

