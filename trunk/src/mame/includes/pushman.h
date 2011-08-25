/*************************************************************************

    Pushman

*************************************************************************/

class pushman_state : public driver_device
{
public:
	pushman_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *   m_videoram;
	UINT16 *   m_spriteram;
//  UINT16 *   m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_tx_tilemap;
	UINT16     m_control[2];

	/* misc */
	UINT8      m_shared_ram[8];
	UINT16     m_latch;
	UINT16     m_new_latch;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_mcu;
};


/*----------- defined in video/pushman.c -----------*/

WRITE16_HANDLER( pushman_scroll_w );
WRITE16_HANDLER( pushman_videoram_w );

VIDEO_START( pushman );

SCREEN_UPDATE( pushman );
