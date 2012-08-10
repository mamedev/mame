class deco_mlc_state : public driver_device
{
public:
	deco_mlc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_mlc_ram(*this, "mlc_ram"),
		m_irq_ram(*this, "irq_ram"),
		m_mlc_clip_ram(*this, "mlc_clip_ram"),
		m_spriteram(*this, "spriteram"),
		m_mlc_vram(*this, "mlc_vram"){ }

	required_shared_ptr<UINT32> m_mlc_ram;
	required_shared_ptr<UINT32> m_irq_ram;
	required_shared_ptr<UINT32> m_mlc_clip_ram;
	required_shared_ptr<UINT32> m_spriteram;
	required_shared_ptr<UINT32> m_mlc_vram;
	timer_device *m_raster_irq_timer;
	int m_mainCpuIsArm;
	int m_mlc_raster_table[9][256];
	UINT32 m_vbl_i;
	int m_lastScanline[9];
	UINT32 m_colour_mask;
	UINT32 *m_mlc_buffered_spriteram;
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
	DECLARE_WRITE32_MEMBER(avengrs_eprom_w);
	DECLARE_DRIVER_INIT(mlc);
	DECLARE_DRIVER_INIT(avengrgs);
};


/*----------- defined in video/deco_mlc.c -----------*/

VIDEO_START( mlc );
SCREEN_UPDATE_RGB32( mlc );
SCREEN_VBLANK( mlc );
