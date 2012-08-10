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
};


/*----------- defined in video/suprloco.c -----------*/

PALETTE_INIT( suprloco );
VIDEO_START( suprloco );
SCREEN_UPDATE_IND16( suprloco );
