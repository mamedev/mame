/*************************************************************************

    Atari Tetris hardware

*************************************************************************/

class atetris_state : public driver_device
{
public:
	atetris_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_nvram(*this, "nvram") ,
		m_videoram(*this, "videoram"){ }

	required_shared_ptr<UINT8>	m_nvram;
	required_shared_ptr<UINT8> m_videoram;
	UINT8 *m_slapstic_source;
	UINT8 *m_slapstic_base;
	UINT8 m_current_bank;
	UINT8 m_nvram_write_enable;
	emu_timer *m_interrupt_timer;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(irq_ack_w);
	DECLARE_READ8_MEMBER(atetris_slapstic_r);
	DECLARE_WRITE8_MEMBER(coincount_w);
	DECLARE_WRITE8_MEMBER(nvram_w);
	DECLARE_WRITE8_MEMBER(nvram_enable_w);
	DECLARE_WRITE8_MEMBER(atetris_videoram_w);
	DECLARE_DRIVER_INIT(atetris);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_atetris(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

/*----------- defined in video/atetris.c -----------*/




