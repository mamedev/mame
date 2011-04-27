/***************************************************************************

    Black Tiger

***************************************************************************/

class blktiger_state : public driver_device
{
public:
	blktiger_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 * m_txvideoram;
//  UINT8 * m_spriteram;  // currently this uses generic buffer_spriteram_w
//  UINT8 * m_paletteram; // currently this uses generic palette handling
//  UINT8 * m_paletteram2;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_bg_tilemap8x4;
	tilemap_t *m_bg_tilemap4x8;
	UINT32  m_scroll_bank;
	UINT8   m_scroll_x[2];
	UINT8   m_scroll_y[2];
	UINT8   *m_scroll_ram;
	UINT8   m_screen_layout;
	UINT8   m_chon;
	UINT8   m_objon;
	UINT8   m_bgon;

	/* mcu-related */
	UINT8   m_z80_latch;
	UINT8   m_i8751_latch;

	/* devices */
	device_t *m_mcu;
	device_t *m_audiocpu;
};


/*----------- defined in video/blktiger.c -----------*/

WRITE8_HANDLER( blktiger_screen_layout_w );

READ8_HANDLER( blktiger_bgvideoram_r );
WRITE8_HANDLER( blktiger_bgvideoram_w );
WRITE8_HANDLER( blktiger_txvideoram_w );
WRITE8_HANDLER( blktiger_video_control_w );
WRITE8_HANDLER( blktiger_video_enable_w );
WRITE8_HANDLER( blktiger_bgvideoram_bank_w );
WRITE8_HANDLER( blktiger_scrollx_w );
WRITE8_HANDLER( blktiger_scrolly_w );

VIDEO_START( blktiger );
SCREEN_UPDATE( blktiger );
SCREEN_EOF( blktiger );
