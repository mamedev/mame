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
};


/*----------- defined in video/m72.c -----------*/

VIDEO_START( m72 );
VIDEO_START( rtype2 );
VIDEO_START( majtitle );
VIDEO_START( hharry );
VIDEO_START( poundfor );
VIDEO_START( xmultipl );
VIDEO_START( hharryu );

READ16_HANDLER( m72_palette1_r );
READ16_HANDLER( m72_palette2_r );
WRITE16_HANDLER( m72_palette1_w );
WRITE16_HANDLER( m72_palette2_w );
WRITE16_HANDLER( m72_videoram1_w );
WRITE16_HANDLER( m72_videoram2_w );
WRITE16_HANDLER( m72_irq_line_w );
WRITE16_HANDLER( m72_scrollx1_w );
WRITE16_HANDLER( m72_scrollx2_w );
WRITE16_HANDLER( m72_scrolly1_w );
WRITE16_HANDLER( m72_scrolly2_w );
WRITE16_HANDLER( m72_dmaon_w );
WRITE16_HANDLER( m72_port02_w );
WRITE16_HANDLER( rtype2_port02_w );
WRITE16_HANDLER( majtitle_gfx_ctrl_w );

SCREEN_UPDATE( m72 );
SCREEN_UPDATE( majtitle );
