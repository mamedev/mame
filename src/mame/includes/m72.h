/*************************************************************************

    Irem M72 hardware

*************************************************************************/

class m72_state : public driver_device
{
public:
	m72_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_protection_ram;
	emu_timer *m_scanline_timer;
	UINT8 m_irq_base;
	UINT8 m_mcu_snd_cmd_latch;
	UINT8 m_mcu_sample_latch;
	UINT32 m_mcu_sample_addr;
	const UINT8 *m_protection_code;
	const UINT8 *m_protection_crc;
	UINT8 *m_soundram;
	int m_prev[4];
	int m_diff[4];
	UINT16 *m_videoram1;
	UINT16 *m_videoram2;
	UINT16 *m_majtitle_rowscrollram;
	UINT32 m_raster_irq_position;
	UINT16 *m_spriteram;
	UINT16 *m_spriteram2;
	UINT16 *m_buffered_spriteram;
	size_t m_spriteram_size;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	INT32 m_scrollx1;
	INT32 m_scrolly1;
	INT32 m_scrollx2;
	INT32 m_scrolly2;
	INT32 m_video_off;
	int m_majtitle_rowscroll;
	device_t *m_audio;
	DECLARE_WRITE16_MEMBER(m72_main_mcu_sound_w);
	DECLARE_WRITE16_MEMBER(m72_main_mcu_w);
	DECLARE_WRITE8_MEMBER(m72_mcu_data_w);
	DECLARE_READ8_MEMBER(m72_mcu_data_r);
	DECLARE_READ8_MEMBER(m72_mcu_sample_r);
	DECLARE_WRITE8_MEMBER(m72_mcu_ack_w);
	DECLARE_READ8_MEMBER(m72_mcu_snd_r);
	DECLARE_READ8_MEMBER(m72_mcu_port_r);
	DECLARE_WRITE8_MEMBER(m72_mcu_port_w);
	DECLARE_WRITE8_MEMBER(m72_mcu_low_w);
	DECLARE_WRITE8_MEMBER(m72_mcu_high_w);
	DECLARE_READ8_MEMBER(m72_snd_cpu_sample_r);
	DECLARE_WRITE16_MEMBER(bchopper_sample_trigger_w);
	DECLARE_WRITE16_MEMBER(nspirit_sample_trigger_w);
	DECLARE_WRITE16_MEMBER(imgfight_sample_trigger_w);
	DECLARE_WRITE16_MEMBER(loht_sample_trigger_w);
	DECLARE_WRITE16_MEMBER(xmultiplm72_sample_trigger_w);
	DECLARE_WRITE16_MEMBER(dbreedm72_sample_trigger_w);
	DECLARE_WRITE16_MEMBER(airduel_sample_trigger_w);
	DECLARE_WRITE16_MEMBER(dkgenm72_sample_trigger_w);
	DECLARE_WRITE16_MEMBER(gallop_sample_trigger_w);
	DECLARE_READ16_MEMBER(protection_r);
	DECLARE_WRITE16_MEMBER(protection_w);
	DECLARE_READ16_MEMBER(soundram_r);
	DECLARE_WRITE16_MEMBER(soundram_w);
	DECLARE_READ16_MEMBER(poundfor_trackball_r);
	DECLARE_READ16_MEMBER(m72_palette1_r);
	DECLARE_READ16_MEMBER(m72_palette2_r);
	DECLARE_WRITE16_MEMBER(m72_palette1_w);
	DECLARE_WRITE16_MEMBER(m72_palette2_w);
	DECLARE_WRITE16_MEMBER(m72_videoram1_w);
	DECLARE_WRITE16_MEMBER(m72_videoram2_w);
	DECLARE_WRITE16_MEMBER(m72_irq_line_w);
	DECLARE_WRITE16_MEMBER(m72_scrollx1_w);
	DECLARE_WRITE16_MEMBER(m72_scrollx2_w);
	DECLARE_WRITE16_MEMBER(m72_scrolly1_w);
	DECLARE_WRITE16_MEMBER(m72_scrolly2_w);
	DECLARE_WRITE16_MEMBER(m72_dmaon_w);
	DECLARE_WRITE16_MEMBER(m72_port02_w);
	DECLARE_WRITE16_MEMBER(rtype2_port02_w);
	DECLARE_WRITE16_MEMBER(majtitle_gfx_ctrl_w);
};


/*----------- defined in video/m72.c -----------*/

VIDEO_START( m72 );
VIDEO_START( rtype2 );
VIDEO_START( majtitle );
VIDEO_START( hharry );
VIDEO_START( poundfor );
VIDEO_START( xmultipl );
VIDEO_START( hharryu );


SCREEN_UPDATE_IND16( m72 );
SCREEN_UPDATE_IND16( majtitle );
