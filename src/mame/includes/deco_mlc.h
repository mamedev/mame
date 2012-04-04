class deco_mlc_state : public driver_device
{
public:
	deco_mlc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT32 *m_mlc_ram;
	UINT32 *m_irq_ram;
	timer_device *m_raster_irq_timer;
	int m_mainCpuIsArm;
	int m_mlc_raster_table[9][256];
	UINT32 m_vbl_i;
	int m_lastScanline[9];
	UINT32 *m_mlc_vram;
	UINT32 *m_mlc_clip_ram;
	UINT32 m_colour_mask;
	UINT32 *m_mlc_buffered_spriteram;
	UINT32 *m_spriteram;
	size_t m_spriteram_size;
	DECLARE_READ32_MEMBER(test2_r);
	DECLARE_READ32_MEMBER(test3_r);
	DECLARE_WRITE32_MEMBER(avengrs_palette_w);
	DECLARE_READ32_MEMBER(decomlc_vbl_r);
	DECLARE_READ32_MEMBER(mlc_scanline_r);
	DECLARE_WRITE32_MEMBER(mlc_irq_w);
	DECLARE_READ32_MEMBER(mlc_spriteram_r);
	DECLARE_READ32_MEMBER(mlc_vram_r);
	DECLARE_READ32_MEMBER(stadhr96_prot_146_r);
	DECLARE_READ32_MEMBER(avengrgs_speedup_r);
};


/*----------- defined in video/deco_mlc.c -----------*/

VIDEO_START( mlc );
SCREEN_UPDATE_RGB32( mlc );
SCREEN_VBLANK( mlc );
