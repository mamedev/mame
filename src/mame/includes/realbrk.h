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
	bitmap_ind16 *m_tmpbitmap0;
	bitmap_ind16 *m_tmpbitmap1;
	int m_disable_video;
	tilemap_t *m_tilemap_0;
	tilemap_t *m_tilemap_1;
	tilemap_t *m_tilemap_2;
	UINT16 *m_spriteram;
	DECLARE_READ16_MEMBER(realbrk_dsw_r);
	DECLARE_READ16_MEMBER(pkgnsh_input_r);
	DECLARE_READ16_MEMBER(pkgnshdx_input_r);
	DECLARE_READ16_MEMBER(backup_ram_r);
	DECLARE_READ16_MEMBER(backup_ram_dx_r);
	DECLARE_WRITE16_MEMBER(backup_ram_w);
	DECLARE_WRITE16_MEMBER(realbrk_flipscreen_w);
	DECLARE_WRITE16_MEMBER(dai2kaku_flipscreen_w);
	DECLARE_WRITE16_MEMBER(realbrk_vram_0_w);
	DECLARE_WRITE16_MEMBER(realbrk_vram_1_w);
	DECLARE_WRITE16_MEMBER(realbrk_vram_2_w);
	DECLARE_WRITE16_MEMBER(realbrk_vregs_w);
};


/*----------- defined in video/realbrk.c -----------*/

#ifndef REALBRK_H
#define REALBRK_H

VIDEO_START(realbrk);
SCREEN_UPDATE_IND16(realbrk);
SCREEN_UPDATE_IND16(dai2kaku);



#endif

