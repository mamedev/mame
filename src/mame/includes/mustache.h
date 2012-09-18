class mustache_state : public driver_device
{
public:
	mustache_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"){ }

	required_shared_ptr<UINT8> m_videoram;
	emu_timer *m_clear_irq_timer;
	tilemap_t *m_bg_tilemap;
	int m_control_byte;
	required_shared_ptr<UINT8> m_spriteram;
	DECLARE_WRITE8_MEMBER(mustache_videoram_w);
	DECLARE_WRITE8_MEMBER(mustache_video_control_w);
	DECLARE_WRITE8_MEMBER(mustache_scroll_w);
	DECLARE_DRIVER_INIT(mustache);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start();
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_mustache(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
