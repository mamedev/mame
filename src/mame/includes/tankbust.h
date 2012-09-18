class tankbust_state : public driver_device
{
public:
	tankbust_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_txtram(*this, "txtram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"){ }

	int m_latch;
	UINT32 m_timer1;
	int m_e0xx_data[8];
	UINT8 m_variable_data;
	required_shared_ptr<UINT8> m_txtram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_txt_tilemap;
	UINT8 m_xscroll[2];
	UINT8 m_yscroll[2];
	required_shared_ptr<UINT8> m_spriteram;

	UINT8 m_irq_mask;
	DECLARE_WRITE8_MEMBER(tankbust_soundlatch_w);
	DECLARE_WRITE8_MEMBER(tankbust_e0xx_w);
	DECLARE_READ8_MEMBER(debug_output_area_r);
	DECLARE_READ8_MEMBER(read_from_unmapped_memory);
	DECLARE_READ8_MEMBER(some_changing_input);
	DECLARE_WRITE8_MEMBER(tankbust_background_videoram_w);
	DECLARE_READ8_MEMBER(tankbust_background_videoram_r);
	DECLARE_WRITE8_MEMBER(tankbust_background_colorram_w);
	DECLARE_READ8_MEMBER(tankbust_background_colorram_r);
	DECLARE_WRITE8_MEMBER(tankbust_txtram_w);
	DECLARE_READ8_MEMBER(tankbust_txtram_r);
	DECLARE_WRITE8_MEMBER(tankbust_xscroll_w);
	DECLARE_WRITE8_MEMBER(tankbust_yscroll_w);
	DECLARE_READ8_MEMBER(tankbust_soundlatch_r);
	DECLARE_READ8_MEMBER(tankbust_soundtimer_r);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_txt_tile_info);
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_tankbust(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
