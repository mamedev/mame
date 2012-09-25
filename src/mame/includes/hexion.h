class hexion_state : public driver_device
{
public:
	hexion_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_vram[2];
	UINT8 *m_unkram;
	int m_bankctrl;
	int m_rambank;
	int m_pmcbank;
	int m_gfxrom_select;
	tilemap_t *m_bg_tilemap[2];
	DECLARE_WRITE8_MEMBER(coincntr_w);
	DECLARE_WRITE8_MEMBER(hexion_bankswitch_w);
	DECLARE_READ8_MEMBER(hexion_bankedram_r);
	DECLARE_WRITE8_MEMBER(hexion_bankedram_w);
	DECLARE_WRITE8_MEMBER(hexion_bankctrl_w);
	DECLARE_WRITE8_MEMBER(hexion_gfxrom_select_w);
	DECLARE_WRITE_LINE_MEMBER(hexion_irq_ack_w);
	DECLARE_WRITE_LINE_MEMBER(hexion_nmi_ack_w);
	TILE_GET_INFO_MEMBER(get_tile_info0);
	TILE_GET_INFO_MEMBER(get_tile_info1);
	virtual void video_start();
	UINT32 screen_update_hexion(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(hexion_scanline);
};
