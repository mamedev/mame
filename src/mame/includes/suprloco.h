class suprloco_state : public driver_device
{
public:
	suprloco_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_scrollram(*this, "scrollram"){ }

	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_scrollram;
	tilemap_t *m_bg_tilemap;
	int m_control;

	DECLARE_WRITE8_MEMBER(suprloco_soundport_w);
	DECLARE_WRITE8_MEMBER(suprloco_videoram_w);
	DECLARE_WRITE8_MEMBER(suprloco_scrollram_w);
	DECLARE_WRITE8_MEMBER(suprloco_control_w);
	DECLARE_READ8_MEMBER(suprloco_control_r);
	DECLARE_DRIVER_INIT(suprloco);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_suprloco(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
