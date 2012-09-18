

class docastle_state : public driver_device
{
public:
	docastle_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t  *m_do_tilemap;

	/* misc */
	int      m_adpcm_pos;
	int      m_adpcm_idle;
	int      m_adpcm_data;
	int      m_adpcm_status;
	UINT8    m_buffer0[9];
	UINT8    m_buffer1[9];

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_slave;
	DECLARE_READ8_MEMBER(docastle_shared0_r);
	DECLARE_READ8_MEMBER(docastle_shared1_r);
	DECLARE_WRITE8_MEMBER(docastle_shared0_w);
	DECLARE_WRITE8_MEMBER(docastle_shared1_w);
	DECLARE_WRITE8_MEMBER(docastle_nmitrigger_w);
	DECLARE_WRITE8_MEMBER(docastle_videoram_w);
	DECLARE_WRITE8_MEMBER(docastle_colorram_w);
	DECLARE_READ8_MEMBER(docastle_flipscreen_off_r);
	DECLARE_READ8_MEMBER(docastle_flipscreen_on_r);
	DECLARE_WRITE8_MEMBER(docastle_flipscreen_off_w);
	DECLARE_WRITE8_MEMBER(docastle_flipscreen_on_w);
	DECLARE_READ8_MEMBER(idsoccer_adpcm_status_r);
	DECLARE_WRITE8_MEMBER(idsoccer_adpcm_w);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	DECLARE_VIDEO_START(dorunrun);
	UINT32 screen_update_docastle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
