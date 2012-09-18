class sbugger_state : public driver_device
{
public:
	sbugger_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram_attr(*this, "videoram_attr"),
		m_videoram(*this, "videoram"){ }

	required_shared_ptr<UINT8> m_videoram_attr;
	required_shared_ptr<UINT8> m_videoram;

	tilemap_t *m_tilemap;
	DECLARE_WRITE8_MEMBER(sbugger_videoram_w);
	DECLARE_WRITE8_MEMBER(sbugger_videoram_attr_w);
	DECLARE_WRITE_LINE_MEMBER(sbugger_interrupt);
	TILE_GET_INFO_MEMBER(get_sbugger_tile_info);
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_sbugger(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
